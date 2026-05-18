#pragma once

#include <stdint.h>
#include "../llm-baremetal/engine/ssm/core/symbion.h"

/**
 * XHCI Driver — Modern USB 3.0 Host Controller
 */

typedef struct {
    uint32_t cap_len;
    uint32_t op_off;
    uint32_t rt_off;
    uint32_t db_off;
    
    uint32_t mmio_base;
    int      active;
} XhciCtx;

int xhci_init(XhciCtx *ctx, SymbionPCIDevice *pci);
int xhci_reset(XhciCtx *ctx);
void xhci_poll_events(XhciCtx *ctx);
