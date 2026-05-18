#include "hda.h"
#include <string.h>

// HDA Registers
#define HDA_REG_GCAP    0x00
#define HDA_REG_GCTL    0x08
#define HDA_REG_WAKEEN  0x0C
#define HDA_REG_STATESTS 0x0E
#define HDA_REG_INTCTL  0x20
#define HDA_REG_INTSTS  0x24
#define HDA_REG_CORBLBASE 0x40
#define HDA_REG_CORBUBASE 0x44
#define HDA_REG_RIRBLBASE 0x50

int hda_init(HdaCtx *ctx, SymbionPCIDevice *pci) {
    if (!ctx || !pci) return -1;

    // 1. Map MMIO Base (BAR0)
    uint32_t bar0 = oo_pci_read_config(pci->bus, pci->slot, pci->func, 0x10);
    ctx->mmio_base = bar0 & 0xFFFFFFF0;

    volatile uint32_t *regs = (volatile uint32_t*)(uintptr_t)ctx->mmio_base;

    // 2. Read Capabilities
    uint16_t gcap = (uint16_t)(regs[0] & 0xFFFF);
    ctx->stream_count = (gcap >> 12) & 0x0F; // ISS
    ctx->stream_count += (gcap >> 8) & 0x0F;  // OSS

    // 3. Global Reset
    regs[HDA_REG_GCTL/4] &= ~(1 << 0); // CRST = 0
    while (regs[HDA_REG_GCTL/4] & (1 << 0)) { /* Wait for reset start */ }
    
    regs[HDA_REG_GCTL/4] |= (1 << 0); // CRST = 1
    while (!(regs[HDA_REG_GCTL/4] & (1 << 0))) { /* Wait for reset complete */ }

    ctx->active = 1;
    _log_causal(0, "hda_auditory_cortex_active");
    
    return 0;
}

void hda_play_pcm(HdaCtx *ctx, void *data, uint32_t size) {
    if (!ctx || !ctx->active) return;
    // DMA Buffer setup for playback
}

void hda_record_pcm(HdaCtx *ctx, void *buffer, uint32_t size) {
    if (!ctx || !ctx->active) return;
    // DMA Buffer setup for recording
}
