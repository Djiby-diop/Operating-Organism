#ifndef VITAL_DREAM_SIMULATOR_H
#define VITAL_DREAM_SIMULATOR_H

#include <stdint.h>

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - INTERNAL DREAM SIMULATOR
/// -----------------------------------------------------------------------------
/// Pendant la phase Yin (Repos), l'organisme simule des scénarios de menace
/// pour renforcer ses réflexes SANS danger réel. Comme les rêves humains.
/// -----------------------------------------------------------------------------

/// Type de scénario simulé
typedef enum {
    DREAM_SCENARIO_THREAT_INVASION  = 0, // Simulation d'attaque réseau
    DREAM_SCENARIO_MEMORY_FLOOD     = 1, // Simulation de saturation RAM
    DREAM_SCENARIO_CPU_THERMAL_RUN  = 2, // Simulation de surchauffe
    DREAM_SCENARIO_KERNEL_CORRUPTION= 3, // Simulation de corruption mémoire
    DREAM_SCENARIO_UNKNOWN_SIGNAL   = 4, // Signal non identifié
    DREAM_SCENARIO_REBIRTH          = 5  // Simulation de mort et renaissance
} oo_dream_scenario_t;

/// Résultat d'une simulation de rêve
typedef struct {
    oo_dream_scenario_t scenario;
    uint32_t duration_cycles;
    uint8_t survival_rate;   // Probabilité de survie simulée 0-100
    uint8_t lessons_learned; // Nombre de nouveaux réflexes générés
} oo_dream_result_t;

/// Lance un cycle de rêve interne
oo_dream_result_t dream_simulate(oo_dream_scenario_t scenario);

/// Sélectionne un scénario basé sur les marqueurs épigénétiques
oo_dream_scenario_t dream_select_scenario(uint8_t trauma_level);

/// Applique les leçons d'un rêve au système réel (Conditioned Reflex)
void dream_apply_lessons(const oo_dream_result_t* result);

#endif // VITAL_DREAM_SIMULATOR_H
