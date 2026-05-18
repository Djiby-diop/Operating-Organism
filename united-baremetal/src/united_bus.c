#include "../include/united_bus.h"

// Le volume sanguin (Taille du Ring Buffer principal)
#define BLOOD_VOLUME 2048 // Augmentation du volume sanguin pour plus de stabilité

// Structure pour suivre la santé de l'artère
typedef struct {
    uint32_t drops;
    uint32_t total_pumps;
} blood_health_t;

static globule_t blood_stream[BLOOD_VOLUME];
static volatile uint32_t current_capacity = 1024; // Capacité de base
static volatile uint32_t heart_systole = 0;
static volatile uint32_t heart_diastole = 0;
static blood_health_t health = {0, 0};

// Index de lecture par organe (pour ne pas rater les broadcasts)
#define MAX_ORGANS 16
static uint32_t organ_read_heads[MAX_ORGANS];

// Compteur global unique pour les globules
static uint32_t next_globule_id = 1;

extern void oo_print(const char* msg);

void united_bus_init(void) {
    heart_systole = 0;
    heart_diastole = 0;
    for (int i = 0; i < MAX_ORGANS; i++) {
        organ_read_heads[i] = 0;
    }
    oo_print("[UnitedBus] Le Myocarde bat. Le flux sanguin est initialise.\n");
}

int united_bus_pump(globule_t globule) {
    health.total_pumps++;
    uint32_t next_systole = (heart_systole + 1) % current_capacity;
    
    // Vérifier la congestion artérielle
    if (next_systole == heart_diastole) {
        if (globule.type == GLOBULE_WHITE || globule.type == GLOBULE_YELLOW) {
            heart_diastole = (heart_diastole + 1) % current_capacity;
            // On s'assure que les têtes de lecture des organes ne sont pas dépassées
            for (int i = 0; i < MAX_ORGANS; i++) {
                if (organ_read_heads[i] == heart_systole) {
                    organ_read_heads[i] = heart_diastole;
                }
            }
        } else {
            health.drops++;
            return -1; // Échec pour les globules rouges (Données non-prioritaires)
        }
    }
    
    // Barrière Hémato-Encéphalique (Filtre pour le Cortex)
    if (globule.target_organ == ORGAN_CORTEX) {
        if (globule.type == GLOBULE_RED && globule.source_organ != ORGAN_SENSORY) {
            health.drops++;
            return -2;
        }
    }

    // Dilatation Vasculaire : Si on est en COMBAT (Alerte Immunitaire), on augmente le débit
    if (globule.type == GLOBULE_WHITE) {
        current_capacity = BLOOD_VOLUME; // Expansion maximale
    } else if (current_capacity > 1024) {
        current_capacity--; // Rétractation lente vers la normale
    }

    globule.globule_id = next_globule_id++;
    blood_stream[heart_systole] = globule;
    
    // Flush (Memory Barrier en multi-core)
    heart_systole = next_systole;
    return 0;
}

int united_bus_absorb(uint8_t organ_id, globule_t* out_buffer, int max_globules) {
    if (organ_id >= MAX_ORGANS) return 0;
    
    uint32_t head = organ_read_heads[organ_id];
    int absorbed = 0;
    
    while (head != heart_systole && absorbed < max_globules) {
        globule_t g = blood_stream[head];
        
        // L'organe absorbe le globule si :
        // 1. C'est un Broadcast (0xFF)
        // 2. Il en est la cible explicite
        if (g.target_organ == 0xFF || g.target_organ == organ_id) {
            out_buffer[absorbed] = g;
            absorbed++;
        }
        
        head = (head + 1) % current_capacity;
    }
    
    // Mise à jour de la tête de lecture de l'organe
    organ_read_heads[organ_id] = head;
    
    // Mise à jour de la diastole globale (le globule le plus vieux lu par TOUT le monde)
    // (Simplifié ici : on pourrait avancer heart_diastole en trouvant le min de organ_read_heads)
    
    return absorbed;
}

void united_bus_gc(void) {
    uint32_t min_head = heart_systole;
    for (int i = 0; i < MAX_ORGANS; i++) {
        uint32_t h = organ_read_heads[i];
        uint32_t dist_h   = (heart_systole - h + current_capacity) % current_capacity;
        uint32_t dist_min = (heart_systole - min_head + current_capacity) % current_capacity;
        if (dist_h > dist_min) min_head = h;
    }
    heart_diastole = min_head;
}

united_bus_health_t united_bus_get_health(void) {
    united_bus_health_t h;
    h.drops           = health.drops;
    h.total_pumps     = health.total_pumps;
    h.overflow_count  = health.drops;
    h.immune_triggers = 0;
    h.pending_globules = (heart_systole - heart_diastole + current_capacity) % current_capacity;
    h.capacity        = current_capacity;
    return h;
}

int united_bus_broadcast_yellow(uint8_t source, uint32_t signal_code) {
    globule_t g;
    g.type         = GLOBULE_YELLOW;
    g.source_organ = source;
    g.target_organ = ORGAN_BROADCAST;
    g.payload_addr = 0;
    g.payload_size = signal_code;
    return united_bus_pump(g);
}

int united_bus_broadcast_white(uint8_t source, uint32_t threat_id) {
    globule_t g;
    g.type         = GLOBULE_WHITE;
    g.source_organ = source;
    g.target_organ = ORGAN_BROADCAST;
    g.payload_addr = 0;
    g.payload_size = threat_id;
    return united_bus_pump(g);
}
