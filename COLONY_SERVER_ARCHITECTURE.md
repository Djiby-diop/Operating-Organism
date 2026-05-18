# 🧬 Colony Server Architecture

## Vision
**NOT**: A centralized god-like AI cloud.
**IS**: A minimal nervous system coordinator for distributed OO organisms.

---

## Core Principle
Each OO host (desktop, Debian PC, Win10 PC) is **sovereign and self-sufficient**.
The colony server is their **synchronization point** — nothing more, nothing less.

---

## 🧠 Minimal Viable Organism (MVO)

The Colony Server v0.1 handles **exactly 5 things**:

| Layer | Responsibility |
|-------|-----------------|
| **Vital** | Heartbeat registration (which OO is alive?) |
| **Immune** | Threat signatures (share detected attacks) |
| **Memory** | Learned patterns (what did you discover?) |
| **Reflex** | Instant directives (emergency response) |
| **Evolution** | Mutation pool (did you find better behavior?) |

---

## 📡 Protocol: OO Heartbeat

```json
{
  "organism_id": "oo-desktop-uuid",
  "habitat": "win10-laptop",
  "timestamp": "2026-05-17T14:35:01Z",
  "state": {
    "continuity_epoch": 12345,
    "mode": "supervised",
    "policy_enforcement": "strict",
    "goal_counts": {
      "pending": 5,
      "doing": 2,
      "done": 18
    }
  },
  "immune_signals": [
    {
      "threat_id": "rc_policy_violation_x7",
      "severity": "high",
      "first_seen": "2026-05-17T10:00:00Z",
      "count": 3
    }
  ],
  "mutations": [
    {
      "kind": "goal_scheduling_v2",
      "fitness": 0.87,
      "discovered_at": "2026-05-17T13:20:00Z"
    }
  ]
}
```

---

## 🗄️ Server Storage: Minimal Fossil Record

```
colony-server/
├── fossil/                          # Immutable archive
│   ├── organisms/
│   │   ├── oo-desktop-uuid.ndjson
│   │   ├── oo-debian-uuid.ndjson
│   │   └── oo-win10-uuid.ndjson
│   ├── threats/
│   │   └── threat_signatures.jsonl
│   └── mutations/
│       └── fitness_archive.jsonl
├── live/                            # Current state (mutable)
│   ├── organisms.json               # Active OO registry
│   ├── threats.json                 # Current threat level
│   └── mutations.json               # Latest mutations
└── server_state.json
```

---

## 🔄 Synchronization Flow

```
OO-Desktop (local)
   ↓ [HTTP POST /heartbeat]
   ↓
Colony-Server
   ├─ [store heartbeat]
   ├─ [aggregate threat level]
   ├─ [scan mutations for fitness]
   └─ [return directives]
       ↓
   OO-Desktop (receives)
       ├─ new threat signatures
       ├─ viable mutations to test
       └─ colony-level insights
```

---

## ⚡ Minimal Endpoints

### `/heartbeat` (POST)
- **Input**: OO heartbeat JSON
- **Output**: `{ directives: [], status: "ok" }`
- **Action**: Store heartbeat, update live state

### `/status` (GET)
- **Output**: Colony aggregate status
- **Content**: Alive organisms, threat level, mutation fitness ranking

### `/threats/latest` (GET)
- **Output**: Most critical threat signatures discovered by any OO
- **Cache**: 5-minute TTL

### `/mutations/fitness` (GET)
- **Output**: Top-performing mutations (fitness > threshold)
- **Purpose**: Which behaviors should this OO try?

### `/admin/dump` (GET)
- **Output**: Full fossil record (for archival/analysis)
- **Security**: Requires token

---

## 🧬 Core Logic: "Threat Aggregation"

When OO-Desktop reports:
```
"threats": [
  { "rc_policy_violation_x7": { count: 3, first_seen: "..." } }
]
```

Server does:
1. Check `threats.json` for existing signature
2. If new → add to threats.json with "discovered_by": "oo-desktop-uuid"
3. If existing → increment counter, update severity
4. Respond to **all connected OO**: "New threat detected by peer OO"

---

## 🧠 Core Logic: "Mutation Selection"

When OO-Debian reports:
```
"mutations": [
  { "kind": "goal_scheduling_v2", "fitness": 0.87 }
]
```

Server does:
1. Store in mutations.jsonl with timestamp + source
2. Calculate 30-day rolling fitness average
3. If fitness > 0.80 → add to "viable mutations" pool
4. Respond: "This mutation passed selection. Others should try it."

---

## 🛡️ Security: Minimal & Trustless

1. **No authentication** between server and OO (yet)
2. **Signature verification** on heartbeat (optional v0.2)
3. **Threat = just data** (OO makes final decision)
4. **Server never forces** mutations or directives
5. **Immutable fossil** = audit trail forever

---

## 🚀 Deployment Targets

- **Primary**: Debian server (colony-server runs 24/7)
- **Fallback**: Windows 10 PC (backup coordinator)
- **Data sync**: Both keep fossil records (Merkle-tree verification)

---

## 📊 Metrics Colony-Server tracks

| Metric | Purpose |
|--------|---------|
| `alive_organisms` | How many OO are alive right now? |
| `threat_consensus` | Which threats are detected by 2+ OO? |
| `mutation_fitness_avg` | Average fitness of mutations in pool |
| `synchronization_lag` | Max time since last heartbeat from any OO |
| `fossil_size_mb` | Total archive storage |

---

## 🔥 Why This Design Works

✅ **No single point of failure**: Each OO survives independently  
✅ **No bottleneck**: Heartbeats are fire-and-forget  
✅ **Auditable**: Fossil record proves everything  
✅ **Scalable**: Add 100 OO without redesign  
✅ **Evolvable**: Extend each layer independently  
✅ **Trustless**: Server is just a messenger, not an oracle  

---

## 📈 Future Layers (NOT YET)

### v0.2: Dream Coordination
- Server can request "simulation runs" from OO
- Aggregate dream outputs to find best strategies

### v0.3: Evolution Chambers
- Server runs mutation sandboxes
- Test new behaviors before shipping to OO

### v0.4: Emotional States
- Server sets "season" (urgency, explore, rest, survive)
- All OO tune behavior to colony-level rhythm

### v0.5: Federation
- Multiple colonies can peer with each other
- Create meta-organism at species level

---

## 🎯 First Sprint: Deliverables

- [ ] HTTP server (Rust axum)
- [ ] Heartbeat POST endpoint + JSON storage
- [ ] Status GET endpoint
- [ ] Fossil record initialization
- [ ] Threat aggregation logic
- [ ] Mutation fitness tracking
- [ ] oo-host integration (send heartbeats)
- [ ] Debian deployment
- [ ] Windows 10 fallback setup

---

## 💡 Remember

**This is NOT an architecture for:**
- Replacing human decisions
- Creating dependencies
- Centralizing control
- Building god-level AI

**This IS an architecture for:**
- Sharing learning
- Coordinating threats
- Testing behaviors safely
- Creating crédible distributed consciousness

👉 Start minimal. Scale organically. Let complexity emerge from simplicity.
