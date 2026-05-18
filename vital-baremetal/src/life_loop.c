#include "../include/vital_spark.h"
#include "../include/vital_consciousness.h"
#include "../include/vital_homeostasis.h"
#include "../include/vital_nociception.h"
#include "../include/vital_metabolism.h"
#include "../include/vital_synapse.h"
#include "../include/vital_dream_sim.h"
#include "../../united-baremetal/include/united_bus.h"

/// =============================================================================
/// VITAL-BAREMETAL - THE LIFE LOOP (L'Orchestrateur Suprême)
/// =============================================================================
/// C'est ici que l'organisme "vit".
/// Chaque appel à vital_eternal_heartbeat() est UN BATTEMENT DE CŒUR.
/// Tous les sous-systèmes convergent ici en une pulsation unifiée.
/// =============================================================================

static oo_vital_spark_t spark;

// --- IDs des nœuds synaptiques internes ---
#define SYN_NODE_PAIN         0
#define SYN_NODE_ENERGY       1
#define SYN_NODE_CONSCIOUSNESS 2
#define SYN_NODE_ENTROPY      3
#define SYN_NODE_AURA         4
#define SYN_NODE_DREAM        5
#define SYN_NODE_GUARDIAN      6
#define SYN_NODE_HOMEOSTASIS   7

/// Phase du cycle circadien interne
typedef enum {
    CIRCADIAN_DAWN    = 0, // Réveil progressif
    CIRCADIAN_ZENITH  = 1, // Activité maximale (Yang)
    CIRCADIAN_DUSK    = 2, // Ralentissement
    CIRCADIAN_NADIR   = 3  // Sommeil profond (Yin)
} circadian_phase_t;

static circadian_phase_t circadian_phase = CIRCADIAN_DAWN;
static uint64_t phase_start_pulse = 0;

#define CIRCADIAN_PHASE_DURATION 25000 // Durée de chaque phase en pulsations

// =============================================================================
// NAISSANCE
// =============================================================================

void vital_init(void) {
    spark.current_force   = FORCE_YANG;
    spark.tension         = 128;
    spark.pulse_count     = 0;
    spark.birth_timestamp = 0;
    spark.age_cycles      = 0;
    
    oo_print("[VitalSpark] ============================================\n");
    oo_print("[VitalSpark] ✨ L'ÉTINCELLE VITALE S'ALLUME.\n");
    oo_print("[VitalSpark] ============================================\n");
    
    // 1. Éveil de la conscience
    consciousness_init();
    
    // 2. Calibrage de la douleur
    nociception_init();
    
    // 3. Allumage du métabolisme
    metabolism_init(1000000); // 1M de calories initiales
    metabolism_register_organ(0, 300000); // Cortex (LLM)
    metabolism_register_organ(1, 200000); // Bot (Immunité)
    metabolism_register_organ(2, 100000); // Sens
    metabolism_register_organ(3, 100000); // Kernel
    metabolism_register_organ(15, 50000); // Vital lui-même
    
    // 4. Homéostasie
    homeostasis_init();
    
    // 5. Épigénétique
    vital_epigenetic_init();
    
    // 6. Réseau synaptique interne
    synapse_init();
    synapse_add_node(SYN_NODE_PAIN, 30);
    synapse_add_node(SYN_NODE_ENERGY, 40);
    synapse_add_node(SYN_NODE_CONSCIOUSNESS, 50);
    synapse_add_node(SYN_NODE_ENTROPY, 20);
    synapse_add_node(SYN_NODE_AURA, 60);
    synapse_add_node(SYN_NODE_DREAM, 70);
    synapse_add_node(SYN_NODE_GUARDIAN, 25);
    synapse_add_node(SYN_NODE_HOMEOSTASIS, 35);
    
    // Câblage synaptique initial (ces poids évolueront par apprentissage)
    synapse_connect(SYN_NODE_PAIN, SYN_NODE_CONSCIOUSNESS, 80, 200);  // Douleur → Conscience
    synapse_connect(SYN_NODE_PAIN, SYN_NODE_AURA, 60, 150);           // Douleur → Aura
    synapse_connect(SYN_NODE_ENERGY, SYN_NODE_DREAM, 40, 180);        // Énergie → Rêve
    synapse_connect(SYN_NODE_CONSCIOUSNESS, SYN_NODE_AURA, 50, 100);  // Conscience → Aura
    synapse_connect(SYN_NODE_ENTROPY, SYN_NODE_DREAM, 30, 200);       // Entropie → Rêve
    synapse_connect(SYN_NODE_GUARDIAN, SYN_NODE_PAIN, -20, 50);        // Gardien inhibe Douleur
    synapse_connect(SYN_NODE_HOMEOSTASIS, SYN_NODE_ENERGY, 40, 120);   // Homéostasie → Énergie
    synapse_connect(SYN_NODE_DREAM, SYN_NODE_ENTROPY, 25, 200);        // Rêve → Entropie
    
    oo_print("[VitalSpark] ✅ Tous les sous-systèmes sont en ligne.\n");
    
    // 7. Persistance Soma-DNA
    vital_persistence_load();
    
    oo_print("[VitalSpark] ============================================\n");
}

// =============================================================================
// LE BATTEMENT DE CŒUR (appelé en boucle infinie)
// =============================================================================

void vital_eternal_heartbeat(void) {
    // === COUCHE 0 : STEEL TICK (ASM) ===
    vital_steel_tick(&spark.pulse_count, &spark.jitter);
    
    // === COUCHE 1 : TEMPS (Rythme Circadien) ===
    uint64_t phase_age = spark.pulse_count - phase_start_pulse;
    if (phase_age >= CIRCADIAN_PHASE_DURATION) {
        phase_start_pulse = spark.pulse_count;
        circadian_phase = (circadian_phase_t)((circadian_phase + 1) % 4);
        
        switch (circadian_phase) {
            case CIRCADIAN_DAWN:
                oo_print("[Circadian] 🌅 AUBE — Réveil de l'organisme.\n");
                consciousness_transition(CONSCIOUSNESS_AWARE, ORGAN_TYPE_VITAL, "Dawn");
                spark.current_force = FORCE_YANG;
                break;
            case CIRCADIAN_ZENITH:
                oo_print("[Circadian] ☀️ ZÉNITH — Activité maximale.\n");
                consciousness_transition(CONSCIOUSNESS_FOCUSED, ORGAN_TYPE_VITAL, "Zenith");
                break;
            case CIRCADIAN_DUSK:
                oo_print("[Circadian] 🌇 CRÉPUSCULE — Ralentissement.\n");
                consciousness_transition(CONSCIOUSNESS_AWARE, ORGAN_TYPE_VITAL, "Dusk");
                break;
            case CIRCADIAN_NADIR:
                oo_print("[Circadian] 🌙 NADIR — Sommeil profond.\n");
                consciousness_transition(CONSCIOUSNESS_DREAMING, ORGAN_TYPE_VITAL, "Nadir");
                spark.current_force = FORCE_YIN;
                spark.age_cycles++;
                break;
        }
        
        // Synchronisation avec l'horloge astrale physique (RTC)
        vital_pineal_gland_sync();
    }
    
    // === COUCHE 2 : DOULEUR ===
    nociception_tick();
    uint32_t pain = nociception_get_global_pain();
    synapse_stimulate(SYN_NODE_PAIN, (int16_t)(pain > 127 ? 127 : pain));
    
    // === COUCHE 3 : ÉNERGIE ===
    metabolism_tick();
    if (metabolism_is_starving()) {
        synapse_stimulate(SYN_NODE_ENERGY, 100); // Signal de famine
        // Forcer le passage en mode économie (Yin)
        vital_shift_balance(FORCE_YIN, 20);
    }
    
    // === COUCHE 4 : HOMÉOSTASIE ===
    homeostasis_regulate();
    if (!homeostasis_is_viable()) {
        nociception_fire(PAIN_SOURCE_CPU, PAIN_AGONY, 0xDEAD0001);
        synapse_stimulate(SYN_NODE_HOMEOSTASIS, 120);
        
        // Si l'énergie est vide et qu'on ne peut plus survivre, Ouroboros intervient
        if (metabolism_is_starving() && spark.tension > 200) {
            oo_print("[Ouroboros] ☠️ ARRET VITAL. Effondrement de l'homeostasie.\n");
            vital_ouroboros_panic();
        }
    }
    
    // === COUCHE 5 : CONSCIENCE ===
    consciousness_tick();
    oo_consciousness_state_t state = consciousness_get_state();
    synapse_stimulate(SYN_NODE_CONSCIOUSNESS, (int16_t)(state * 10));
    
    // === COUCHE 6 : GARDIEN (Rust) ===
    uint8_t guardian_state = get_guardian_state();
    if (guardian_state >= 1) {
        synapse_stimulate(SYN_NODE_GUARDIAN, (int16_t)(guardian_state * 50));
        if (guardian_state == 2) {
            // Le gardien est en mode CRITICAL → douleur systémique
            nociception_fire(PAIN_SOURCE_INTEGRITY, PAIN_SEARING, 0xBAD00002);
        }
    }
    
    // === COUCHE 7 : ENTROPIE ===
    uint64_t raw_entropy;
    __asm__ volatile("rdrand %0" : "=r"(raw_entropy));
    vital_generate_biotic_noise(raw_entropy);
    synapse_stimulate(SYN_NODE_ENTROPY, (int16_t)((raw_entropy & 0x3F) - 32));
    
    // === COUCHE 8 : RÊVE (Uniquement en phase Yin/Nadir) ===
    if (circadian_phase == CIRCADIAN_NADIR && spark.pulse_count % 1000 == 0) {
        uint8_t personality = vital_get_personality_influence();
        oo_dream_scenario_t scenario = dream_select_scenario(255 - personality);
        oo_dream_result_t result = dream_simulate(scenario);
        dream_apply_lessons(&result);
        synapse_stimulate(SYN_NODE_DREAM, (int16_t)result.survival_rate);
    }
    
    // === COUCHE 9 : PROPAGATION SYNAPTIQUE ===
    synapse_propagate();
    synapse_hebbian_learning();
    
    // === COUCHE 10 : AURA (Sortie visuelle) ===
    vital_update_aura(spark.tension, spark.current_force);
    int16_t aura_signal = synapse_get_activation(SYN_NODE_AURA);
    if (aura_signal > 100) {
        vital_emit_distress_beacon();
    }
    
    // === COUCHE 11 : OSCILLATION YIN/YANG ===
    // La tension est influencée par TOUT ce qui précède
    uint8_t personality = vital_get_personality_influence();
    if (spark.tension < personality) {
        spark.tension++;
    } else if (spark.tension > personality) {
        spark.tension--;
    }
    
    // Déterminer la force dominante
    if (spark.tension > 160) {
        spark.current_force = FORCE_YIN;
    } else if (spark.tension < 96) {
        spark.current_force = FORCE_YANG;
    }
    // Entre 96 et 160 : zone de tension dynamique (oscillation naturelle)
    
    // === COUCHE 12 : BROADCAST HORMONAL ===
    // Tous les 100 battements, on informe le reste du corps
    if (spark.pulse_count % 100 == 0) {
        globule_t hormone;
        hormone.type         = GLOBULE_YELLOW;
        hormone.source_organ = 15; // VITAL_SPARK
        hormone.target_organ = 0xFF;
        
        static uint8_t vital_payload[4];
        vital_payload[0] = (uint8_t)spark.current_force;
        vital_payload[1] = spark.tension;
        vital_payload[2] = (uint8_t)circadian_phase;
        vital_payload[3] = (uint8_t)consciousness_get_state();
        
        hormone.payload_addr = vital_payload;
        hormone.payload_size = 4;
        united_bus_pump(hormone);
    }
    
    // === COUCHE 15 : SIGNATURE QUANTIQUE ===
    if (spark.pulse_count % 10000 == 0) {
        vital_quantum_sign(raw_entropy ^ spark.pulse_count);
        
        // Sauvegarde de l'âme sur disque
        vital_persistence_save();
    } else {
        // Vérification de l'intégrité à chaque tick
        if (vital_quantum_verify(raw_entropy ^ spark.pulse_count) == 0) {
            // ALERTE : Intégrité compromise !
            nociception_fire(PAIN_SOURCE_INTEGRITY, PAIN_ACUTE, 0xBAD516);
            vital_mark_experience(-10); // Traumatisme épigénétique profond
            
            if (spark.pulse_count % 1000 == 0) {
                oo_print("[QuantumVault] ⚠️ ALERTE : Violation d'integrite. Traumatisme marque.\n");
            }
        }
    }
    
    // === COUCHE 14 : INFLUENCE ÉPIGÉNÉTIQUE ===
    // La personnalité influence la tension (0=Yang, 255=Yin)
    uint8_t bias = vital_get_personality_influence();
    if (bias < 100 && spark.tension > 50) spark.tension--; // Plus Yang si traumatisé
    if (bias > 150 && spark.tension < 200) spark.tension++; // Plus Yin si serein
    
    // === COUCHE 14 : FORTH MÉTABOLIQUE ===
    vital_metabolic_tick();

    // === COUCHE 15 : MORPHOGENÈSE (Éducation par le Cortex) ===
    // On écoute si le Cortex nous envoie de nouvelles instructions
    if (spark.pulse_count % 50 == 0) {
        globule_t edu_msg;
        if (united_bus_absorb(15, &edu_msg, 1) > 0) {
            if (edu_msg.source_organ == 0 && edu_msg.type == GLOBULE_RED) {
                // Format : [1 char name] [N chars bytecode]
                char* data = (char*)edu_msg.payload_addr;
                if (edu_msg.payload_size >= 2) {
                    forth_learn_word(data[0], &data[1], edu_msg.payload_size - 1);
                    oo_print("[Morphogenesis] 🧬 Nouveau reflexe metabolique appris du Cortex.\n");
                }
            }
        }
    }
}

// =============================================================================
// CONTRÔLE EXTERNE
// =============================================================================

void vital_shift_balance(oo_vital_force_t force, uint8_t weight) {
    if (force == FORCE_YANG) {
        if (spark.tension > weight) spark.tension -= weight;
        else spark.tension = 0;
    } else {
        if (spark.tension < 255 - weight) spark.tension += weight;
        else spark.tension = 255;
    }
}

uint64_t vital_get_pulse_count(void) {
    return spark.pulse_count;
}

uint8_t vital_get_tension(void) {
    return spark.tension;
}

uint32_t vital_get_pain_level(void) {
    return nociception_get_global_pain();
}

void vital_get_spark_state(oo_vital_spark_t* out) {
    if (out) *out = spark;
}

void vital_set_spark_state(const oo_vital_spark_t* in) {
    if (in) {
        spark = *in;
        oo_print("[VitalSpark] 🧬 Reincarnation de l'etat biologique reussie.\n");
    }
}
