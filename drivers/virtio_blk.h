#pragma once

#include <stdint.h>
#include "../llm-baremetal/engine/ssm/core/symbion.h"

/**
 * VirtIO-Blk Driver — High Speed Virtualized Storage
 */

typedef struct {
    uint32_t mmio_base;
    uint32_t capacity;
    int      active;
} VirtioBlkCtx;

int virtio_blk_init(VirtioBlkCtx *ctx, SymbionPCIDevice *pci);
int virtio_blk_read(VirtioBlkCtx *ctx, uint64_t sector, uint32_t count, void *buffer);
int virtio_blk_write(VirtioBlkCtx *ctx, uint64_t sector, uint32_t count, void *buffer);
