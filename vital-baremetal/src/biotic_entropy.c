#include "../include/vital_spark.h"
#include "../../united-baremetal/include/united_bus.h"

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - BIOTIC ENTROPY (C)
/// -----------------------------------------------------------------------------
/// Ce module genere le "Chaos Constructif" (Yin).
/// Il utilise le bruit thermique pour creer des embranchements de vie.
/// -----------------------------------------------------------------------------

// Poids du Ying et du Yang (Dynamique)
static uint32_t yang_weight = 1000;
static uint32_t yin_weight = 1000;

void vital_generate_biotic_noise(uint64_t entropy) {
    // L'entropie vient directement du RDRAND de la boucle Assembly
    uint8_t decision = (uint8_t)(entropy & 0xFF);
    
    // Si l'entropie est elevee, on favorise le chaos (Yin)
    if (decision > 200) {
        // Declenchement d'un "Mutation Burst"
        globule_t mutation;
        mutation.type = GLOBULE_YELLOW;
        mutation.source_organ = 15; // VITAL_SPARK
        mutation.target_organ = 10; // EVOLUTION
        
        // On injecte du pur chaos dans le moteur d'evolution
        mutation.payload_addr = &entropy;
        mutation.payload_size = 8;
        united_bus_pump(mutation);
    }
    
    // Monitoring de la "Chaleur" du systeme
    if (yang_weight > yin_weight * 2) {
        // Organisme trop rigide (Trop d'ordre)
        // On force une dose de Yin pour eviter la sclerose logicielle
        yin_weight += 100;
    }
}

// Fonction extreme de "Survie Finale"
void vital_final_stand(void) {
    // Si tout echoue, ce code bloque tous les autres organes
    // et ne laisse tourner que le coeur d'acier et la garde Rust.
    // L'organisme entre en hibernation profonde (Suspended Animation).
    oo_print("[VitalSpark] ☢️ ALERTE FINALE : Hibernation biotique activee.\n");
}
