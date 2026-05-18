#pragma once

#include <stdint.h>
#include "../llm-baremetal/engine/ssm/core/symbion.h"

/**
 * VirtIO-Net Driver — High Speed Virtualized Networking
 */

typedef struct {
    uint32_t mmio_base;
    uint32_t features;
    int      active;
} VirtioNetCtx;

int virtio_net_init(VirtioNetCtx *ctx, SymbionPCIDevice *pci);
int virtio_net_send(VirtioNetCtx *ctx, void *data, uint16_t len);
int virtio_net_receive(VirtioNetCtx *ctx, void *buffer, uint16_t *len);
