#ifndef VITAL_METABOLISM_H
#define VITAL_METABOLISM_H

#include <stdint.h>

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - ENERGY METABOLISM
/// -----------------------------------------------------------------------------
/// Les cycles CPU sont les "calories" de l'organisme.
/// Ce module gère le budget énergétique et empêche l'organisme de mourir
/// d'épuisement ou de brûler vif par surconsommation.
/// -----------------------------------------------------------------------------

/// Budget énergétique d'un organe
typedef struct {
    uint8_t  organ_id;
    uint64_t calories_consumed;    // Cycles CPU consommés
    uint64_t calories_allocated;   // Budget maximum autorisé
    uint32_t metabolic_rate;       // Vitesse de consommation (cycles/tick)
    uint8_t  is_fasting;           // 1 = en jeûne forcé (budget épuisé)
} oo_organ_metabolism_t;

/// Le "Foie" de l'organisme (Gestionnaire d'énergie central)
typedef struct {
    uint64_t total_energy_pool;    // Pool global de calories disponibles
    uint64_t energy_generated;     // Calories produites depuis la naissance
    uint64_t energy_wasted;        // Calories perdues (chaleur, friction)
    uint32_t basal_metabolic_rate; // Consommation minimale au repos
} oo_liver_t;

/// Initialise le métabolisme
void metabolism_init(uint64_t initial_energy_pool);

/// Enregistre un organe dans le système métabolique
void metabolism_register_organ(uint8_t organ_id, uint64_t budget);

/// Un organe demande de l'énergie (retourne le nombre de calories accordées)
uint64_t metabolism_request_energy(uint8_t organ_id, uint64_t amount);

/// Génère de l'énergie (appelé quand le système est au repos — anabolisme)
void metabolism_generate_energy(uint64_t amount);

/// Tick métabolique (consommation basale, recyclage)
void metabolism_tick(void);

/// Retourne 1 si l'organisme est en famine énergétique
int metabolism_is_starving(void);

/// Snapshot du foie
void metabolism_snapshot(oo_liver_t* out);

#endif // VITAL_METABOLISM_H
