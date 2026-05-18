#include <stdint.h>
#include <stddef.h>
#include "united_bus.h"

#ifdef VITAL_STANDALONE
// --- UNITED BUS (Stub pour mode standalone) ---
#define STUB_BLOOD_VOLUME 64
static globule_t stub_blood[STUB_BLOOD_VOLUME];
static uint32_t stub_systole = 0;
static uint32_t stub_diastole = 0;

void united_bus_init(void) {}

int united_bus_pump(globule_t globule) {
    uint32_t next = (stub_systole + 1) % STUB_BLOOD_VOLUME;
    if (next == stub_diastole) return -1;
    stub_blood[stub_systole] = globule;
    stub_systole = next;
    return 0;
}

int united_bus_absorb(uint8_t organ_id, globule_t* out_buffer, int max_globules) {
    (void)organ_id;
    int absorbed = 0;
    while (stub_diastole != stub_systole && absorbed < max_globules) {
        out_buffer[absorbed] = stub_blood[stub_diastole];
        stub_diastole = (stub_diastole + 1) % STUB_BLOOD_VOLUME;
        absorbed++;
    }
    return absorbed;
}
#endif
