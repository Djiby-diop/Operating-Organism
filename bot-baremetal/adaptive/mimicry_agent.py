"""
BOT-BAREMETAL — MimicryAgent
L'agent d'adaptation : observe les attaques et forge les anticorps.

"L'ennemi est le meilleur professeur."
"""

from __future__ import annotations

import hashlib
import json
import time
from dataclasses import dataclass, field, asdict
from enum import IntEnum
from pathlib import Path


MIN_CONFIDENCE_TO_PROMOTE = 80
MAX_FALSE_POSITIVE_RATE   = 0.05
MAX_PATTERN_AGE_DAYS      = 30


class ThreatDomain(IntEnum):
    ANTIVIRUS    = 1 << 0
    ANTIPIRACY   = 1 << 1
    ANTITHEFT    = 1 << 2
    ANTIROOTKIT  = 1 << 3
    ANTIRANSOM   = 1 << 4
    ANTISOCIAL   = 1 << 5
    ANTIBOT      = 1 << 6
    SELF_PROTECT = 1 << 7


@dataclass
class BehaviorPattern:
    """Anticorps comportemental appris d'une attaque réelle."""
    pattern_id:           str   = ""
    name:                 str   = ""
    description:          str   = ""
    threat_domain:        int   = 0
    severity:             int   = 0
    confidence:           int   = 0
    behaviors:            list  = field(default_factory=list)
    ioc_behaviors:        list  = field(default_factory=list)
    learned_at:           float = field(default_factory=time.time)
    last_hit:             float = 0.0
    hit_count:            int   = 0
    false_positive_count: int   = 0
    is_validated:         bool  = False
    is_active:            bool  = True
    learned_from:         str   = ""
    attack_vector:        str   = "unknown"

    def compute_id(self) -> str:
        content = json.dumps({
            "name": self.name,
            "behaviors": sorted(str(b) for b in self.behaviors),
            "threat_domain": self.threat_domain,
        }, sort_keys=True).encode()
        return hashlib.sha256(content).hexdigest()[:16]

    def false_positive_rate(self) -> float:
        total = self.hit_count + self.false_positive_count
        return 0.0 if total == 0 else self.false_positive_count / total

    def is_eligible_for_promotion(self) -> bool:
        return (
            self.confidence >= MIN_CONFIDENCE_TO_PROMOTE
            and self.false_positive_rate() <= MAX_FALSE_POSITIVE_RATE
            and self.hit_count >= 3
            and self.is_active
            and not self.is_validated
        )

    def to_dict(self) -> dict:
        return asdict(self)

    @classmethod
    def from_dict(cls, d: dict) -> "BehaviorPattern":
        return cls(**d)


class MimicryAgent:
    """
    Observe les attaques, distille des patterns, immunise l'essaim.

    Cycle :
        ATTAQUE OBSERVÉE → PATTERN → VALIDATION → GÉNOME → DIFFUSION
    """

    def __init__(self, storage_path: Path):
        self.storage_path = storage_path
        self.storage_path.mkdir(parents=True, exist_ok=True)
        self.patterns_file = storage_path / "patterns.json"
        self.patterns: dict[str, BehaviorPattern] = {}
        self._load_patterns()
        print(f"[MimicryAgent] Ready — {len(self.patterns)} patterns loaded")

    def _load_patterns(self) -> None:
        if not self.patterns_file.exists():
            return
        with open(self.patterns_file, "r", encoding="utf-8") as f:
            raw = json.load(f)
        for pid, pdata in raw.items():
            self.patterns[pid] = BehaviorPattern.from_dict(pdata)

    def _save_patterns(self) -> None:
        data = {pid: p.to_dict() for pid, p in self.patterns.items()}
        with open(self.patterns_file, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2)

    def observe_attack(self, name: str, description: str, behaviors: list,
                       threat_domain: int, severity: int,
                       attack_vector: str = "unknown",
                       learned_from: str = "") -> BehaviorPattern:
        """Forge un nouvel anticorps depuis une attaque observée."""
        pattern = BehaviorPattern(
            name=name, description=description, behaviors=behaviors,
            threat_domain=threat_domain, severity=severity,
            attack_vector=attack_vector, learned_from=learned_from,
            confidence=50,
        )
        pattern.pattern_id = pattern.compute_id()

        if pattern.pattern_id in self.patterns:
            existing = self.patterns[pattern.pattern_id]
            existing.hit_count += 1
            existing.confidence = min(100, existing.confidence + 5)
            existing.last_hit = time.time()
            self._save_patterns()
            print(f"[MimicryAgent] Reinforced existing: '{name}'")
            return existing

        self.patterns[pattern.pattern_id] = pattern
        self._save_patterns()
        print(f"[MimicryAgent] New anticorps forged: '{name}' id={pattern.pattern_id}")
        return pattern

    def confirm_detection(self, pattern_id: str) -> None:
        """Confirme une vraie détection — augmente la confiance."""
        if pattern_id not in self.patterns:
            return
        p = self.patterns[pattern_id]
        p.hit_count += 1
        p.last_hit = time.time()
        p.confidence = min(100, p.confidence + 5)
        self._save_patterns()

    def report_false_positive(self, pattern_id: str) -> None:
        """Signale un faux positif — diminue la confiance."""
        if pattern_id not in self.patterns:
            return
        p = self.patterns[pattern_id]
        p.false_positive_count += 1
        p.confidence = max(0, p.confidence - 10)
        if p.false_positive_rate() > MAX_FALSE_POSITIVE_RATE * 2:
            p.is_active = False
            print(f"[MimicryAgent] Pattern '{p.name}' DEACTIVATED — too many FP")
        self._save_patterns()

    def get_patterns_ready_for_promotion(self) -> list[BehaviorPattern]:
        """Patterns prêts pour intégration au génome de l'essaim."""
        return [p for p in self.patterns.values() if p.is_eligible_for_promotion()]

    def promote_pattern(self, pattern_id: str) -> bool:
        """Valide l'intégration d'un pattern dans le génome."""
        if pattern_id not in self.patterns:
            return False
        self.patterns[pattern_id].is_validated = True
        self._save_patterns()
        print(f"[MimicryAgent] '{self.patterns[pattern_id].name}' → GENOME ✓")
        return True

    def cleanup_stale_patterns(self) -> int:
        """Supprime les patterns trop vieux non confirmés."""
        now = time.time()
        max_age = MAX_PATTERN_AGE_DAYS * 24 * 3600
        stale = [
            pid for pid, p in self.patterns.items()
            if not p.is_validated and not p.is_active
            and (now - p.learned_at) > max_age
        ]
        for pid in stale:
            del self.patterns[pid]
        if stale:
            self._save_patterns()
        return len(stale)

    def status_report(self) -> dict:
        total     = len(self.patterns)
        validated = sum(1 for p in self.patterns.values() if p.is_validated)
        active    = sum(1 for p in self.patterns.values() if p.is_active)
        ready     = len(self.get_patterns_ready_for_promotion())
        return {
            "total_patterns":      total,
            "validated_in_genome": validated,
            "active_testing":      active,
            "ready_for_promotion": ready,
        }

    def print_status(self) -> None:
        r = self.status_report()
        print("\n=== MimicryAgent Status ===")
        for k, v in r.items():
            print(f"  {k:30s}: {v}")
        print("===========================\n")


if __name__ == "__main__":
    import tempfile
    with tempfile.TemporaryDirectory() as tmp:
        agent = MimicryAgent(Path(tmp))
        p = agent.observe_attack(
            name="ProcessHollowing-OfficeToCmd",
            description="Process hollowing via Office macro",
            behaviors=[
                {"type": "proc_spawn", "parent": "winword.exe", "child": "cmd.exe"},
                {"type": "mem_write_exec", "region": "target_proc"},
            ],
            threat_domain=int(ThreatDomain.ANTIVIRUS),
            severity=8,
        )
        for _ in range(4):
            agent.confirm_detection(p.pattern_id)

        print(agent.status_report())
        print("Ready:", [r.name for r in agent.get_patterns_ready_for_promotion()])
