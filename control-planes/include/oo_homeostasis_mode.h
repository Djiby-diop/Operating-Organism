#ifndef OO_HOMEOSTASIS_MODE_H
#define OO_HOMEOSTASIS_MODE_H

#include <stdint.h>

/* -----------------------------------------------------------------------------
 * OO Homeostasis Mode FSM
 * -----------------------------------------------------------------------------
 * Implements the runtime mode machine from OO_HOMEOSTASIS_INVARIANTS.md:
 *     NORMAL -> DEGRADED -> SAFE -> RECOVERY -> NORMAL
 * Survival-first: transitions are driven by aggregate organ health and the
 * liveness of the minimal vital chain. The machine never crashes; on total
 * uncertainty it falls back to SAFE.
 * ---------------------------------------------------------------------------*/

typedef enum {
    OO_MODE_NORMAL   = 0,
    OO_MODE_DEGRADED = 1,
    OO_MODE_SAFE     = 2,
    OO_MODE_RECOVERY = 3
} oo_homeostasis_mode_t;

/* Health thresholds (aggregate health 0..100) used for transitions. */
#define OO_HEALTH_DEGRADED_BELOW  75   /* below -> at least DEGRADED */
#define OO_HEALTH_SAFE_BELOW      40   /* below -> SAFE              */
#define OO_HEALTH_RECOVER_ABOVE   85   /* above (in SAFE) -> RECOVERY*/
#define OO_HEALTH_NORMAL_ABOVE    90   /* above (in RECOVERY) -> NORMAL */

void                  homeostasis_mode_init(void);

/* Recompute the mode from current aggregate health + vital-chain liveness.
 * Returns the (possibly new) mode. Logs and broadcasts on transitions. */
oo_homeostasis_mode_t homeostasis_mode_evaluate(void);

oo_homeostasis_mode_t homeostasis_mode_current(void);

/* Human-readable, ASCII-only name of a mode. */
const char*           homeostasis_mode_name(oo_homeostasis_mode_t mode);

#endif /* OO_HOMEOSTASIS_MODE_H */
