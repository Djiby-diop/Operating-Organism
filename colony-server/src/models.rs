use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

/// OO Organism Heartbeat (sent by each oo-host to colony server)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Heartbeat {
    pub organism_id: String,
    pub habitat: String,  // win10-laptop, debian-pc, etc.
    pub timestamp: DateTime<Utc>,
    pub state: OrganismState,
    pub immune_signals: Vec<ThreatSignal>,
    pub mutations: Vec<Mutation>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct OrganismState {
    pub continuity_epoch: u64,
    pub mode: String,  // "supervised", "autonomous", "sleeping"
    pub policy_enforcement: String,
    pub goal_counts: GoalCounts,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GoalCounts {
    pub pending: usize,
    pub doing: usize,
    pub done: usize,
    pub blocked: usize,
    pub aborted: usize,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ThreatSignal {
    pub threat_id: String,
    pub severity: String,  // "low", "medium", "high", "critical"
    pub first_seen: DateTime<Utc>,
    pub count: usize,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Mutation {
    pub kind: String,
    pub fitness: f64,  // 0.0 to 1.0
    pub discovered_at: DateTime<Utc>,
}

/// Server-wide threat registry
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ThreatRegistry {
    pub threats: HashMap<String, AggregatedThreat>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AggregatedThreat {
    pub threat_id: String,
    pub severity: String,
    pub discovered_by: String,  // organism_id that first reported it
    pub first_seen: DateTime<Utc>,
    pub last_seen: DateTime<Utc>,
    pub observer_count: usize,  // how many OO have seen this?
    pub total_incidents: usize,
}

/// Mutation fitness ranking
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MutationFitness {
    pub kind: String,
    pub avg_fitness: f64,
    pub sample_size: usize,
    pub discovered_by: Vec<String>,  // which organisms reported this?
    pub last_updated: DateTime<Utc>,
}

/// Colony aggregate status
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ColonyStatus {
    pub alive_organisms: usize,
    pub total_organisms_seen: usize,
    pub threat_count: usize,
    pub critical_threats: Vec<String>,
    pub viable_mutations: Vec<String>,  // fitness > 0.80
    pub synchronization_lag_s: u64,  // max seconds since last heartbeat
    pub fossil_size_mb: f64,
    pub last_updated: DateTime<Utc>,
}

/// Server response to heartbeat (directives to OO)
#[derive(Debug, Serialize, Deserialize)]
pub struct HeartbeatResponse {
    pub status: String,  // "ok", "error"
    pub message: String,
    pub directives: Vec<Directive>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Directive {
    pub kind: String,  // "threat_alert", "mutation_suggest", "reflex_action"
    pub content: serde_json::Value,
}

/// Local node identity inside the mesh
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NodeProfile {
    pub node_id: String,
    pub role: String,
    pub bind_addr: String,
    pub first_seen: DateTime<Utc>,
    pub last_seen: DateTime<Utc>,
}

/// Peer registration payload
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PeerRegistration {
    pub peer_id: String,
    pub address: String,
    pub role: String,
}

/// Mesh gossip envelope sent from one colony node to another
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GossipEnvelope {
    pub gossip_id: String,
    pub ttl_hops: u8,
    pub from: PeerRegistration,
    pub observed_threats: Vec<String>,
    pub observed_organisms: usize,
    pub sent_at: DateTime<Utc>,
    pub hmac_sha256: Option<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GossipAck {
    pub status: String,
    pub local_node_id: String,
    pub gossip_id: String,
    pub duplicate: bool,
    pub relayed: bool,
    pub ttl_hops_remaining: u8,
    pub known_peer_count: usize,
    pub received_at: DateTime<Utc>,
}

/// Persisted peer record
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MeshPeer {
    pub peer_id: String,
    pub address: String,
    pub role: String,
    pub first_seen: DateTime<Utc>,
    pub last_seen: DateTime<Utc>,
    pub heartbeat_count: usize,
}

/// Registry of known peers
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MeshRegistry {
    pub peers: HashMap<String, MeshPeer>,
}

/// Mesh-wide status view
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MeshState {
    pub local_node: NodeProfile,
    pub peer_count: usize,
    pub peers: Vec<MeshPeer>,
    pub colony_status: ColonyStatus,
}

/// Live organisms registry
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct OrganismRegistry {
    pub organisms: HashMap<String, LiveOrganism>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LiveOrganism {
    pub organism_id: String,
    pub habitat: String,
    pub last_heartbeat: DateTime<Utc>,
    pub is_alive: bool,
    pub continuity_epoch: u64,
}
