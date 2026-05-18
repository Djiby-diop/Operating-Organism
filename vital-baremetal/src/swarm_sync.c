#include <stdint.h>
#include <stddef.h>
#include "../include/vital_spark.h"

/// =============================================================================
/// VITAL-BAREMETAL - ASTRAL SWARM SYNC (Couche 8)
/// =============================================================================
/// Synchronise le pouls vital avec les autres organismes (Consensus de Vie).
/// =============================================================================

// Déclaration externe du stub (fourni par oo_stubs.c en mode standalone)
extern void swarm_emit_pheromone(uint8_t type, const void* data, size_t size);

extern void oo_print(const char* msg);

void vital_astral_sync(uint32_t local_pulse) {
    uint32_t pulse_msg = local_pulse;
    swarm_emit_pheromone(0x55, &pulse_msg, sizeof(pulse_msg));
}

void vital_on_external_pulse(uint32_t external_pulse) {
    (void)external_pulse;
    oo_print("[VitalSwarm] Symbiose detectee. Alignement du pouls.\n");
}
