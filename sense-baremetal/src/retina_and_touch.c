#include "../include/sensory_receptors.h"
#include "../../united-baremetal/include/united_bus.h"

extern void oo_print(const char* msg);

static uint8_t inb(uint16_t port) { (void)port; return 0; }

void sense_init(void) {
    oo_print("[SenseBaremetal] Recepteurs sensoriels (Toucher, Vue) calibres.\n");
}

void sense_transduce_keystroke(uint8_t scancode) {
    static oo_stimulus_t last_stimulus;
    last_stimulus.type        = STIMULUS_TOUCH;
    last_stimulus.intensity   = 100;
    last_stimulus.timestamp   = 0;
    last_stimulus.raw_data[0] = scancode;

    globule_t g;
    g.type         = GLOBULE_RED;
    g.source_organ = ORGAN_SENSORY;
    g.target_organ = ORGAN_CORTEX;
    g.payload_addr = &last_stimulus;
    g.payload_size = sizeof(oo_stimulus_t);
    united_bus_pump(g);
}

void sense_update_retina(const char* visual_buffer, uint32_t size) {
    if (!visual_buffer || size == 0) return;

    /* Bare-metal: write text to VGA text buffer at 0xB8000 (80x25 color text mode) */
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    uint8_t  attr      = 0x0F; /* white on black */
    uint32_t max_chars = 80 * 25;
    uint32_t n         = size < max_chars ? size : max_chars;

    for (uint32_t i = 0; i < n; i++) {
        vga[i] = (uint16_t)((attr << 8) | (uint8_t)visual_buffer[i]);
    }

    globule_t g;
    g.type         = GLOBULE_RED;
    g.source_organ = ORGAN_SENSORY;
    g.target_organ = ORGAN_CORTEX;
    g.payload_addr = 0;
    g.payload_size = size;
    united_bus_pump(g);
}

void sense_feel_temperature(void) {
    /* Read from MSR or platform EC — stubbed for bare-metal prototype */
    uint32_t current_temp = 45;
    if (current_temp > 95) {
        globule_t alarm;
        alarm.type         = GLOBULE_WHITE;
        alarm.source_organ = ORGAN_SENSORY;
        alarm.target_organ = 0xFF; /* broadcast */
        alarm.payload_addr = 0;
        alarm.payload_size = 0;
        united_bus_pump(alarm);
    }
}

void sense_transduce_serial(const char* data, uint32_t len) {
    static oo_stimulus_t last;
    last.type      = STIMULUS_SERIAL;
    last.intensity = 100;
    last.timestamp = 0;
    uint32_t copy  = len < 16 ? len : 16;
    for (uint32_t i = 0; i < copy; i++) last.raw_data[i] = (uint8_t)data[i];

    globule_t g;
    g.type         = GLOBULE_RED;
    g.source_organ = ORGAN_SENSORY;
    g.target_organ = ORGAN_CORTEX;
    g.payload_addr = 0;
    g.payload_size = len;
    united_bus_pump(g);
}

void sense_transduce_timer(uint64_t tick) {
    united_bus_broadcast_yellow(ORGAN_SENSORY, (uint32_t)(tick & 0xFFFFFFFF));
}

oo_stimulus_t sense_get_last_stimulus(void) {
    static oo_stimulus_t last_stimulus;
    return last_stimulus;
}
