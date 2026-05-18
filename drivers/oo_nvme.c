#include "oo_nvme.h"
#include <string.h>

// NVMe Registers (simplified)
#define NVME_REG_CAP    0x00
#define NVME_REG_VS     0x08
#define NVME_REG_INTMS  0x0C
#define NVME_REG_CC     0x14
#define NVME_REG_CSTS   0x1C
#define NVME_REG_AQA    0x24
#define NVME_REG_ASQ    0x28
#define NVME_REG_ACQ    0x30

int nvme_init(NvmeCtx *ctx, SymbionPCIDevice *pci) {
    if (!ctx || !pci) return -1;
    
    // 1. Map BAR0
    ctx->bar0 = (uint64_t)oo_pci_read_config(pci->bus, pci->slot, pci->func, 0x10);
    // (Add support for 64-bit BARs here)
    
    volatile uint32_t *regs = (volatile uint32_t*)ctx->bar0;
    
    // 2. Check Controller Capabilities
    ctx->caps_lo = regs[0];
    ctx->caps_hi = regs[1];
    
    // 3. Reset Controller
    regs[5] &= ~(1 << 0); // CC.EN = 0
    while (regs[7] & (1 << 0)) { /* Wait for CSTS.RDY to clear */ }
    
    // 4. Setup Admin Queues (Placeholder)
    // In a real implementation, we would allocate physical memory for ASQ and ACQ
    
    ctx->active = 1;
    return 0;
}

int nvme_read_blocks(NvmeCtx *ctx, uint64_t lba, uint32_t count, void *buffer) {
    if (!ctx || !ctx->active) return -1;
    // NVMe Submission Queue entry creation goes here
    return 0;
}

int nvme_write_blocks(NvmeCtx *ctx, uint64_t lba, uint32_t count, void *buffer) {
    if (!ctx || !ctx->active) return -1;
    // NVMe Submission Queue entry creation goes here
    return 0;
}
