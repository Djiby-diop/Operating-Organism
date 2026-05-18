"""
BOT-BAREMETAL — GenomeSync
Synchronisation du génome entre le monde Python (MutationEngine)
et le monde C/Rust (InstinctLayer, agents bas niveau).

"Les anticorps appris en Python doivent atteindre les agents C
 en microsecondes. GenomeSync est ce pont."

Cycle de sync :
  MutationEngine (Python)
       ↓ export JSON
  GenomeSync
       ↓ génère header C + payload binaire
  InstinctLayer (C) charge les nouveaux patterns
       ↓
  SwarmMind (Rust) diffuse à la flotte
"""

from __future__ import annotations

import json
import hashlib
import struct
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from pattern_store import PatternStore, StoreEntry, PatternStatus


# ─── Format binaire d'un pattern (compatible C struct) ───────────────────────
#
# struct BotSyncPattern {
#     char     id[16];           // Pattern ID (ASCII hex)
#     char     name[64];         // Nom lisible
#     uint32_t threat_domain;    // Bitmap domaines
#     uint32_t severity;         // 0-10
#     uint32_t confidence;       // 0-100
#     uint32_t generation;       // Génération
#     float    fitness;          // Score fitness
#     uint64_t hit_count;        // Détections confirmées
#     uint8_t  is_validated;     // 1 = intégré génome
#     uint8_t  _pad[7];
# } __attribute__((packed));
#
# Taille : 16 + 64 + 4*4 + 4 + 8 + 1 + 7 = 116 bytes

SYNC_MAGIC   = b"BSYN"    # Magic header du fichier de sync
SYNC_VERSION = 1
PATTERN_STRUCT_SIZE = 116


def _pack_pattern(e: StoreEntry) -> bytes:
    """Sérialise un StoreEntry en bytes C-compatible."""
    pid_bytes  = e.pattern_id.encode("ascii")[:16].ljust(16, b"\x00")
    name_bytes = e.name.encode("utf-8")[:64].ljust(64, b"\x00")
    return struct.pack(
        "<16s64sIIIIf Q B 7x",
        pid_bytes,
        name_bytes,
        e.threat_domain & 0xFFFFFFFF,
        max(0, min(10, e.severity)),
        max(0, min(100, e.confidence)),
        e.generation,
        float(e.fitness_score),
        e.hit_count,
        1 if e.status == PatternStatus.VALIDATED else 0,
    )


def _unpack_pattern(data: bytes, offset: int) -> Optional[dict]:
    """Désérialise un pattern depuis des bytes."""
    if offset + PATTERN_STRUCT_SIZE > len(data):
        return None
    fields = struct.unpack_from("<16s64sIIIIf Q B 7x", data, offset)
    return {
        "pattern_id":    fields[0].rstrip(b"\x00").decode("ascii"),
        "name":          fields[1].rstrip(b"\x00").decode("utf-8"),
        "threat_domain": fields[2],
        "severity":      fields[3],
        "confidence":    fields[4],
        "generation":    fields[5],
        "fitness_score": fields[6],
        "hit_count":     fields[7],
        "is_validated":  bool(fields[8]),
    }


# ─── Fichier de sync binaire ─────────────────────────────────────────────────
#
# Format :
#   [magic:4][version:4][timestamp:8][count:4][checksum:4]
#   [pattern_0: 116 bytes]
#   [pattern_1: 116 bytes]
#   ...

SYNC_HEADER_SIZE = 24   # magic(4) + version(4) + ts(8) + count(4) + crc(4)


class GenomeSync:
    """
    Pont bidirectionnel entre le génome Python et les agents C/Rust.

    Exports :
      - Fichier binaire (.bsyn) : consommé par InstinctLayer C
      - Header C auto-généré   : inclus à la compilation
      - JSON enrichi           : consommé par SwarmMind Rust

    Import :
      - Feedback des agents C  : confirmations de détection
    """

    def __init__(self, sync_dir: Path, store: PatternStore):
        self.sync_dir = sync_dir
        self.sync_dir.mkdir(parents=True, exist_ok=True)
        self.store = store

        self.binary_file  = sync_dir / "genome.bsyn"
        self.json_file    = sync_dir / "genome_sync.json"
        self.c_header     = sync_dir / "bot_genome_sync.h"
        self.feedback_file = sync_dir / "agent_feedback.jsonl"

        self._last_sync: float = 0.0
        self._sync_count: int  = 0

    # ── Export Python → C/Rust ────────────────────────────────────────────────

    def export_all(self) -> int:
        """
        Exporte le génome complet vers tous les formats consommables.
        Retourne le nombre de patterns exportés.
        """
        validated = self.store.by_status(PatternStatus.VALIDATED)
        n = len(validated)

        self._write_binary(validated)
        self._write_json(validated)
        self._write_c_header(validated)

        self._last_sync  = time.time()
        self._sync_count += 1

        print(f"[GenomeSync] Export #{self._sync_count} — {n} patterns → "
              f"binary + json + C header")
        return n

    def _write_binary(self, patterns: list[StoreEntry]) -> None:
        """Écrit le fichier .bsyn pour les agents C."""
        payload = b"".join(_pack_pattern(e) for e in patterns)
        # Checksum CRC32 simple
        import zlib
        crc = zlib.crc32(payload) & 0xFFFFFFFF
        header = struct.pack(
            "<4sI d I I",
            SYNC_MAGIC,
            SYNC_VERSION,
            time.time(),
            len(patterns),
            crc,
        )
        with open(self.binary_file, "wb") as f:
            f.write(header + payload)

    def _write_json(self, patterns: list[StoreEntry]) -> None:
        """Écrit le fichier JSON pour SwarmMind Rust."""
        data = {
            "version":    SYNC_VERSION,
            "synced_at":  time.time(),
            "count":      len(patterns),
            "patterns": [
                {
                    "id":           e.pattern_id,
                    "name":         e.name,
                    "threat_domain": e.threat_domain,
                    "severity":     e.severity,
                    "confidence":   e.confidence,
                    "generation":   e.generation,
                    "fitness":      round(e.fitness_score, 3),
                    "hit_count":    e.hit_count,
                }
                for e in patterns
            ],
        }
        with open(self.json_file, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)

    def _write_c_header(self, patterns: list[StoreEntry]) -> None:
        """Génère un header C compilable avec les patterns validés."""
        lines = [
            "/* AUTO-GENERATED by GenomeSync — DO NOT EDIT MANUALLY */",
            f"/* Synced: {time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())} */",
            f"/* Patterns: {len(patterns)} validated */",
            "#ifndef BOT_GENOME_SYNC_H",
            "#define BOT_GENOME_SYNC_H",
            "",
            "#include <stdint.h>",
            "",
            "typedef struct {",
            "    const char     *id;",
            "    const char     *name;",
            "    uint32_t        threat_domain;",
            "    uint32_t        severity;",
            "    uint32_t        confidence;",
            "    uint32_t        generation;",
            "    float           fitness;",
            "    uint64_t        hit_count;",
            "} BotGenomePattern;",
            "",
            f"#define BOT_GENOME_COUNT {len(patterns)}",
            "",
            "static const BotGenomePattern BOT_GENOME[] = {",
        ]
        for e in patterns:
            safe_name = e.name.replace('"', '\\"')
            lines.append(
                f'    {{"{e.pattern_id}", "{safe_name}", '
                f'0x{e.threat_domain:04X}u, {e.severity}u, '
                f'{e.confidence}u, {e.generation}u, '
                f'{e.fitness_score:.3f}f, {e.hit_count}ULL}},'
            )
        lines += ["};", "", "#endif /* BOT_GENOME_SYNC_H */", ""]
        self.c_header.write_text("\n".join(lines), encoding="utf-8")

    # ── Import C → Python (feedback des agents) ───────────────────────────────

    def process_agent_feedback(self) -> int:
        """
        Lit les feedbacks des agents C/Rust (détections confirmées, faux positifs).
        Met à jour le PatternStore en conséquence.
        Retourne le nombre de feedbacks traités.
        """
        if not self.feedback_file.exists():
            return 0

        processed = 0
        lines = self.feedback_file.read_text(encoding="utf-8").strip().split("\n")

        for line in lines:
            if not line.strip():
                continue
            try:
                fb = json.loads(line)
                pid  = fb.get("pattern_id", "")
                kind = fb.get("kind", "")     # "hit" ou "false_positive"

                entry = self.store.get(pid)
                if entry is None:
                    continue

                if kind == "hit":
                    self.store.update(pid,
                                      hit_count=entry.hit_count + 1,
                                      confidence=min(100, entry.confidence + 2))
                elif kind == "false_positive":
                    self.store.update(pid,
                                      false_pos_count=entry.false_pos_count + 1,
                                      confidence=max(0, entry.confidence - 5))

                processed += 1
            except (json.JSONDecodeError, KeyError):
                pass

        if processed:
            # Vider le fichier de feedback après traitement
            self.feedback_file.write_text("", encoding="utf-8")
            print(f"[GenomeSync] Processed {processed} agent feedback(s)")

        return processed

    def write_agent_feedback(self, pattern_id: str, kind: str) -> None:
        """
        Enregistre un feedback d'agent (appelé par les agents C via ce wrapper Python,
        ou directement depuis Rust via le fichier JSONL).
        """
        fb = json.dumps({"pattern_id": pattern_id, "kind": kind,
                         "ts": time.time()})
        with open(self.feedback_file, "a", encoding="utf-8") as f:
            f.write(fb + "\n")

    # ── Statut ────────────────────────────────────────────────────────────────

    def status(self) -> dict:
        return {
            "sync_count":     self._sync_count,
            "last_sync":      time.strftime(
                "%Y-%m-%dT%H:%M:%SZ", time.gmtime(self._last_sync)
            ) if self._last_sync else "never",
            "binary_exists":  self.binary_file.exists(),
            "json_exists":    self.json_file.exists(),
            "c_header_exists": self.c_header.exists(),
            "total_in_store": len(self.store),
        }

    def print_status(self) -> None:
        s = self.status()
        print("\n=== GenomeSync Status ===")
        for k, v in s.items():
            print(f"  {k:25s}: {v}")
        print("=========================\n")


# ─── Test ─────────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    import tempfile

    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)

        # Créer un store avec des patterns validés
        store = PatternStore(tmp_path / "store")

        for name, domain, sev in [
            ("Ransomware-v2",  0x11, 10),
            ("APT-LateralMove", 0x03, 9),
        ]:
            pid = hashlib.sha256(name.encode()).hexdigest()[:16]
            from pattern_store import StoreEntry, PatternStatus
            e = StoreEntry(pid, name, domain, sev, confidence=90,
                           hit_count=10, generation=2, fitness_score=0.85)
            e.status = PatternStatus.VALIDATED
            store.add(e)

        # Sync
        sync = GenomeSync(tmp_path / "sync", store)
        n = sync.export_all()
        print(f"Exported {n} patterns")

        # Simuler feedback agent
        pids = list(store._entries.keys())
        if pids:
            sync.write_agent_feedback(pids[0], "hit")
            sync.write_agent_feedback(pids[0], "hit")
            sync.write_agent_feedback(pids[0], "false_positive")
            processed = sync.process_agent_feedback()
            print(f"Feedback processed: {processed}")

        sync.print_status()

        # Vérifier le header C généré
        if sync.c_header.exists():
            print("\n--- Generated C Header (first 10 lines) ---")
            lines = sync.c_header.read_text().split("\n")[:10]
            for l in lines:
                print(f"  {l}")
