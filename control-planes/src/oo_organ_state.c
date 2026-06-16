#include "../include/oo_organ_state.h"
#include "../../united-baremetal/include/united_bus.h"

extern void oo_print(const char* msg);

static oo_organ_snapshot_t g_registry[OO_ORGAN_COUNT];
static int                 g_inited = 0;

void organ_state_init(void) {
    if (g_inited) return;
    for (int i = 0; i < OO_ORGAN_COUNT; i++) {
        g_registry[i].present   = 0;
        g_registry[i].health    = 0;
        g_registry[i].mode      = 0;
        g_registry[i]._pad      = 0;
        g_registry[i].heartbeat = 0;
    }
    g_inited = 1;
}

void organ_state_publish(oo_organ_t organ, uint8_t health, uint8_t mode) {
    if (!g_inited) organ_state_init();
    if ((int)organ < 0 || (int)organ >= OO_ORGAN_COUNT) return;

    if (health > 100) health = 100;

    oo_organ_snapshot_t* s = &g_registry[organ];
    s->present = 1;
    s->health  = health;
    s->mode    = mode;
    s->heartbeat++;

    /* Announce the new state on the bus as a YELLOW control globule so that
     * the reflex/strategic/autonomy planes can subscribe and react. The
     * payload encodes (organ<<16 | mode<<8 | health) in the signal code. */
    uint32_t signal = ((uint32_t)organ << 16) |
                      ((uint32_t)mode   << 8)  |
                      (uint32_t)health;
    united_bus_broadcast_yellow((uint8_t)organ, signal);
}

int organ_state_get(oo_organ_t organ, oo_organ_snapshot_t* out) {
    if (!out) return -1;
    if ((int)organ < 0 || (int)organ >= OO_ORGAN_COUNT) return -1;
    *out = g_registry[organ];
    return 0;
}

uint8_t organ_state_aggregate_health(void) {
    if (!g_inited) organ_state_init();
    uint32_t sum = 0;
    uint32_t n   = 0;
    for (int i = 0; i < OO_ORGAN_COUNT; i++) {
        if (g_registry[i].present) {
            sum += g_registry[i].health;
            n++;
        }
    }
    if (n == 0) return 100; /* nothing reported yet: assume healthy boot */
    return (uint8_t)(sum / n);
}

uint8_t organ_state_worst_nonvital_health(void) {
    if (!g_inited) organ_state_init();
    uint8_t worst = 100;
    for (int i = 0; i < OO_ORGAN_COUNT; i++) {
        if (i == OO_ORGAN_UNITED || i == OO_ORGAN_KERNEL ||
            i == OO_ORGAN_MEMORY || i == OO_ORGAN_REFLEX ||
            i == OO_ORGAN_VITAL) {
            continue; /* vital chain handled separately */
        }
        if (g_registry[i].present && g_registry[i].health < worst) {
            worst = g_registry[i].health;
        }
    }
    return worst;
}

int organ_state_vital_chain_alive(void) {
    if (!g_inited) organ_state_init();
    static const oo_organ_t chain[] = {
        OO_ORGAN_UNITED, OO_ORGAN_KERNEL, OO_ORGAN_MEMORY,
        OO_ORGAN_REFLEX, OO_ORGAN_VITAL
    };
    for (unsigned i = 0; i < sizeof(chain) / sizeof(chain[0]); i++) {
        const oo_organ_snapshot_t* s = &g_registry[chain[i]];
        /* A vital organ that has reported and is at zero health breaks the
         * chain. Organs that never reported are treated as not-yet-failed. */
        if (s->present && s->health == 0) return 0;
    }
    return 1;
}
