"""
BOT-BAREMETAL — PatternStore
Stockage centralisé et indexé des patterns comportementaux.

Rôle : source de vérité unique pour tous les patterns.
       MimicryAgent écrit, MutationEngine lit et enrichit,
       les agents C/Rust consomment via export JSON.

Index :
  - par ID          : accès O(1)
  - par domaine     : filtrage rapide
  - par sévérité    : tri par priorité
  - par statut      : actif / validé / archivé
"""

from __future__ import annotations

import json
import time
from dataclasses import dataclass, field, asdict
from enum import IntEnum
from pathlib import Path
from typing import Iterator, Optional


# ─── Statuts d'un pattern ────────────────────────────────────────────────────

class PatternStatus(IntEnum):
    TESTING   = 0   # En période de test (nouveau)
    VALIDATED = 1   # Intégré au génome
    MUTATING  = 2   # En cours de mutation
    ARCHIVED  = 3   # Désactivé, conservé pour historique
    FUSED     = 4   # Fusionné dans un pattern hybride


# ─── Entrée dans le store ─────────────────────────────────────────────────────

@dataclass
class StoreEntry:
    """Enveloppe d'un pattern dans le store central."""
    pattern_id:      str
    name:            str
    threat_domain:   int
    severity:        int
    confidence:      int
    behaviors:       list         = field(default_factory=list)
    status:          int          = PatternStatus.TESTING
    generation:      int          = 1
    fitness_score:   float        = 0.0
    hit_count:       int          = 0
    false_pos_count: int          = 0
    created_at:      float        = field(default_factory=time.time)
    updated_at:      float        = field(default_factory=time.time)
    tags:            list         = field(default_factory=list)
    parent_ids:      list         = field(default_factory=list)   # Pour les fusions
    child_id:        str          = ""                            # Si fusionné dans

    def to_dict(self) -> dict:
        return asdict(self)

    @classmethod
    def from_dict(cls, d: dict) -> "StoreEntry":
        return cls(**d)

    @property
    def is_active(self) -> bool:
        return self.status in (PatternStatus.TESTING, PatternStatus.VALIDATED,
                               PatternStatus.MUTATING)

    @property
    def false_positive_rate(self) -> float:
        total = self.hit_count + self.false_pos_count
        return 0.0 if total == 0 else self.false_pos_count / total


# ─── PatternStore ─────────────────────────────────────────────────────────────

class PatternStore:
    """
    Stockage centralisé et indexé de tous les patterns comportementaux.

    Un seul PatternStore par instance OO.
    Tous les agents (MimicryAgent, MutationEngine) passent par lui.
    """

    STORE_VERSION = 1

    def __init__(self, store_path: Path):
        self.store_path = store_path
        self.store_path.mkdir(parents=True, exist_ok=True)
        self.db_file = store_path / "pattern_store.json"

        # Index principal
        self._entries: dict[str, StoreEntry] = {}

        # Index secondaires (reconstruits depuis _entries)
        self._by_domain:   dict[int, list[str]] = {}   # domain_bit → [ids]
        self._by_status:   dict[int, list[str]] = {}   # status → [ids]
        self._by_severity: dict[int, list[str]] = {}   # severity → [ids]

        self._load()
        self._rebuild_indexes()
        print(f"[PatternStore] Ready — {len(self._entries)} patterns")

    # ── Persistence ───────────────────────────────────────────────────────────

    def _load(self) -> None:
        if not self.db_file.exists():
            return
        with open(self.db_file, "r", encoding="utf-8") as f:
            raw = json.load(f)
        if raw.get("version") != self.STORE_VERSION:
            print(f"[PatternStore] WARNING: version mismatch, skipping load")
            return
        for eid, edata in raw.get("entries", {}).items():
            self._entries[eid] = StoreEntry.from_dict(edata)

    def _save(self) -> None:
        data = {
            "version":  self.STORE_VERSION,
            "saved_at": time.time(),
            "count":    len(self._entries),
            "entries":  {eid: e.to_dict() for eid, e in self._entries.items()},
        }
        with open(self.db_file, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)

    def _rebuild_indexes(self) -> None:
        self._by_domain.clear()
        self._by_status.clear()
        self._by_severity.clear()

        for eid, e in self._entries.items():
            # Par domaine (bitmap → plusieurs domaines possibles)
            for bit in range(8):
                if e.threat_domain & (1 << bit):
                    self._by_domain.setdefault(1 << bit, []).append(eid)
            # Par statut
            self._by_status.setdefault(e.status, []).append(eid)
            # Par sévérité
            self._by_severity.setdefault(e.severity, []).append(eid)

    # ── CRUD ──────────────────────────────────────────────────────────────────

    def add(self, entry: StoreEntry) -> bool:
        """Ajoute un pattern. Retourne False s'il existe déjà."""
        if entry.pattern_id in self._entries:
            return False
        self._entries[entry.pattern_id] = entry
        self._rebuild_indexes()
        self._save()
        return True

    def get(self, pattern_id: str) -> Optional[StoreEntry]:
        return self._entries.get(pattern_id)

    def update(self, pattern_id: str, **kwargs) -> bool:
        """Met à jour des champs d'un pattern existant."""
        e = self._entries.get(pattern_id)
        if e is None:
            return False
        for k, v in kwargs.items():
            if hasattr(e, k):
                setattr(e, k, v)
        e.updated_at = time.time()
        self._rebuild_indexes()
        self._save()
        return True

    def set_status(self, pattern_id: str, status: PatternStatus,
                   reason: str = "") -> bool:
        if reason:
            print(f"[PatternStore] {pattern_id} → {status.name}: {reason}")
        return self.update(pattern_id, status=int(status))

    # ── Queries ───────────────────────────────────────────────────────────────

    def all_active(self) -> list[StoreEntry]:
        """Tous les patterns actifs, triés par fitness décroissant."""
        active = [e for e in self._entries.values() if e.is_active]
        return sorted(active, key=lambda e: e.fitness_score, reverse=True)

    def by_domain(self, domain_bit: int) -> list[StoreEntry]:
        """Patterns couvrant un domaine spécifique."""
        ids = self._by_domain.get(domain_bit, [])
        return [self._entries[i] for i in ids if i in self._entries]

    def by_status(self, status: PatternStatus) -> list[StoreEntry]:
        ids = self._by_status.get(int(status), [])
        return [self._entries[i] for i in ids if i in self._entries]

    def by_min_severity(self, min_sev: int) -> list[StoreEntry]:
        """Patterns de sévérité >= min_sev."""
        return [e for e in self._entries.values() if e.severity >= min_sev]

    def ready_for_promotion(self,
                             min_confidence: int = 80,
                             max_fp_rate: float = 0.05,
                             min_hits: int = 3) -> list[StoreEntry]:
        """Patterns prêts à être promus dans le génome."""
        return [
            e for e in self._entries.values()
            if e.status == PatternStatus.TESTING
            and e.confidence >= min_confidence
            and e.false_positive_rate <= max_fp_rate
            and e.hit_count >= min_hits
        ]

    def search(self, text: str) -> list[StoreEntry]:
        """Recherche full-text dans les noms et descriptions."""
        text = text.lower()
        return [
            e for e in self._entries.values()
            if text in e.name.lower() or
               any(text in str(b).lower() for b in e.behaviors)
        ]

    # ── Stats ─────────────────────────────────────────────────────────────────

    def stats(self) -> dict:
        total     = len(self._entries)
        by_status = {
            PatternStatus(k).name: len(v)
            for k, v in self._by_status.items()
        }
        avg_conf = (
            sum(e.confidence for e in self._entries.values()) / total
            if total else 0
        )
        top = sorted(self._entries.values(),
                     key=lambda e: e.fitness_score, reverse=True)[:3]
        return {
            "total":       total,
            "by_status":   by_status,
            "avg_confidence": round(avg_conf, 1),
            "top_patterns": [e.name for e in top],
        }

    def print_stats(self) -> None:
        s = self.stats()
        print("\n=== PatternStore Stats ===")
        print(f"  Total patterns  : {s['total']}")
        print(f"  By status       : {s['by_status']}")
        print(f"  Avg confidence  : {s['avg_confidence']}%")
        print(f"  Top patterns    : {s['top_patterns']}")
        print("==========================\n")

    # ── Export C-compatible ───────────────────────────────────────────────────

    def export_c_header(self, output_path: Path) -> None:
        """
        Exporte les patterns validés sous forme de tableau C statique.
        Consommé par l'InstinctLayer pour enrichir ses règles.
        """
        active = self.by_status(PatternStatus.VALIDATED)
        lines = [
            "/* AUTO-GENERATED by PatternStore — DO NOT EDIT */",
            f"/* {len(active)} validated patterns */",
            "#ifndef BOT_PATTERNS_GENERATED_H",
            "#define BOT_PATTERNS_GENERATED_H",
            "",
            f"#define BOT_LEARNED_PATTERN_COUNT {len(active)}",
            "",
            "typedef struct {",
            "    const char *name;",
            "    unsigned int threat_domain;",
            "    unsigned int severity;",
            "    unsigned int confidence;",
            "    unsigned int generation;",
            "} BotLearnedPattern;",
            "",
            "static const BotLearnedPattern BOT_LEARNED_PATTERNS[] = {",
        ]
        for e in active:
            lines.append(
                f'    {{"{e.name}", 0x{e.threat_domain:04x}, '
                f'{e.severity}, {e.confidence}, {e.generation}}},'
            )
        lines += ["};", "", "#endif /* BOT_PATTERNS_GENERATED_H */", ""]

        output_path.write_text("\n".join(lines), encoding="utf-8")
        print(f"[PatternStore] C header exported: {len(active)} patterns → {output_path}")

    def __len__(self) -> int:
        return len(self._entries)

    def __iter__(self) -> Iterator[StoreEntry]:
        return iter(self._entries.values())


# ─── Test ─────────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    import tempfile, hashlib

    with tempfile.TemporaryDirectory() as tmp:
        store = PatternStore(Path(tmp))

        # Ajouter quelques patterns
        for i, (name, domain, sev) in enumerate([
            ("Ransomware-MassEncrypt",  0b00010001, 10),
            ("APT-LateralMove",         0b00000011, 9),
            ("Rootkit-KernelHook",      0b00001000, 10),
        ]):
            pid = hashlib.sha256(name.encode()).hexdigest()[:16]
            entry = StoreEntry(
                pattern_id=pid, name=name,
                threat_domain=domain, severity=sev,
                confidence=85, hit_count=5, generation=1,
            )
            store.add(entry)
            store.set_status(pid, PatternStatus.VALIDATED, "test promotion")

        store.print_stats()

        # Export C header
        store.export_c_header(Path(tmp) / "bot_patterns_generated.h")

        # Test query
        rootkits = store.by_domain(0b00001000)
        print(f"Anti-rootkit patterns: {[e.name for e in rootkits]}")
