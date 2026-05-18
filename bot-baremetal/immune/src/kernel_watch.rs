//! BOT-BAREMETAL — KernelWatchAgent
//! Gardien du kernel — priorité maximale dans la flotte.
//!
//! "Si le kernel est compromis, tout le reste l'est aussi.
//!  KernelWatch ne dort jamais."
//!
//! Détecte : SSDT hooks, DKOM, rootkits, modifications des structures kernel.

use crate::swarm_mind::{AgentRole, SwarmEvent, ThreatLevel};

/// Type de modification kernel détectée
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum KernelThreatType {
    SsdtHook,             // System Service Descriptor Table hook
    InlineHook,           // Hook inline dans une fonction kernel
    Dkom,                 // Direct Kernel Object Manipulation
    RootkitSignature,     // Signature comportementale de rootkit
    IrpHook,              // I/O Request Packet hook (drivers)
    HiddenDriver,         // Driver caché dans la liste des modules
    BootkitIndicator,     // Indicateur de bootkit (ex: MBR modifié)
}

/// Détection kernel avec sa sévérité
#[derive(Debug, Clone)]
pub struct KernelDetection {
    pub threat_type: KernelThreatType,
    pub address:     u64,          // Adresse kernel concernée
    pub module:      String,       // Module kernel associé
    pub confidence:  u8,
    pub timestamp:   u64,
}

pub struct KernelWatchAgent {
    pub detections:     Vec<KernelDetection>,
    pub total_scans:    u64,
    pub threats_found:  u64,
    /// Table de référence des adresses kernel légitimes
    pub reference_ssdt: Vec<u64>,    // SSDT d'origine (snapshot au boot)
}

impl KernelWatchAgent {
    pub fn new() -> Self {
        Self {
            detections:     Vec::new(),
            total_scans:    0,
            threats_found:  0,
            reference_ssdt: Vec::new(),
        }
    }

    /// Prend un snapshot de la SSDT au démarrage (référence saine).
    /// À appeler AVANT tout chargement de driver tiers.
    pub fn snapshot_ssdt(&mut self, ssdt: Vec<u64>) {
        self.reference_ssdt = ssdt;
        eprintln!(
            "[KernelWatch] SSDT snapshot taken: {} entries",
            self.reference_ssdt.len()
        );
    }

    /// Vérifie la SSDT courante contre le snapshot de référence.
    /// Toute divergence est un SSDT hook.
    pub fn check_ssdt(&mut self, current_ssdt: &[u64]) -> Option<SwarmEvent> {
        self.total_scans += 1;

        if self.reference_ssdt.is_empty() {
            return None; // Pas de baseline → ne peut pas comparer
        }

        let hooked: Vec<usize> = current_ssdt
            .iter()
            .enumerate()
            .filter(|(i, &addr)| {
                self.reference_ssdt.get(*i).map_or(false, |&ref_addr| ref_addr != addr)
            })
            .map(|(i, _)| i)
            .collect();

        if hooked.is_empty() {
            return None;
        }

        let confidence = if hooked.len() > 5 { 95 } else { 85 };

        let det = KernelDetection {
            threat_type: KernelThreatType::SsdtHook,
            address:     current_ssdt.get(hooked[0]).copied().unwrap_or(0),
            module:      "unknown".to_string(),
            confidence,
            timestamp:   0,
        };
        self.detections.push(det);
        self.threats_found += 1;

        eprintln!(
            "[KernelWatch] SSDT HOOK DETECTED: {} entries modified | conf={}%",
            hooked.len(), confidence
        );

        Some(SwarmEvent {
            from_role:    AgentRole::KernelWatch,
            threat_level: ThreatLevel::Combat,
            description:  format!(
                "SSDT hook: {} syscall(s) hijacked", hooked.len()
            ),
            timestamp_ns: 0,
            confidence,
        })
    }

    /// Détecte un driver caché (absent de la liste visible mais présent en mémoire).
    pub fn check_hidden_driver(&mut self, driver_base: u64,
                                driver_size: u64) -> Option<SwarmEvent> {
        self.total_scans += 1;

        // En production : croiser avec PsLoadedModuleList
        // Ici : simulation — tout driver à une adresse non-standard est suspect
        let is_standard_range = (driver_base >= 0xFFFF800000000000)
                              && (driver_base < 0xFFFFF80000000000);

        if !is_standard_range && driver_size > 0 {
            self.threats_found += 1;
            let det = KernelDetection {
                threat_type: KernelThreatType::HiddenDriver,
                address:     driver_base,
                module:      format!("hidden@{:#x}", driver_base),
                confidence:  80,
                timestamp:   0,
            };
            self.detections.push(det);

            eprintln!(
                "[KernelWatch] HIDDEN DRIVER at {:#x} size={:#x}",
                driver_base, driver_size
            );

            return Some(SwarmEvent {
                from_role:    AgentRole::KernelWatch,
                threat_level: ThreatLevel::Combat,
                description:  format!(
                    "Hidden driver detected at {:#x} ({} bytes)",
                    driver_base, driver_size
                ),
                timestamp_ns: 0,
                confidence:   80,
            });
        }
        None
    }

    /// Détecte une manipulation DKOM (objet EPROCESS masqué par unlinking).
    pub fn check_dkom_eprocess(&mut self, visible_pids: &[u32],
                                real_pids: &[u32]) -> Option<SwarmEvent> {
        self.total_scans += 1;

        let hidden: Vec<u32> = real_pids
            .iter()
            .filter(|&&pid| !visible_pids.contains(&pid))
            .copied()
            .collect();

        if hidden.is_empty() {
            return None;
        }

        self.threats_found += 1;
        eprintln!(
            "[KernelWatch] DKOM: {} hidden process(es): {:?}",
            hidden.len(), hidden
        );

        Some(SwarmEvent {
            from_role:    AgentRole::KernelWatch,
            threat_level: ThreatLevel::Combat,
            description:  format!(
                "DKOM detected: {} process(es) hidden from OS list: {:?}",
                hidden.len(), hidden
            ),
            timestamp_ns: 0,
            confidence:   92,
        })
    }
}

impl Default for KernelWatchAgent {
    fn default() -> Self { Self::new() }
}
