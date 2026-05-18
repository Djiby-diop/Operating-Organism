"""
BOT-BAREMETAL — MutationEngine
Le moteur d'évolution : forge les mutations, teste les variants, immunise l'essaim.

"Ce qui ne tue pas le Bot le rend plus fort — littéralement."

Cycle complet :
    MimicryAgent → patterns prêts → MutationEngine → intégration génome
                                                    → diffusion aux agents
                                                    → benchmarking continu
                                                    → élimination des faibles
                                                    → fusion des forts
"""

from __future__ import annotations

import json
import random
import time
from dataclasses import dataclass, field, asdict
from pathlib import Path
from typing import Optional

from mimicry_agent import BehaviorPattern, MimicryAgent, ThreatDomain


# ─── Génome de l'Essaim ──────────────────────────────────────────────────────

@dataclass
class GenomeEntry:
    """Un pattern intégré au génome de toute la flotte."""
    pattern_id:     str
    pattern_name:   str
    threat_domain:  int
    severity:       int
    behaviors:      list        = field(default_factory=list)
    generation:     int         = 1        # Génération (mute au fil du temps)
    fitness_score:  float       = 0.0      # Score d'efficacité 0.0 - 1.0
    mutation_count: int         = 0        # Nombre de mutations subies
    added_at:       float       = field(default_factory=time.time)
    last_evaluated: float       = 0.0
    is_active:      bool        = True

    def to_dict(self) -> dict:
        return asdict(self)

    @classmethod
    def from_pattern(cls, p: BehaviorPattern) -> "GenomeEntry":
        return cls(
            pattern_id=p.pattern_id,
            pattern_name=p.name,
            threat_domain=p.threat_domain,
            severity=p.severity,
            behaviors=list(p.behaviors),
        )

    @classmethod
    def from_dict(cls, d: dict) -> "GenomeEntry":
        return cls(**d)


# ─── Résultat d'Évaluation ───────────────────────────────────────────────────

@dataclass
class EvaluationResult:
    pattern_id:      str
    detection_rate:  float   # 0.0 - 1.0
    false_pos_rate:  float   # 0.0 - 1.0
    speed_score:     float   # 0.0 - 1.0 (1.0 = très rapide)
    fitness_score:   float   # Calculé automatiquement
    evaluated_at:    float   = field(default_factory=time.time)

    @staticmethod
    def compute_fitness(detection: float, false_pos: float, speed: float) -> float:
        """
        Formule de fitness :
          detection_rate pèse 60%
          false_pos_rate pèse -30% (pénalité)
          speed_score pèse 10%
        """
        return max(0.0, detection * 0.6 - false_pos * 0.3 + speed * 0.1)


# ─── MutationEngine ──────────────────────────────────────────────────────────

class MutationEngine:
    """
    Gère l'évolution du génome de l'essaim.

    Responsabilités :
    - Intégrer les patterns validés par MimicryAgent
    - Évaluer l'efficacité de chaque pattern (fitness)
    - Muter les patterns faibles pour tester de nouvelles variantes
    - Fusionner les patterns complémentaires
    - Éliminer les patterns défectueux
    - Diffuser le génome à jour à tous les agents
    """

    def __init__(self, storage_path: Path, mimicry: MimicryAgent):
        self.storage_path = storage_path
        self.storage_path.mkdir(parents=True, exist_ok=True)
        self.mimicry = mimicry
        self.genome_file = storage_path / "genome.json"
        self.genome: dict[str, GenomeEntry] = {}
        self._load_genome()
        print(f"[MutationEngine] Ready — {len(self.genome)} entries in genome")

    # ── Persistence ──────────────────────────────────────────────────────────

    def _load_genome(self) -> None:
        if not self.genome_file.exists():
            return
        with open(self.genome_file, "r", encoding="utf-8") as f:
            raw = json.load(f)
        for gid, gdata in raw.items():
            self.genome[gid] = GenomeEntry.from_dict(gdata)

    def _save_genome(self) -> None:
        data = {gid: g.to_dict() for gid, g in self.genome.items()}
        with open(self.genome_file, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2)

    # ── Intégration depuis MimicryAgent ──────────────────────────────────────

    def absorb_ready_patterns(self) -> int:
        """
        Récupère les patterns prêts chez MimicryAgent et les intègre au génome.
        Retourne le nombre de patterns intégrés.
        """
        ready = self.mimicry.get_patterns_ready_for_promotion()
        count = 0
        for pattern in ready:
            if pattern.pattern_id not in self.genome:
                entry = GenomeEntry.from_pattern(pattern)
                self.genome[pattern.pattern_id] = entry
                self.mimicry.promote_pattern(pattern.pattern_id)
                print(f"[MutationEngine] Absorbed into genome: '{pattern.name}'")
                count += 1
        if count:
            self._save_genome()
        return count

    # ── Évaluation de Fitness ─────────────────────────────────────────────────

    def evaluate(self, pattern_id: str,
                 detection_rate: float,
                 false_pos_rate: float,
                 speed_score: float = 1.0) -> EvaluationResult:
        """
        Évalue l'efficacité d'un pattern dans le génome.
        En production, ces métriques viendraient des agents en temps réel.
        """
        fitness = EvaluationResult.compute_fitness(
            detection_rate, false_pos_rate, speed_score
        )
        result = EvaluationResult(
            pattern_id=pattern_id,
            detection_rate=detection_rate,
            false_pos_rate=false_pos_rate,
            speed_score=speed_score,
            fitness_score=fitness,
        )
        if pattern_id in self.genome:
            self.genome[pattern_id].fitness_score = fitness
            self.genome[pattern_id].last_evaluated = time.time()
            self._save_genome()
        return result

    # ── Mutation ──────────────────────────────────────────────────────────────

    def mutate_weak_patterns(self, fitness_threshold: float = 0.4) -> int:
        """
        Mute les patterns dont le fitness est sous le seuil.
        Une mutation = ajout/modification d'une règle de comportement.
        Retourne le nombre de patterns mutés.
        """
        mutated = 0
        for gid, entry in self.genome.items():
            if not entry.is_active:
                continue
            if entry.fitness_score < fitness_threshold and entry.last_evaluated > 0:
                # Stratégie de mutation : relaxer les critères de détection
                new_behavior = {
                    "type": "mutation_relaxed",
                    "generation": entry.generation + 1,
                    "strategy": random.choice([
                        "broaden_scope",
                        "increase_sensitivity",
                        "add_temporal_context",
                        "combine_triggers",
                    ])
                }
                entry.behaviors.append(new_behavior)
                entry.generation += 1
                entry.mutation_count += 1
                entry.fitness_score = 0.0  # Reset pour réévaluation
                print(f"[MutationEngine] Mutated '{entry.pattern_name}' "
                      f"gen={entry.generation} strategy={new_behavior['strategy']}")
                mutated += 1
        if mutated:
            self._save_genome()
        return mutated

    # ── Fusion ────────────────────────────────────────────────────────────────

    def fuse_patterns(self, id_a: str, id_b: str, new_name: str) -> Optional[GenomeEntry]:
        """
        Fusionne deux patterns complémentaires en un hybride plus fort.
        Utilisé quand deux patterns détectent des facettes du même attack type.
        """
        if id_a not in self.genome or id_b not in self.genome:
            return None

        a, b = self.genome[id_a], self.genome[id_b]

        import hashlib
        fused_id = hashlib.sha256(f"{id_a}+{id_b}".encode()).hexdigest()[:16]

        fused = GenomeEntry(
            pattern_id=fused_id,
            pattern_name=new_name,
            threat_domain=a.threat_domain | b.threat_domain,
            severity=max(a.severity, b.severity),
            behaviors=list(a.behaviors) + list(b.behaviors),
            generation=max(a.generation, b.generation) + 1,
            fitness_score=(a.fitness_score + b.fitness_score) / 2,
        )

        # Désactiver les parents
        a.is_active = False
        b.is_active = False

        self.genome[fused_id] = fused
        self._save_genome()
        print(f"[MutationEngine] Fusion: '{a.pattern_name}' + '{b.pattern_name}' "
              f"→ '{new_name}'")
        return fused

    # ── Élimination ───────────────────────────────────────────────────────────

    def eliminate_defective(self, fitness_floor: float = 0.1) -> int:
        """
        Désactive les patterns trop mauvais après plusieurs évaluations.
        Ils restent dans la mémoire (archivés) mais ne sont plus utilisés.
        """
        eliminated = 0
        for gid, entry in self.genome.items():
            if not entry.is_active:
                continue
            if (entry.fitness_score < fitness_floor
                    and entry.last_evaluated > 0
                    and entry.mutation_count >= 3):
                entry.is_active = False
                print(f"[MutationEngine] Eliminated '{entry.pattern_name}' "
                      f"(fitness={entry.fitness_score:.2f})")
                eliminated += 1
        if eliminated:
            self._save_genome()
        return eliminated

    # ── Export vers les Agents ────────────────────────────────────────────────

    def export_active_genome(self) -> list[GenomeEntry]:
        """
        Retourne le génome actif prêt à être diffusé aux agents.
        Trié par fitness décroissant.
        """
        active = [g for g in self.genome.values() if g.is_active]
        active.sort(key=lambda g: g.fitness_score, reverse=True)
        return active

    def export_to_json(self, output_path: Path) -> None:
        """Exporte le génome actif en JSON pour consommation par les agents C/Rust."""
        active = self.export_active_genome()
        data = [g.to_dict() for g in active]
        with open(output_path, "w", encoding="utf-8") as f:
            json.dump({"version": 1, "count": len(data), "genome": data}, f, indent=2)
        print(f"[MutationEngine] Genome exported: {len(data)} active patterns → {output_path}")

    # ── Rapport ───────────────────────────────────────────────────────────────

    def status_report(self) -> dict:
        total    = len(self.genome)
        active   = sum(1 for g in self.genome.values() if g.is_active)
        avg_fit  = (sum(g.fitness_score for g in self.genome.values() if g.is_active)
                    / active) if active else 0.0
        mutated  = sum(g.mutation_count for g in self.genome.values())
        return {
            "genome_size":        total,
            "active_patterns":    active,
            "inactive_archived":  total - active,
            "avg_fitness_active": round(avg_fit, 3),
            "total_mutations":    mutated,
        }

    def print_status(self) -> None:
        r = self.status_report()
        print("\n=== MutationEngine Status ===")
        for k, v in r.items():
            print(f"  {k:30s}: {v}")
        print("==============================\n")


# ─── Point d'Entrée (test) ───────────────────────────────────────────────────

if __name__ == "__main__":
    import tempfile

    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)

        # MimicryAgent — apprend une attaque
        mimicry = MimicryAgent(tmp_path / "mimicry")
        p = mimicry.observe_attack(
            name="LateralMovement-PsExec",
            description="Lateral movement via PsExec",
            behaviors=[
                {"type": "net_connect", "port": 445},
                {"type": "proc_spawn", "parent": "services.exe", "child": "cmd.exe"},
                {"type": "file_write", "path": "C:\\Windows\\Temp\\"},
            ],
            threat_domain=int(ThreatDomain.ANTIVIRUS | ThreatDomain.ANTIPIRACY),
            severity=9,
        )
        for _ in range(4):
            mimicry.confirm_detection(p.pattern_id)

        # MutationEngine — absorbe et évalue
        engine = MutationEngine(tmp_path / "mutation", mimicry)
        absorbed = engine.absorb_ready_patterns()
        print(f"Absorbed: {absorbed} patterns")

        engine.evaluate(p.pattern_id,
                        detection_rate=0.85,
                        false_pos_rate=0.02,
                        speed_score=0.9)

        engine.print_status()
        engine.export_to_json(tmp_path / "genome_export.json")
