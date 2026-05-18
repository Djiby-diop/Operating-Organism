#include "../include/sensory_receptors.h"
#include "../../united-baremetal/include/united_bus.h"

extern void oo_print(const char* msg);

// Mock function representing raw hardware port read in x86
static uint8_t inb(uint16_t port) { return 0; }

void sense_init(void) {
    oo_print("[SenseBaremetal] 👁️ Recepteurs sensoriels (Toucher, Vue) calibres.\n");
    // Configuration du contrôleur clavier 8042, etc.
}

// Transduction : Convertir un signal physique (électrique) en information biologique (Globule)
void sense_transduce_keystroke(uint8_t scancode) {
    oo_stimulus_t stimulus;
    stimulus.type = STIMULUS_TOUCH;
    stimulus.intensity = 100;
    stimulus.timestamp = 0; // Mock timestamp
    stimulus.raw_data[0] = scancode;
    
    // Émettre un Globule Rouge contenant le stimulus pour le Cortex
    globule_t red_cell;
    red_cell.type = GLOBULE_RED;
    red_cell.source_organ = 2; // ORGAN_TYPE_SENSORY
    red_cell.target_organ = 0; // ORGAN_TYPE_CORTEX (LLM)
    
    // On copie le stimulus dans le pool de l'organe (simulé ici)
    static oo_stimulus_t last_stimulus;
    last_stimulus = stimulus;
    
    red_cell.payload_addr = &last_stimulus;
    red_cell.payload_size = sizeof(oo_stimulus_t);
    
    united_bus_pump(red_cell);
}

void sense_feel_temperature(void) {
    oo_print("[Sense] Thermoception: temperature checked (stub).\n");
}

void sense_transduce_serial(const char* data, uint32_t len) {
    static oo_stimulus_t last;
    last.type = STIMULUS_SERIAL;
    last.intensity = 100;
    last.timestamp = 0;
    uint32_t copy = len < 16 ? len : 16;
    for (uint32_t i = 0; i < copy; i++) last.raw_data[i] = (uint8_t)data[i];

    globule_t g;
    g.type         = GLOBULE_RED;
    g.source_organ = ORGAN_SENSORY;
    g.target_organ = ORGAN_CORTEX;
    g.payload_addr = 0;  /* bare-metal: no heap ptr, use bus copy semantics */
    g.payload_size = len;
    united_bus_pump(g);
}

void sense_transduce_timer(uint64_t tick) {
    (void)tick;
    /* Broadcast heartbeat tick as YELLOW to all organs */
    united_bus_broadcast_yellow(ORGAN_SENSORY, (uint32_t)(tick & 0xFFFFFFFF));
}

oo_stimulus_t sense_get_last_stimulus(void) {
    static oo_stimulus_t last_stimulus;
    return last_stimulus;
}
    // Lecture imaginaire du registre MSR thermique (Model-Specific Register)
    uint32_t current_temp = 45; // Degrés Celsius
    
    if (current_temp > 95) {
        // Envoi d'un influx nerveux d'urgence directemet vers la moelle épinière (Reflex-Baremetal)
        // Mais en biologie, le reflex intercepte directement l'IRQ. 
        // Si le sense détecte l'élévation, il alerte le système immunitaire.
        globule_t fever_signal;
        fever_signal.type = GLOBULE_WHITE; // C'est une menace vitale
        fever_signal.target_organ = 0xFF;  // Broadcast (Kernel, Reflex, Bot)
        united_bus_pump(fever_signal);
    }
}
