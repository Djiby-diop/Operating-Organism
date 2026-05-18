# 🦠 Bot-Baremetal

> L'Organisme Immunitaire Instinctif de l'Operating Organism

[![Status](https://img.shields.io/badge/status-genèse-blueviolet)](.)
[![OO](https://img.shields.io/badge/OO-compatible-brightgreen)](.)
[![Phase](https://img.shields.io/badge/phase-0%20%E2%80%94%20fondation-orange)](.)

---

## Vision

Le **Bot-Baremetal** n'est pas un antivirus.
C'est un **organisme défensif instinctif** natif à l'Operating Organism (OO).

Il complète le [LLM-Baremetal](../llm-baremetal/) :

| LLM-Baremetal | Bot-Baremetal |
|---|---|
| Cortex — il pense | Système immunitaire — il survit |
| Raison | Instinct |
| Secondes | Microsecondes |
| Intelligence conversationnelle | Intelligence systémique |

---

## Structure du Projet

```
bot-baremetal/
├── BOT_MANIFESTE.md       ← Constitution fondatrice (lire en premier)
├── README.md              ← Ce fichier
│
├── core/                  ← Le Soma du Bot (C)
│   ├── bot_dna.h          ← Structure ADN de chaque agent
│   ├── instinct_layer.c   ← Réflexes primitifs (< 1ms)
│   ├── territory_map.c    ← Cartographie du système
│   └── agent_base.c       ← Template d'agent
│
├── immune/                ← Système Immunitaire (Rust)
│   ├── src/
│   │   ├── swarm_mind.rs  ← Chef d'État — coordinateur
│   │   ├── mem_watch.rs   ← Surveillance mémoire
│   │   ├── fs_watch.rs    ← Surveillance fichiers
│   │   ├── net_watch.rs   ← Surveillance réseau
│   │   ├── proc_watch.rs  ← Surveillance processus
│   │   ├── quarantine.rs  ← Isolation de menaces
│   │   └── neutralizer.rs ← Neutralisation confirmée
│   └── Cargo.toml
│
├── instinct/              ← Couche Réflexe Pure (C, ultra-rapide)
│   ├── triggers.c         ← Déclencheurs d'instinct
│   ├── reactions.c        ← Réactions primitives
│   └── threat_levels.h   ← Niveaux de menace 0-5
│
├── adaptive/              ← Moteur d'Adaptation (Python)
│   ├── mimicry_agent.py   ← Apprentissage des attaques
│   ├── mutation_engine.py ← Évolution des agents
│   └── pattern_store.py   ← Base de patterns appris
│
├── agents/                ← Agents Spécialisés
│   ├── honey_trap/        ← Leurres et pièges
│   ├── chameleon/         ← Camouflage du Bot
│   └── regen/             ← Auto-reconstruction
│
├── oo_bridge/             ← Interface avec LLM-Baremetal
│   ├── bridge_agent.py    ← Communication OO
│   └── oo_protocol.h     ← Protocole de communication
│
├── tests/                 ← Tests et simulations
│   ├── attack_sim/        ← Simulateur d'attaques
│   └── swarm_test/        ← Tests de l'essaim
│
└── docs/
    ├── ARCHITECTURE.md    ← Architecture détaillée
    ├── AGENTS.md          ← Catalogue des agents
    └── INTEGRATION_OO.md  ← Guide d'intégration OO
```

## Facade Canonique Baremetal

Pour alignement avec le standard racine (`BAREMETAL_STANDARD.md`), le module expose aussi:

- `include/bot_baremetal.h`
- `src/bot_baremetal_entry.c`

Ces fichiers servent d'interface canonique `include/src` sans remplacer l'architecture interne actuelle (`core/`, `immune/`, `instinct/`, etc.).

---

## Lire en premier

1. **[BOT_MANIFESTE.md](./BOT_MANIFESTE.md)** — La constitution complète
2. **[docs/ARCHITECTURE.md](./docs/ARCHITECTURE.md)** — Architecture technique
3. **[docs/AGENTS.md](./docs/AGENTS.md)** — Catalogue des agents

---

## Développement Séparé, Cohabitation Future

Ce projet se développe indépendamment du LLM-Baremetal.
Les deux rejoindront l'OO quand chacun sera stable.

Règle de développement :
- **Aucune dépendance** vers llm-baremetal dans le code
- **Interface commune** uniquement via `oo_bridge/`
- **Journal OO compatible** avec le format existant

---

## Niveaux de Menace

```
0 — DORMANT     Surveillance passive
1 — VIGILANCE   Activité suspecte détectée
2 — ALERTE      Comportement malveillant probable
3 — COMBAT      Intrusion confirmée, réponse active
4 — SURVIE      Le Bot lui-même est attaqué
5 — CONFINEMENT Menace existentielle pour l'OO
```

---

## Philosophy

> "Un bot plus avancé qu'une IA moderne ne pense pas mieux.
>  Il SURVIT mieux."

---

*Bot-Baremetal — Part of the Operating Organism Ecosystem*
