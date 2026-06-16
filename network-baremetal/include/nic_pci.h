#ifndef OO_NIC_PCI_H
#define OO_NIC_PCI_H

#include <stdint.h>

/* Minimal PCI configuration-space access (legacy 0xCF8/0xCFC mechanism). */

typedef struct {
    uint8_t  bus;
    uint8_t  slot;
    uint8_t  func;
    uint16_t vendor_id;
    uint16_t device_id;
    uint32_t bar0;
    uint32_t bar1;
} pci_dev_t;

uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t off);
void     pci_config_write32(uint8_t bus, uint8_t slot, uint8_t func,
                            uint8_t off, uint32_t val);

/* Scan PCI for a device matching vendor/device. Returns 1 and fills out on
 * match, 0 otherwise. */
int pci_find(uint16_t vendor, uint16_t device, pci_dev_t* out);

/* Enable bus-mastering + I/O/memory space for a device (command register). */
void pci_enable_busmaster(const pci_dev_t* dev);

#endif /* OO_NIC_PCI_H */
