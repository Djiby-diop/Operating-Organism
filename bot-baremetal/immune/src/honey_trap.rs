//! BOT-BAREMETAL — HoneyTrap Agent
//! Maître du leurre : crée de faux environnements pour piéger et étudier l'attaquant.
//!
//! "L'attaquant pénètre dans un mirage.
//!  Chaque mouvement qu'il fait, nous l'apprenons."

use crate::swarm_mind::{AgentRole, SwarmEvent, ThreatLevel};

/// Type de leurre déployé
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum HoneyType {
    FakeCredentialFile,    // Faux fichier credentials
    FakeConfigFile,        // Faux fichier de config avec "secrets"
    FakeNetworkService,    // Faux service réseau (SSH, RDP)
    FakeApiEndpoint,       // Faux endpoint API
    FakeKernelObject,      // Faux objet kernel pour tromper les scanners
    FakeBotProcess,        // Leurre de processus Bot (sacrificiel)
}

/// Un leurre actif dans le système
#[derive(Debug, Clone)]
pub struct HoneyPot {
    pub honey_type:     HoneyType,
    pub location:       String,     // Chemin ou adresse du leurre
    pub is_active:      bool,
    pub touch_count:    u64,        // Nombre de fois que l'attaquant l'a touché
    pub last_touched:   u64,        // Timestamp dernier contact
}

impl HoneyPot {
    pub fn new(honey_type: HoneyType, location: String) -> Self {
        Self {
            honey_type,
            location,
            is_active:    true,
            touch_count:  0,
            last_touched: 0,
        }
    }
}

pub struct HoneyTrapAgent {
    pub pots:        Vec<HoneyPot>,
    pub total_touches: u64,
    pub unique_attackers: u64,
}

impl HoneyTrapAgent {
    pub fn new() -> Self {
        Self {
            pots: Vec::new(),
            total_touches: 0,
            unique_attackers: 0,
        }
    }

    /// Déploie un leurre de type "credentials" — le plus attractif pour un attaquant.
    pub fn deploy_fake_credentials(&mut self, path: &str) -> &HoneyPot {
        let pot = HoneyPot::new(
            HoneyType::FakeCredentialFile,
            format!("{}/credentials.json", path),
        );
        self.pots.push(pot);
        self.pots.last().unwrap()
    }

    /// Déploie un faux processus Bot (sacrificiel) pour attirer les kill attempts.
    pub fn deploy_fake_bot_process(&mut self, fake_pid_name: &str) -> &HoneyPot {
        let pot = HoneyPot::new(
            HoneyType::FakeBotProcess,
            format!("pid_decoy:{}", fake_pid_name),
        );
        self.pots.push(pot);
        self.pots.last().unwrap()
    }

    /// Signale qu'un leurre a été touché.
    /// Retourne un SwarmEvent — le Bot a capturé l'attaquant in flagrante.
    pub fn on_honey_touched(&mut self, location: &str,
                            attacker_pid: u32) -> Option<SwarmEvent> {
        for pot in &mut self.pots {
            if pot.is_active && pot.location == location {
                pot.touch_count  += 1;
                pot.last_touched  = 0; // TODO: timestamp réel
                self.total_touches += 1;

                eprintln!(
                    "[HoneyTrap] TRIGGERED: {:?} at '{}' by pid={} (touch #{})",
                    pot.honey_type, location, attacker_pid, pot.touch_count
                );

                return Some(SwarmEvent {
                    from_role:    AgentRole::HoneyTrap,
                    threat_level: ThreatLevel::Combat,
                    description:  format!(
                        "HoneyPot touched: {:?} at '{}' by pid={}",
                        pot.honey_type, location, attacker_pid
                    ),
                    timestamp_ns: 0,
                    confidence:   98, // Quasi-certitude : seul un attaquant touche un leurre
                });
            }
        }
        None
    }

    /// Désactive tous les leurres (fin de session de combat).
    pub fn deactivate_all(&mut self) {
        for pot in &mut self.pots {
            pot.is_active = false;
        }
        eprintln!("[HoneyTrap] All honey pots deactivated");
    }

    pub fn active_count(&self) -> usize {
        self.pots.iter().filter(|p| p.is_active).count()
    }

    pub fn total_touches(&self) -> u64 {
        self.total_touches
    }
}

impl Default for HoneyTrapAgent {
    fn default() -> Self { Self::new() }
}
