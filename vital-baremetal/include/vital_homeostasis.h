#ifndef VITAL_HOMEOSTASIS_H
#define VITAL_HOMEOSTASIS_H

#include <stdint.h>

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - HOMEOSTASIS REGULATOR
/// -----------------------------------------------------------------------------
/// Maintient tous les paramètres vitaux dans leur "Zone de Vie".
/// Si un paramètre dévie, le régulateur applique une correction proportionnelle.
/// -----------------------------------------------------------------------------

/// Un paramètre vital avec ses bornes biologiques
typedef struct {
    uint32_t value;        // Valeur actuelle
    uint32_t target;       // Valeur idéale (Homéostat)
    uint32_t min_viable;   // En dessous = mort organique
    uint32_t max_viable;   // Au dessus = nécrose thermique
    uint8_t  correction_rate; // Vitesse de rappel vers la cible
} oo_vital_param_t;

/// Le tableau de bord homéostatique complet de l'organisme
typedef struct {
    oo_vital_param_t cpu_temperature;  // Température CPU simulée
    oo_vital_param_t tension_yin_yang; // Balance 0=Yang pur, 255=Yin pur
    oo_vital_param_t immune_load;      // Charge du système immunitaire 0-100
    oo_vital_param_t cortex_activity;  // Activité LLM 0-100
    oo_vital_param_t pulse_frequency;  // Fréquence cardiaque (Hz)
    oo_vital_param_t entropy_level;    // Niveau d'entropie disponible 0-100
    oo_vital_param_t synaptic_weight;  // Poids des connexions appris
    oo_vital_param_t epigenetic_bias;  // Biais de personnalité 0-255
} oo_homeostasis_board_t;

/// Initialise le régulateur homéostatique
void homeostasis_init(void);

/// Tick de régulation (appelé depuis la boucle éternelle)
void homeostasis_regulate(void);

/// Injecte une valeur externe dans un paramètre vital
void homeostasis_inject(oo_vital_param_t* param, uint32_t new_value);

/// Retourne 1 si l'organisme est dans la "Zone de Vie"
int homeostasis_is_viable(void);

/// Snapshot de l'état homéostatique complet
void homeostasis_snapshot(oo_homeostasis_board_t* out);

#endif // VITAL_HOMEOSTASIS_H
