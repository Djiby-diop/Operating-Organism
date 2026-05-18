//! BOT-BAREMETAL — BootWatchAgent
//! Gardien du boot UEFI — protège la racine de confiance du système.
//!
//! "Si le boot est compromis, tout ce qui tourne dessus est compromis.
//!  BootWatch est la première et dernière ligne de défense."
//!
//! Domaine : natif UEFI/bare-metal.
//! Compatible avec le Soma de l'OO (couche C / UEFI).

use crate::swarm_mind::{AgentRole, SwarmEvent, ThreatLevel};

/// Composants de boot surveillés
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BootComponent {
    MbrVbr,           // Master/Volume Boot Record
    EfiApplication,   // Application EFI (.efi)
    EfiVariable,      // Variable UEFI (NVRAM)
    Bootloader,       // Bootloader (GRUB, Windows Boot Manager)
    OoKernel,         // Le kernel OO lui-même (KERNEL.EFI)
    OoManifest,       // ORGANISM_MANIFEST.md, fichiers OO critiques
}

/// Entrée dans la liste des fichiers de boot surveillés
#[derive(Debug, Clone)]
pub struct BootFileEntry {
    pub component:      BootComponent,
    pub path:           String,
    pub expected_hash:  [u8; 32],    // SHA256 à la création
    pub current_hash:   [u8; 32],    // SHA256 actuel
    pub is_intact:      bool,
    pub last_checked:   u64,
}

impl BootFileEntry {
    pub fn new(component: BootComponent, path: &str, hash: [u8; 32]) -> Self {
        Self {
            component,
            path: path.to_string(),
            expected_hash: hash,
            current_hash: hash,    // Initialement identique
            is_intact: true,
            last_checked: 0,
        }
    }

    pub fn check_integrity(&self) -> bool {
        self.expected_hash == self.current_hash
    }
}

pub struct BootWatchAgent {
    pub watched:         Vec<BootFileEntry>,
    pub violations:      Vec<String>,
    pub total_checks:    u64,
    pub violations_count: u64,
}

impl BootWatchAgent {
    pub fn new() -> Self {
        Self {
            watched:          Vec::new(),
            violations:       Vec::new(),
            total_checks:     0,
            violations_count: 0,
        }
    }

    /// Enregistre un fichier de boot à surveiller.
    pub fn watch(&mut self, component: BootComponent,
                 path: &str, expected_hash: [u8; 32]) {
        self.watched.push(BootFileEntry::new(component, path, expected_hash));
        eprintln!("[BootWatch] Now watching: {:?} at '{}'", component, path);
    }

    /// Enregistre les fichiers OO par défaut (KERNEL.EFI, etc.).
    pub fn watch_oo_defaults(&mut self) {
        // Hash fictifs (en production : calculés au premier boot sain)
        let default_hash = [0xABu8; 32];

        let defaults = [
            (BootComponent::OoKernel,   "\\EFI\\BOOT\\KERNEL.EFI"),
            (BootComponent::OoManifest, "\\ORGANISM_MANIFEST.md"),
            (BootComponent::EfiApplication, "\\EFI\\BOOT\\BOOTX64.EFI"),
            (BootComponent::MbrVbr,     "\\MBR"),
        ];

        for (comp, path) in &defaults {
            self.watch(*comp, path, default_hash);
        }
        eprintln!(
            "[BootWatch] OO defaults registered: {} files",
            defaults.len()
        );
    }

    /// Simule une vérification d'intégrité.
    /// En production : lit le hash SHA256 réel du fichier.
    pub fn verify_file(&mut self, path: &str,
                        actual_hash: [u8; 32]) -> Option<SwarmEvent> {
        self.total_checks += 1;

        let entry = self.watched.iter_mut().find(|e| e.path == path)?;
        entry.current_hash = actual_hash;
        entry.last_checked = 0; // TODO: timestamp réel
        entry.is_intact = entry.check_integrity();

        if !entry.is_intact {
            self.violations_count += 1;
            let violation = format!(
                "Boot file modified: {:?} at '{}'", entry.component, path
            );
            self.violations.push(violation.clone());

            eprintln!("[BootWatch] INTEGRITY VIOLATION: {}", violation);

            // Sévérité maximale pour OO Kernel
            let level = if matches!(entry.component,
                BootComponent::OoKernel | BootComponent::OoManifest)
            {
                ThreatLevel::Confinement
            } else {
                ThreatLevel::Combat
            };

            return Some(SwarmEvent {
                from_role:    AgentRole::BootWatch,
                threat_level: level,
                description:  violation,
                timestamp_ns: 0,
                confidence:   99,   // Hash = preuve absolue
            });
        }

        None
    }

    /// Vérifie tous les fichiers enregistrés (batch scan).
    /// En production : appelé à chaque boot et à intervalle régulier.
    pub fn verify_all_with_mock(&mut self) -> Vec<SwarmEvent> {
        let paths: Vec<String> = self.watched.iter().map(|e| e.path.clone()).collect();
        let hashes: Vec<[u8; 32]> = self.watched
            .iter()
            .map(|e| e.expected_hash)  // Mock : hash "correct" pour test
            .collect();

        let mut events = Vec::new();
        for (path, hash) in paths.iter().zip(hashes.iter()) {
            if let Some(ev) = self.verify_file(path, *hash) {
                events.push(ev);
            }
        }

        eprintln!(
            "[BootWatch] Batch scan: {}/{} files intact",
            self.watched.iter().filter(|e| e.is_intact).count(),
            self.watched.len()
        );
        events
    }

    pub fn all_intact(&self) -> bool {
        self.watched.iter().all(|e| e.is_intact)
    }

    pub fn watched_count(&self) -> usize {
        self.watched.len()
    }
}

impl Default for BootWatchAgent {
    fn default() -> Self { Self::new() }
}
