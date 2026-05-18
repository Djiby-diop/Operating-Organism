#ifndef OO_SCHEDULER_H
#define OO_SCHEDULER_H

#include <stdint.h>

/// États homéostatiques de l'Operating Organism
typedef enum {
    OO_STATE_RELAXED = 0,    // Le LLM (Cortex) a 90% du CPU (Génération, Rêve, Apprentissage)
    OO_STATE_VIGILANT = 1,   // Le Bot et le LLM se partagent le CPU (50/50)
    OO_STATE_COMBAT = 2,     // Le Bot (Instinct/Immunité) a 90% du CPU. Le LLM est suspendu ou réduit.
    OO_STATE_SURVIVAL = 3    // Le Kernel tue tous les process non-vitaux pour régénérer le Bot.
} oo_homeostasis_state_t;

/// Niveaux hormonaux de l'organisme (0-255)
typedef struct {
    uint8_t adrenaline; // Augmente la priorité du Bot/Reflex
    uint8_t melatonin;  // Induit le sommeil (Dream/Evolution)
    uint8_t endorphin;  // Réduit la douleur (ignore les erreurs mineures)
} oo_hormones_t;

/// Types d'organes (Threads/Processus)
typedef enum {
    ORGAN_TYPE_CORTEX = 0,   // LLM, Diop
    ORGAN_TYPE_IMMUNE = 1,   // Bot-Baremetal, Watchers
    ORGAN_TYPE_SENSORY = 2,  // Sense-Baremetal, Drivers
    ORGAN_TYPE_VITAL = 3     // Kernel lui-même
} oo_organ_type_t;

/// Structure d'un composant biologique (Process Control Block)
typedef struct oo_organ_task {
    uint32_t task_id;
    oo_organ_type_t type;
    void (*entry_point)(void);
    uint8_t current_cpu_share; // 0-100%
    uint8_t is_sleeping;
    struct oo_organ_task* next;
} oo_organ_task_t;

// Initialise le Tronc Cérébral (Scheduler)
void oo_scheduler_init(void);

// Enregistre un nouvel organe dans le corps
void oo_scheduler_register_organ(oo_organ_type_t type, void (*entry_point)(void));

// Change l'état hormonal/menace de l'organisme (appelé par bot-baremetal)
void oo_scheduler_set_state(oo_homeostasis_state_t new_state);

// Le "battement de coeur" appelé par le timer (PIT/APIC)
void oo_scheduler_heartbeat(void);

// Phase 6F: Query current homeostasis state (used by organ_bus)
oo_homeostasis_state_t oo_scheduler_get_state(void);

#endif // OO_SCHEDULER_H
