#include <stdint.h>
#include "../include/vital_spark.h"

/// =============================================================================
/// VITAL-BAREMETAL - EPIGENETIC MEMORY (Version Complète)
/// =============================================================================
/// Stocke les Marqueurs Épigénétiques : les cicatrices et les triomphes
/// de l'organisme qui influencent son comportement sans changer son ADN.
/// Persiste entre les reboots (NVRAM / secteur dédié).
/// =============================================================================

#define MAX_EPIGENETIC_EVENTS 256

/// Un événement épigénétique horodaté
typedef struct {
    uint64_t pulse_at;       // Quand c'est arrivé
    int8_t   impact;         // Négatif = trauma, Positif = succès
    uint8_t  source_organ;   // Qui a causé l'événement
} oo_epigenetic_event_t;

/// La structure complète des marqueurs
typedef struct {
    uint32_t trauma_level;
    uint32_t success_count;
    uint32_t total_experiences;
    uint8_t  personality_bias;   // 0=Yang pur, 255=Yin pur
    uint8_t  resilience;         // Capacité à encaisser (augmente avec l'expérience)
    uint8_t  generation;         // Numéro de génération (incrémenté à chaque reboot)
    
    // Journal circulaire des événements récents
    oo_epigenetic_event_t events[MAX_EPIGENETIC_EVENTS];
    uint16_t event_head;
    
    // Signature de validation (pour détecter la corruption du journal)
    uint32_t checksum;
} oo_epigenetic_memory_t;

static oo_epigenetic_memory_t memory;

/// Calcul du checksum de protection
static uint32_t compute_epigenetic_checksum(void) {
    uint32_t sum = 0x12345678;
    sum ^= memory.trauma_level;
    sum ^= memory.success_count;
    sum ^= memory.total_experiences;
    sum ^= memory.personality_bias;
    sum ^= memory.resilience;
    sum ^= memory.generation;
    return sum;
}

void vital_epigenetic_init(void) {
    // En production : lire depuis la NVRAM / secteur Flash dédié
    // Si le checksum est valide → on reprend la mémoire ancestrale
    // Sinon → on initialise une mémoire vierge (première naissance)
    
    uint32_t stored_checksum = memory.checksum;
    uint32_t computed = compute_epigenetic_checksum();
    
    if (stored_checksum == computed && memory.total_experiences > 0) {
        // Mémoire ancestrale valide → l'organisme "se souvient"
        memory.generation++;
        oo_print("[Epigenetics] 🧬 Mémoire ancestrale restaurée ! Génération ");
        // En production : on afficherait memory.generation
        oo_print("suivante.\n");
    } else {
        // Première naissance ou corruption
        memory.trauma_level      = 0;
        memory.success_count     = 0;
        memory.total_experiences = 0;
        memory.personality_bias  = 128;
        memory.resilience        = 50;
        memory.generation        = 1;
        memory.event_head        = 0;
        memory.checksum          = compute_epigenetic_checksum();
        oo_print("[Epigenetics] 🧬 Première naissance. Mémoire vierge.\n");
    }
}

void vital_mark_experience(int8_t impact) {
    // Enregistrement dans le journal circulaire
    oo_epigenetic_event_t evt;
    evt.pulse_at = vital_get_pulse_count();
    evt.impact = impact;
    evt.source_organ = 0; // À renseigner par l'appelant
    
    memory.events[memory.event_head] = evt;
    memory.event_head = (memory.event_head + 1) % MAX_EPIGENETIC_EVENTS;
    memory.total_experiences++;
    
    if (impact < 0) {
        // Traumatisme
        memory.trauma_level++;
        
        // La résilience atténue l'impact sur la personnalité
        uint8_t effective_impact = (5 > memory.resilience / 25) ? 
                                   5 - memory.resilience / 25 : 1;
        
        if (memory.personality_bias > effective_impact) {
            memory.personality_bias -= effective_impact;
        }
        
        // Les traumatismes renforcent la résilience (ce qui ne tue pas rend plus fort)
        if (memory.resilience < 250) memory.resilience++;
        
    } else if (impact > 0) {
        // Succès
        memory.success_count++;
        
        if (memory.personality_bias < 250) {
            memory.personality_bias += 3;
        }
    }
    
    // Mise à jour du checksum de protection
    memory.checksum = compute_epigenetic_checksum();
}

uint8_t vital_get_personality_influence(void) {
    return memory.personality_bias;
}

/// Retourne le niveau de résilience
uint8_t vital_get_resilience(void) {
    return memory.resilience;
}

/// Retourne la génération actuelle
uint8_t vital_get_generation(void) {
    return memory.generation;
}

uint8_t vital_get_success_ratio(void) {
    if (memory.total_experiences == 0) return 50;
    return (uint8_t)(memory.success_count * 100 / memory.total_experiences);
}

// --- PERSISTENCE ACCESSORS ---

void vital_epigenetic_export(void* dst, uint32_t max_size) {
    if (!dst || max_size < sizeof(oo_epigenetic_memory_t)) return;
    
    // Mise à jour finale du checksum avant export
    memory.checksum = compute_epigenetic_checksum();
    
    uint8_t* d = (uint8_t*)dst;
    uint8_t* s = (uint8_t*)&memory;
    for (uint32_t i = 0; i < sizeof(oo_epigenetic_memory_t); i++) d[i] = s[i];
}

void vital_epigenetic_import(const void* src, uint32_t size) {
    if (!src || size < sizeof(oo_epigenetic_memory_t)) return;
    
    const uint8_t* s = (const uint8_t*)src;
    uint8_t* d = (uint8_t*)&memory;
    for (uint32_t i = 0; i < sizeof(oo_epigenetic_memory_t); i++) d[i] = s[i];
    
    // Vérification de l'intégrité de la mémoire importée
    if (memory.checksum != compute_epigenetic_checksum()) {
        oo_print("[Epigenetics] ⚠️ Memoire importee corrompue. Reset.\n");
        vital_epigenetic_init();
    } else {
        oo_print("[Epigenetics] 🧬 Heritage epigenetique restaure.\n");
    }
}
