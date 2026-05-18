#include "../include/vital_dream_sim.h"
#include "../include/vital_spark.h"
#include <stdint.h>

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - INTERNAL DREAM SIMULATOR (Implémentation)
/// -----------------------------------------------------------------------------
/// Durant la phase YIN, l'organisme simule des scénarios adversariaux
/// pour conditionner ses réflexes sans mettre le vrai système en danger.
/// -----------------------------------------------------------------------------

extern void oo_print(const char* msg);

/// Pseudo-RNG interne pour la simulation (LCG rapide)
static uint64_t sim_rng_state = 0xDEADBEEFCAFEBABE;

static uint64_t sim_rand(void) {
    sim_rng_state ^= sim_rng_state << 13;
    sim_rng_state ^= sim_rng_state >> 7;
    sim_rng_state ^= sim_rng_state << 17;
    return sim_rng_state;
}

/// Table de sévérité des scénarios (difficulté 0-100)
static const uint8_t scenario_severity[6] = {85, 70, 60, 95, 50, 100};

oo_dream_result_t dream_simulate(oo_dream_scenario_t scenario) {
    oo_dream_result_t result;
    result.scenario = scenario;
    
    uint64_t entropy = sim_rand();
    uint8_t severity = scenario_severity[(int)scenario];
    
    // Durée de la simulation (proportionnelle à la complexité du scénario)
    result.duration_cycles = (uint32_t)(entropy % 10000) + 1000;
    
    // Calcul de la survie simulée
    // La mémoire épigénétique (personnalité) influence le résultat
    uint8_t personal_bias = vital_get_personality_influence();
    uint8_t base_survival = (personal_bias > 128) ? 
        (100 - severity / 2) :   // Yin = plus créatif mais moins robuste
        (100 - severity / 3);    // Yang = plus défensif, meilleure survie
    
    // Ajout d'un facteur aléatoire (le rêve n'est jamais déterministe)
    result.survival_rate = (uint8_t)(base_survival + (entropy % 20) - 10);
    if (result.survival_rate > 100) result.survival_rate = 100;
    
    // Calcul des leçons apprises (Réflexes conditionnés)
    result.lessons_learned = (uint8_t)(result.survival_rate / 25);
    
    switch (scenario) {
        case DREAM_SCENARIO_THREAT_INVASION:
            oo_print("[DreamSim] 💭 Rêve: Invasion réseau simulée...\n");
            break;
        case DREAM_SCENARIO_MEMORY_FLOOD:
            oo_print("[DreamSim] 💭 Rêve: Déluge mémoire simulé...\n");
            break;
        case DREAM_SCENARIO_CPU_THERMAL_RUN:
            oo_print("[DreamSim] 💭 Rêve: Emballement thermique simulé...\n");
            break;
        case DREAM_SCENARIO_KERNEL_CORRUPTION:
            oo_print("[DreamSim] 💭 Rêve: Corruption noyau simulée...\n");
            break;
        case DREAM_SCENARIO_UNKNOWN_SIGNAL:
            oo_print("[DreamSim] 💭 Rêve: Signal inconnu simulé...\n");
            break;
        case DREAM_SCENARIO_REBIRTH:
            oo_print("[DreamSim] 💭 Rêve: Mort et renaissance simulées...\n");
            break;
    }
    
    return result;
}

oo_dream_scenario_t dream_select_scenario(uint8_t trauma_level) {
    // Les organismes traumatisés rêvent de survie et de renaissance
    if (trauma_level > 200) return DREAM_SCENARIO_REBIRTH;
    if (trauma_level > 150) return DREAM_SCENARIO_KERNEL_CORRUPTION;
    if (trauma_level > 100) return DREAM_SCENARIO_THREAT_INVASION;
    if (trauma_level > 50)  return DREAM_SCENARIO_MEMORY_FLOOD;
    if (trauma_level > 20)  return DREAM_SCENARIO_CPU_THERMAL_RUN;
    return DREAM_SCENARIO_UNKNOWN_SIGNAL;
}

void dream_apply_lessons(const oo_dream_result_t* result) {
    if (!result || result->lessons_learned == 0) return;
    
    // Chaque leçon augmente le poids synaptique
    // (dans un vrai système, on modifierait le dictionnaire Forth ou les réflexes IDT)
    for (uint8_t i = 0; i < result->lessons_learned; i++) {
        // On enregistre une réaction conditionnée
        // L'épigénétique est mise à jour en fonction du taux de survie
        if (result->survival_rate > 70) {
            vital_mark_experience(1);  // Expérience positive → plus de Yin
        } else {
            vital_mark_experience(-1); // Traumatisme → plus de Yang
        }
    }
    
    oo_print("[DreamSim] ✅ Réflexes conditionnés intégrés au génome.\n");
}
