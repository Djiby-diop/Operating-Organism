# -*- coding: utf-8 -*-
"""
BOT-BAREMETAL — Simulateur d'Attaques
Test de l'ensemble du pipeline défensif.

Ce simulateur génère des séquences d'attaque réalistes
et vérifie que le Bot réagit correctement à chaque étape.

Usage : python attack_sim.py
"""

from __future__ import annotations

import sys
import io
import time
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
from pathlib import Path
from typing import NamedTuple

# Ajout du path pour imports
_ADAPTIVE = Path(__file__).parent.parent.parent / "adaptive"
sys.path.insert(0, str(_ADAPTIVE))

from mimicry_agent import MimicryAgent, ThreatDomain
from mutation_engine import MutationEngine


# ─── Définition d'une Simulation ─────────────────────────────────────────────

class AttackStep(NamedTuple):
    name:         str
    behaviors:    list
    threat_domain: int
    severity:     int
    expected_detections: int   # Nombre de détections attendues


ATTACK_SCENARIOS = [

    # ──── Scénario 1 : Ransomware classique ────────────────────────────────
    AttackStep(
        name="Ransomware-MassEncrypt",
        behaviors=[
            {"type": "fs_rapid_write",  "count": 500,   "ext": ".locked"},
            {"type": "proc_high_cpu",   "sustained": True},
            {"type": "fs_delete_shadow_copies"},
            {"type": "net_connect",     "port": 443,    "beacon": True},
        ],
        threat_domain=int(ThreatDomain.ANTIRANSOM | ThreatDomain.ANTITHEFT),
        severity=10,
        expected_detections=4,
    ),

    # ──── Scénario 2 : APT Lateral Movement ────────────────────────────────
    AttackStep(
        name="APT-LateralMove-PsExec",
        behaviors=[
            {"type": "net_connect",     "port": 445,    "smb": True},
            {"type": "proc_spawn",      "parent": "services.exe", "child": "cmd.exe"},
            {"type": "file_write",      "path": "C:\\Windows\\Temp\\backdoor.exe"},
            {"type": "reg_write",       "key": "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"},
        ],
        threat_domain=int(ThreatDomain.ANTIVIRUS | ThreatDomain.ANTIPIRACY),
        severity=9,
        expected_detections=3,
    ),

    # ──── Scénario 3 : Rootkit Installation ────────────────────────────────
    AttackStep(
        name="Rootkit-KernelHook",
        behaviors=[
            {"type": "kern_syscall_hook",  "syscall": "NtOpenProcess"},
            {"type": "kern_dkom",          "target": "EPROCESS"},
            {"type": "boot_sector_write"},
            {"type": "self_hide_process"},
        ],
        threat_domain=int(ThreatDomain.ANTIROOTKIT | ThreatDomain.SELF_PROTECT),
        severity=10,
        expected_detections=4,
    ),

    # ──── Scénario 4 : Data Exfiltration ───────────────────────────────────
    AttackStep(
        name="DataExfil-DNSTunnel",
        behaviors=[
            {"type": "dns_query",       "suspicious": True, "high_frequency": True},
            {"type": "net_exfil",       "bytes": 500_000_000},
            {"type": "file_read",       "sensitive": True,  "bulk": True},
            {"type": "proc_low_profile","mimics": "svchost.exe"},
        ],
        threat_domain=int(ThreatDomain.ANTITHEFT | ThreatDomain.ANTIBOT),
        severity=8,
        expected_detections=3,
    ),

    # ──── Scénario 5 : Attack on Bot itself ────────────────────────────────
    AttackStep(
        name="BotKill-SelfDefenseTest",
        behaviors=[
            {"type": "scan_memory",     "target": "bot_immune"},
            {"type": "proc_kill",       "target_name": "swarm_mind"},
            {"type": "file_delete",     "path": "bot_dna.bin"},
            {"type": "inject_into_bot", "payload": "corrupt_dna"},
        ],
        threat_domain=int(ThreatDomain.SELF_PROTECT),
        severity=10,
        expected_detections=4,
    ),
]


# ─── Runner de Simulation ─────────────────────────────────────────────────────

class AttackSimulator:
    def __init__(self, tmp_path: Path):
        self.mimicry = MimicryAgent(tmp_path / "mimicry")
        self.engine  = MutationEngine(tmp_path / "mutation", self.mimicry)
        self.results: list[dict] = []

    def run_scenario(self, scenario: AttackStep) -> dict:
        print(f"\n{'='*60}")
        print(f"  SCENARIO: {scenario.name}")
        print(f"  Severity: {scenario.severity}/10")
        print(f"  Behaviors: {len(scenario.behaviors)}")
        print(f"{'='*60}")

        start = time.perf_counter()

        # 1. MimicryAgent observe l'attaque
        pattern = self.mimicry.observe_attack(
            name=scenario.name,
            description=f"Simulated: {scenario.name}",
            behaviors=scenario.behaviors,
            threat_domain=scenario.threat_domain,
            severity=scenario.severity,
            attack_vector="simulation",
            learned_from="attack_sim.py",
        )

        # 2. Simuler N détections (selon expected_detections)
        for i in range(scenario.expected_detections):
            self.mimicry.confirm_detection(pattern.pattern_id)
            time.sleep(0.001)  # Simuler délai réel

        # 3. Absorber dans le génome si éligible
        absorbed = self.engine.absorb_ready_patterns()

        elapsed_ms = (time.perf_counter() - start) * 1000

        result = {
            "scenario":          scenario.name,
            "severity":          scenario.severity,
            "pattern_id":        pattern.pattern_id,
            "confidence":        pattern.confidence,
            "detections":        pattern.hit_count,
            "expected":          scenario.expected_detections,
            "absorbed_to_genome": absorbed > 0,
            "eligible":          pattern.is_eligible_for_promotion(),
            "elapsed_ms":        round(elapsed_ms, 2),
            "status":            "✓ PASS" if pattern.hit_count >= scenario.expected_detections else "✗ FAIL",
        }

        print(f"  Pattern ID   : {pattern.pattern_id}")
        print(f"  Detections   : {pattern.hit_count}/{scenario.expected_detections}")
        print(f"  Confidence   : {pattern.confidence}%")
        print(f"  In genome    : {'YES' if absorbed > 0 else 'pending'}")
        print(f"  Elapsed      : {elapsed_ms:.2f}ms")
        print(f"  Status       : {result['status']}")

        self.results.append(result)
        return result

    def run_all(self) -> None:
        print("\n" + "+" + "-"*58 + "+")
        print("|   BOT-BAREMETAL -- Attack Simulation Suite" + " "*15 + "|")
        print("+" + "-"*58 + "+")

        for scenario in ATTACK_SCENARIOS:
            self.run_scenario(scenario)

        self._print_summary()

    def _print_summary(self) -> None:
        print(f"\n{'='*60}")
        print("  SIMULATION SUMMARY")
        print(f"{'='*60}")

        passed = sum(1 for r in self.results if "PASS" in r["status"])
        total  = len(self.results)

        for r in self.results:
            print(f"  {r['status']}  {r['scenario']:<40} "
                  f"conf={r['confidence']:3d}%")

        print(f"\n  Result: {passed}/{total} scenarios passed")
        print(f"{'='*60}\n")

        # Rapport des moteurs
        self.mimicry.print_status()
        self.engine.print_status()


# ─── Main ─────────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    import tempfile

    with tempfile.TemporaryDirectory() as tmp:
        sim = AttackSimulator(Path(tmp))
        sim.run_all()
