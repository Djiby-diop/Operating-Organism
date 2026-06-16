#include "../include/nic_pci.h"
#include "../include/io_ports.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static uint32_t pci_addr(uint8_t bus, uint8_t slot, uint8_t func, uint8_t off) {
    return (uint32_t)((1u << 31) |
                      ((uint32_t)bus  << 16) |
                      ((uint32_t)slot << 11) |
                      ((uint32_t)func << 8)  |
                      (off & 0xFC));
}

uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t off) {
    outl(PCI_CONFIG_ADDRESS, pci_addr(bus, slot, func, off));
    return inl(PCI_CONFIG_DATA);
}

void pci_config_write32(uint8_t bus, uint8_t slot, uint8_t func,
                        uint8_t off, uint32_t val) {
    outl(PCI_CONFIG_ADDRESS, pci_addr(bus, slot, func, off));
    outl(PCI_CONFIG_DATA, val);
}

int pci_find(uint16_t vendor, uint16_t device, pci_dev_t* out) {
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id = pci_config_read32((uint8_t)bus, slot, func, 0x00);
                uint16_t vid = (uint16_t)(id & 0xFFFF);
                uint16_t did = (uint16_t)((id >> 16) & 0xFFFF);
                if (vid == 0xFFFF) continue; /* no device */
                if (vid == vendor && did == device) {
                    if (out) {
                        out->bus       = (uint8_t)bus;
                        out->slot      = slot;
                        out->func      = func;
                        out->vendor_id = vid;
                        out->device_id = did;
                        out->bar0 = pci_config_read32((uint8_t)bus, slot, func, 0x10);
                        out->bar1 = pci_config_read32((uint8_t)bus, slot, func, 0x14);
                    }
                    return 1;
                }
                if (func == 0) {
                    /* header type: if not multifunction, skip other funcs */
                    uint32_t hdr = pci_config_read32((uint8_t)bus, slot, 0, 0x0C);
                    if (((hdr >> 16) & 0x80) == 0) break;
                }
            }
        }
    }
    return 0;
}

void pci_enable_busmaster(const pci_dev_t* dev) {
    if (!dev) return;
    uint32_t cmd = pci_config_read32(dev->bus, dev->slot, dev->func, 0x04);
    cmd |= 0x07; /* I/O space | memory space | bus master */
    pci_config_write32(dev->bus, dev->slot, dev->func, 0x04, cmd);
}
