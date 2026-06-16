#include "../include/oo_control_planes.h"

/* -----------------------------------------------------------------------------
 * StrategicBrainEngine - centralized strategic plane.
 * Runs after reflexes. It only proposes optimization/recovery when survival is
 * not at stake. Policy (survival) always overrides optimization.
 * ---------------------------------------------------------------------------*/

#define STRAT_IDLE      0u
#define STRAT_OPTIMIZE  1u
#define STRAT_RECOVER   2u
#define STRAT_DEFER     3u

oo_decision_t strategic_brain_run(oo_homeostasis_mode_t mode, uint8_t agg_health,
                                  oo_plane_status_t* status) {
    oo_decision_t decision = OO_DECISION_HOLD;
    uint16_t      reason    = STRAT_IDLE;
    uint8_t       risk      = 0;

    switch (mode) {
        case OO_MODE_NORMAL:
            /* Plenty of headroom: pursue optimization (lowest priority). */
            decision = (agg_health >= OO_HEALTH_NORMAL_ABOVE)
                         ? OO_DECISION_OPTIMIZE : OO_DECISION_HOLD;
            reason   = (decision == OO_DECISION_OPTIMIZE) ? STRAT_OPTIMIZE : STRAT_IDLE;
            risk     = 10;
            break;
        case OO_MODE_RECOVERY:
            decision = OO_DECISION_RECOVER;
            reason   = STRAT_RECOVER;
            risk     = 30;
            break;
        case OO_MODE_DEGRADED:
        case OO_MODE_SAFE:
        default:
            /* Survival regime: strategy defers to the reflex plane. */
            decision = OO_DECISION_HOLD;
            reason   = STRAT_DEFER;
            risk     = 50;
            break;
    }

    if (status) {
        status->active        = 1;
        status->last_decision = decision;
        status->reason_code   = reason;
        status->risk_score    = risk;
    }
    return decision;
}
