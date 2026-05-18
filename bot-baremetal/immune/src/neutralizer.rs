//! BOT-BAREMETAL — NeutralizerAgent
//! Neutralisation confirmée. Frappe uniquement avec preuve absolue.
//!
//! "La précision est la vertu du prédateur patient.
//!  Le Neutralizer ne frappe jamais à l'aveugle."
//!
//! Règle d'or : JAMAIS le premier à agir.
//! Sequence obligatoire : Quarantine → Analyse → Confirmation → Neutralisation.

use crate::swarm_mind::{AgentRole, SwarmEvent, ThreatLevel};

/// Action de neutralisation effectuée
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NeutralizeAction {
    TerminateProcess,
    DeleteFile,
    CleanRegistry,
    RevokeNetworkAccess,
    FlushMaliciousMemory,
}

/// Résultat d'une neutralisation
#[derive(Debug, Clone)]
pub struct NeutralizeResult {
    pub action:      NeutralizeAction,
    pub target:      String,
    pub success:     bool,
    pub confidence:  u8,
    pub timestamp:   u64,
    pub audit_note:  String,    // Toujours journalisé pour auditabilité
}

pub struct NeutralizerAgent {
    pub actions_taken:   Vec<NeutralizeResult>,
    pub total_kills:     u64,
    /// Confiance minimum requise pour agir (défaut: 90%)
    pub min_confidence:  u8,
}

impl NeutralizerAgent {
    pub fn new() -> Self {
        Self {
            actions_taken:  Vec::new(),
            total_kills:    0,
            min_confidence: 90,
        }
    }

    /// Vérifie si le Neutralizer est autorisé à agir.
    /// Refuse si la confiance est insuffisante.
    fn can_act(&self, confidence: u8) -> bool {
        if confidence < self.min_confidence {
            eprintln!(
                "[Neutralizer] BLOCKED: confidence={}% < required={}%",
                confidence, self.min_confidence
            );
            false
        } else {
            true
        }
    }

    /// Termine un processus malveillant confirmé.
    pub fn terminate_process(&mut self, pid: u32,
                              reason: &str,
                              confidence: u8) -> Option<SwarmEvent> {
        if !self.can_act(confidence) {
            return None;
        }

        let result = NeutralizeResult {
            action:     NeutralizeAction::TerminateProcess,
            target:     format!("pid:{}", pid),
            success:    true,   // En production : vrai résultat de l'OS
            confidence,
            timestamp:  0,
            audit_note: format!("Reason: {} | conf={}%", reason, confidence),
        };

        eprintln!(
            "[Neutralizer] TERMINATE pid={} | conf={}% | reason={}",
            pid, confidence, reason
        );

        self.actions_taken.push(result);
        self.total_kills += 1;

        Some(SwarmEvent {
            from_role:    AgentRole::Neutralizer,
            threat_level: ThreatLevel::Combat,
            description:  format!(
                "Process pid={} terminated: {} (conf={}%)",
                pid, reason, confidence
            ),
            timestamp_ns: 0,
            confidence,
        })
    }

    /// Supprime un fichier malveillant isolé.
    /// UNIQUEMENT sur des fichiers préalablement mis en quarantaine.
    pub fn delete_quarantined_file(&mut self, path: &str,
                                    confidence: u8) -> Option<SwarmEvent> {
        if !self.can_act(confidence) {
            return None;
        }

        // Vérification : le fichier doit être en quarantaine (nom suffixé)
        if !path.contains(".quarantine") && !path.contains("QUAR_") {
            eprintln!(
                "[Neutralizer] REFUSED: '{}' is not in quarantine zone",
                path
            );
            return None;
        }

        let result = NeutralizeResult {
            action:     NeutralizeAction::DeleteFile,
            target:     path.to_string(),
            success:    true,
            confidence,
            timestamp:  0,
            audit_note: format!("Quarantine delete | conf={}%", confidence),
        };

        eprintln!(
            "[Neutralizer] DELETE file='{}' | conf={}%",
            path, confidence
        );

        self.actions_taken.push(result);
        self.total_kills += 1;

        Some(SwarmEvent {
            from_role:    AgentRole::Neutralizer,
            threat_level: ThreatLevel::Combat,
            description:  format!("File deleted from quarantine: '{}'", path),
            timestamp_ns: 0,
            confidence,
        })
    }

    /// Génère un rapport d'audit complet de toutes les actions.
    /// Requis par la prime_directive 6 : JOURNALISER_TOUTE_ACTION.
    pub fn audit_report(&self) -> String {
        let mut report = String::from("=== NeutralizerAgent Audit Report ===\n");
        report.push_str(&format!(
            "Total actions: {} | Min confidence threshold: {}%\n",
            self.actions_taken.len(), self.min_confidence
        ));
        for (i, r) in self.actions_taken.iter().enumerate() {
            report.push_str(&format!(
                "  [{:03}] {:?} | target='{}' | conf={}% | success={} | {}\n",
                i + 1, r.action, r.target, r.confidence, r.success, r.audit_note
            ));
        }
        report.push_str("=====================================\n");
        report
    }

    pub fn actions_count(&self) -> usize {
        self.actions_taken.len()
    }
}

impl Default for NeutralizerAgent {
    fn default() -> Self { Self::new() }
}
