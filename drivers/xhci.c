#include "xhci.h"
#include <string.h>

// XHCI Registers
#define XHCI_REG_CAPLENGTH  0x00
#define XHCI_REG_HCIVERSION 0x02
#define XHCI_REG_HCSPARAMS1 0x04
#define XHCI_REG_HCCPARAMS1 0x08
#define XHCI_REG_DBOFF      0x14
#define XHCI_REG_RTSOFF      0x18

#define XHCI_OP_USBCMD      0x00
#define XHCI_OP_USBSTS      0x04
#define XHCI_OP_DNCTRL      0x14
#define XHCI_OP_CRCR        0x18
#define XHCI_OP_DCBAAP      0x30
#define XHCI_OP_CONFIG      0x38

int xhci_init(XhciCtx *ctx, SymbionPCIDevice *pci) {
    if (!ctx || !pci) return -1;

    // 1. Map MMIO Base (BAR0)
    uint32_t bar0 = oo_pci_read_config(pci->bus, pci->slot, pci->func, 0x10);
    ctx->mmio_base = bar0 & 0xFFFFFFF0;

    volatile uint8_t *base = (volatile uint8_t*)(uintptr_t)ctx->mmio_base;
    
    // 2. Read Capacity Registers
    ctx->cap_len = base[XHCI_REG_CAPLENGTH];
    ctx->op_off = ctx->cap_len;
    
    uint32_t dboff_reg = *(volatile uint32_t*)(base + XHCI_REG_DBOFF);
    ctx->db_off = dboff_reg;
    
    uint32_t rtsoff_reg = *(volatile uint32_t*)(base + XHCI_REG_RTSOFF);
    ctx->rt_off = rtsoff_reg;

    // 3. Reset the Controller
    xhci_reset(ctx);
    
    ctx->active = 1;
    _log_causal(0, "xhci_controller_ignited");
    
    return 0;
}

int xhci_reset(XhciCtx *ctx) {
    volatile uint32_t *op_regs = (volatile uint32_t*)(uintptr_t)(ctx->mmio_base + ctx->op_off);
    
    // Stop the controller first
    op_regs[XHCI_OP_USBCMD/4] &= ~(1 << 0); // RS = 0
    
    // Wait for HCHalted
    while (!(op_regs[XHCI_OP_USBSTS/4] & (1 << 0))) { /* Wait */ }
    
    // Trigger Reset
    op_regs[XHCI_OP_USBCMD/4] |= (1 << 1); // HCRST = 1
    
    // Wait for reset to complete
    while (op_regs[XHCI_OP_USBCMD/4] & (1 << 1)) { /* Wait */ }
    
    return 0;
}

void xhci_poll_events(XhciCtx *ctx) {
    if (!ctx || !ctx->active) return;
    // Event ring polling would happen here
}
