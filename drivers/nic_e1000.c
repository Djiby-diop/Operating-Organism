#include "nic_e1000.h"
#include <string.h>

// E1000 Registers
#define E1000_REG_CTRL      0x0000
#define E1000_REG_STATUS    0x0008
#define E1000_REG_EERD      0x0014
#define E1000_REG_ICR       0x00C0
#define E1000_REG_IMS       0x00D0
#define E1000_REG_RCTL      0x0100
#define E1000_REG_TCTL      0x0400
#define E1000_REG_RAL       0x5400
#define E1000_REG_RAH       0x5404

static inline void e1000_write(NicE1000Ctx *ctx, uint32_t reg, uint32_t val) {
    *(volatile uint32_t*)(uintptr_t)(ctx->mmio_base + reg) = val;
}

static inline uint32_t e1000_read(NicE1000Ctx *ctx, uint32_t reg) {
    return *(volatile uint32_t*)(uintptr_t)(ctx->mmio_base + reg);
}

int nic_e1000_init(NicE1000Ctx *ctx, SymbionPCIDevice *pci) {
    if (!ctx || !pci) return -1;
    
    // 1. Get MMIO Base from BAR0
    uint32_t bar0 = oo_pci_read_config(pci->bus, pci->slot, pci->func, 0x10);
    ctx->mmio_base = bar0 & 0xFFFFFFF0;

    // 2. Enable Bus Mastering and Memory Space
    uint32_t cmd = oo_pci_read_config(pci->bus, pci->slot, pci->func, 0x04);
    cmd |= (1 << 1) | (1 << 2); // Memory + Master
    // (Assuming a pci_write_config exists or we use raw ports)
    // For now we assume hardware already has this set or handled by Symbion

    // 3. Read MAC Address
    uint32_t ral = e1000_read(ctx, E1000_REG_RAL);
    uint32_t rah = e1000_read(ctx, E1000_REG_RAH);
    ctx->mac[0] = (uint8_t)(ral & 0xFF);
    ctx->mac[1] = (uint8_t)((ral >> 8) & 0xFF);
    ctx->mac[2] = (uint8_t)((ral >> 16) & 0xFF);
    ctx->mac[3] = (uint8_t)((ral >> 24) & 0xFF);
    ctx->mac[4] = (uint8_t)(rah & 0xFF);
    ctx->mac[5] = (uint8_t)((rah >> 8) & 0xFF);

    // 4. Reset and Init
    e1000_write(ctx, E1000_REG_CTRL, (1 << 26)); // Reset
    
    ctx->active = 1;
    return 0;
}

int nic_e1000_send(NicE1000Ctx *ctx, void *data, uint16_t len) {
    if (!ctx || !ctx->active) return -1;
    // Simple polling send implementation would go here
    return 0;
}

int nic_e1000_receive(NicE1000Ctx *ctx, void *buffer, uint16_t *len) {
    if (!ctx || !ctx->active) return -1;
    // Simple polling receive implementation would go here
    return 0;
}
