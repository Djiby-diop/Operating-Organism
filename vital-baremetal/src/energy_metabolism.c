#include "../include/vital_metabolism.h"
#include "../include/vital_nociception.h"
#include "../include/vital_consciousness.h"
#include <stdint.h>
#include <stddef.h>

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - ENERGY METABOLISM (Implémentation / Le Foie)
/// -----------------------------------------------------------------------------

#define MAX_METABOLIC_ORGANS 16

static oo_organ_metabolism_t organ_budgets[MAX_METABOLIC_ORGANS];
static uint8_t registered_organs = 0;
static oo_liver_t liver;

extern void oo_print(const char* msg);

void metabolism_init(uint64_t initial_energy_pool) {
    liver.total_energy_pool    = initial_energy_pool;
    liver.energy_generated     = initial_energy_pool;
    liver.energy_wasted        = 0;
    liver.basal_metabolic_rate = 100; // 100 calories/tick au repos
    registered_organs = 0;
    oo_print("[Metabolism] 🔋 Le Foie est opérationnel. Pool d'énergie initial chargé.\n");
}

void metabolism_register_organ(uint8_t organ_id, uint64_t budget) {
    if (registered_organs >= MAX_METABOLIC_ORGANS) return;
    
    organ_budgets[registered_organs].organ_id           = organ_id;
    organ_budgets[registered_organs].calories_consumed   = 0;
    organ_budgets[registered_organs].calories_allocated  = budget;
    organ_budgets[registered_organs].metabolic_rate       = 10;
    organ_budgets[registered_organs].is_fasting           = 0;
    registered_organs++;
}

uint64_t metabolism_request_energy(uint8_t organ_id, uint64_t amount) {
    // Recherche de l'organe
    oo_organ_metabolism_t* organ = NULL;
    for (int i = 0; i < registered_organs; i++) {
        if (organ_budgets[i].organ_id == organ_id) {
            organ = &organ_budgets[i];
            break;
        }
    }
    if (!organ) return 0;
    
    // Vérifie si l'organe est en jeûne forcé
    if (organ->is_fasting) return 0;
    
    // Vérifie le budget individuel
    uint64_t remaining_budget = organ->calories_allocated - organ->calories_consumed;
    uint64_t granted = (amount <= remaining_budget) ? amount : remaining_budget;
    
    // Vérifie le pool global
    if (granted > liver.total_energy_pool) {
        granted = liver.total_energy_pool;
    }
    
    // Transaction
    organ->calories_consumed += granted;
    liver.total_energy_pool  -= granted;
    
    // Si le budget individuel est épuisé → jeûne
    if (organ->calories_consumed >= organ->calories_allocated) {
        organ->is_fasting = 1;
        oo_print("[Metabolism] ⚠️ Organe en jeûne forcé (budget épuisé).\n");
    }
    
    return granted;
}

void metabolism_generate_energy(uint64_t amount) {
    liver.total_energy_pool += amount;
    liver.energy_generated  += amount;
}

void metabolism_tick(void) {
    // 1. Consommation basale (le coût d'être en vie)
    if (liver.total_energy_pool > liver.basal_metabolic_rate) {
        liver.total_energy_pool -= liver.basal_metabolic_rate;
        liver.energy_wasted     += liver.basal_metabolic_rate;
    } else {
        // FAMINE CRITIQUE
        liver.total_energy_pool = 0;
        nociception_fire(PAIN_SOURCE_CPU, PAIN_SEARING, 0xFADE0001);
        oo_print("[Metabolism] 🚨 FAMINE ÉNERGÉTIQUE ! Pool à zéro.\n");
    }
    
    // 2. Régénération naturelle (anabolisme pendant le repos)
    oo_consciousness_state_t state = consciousness_get_state();
    if (state == CONSCIOUSNESS_DORMANT || state == CONSCIOUSNESS_DREAMING) {
        // En sommeil, le métabolisme ralentit et le pool se régénère
        metabolism_generate_energy(liver.basal_metabolic_rate * 2);
    }
    
    // 3. Recyclage des budgets (cycle métabolique périodique)
    static uint32_t cycle_counter = 0;
    cycle_counter++;
    if (cycle_counter % 50000 == 0) {
        for (int i = 0; i < registered_organs; i++) {
            // Réinitialisation partielle des budgets (nouveau cycle catabolique)
            organ_budgets[i].calories_consumed = organ_budgets[i].calories_consumed / 2;
            organ_budgets[i].is_fasting = 0;
        }
        oo_print("[Metabolism] 🔄 Cycle catabolique : budgets partiellement recyclés.\n");
    }
    
    // 4. Adaptation du métabolisme de base
    // Si le pool est critique, on réduit la consommation basale (économie d'énergie)
    if (liver.total_energy_pool < 10000) {
        liver.basal_metabolic_rate = 50; // Mode économie
    } else {
        liver.basal_metabolic_rate = 100; // Mode normal
    }
}

int metabolism_is_starving(void) {
    return (liver.total_energy_pool < liver.basal_metabolic_rate * 10);
}

void metabolism_snapshot(oo_liver_t* out) {
    *out = liver;
}
