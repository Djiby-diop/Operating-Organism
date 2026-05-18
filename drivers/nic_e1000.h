#pragma once

#include <stdint.h>
#include "../llm-baremetal/engine/ssm/core/symbion.h"

/**
 * NIC E1000 Driver — High Speed Bare-Metal Network
 * 
 * Target: Intel 8254x (PRO/1000)
 */

typedef struct {
    uint32_t mmio_base;
    uint8_t  mac[6];
    
    // RX/TX Ring state
    uint32_t rx_cur;
    uint32_t tx_cur;
    
    int      active;
} NicE1000Ctx;

/**
 * Initializes the E1000 card using the PCI info from Symbion.
 */
int nic_e1000_init(NicE1000Ctx *ctx, SymbionPCIDevice *pci);

/**
 * Sends a raw packet.
 */
int nic_e1000_send(NicE1000Ctx *ctx, void *data, uint16_t len);

/**
 * Receives a packet (polling mode for zero-jitter).
 */
int nic_e1000_receive(NicE1000Ctx *ctx, void *buffer, uint16_t *len);
