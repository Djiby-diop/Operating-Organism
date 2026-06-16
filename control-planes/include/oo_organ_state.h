#ifndef OO_ORGAN_STATE_H
#define OO_ORGAN_STATE_H

#include <stdint.h>

/* -----------------------------------------------------------------------------
 * OO Organ State Registry
 * -----------------------------------------------------------------------------
 * Central, ASCII-only registry that every organ uses to publish a compact
 * health/mode snapshot onto the united-bus (as a YELLOW control globule).
 * The control-tick aggregates these to drive homeostasis and telemetry.
 * ---------------------------------------------------------------------------*/

/* Canonical organ identifiers (16 baremetal organs + cortex).
 * The numeric order matches the root Makefile ORGANS list so that the
 * telemetry consumer (yamaoo) can map ids to organ names deterministically. */
typedef enum {
    OO_ORGAN_UNITED        = 0,
    OO_ORGAN_KERNEL        = 1,
    OO_ORGAN_MEMORY        = 2,
    OO_ORGAN_NETWORK       = 3,
    OO_ORGAN_IDENTITY      = 4,
    OO_ORGAN_SENSE         = 5,
    OO_ORGAN_VOCAL         = 6,
    OO_ORGAN_REFLEX        = 7,
    OO_ORGAN_EVOLUTION     = 8,
    OO_ORGAN_DREAM         = 9,
    OO_ORGAN_REGEN         = 10,
    OO_ORGAN_SWARM         = 11,
    OO_ORGAN_SHADOW        = 12,
    OO_ORGAN_BOT           = 13,
    OO_ORGAN_VITAL         = 14,
    OO_ORGAN_PROPRIOCEPTION= 15,
    OO_ORGAN_CORTEX        = 16,
    OO_ORGAN_COUNT         = 17
} oo_organ_t;

/* Vital chain organs must never be reported as dead; the homeostasis FSM
 * relies on this minimal chain staying alive (survival-first). */
#define OO_VITAL_CHAIN_MASK ( \
    (1u << OO_ORGAN_REFLEX) | \
    (1u << OO_ORGAN_VITAL)  | \
    (1u << OO_ORGAN_KERNEL) | \
    (1u << OO_ORGAN_MEMORY) | \
    (1u << OO_ORGAN_UNITED) )

/* Per-organ snapshot kept in the registry. */
typedef struct {
    uint8_t  present;   /* 1 once the organ has registered at least once   */
    uint8_t  health;    /* 0..100                                          */
    uint8_t  mode;      /* see oo_homeostasis_mode_t                       */
    uint8_t  _pad;
    uint32_t heartbeat; /* increments on each organ_state_publish()        */
} oo_organ_snapshot_t;

/* Initialise the registry (idempotent). */
void organ_state_init(void);

/* An organ publishes its current health/mode. Also emits a YELLOW globule on
 * the united-bus so other planes can react. Safe to call from any organ. */
void organ_state_publish(oo_organ_t organ, uint8_t health, uint8_t mode);

/* Read back a snapshot. Returns 0 on success, -1 on invalid id. */
int organ_state_get(oo_organ_t organ, oo_organ_snapshot_t* out);

/* Aggregate health (0..100) across present organs, used by homeostasis. */
uint8_t organ_state_aggregate_health(void);

/* Worst (minimum) health among present non-vital organs (0..100). Returns 100
 * if no non-vital organ has reported yet. Lets a single organ failure escalate
 * the homeostasis mode even when the average stays high. */
uint8_t organ_state_worst_nonvital_health(void);

/* Returns 1 if the minimal vital chain is alive (health > 0), else 0. */
int organ_state_vital_chain_alive(void);

#endif /* OO_ORGAN_STATE_H */
