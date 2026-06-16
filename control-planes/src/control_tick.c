#include "../include/oo_control_planes.h"
#include "../include/oo_organ_state.h"
#include "../include/oo_homeostasis_mode.h"
#include "../include/oo_telemetry.h"

/* network-baremetal exhale (UDP/NIC). Declared here to avoid a hard include
 * dependency on the lungs header from this module. */
extern void network_exhale(const uint8_t* data, unsigned long size);

static oo_plane_status_t g_reflex;
static oo_plane_status_t g_strategic;
static oo_plane_status_t g_autonomy;
static uint32_t          g_ticks = 0;

/* Priority rank: lower number = higher priority (survival dominates). */
static int decision_rank(oo_decision_t d) {
    switch (d) {
        case OO_DECISION_THROTTLE: return 0; /* protect survival first */
        case OO_DECISION_ISOLATE:  return 1;
        case OO_DECISION_RECOVER:  return 2;
        case OO_DECISION_HOLD:     return 3; /* reversible default      */
        case OO_DECISION_OPTIMIZE: return 4; /* lowest priority         */
        default:                   return 5;
    }
}

oo_decision_t control_tick(void) {
    g_ticks++;

    /* 1. Read vitals and update the homeostasis mode. */
    oo_homeostasis_mode_t mode = homeostasis_mode_evaluate();
    uint8_t health             = organ_state_aggregate_health();

    /* 2. Mandatory reflexes first. */
    oo_decision_t reflex = reflex_engine_run(mode, health, &g_reflex);

    /* 3. Strategic priorities. */
    oo_decision_t strat = strategic_brain_run(mode, health, &g_strategic);

    /* 4. Distributed organ autonomy. */
    oo_decision_t auton = organ_autonomy_run(mode, health, &g_autonomy);

    /* 5. Resolve by priority: survival (lowest rank) wins. The strategic plane
     *    may only win when it is at least as safe as the others. */
    oo_decision_t resolved = reflex;
    if (decision_rank(auton)  < decision_rank(resolved)) resolved = auton;
    if (decision_rank(strat)  < decision_rank(resolved)) resolved = strat;

    /* 6. Emit telemetry over the network (UDP). */
    oo_telemetry_pkt_t pkt;
    oo_telemetry_fill(&pkt);
    uint8_t buf[sizeof(oo_telemetry_pkt_t)];
    size_t  n = oo_telemetry_serialize(&pkt, buf, sizeof(buf));
    if (n > 0) {
        network_exhale(buf, (unsigned long)n);
    }

    return resolved;
}

const oo_plane_status_t* control_planes_reflex_status(void)    { return &g_reflex; }
const oo_plane_status_t* control_planes_strategic_status(void) { return &g_strategic; }
const oo_plane_status_t* control_planes_autonomy_status(void)  { return &g_autonomy; }
uint32_t                 control_planes_tick_count(void)       { return g_ticks; }
