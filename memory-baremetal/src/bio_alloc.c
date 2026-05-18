#include "../include/bio_mem.h"
#include "../../united-baremetal/include/united_bus.h"

// Variables d'état de l'allocateur
static uint64_t mem_base = 0;
static uint64_t mem_total = 0;

// Bitmap primitif pour suivre l'état des cellules (1 bit par page de 4KB)
// Note: En mode baremetal pur, ce bitmap serait placé à la fin du kernel.
// Pour l'exemple, nous mockons une structure fixe.
#define BITMAP_MAX_CELLS 102400 // 400MB gérés
static uint8_t cell_bitmap[BITMAP_MAX_CELLS / 8];

static inline void set_bit(uint32_t index) {
    cell_bitmap[index / 8] |= (1 << (index % 8));
}

static inline void clear_bit(uint32_t index) {
    cell_bitmap[index / 8] &= ~(1 << (index % 8));
}

static inline int test_bit(uint32_t index) {
    return cell_bitmap[index / 8] & (1 << (index % 8));
}

void bio_mem_init(uint64_t physical_memory_base, uint64_t total_memory_bytes) {
    mem_base = physical_memory_base;
    mem_total = total_memory_bytes;
    
    // Initialiser toutes les cellules à 0 (libres)
    for (int i = 0; i < sizeof(cell_bitmap); i++) {
        cell_bitmap[i] = 0;
    }
}

void* bio_allocate_cell(void) {
    uint32_t free_count = 0;
    void* allocated = NULL;
    
    for (uint32_t i = 0; i < BITMAP_MAX_CELLS; i++) {
        if (!test_bit(i)) {
            if (!allocated) {
                set_bit(i);
                allocated = (void*)(mem_base + (i * CELL_SIZE));
            } else {
                free_count++;
            }
        }
    }
    
    // Alerte de pression métabolique (Si moins de 5% de RAM libre)
    if (free_count < (BITMAP_MAX_CELLS / 20)) {
        globule_t pressure;
        pressure.type = GLOBULE_YELLOW;
        pressure.target_organ = 3; // ORGAN_TYPE_VITAL (Kernel)
        // Signal d'hypoglycémie/manque de ressources
        united_bus_pump(pressure);
    }
    
    return allocated;
}

void bio_free_cell(void* ptr) {
    uint64_t addr = (uint64_t)ptr;
    if (addr >= mem_base) {
        uint32_t index = (addr - mem_base) / CELL_SIZE;
        if (index < BITMAP_MAX_CELLS) {
            clear_bit(index);
        }
    }
}

// Allocateur massif pour les tenseurs du LLM (GGUF weights & KV Cache)
void* bio_allocate_neural_tissue(size_t size_in_bytes) {
    // Calcul du nombre de cellules requises
    uint32_t cells_needed = (size_in_bytes + CELL_SIZE - 1) / CELL_SIZE;
    uint32_t contiguous_count = 0;
    uint32_t start_index = 0;
    
    for (uint32_t i = 0; i < BITMAP_MAX_CELLS; i++) {
        if (!test_bit(i)) {
            if (contiguous_count == 0) start_index = i;
            contiguous_count++;
            
            if (contiguous_count == cells_needed) {
                // Allouer le bloc entier
                for (uint32_t j = start_index; j < start_index + cells_needed; j++) {
                    set_bit(j);
                }
                
                // L'adresse retournée est naturellement alignée sur 4KB,
                // ce qui est parfait pour AVX2 (requiert 32-bytes) ou AVX-512 (64-bytes)
                return (void*)(mem_base + (start_index * CELL_SIZE));
            }
        } else {
            contiguous_count = 0;
        }
    }
    
    return NULL; // Pas de tissu neuronal disponible
}

// Nettoyage agressif par le système immunitaire
void bio_purge_infected_tissue(void* ptr, size_t size_in_bytes) {
    uint64_t addr = (uint64_t)ptr;
    if (addr < mem_base) return;
    
    uint32_t start_index = (addr - mem_base) / CELL_SIZE;
    uint32_t cells_to_purge = (size_in_bytes + CELL_SIZE - 1) / CELL_SIZE;
    
    for (uint32_t i = start_index; i < start_index + cells_to_purge; i++) {
        if (i < BITMAP_MAX_CELLS) {
            // Effacement sécurisé (zeroing cryptographique optionnel)
            uint8_t* memory_ptr = (uint8_t*)(mem_base + (i * CELL_SIZE));
            for (int b = 0; b < CELL_SIZE; b++) {
                memory_ptr[b] = 0x00; // Désintoxication de la cellule
            }
            clear_bit(i);
        }
    }
}

void bio_apoptosis(void* ptr, size_t size_in_bytes) {
    // L'apoptose est une purge contrôlée mais totale.
    // Elle libère également le bitmap pour permettre une ré-instanciation immédiate.
    bio_purge_infected_tissue(ptr, size_in_bytes);
    oo_print("[Memory] Apoptose complete. Tissu pret pour regeneration.\n");
}

void bio_consolidate_memory(void* ptr, size_t size_in_bytes) {
    oo_print("[Memory] 🧠 Consolidation synaptique en cours (RAM -> Flash)...\n");
    // Simulation du transfert vers un stockage persistant
    // En UEFI, on utiliserait le SimpleFileSystemProtocol pour écrire dans un fichier .syn
    
    // Une fois consolidé, on libère la cellule de la mémoire de travail (RAM)
    bio_free_cell(ptr);
}
