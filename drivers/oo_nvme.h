#pragma once

#include <stdint.h>
#include "../llm-baremetal/engine/ssm/core/symbion.h"

/**
 * NVMe Driver — High Performance Flash Storage
 */

typedef struct {
    uint64_t bar0;
    uint32_t caps_lo;
    uint32_t caps_hi;
    int      active;
} NvmeCtx;

int nvme_init(NvmeCtx *ctx, SymbionPCIDevice *pci);
int nvme_read_blocks(NvmeCtx *ctx, uint64_t lba, uint32_t count, void *buffer);
int nvme_write_blocks(NvmeCtx *ctx, uint64_t lba, uint32_t count, void *buffer);
