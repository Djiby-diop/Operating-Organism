#ifndef UNITED_BUS_H
#define UNITED_BUS_H

#include <stdint.h>
#include <stddef.h>

/// Types de Globules (Paquets de données sur le bus)
typedef enum {
    GLOBULE_RED    = 1, // Données (Tenseurs, I/O réseau, texte)
    GLOBULE_WHITE  = 2, // Immunité (Alertes, Signatures, Kills)
    GLOBULE_YELLOW = 3, // Énergie/Contrôle
    GLOBULE_GOLD   = 4, // Données de Marché
    GLOBULE_SILVER = 5, // Signaux d'Exécution
    GLOBULE_PURPLE = 6  // Thought-Blooms (Vecteurs Sémantiques)
} globule_type_t;

/// Identifiants officiels des Organes
typedef enum {
    ORGAN_CORTEX       = 0,  // Python / High-level logic
    ORGAN_SOMA         = 1,  // C / Main execution
    ORGAN_IMMUNE       = 2,  // Rust / Neural Protector
    ORGAN_METABOLISM   = 3,  // Zig / Energy profiles
    ORGAN_SENSORY      = 4,  // Drivers / I/O
    ORGAN_COLLECTIVE   = 5,  // Go / Swarm communication
    ORGAN_MEMORY       = 6,  // Java / Persistent Long-term
    ORGAN_MERCATORION  = 7,  // C/ASM / Ultra-advanced Trading
    ORGAN_BROADCAST    = 0xFF
} organ_id_t;

/// Structure universelle d'un Globule circulant dans l'organisme
typedef struct {
    uint32_t globule_id;
    globule_type_t type;
    
    // Organe émetteur (ex: ORGAN_TYPE_IMMUNE, ORGAN_TYPE_CORTEX)
    uint8_t source_organ;
    
    // Organe cible (0xFF pour Broadcast à tous les organes)
    uint8_t target_organ;
    
    // Pointeur vers le payload (situé dans la mémoire biologique)
    void* payload_addr;
    uint32_t payload_size;
} globule_t;

/// Initialise le myocarde (le bus principal)
void united_bus_init(void);

/// Pousse un globule dans le flux sanguin (Publish)
/// Retourne 0 en cas de succès, -1 si le flux est bouché (Hémorragie/Congestion)
int united_bus_pump(globule_t globule);

/// Aspire les globules destinés à un organe spécifique (Subscribe/Pull)
/// Retourne le nombre de globules récupérés.
int united_bus_absorb(uint8_t organ_id, globule_t* out_buffer, int max_globules);

typedef struct {
    uint32_t drops;
    uint32_t total_pumps;
    uint32_t overflow_count;
    uint32_t immune_triggers;
    uint32_t pending_globules;
    uint32_t capacity;
} united_bus_health_t;

united_bus_health_t united_bus_get_health(void);
void                united_bus_gc(void);
int                 united_bus_broadcast_yellow(uint8_t source, uint32_t signal_code);
int                 united_bus_broadcast_white(uint8_t source, uint32_t threat_id);

#endif // UNITED_BUS_H
