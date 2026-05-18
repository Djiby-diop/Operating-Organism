#pragma once

#include <stdint.h>
#include "../llm-baremetal/engine/ssm/core/symbion.h"

/**
 * HDA Driver — Intel High Definition Audio
 */

typedef struct {
    uint32_t mmio_base;
    uint32_t stream_count;
    int      active;
} HdaCtx;

int hda_init(HdaCtx *ctx, SymbionPCIDevice *pci);
void hda_play_pcm(HdaCtx *ctx, void *data, uint32_t size);
void hda_record_pcm(HdaCtx *ctx, void *buffer, uint32_t size);
