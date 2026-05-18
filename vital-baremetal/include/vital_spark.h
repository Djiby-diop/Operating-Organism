#ifndef OO_VITAL_SPARK_H
#define OO_VITAL_SPARK_H

#include <stdint.h>

/// =============================================================================
/// VITAL-BAREMETAL - MASTER HEADER
/// =============================================================================
/// L'Étincelle Vitale est le pilier central de l'Operating Organism.
/// Ce fichier expose l'intégralité de l'API vitale au reste du système.
/// =============================================================================

// --- FORCES FONDAMENTALES ---
#define ORGAN_TYPE_VITAL 15

typedef enum {
    FORCE_YANG = 0, // Ordre, Stabilité, Production, Défense
    FORCE_YIN  = 1  // Chaos, Rêve, Évolution, Exploration
} oo_vital_force_t;

// --- STRUCTURE CENTRALE ---

typedef struct {
    oo_vital_force_t current_force;
    uint8_t  tension;         // Tension Yin/Yang (0=Yang pur, 255=Yin pur)
    uint64_t pulse_count;     // Battements depuis la naissance
    uint64_t jitter;          // Stress temporel accumulé (Steel Heartbeat)
    uint64_t birth_timestamp; // TSC à la naissance
    uint8_t  age_cycles;      // Nombre de cycles circadiens complets
} oo_vital_spark_t;

// --- API LIFECYCLE ---

/// Naissance de l'organisme
void vital_init(void);

/// La Boucle Éternelle (un seul tick)
void vital_eternal_heartbeat(void);

/// Force l'équilibre Yin/Yang
void vital_shift_balance(oo_vital_force_t force, uint8_t weight);

/// Retourne le pulse count actuel
uint64_t vital_get_pulse_count(void);

/// Retourne la tension actuelle
uint8_t vital_get_tension(void);

/// Retourne le niveau de douleur actuel
uint32_t vital_get_pain_level(void);

/// --- PERSISTENCE HELPERS ---
void vital_get_spark_state(oo_vital_spark_t* out);
void vital_set_spark_state(const oo_vital_spark_t* in);

// --- API ÉPIGÉNÉTIQUE ---

void vital_epigenetic_init(void);
void vital_mark_experience(int8_t impact);
uint8_t vital_get_personality_influence(void);

// --- API FORTH MÉTABOLIQUE ---

void vital_forth_exec(const char* instruction);
void forth_learn_word(char name, const char* code, uint8_t len);
void vital_metabolic_tick(void);

// --- API PERSISTENCE (Soma-DNA) ---

void vital_persistence_init(void* storage_ctx);
void vital_persistence_save(void);
void vital_persistence_load(void);

// --- API ENTROPIE ---

void vital_generate_biotic_noise(uint64_t entropy);
void vital_final_stand(void);

// --- API AURA & PINEAL ---

void vital_update_aura(uint8_t tension, oo_vital_force_t force);
void vital_emit_distress_beacon(void);
void vital_pineal_gland_sync(void);

// --- API EXTERNE (Assembleur / Rust) ---

extern void vital_steel_tick(uint64_t* pulse, uint64_t* jitter);
extern uint64_t get_organism_vitality(void);
extern uint8_t  get_guardian_state(void);
extern uint64_t get_total_anomalies(void);
extern uint8_t  get_monitored_cores(void);
extern void vital_quantum_sign(uint64_t entropy);
extern int  vital_quantum_verify(uint64_t entropy);
extern uint64_t vital_temporal_get_jitter(void);
extern void vital_ouroboros_panic(void);

// --- PRINT GLOBAL ---

extern void oo_print(const char* msg);

#endif // OO_VITAL_SPARK_H
