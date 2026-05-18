use crate::models::{
    GossipEnvelope,
    Heartbeat,
    LiveOrganism,
    MeshPeer,
    MeshRegistry,
    MeshState,
    NodeProfile,
    OrganismRegistry,
    PeerRegistration,
    ThreatRegistry,
};
use anyhow::{Context, Result};
use chrono::Utc;
use serde_json::{json, Value};
use std::fs::{self, OpenOptions};
use std::io::{BufRead, BufReader, Write};
use std::path::{Path, PathBuf};
use uuid::Uuid;

const FOSSIL_DIR: &str = "fossil";
const ORGANISMS_DIR: &str = "fossil/organisms";
const MESH_DIR: &str = "fossil/mesh";
const THREATS_FILE: &str = "fossil/threats/threat_signatures.jsonl";
const MUTATIONS_FILE: &str = "fossil/mutations/fitness_archive.jsonl";
const LIVE_ORGANISMS_FILE: &str = "fossil/organisms.json";
const NODE_PROFILE_FILE: &str = "fossil/node.json";
const PEERS_FILE: &str = "fossil/mesh/peers.json";
const GOSSIP_FILE: &str = "fossil/mesh/gossip.ndjson";
const THREATS_LIVE_FILE: &str = "fossil/live/threats.json";
const MUTATIONS_LIVE_FILE: &str = "fossil/live/mutations.json";

pub struct Storage {
    base_dir: PathBuf,
}

impl Storage {
    pub fn new(base_dir: &str) -> Result<Self> {
        let storage = Storage {
            base_dir: PathBuf::from(base_dir),
        };

        storage.ensure_directories()?;
        Ok(storage)
    }

    fn path(&self, relative: &str) -> PathBuf {
        self.base_dir.join(relative)
    }

    fn ensure_directories(&self) -> Result<()> {
        fs::create_dir_all(self.path(ORGANISMS_DIR))?;
        fs::create_dir_all(self.path(MESH_DIR))?;
        fs::create_dir_all(self.path("fossil/live"))?;
        fs::create_dir_all(self.path("fossil/threats"))?;
        fs::create_dir_all(self.path("fossil/mutations"))?;
        Ok(())
    }

    /// Store a heartbeat from an OO organism
    pub fn store_heartbeat(&self, heartbeat: &Heartbeat) -> Result<()> {
        // 1. Append to organism's fossil record
        let org_file = self.path(&format!("{}/{}.ndjson", ORGANISMS_DIR, heartbeat.organism_id));
        let mut file = OpenOptions::new()
            .create(true)
            .append(true)
            .open(&org_file)?;

        writeln!(file, "{}", serde_json::to_string(heartbeat)?)?;

        // 2. Update live registry
        self.update_live_registry(&heartbeat)?;

        // 3. Process threats
        self.process_threats(&heartbeat)?;

        // 4. Process mutations
        self.process_mutations(&heartbeat)?;

        Ok(())
    }

    fn update_live_registry(&self, heartbeat: &Heartbeat) -> Result<()> {
        let mut registry: OrganismRegistry = self
            .read_json(LIVE_ORGANISMS_FILE)
            .unwrap_or_else(|_| OrganismRegistry {
                organisms: Default::default(),
            });

        registry.organisms.insert(
            heartbeat.organism_id.clone(),
            LiveOrganism {
                organism_id: heartbeat.organism_id.clone(),
                habitat: heartbeat.habitat.clone(),
                last_heartbeat: heartbeat.timestamp,
                is_alive: true,
                continuity_epoch: heartbeat.state.continuity_epoch,
            },
        );

        self.write_json(LIVE_ORGANISMS_FILE, &registry)?;
        Ok(())
    }

    /// Create or update the local node profile on disk.
    pub fn get_or_create_node_profile(&self, bind_addr: &str) -> Result<NodeProfile> {
        let path = self.path(NODE_PROFILE_FILE);
        if path.exists() {
            let mut profile: NodeProfile = self.read_json_path(&path)?;
            profile.bind_addr = bind_addr.to_string();
            profile.last_seen = Utc::now();
            self.write_json_path(&path, &profile)?;
            return Ok(profile);
        }

        let profile = NodeProfile {
            node_id: std::env::var("COLONY_NODE_ID").unwrap_or_else(|_| Uuid::new_v4().to_string()),
            role: std::env::var("COLONY_ROLE").unwrap_or_else(|_| "relay".to_string()),
            bind_addr: bind_addr.to_string(),
            first_seen: Utc::now(),
            last_seen: Utc::now(),
        };

        self.write_json_path(&path, &profile)?;
        Ok(profile)
    }

    /// Register a peer in the mesh registry.
    pub fn register_peer(&self, peer: &PeerRegistration) -> Result<MeshPeer> {
        let mut registry = self.get_mesh_registry()?;
        let now = Utc::now();

        let record = registry
            .peers
            .entry(peer.peer_id.clone())
            .and_modify(|existing| {
                existing.address = peer.address.clone();
                existing.role = peer.role.clone();
                existing.last_seen = now;
                existing.heartbeat_count += 1;
            })
            .or_insert_with(|| MeshPeer {
                peer_id: peer.peer_id.clone(),
                address: peer.address.clone(),
                role: peer.role.clone(),
                first_seen: now,
                last_seen: now,
                heartbeat_count: 1,
            })
            .clone();

        self.write_json(PEERS_FILE, &registry)?;
        Ok(record)
    }

    pub fn register_bootstrap_peers(&self, peers: &[PeerRegistration], local_node_id: &str) -> Result<usize> {
        let mut applied = 0usize;

        for peer in peers {
            if peer.peer_id == local_node_id {
                continue;
            }

            self.register_peer(peer)?;
            applied += 1;
        }

        Ok(applied)
    }

    pub fn store_gossip(&self, gossip: &GossipEnvelope) -> Result<MeshPeer> {
        let registered = self.register_peer(&gossip.from)?;

        let mut file = OpenOptions::new()
            .create(true)
            .append(true)
            .open(self.path(GOSSIP_FILE))?;

        writeln!(
            file,
            "{}",
            serde_json::to_string(&json!({
                "gossip_id": gossip.gossip_id,
                "from": gossip.from,
                "observed_threats": gossip.observed_threats,
                "observed_organisms": gossip.observed_organisms,
                "sent_at": gossip.sent_at,
                "received_at": Utc::now(),
            }))?
        )?;

        Ok(registered)
    }

    /// Load recent gossip IDs from disk to populate deduplication cache on startup.
    pub fn load_seen_gossip(&self, dedup_window_s: i64) -> Result<std::collections::HashMap<String, i64>> {
        let mut seen = std::collections::HashMap::new();
        let path = self.path(GOSSIP_FILE);
        if !path.exists() {
            return Ok(seen);
        }

        let file = std::fs::File::open(path)?;
        let reader = std::io::BufReader::new(file);
        let now = Utc::now().timestamp();

        for line in reader.lines() {
            let line = line?;
            if let Ok(record) = serde_json::from_str::<serde_json::Value>(&line) {
                if let (Some(id), Some(received_at_str)) = (
                    record.get("gossip_id").and_then(|v| v.as_str()),
                    record.get("received_at").and_then(|v| v.as_str())
                ) {
                    if let Ok(dt) = chrono::DateTime::parse_from_rfc3339(received_at_str) {
                        let ts = dt.timestamp();
                        if now - ts <= dedup_window_s {
                            seen.insert(id.to_string(), ts);
                        }
                    }
                }
            }
        }
        Ok(seen)
    }

    pub fn get_mesh_registry(&self) -> Result<MeshRegistry> {
        let mut registry: MeshRegistry = self.read_json(PEERS_FILE).unwrap_or_else(|_| {
            MeshRegistry {
                peers: Default::default(),
            }
        });

        // Eviction policy: remove peers not seen for 1 hour
        let now = Utc::now();
        let timeout_s = 3600; // 1 hour
        let initial_len = registry.peers.len();

        registry.peers.retain(|_, peer| {
            (now - peer.last_seen).num_seconds() <= timeout_s
        });

        if registry.peers.len() < initial_len {
            self.write_json(PEERS_FILE, &registry)?;
        }

        Ok(registry)
    }

    pub fn get_mesh_state(&self, local_node: &NodeProfile) -> Result<MeshState> {
        let registry = self.get_mesh_registry()?;
        let colony_status = self.get_colony_status()?;

        Ok(MeshState {
            local_node: local_node.clone(),
            peer_count: registry.peers.len(),
            peers: registry.peers.values().cloned().collect(),
            colony_status,
        })
    }

    fn process_threats(&self, heartbeat: &Heartbeat) -> Result<()> {
        // Append each threat to fossil record
        for threat in &heartbeat.immune_signals {
            let mut file = OpenOptions::new()
                .create(true)
                .append(true)
                .open(self.path(THREATS_FILE))?;

            let record = json!({
                "threat_id": threat.threat_id,
                "severity": threat.severity,
                "discovered_by": heartbeat.organism_id,
                "first_seen": threat.first_seen,
                "count": threat.count,
                "timestamp": Utc::now(),
            });

            writeln!(file, "{}", record.to_string())?;
        }

        Ok(())
    }

    fn process_mutations(&self, heartbeat: &Heartbeat) -> Result<()> {
        // Append each mutation to fossil record
        for mutation in &heartbeat.mutations {
            let mut file = OpenOptions::new()
                .create(true)
                .append(true)
                .open(self.path(MUTATIONS_FILE))?;

            let record = json!({
                "kind": mutation.kind,
                "fitness": mutation.fitness,
                "discovered_by": heartbeat.organism_id,
                "discovered_at": mutation.discovered_at,
                "timestamp": Utc::now(),
            });

            writeln!(file, "{}", record.to_string())?;
        }

        Ok(())
    }

    /// Get aggregated threat signatures with majority-vote conflict resolution for severity
    pub fn get_threat_registry(&self) -> Result<ThreatRegistry> {
        let mut registry = ThreatRegistry {
            threats: Default::default(),
        };

        if !self.path(THREATS_FILE).exists() {
            return Ok(registry);
        }

        let file = std::fs::File::open(self.path(THREATS_FILE))?;
        let reader = BufReader::new(file);

        // Local state for conflict resolution (threat_id -> severity -> votes)
        let mut severity_votes: std::collections::HashMap<String, std::collections::HashMap<String, usize>> = std::collections::HashMap::new();

        for line in reader.lines() {
            let line = line?;
            if let Ok(record) = serde_json::from_str::<Value>(&line) {
                if let (Some(threat_id), Some(severity)) = (
                    record.get("threat_id").and_then(|v| v.as_str()),
                    record.get("severity").and_then(|v| v.as_str())
                ) {
                    let votes = severity_votes.entry(threat_id.to_string()).or_default();
                    *votes.entry(severity.to_string()).or_default() += 1;

                    registry
                        .threats
                        .entry(threat_id.to_string())
                        .and_modify(|t| {
                            t.total_incidents += record
                                .get("count")
                                .and_then(|v| v.as_u64())
                                .unwrap_or(1) as usize;
                            t.observer_count += 1;
                            if let Some(ts) = record
                                .get("timestamp")
                                .and_then(|v| v.as_str())
                                .and_then(|s| chrono::DateTime::parse_from_rfc3339(s).ok())
                            {
                                t.last_seen = ts.with_timezone(&Utc);
                            }
                        })
                        .or_insert_with(|| {
                            crate::models::AggregatedThreat {
                                threat_id: threat_id.to_string(),
                                severity: severity.to_string(), // Initial guess
                                discovered_by: record
                                    .get("discovered_by")
                                    .and_then(|v| v.as_str())
                                    .unwrap_or("unknown")
                                    .to_string(),
                                first_seen: record
                                    .get("first_seen")
                                    .and_then(|v| v.as_str())
                                    .and_then(|s| chrono::DateTime::parse_from_rfc3339(s).ok())
                                    .map(|dt| dt.with_timezone(&Utc))
                                    .unwrap_or_else(Utc::now),
                                last_seen: Utc::now(),
                                observer_count: 1,
                                total_incidents: record
                                    .get("count")
                                    .and_then(|v| v.as_u64())
                                    .unwrap_or(1) as usize,
                            }
                        });
                }
            }
        }

        // Resolve severity conflicts by majority vote
        for (threat_id, votes) in severity_votes {
            if let Some(t) = registry.threats.get_mut(&threat_id) {
                if let Some((best_severity, _)) = votes.into_iter().max_by_key(|&(_, count)| count) {
                    t.severity = best_severity;
                }
            }
        }

        Ok(registry)
    }

    /// Get live organism registry
    pub fn get_organisms(&self) -> Result<OrganismRegistry> {
        self.read_json(LIVE_ORGANISMS_FILE).or_else(|_| {
            Ok(OrganismRegistry {
                organisms: Default::default(),
            })
        })
    }

    /// Read JSON file
    fn read_json<T: serde::de::DeserializeOwned>(&self, path: &str) -> Result<T> {
        let content = fs::read_to_string(self.path(path))?;
        serde_json::from_str(&content).context("Failed to parse JSON")
    }

    fn read_json_path<T: serde::de::DeserializeOwned>(&self, path: &Path) -> Result<T> {
        let content = fs::read_to_string(path)?;
        serde_json::from_str(&content).context("Failed to parse JSON")
    }

    /// Write JSON file
    fn write_json<T: serde::Serialize>(&self, path: &str, data: &T) -> Result<()> {
        let json = serde_json::to_string_pretty(data)?;
        fs::write(self.path(path), json)?;
        Ok(())
    }

    fn write_json_path<T: serde::Serialize>(&self, path: &Path, data: &T) -> Result<()> {
        let json = serde_json::to_string_pretty(data)?;
        fs::write(path, json)?;
        Ok(())
    }

    /// Get aggregate colony status
    pub fn get_colony_status(&self) -> Result<crate::models::ColonyStatus> {
        let organisms = self.get_organisms()?;
        let threats = self.get_threat_registry()?;

        let alive_count = organisms
            .organisms
            .values()
            .filter(|o| o.is_alive)
            .count();

        let critical_threats: Vec<String> = threats
            .threats
            .values()
            .filter(|t| t.severity == "critical")
            .map(|t| t.threat_id.clone())
            .collect();

        let max_lag = organisms
            .organisms
            .values()
            .map(|o| (Utc::now() - o.last_heartbeat).num_seconds() as u64)
            .max()
            .unwrap_or(0);

        Ok(crate::models::ColonyStatus {
            alive_organisms: alive_count,
            total_organisms_seen: organisms.organisms.len(),
            threat_count: threats.threats.len(),
            critical_threats,
            viable_mutations: vec![],  // TODO: compute from mutations
            synchronization_lag_s: max_lag,
            fossil_size_mb: self.compute_fossil_size()?,
            last_updated: Utc::now(),
        })
    }

    fn compute_fossil_size(&self) -> Result<f64> {
        let mut total_size = 0u64;

        for entry in walkdir::WalkDir::new(self.path(FOSSIL_DIR))
            .into_iter()
            .filter_map(|e| e.ok())
        {
            if entry.is_file() {
                total_size += fs::metadata(&entry)?.len();
            }
        }

        Ok(total_size as f64 / (1024.0 * 1024.0))  // Convert to MB
    }
}

// Helper for walkdir
mod walkdir {
    use std::fs;
    use std::path::{Path, PathBuf};

    pub struct WalkDir {
        path: PathBuf,
    }

    impl WalkDir {
        pub fn new<P: AsRef<Path>>(path: P) -> Self {
            WalkDir {
                path: path.as_ref().to_path_buf(),
            }
        }

        pub fn into_iter(self) -> WalkDirIter {
            WalkDirIter {
                dirs: vec![self.path],
                current: None,
            }
        }
    }

    pub struct WalkDirIter {
        dirs: Vec<PathBuf>,
        current: Option<std::vec::IntoIter<PathBuf>>,
    }

    impl Iterator for WalkDirIter {
        type Item = Result<PathBuf, std::io::Error>;

        fn next(&mut self) -> Option<Self::Item> {
            loop {
                if let Some(ref mut current) = self.current {
                    if let Some(path) = current.next() {
                        return Some(Ok(path));
                    }
                }

                if let Some(dir_path) = self.dirs.pop() {
                    match fs::read_dir(&dir_path) {
                        Ok(entries) => {
                            let entries: Vec<_> = entries
                                .filter_map(|e| e.ok())
                                .map(|e| e.path())
                                .collect();

                            // Separate files and directories
                            let mut subdirs = Vec::new();
                            let mut files = Vec::new();

                            for path in entries {
                                if path.is_dir() {
                                    subdirs.push(path);
                                } else {
                                    files.push(path);
                                }
                            }

                            self.dirs.extend(subdirs);
                            self.current = Some(files.into_iter());
                        }
                        Err(e) => return Some(Err(e)),
                    }
                } else {
                    return None;
                }
            }
        }
    }
}
