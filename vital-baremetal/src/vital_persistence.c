#include "../include/vital_spark.h"
#include "../include/vital_synapse.h"
#include "oo_storage.h"

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - PERSISTENCE (Soma-DNA)
/// -----------------------------------------------------------------------------
/// Gère la sauvegarde et le chargement de l'état vital sur le disque.
/// Permet à l'organisme de survivre aux redémarrages (Réincarnation).
/// -----------------------------------------------------------------------------

#define DNA_MAGIC 0x414E444F // "OO_DNA" (little endian)
#define DNA_FILE  "OO_DNA.BIN"

typedef struct {
    uint32_t magic;
    uint32_t version;
    oo_vital_spark_t spark;
    
    // Synaptic state
    uint32_t synapse_size;
    uint8_t  synapse_data[1024]; // Buffer pour les liens synaptiques
    
    // Epigenetic state
    uint8_t  epigenetic_data[512]; // Buffer pour oo_epigenetic_memory_t
    
    // Forth state (à implémenter plus tard)
    uint8_t  forth_data[1024];
    uint32_t forth_size;
    
    uint32_t checksum;
} SomaDNA;

static OoStorageCtx* storage = (OoStorageCtx*)0;

// Accesseurs du module epigenetics.c
void vital_epigenetic_export(void* dst, uint32_t max_size);
void vital_epigenetic_import(const void* src, uint32_t size);

void vital_persistence_init(void* storage_ctx) {
    storage = (OoStorageCtx*)storage_ctx;
    oo_print("[Persistence] 💾 Systeme de persistance Soma-DNA initialise.\n");
}

void vital_persistence_save(void) {
    if (!storage) return;

    SomaDNA dna;
    dna.magic = DNA_MAGIC;
    dna.version = 1;
    
    // 1. Sauvegarde de l'étincelle
    vital_get_spark_state(&dna.spark);
    
    // 2. Sauvegarde des synapses
    dna.synapse_size = synapse_export_links(dna.synapse_data, 1024);
    
    // 3. Sauvegarde de la mémoire épigénétique
    vital_epigenetic_export(dna.epigenetic_data, 512);
    
    // 4. Écriture sur disque
    if (oo_storage_write_all(storage, DNA_FILE, &dna, sizeof(SomaDNA)) == 0) {
        oo_print("[Persistence] ✅ Soma-DNA sauvegarde avec succes.\n");
    } else {
        oo_print("[Persistence] ❌ Echec de la sauvegarde Soma-DNA.\n");
    }
}

void vital_persistence_load(void) {
    if (!storage) return;

    if (!oo_storage_exists(storage, DNA_FILE)) {
        oo_print("[Persistence] ℹ️ Aucun Soma-DNA existant. Nouvelle incarnation.\n");
        return;
    }

    SomaDNA dna;
    if (oo_storage_read_all(storage, DNA_FILE, &dna, sizeof(SomaDNA)) > 0) {
        if (dna.magic == DNA_MAGIC) {
            oo_print("[Persistence] ✨ Soma-DNA detecte. Reincarnation en cours...\n");
            
            // 1. Restaurer les synapses
            synapse_import_links(dna.synapse_data, dna.synapse_size);
            
            // 2. Restaurer l'étincelle
            vital_set_spark_state(&dna.spark);
            
            // 3. Restaurer la mémoire épigénétique
            vital_epigenetic_import(dna.epigenetic_data, 512);
            
            oo_print("[Persistence] ✅ Reincarnation terminee.\n");
        } else {
            oo_print("[Persistence] ⚠️ Soma-DNA corrompu ou invalide.\n");
        }
    }
}
