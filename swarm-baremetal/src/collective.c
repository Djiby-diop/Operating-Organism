#include "../include/pheromones.h"
#include "../../united-baremetal/include/united_bus.h"
#include "../../network-baremetal/include/lungs.h"
#include "../../identity-baremetal/include/dna_hash.h"

extern void oo_print(const char* msg);

void swarm_init(void) {
    oo_print("[SwarmBaremetal] 👥 Intelligence collective active. Recherche de pairs...\n");
}

void swarm_emit_pheromone(uint8_t type, const void* data, size_t size) {
    oo_print("[SwarmBaremetal] 📢 Emission de pheromones vers l'essaim.\n");
    // On utilise les poumons (Network) pour diffuser l'information
    network_exhale(data, size);
}

void swarm_on_pheromone_received(const uint8_t* sender_dna, const void* data, size_t size) {
    oo_print("[SwarmBaremetal] 📥 Pheromone recue d'un pair.\n");
    
    // Validation de l'identite du pair (Est-ce un membre de l'essaim de confiance ?)
    if (identity_is_self((const oo_dna_signature_t*)sender_dna)) {
        oo_print("[SwarmBaremetal] ✅ Pair reconnu. Absorption de l'anticorps.\n");
        
        globule_t shared_immunity;
        shared_immunity.type = GLOBULE_WHITE;
        shared_immunity.source_organ = 10; // ORGAN_TYPE_SWARM
        shared_immunity.target_organ = 1;  // Vers le Système Immunitaire (Bot)
        shared_immunity.payload_addr = (void*)data;
        shared_immunity.payload_size = (uint32_t)size;
        
        united_bus_pump(shared_immunity);
    } else {
        oo_print("[SwarmBaremetal] ⚠️ Pair inconnu ou ADN altere ! Pheromone rejetee.\n");
    }
}
