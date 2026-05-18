#include "../include/bot_baremetal.h"
#include "../instinct/instinct_layer.h"

/* Global instinct layer — accessible by organ_bus */
static InstinctLayer _bot_instinct;
static uint8_t _bot_initialized = 0;

void bot_baremetal_init(void) {
    instinct_init(&_bot_instinct);
    _bot_initialized = 1;
}

/* Phase 6F: return current threat level (0=dormant, 255=critical) */
uint8_t bot_get_threat_level(void) {
    if (!_bot_initialized) return 0;
    return instinct_current_level(&_bot_instinct);
}
