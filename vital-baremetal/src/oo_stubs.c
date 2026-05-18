#include <stdint.h>
#include <stddef.h>
#include "united_bus.h"

/// =============================================================================
/// VITAL-BAREMETAL - STUBS (Bouchons pour compilation standalone)
/// =============================================================================
/// Remplace les fonctions des autres organes et des modules Rust/Asm
/// quand vital-baremetal est compilé seul pour test QEMU.
/// En mode OO complet, ce fichier est exclu du link.
/// =============================================================================

// --- RUST GUARDIAN (stubs) ---

static uint64_t stub_vitality = 100;

uint64_t get_organism_vitality(void) {
    return stub_vitality++;
}

uint8_t get_guardian_state(void) {
    return 0; // HEALTHY
}

uint64_t get_total_anomalies(void) {
    return 0;
}

uint8_t get_monitored_cores(void) {
    return 1;
}

// --- QUANTUM VAULT (stub) ---
void vital_quantum_sign(uint64_t entropy) {
    (void)entropy;
}

// --- TEMPORAL ENGINE (stub) ---
uint64_t vital_temporal_get_jitter(void) {
    return 0;
}

// --- SWARM (stub) ---
void swarm_emit_pheromone(uint8_t type, const void* data, size_t size) {
    (void)type; (void)data; (void)size;
}

// --- IDENTITY (stub) ---
int identity_is_self(const void* sig) {
    (void)sig;
    return 1;
}
