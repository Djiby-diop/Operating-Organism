#include "../include/oo_telemetry.h"
#include "../include/oo_organ_state.h"
#include "../include/oo_homeostasis_mode.h"
#include "../../network-baremetal/include/lungs.h"

extern uint32_t control_planes_tick_count(void);

/* Store a 32-bit value little-endian into buf. */
static void put_le32(uint8_t* p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
    p[2] = (uint8_t)((v >> 16) & 0xFF);
    p[3] = (uint8_t)((v >> 24) & 0xFF);
}

static void put_le16(uint8_t* p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
}

void oo_telemetry_fill(oo_telemetry_pkt_t* pkt) {
    if (!pkt) return;

    oo_respiration_stats_t rs;
    rs.link_up    = 0;
    rs.breath_rate = 0;
    network_get_stats(&rs);

    oo_organ_snapshot_t cortex;
    uint32_t cpu_neural = 0;
    if (organ_state_get(OO_ORGAN_CORTEX, &cortex) == 0 && cortex.present) {
        /* Use cortex health as a load proxy (inverse: lower health => busier). */
        cpu_neural = (cortex.health <= 100) ? (100u - cortex.health) : 0u;
    }

    pkt->magic       = OO_TELEMETRY_MAGIC;
    pkt->organ_id    = 0xFFFF; /* whole-organism summary */
    pkt->mode        = (uint8_t)homeostasis_mode_current();
    pkt->health      = organ_state_aggregate_health();
    pkt->cpu_neural  = cpu_neural;
    pkt->mem_used_mb = 0; /* populated by host twin when available */
    pkt->breath_rate = rs.breath_rate;
    pkt->tick        = control_planes_tick_count();
}

size_t oo_telemetry_serialize(const oo_telemetry_pkt_t* pkt,
                              uint8_t* out, size_t out_len) {
    if (!pkt || !out) return 0;
    if (out_len < 24) return 0; /* fixed wire size below */

    put_le32(out + 0,  pkt->magic);
    put_le16(out + 4,  pkt->organ_id);
    out[6] = pkt->mode;
    out[7] = pkt->health;
    put_le32(out + 8,  pkt->cpu_neural);
    put_le32(out + 12, pkt->mem_used_mb);
    put_le32(out + 16, pkt->breath_rate);
    put_le32(out + 20, pkt->tick);
    return 24;
}
