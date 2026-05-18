//! BOT-BAREMETAL — ChameleonAgent
//! Invisibilité du Bot : camouflage actif pour ne pas être ciblé.
//!
//! "L'art suprême de la guerre est de soumettre l'ennemi sans combattre.
//!  L'art suprême de la défense est de rester invisible."
//!
//! Stratégies :
//!   - Fausse carte mémoire visible par les scanners externes
//!   - Noms de processus aléatoires pour les agents Bot
//!   - Migration des ressources critiques vers des zones inattendues
//!   - Fausses signatures de fichiers Bot

use crate::swarm_mind::{AgentRole, SwarmEvent, ThreatLevel};

/// Une couverture active (alias d'un processus Bot)
#[derive(Debug, Clone)]
pub struct BotCover {
    pub original_name: String,
    pub cover_name:    String,    // Nom sous lequel l'agent apparaît
    pub is_active:     bool,
}

/// Région mémoire leurre (fausse empreinte mémoire)
#[derive(Debug, Clone)]
pub struct FakeMemRegion {
    pub addr:       u64,
    pub size:       u64,
    pub fake_label: String,    // Label trompeur pour les scanners
    pub is_active:  bool,
}

pub struct ChameleonAgent {
    pub covers:           Vec<BotCover>,
    pub fake_mem_regions: Vec<FakeMemRegion>,
    pub is_cloaked:       bool,
    pub cloak_activations: u64,
}

impl ChameleonAgent {
    pub fn new() -> Self {
        Self {
            covers:            Vec::new(),
            fake_mem_regions:  Vec::new(),
            is_cloaked:        false,
            cloak_activations: 0,
        }
    }

    /// Active le camouflage complet de la flotte Bot.
    /// Appelé automatiquement en mode SURVIVAL ou CONFINEMENT.
    pub fn activate_cloak(&mut self) -> SwarmEvent {
        self.is_cloaked = true;
        self.cloak_activations += 1;

        // Générer des couvertures pour les agents principaux
        let agent_covers = [
            ("swarm_mind",   "svchost.exe"),
            ("mem_watch",    "MsMpEng.exe"),
            ("net_watch",    "lsass.exe"),
            ("fs_watch",     "SearchIndexer.exe"),
            ("proc_watch",   "WmiPrvSE.exe"),
            ("honey_trap",   "spoolsv.exe"),
            ("regen_agent",  "RuntimeBroker.exe"),
        ];

        self.covers.clear();
        for (orig, cover) in &agent_covers {
            self.covers.push(BotCover {
                original_name: orig.to_string(),
                cover_name:    cover.to_string(),
                is_active:     true,
            });
        }

        eprintln!(
            "[Chameleon] CLOAK ACTIVATED — {} agents covered (activation #{})",
            self.covers.len(), self.cloak_activations
        );

        SwarmEvent {
            from_role:    AgentRole::Chameleon,
            threat_level: ThreatLevel::Survival,
            description:  format!(
                "Camouflage active: {} agents re-labeled", self.covers.len()
            ),
            timestamp_ns: 0,
            confidence:   100,
        }
    }

    /// Désactive le camouflage (retour en état normal post-menace).
    pub fn deactivate_cloak(&mut self) {
        self.is_cloaked = false;
        for cover in &mut self.covers {
            cover.is_active = false;
        }
        eprintln!("[Chameleon] Cloak deactivated — back to normal identity");
    }

    /// Crée de fausses régions mémoire pour tromper les scanners adverses.
    /// L'attaquant voit une fausse empreinte mémoire du Bot.
    pub fn deploy_fake_memory_map(&mut self, base_addr: u64) {
        let decoys = [
            (base_addr,           0x1000, "kernel32.dll"),
            (base_addr + 0x2000,  0x800,  "ntdll.dll"),
            (base_addr + 0x5000,  0x400,  "[stack]"),
        ];

        for (addr, size, label) in &decoys {
            self.fake_mem_regions.push(FakeMemRegion {
                addr:       *addr,
                size:       *size,
                fake_label: label.to_string(),
                is_active:  true,
            });
        }
        eprintln!(
            "[Chameleon] Fake memory map deployed: {} decoy regions",
            decoys.len()
        );
    }

    /// Retourne le nom de couverture d'un agent (ou None si pas cloaké).
    pub fn get_cover_name(&self, original: &str) -> Option<&str> {
        if !self.is_cloaked {
            return None;
        }
        self.covers
            .iter()
            .find(|c| c.is_active && c.original_name == original)
            .map(|c| c.cover_name.as_str())
    }

    pub fn active_covers(&self) -> usize {
        self.covers.iter().filter(|c| c.is_active).count()
    }
}

impl Default for ChameleonAgent {
    fn default() -> Self { Self::new() }
}
