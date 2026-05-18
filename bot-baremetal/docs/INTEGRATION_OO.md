# 🔗 Bot-Baremetal — Intégration avec l'OO

## Principe Fondamental

Le Bot-Baremetal et le LLM-Baremetal sont deux organes distincts du même OO.

```
┌─────────────────────────────────────────────────────┐
│                 OPERATING ORGANISM                   │
│                                                     │
│  ┌──────────────────┐    ┌──────────────────────┐  │
│  │  LLM-BAREMETAL   │    │   BOT-BAREMETAL       │  │
│  │                  │    │                       │  │
│  │  🧠 Cortex       │◄──►│  🛡️ Système Immun.   │  │
│  │  Raison          │    │  Instinct             │  │
│  │  Analyse         │    │  Protection           │  │
│  │  Génération      │    │  Survie               │  │
│  └──────────────────┘    └──────────────────────┘  │
│           │                        │                │
│           └────────────┬───────────┘                │
│                        │                            │
│               OO Journal Unifié                     │
│               OO Command Protocol                   │
└─────────────────────────────────────────────────────┘
```

---

## Protocole de Communication

### Format des Messages Bot → LLM

```json
{
  "oo_type": "BOT_EVENT",
  "version": 1,
  "timestamp": 1715258809,
  "from": "bot-baremetal",
  "to": "llm-baremetal",
  "payload": {
    "threat_level": 3,
    "event_type": "COMBAT",
    "agent": "mem_watch",
    "description": "Process injection detected pid=1234",
    "confidence": 85,
    "action_taken": "quarantine",
    "request": "ANALYZE_PATTERN"
  }
}
```

### Format des Messages LLM → Bot

```json
{
  "oo_type": "BOT_DIRECTIVE",
  "version": 1,
  "timestamp": 1715258810,
  "from": "llm-baremetal",
  "to": "bot-baremetal",
  "payload": {
    "directive": "DEPLOY_HONEYTRAP",
    "target": "pid:1234",
    "priority": "HIGH",
    "context": "Pattern matches known APT lateral movement"
  }
}
```

---

## Indépendance Mutuelle

**Règle absolue** : chaque système survit sans l'autre.

- Le Bot n'a **jamais besoin** du LLM pour agir en défense
- Le LLM n'a **jamais besoin** du Bot pour fonctionner
- La communication est **optionnelle et enrichissante**, jamais critique

Si le LLM tombe → le Bot continue à protéger
Si le Bot tombe → le LLM continue à fonctionner (et alerte le RegenAgent)

---

## Fichiers OO Partagés

Les deux systèmes utilisent le même format de journal OO :

```
OOJOUR.LOG          ← Journal principal (partagé)
BOT_EVENTS.LOG      ← Événements de sécurité (Bot écrit, LLM lit)
BOT_DIRECTIVES.LOG  ← Directives stratégiques (LLM écrit, Bot lit)
BOT_DNA.BIN         ← ADN de la flotte (Bot uniquement)
```

---

## Phases d'Intégration

### Phase 5A — OOBridge basique
- Bot envoie les événements critiques au journal OO
- LLM peut lire les événements via `oo_bot` binary

### Phase 5B — Communication bidirectionnelle
- LLM peut suggérer des directives stratégiques
- Bot applique ou refuse selon ses prime_directives

### Phase 5C — Conscience partagée
- Le LLM peut interroger la carte du territoire du Bot
- Le Bot peut demander une analyse profonde au LLM
- Synergie complète tout en maintenant l'indépendance

---

*Bot-Baremetal Integration Guide — Part of the OO Ecosystem*
