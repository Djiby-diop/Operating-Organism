#include "../include/oo_homeostasis_mode.h"
#include "../include/oo_organ_state.h"
#include "../../united-baremetal/include/united_bus.h"

extern void oo_print(const char* msg);

static oo_homeostasis_mode_t g_mode = OO_MODE_NORMAL;

void homeostasis_mode_init(void) {
    g_mode = OO_MODE_NORMAL;
}

const char* homeostasis_mode_name(oo_homeostasis_mode_t mode) {
    switch (mode) {
        case OO_MODE_NORMAL:   return "NORMAL";
        case OO_MODE_DEGRADED: return "DEGRADED";
        case OO_MODE_SAFE:     return "SAFE";
        case OO_MODE_RECOVERY: return "RECOVERY";
        default:               return "UNKNOWN";
    }
}

oo_homeostasis_mode_t homeostasis_mode_current(void) {
    return g_mode;
}

oo_homeostasis_mode_t homeostasis_mode_evaluate(void) {
    uint8_t agg    = organ_state_aggregate_health();
    uint8_t worst  = organ_state_worst_nonvital_health();
    int     vital  = organ_state_vital_chain_alive();

    /* Effective health blends the average with the worst single organ so that
     * one failing non-vital organ still escalates the mode (survival-first). */
    uint8_t health = (worst < agg) ? worst : agg;

    oo_homeostasis_mode_t next = g_mode;

    /* Survival-first: a broken vital chain forces SAFE immediately,
     * regardless of the current mode. */
    if (!vital) {
        next = OO_MODE_SAFE;
    } else {
        switch (g_mode) {
            case OO_MODE_NORMAL:
                if (health < OO_HEALTH_SAFE_BELOW)          next = OO_MODE_SAFE;
                else if (health < OO_HEALTH_DEGRADED_BELOW) next = OO_MODE_DEGRADED;
                break;
            case OO_MODE_DEGRADED:
                if (health < OO_HEALTH_SAFE_BELOW)          next = OO_MODE_SAFE;
                else if (health >= OO_HEALTH_NORMAL_ABOVE)  next = OO_MODE_NORMAL;
                break;
            case OO_MODE_SAFE:
                /* Only leave SAFE through RECOVERY, and only once healthy. */
                if (health >= OO_HEALTH_RECOVER_ABOVE)      next = OO_MODE_RECOVERY;
                break;
            case OO_MODE_RECOVERY:
                if (health < OO_HEALTH_SAFE_BELOW)          next = OO_MODE_SAFE;
                else if (health >= OO_HEALTH_NORMAL_ABOVE)  next = OO_MODE_NORMAL;
                break;
            default:
                next = OO_MODE_SAFE; /* uncertain: fall back to SAFE */
                break;
        }
    }

    if (next != g_mode) {
        oo_print("[Homeostasis] mode transition: ");
        oo_print(homeostasis_mode_name(g_mode));
        oo_print(" -> ");
        oo_print(homeostasis_mode_name(next));
        oo_print("\n");
        /* Broadcast a YELLOW control signal carrying the new mode. */
        united_bus_broadcast_yellow((uint8_t)OO_ORGAN_VITAL,
                                    0x4D000000u | (uint32_t)next);
        g_mode = next;
    }

    return g_mode;
}
