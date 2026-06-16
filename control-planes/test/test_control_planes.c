/* Native host test for the OO control planes + homeostasis FSM.
 * Builds the control-planes sources with lightweight stubs for the bus,
 * logging and the network, then asserts the survival-first behaviour. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "../include/oo_organ_state.h"
#include "../include/oo_homeostasis_mode.h"
#include "../include/oo_control_planes.h"
#include "../include/oo_telemetry.h"
#include "../../network-baremetal/include/lungs.h"

/* ---- stubs expected by the control-planes objects ---- */
void oo_print(const char* msg) { (void)msg; }

int united_bus_broadcast_yellow(uint8_t source, uint32_t signal_code) {
    (void)source; (void)signal_code; return 0;
}

static int g_exhale_calls = 0;
static size_t g_last_exhale_len = 0;
void network_exhale(const uint8_t* data, size_t size) {
    (void)data; g_exhale_calls++; g_last_exhale_len = size;
}
void network_get_stats(oo_respiration_stats_t* stats) {
    if (stats) { stats->link_up = 1; stats->breath_rate = 7; }
}

static int failures = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } \
    else         { printf("PASS: %s\n", msg); } } while (0)

int main(void) {
    organ_state_init();
    homeostasis_mode_init();

    /* All healthy: should stay NORMAL. */
    for (int i = 0; i < OO_ORGAN_COUNT; i++)
        organ_state_publish((oo_organ_t)i, 100, 0);
    CHECK(homeostasis_mode_evaluate() == OO_MODE_NORMAL, "all healthy -> NORMAL");

    /* A non-vital organ degrades below the degraded threshold -> DEGRADED. */
    organ_state_publish(OO_ORGAN_SWARM, 10, 0);
    oo_homeostasis_mode_t m = homeostasis_mode_evaluate();
    CHECK(m == OO_MODE_DEGRADED || m == OO_MODE_SAFE,
          "non-vital failure -> at least DEGRADED");

    /* Reflex must fire (ISOLATE/THROTTLE) before strategy optimizes. */
    oo_plane_status_t rs, ss;
    uint8_t h = organ_state_aggregate_health();
    oo_decision_t rdec = reflex_engine_run(m, h, &rs);
    oo_decision_t sdec = strategic_brain_run(m, h, &ss);
    CHECK(rs.active && rdec != OO_DECISION_OPTIMIZE,
          "reflex active and non-optimizing under degradation");
    CHECK(sdec != OO_DECISION_OPTIMIZE,
          "strategy defers (no optimize) under degradation");

    /* Break the vital chain -> SAFE immediately (survival-first). */
    organ_state_publish(OO_ORGAN_KERNEL, 0, 0);
    CHECK(homeostasis_mode_evaluate() == OO_MODE_SAFE,
          "vital chain breach -> SAFE");

    /* control_tick resolves to a survival decision and emits telemetry. */
    oo_decision_t resolved = control_tick();
    CHECK(resolved == OO_DECISION_THROTTLE,
          "control_tick resolves to THROTTLE on vital breach");
    CHECK(g_exhale_calls > 0 && g_last_exhale_len == 24,
          "control_tick emitted a 24-byte telemetry frame");

    /* Telemetry serialization round-trip sanity. */
    oo_telemetry_pkt_t pkt; uint8_t buf[24];
    oo_telemetry_fill(&pkt);
    size_t n = oo_telemetry_serialize(&pkt, buf, sizeof(buf));
    uint32_t magic = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) |
                     ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
    CHECK(n == 24 && magic == OO_TELEMETRY_MAGIC,
          "telemetry serialized with correct magic");

    printf("\n%s (%d failures)\n", failures ? "TEST SUITE FAILED" : "ALL TESTS PASSED", failures);
    return failures ? 1 : 0;
}
