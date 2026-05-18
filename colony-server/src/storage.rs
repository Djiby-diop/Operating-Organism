use crate::models::{Heartbeat, ThreatRegistry, OrganismRegistry, LiveOrganism};
use anyhow::{Context, Result};
use chrono::Utc;
use serde_json::{json, Value};
use std::fs::{self, OpenOptions};
use std::io::{BufRead, BufReader, Write};
use std::path::Path;

const FOSSIL_DIR: &str = "fossil";
const ORGANISMS_DIR: &str = "fossil/organisms";
const THREATS_FILE: &str = "fossil/threats/threat_signatures.jsonl";
const MUTATIONS_FILE: &str = "fossil/mutations/fitness_archive.jsonl";
const LIVE_ORGANISMS_FILE: &str = "fossil/organisms.json";
const THREATS_LIVE_FILE: &str = "fossil/live/threats.json";
const MUTATIONS_LIVE_FILE: &str = "fossil/live/mutations.json";

pub struct Storage {
    base_dir: String,
}

impl Storage {
    pub fn new(base_dir: &str) -> Result<Self> {
        // Ensure directory structure exists
        fs::create_dir_all(ORGANISMS_DIR)?;
        fs::create_dir_all("fossil/live")?;
        fs::create_dir_all("fossil/threats")?;
        fs::create_dir_all("fossil/mutations")?;

        Ok(Storage {
            base_dir: base_dir.to_string(),
        })
    }

    /// Store a heartbeat from an OO organism
    pub fn store_heartbeat(&self, heartbeat: &Heartbeat) -> Result<()> {
        // 1. Append to organism's fossil record
        let org_file = format!("{}/{}.ndjson", ORGANISMS_DIR, heartbeat.organism_id);
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

    fn process_threats(&self, heartbeat: &Heartbeat) -> Result<()> {
        // Append each threat to fossil record
        for threat in &heartbeat.immune_signals {
            let mut file = OpenOptions::new()
                .create(true)
                .append(true)
                .open(THREATS_FILE)?;

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
                .open(MUTATIONS_FILE)?;

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

    /// Get aggregated threat signatures
    pub fn get_threat_registry(&self) -> Result<ThreatRegistry> {
        let mut registry = ThreatRegistry {
            threats: Default::default(),
        };

        if !Path::new(THREATS_FILE).exists() {
            return Ok(registry);
        }

        let file = std::fs::File::open(THREATS_FILE)?;
        let reader = BufReader::new(file);

        for line in reader.lines() {
            let line = line?;
            if let Ok(record) = serde_json::from_str::<Value>(&line) {
                if let Some(threat_id) = record.get("threat_id").and_then(|v| v.as_str()) {
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
                                severity: record
                                    .get("severity")
                                    .and_then(|v| v.as_str())
                                    .unwrap_or("unknown")
                                    .to_string(),
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
        let content = fs::read_to_string(path)?;
        serde_json::from_str(&content).context("Failed to parse JSON")
    }

    /// Write JSON file
    fn write_json<T: serde::Serialize>(&self, path: &str, data: &T) -> Result<()> {
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

        for entry in walkdir::WalkDir::new(FOSSIL_DIR)
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
