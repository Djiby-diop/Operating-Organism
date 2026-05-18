#ifndef BA7209CE_ABE5_4864_A791_8D925D31B50C
#define BA7209CE_ABE5_4864_A791_8D925D31B50C
#ifndef BOT_BAREMETAL_H
#define BOT_BAREMETAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../core/bot_dna.h"
#include "../core/territory_map.h"

// Canonical module facade for strict baremetal layout checks.
void bot_baremetal_init(void);

// Phase 6F: Return current threat level for organ_bus (0=dormant, 255=critical)
uint8_t bot_get_threat_level(void);

#ifdef __cplusplus
}
#endif

#endif // BOT_BAREMETAL_H


#endif /* BA7209CE_ABE5_4864_A791_8D925D31B50C */
