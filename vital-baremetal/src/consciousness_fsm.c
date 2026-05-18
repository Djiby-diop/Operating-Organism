#include "../include/vital_consciousness.h"
#include "../include/vital_spark.h"
#include "../../united-baremetal/include/united_bus.h"
#include <stddef.h>
#include <stdint.h>

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - CONSCIOUSNESS STATE MACHINE (Implémentation)
/// -----------------------------------------------------------------------------

/// Matrice légale de transition [FROM][TO]
/// 1 = Transition légale, 0 = Transition interdite
const uint8_t CONSCIOUSNESS_TRANSITION_MATRIX[9][9] = {
//  TO: VOID EMB  DOR  DRM  AWR  FOC  ALT  SUR  TRN
    {0,  1,   0,   0,   0,   0,   0,   0,   0}, // FROM VOID
    {0,  0,   1,   0,   1,   0,   0,   0,   0}, // FROM EMBRYONIC
    {0,  0,   0,   1,   1,   0,   0,   0,   0}, // FROM DORMANT
    {0,  0,   1,   0,   1,   0,   0,   0,   1}, // FROM DREAMING
    {0,  0,   1,   1,   0,   1,   1,   0,   0}, // FROM AWARE
    {0,  0,   0,   0,   1,   0,   1,   0,   1}, // FROM FOCUSED
    {0,  0,   0,   0,   1,   1,   0,   1,   0}, // FROM ALERT
    {1,  0,   0,   0,   0,   0,   1,   0,   0}, // FROM SURVIVAL
    {0,  0,   1,   1,   1,   1,   1,   1,   0}, // FROM TRANSCEND
};

/// Historique circulaire des transitions
#define HISTORY_SIZE 64
static oo_transition_event_t history[HISTORY_SIZE];
static uint32_t history_head = 0;
static uint32_t history_count = 0;

static oo_consciousness_state_t current_state = CONSCIOUSNESS_VOID;
static uint64_t state_entry_cycle = 0;
static uint64_t total_cycles = 0;

extern void oo_print(const char* msg);

void consciousness_init(void) {
    current_state = CONSCIOUSNESS_EMBRYONIC;
    state_entry_cycle = 0;
    oo_print("[Consciousness] 🧠 Conscience embryonnaire éveillée.\n");
    
    // Transition automatique vers AWARE après init
    consciousness_transition(CONSCIOUSNESS_AWARE, ORGAN_TYPE_VITAL, "Init sequence complete");
}

int consciousness_transition(oo_consciousness_state_t target,
                             uint8_t trigger_organ,
                             const char* reason) {
    uint8_t from_idx = (uint8_t)current_state;
    uint8_t to_idx   = (uint8_t)target;
    
    // Vérification de la matrice de légalité
    if (!CONSCIOUSNESS_TRANSITION_MATRIX[from_idx][to_idx]) {
        oo_print("[Consciousness] ❌ Transition refusée (Loi biologique).\n");
        return 0;
    }
    
    // Enregistrement dans l'historique
    oo_transition_event_t evt;
    evt.from           = current_state;
    evt.to             = target;
    evt.timestamp_ns   = total_cycles;
    evt.trigger_organ  = trigger_organ;
    for (int i = 0; i < 63 && reason[i]; i++) evt.reason[i] = reason[i];
    evt.reason[63] = '\0';
    
    history[history_head] = evt;
    history_head = (history_head + 1) % HISTORY_SIZE;
    if (history_count < HISTORY_SIZE) history_count++;
    
    // Émission d'un Globule Jaune pour informer tout le corps
    globule_t signal;
    signal.type         = GLOBULE_YELLOW;
    signal.source_organ = ORGAN_TYPE_VITAL;
    signal.target_organ = 0xFF; // Broadcast
    
    static oo_consciousness_state_t state_payload;
    state_payload = target;
    signal.payload_addr = &state_payload;
    signal.payload_size = sizeof(state_payload);
    united_bus_pump(signal);
    
    current_state = target;
    state_entry_cycle = total_cycles;
    oo_print("[Consciousness] ✅ Transition de conscience acceptée.\n");
    return 1;
}

oo_consciousness_state_t consciousness_get_state(void) {
    return current_state;
}

int consciousness_get_history(oo_transition_event_t* buf, int max) {
    int count = (history_count < (uint32_t)max) ? (int)history_count : max;
    for (int i = 0; i < count; i++) {
        uint32_t idx = (history_head + HISTORY_SIZE - count + i) % HISTORY_SIZE;
        buf[i] = history[idx];
    }
    return count;
}

void consciousness_tick(void) {
    total_cycles++;
    uint64_t time_in_state = total_cycles - state_entry_cycle;
    
    // Auto-transitions basées sur le temps passé dans un état
    switch (current_state) {
        case CONSCIOUSNESS_AWARE:
            // Si inactif trop longtemps -> glisse vers le rêve (Yin)
            if (time_in_state > 50000) {
                consciousness_transition(CONSCIOUSNESS_DREAMING,
                                        ORGAN_TYPE_VITAL,
                                        "Idle timeout -> Dreaming");
            }
            break;
        
        case CONSCIOUSNESS_DREAMING:
            // Les rêves ont une durée limitée
            if (time_in_state > 10000) {
                consciousness_transition(CONSCIOUSNESS_AWARE,
                                        ORGAN_TYPE_VITAL,
                                        "Dream cycle complete");
            }
            break;

        case CONSCIOUSNESS_ALERT:
            // Si l'alerte n'est pas suivie d'une escalade -> retour FOCUSED
            if (time_in_state > 5000) {
                consciousness_transition(CONSCIOUSNESS_FOCUSED,
                                        ORGAN_TYPE_VITAL,
                                        "Alert resolved autonomously");
            }
            break;

        default:
            break;
    }
}
