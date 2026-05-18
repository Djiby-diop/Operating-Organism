//! BOT-BAREMETAL — Quarantine Agent
//! Isolation instantanée de processus et fichiers suspects.
//! Décision en < 5ms. Aucune hésitation.

use crate::swarm_mind::{AgentRole, SwarmEvent, ThreatLevel};

#[derive(Debug, Clone)]
pub struct QuarantineEntry {
    pub kind:        QuarantineKind,
    pub identifier:  String,   // PID ou chemin fichier
    pub reason:      String,
    pub timestamp:   u64,
    pub is_released: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum QuarantineKind {
    Process,
    File,
    NetworkEndpoint,
}

pub struct QuarantineAgent {
    pub quarantined: Vec<QuarantineEntry>,
    pub total_quarantined: u64,
}

impl QuarantineAgent {
    pub fn new() -> Self {
        Self {
            quarantined: Vec::new(),
            total_quarantined: 0,
        }
    }

    /// Isole un processus suspect.
    /// En production : suspend le processus, coupe son accès réseau.
    pub fn quarantine_process(&mut self, pid: u32, reason: &str) -> SwarmEvent {
        let entry = QuarantineEntry {
            kind:        QuarantineKind::Process,
            identifier:  format!("pid:{}", pid),
            reason:      reason.to_string(),
            timestamp:   0,
            is_released: false,
        };
        self.quarantined.push(entry);
        self.total_quarantined += 1;

        eprintln!("[Quarantine] Process pid={} isolated — reason: {}", pid, reason);

        SwarmEvent {
            from_role:    AgentRole::Quarantine,
            threat_level: ThreatLevel::Alert,
            description:  format!("Process {} quarantined: {}", pid, reason),
            timestamp_ns: 0,
            confidence:   95,
        }
    }

    /// Isole un fichier suspect.
    pub fn quarantine_file(&mut self, path: &str, reason: &str) -> SwarmEvent {
        let entry = QuarantineEntry {
            kind:        QuarantineKind::File,
            identifier:  path.to_string(),
            reason:      reason.to_string(),
            timestamp:   0,
            is_released: false,
        };
        self.quarantined.push(entry);
        self.total_quarantined += 1;

        eprintln!("[Quarantine] File '{}' isolated — reason: {}", path, reason);

        SwarmEvent {
            from_role:    AgentRole::Quarantine,
            threat_level: ThreatLevel::Alert,
            description:  format!("File '{}' quarantined: {}", path, reason),
            timestamp_ns: 0,
            confidence:   90,
        }
    }

    /// Libère une entrée de quarantaine (après validation).
    pub fn release(&mut self, identifier: &str) -> bool {
        for entry in &mut self.quarantined {
            if entry.identifier == identifier && !entry.is_released {
                entry.is_released = true;
                eprintln!("[Quarantine] Released: {}", identifier);
                return true;
            }
        }
        false
    }

    pub fn active_count(&self) -> usize {
        self.quarantined.iter().filter(|e| !e.is_released).count()
    }
}

impl Default for QuarantineAgent {
    fn default() -> Self { Self::new() }
}
