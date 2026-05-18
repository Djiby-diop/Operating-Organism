#include "../include/nervous_system.h"
#include "../../united-baremetal/include/united_bus.h"

#define MAX_REFLEXES 256
static void (*reflex_table[MAX_REFLEXES])(void);

static uint32_t armed_count    = 0;
static uint32_t triggered_count = 0;
static uint8_t  last_vector    = 0;
static uint32_t watchdog_counter = 0;

extern void oo_print(const char* msg);
extern void cpu_halt(void);

void reflex_init(void) {
    for (int i = 0; i < MAX_REFLEXES; i++) {
        reflex_table[i] = 0;
    }
    oo_print("[ReflexBaremetal] \xe2\x9a\xa1 Moelle epiniere en ligne. Reflexes armes.\n");

    reflex_bind_action(OO_REFLEX_PAIN,     reflex_on_pain);
    reflex_bind_action(OO_REFLEX_WATCHDOG, reflex_on_thermal_burn);
    reflex_bind_action(OO_REFLEX_THERMAL,  reflex_on_thermal_burn);
    armed_count = 3;
}

void reflex_bind_action(uint8_t interrupt_vector, void (*reflex_action)(void)) {
    reflex_table[interrupt_vector] = reflex_action;
}

void reflex_trigger(uint8_t interrupt_vector) {
    if (reflex_table[interrupt_vector]) {
        last_vector = interrupt_vector;
        triggered_count++;
        reflex_table[interrupt_vector]();
    }
}

void reflex_on_thermal_burn(void) {
    oo_print("[ReflexBaremetal] \xf0\x9f\x94\xa5 SURCHAUFFE ! Throttle materiel immediat.\n");
    globule_t pain_signal;
    pain_signal.type         = GLOBULE_YELLOW;
    pain_signal.source_organ = ORGAN_REFLEX_SOURCE;
    pain_signal.target_organ = ORGAN_BROADCAST;
    pain_signal.payload_addr = 0;
    pain_signal.payload_size = OO_REFLEX_THERMAL;
    united_bus_pump(pain_signal);
}

void reflex_on_pain(void) {
    oo_print("[ReflexBaremetal] \xe2\x9a\xa1 NMI (Douleur) ! Emission alerte immune.\n");
    united_bus_broadcast_white(ORGAN_REFLEX_SOURCE, 0x01);
    cpu_halt();
}

void reflex_watchdog_kick(void) {
    watchdog_counter = 0;
}

void reflex_thermal_check(uint32_t temp_celsius) {
    if (temp_celsius > 85) {
        oo_print("[ReflexBaremetal] THERMAL CRITICAL — switching to SAFE mode.\n");
        united_bus_broadcast_white(ORGAN_REFLEX_SOURCE, OO_REFLEX_THERMAL);
        united_bus_broadcast_yellow(ORGAN_REFLEX_SOURCE, 0xA0FE0001);
        reflex_on_thermal_burn();
    }
}

reflex_status_t reflex_get_status(void) {
    reflex_status_t s;
    s.armed_count     = armed_count;
    s.triggered_count = triggered_count;
    s.last_vector     = last_vector;
    return s;
}
