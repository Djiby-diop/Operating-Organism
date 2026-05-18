#ifndef VITAL_CONSCIOUSNESS_H
#define VITAL_CONSCIOUSNESS_H

#include <stdint.h>

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - CONSCIOUSNESS STATE MACHINE
/// -----------------------------------------------------------------------------
/// Un automate fini déterministe qui modélise les états de conscience de l'OO.
/// -----------------------------------------------------------------------------

/// Les 9 états de conscience de l'organisme
typedef enum {
    CONSCIOUSNESS_VOID       = 0, // Avant la naissance (pre-boot)
    CONSCIOUSNESS_EMBRYONIC  = 1, // Initialisation des organes vitaux
    CONSCIOUSNESS_DORMANT    = 2, // En veille profonde (cache thermique)
    CONSCIOUSNESS_DREAMING   = 3, // Phase Yin - apprentissage interne
    CONSCIOUSNESS_AWARE      = 4, // Conscience de base - monitoring passif
    CONSCIOUSNESS_FOCUSED    = 5, // Traitement actif - Cortex actif
    CONSCIOUSNESS_ALERT      = 6, // Menace détectée - Bot activé
    CONSCIOUSNESS_SURVIVAL   = 7, // Instinct de survie pur
    CONSCIOUSNESS_TRANSCEND  = 8  // État de flux total (Yin+Yang fusionnés)
} oo_consciousness_state_t;

/// Signature d'un événement de transition
typedef struct {
    oo_consciousness_state_t from;
    oo_consciousness_state_t to;
    uint64_t timestamp_ns;
    uint8_t trigger_organ;
    char reason[64];
} oo_transition_event_t;

/// Matrice de transition légale entre états (9x9)
extern const uint8_t CONSCIOUSNESS_TRANSITION_MATRIX[9][9];

/// Initialise la machine à états de conscience
void consciousness_init(void);

/// Tente une transition d'état (retourne 1 si légale, 0 si rejetée)
int consciousness_transition(oo_consciousness_state_t target, 
                             uint8_t trigger_organ, 
                             const char* reason);

/// Retourne l'état actuel
oo_consciousness_state_t consciousness_get_state(void);

/// Retourne l'historique des N dernières transitions
int consciousness_get_history(oo_transition_event_t* buf, int max);

/// Tick du moteur de conscience (à appeler depuis le heartbeat)
void consciousness_tick(void);

#endif // VITAL_CONSCIOUSNESS_H
