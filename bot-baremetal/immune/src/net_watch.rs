//! BOT-BAREMETAL — NetWatch Agent
//! Surveillance réseau : C2 beacons, exfiltration, tunneling DNS.

use crate::swarm_mind::{AgentRole, SwarmEvent, ThreatLevel};
use std::collections::HashMap;

pub struct NetWatchAgent {
    /// Connexions par destination : ip:port → count
    connection_counts: HashMap<String, u64>,
    /// Volume de données sortantes en bytes (fenêtre glissante)
    outbound_bytes: u64,
    pub threats_caught: u64,
    /// Seuil de beacon (connexions répétées vers même dest)
    beacon_threshold: u64,
    /// Seuil d'exfiltration (bytes sortants)
    exfil_threshold_bytes: u64,
}

impl NetWatchAgent {
    pub fn new() -> Self {
        Self {
            connection_counts: HashMap::new(),
            outbound_bytes: 0,
            threats_caught: 0,
            beacon_threshold: 10,
            exfil_threshold_bytes: 50 * 1024 * 1024, // 50 MB
        }
    }

    /// Signale une connexion sortante.
    pub fn on_connection(&mut self, dest_ip: &str, dest_port: u16,
                         bytes_sent: u64) -> Option<SwarmEvent> {
        let key = format!("{}:{}", dest_ip, dest_port);
        let count = self.connection_counts.entry(key.clone()).or_insert(0);
        *count += 1;
        self.outbound_bytes += bytes_sent;

        // Détection de C2 beacon : trop de connexions vers la même destination
        if *count >= self.beacon_threshold {
            self.threats_caught += 1;
            return Some(SwarmEvent {
                from_role:    AgentRole::NetWatch,
                threat_level: ThreatLevel::Alert,
                description:  format!(
                    "C2 beacon suspected: {} ({} connections)", key, count
                ),
                timestamp_ns: 0,
                confidence:   72,
            });
        }

        // Détection d'exfiltration : volume de données sortantes anormal
        if self.outbound_bytes >= self.exfil_threshold_bytes {
            self.threats_caught += 1;
            return Some(SwarmEvent {
                from_role:    AgentRole::NetWatch,
                threat_level: ThreatLevel::Combat,
                description:  format!(
                    "Data exfiltration suspected: {} bytes sent", self.outbound_bytes
                ),
                timestamp_ns: 0,
                confidence:   80,
            });
        }

        None
    }

    pub fn reset_window(&mut self) {
        self.outbound_bytes = 0;
        self.connection_counts.clear();
    }
}

impl Default for NetWatchAgent {
    fn default() -> Self { Self::new() }
}
