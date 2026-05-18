//! BOT-BAREMETAL — FsWatch Agent
//! Surveillance du système de fichiers.
//! Détecte : chiffrement de masse, modifications critiques, suppressions rapides.

use crate::swarm_mind::{AgentRole, SwarmEvent, ThreatLevel};
use std::collections::HashMap;

pub struct FsWatchAgent {
    /// Suivi des modifications récentes : chemin → compteur d'opérations
    recent_modifications: HashMap<String, u32>,
    pub threats_caught: u64,
    /// Seuil de modifications en rafale (ransomware heuristic)
    mass_modify_threshold: u32,
}

impl FsWatchAgent {
    pub fn new() -> Self {
        Self {
            recent_modifications: HashMap::new(),
            threats_caught: 0,
            mass_modify_threshold: 50,
        }
    }

    /// Signale une modification de fichier.
    /// Retourne un SwarmEvent si un comportement suspect est détecté.
    pub fn on_file_modified(&mut self, path: &str, extension: &str) -> Option<SwarmEvent> {
        let count = self.recent_modifications
            .entry(path.to_string())
            .or_insert(0);
        *count += 1;

        // Extensions typiques de ransomware
        let ransom_exts = [".locked", ".encrypted", ".crypt", ".enc",
                           ".pay2me", ".zzzzz", ".zepto"];
        if ransom_exts.iter().any(|e| extension.ends_with(e)) {
            self.threats_caught += 1;
            return Some(SwarmEvent {
                from_role:    AgentRole::FsWatch,
                threat_level: ThreatLevel::Combat,
                description:  format!("Ransomware extension detected: {}", path),
                timestamp_ns: 0,
                confidence:   90,
            });
        }

        // Heuristique : trop de modifications en rafale
        let total_mods: u32 = self.recent_modifications.values().sum();
        if total_mods >= self.mass_modify_threshold {
            self.threats_caught += 1;
            return Some(SwarmEvent {
                from_role:    AgentRole::FsWatch,
                threat_level: ThreatLevel::Alert,
                description:  format!(
                    "Mass file modification detected ({} files)", total_mods
                ),
                timestamp_ns: 0,
                confidence:   70,
            });
        }

        None
    }

    /// Réinitialise les compteurs (ex: après validation d'une activité légitime).
    pub fn reset_counters(&mut self) {
        self.recent_modifications.clear();
    }
}

impl Default for FsWatchAgent {
    fn default() -> Self { Self::new() }
}
