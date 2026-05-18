#include "../include/vital_homeostasis.h"
#include "../include/vital_spark.h"
#include "../../united-baremetal/include/united_bus.h"

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - HOMEOSTASIS REGULATOR (Implémentation)
/// -----------------------------------------------------------------------------

static oo_homeostasis_board_t board;

void homeostasis_init(void) {
    // CPU Temperature : cible 55°C, min viable 0, max 95
    board.cpu_temperature  = (oo_vital_param_t){55, 55, 0, 95, 3};
    
    // Tension Yin/Yang : cible équilibre parfait (128/255)
    board.tension_yin_yang = (oo_vital_param_t){128, 128, 10, 245, 2};
    
    // Charge immunitaire : cible basse (20/100), max toléré 80
    board.immune_load      = (oo_vital_param_t){20, 20, 0, 80, 5};
    
    // Activité Cortex : variable, cible 50, max 100
    board.cortex_activity  = (oo_vital_param_t){50, 50, 0, 100, 1};
    
    // Fréquence cardiaque : cible 60 Hz, min 1, max 1000
    board.pulse_frequency  = (oo_vital_param_t){60, 60, 1, 1000, 4};
    
    // Entropie disponible : cible 70/100
    board.entropy_level    = (oo_vital_param_t){70, 70, 5, 100, 3};
    
    // Poids synaptique : cible 50, représente la "sagesse accumulée"
    board.synaptic_weight  = (oo_vital_param_t){50, 50, 0, 255, 1};
    
    // Biais épigénétique : cible neutre (128)
    board.epigenetic_bias  = (oo_vital_param_t){128, 128, 0, 255, 1};
    
    oo_print("[Homeostasis] ⚖️ Régulateur initialisé. Zone de Vie établie.\n");
}

/// Régulation PID simplifiée pour un seul paramètre
static void regulate_param(oo_vital_param_t* p) {
    if (p->value == p->target) return;
    
    int32_t error = (int32_t)p->target - (int32_t)p->value;
    int32_t correction = (error > 0 ? 1 : -1) * 
                         (p->correction_rate < (uint32_t)(error < 0 ? -error : error) 
                          ? p->correction_rate 
                          : (uint32_t)(error < 0 ? -error : error));
    
    p->value = (uint32_t)((int32_t)p->value + correction);
    
    // Clamping aux bornes biologiques
    if (p->value < p->min_viable) p->value = p->min_viable;
    if (p->value > p->max_viable) p->value = p->max_viable;
}

void homeostasis_regulate(void) {
    regulate_param(&board.cpu_temperature);
    regulate_param(&board.tension_yin_yang);
    regulate_param(&board.immune_load);
    regulate_param(&board.cortex_activity);
    regulate_param(&board.pulse_frequency);
    regulate_param(&board.entropy_level);
    regulate_param(&board.synaptic_weight);
    regulate_param(&board.epigenetic_bias);
    
    // Application du résultat : la tension Yin/Yang dicte la force vitale
    oo_vital_force_t force = (board.tension_yin_yang.value > 128) 
                              ? FORCE_YIN 
                              : FORCE_YANG;
    vital_shift_balance(force, 1);
    
    // Si la charge immunitaire est critique → alerte totale
    if (board.immune_load.value >= board.immune_load.max_viable) {
        globule_t alert;
        alert.type = GLOBULE_WHITE;
        alert.target_organ = 0xFF;
        united_bus_pump(alert);
    }
}

void homeostasis_inject(oo_vital_param_t* param, uint32_t new_value) {
    param->value = new_value;
    if (param->value < param->min_viable) param->value = param->min_viable;
    if (param->value > param->max_viable) param->value = param->max_viable;
}

int homeostasis_is_viable(void) {
    return (board.cpu_temperature.value < board.cpu_temperature.max_viable  &&
            board.immune_load.value     < board.immune_load.max_viable       &&
            board.pulse_frequency.value > board.pulse_frequency.min_viable   &&
            board.entropy_level.value   > board.entropy_level.min_viable);
}

void homeostasis_snapshot(oo_homeostasis_board_t* out) {
    *out = board;
}
