#ifndef OO_TELEMETRY_H
#define OO_TELEMETRY_H

#include <stdint.h>
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * OO Telemetry Packet
 * -----------------------------------------------------------------------------
 * Compact, fixed-layout frame emitted periodically by the control-tick and sent
 * over UDP by network-baremetal towards yamaoo. All fields little-endian; the
 * struct is packed so the wire layout is stable across compilers.
 * ---------------------------------------------------------------------------*/

#define OO_TELEMETRY_MAGIC 0x4F4F5431u /* "OOT1" */

#if defined(__GNUC__)
#define OO_PACKED __attribute__((packed))
#else
#define OO_PACKED
#endif

typedef struct OO_PACKED {
    uint32_t magic;       /* OO_TELEMETRY_MAGIC                              */
    uint16_t organ_id;    /* originating organ (oo_organ_t), 0xFFFF=whole org*/
    uint8_t  mode;        /* oo_homeostasis_mode_t                           */
    uint8_t  health;      /* 0..100 aggregate or per-organ                   */
    uint32_t cpu_neural;  /* cortex load proxy 0..100                        */
    uint32_t mem_used_mb;
    uint32_t breath_rate; /* network_get_stats breath rate                  */
    uint32_t tick;        /* control-tick counter                           */
} oo_telemetry_pkt_t;

/* Fill a telemetry packet from the current organism state. */
void oo_telemetry_fill(oo_telemetry_pkt_t* pkt);

/* Serialise pkt into out (must be >= sizeof(oo_telemetry_pkt_t)).
 * Returns number of bytes written, or 0 on bad args. */
size_t oo_telemetry_serialize(const oo_telemetry_pkt_t* pkt,
                              uint8_t* out, size_t out_len);

#endif /* OO_TELEMETRY_H */
