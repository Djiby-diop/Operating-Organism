"""
BOT-BAREMETAL — GenomeExporter
Exporte le génome (patterns) du Bot vers le LLM dans un format
compréhensible pour une intégration OO (Operating Organism).

Rôle : 
 - Lire le PatternStore ou GenomeSync.
 - Formater en JSON-lines pour ajout au OOJOUR.LOG ou BOT_EVENTS.LOG.
 - Permettre au LLM d'analyser l'évolution de la flotte.
"""

from __future__ import annotations

import json
import time
from pathlib import Path
from typing import List

class GenomeExporter:
    """
    Exporte l'état courant de l'apprentissage (anticorps).
    Le LLM peut s'en servir pour comprendre quelles menaces le Bot sait bloquer.
    """

    def __init__(self, oo_data_path: Path, genome_json_path: Path):
        self.oo_data_path = oo_data_path
        self.genome_json_path = genome_json_path
        self.oo_events_log = oo_data_path / "BOT_EVENTS.LOG"

    def export_to_oo_journal(self) -> int:
        """
        Lit le génome exporté (genome_sync.json) et crée un événement OO_MSG_BOT_GENOME.
        """
        if not self.genome_json_path.exists():
            print(f"[GenomeExporter] No genome found at {self.genome_json_path}")
            return 0

        with open(self.genome_json_path, "r", encoding="utf-8") as f:
            data = json.load(f)

        patterns = data.get("patterns", [])
        if not patterns:
            return 0

        # On emballe le génome dans un OOMessage
        # OO_MSG_BOT_GENOME = 4 (cf oo_protocol.h)
        msg_payload = {
            "version": data.get("version", 1),
            "synced_at": data.get("synced_at", time.time()),
            "pattern_count": data.get("count", len(patterns)),
            "patterns": patterns
        }

        import hashlib
        ts = time.time()
        msg_id = hashlib.sha256(f"{ts}_genome_export".encode()).hexdigest()[:12]

        oo_msg = {
            "oo_type": 4, # BOT_GENOME
            "version": 1,
            "timestamp": ts,
            "from_entity": "bot-baremetal",
            "to_entity": "llm-baremetal",
            "msg_id": msg_id,
            "payload": msg_payload
        }

        self.oo_data_path.mkdir(parents=True, exist_ok=True)
        with open(self.oo_events_log, "a", encoding="utf-8") as f:
            f.write(json.dumps(oo_msg, ensure_ascii=False) + "\n")

        print(f"[GenomeExporter] Exported {len(patterns)} patterns to OO log ({self.oo_events_log})")
        return len(patterns)

if __name__ == "__main__":
    import tempfile
    
    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        
        # Fake genome_sync.json
        genome_file = tmp_path / "genome_sync.json"
        genome_file.write_text(json.dumps({
            "version": 1,
            "synced_at": time.time(),
            "count": 2,
            "patterns": [
                {"id": "abc", "name": "FakeRansom", "threat_domain": 16, "severity": 10, "confidence": 99, "generation": 1, "fitness": 0.9, "hit_count": 5},
                {"id": "def", "name": "FakeRootkit", "threat_domain": 8, "severity": 10, "confidence": 95, "generation": 2, "fitness": 0.8, "hit_count": 2}
            ]
        }))

        exporter = GenomeExporter(tmp_path / "oo_data", genome_file)
        exporter.export_to_oo_journal()
