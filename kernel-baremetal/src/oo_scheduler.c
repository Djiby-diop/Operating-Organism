#include "../include/oo_scheduler.h"
#include "../../united-baremetal/include/united_bus.h"
#include <stddef.h>

#define MAX_ORGANS 16

static oo_organ_task_t organ_pool[MAX_ORGANS];
static uint8_t next_task_id = 1;

static oo_organ_task_t* head_organ = NULL;
static oo_homeostasis_state_t current_state = OO_STATE_RELAXED;
static uint64_t circadian_clock = 0;
static oo_hormones_t body_hormones = {0, 0, 0};
#define CYCLE_DURATION 10000 // 10000 battements = 1 cycle jour/nuit

// Mock print function for baremetal
extern void oo_print(const char* msg);

void oo_scheduler_init(void) {
    head_organ = NULL;
    current_state = OO_STATE_RELAXED;
    for (int i = 0; i < MAX_ORGANS; i++) {
        organ_pool[i].task_id = 0; // 0 means unused
    }
    oo_print("[Kernel] Tronc Cerebral (Brainstem) initialized. State: RELAXED\n");
}

void oo_scheduler_register_organ(oo_organ_type_t type, void (*entry_point)(void)) {
    for (int i = 0; i < MAX_ORGANS; i++) {
        if (organ_pool[i].task_id == 0) {
            organ_pool[i].task_id = next_task_id++;
            organ_pool[i].type = type;
            organ_pool[i].entry_point = entry_point;
            organ_pool[i].is_sleeping = 0;
            
            // Default CPU shares
            if (type == ORGAN_TYPE_VITAL) organ_pool[i].current_cpu_share = 100;
            else organ_pool[i].current_cpu_share = 10;
            
            organ_pool[i].next = head_organ;
            head_organ = &organ_pool[i];
            
            oo_print("[Kernel] Organ registered successfully.\n");
            return;
        }
    }
    oo_print("[Kernel] CRITICAL: Organism at max capacity. Cannot register organ.\n");
}

void oo_scheduler_set_state(oo_homeostasis_state_t new_state) {
    if (current_state == new_state) return;
    current_state = new_state;
    
    oo_organ_task_t* curr = head_organ;
    
    // Dynamic biological redistribution of CPU power
    while (curr != NULL) {
        switch (new_state) {
            case OO_STATE_RELAXED:
                if (curr->type == ORGAN_TYPE_CORTEX) curr->current_cpu_share = 80;
                if (curr->type == ORGAN_TYPE_IMMUNE) curr->current_cpu_share = 10;
                curr->is_sleeping = 0;
                break;
                
            case OO_STATE_VIGILANT:
                if (curr->type == ORGAN_TYPE_CORTEX) curr->current_cpu_share = 50;
                if (curr->type == ORGAN_TYPE_IMMUNE) curr->current_cpu_share = 40;
                break;
                
            case OO_STATE_COMBAT:
                // Adrenaline rush: All power to the immune system. Cortex is suppressed.
                if (curr->type == ORGAN_TYPE_CORTEX) curr->current_cpu_share = 5; // Minimal thought
                if (curr->type == ORGAN_TYPE_IMMUNE) curr->current_cpu_share = 90;
                break;
                
            case OO_STATE_SURVIVAL:
                // Coma state: Shut down cortex to save power/regenerate.
                if (curr->type == ORGAN_TYPE_CORTEX) curr->is_sleeping = 1;
                if (curr->type == ORGAN_TYPE_IMMUNE) curr->current_cpu_share = 95;
                break;
        }
        curr = curr->next;
    }
    oo_print("[Kernel] Homeostasis state shifted. CPU resources reallocated.\n");
}

void oo_scheduler_heartbeat(void) {
    // Écoute des signaux de contrôle sur le bus sanguin
    globule_t inbox[4];
    int control_msgs = united_bus_absorb(ORGAN_TYPE_VITAL, inbox, 4);
    
    // Rythme Circadien : Alternance Jour/Nuit automatique
    circadian_clock++;
    if (circadian_clock % CYCLE_DURATION == 0) {
        if (current_state == OO_STATE_RELAXED) {
            oo_scheduler_set_state(OO_STATE_VIGILANT);
            body_hormones.melatonin = 0;
            body_hormones.adrenaline = 50; // Réveil
        } else if (current_state == OO_STATE_VIGILANT) {
            oo_scheduler_set_state(OO_STATE_RELAXED);
            body_hormones.melatonin = 200; // Endormissement
            body_hormones.adrenaline = 0;
        }
    }

    // Modulation Hormonale des priorités
    oo_organ_task_t* temp = head_organ;
    while (temp != NULL) {
        if (temp->type == ORGAN_TYPE_IMMUNE && body_hormones.adrenaline > 150) {
            temp->current_cpu_share = 90; // Mode Combat forcé par adrénaline
        }
        if (temp->type == ORGAN_TYPE_CORTEX && body_hormones.melatonin > 150) {
            temp->is_sleeping = 1; // Le Cortex dort si la mélatonine est haute
        }
        temp = temp->next;
    }

    // Battement de cœur : Exécution des tâches
    oo_organ_task_t* curr = head_organ;
    while (curr != NULL) {
        if (!curr->is_sleeping && curr->current_cpu_share > 0) {
            // Dans un vrai noyau, on ferait un saut de contexte ici.
            // Pour le "peaufinage", on simule l'activité cérébrale/immunitaire.
            if (curr->entry_point) {
                curr->entry_point();
            }
        }
        curr = curr->next;
    }
}

/* Phase 6F: Query current homeostasis state for organ_bus */
oo_homeostasis_state_t oo_scheduler_get_state(void) {
    return current_state;
}
