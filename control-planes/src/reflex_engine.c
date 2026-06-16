#include "../include/oo_control_planes.h"
#include "../include/oo_organ_state.h"

/* -----------------------------------------------------------------------------
 * ReflexEngine - hybrid reflex/safety plane (mandatory, highest priority).
 * Fires before any strategic reasoning. Its job is pure survival: detect a
 * threshold breach and emit an immediate, reversible protective decision.
 * ---------------------------------------------------------------------------*/

/* Reason codes for observability. */
#define REFLEX_OK              0u
#define REFLEX_VITAL_BREACH    1u
#define REFLEX_HEALTH_CRITICAL 2u
#define REFLEX_HEALTH_LOW      3u

oo_decision_t reflex_engine_run(oo_homeostasis_mode_t mode, uint8_t agg_health,
                                oo_plane_status_t* status) {
    oo_decision_t decision = OO_DECISION_HOLD;
    uint16_t      reason    = REFLEX_OK;
    uint8_t       risk      = 0;

    (void)mode;

    if (!organ_state_vital_chain_alive()) {
        /* Survival dominates: throttle everything to protect the vital chain. */
        decision = OO_DECISION_THROTTLE;
        reason   = REFLEX_VITAL_BREACH;
        risk     = 100;
    } else if (agg_health < OO_HEALTH_SAFE_BELOW) {
        decision = OO_DECISION_THROTTLE;
        reason   = REFLEX_HEALTH_CRITICAL;
        risk     = 90;
    } else if (agg_health < OO_HEALTH_DEGRADED_BELOW) {
        decision = OO_DECISION_ISOLATE; /* isolate a failing non-vital organ */
        reason   = REFLEX_HEALTH_LOW;
        risk     = 60;
    }

    if (status) {
        status->active        = 1;
        status->last_decision = decision;
        status->reason_code   = reason;
        status->risk_score    = risk;
    }
    return decision;
}
