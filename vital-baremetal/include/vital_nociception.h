#ifndef VITAL_NOCICEPTION_H
#define VITAL_NOCICEPTION_H

#include <stdint.h>

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - NOCICEPTION (Le Système de la Douleur)
/// -----------------------------------------------------------------------------
/// La douleur n'est pas un bug, c'est un signal de survie.
/// Ce module traduit les erreurs matérielles et logicielles en "douleur"
/// proportionnelle, permettant à l'organisme de réagir avec la bonne intensité.
/// -----------------------------------------------------------------------------

/// Types de douleur que l'organisme peut ressentir
typedef enum {
    PAIN_NONE          = 0,
    PAIN_DISCOMFORT    = 1,  // Warning mineur (ex: fragmentation mémoire)
    PAIN_ACUTE         = 2,  // Erreur sérieuse (ex: page fault inattendu)
    PAIN_CHRONIC       = 3,  // Problème persistant (ex: fuite mémoire)
    PAIN_SEARING       = 4,  // Urgence matérielle (ex: surchauffe)
    PAIN_AGONY         = 5   // Défaillance critique (ex: corruption kernel)
} oo_pain_level_t;

/// Source anatomique de la douleur
typedef enum {
    PAIN_SOURCE_CPU       = 0,
    PAIN_SOURCE_MEMORY    = 1,
    PAIN_SOURCE_DISK      = 2,
    PAIN_SOURCE_NETWORK   = 3,
    PAIN_SOURCE_INTEGRITY = 4,
    PAIN_SOURCE_TEMPORAL  = 5
} oo_pain_source_t;

/// Signal de douleur complet
typedef struct {
    oo_pain_level_t  level;
    oo_pain_source_t source;
    uint64_t         timestamp;
    uint32_t         raw_error_code;
    uint8_t          endorphin_dampened; // 1 si atténué par les endorphines
} oo_pain_signal_t;

/// Récepteur de douleur (Nocicepteur)
typedef struct {
    oo_pain_source_t watches;
    uint8_t          sensitivity;   // 0=insensible, 255=hypersensible
    uint32_t         total_signals;
    oo_pain_signal_t last_signal;
} oo_nociceptor_t;

/// Initialise le système nociceptif
void nociception_init(void);

/// Envoie un signal de douleur (appelé par n'importe quel organe)
void nociception_fire(oo_pain_source_t source, oo_pain_level_t level, uint32_t error_code);

/// Retourne le niveau de douleur global actuel (somme pondérée)
uint32_t nociception_get_global_pain(void);

/// Applique des endorphines pour atténuer la douleur (effet temporaire)
void nociception_inject_endorphin(uint8_t dose);

/// Tick du système nociceptif (décroissance naturelle de la douleur)
void nociception_tick(void);

#endif // VITAL_NOCICEPTION_H
