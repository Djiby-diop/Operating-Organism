#include "../include/oo_control_planes.h"
#include "../include/oo_organ_state.h"

/* -----------------------------------------------------------------------------
 * OrganAutonomyEngine - distributed plane.
 * Each non-vital organ can locally self-heal/throttle within bounds. This
 * engine scans the registry and proposes the most urgent local action without
 * touching the vital chain (those are reflex-only).
 * ---------------------------------------------------------------------------*/

#define AUTON_IDLE        0u
#define AUTON_SELFHEAL    1u
#define AUTON_LOCAL_THROT 2u

oo_decision_t organ_autonomy_run(oo_homeostasis_mode_t mode, uint8_t agg_health,
                                 oo_plane_status_t* status) {
    (void)mode;
    (void)agg_health;

    oo_decision_t decision = OO_DECISION_HOLD;
    uint16_t      reason    = AUTON_IDLE;
    uint8_t       risk      = 0;
    uint8_t       worst     = 100;

    for (int i = 0; i < OO_ORGAN_COUNT; i++) {
        if (i == OO_ORGAN_UNITED || i == OO_ORGAN_KERNEL ||
            i == OO_ORGAN_MEMORY || i == OO_ORGAN_REFLEX ||
            i == OO_ORGAN_VITAL) {
            continue; /* vital chain is reflex-only */
        }
        oo_organ_snapshot_t s;
        if (organ_state_get((oo_organ_t)i, &s) == 0 && s.present) {
            if (s.health < worst) worst = s.health;
        }
    }

    if (worst < OO_HEALTH_SAFE_BELOW) {
        decision = OO_DECISION_RECOVER; /* attempt local self-heal */
        reason   = AUTON_SELFHEAL;
        risk     = 40;
    } else if (worst < OO_HEALTH_DEGRADED_BELOW) {
        decision = OO_DECISION_THROTTLE; /* local backpressure */
        reason   = AUTON_LOCAL_THROT;
        risk     = 25;
    }

    if (status) {
        status->active        = 1;
        status->last_decision = decision;
        status->reason_code   = reason;
        status->risk_score    = risk;
    }
    return decision;
}
