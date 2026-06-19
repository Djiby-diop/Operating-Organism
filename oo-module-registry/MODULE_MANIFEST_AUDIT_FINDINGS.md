# Module Manifest Audit Findings
**Date:** 2026-06-19  
**Status:** Critical discrepancies identified

## Summary
The current `MODULE_MANIFEST.json` does not match the actual filesystem. 15 of 16 organs have naming mismatches, and several critical files are missing or undocumented.

## Critical Issues

### 1. Duplicate Entry (vocal-baremetal)
- **Manifest IDs 9 and 14 both reference vocal-baremetal**
- **Action:** Keep ID 9, remove ID 14

### 2. Naming Convention Mismatch
The manifest uses `oo_*` pattern but actual code uses biological naming:
- `kernel-baremetal`: Uses `nervous_system.h` (not `oo_kernel.h`)
- `reflex-baremetal`: Uses `spinal_cord.c` (not `oo_reflex.c`)
- `memory-baremetal`: Uses `bio_alloc.c`, `hedged_malloc.c` (not `oo_memory.c`)
- `network-baremetal`: Uses `lungs.h`, `respiration.c` (not `oo_network.h`)
- `sense-baremetal`: Uses `sensory_receptors.h` (not `oo_sense.h`)
- `vocal-baremetal`: Uses `speaker_vocal.c` (not `oo_vocal.c`)
- And 8 more...

**Decision:** Update manifest to match actual filenames (biological metaphor is intentional per design).

### 3. vital-baremetal: 7 Headers, 30+ Implementations
Manifest lists 1 header (`oo_vital.h`) and 1 implementation (`oo_vital.c`).  
**Reality:** 7 separate headers:
- `vital_consciousness.h`
- `vital_dream_sim.h`
- `vital_homeostasis.h`
- `vital_metabolism.h`
- `vital_nociception.h`
- `vital_spark.h`
- `vital_synapse.h`

Plus 30+ implementation files (.c, .rs, .asm, .o artifacts).

**Decision:** Expand vital-baremetal to accurate listing (or mark as "complex subsystem").

### 4. network-baremetal: Missing Drivers
Manifest lists:
- `nic_e1000.c` ❌ NOT FOUND
- `nic_virtio.c` ❌ NOT FOUND  
- `udp_stack.c` ❌ NOT FOUND

Only actual files:
- `nic_pci.c`
- `respiration.c`

**Decision:** Either find missing drivers or update manifest to reflect what exists.

### 5. bot-baremetal: Consolidated vs Modular
Manifest expects 4 separate headers + 5 implementations.  
**Reality:** Consolidated into:
- `bot_baremetal.h`
- `bot_baremetal_entry.c`

**Decision:** Confirm if consolidation was intentional; update manifest accordingly.

### 6. llm-baremetal: Complex Subsystem, Not Simple Module
Manifest shows `oo_llm.h` / `oo_llm.c`.  
**Reality:** Entire OS-G subsystem with:
- 20+ source files
- Multiple directories (core/, engine/, models/, diop/, tests/)
- EFI boot integration
- Inference logic
- Logging and sentinel systems

**Decision:** Either simplify manifest to facade pattern or acknowledge this as "subsystem, not module".

## Organs by Status

| Organ | Manifest Match | Actual Files | Status |
|-------|---|---|---|
| united-baremetal | ✅ Perfect | `united_bus.*` | COMPLETE |
| kernel-baremetal | ❌ Name mismatch | `nervous_system.*` | COMPLETE |
| memory-baremetal | ❌ Major mismatch | `bio_*.c`, `hedged_*.c` | COMPLETE |
| reflex-baremetal | ❌ Name mismatch | `spinal_cord.*` | COMPLETE |
| vital-baremetal | ❌ CRITICAL (1 → 7+30) | 7 headers, 30+ files | VASTLY MORE COMPLEX |
| network-baremetal | ❌ Missing drivers | Partial (2/4) | INCOMPLETE |
| bot-baremetal | ❌ Consolidated | Single entry point | SIMPLIFIED |
| sense-baremetal | ❌ Name mismatch | `sensory_receptors.*` | COMPLETE |
| proprioception-baremetal | ⚠️ Minor mismatch | `proprioception_stub.c` | STUB (as documented) |
| vocal-baremetal | ❌ Duplicate + mismatch | `speaker_vocal.c` | PARTIAL |
| dream-baremetal | ❌ Name mismatch | `dream_daemon.c` | COMPLETE |
| evolution-baremetal | ❌ Name mismatch | `mutation.c` | COMPLETE |
| shadow-baremetal | ❌ Name mismatch | `anti_forensics.c` | COMPLETE |
| identity-baremetal | ❌ Name mismatch | `self_recognition.c` | COMPLETE |
| swarm-baremetal | ❌ Name mismatch | `collective.c` | COMPLETE |
| llm-baremetal | ❌ Facade mismatch | OS-G subsystem | SUBSYSTEM |

## Next Actions

**Phase 3A (Immediate):**
1. [ ] Fix duplicate vocal-baremetal entry
2. [ ] Update all file names to match biological naming convention
3. [ ] Expand vital-baremetal to full listing
4. [ ] Audit network-baremetal: find or remove missing drivers
5. [ ] Confirm bot-baremetal consolidation is intentional

**Phase 3B (Validation):**
1. [ ] Rebuild all organs with corrected manifest
2. [ ] Run symbol validation: `nm -u *.o`
3. [ ] Verify CI smoke passes

**Phase 3C (Documentation):**
1. [ ] Add note to ARCHITECTURE.md: "Manifest lists biological filenames"
2. [ ] Document why organs expanded beyond manifest baseline
3. [ ] Clarify llm-baremetal as OS-G subsystem (not just cortex module)

