//! BOT-BAREMETAL — RegenAgent
//! Auto-reconstruction de la flotte.
//!
//! "Le Bot ne peut pas mourir durablement.
//!  Il se régénère comme une cellule souche."

use crate::swarm_mind::{AgentRole, SwarmEvent, ThreatLevel};

/// Snapshot de l'état d'un agent — utilisé pour la régénération
#[derive(Debug, Clone)]
pub struct AgentSnapshot {
    pub role:         AgentRole,
    pub generation:   u32,
    pub dna_hash:     [u8; 32],     // Hash SHA256 de l'ADN de l'agent
    pub snapshot_time: u64,
    pub is_valid:     bool,
}

impl AgentSnapshot {
    pub fn new(role: AgentRole, generation: u32) -> Self {
        Self {
            role,
            generation,
            dna_hash: [0u8; 32],   // À remplir avec le vrai hash
            snapshot_time: 0,
            is_valid: true,
        }
    }
}

pub struct RegenAgent {
    /// Bibliothèque de snapshots sains (ADN de référence)
    pub snapshots:       Vec<AgentSnapshot>,
    pub regens_performed: u64,
    pub is_healthy:      bool,
}

impl RegenAgent {
    pub fn new() -> Self {
        Self {
            snapshots:        Vec::new(),
            regens_performed: 0,
            is_healthy:       true,
        }
    }

    /// Enregistre un snapshot sain d'un agent.
    /// À appeler périodiquement quand le système est en état DORMANT.
    pub fn snapshot_agent(&mut self, role: AgentRole, generation: u32) {
        // Mise à jour du snapshot existant ou création
        if let Some(snap) = self.snapshots.iter_mut().find(|s| s.role == role) {
            snap.generation    = generation;
            snap.snapshot_time = 0; // TODO: timestamp réel
            snap.is_valid      = true;
        } else {
            self.snapshots.push(AgentSnapshot::new(role, generation));
        }
        eprintln!("[RegenAgent] Snapshot saved: {:?} gen={}", role, generation);
    }

    /// Régénère un agent depuis son dernier snapshot sain.
    /// Retourne un SwarmEvent confirmant la régénération.
    pub fn regenerate(&mut self, role: AgentRole) -> Option<SwarmEvent> {
        // Vérifier qu'on a un snapshot valide
        let snap = self.snapshots.iter().find(|s| s.role == role && s.is_valid)?;
        let new_gen = snap.generation + 1;

        self.regens_performed += 1;

        eprintln!(
            "[RegenAgent] REGENERATING {:?} | gen {} → {} | regen #{}",
            role, snap.generation, new_gen, self.regens_performed
        );

        // En production : recréer l'agent depuis le snapshot ADN
        // Pour l'instant : signal de confirmation
        Some(SwarmEvent {
            from_role:    AgentRole::Regen,
            threat_level: ThreatLevel::Survival,
            description:  format!(
                "Agent {:?} regenerated (gen {}→{})",
                role, snap.generation, new_gen
            ),
            timestamp_ns: 0,
            confidence:   100,
        })
    }

    /// Vérifie la santé de tous les agents listés et régénère si nécessaire.
    /// `dead_agents` : liste des rôles d'agents qui ne répondent plus.
    pub fn check_and_heal(&mut self, dead_agents: &[AgentRole]) -> Vec<SwarmEvent> {
        let mut events = Vec::new();
        for &role in dead_agents {
            if let Some(ev) = self.regenerate(role) {
                events.push(ev);
            } else {
                eprintln!(
                    "[RegenAgent] WARNING: no valid snapshot for {:?} — cannot regen",
                    role
                );
                // Tenter une régénération depuis zéro (génération 1)
                events.push(SwarmEvent {
                    from_role:    AgentRole::Regen,
                    threat_level: ThreatLevel::Survival,
                    description:  format!(
                        "Agent {:?} cold-regenerated (no snapshot)", role
                    ),
                    timestamp_ns: 0,
                    confidence:   70,
                });
            }
        }
        events
    }

    /// Invalide tous les snapshots (après suspicion de corruption globale).
    pub fn invalidate_all_snapshots(&mut self) {
        for snap in &mut self.snapshots {
            snap.is_valid = false;
        }
        eprintln!("[RegenAgent] All snapshots invalidated — cold regen only");
    }

    pub fn snapshot_count(&self) -> usize {
        self.snapshots.iter().filter(|s| s.is_valid).count()
    }
}

impl Default for RegenAgent {
    fn default() -> Self { Self::new() }
}
