//! BOT-BAREMETAL — ProcWatch Agent
//! Surveillance des processus : hollowing, injection, escalade de privilèges.

use crate::swarm_mind::{AgentRole, SwarmEvent, ThreatLevel};

pub struct ProcWatchAgent {
    pub threats_caught:  u64,
    pub procs_monitored: u64,
}

impl ProcWatchAgent {
    pub fn new() -> Self {
        Self { threats_caught: 0, procs_monitored: 0 }
    }

    /// Analyse un couple parent→enfant pour détecter les spawns suspects.
    /// Retourne un SwarmEvent si l'arborescence est anormale.
    pub fn check_spawn(&mut self, parent_name: &str, child_name: &str,
                       child_pid: u32) -> Option<SwarmEvent> {
        self.procs_monitored += 1;

        // Couples parent→enfant historiquement liés à des attaques
        let suspicious_pairs = [
            ("winword.exe",   "cmd.exe"),
            ("winword.exe",   "powershell.exe"),
            ("excel.exe",     "cmd.exe"),
            ("excel.exe",     "wscript.exe"),
            ("outlook.exe",   "powershell.exe"),
            ("explorer.exe",  "regsvr32.exe"),
            ("services.exe",  "cmd.exe"),
            ("svchost.exe",   "powershell.exe"),
            ("msiexec.exe",   "cmd.exe"),
            ("werfault.exe",  "cmd.exe"),
        ];

        let parent_l = parent_name.to_lowercase();
        let child_l  = child_name.to_lowercase();

        for (p, c) in &suspicious_pairs {
            if parent_l.contains(p) && child_l.contains(c) {
                self.threats_caught += 1;
                return Some(SwarmEvent {
                    from_role:    AgentRole::ProcWatch,
                    threat_level: ThreatLevel::Alert,
                    description:  format!(
                        "Suspicious spawn: {} → {} (pid={})",
                        parent_name, child_name, child_pid
                    ),
                    timestamp_ns: 0,
                    confidence:   78,
                });
            }
        }
        None
    }

    /// Détecte une tentative d'escalade de privilèges.
    pub fn check_priv_escalation(&mut self, pid: u32,
                                  from_level: u8,
                                  to_level: u8) -> Option<SwarmEvent> {
        // Montée vers SYSTEM (level 4) depuis un niveau normal (< 3) = suspect
        if from_level < 3 && to_level >= 4 {
            self.threats_caught += 1;
            return Some(SwarmEvent {
                from_role:    AgentRole::ProcWatch,
                threat_level: ThreatLevel::Alert,
                description:  format!(
                    "Privilege escalation: pid={} level {} → {}",
                    pid, from_level, to_level
                ),
                timestamp_ns: 0,
                confidence:   82,
            });
        }
        None
    }

    /// Détecte la présence d'un débogueur non autorisé ciblant le Bot.
    pub fn check_debugger_attach(&mut self, target_pid: u32,
                                  bot_pids: &[u32]) -> Option<SwarmEvent> {
        if bot_pids.contains(&target_pid) {
            self.threats_caught += 1;
            return Some(SwarmEvent {
                from_role:    AgentRole::ProcWatch,
                threat_level: ThreatLevel::Survival,
                description:  format!(
                    "Debugger attached to Bot process pid={}", target_pid
                ),
                timestamp_ns: 0,
                confidence:   95,
            });
        }
        None
    }
}

impl Default for ProcWatchAgent {
    fn default() -> Self { Self::new() }
}
