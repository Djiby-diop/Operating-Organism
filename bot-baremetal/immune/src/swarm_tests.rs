//! BOT-BAREMETAL — Swarm Integration Test
//! Test complet de toute la flotte Rust ensemble.

#[cfg(test)]
mod swarm_tests {
    use crate::{
        swarm_mind::{SwarmMind, SwarmEvent, ThreatLevel, AgentRole},
        mem_watch::MemWatchAgent,
        fs_watch::FsWatchAgent,
        net_watch::NetWatchAgent,
        proc_watch::ProcWatchAgent,
        quarantine::QuarantineAgent,
        honey_trap::HoneyTrapAgent,
        regen::RegenAgent,
        chameleon::ChameleonAgent,
        neutralizer::NeutralizerAgent,
        kernel_watch::KernelWatchAgent,
        boot_watch::{BootWatchAgent, BootComponent},
    };

    // ── SwarmMind ────────────────────────────────────────────────────────────

    #[test]
    fn test_swarm_starts_dormant() {
        let swarm = SwarmMind::new();
        assert_eq!(swarm.current_level(), ThreatLevel::Dormant);
    }

    #[test]
    fn test_threat_escalation_vigilance() {
        let mut swarm = SwarmMind::new();
        let ev = SwarmEvent {
            from_role:    AgentRole::MemWatch,
            threat_level: ThreatLevel::Vigilance,
            description:  "Unusual memory pattern".into(),
            timestamp_ns: 0,
            confidence:   70,
        };
        let result = swarm.process_event(ev);
        assert_eq!(result, Some(ThreatLevel::Vigilance));
        assert_eq!(swarm.current_level(), ThreatLevel::Vigilance);
    }

    #[test]
    fn test_threat_escalation_full_sequence() {
        let mut swarm = SwarmMind::new();

        // DORMANT → VIGILANCE
        swarm.process_event(SwarmEvent {
            from_role: AgentRole::MemWatch, threat_level: ThreatLevel::Vigilance,
            description: "s1".into(), timestamp_ns: 0, confidence: 70,
        });
        assert_eq!(swarm.current_level(), ThreatLevel::Vigilance);

        // VIGILANCE → ALERT
        swarm.process_event(SwarmEvent {
            from_role: AgentRole::FsWatch, threat_level: ThreatLevel::Alert,
            description: "s2".into(), timestamp_ns: 0, confidence: 80,
        });
        assert_eq!(swarm.current_level(), ThreatLevel::Alert);

        // ALERT → COMBAT
        swarm.process_event(SwarmEvent {
            from_role: AgentRole::ProcWatch, threat_level: ThreatLevel::Combat,
            description: "s3".into(), timestamp_ns: 0, confidence: 91,
        });
        assert_eq!(swarm.current_level(), ThreatLevel::Combat);
    }

    #[test]
    fn test_invalid_transition_blocked() {
        let mut swarm = SwarmMind::new();
        // DORMANT → COMBAT direct est interdit (matrice de transition)
        let ev = SwarmEvent {
            from_role:    AgentRole::KernelWatch,
            threat_level: ThreatLevel::Combat,
            description:  "direct jump".into(),
            timestamp_ns: 0,
            confidence:   99,
        };
        let result = swarm.process_event(ev);
        // La transition doit être bloquée
        assert_eq!(result, None);
        assert_eq!(swarm.current_level(), ThreatLevel::Dormant);
    }

    #[test]
    fn test_low_confidence_blocked() {
        let mut swarm = SwarmMind::new();
        let ev = SwarmEvent {
            from_role:    AgentRole::MemWatch,
            threat_level: ThreatLevel::Vigilance,
            description:  "low confidence".into(),
            timestamp_ns: 0,
            confidence:   30,   // < 60% → bloqué
        };
        let result = swarm.process_event(ev);
        assert_eq!(result, None);
        assert_eq!(swarm.current_level(), ThreatLevel::Dormant);
    }

    // ── MemWatch ─────────────────────────────────────────────────────────────

    #[test]
    fn test_memwatch_nop_sled() {
        let mut agent = MemWatchAgent::new();
        let region: Vec<u8> = vec![0x90u8; 32];   // NOP sled
        let event = agent.scan_region(&region, 0xDEADBEEF);
        assert!(event.is_some());
        let ev = event.unwrap();
        assert_eq!(ev.from_role, AgentRole::MemWatch);
        assert_eq!(ev.threat_level, ThreatLevel::Alert);
    }

    #[test]
    fn test_memwatch_mz_header() {
        let mut agent = MemWatchAgent::new();
        let region: Vec<u8> = vec![0x4D, 0x5A, 0x00, 0x00];  // MZ header
        let event = agent.scan_region(&region, 0x1000);
        assert!(event.is_some());
    }

    #[test]
    fn test_memwatch_clean_region() {
        let mut agent = MemWatchAgent::new();
        let region: Vec<u8> = vec![0x48, 0x89, 0xE5, 0x48];  // Normal code
        let event = agent.scan_region(&region, 0x1000);
        assert!(event.is_none());
    }

    // ── FsWatch ──────────────────────────────────────────────────────────────

    #[test]
    fn test_fswatch_ransomware_extension() {
        let mut agent = FsWatchAgent::new();
        let event = agent.on_file_modified("C:\\Users\\doc.locked", ".locked");
        assert!(event.is_some());
        assert_eq!(event.unwrap().threat_level, ThreatLevel::Combat);
    }

    #[test]
    fn test_fswatch_mass_modification() {
        let mut agent = FsWatchAgent::new();
        // Simuler 60 modifications (> seuil de 50)
        let mut last_event = None;
        for i in 0..60 {
            last_event = agent.on_file_modified(
                &format!("C:\\file{}.txt", i), ".txt"
            );
        }
        assert!(last_event.is_some());
    }

    // ── NetWatch ─────────────────────────────────────────────────────────────

    #[test]
    fn test_netwatch_c2_beacon() {
        let mut agent = NetWatchAgent::new();
        let mut event = None;
        for _ in 0..12 {    // > seuil de 10
            event = agent.on_connection("192.168.1.100", 4444, 100);
        }
        assert!(event.is_some());
        let ev = event.unwrap();
        assert_eq!(ev.from_role, AgentRole::NetWatch);
    }

    #[test]
    fn test_netwatch_exfil() {
        let mut agent = NetWatchAgent::new();
        // 60 MB de données sortantes (> seuil de 50 MB)
        let event = agent.on_connection("10.0.0.1", 443, 60 * 1024 * 1024);
        assert!(event.is_some());
        assert_eq!(event.unwrap().threat_level, ThreatLevel::Combat);
    }

    // ── ProcWatch ─────────────────────────────────────────────────────────────

    #[test]
    fn test_procwatch_office_cmd_spawn() {
        let mut agent = ProcWatchAgent::new();
        let event = agent.check_spawn("WINWORD.EXE", "cmd.exe", 1234);
        assert!(event.is_some());
        assert_eq!(event.unwrap().threat_level, ThreatLevel::Alert);
    }

    #[test]
    fn test_procwatch_normal_spawn() {
        let mut agent = ProcWatchAgent::new();
        let event = agent.check_spawn("explorer.exe", "notepad.exe", 5678);
        assert!(event.is_none());
    }

    #[test]
    fn test_procwatch_priv_escalation() {
        let mut agent = ProcWatchAgent::new();
        let event = agent.check_priv_escalation(999, 1, 4);  // user → SYSTEM
        assert!(event.is_some());
    }

    // ── Quarantine ────────────────────────────────────────────────────────────

    #[test]
    fn test_quarantine_process() {
        let mut agent = QuarantineAgent::new();
        let ev = agent.quarantine_process(4321, "shellcode injection");
        assert_eq!(ev.from_role, AgentRole::Quarantine);
        assert_eq!(agent.active_count(), 1);
    }

    #[test]
    fn test_quarantine_release() {
        let mut agent = QuarantineAgent::new();
        agent.quarantine_process(100, "suspicious");
        assert_eq!(agent.active_count(), 1);
        agent.release("pid:100");
        assert_eq!(agent.active_count(), 0);
    }

    // ── HoneyTrap ─────────────────────────────────────────────────────────────

    #[test]
    fn test_honeytrap_deploy_and_trigger() {
        let mut agent = HoneyTrapAgent::new();
        agent.deploy_fake_credentials("/tmp/honey");
        assert_eq!(agent.active_count(), 1);

        let event = agent.on_honey_touched("/tmp/honey/credentials.json", 9999);
        assert!(event.is_some());
        let ev = event.unwrap();
        assert_eq!(ev.confidence, 98);   // Quasi-certitude
        assert_eq!(ev.threat_level, ThreatLevel::Combat);
    }

    // ── RegenAgent ────────────────────────────────────────────────────────────

    #[test]
    fn test_regen_with_snapshot() {
        let mut agent = RegenAgent::new();
        agent.snapshot_agent(AgentRole::MemWatch, 1);
        assert_eq!(agent.snapshot_count(), 1);

        let ev = agent.regenerate(AgentRole::MemWatch);
        assert!(ev.is_some());
        assert_eq!(ev.unwrap().from_role, AgentRole::Regen);
    }

    #[test]
    fn test_regen_without_snapshot() {
        let mut agent = RegenAgent::new();
        // Pas de snapshot → retourne None
        let ev = agent.regenerate(AgentRole::NetWatch);
        assert!(ev.is_none());
    }

    // ── ChameleonAgent ────────────────────────────────────────────────────────

    #[test]
    fn test_chameleon_cloak_activate() {
        let mut agent = ChameleonAgent::new();
        assert!(!agent.is_cloaked);
        agent.activate_cloak();
        assert!(agent.is_cloaked);
        assert!(agent.active_covers() > 0);
    }

    #[test]
    fn test_chameleon_get_cover() {
        let mut agent = ChameleonAgent::new();
        agent.activate_cloak();
        let cover = agent.get_cover_name("swarm_mind");
        assert!(cover.is_some());
    }

    #[test]
    fn test_chameleon_deactivate() {
        let mut agent = ChameleonAgent::new();
        agent.activate_cloak();
        agent.deactivate_cloak();
        assert!(!agent.is_cloaked);
    }

    // ── NeutralizerAgent ──────────────────────────────────────────────────────

    #[test]
    fn test_neutralizer_high_confidence_kills() {
        let mut agent = NeutralizerAgent::new();
        let ev = agent.terminate_process(666, "confirmed malware", 95);
        assert!(ev.is_some());
        assert_eq!(agent.actions_count(), 1);
    }

    #[test]
    fn test_neutralizer_low_confidence_blocked() {
        let mut agent = NeutralizerAgent::new();
        let ev = agent.terminate_process(777, "maybe malware", 50);
        assert!(ev.is_none());   // Bloqué : conf < 90%
        assert_eq!(agent.actions_count(), 0);
    }

    #[test]
    fn test_neutralizer_refuses_non_quarantined_file() {
        let mut agent = NeutralizerAgent::new();
        // Fichier non en quarantaine → refus
        let ev = agent.delete_quarantined_file("C:\\Windows\\system32\\ntdll.dll", 99);
        assert!(ev.is_none());
    }

    #[test]
    fn test_neutralizer_deletes_quarantined_file() {
        let mut agent = NeutralizerAgent::new();
        let ev = agent.delete_quarantined_file("QUAR_malware.exe", 95);
        assert!(ev.is_some());
    }

    // ── KernelWatch ───────────────────────────────────────────────────────────

    #[test]
    fn test_kernelwatch_ssdt_hook() {
        let mut agent = KernelWatchAgent::new();
        let reference = vec![0x1000u64, 0x2000, 0x3000, 0x4000];
        agent.snapshot_ssdt(reference.clone());

        // Modifier une entrée (hook simulé)
        let mut tampered = reference.clone();
        tampered[1] = 0xDEAD_BEEF_u64;

        let ev = agent.check_ssdt(&tampered);
        assert!(ev.is_some());
        assert_eq!(ev.unwrap().threat_level, ThreatLevel::Combat);
    }

    #[test]
    fn test_kernelwatch_clean_ssdt() {
        let mut agent = KernelWatchAgent::new();
        let ssdt = vec![0x1000u64, 0x2000, 0x3000];
        agent.snapshot_ssdt(ssdt.clone());
        let ev = agent.check_ssdt(&ssdt);
        assert!(ev.is_none());
    }

    #[test]
    fn test_kernelwatch_dkom() {
        let mut agent = KernelWatchAgent::new();
        let visible = vec![100u32, 200, 300];
        let real    = vec![100u32, 200, 300, 999];  // pid 999 caché
        let ev = agent.check_dkom_eprocess(&visible, &real);
        assert!(ev.is_some());
    }

    // ── BootWatch ─────────────────────────────────────────────────────────────

    #[test]
    fn test_bootwatch_oo_defaults() {
        let mut agent = BootWatchAgent::new();
        agent.watch_oo_defaults();
        assert!(agent.watched_count() > 0);
    }

    #[test]
    fn test_bootwatch_integrity_ok() {
        let mut agent = BootWatchAgent::new();
        let hash = [0xABu8; 32];
        agent.watch(
            BootComponent::OoKernel,
            "\\EFI\\BOOT\\KERNEL.EFI",
            hash
        );
        let ev = agent.verify_file("\\EFI\\BOOT\\KERNEL.EFI", hash);
        assert!(ev.is_none());     // Hash identique → intact
        assert!(agent.all_intact());
    }

    #[test]
    fn test_bootwatch_integrity_violation() {
        let mut agent = BootWatchAgent::new();
        let expected = [0xABu8; 32];
        let tampered = [0xFFu8; 32];
        agent.watch(
            BootComponent::OoKernel,
            "\\EFI\\BOOT\\KERNEL.EFI",
            expected
        );
        let ev = agent.verify_file("\\EFI\\BOOT\\KERNEL.EFI", tampered);
        assert!(ev.is_some());
        // OO Kernel modifié → CONFINEMENT
        assert_eq!(ev.unwrap().threat_level, ThreatLevel::Confinement);
        assert!(!agent.all_intact());
    }
}
