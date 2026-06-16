#ifndef OO_NIC_H
#define OO_NIC_H

#include <stdint.h>
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * OO NIC abstraction
 * -----------------------------------------------------------------------------
 * Minimal, ASCII-only network interface used by the UDP stack. Two backends are
 * provided: virtio-net (QEMU) and Intel e1000 (real PC). The active backend is
 * selected at init time by PCI probing; virtio is preferred under QEMU, e1000
 * is the fallback for real hardware.
 * ---------------------------------------------------------------------------*/

typedef enum {
    NIC_NONE   = 0,
    NIC_VIRTIO = 1,
    NIC_E1000  = 2
} nic_kind_t;

typedef struct {
    nic_kind_t kind;
    uint8_t    mac[6];
    uint8_t    link_up;
    uint16_t   pci_bus;
    uint16_t   pci_slot;
    uint32_t   io_base;   /* PIO base (e1000 may use MMIO bar0)             */
    uint64_t   mmio_base; /* MMIO base address                             */
} nic_device_t;

/* Probe PCI and initialise the first supported NIC. Returns the active kind
 * (NIC_NONE if no NIC found). Safe to call when no hardware exists. */
nic_kind_t nic_init(void);

/* Transmit a raw L2 frame (Ethernet). Returns 0 on success, -1 otherwise.
 * A no-op returning -1 when no NIC is active (caller falls back to UART). */
int nic_transmit(const uint8_t* frame, size_t len);

/* Copy the active NIC's MAC into out[6]. Returns 0 on success. */
int nic_get_mac(uint8_t out[6]);

/* Active device descriptor (read-only). */
const nic_device_t* nic_active(void);

#endif /* OO_NIC_H */
