#include <stdint.h>
#include "../include/vital_spark.h"

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - DIGITAL BIOLUMINESCENCE (L'Aura)
/// -----------------------------------------------------------------------------
/// Traduit les etats internes (Tension, Ying/Yang) en patterns visuels.
/// L'organisme communique son "ame" par la lumiere.
/// -----------------------------------------------------------------------------

typedef struct {
    uint8_t r, g, b;
    uint8_t pulse_intensity;
} oo_aura_t;

static oo_aura_t current_aura;

void vital_update_aura(uint8_t tension, oo_vital_force_t force) {
    // 1. Calcul de la couleur de base
    if (force == FORCE_YANG) {
        // Yang = Blanc / Dore / Bleu electrique (Ordre/Energie)
        current_aura.r = tension;
        current_aura.g = tension;
        current_aura.b = 255;
    } else {
        // Yin = Violet / Indigo / Noir profond (Chaos/Reve)
        current_aura.r = 128 - (tension / 2);
        current_aura.g = 0;
        current_aura.b = tension;
    }

    // 2. Intensite du battement (Bioluminescence)
    // On simule une pulsation lumineuse calee sur le coeur d'acier
    static uint8_t sine_wave = 0;
    sine_wave += 5;
    current_aura.pulse_intensity = sine_wave;

    // 3. Application (Dans un vrai baremetal, on ecrirait dans le Framebuffer LFB)
    // oo_lfb_draw_aura(current_aura);
}

void vital_emit_distress_beacon() {
    // En cas de trauma epigenetique severe, l'aura flashe en Rouge Sang
    current_aura.r = 255; current_aura.g = 0; current_aura.b = 0;
    oo_print("[Aura] 🚨 SIGNAL DE DETRESSE BIOLOGIQUE (Aura Rouge).\n");
}
