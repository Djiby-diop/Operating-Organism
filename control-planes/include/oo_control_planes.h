#ifndef OO_CONTROL_PLANES_H
#define OO_CONTROL_PLANES_H

#include <stdint.h>
#include "oo_homeostasis_mode.h"

/* -----------------------------------------------------------------------------
 * OO Control Planes + Control-Tick
 * -----------------------------------------------------------------------------
 * Real (not just documented) minimal engines for the three planes described in
 * OO_CONTROL_PLANES.md, coordinated by a single control-tick:
 *   1. ReflexEngine        - hybrid reflex/safety, mandatory, runs first.
 *   2. StrategicBrainEngine- centralized strategic priorities.
 *   3. OrganAutonomyEngine - distributed local organ autonomy.
 * Priority rule: survival dominates. On conflict: policy > optimization, and an
 * uncertain decision must be reversible.
 * ---------------------------------------------------------------------------*/

/* Decision codes emitted by an engine for observability. */
typedef enum {
    OO_DECISION_NONE        = 0,
    OO_DECISION_HOLD        = 1,  /* keep current posture (reversible)        */
    OO_DECISION_THROTTLE    = 2,  /* reduce load to protect survival          */
    OO_DECISION_ISOLATE     = 3,  /* isolate a failing non-vital organ        */
    OO_DECISION_RECOVER     = 4,  /* attempt recovery of a degraded organ     */
    OO_DECISION_OPTIMIZE    = 5   /* strategic optimization (lowest priority) */
} oo_decision_t;

/* Per-plane observability record (required by OO_CONTROL_PLANES.md). */
typedef struct {
    uint8_t       active;        /* 1 if the plane ran this tick             */
    oo_decision_t last_decision; /* last decision code                       */
    uint16_t      reason_code;   /* engine-specific reason                   */
    uint8_t       risk_score;    /* 0..100, higher = riskier                 */
} oo_plane_status_t;

/* The three engines. Each returns its chosen decision and fills its status. */
oo_decision_t reflex_engine_run(oo_homeostasis_mode_t mode, uint8_t agg_health,
                                oo_plane_status_t* status);
oo_decision_t strategic_brain_run(oo_homeostasis_mode_t mode, uint8_t agg_health,
                                  oo_plane_status_t* status);
oo_decision_t organ_autonomy_run(oo_homeostasis_mode_t mode, uint8_t agg_health,
                                 oo_plane_status_t* status);

/* Coordinated control-tick: vitals -> mandatory reflexes -> strategic
 * priorities -> bounded dispatch to organs -> collect/journal. Returns the
 * resolved decision after applying priority rules. */
oo_decision_t control_tick(void);

/* Observability accessors for the last control-tick. */
const oo_plane_status_t* control_planes_reflex_status(void);
const oo_plane_status_t* control_planes_strategic_status(void);
const oo_plane_status_t* control_planes_autonomy_status(void);
uint32_t                 control_planes_tick_count(void);

#endif /* OO_CONTROL_PLANES_H */
