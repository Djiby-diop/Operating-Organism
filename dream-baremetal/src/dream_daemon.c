#include <stdint.h>
#include <stddef.h>

// On inclut le bus central pour écouter les signaux du Kernel (Globules Jaunes)
#include "../../united-baremetal/include/united_bus.h"

// Définitions mock
extern void oo_print(const char* msg);
extern void trigger_diop_sleep_learning(void); // FFI vers le moteur Python/C++

#define DREAM_ORGAN_ID 7

static int is_sleeping = 0;

/// Boucle principale de l'organe du Rêve
void dream_baremetal_loop(void) {
    globule_t inbox[10];
    
    while (1) {
        // Le Dream-Baremetal écoute le flux sanguin (Myocarde)
        int count = united_bus_absorb(DREAM_ORGAN_ID, inbox, 10);
        
        for (int i = 0; i < count; i++) {
            globule_t g = inbox[i];
            
            // On écoute les Globules Jaunes (Hormones d'énergie/sommeil du Kernel)
            if (g.type == GLOBULE_YELLOW) {
                // Si le Kernel déclare l'état RELAXED (inactif)
                // Le payload pourrait être l'état de l'homéostasie
                uint8_t state = *(uint8_t*)g.payload_addr;
                
                if (state == 0 /* OO_STATE_RELAXED */) {
                    if (!is_sleeping) {
                        oo_print("[DreamBaremetal] 💤 L'organisme s'endort. Lancement du Sommeil Paradoxal...\n");
                        is_sleeping = 1;
                        
                        // Lancement de l'apprentissage (consolidation mémoire)
                        // Cela appellera votre logique DIOP `SleepLearningEngine` en background
                        trigger_diop_sleep_learning();
                    }
                } else {
                    if (is_sleeping) {
                        oo_print("[DreamBaremetal] ☀️ Réveil brut ! Arrêt des rêves.\n");
                        is_sleeping = 0;
                    }
                }
            }
        }
        
        // Yield() ou HLT pour ne pas consommer de CPU en attendant
    }
}
