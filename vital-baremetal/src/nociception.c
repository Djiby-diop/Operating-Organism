#include "../include/vital_nociception.h"
#include "../include/vital_consciousness.h"
#include "../include/vital_spark.h"
#include "../../united-baremetal/include/united_bus.h"
#include <stdint.h>

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - NOCICEPTION SYSTEM (Implémentation)
/// -----------------------------------------------------------------------------

#define MAX_NOCICEPTORS 6

static oo_nociceptor_t nociceptors[MAX_NOCICEPTORS];
static uint32_t global_pain_accumulator = 0;
static uint8_t  endorphin_level = 0;
static uint64_t pain_tick_counter = 0;

extern void oo_print(const char* msg);
extern void vital_mark_experience(int8_t impact);

/// Poids de chaque type de douleur (multiplicateur)
static const uint8_t PAIN_WEIGHT[] = {0, 1, 5, 10, 30, 100};

void nociception_init(void) {
    for (int i = 0; i < MAX_NOCICEPTORS; i++) {
        nociceptors[i].watches      = (oo_pain_source_t)i;
        nociceptors[i].sensitivity  = 128; // Sensibilité neutre
        nociceptors[i].total_signals = 0;
        nociceptors[i].last_signal.level = PAIN_NONE;
    }
    global_pain_accumulator = 0;
    endorphin_level = 0;
    oo_print("[Nociception] 🩺 Récepteurs de douleur calibrés.\n");
}

void nociception_fire(oo_pain_source_t source, oo_pain_level_t level, uint32_t error_code) {
    if (source >= MAX_NOCICEPTORS) return;
    
    oo_nociceptor_t* receptor = &nociceptors[(int)source];
    
    // Construction du signal de douleur
    oo_pain_signal_t signal;
    signal.level          = level;
    signal.source         = source;
    signal.timestamp      = pain_tick_counter;
    signal.raw_error_code = error_code;
    signal.endorphin_dampened = 0;
    
    // Application de la sensibilité du récepteur
    uint32_t effective_pain = (uint32_t)PAIN_WEIGHT[(int)level] * receptor->sensitivity / 128;
    
    // Atténuation par les endorphines
    if (endorphin_level > 0) {
        uint32_t damping = effective_pain * endorphin_level / 255;
        effective_pain = (effective_pain > damping) ? effective_pain - damping : 0;
        signal.endorphin_dampened = 1;
    }
    
    // Accumulation
    global_pain_accumulator += effective_pain;
    receptor->total_signals++;
    receptor->last_signal = signal;
    
    // Réaction automatique selon l'intensité
    if (level >= PAIN_SEARING) {
        // Douleur aiguë → Transition de conscience vers ALERT ou SURVIVAL
        consciousness_transition(CONSCIOUSNESS_ALERT, ORGAN_TYPE_VITAL, "Acute pain detected");
        
        // Injection dans le flux sanguin (Globule Blanc d'urgence)
        globule_t pain_globule;
        pain_globule.type         = GLOBULE_WHITE;
        pain_globule.source_organ = 15; // VITAL_SPARK
        pain_globule.target_organ = 0xFF; // Broadcast
        pain_globule.payload_addr = &signal;
        pain_globule.payload_size = sizeof(signal);
        united_bus_pump(pain_globule);
        
        oo_print("[Nociception] 🔴 DOULEUR AIGUË ! Signal d'urgence émis.\n");
    }
    
    if (level >= PAIN_AGONY) {
        // Douleur agonisante → bascule vers SURVIVAL + endorphines automatiques
        consciousness_transition(CONSCIOUSNESS_SURVIVAL, ORGAN_TYPE_VITAL, "Agony threshold breached");
        nociception_inject_endorphin(200); // Auto-médication d'urgence
        
        // Marquage épigénétique (traumatisme profond)
        vital_mark_experience(-5);
        oo_print("[Nociception] ⚫ AGONIE. Endorphines d'urgence libérées.\n");
    }
    
    // Adaptation de la sensibilité (Habituation vs Sensibilisation)
    if (level <= PAIN_DISCOMFORT && receptor->total_signals > 100) {
        // Habituation : douleur mineure répétée → le récepteur s'engourdit
        if (receptor->sensitivity > 30) receptor->sensitivity--;
    } else if (level >= PAIN_ACUTE) {
        // Sensibilisation : douleur forte → le récepteur devient plus sensible
        if (receptor->sensitivity < 250) receptor->sensitivity += 2;
    }
}

uint32_t nociception_get_global_pain(void) {
    return global_pain_accumulator;
}

void nociception_inject_endorphin(uint8_t dose) {
    uint16_t total = (uint16_t)endorphin_level + dose;
    endorphin_level = (total > 255) ? 255 : (uint8_t)total;
}

void nociception_tick(void) {
    pain_tick_counter++;
    
    // Décroissance naturelle de la douleur (le corps "guérit")
    if (global_pain_accumulator > 0) {
        global_pain_accumulator = global_pain_accumulator * 254 / 255; // ~0.4% de décroissance par tick
    }
    
    // Métabolisation des endorphines
    if (endorphin_level > 0) {
        endorphin_level--;
    }
    
    // Douleur chronique : si l'accumulation ne redescend pas → marquage épigénétique
    if (global_pain_accumulator > 5000 && pain_tick_counter % 10000 == 0) {
        vital_mark_experience(-1);
        oo_print("[Nociception] ⚠️ Douleur chronique détectée. Marquage épigénétique.\n");
    }
}
