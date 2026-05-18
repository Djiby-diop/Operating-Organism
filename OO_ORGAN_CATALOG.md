# OO Organ Catalog (Biological + Engineering Views)

This file defines the complete organ/system map for OO (Operating Organism).
It does not change any existing directory structure.

## Scope

- Human-inspired full map, but technically measurable.
- Each biological system maps to one or more software engines.
- Existing repositories are reused as they are.

## Organ/System Mapping Matrix

| Biological System | Biological Role | OO Engineering Role | Primary Engines | Existing Module Anchors |
|---|---|---|---|---|
| Nervous System (CNS/PNS) | Perception, decision, coordination | Global orchestration, signal routing, state transition control | `StrategicBrainEngine`, `SignalBusEngine`, `StateTransitionEngine` | `oo-system`, `oo-host`, `llm-baremetal` |
| Brain Cortex | Planning, reasoning | Long-horizon planning and interpretation | `PlannerEngine`, `ReasoningEngine` | `oo-host`, `oo-model`, `llm-baremetal` |
| Brainstem | Vital reflexes | Hard safety path and survival fallback | `ReflexEngine`, `SafeModeEngine` | `llm-baremetal`, `oo-dplus` |
| Spinal Cord | Fast local reflex loops | Low-latency local correction before global planning | `LocalReflexLoop`, `InterruptActionEngine` | `llm-baremetal`, `oo-sim` |
| Sensory Organs | Observe environment/body | Input ingestion and normalization | `PerceptionIngestEngine`, `SensorFusionEngine` | `llm-baremetal`, `oo-sim`, `oo-lab` |
| Cardiovascular System | Circulate oxygen/nutrients | Resource and signal circulation across modules | `FlowControlEngine`, `PriorityDispatchEngine` | `oo-system`, `oo-host` |
| Blood (RBC/WBC/Plasma) | Transport + immune patrol | Event transport + health guard + metadata transport | `EventTransport`, `ImmunePatrol`, `ContextCarrier` | `oo-host`, `oo-dplus`, `llm-baremetal` |
| Respiratory System | Oxygen intake, gas exchange | Compute pressure regulation, throughput breathing | `LoadRegulationEngine`, `ThroughputBreathingEngine` | `llm-baremetal`, `oo-host` |
| Digestive System | Transform external input to energy | Parse/transform external data into usable state | `IngestionPipelineEngine`, `NormalizationEngine` | `oo-lab`, `oo-model`, `oo-host` |
| Hepatic System (Liver) | Filter toxins, regulate chemistry | Policy filtering, data sanitation, risk scoring | `PolicyFilterEngine`, `SanitizationEngine` | `oo-dplus`, `oo-host` |
| Renal System (Kidneys) | Remove waste, preserve balance | Garbage/redundancy cleanup, artifact pruning | `WastePruningEngine`, `RetentionPolicyEngine` | `oo-host`, `oo-system` |
| Immune System | Detect and isolate threats | Integrity checks, anomaly detection, quarantine | `IntegrityGuardEngine`, `AnomalyDetectorEngine`, `QuarantineEngine` | `oo-dplus`, `llm-baremetal`, `oo-host` |
| Endocrine System | Hormonal regulation | Global mode signaling and adaptive thresholds | `ModeSignalEngine`, `AdaptiveThresholdEngine` | `oo-system`, `oo-host`, `oo-dplus` |
| Musculoskeletal System | Motion and force | Actuation pipeline and execution primitives | `ActionExecutionEngine`, `TaskMotorEngine` | `llm-baremetal`, `oo-host`, `oo-sim` |
| Integumentary System (Skin) | Boundary and protection | External boundary, trust zone ingress control | `BoundaryControlEngine`, `IngressGuardEngine` | `llm-baremetal`, `oo-dplus` |
| Lymphatic System | Fluid balance, immune transport | Backpressure balancing, distributed health propagation | `BackpressureEngine`, `HealthPropagationEngine` | `oo-host`, `oo-system` |
| Reproductive/Mutation System | Variation over time | Controlled evolution and module mutation workflow | `MutationGovernanceEngine`, `VariantEvalEngine` | `oo-dplus`, `oo-model`, `oo-lab` |
| Memory System (Short/Long) | Retention and recall | Working memory, episodic logs, long-term state | `WorkingMemoryEngine`, `PersistentMemoryEngine`, `RecallEngine` | `oo-host`, `llm-baremetal`, `oo-model` |
| Sleep/Recovery System | Consolidation and repair | Maintenance windows, repair tasks, state compaction | `RecoveryCycleEngine`, `CompactionEngine` | `oo-host`, `llm-baremetal` |
| Speech/Communication | Exchange with humans/peers | CLI/API/reporting/handoff interfaces | `DialogueEngine`, `HandoffEngine`, `ReportEngine` | `oo-system/interface`, `oo-host`, `llm-baremetal` |

## Organ Classes

- Vital Organs: brainstem, cardiovascular, respiratory, immune, memory core.
- Adaptive Organs: cortex, endocrine, digestive, mutation system.
- Interface Organs: sensory, communication, boundary.
- Maintenance Organs: renal, sleep/recovery, lymphatic.

## Minimal Vital Chain

The chain that must always remain alive:

1. `ReflexEngine` (survival immediate loop)
2. `IntegrityGuardEngine` (threat gate)
3. `StateTransitionEngine` (safe state machine)
4. `PersistentMemoryEngine` (continuity)
5. `FlowControlEngine` (resource circulation)

If non-vital organs fail, this chain keeps OO alive in degraded mode.

## Terminology Rules

- Biological naming is allowed for architecture readability.
- Engineering naming is mandatory for implementation/test references.
- Every organ description must include measurable inputs/outputs.
