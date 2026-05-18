//! BOT-BAREMETAL — SwarmMind
//! Le Chef d'État : coordinateur de toute la flotte d'agents.
//!
//! Il ne combat pas lui-même.
//! Il observe, coordonne, ordonne, déploie, retire.
//!
//! "Un général ne porte pas l'épée. Il commande ceux qui la portent."

use std::sync::atomic::{AtomicU8, AtomicU64, Ordering};
use std::sync::Arc;
use std::fs::OpenOptions;
use std::io::Write;

/// Niveaux de menace — miroir de threat_levels.h
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum ThreatLevel {
    Dormant     = 0,
    Vigilance   = 1,
    Alert       = 2,
    Combat      = 3,
    Survival    = 4,
    Confinement = 5,
}

impl ThreatLevel {
    pub fn from_u8(v: u8) -> Option<Self> {
        match v {
            0 => Some(Self::Dormant),
            1 => Some(Self::Vigilance),
            2 => Some(Self::Alert),
            3 => Some(Self::Combat),
            4 => Some(Self::Survival),
            5 => Some(Self::Confinement),
            _ => None,
        }
    }

    /// Vérifie si une transition est valide selon la matrice de transition.
    pub fn can_transition_to(self, next: ThreatLevel) -> bool {
        const MATRIX: [[bool; 6]; 6] = [
            // TO: 0      1      2      3      4      5
            [false, true,  false, false, false, false], // FROM 0
            [true,  false, true,  false, false, false], // FROM 1
            [false, true,  false, true,  false, false], // FROM 2
            [true,  false, true,  false, true,  true ], // FROM 3
            [false, false, false, true,  false, true ], // FROM 4
            [false, false, false, false, true,  false], // FROM 5
        ];
        MATRIX[self as usize][next as usize]
    }
}

/// Rôles des agents — miroir de bot_dna.h
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AgentRole {
    MemWatch    = 0x01,
    NetWatch    = 0x02,
    FsWatch     = 0x03,
    ProcWatch   = 0x04,
    KernelWatch = 0x05,
    BootWatch   = 0x06,
    Quarantine  = 0x10,
    HoneyTrap   = 0x11,
    Neutralizer = 0x12,
    Chameleon   = 0x13,
    Mimicry     = 0x20,
    Mutation    = 0x21,
    Regen       = 0x30,
    OoBridge    = 0x31,
    SwarmMind   = 0xFF,
}

/// Statut d'un agent dans la flotte
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AgentStatus {
    Active,
    Standby,
    Deployed,
    Regenerating,
    Dead,
}

/// Fiche d'un agent dans la flotte
#[derive(Debug, Clone)]
pub struct AgentRecord {
    pub role:           AgentRole,
    pub status:         AgentStatus,
    pub generation:     u32,
    pub threats_caught: u64,
    pub false_positives: u64,
    pub health:         u8,     // 0-100
}

impl AgentRecord {
    pub fn new(role: AgentRole) -> Self {
        Self {
            role,
            status: AgentStatus::Standby,
            generation: 1,
            threats_caught: 0,
            false_positives: 0,
            health: 100,
        }
    }

    pub fn is_alive(&self) -> bool {
        self.status != AgentStatus::Dead && self.health > 0
    }
}

/// Événement transmis au SwarmMind par un agent
#[derive(Debug, Clone)]
pub struct SwarmEvent {
    pub from_role:      AgentRole,
    pub threat_level:   ThreatLevel,
    pub description:    String,
    pub timestamp_ns:   u64,
    pub confidence:     u8,   // 0-100
    pub signature:      Option<[u8; 32]>, // Signature ADN de la menace
}

/// Structure d'un antigène appris
#[derive(Debug, Clone)]
pub struct Antigen {
    pub signature: [u8; 32],
    pub frequency: u32,
    pub last_seen: u64,
}

/// Le SwarmMind — coordinateur central de la flotte
pub struct SwarmMind {
    /// Niveau de menace courant (atomique pour lecture lock-free)
    current_level: Arc<AtomicU8>,

    /// Compteurs globaux
    total_events:      AtomicU64,
    total_transitions: AtomicU64,

    /// Flotte d'agents (fixe, pas d'allocation dynamique en runtime)
    agents: Vec<AgentRecord>,

    /// Base de données lymphatique (Antigènes appris)
    antigens: Vec<Antigen>,
}

impl SwarmMind {
    pub fn new() -> Self {
        let agents = vec![
            AgentRecord::new(AgentRole::MemWatch),
            AgentRecord::new(AgentRole::NetWatch),
            AgentRecord::new(AgentRole::FsWatch),
            AgentRecord::new(AgentRole::ProcWatch),
            AgentRecord::new(AgentRole::KernelWatch),
            AgentRecord::new(AgentRole::BootWatch),
            AgentRecord::new(AgentRole::Quarantine),
            AgentRecord::new(AgentRole::HoneyTrap),
            AgentRecord::new(AgentRole::Neutralizer),
            AgentRecord::new(AgentRole::Chameleon),
            AgentRecord::new(AgentRole::Mimicry),
            AgentRecord::new(AgentRole::Regen),
            AgentRecord::new(AgentRole::OoBridge),
        ];

        Self {
            current_level: Arc::new(AtomicU8::new(ThreatLevel::Dormant as u8)),
            total_events: AtomicU64::new(0),
            total_transitions: AtomicU64::new(0),
            agents,
            antigens: Vec::with_capacity(256),
        }
    }

    /// Retourne le niveau de menace actuel (lock-free).
    pub fn current_level(&self) -> ThreatLevel {
        ThreatLevel::from_u8(self.current_level.load(Ordering::Acquire))
            .unwrap_or(ThreatLevel::Dormant)
    }

    /// Traite un événement d'un agent.
    /// Le SwarmMind décide si une transition est nécessaire.
    pub fn process_event(&mut self, event: SwarmEvent) -> Option<ThreatLevel> {
        self.total_events.fetch_add(1, Ordering::Relaxed);

        let current = self.current_level();

        // Décision de transition selon la confiance et le niveau proposé
        let proposed = event.threat_level;

        if proposed == current {
            return None; // Déjà au bon niveau
        }

        // La transition doit être valide ET la confiance suffisante
        if current.can_transition_to(proposed) && event.confidence >= 60 {
            self.current_level.store(proposed as u8, Ordering::Release);
            self.total_transitions.fetch_add(1, Ordering::Relaxed);

            eprintln!(
                "[SwarmMind] TRANSITION {:?} → {:?} | from={:?} | reason={}",
                current, proposed, event.from_role, event.description
            );

            self.on_level_changed(current, proposed);
            return Some(proposed);
        }

        // Apprentissage de l'antigène si une signature est présente
        if let Some(sig) = event.signature {
            self.learn_antigen(sig);
        }

        None
    }

    /// Apprend ou renforce un antigène
    fn learn_antigen(&mut self, signature: [u8; 32]) {
        if let Some(antigen) = self.antigens.iter_mut().find(|a| a.signature == signature) {
            antigen.frequency += 1;
        } else if self.antigens.len() < 256 {
            self.antigens.push(Antigen {
                signature,
                frequency: 1,
                last_seen: 0, // Mock timestamp
            });
            eprintln!("[SwarmMind] 🛡️ Nouvel antigène appris via signature ADN.");
        }
    }

    /// Réagit à un changement de niveau — ordonne les agents.
    fn on_level_changed(&mut self, from: ThreatLevel, to: ThreatLevel) {
        let msg = format!("[SwarmMind] Transition from {:?} to {:?}", from, to);
        self.log_to_oojour(&msg);

        match to {
            ThreatLevel::Dormant => {
                eprintln!("[SwarmMind] → DORMANT: all agents to standby");
                for agent in &mut self.agents {
                    if agent.is_alive() {
                        agent.status = AgentStatus::Standby;
                    }
                }
            }
            ThreatLevel::Vigilance => {
                eprintln!("[SwarmMind] → VIGILANCE: watchers active, responders standby");
                // Activer les watchers
                for agent in &mut self.agents {
                    if matches!(agent.role,
                        AgentRole::MemWatch | AgentRole::NetWatch |
                        AgentRole::FsWatch  | AgentRole::ProcWatch)
                    {
                        agent.status = AgentStatus::Active;
                    }
                }
            }
            ThreatLevel::Alert => {
                eprintln!("[SwarmMind] → ALERT: honeytrap + mimicry deployed");
                for agent in &mut self.agents {
                    if matches!(agent.role, AgentRole::HoneyTrap | AgentRole::Mimicry) {
                        agent.status = AgentStatus::Deployed;
                    }
                }
            }
            ThreatLevel::Combat => {
                eprintln!("[SwarmMind] → COMBAT: full fleet active");
                for agent in &mut self.agents {
                    if agent.is_alive() {
                        agent.status = AgentStatus::Active;
                    }
                }
            }
            ThreatLevel::Survival => {
                eprintln!("[SwarmMind] → SURVIVAL: regen + chameleon priority");
                for agent in &mut self.agents {
                    if matches!(agent.role, AgentRole::Regen | AgentRole::Chameleon) {
                        agent.status = AgentStatus::Active;
                    }
                }
            }
            ThreatLevel::Confinement => {
                eprintln!("[SwarmMind] → CONFINEMENT: lockdown, preserve OO DNA");
                // En confinement : seul l'OoBridge reste actif
                for agent in &mut self.agents {
                    agent.status = if agent.role == AgentRole::OoBridge {
                        AgentStatus::Active
                    } else {
                        AgentStatus::Standby
                    };
                }
            }
        }
    }

    /// Écrit l'événement dans le journal partagé OOJOUR.LOG via l'UEFI SimpleFileSystem
    fn log_to_oojour(&self, message: &str) {
        if let Ok(mut file) = OpenOptions::new().create(true).append(true).open("OOJOUR.LOG") {
            let timestamp = 1715258800; // Mock timestamp
            let json = format!(
                "{{\"oo_type\":\"BOT_EVENT\",\"version\":1,\"timestamp\":{},\"from\":\"bot-baremetal\",\"to\":\"llm-baremetal\",\"payload\":{{\"message\":\"{}\"}}}}\n",
                timestamp, message
            );
            let _ = file.write_all(json.as_bytes());
        }
    }

    /// Vérifie et régénère les agents morts si RegenAgent est sain.
    pub fn check_and_regen(&mut self) {
        let regen_healthy = self.agents.iter()
            .any(|a| a.role == AgentRole::Regen && a.is_alive());

        if !regen_healthy {
            eprintln!("[SwarmMind] WARNING: RegenAgent is dead — cannot regen");
            return;
        }

        let dead_roles: Vec<AgentRole> = self.agents.iter()
            .filter(|a| !a.is_alive())
            .map(|a| a.role)
            .collect();

        for role in dead_roles {
            if let Some(agent) = self.agents.iter_mut().find(|a| a.role == role) {
                agent.health = 100;
                agent.generation += 1;
                agent.status = AgentStatus::Regenerating;
                eprintln!("[SwarmMind] Regenerating {:?} gen={}", role, agent.generation);
            }
        }
    }

    /// Rapport de santé de la flotte.
    pub fn fleet_status(&self) {
        eprintln!("\n=== SwarmMind Fleet Status ===");
        eprintln!("  Threat Level : {:?}", self.current_level());
        eprintln!("  Total Events : {}", self.total_events.load(Ordering::Relaxed));
        eprintln!("  Transitions  : {}", self.total_transitions.load(Ordering::Relaxed));
        eprintln!("  --- Agents ---");
        for agent in &self.agents {
            eprintln!("  {:<15?} | {:<12?} | health={:3} | gen={} | threats={}",
                agent.role, agent.status, agent.health,
                agent.generation, agent.threats_caught);
        }
        eprintln!("==============================\n");
    }
}

impl Default for SwarmMind {
    fn default() -> Self {
        Self::new()
    }
}
