# 🦠 Bot-Baremetal — Architecture Technique

## Vue d'Ensemble

```
┌─────────────────────────────────────────────────────────────────┐
│                       BOT-BAREMETAL                             │
│                  L'Organisme Immunitaire de l'OO                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  COUCHE 7 — CONSCIENCE DE FLOTTE            [Rust]             │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  SwarmMind (swarm_mind.rs)                              │   │
│  │  • Coordinateur de toute la flotte                      │   │
│  │  • Machine d'état atomique (lock-free)                  │   │
│  │  • Ordres de déploiement/retrait des agents             │   │
│  │  • Supervision de la santé de la flotte                 │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  COUCHE 6 — MOTEUR D'ÉVOLUTION              [Python]           │
│  ┌───────────────────┐  ┌────────────────────────────────┐    │
│  │  MimicryAgent     │  │  MutationEngine                │    │
│  │  • Observe        │→ │  • Intègre au génome           │    │
│  │  • Distille       │  │  • Évalue fitness              │    │
│  │  • Forge anticorps│  │  • Mute / Fusionne             │    │
│  └───────────────────┘  │  • Élimine défectueux          │    │
│                          └────────────────────────────────┘    │
│                                                                 │
│  COUCHE 5 — AGENTS DE RÉPONSE               [Rust]             │
│  ┌────────────┐ ┌──────────────┐ ┌──────────┐ ┌──────────┐   │
│  │ Quarantine │ │  HoneyTrap   │ │Neutralize│ │Chameleon │   │
│  │ (isoler)   │ │  (leurrer)   │ │ (élimin.)│ │(cacher)  │   │
│  └────────────┘ └──────────────┘ └──────────┘ └──────────┘   │
│                                                                 │
│  COUCHE 4 — RÉSEAU CAPTEURS                 [Rust]             │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────────────┐  │
│  │MemWatch  │ │NetWatch  │ │FsWatch   │ │ProcWatch         │  │
│  │(RAM)     │ │(Réseau)  │ │(Fichiers)│ │(Processus)       │  │
│  └──────────┘ └──────────┘ └──────────┘ └──────────────────┘  │
│  ┌──────────┐ ┌──────────┐                                     │
│  │KernWatch │ │BootWatch │                                     │
│  │(Kernel)  │ │(UEFI)    │                                     │
│  └──────────┘ └──────────┘                                     │
│                                                                 │
│  COUCHE 3 — INSTINCT                        [C, < 1ms]         │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  InstinctLayer (instinct_layer.c)                       │   │
│  │  • Table de règles instinctives (lookup, O(n))          │   │
│  │  • Déclenchement → Réaction en < 1ms                    │   │
│  │  • Historique circulaire (sans allocation)              │   │
│  │  • Callbacks vers agents (non bloquants)                │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  COUCHE 2 — CARTE DU TERRITOIRE             [C]                │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  TerritoryMap (territory_map.h / .c)                    │   │
│  │  • Processus (512 max)     • Connexions (256 max)       │   │
│  │  • Fichiers critiques (64) • Régions mémoire (128)      │   │
│  │  • Anomalies historique circulaire (256)                │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  COUCHE 1 — ADN                             [C]                │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  BotDNA (bot_dna.h / .c)                                │   │
│  │  • Identité (UUID 128-bit)   • Rôle spécialisé         │   │
│  │  • Capacités (bitmap 64-bit) • 64 patterns max          │   │
│  │  • SHA256 d'intégrité        • Directives immuables     │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Langages par Couche

| Couche | Rôle | Langage | Justification |
|--------|------|---------|---------------|
| 7 — SwarmMind | Coordinateur | **Rust** | Sécurité mémoire, atomics, lock-free |
| 6 — Évolution | Apprentissage | **Python** | Flexibilité, datascience, itération rapide |
| 5 — Réponse | Agents actifs | **Rust** | Fiabilité, pas de segfaults |
| 4 — Capteurs | Surveillance | **Rust** | Performance, sécurité |
| 3 — Instinct | Réflexes | **C** | Ultra-rapide, bare-metal compatible |
| 2 — Territoire | Cartographie | **C** | Pools fixes, pas d'allocation |
| 1 — ADN | Identité | **C** | Sérialisable, bare-metal compatible |

> **Règle OO** : le Bot suit la même hiérarchie de langages que le manifeste OO.
> C = Soma, Rust = Système Immunitaire, Python = Cortex.

---

## Flux de Données Principal

```
ÉVÉNEMENT SYSTÈME
      │
      ▼
┌─────────────┐    trigger()    ┌──────────────────┐
│  Capteur    │ ─────────────→ │  InstinctLayer   │
│  (Watcher)  │                │  < 1ms           │
└─────────────┘                └──────────┬───────┘
                                          │ callback
                                          ▼
                               ┌──────────────────┐
                               │   SwarmMind      │
                               │   coordonne      │
                               └──────────┬───────┘
                                          │ ordres
                               ┌──────────┴───────┐
                        ┌──────┘                  └──────┐
                        ▼                                ▼
               ┌────────────────┐            ┌───────────────────┐
               │ Agent Réponse  │            │  MimicryAgent     │
               │ (Quarantine,   │            │  (Apprentissage   │
               │  HoneyTrap...) │            │   du pattern)     │
               └────────────────┘            └────────┬──────────┘
                                                      │
                                                      ▼
                                             ┌────────────────┐
                                             │ MutationEngine │
                                             │ (Immunisation  │
                                             │  de l'essaim)  │
                                             └────────────────┘
```

---

## Contraintes de Performance

| Opération | Cible | Mesure |
|-----------|-------|--------|
| `instinct_trigger()` | < 1ms | Lookup table O(n) |
| `quarantine_process()` | < 5ms | Suspension directe |
| `territory_observe_*()` | < 100µs | Pool fixe, pas d'alloc |
| `bot_dna_verify()` | < 500µs | SHA256 in-place |
| `swarm.process_event()` | < 2ms | Atomics lock-free |
| `mimicry.observe_attack()` | < 50ms | I/O JSON + hash |
| `engine.absorb_ready()` | < 100ms | Batch JSON |

> **Règle absolue** : aucune opération sur le chemin critique (instinct → réaction)
> ne doit dépasser 5ms. Au-delà, le Bot perd son avantage sur l'attaquant.

---

## Gestion Mémoire

**Règle du Bot** : zéro allocation dynamique sur le chemin critique.

| Structure | Stratégie | Taille max |
|-----------|-----------|------------|
| `BotDNA` | Stack ou pool statique | ~8 KB par agent |
| `TerritoryMap` | Pool fixe statique | ~500 KB |
| `InstinctLayer` | Stack | ~20 KB |
| `SwarmMind` | Heap Rust (une fois à l'init) | ~2 MB flotte complète |

---

## Sérialisation OO

Tous les états critiques sont sérialisables au format binaire compact
pour persistance dans le journal OO (compatible avec UEFI FAT).

Format `BotDNA` sérialisé :
```
[magic:4][version:4][agent_id:16][role:4][gen:4][name:32]
[capabilities:8][domains:4][threat_level:4]
[patterns:N*80][pattern_count:4]
[stats:40][integrity_hash:32][prime_dir:256][pad:64]
Total : ~1.2 KB par agent
```

---

## Compatibilité Bare-Metal

Le cœur C (couches 1-3) est conçu pour être compilé sans libc :

- Pas de `malloc` / `free`
- Pas de `printf` (remplacé par callback journal OO)
- SHA256 implémenté inline (pas d'OpenSSL)
- Pools fixes sur `.bss` ou stack
- Compatible UEFI (appels GOP/SimpleFS uniquement)

---

*Bot-Baremetal Architecture — Part of the Operating Organism*
