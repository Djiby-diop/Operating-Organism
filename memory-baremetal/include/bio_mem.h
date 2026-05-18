#ifndef BIO_MEM_H
#define BIO_MEM_H

#include <stdint.h>
#include <stddef.h>

/// --- TYPES BIOLOGIQUES ---

/// Une Cellule est l'équivalent d'une Page Mémoire (4KB par défaut sur x86)
#define CELL_SIZE 4096

/// Une Macro-Cellule (Huge Page 2MB) pour les Tenseurs Neureaux
#define MACRO_CELL_SIZE (2 * 1024 * 1024)

typedef struct {
    uint64_t physical_addr;
    uint8_t is_infected; // Flag mis à 1 par l'Immune System si compromis
    uint8_t is_active;
} bio_cell_t;

typedef struct {
    uint64_t replica_addr;
    uint32_t logical_index;
    uint32_t replica_index;
    uint32_t simulated_latency_ns;
} bio_hedged_result_t;

/// --- API DU SYSTÈME LYMPHATIQUE (ALLOCATEUR) ---

// Initialise le manager de mémoire avec l'adresse de base (récupérée de l'UEFI)
void bio_mem_init(uint64_t physical_memory_base, uint64_t total_memory_bytes);

// Allocation d'une cellule basique (4KB)
void* bio_allocate_cell(void);

// Libération d'une cellule basique (digestion lymphatique)
void bio_free_cell(void* ptr);

// Alloue un grand bloc contigu pour le Cortex (Tenseurs LLM)
// Retourne une adresse alignée sur 32-bytes (AVX2/AVX-512)
void* bio_allocate_neural_tissue(size_t size_in_bytes);

// Système immunitaire : Purge une zone mémoire corrompue
void bio_purge_infected_tissue(void* ptr, size_t size_in_bytes);

// Apoptose : Suicide cellulaire d'un organe pour régénération
void bio_apoptosis(void* ptr, size_t size_in_bytes);

// Consolidation : Transfère une cellule de la mémoire vive (RAM) vers la mémoire à long terme (Flash)
// Simule le renforcement synaptique.
void bio_consolidate_memory(void* ptr, size_t size_in_bytes);

/// --- GDT & PAGING (ABSTRACTION MATÉRIELLE) ---

// Initialise la table des pages (Page Directory)
void bio_paging_init(void);

// Mappe une adresse virtuelle vers une cellule physique
int bio_map_cell(uint64_t virtual_addr, uint64_t physical_addr, uint32_t flags);

/// --- HEDGED READS (TAIL LATENCY REDUCTION) ---

// Configure le plan de replication mémoire pour les lectures hedged.
// Retourne 0 si OK, -1 sur erreur d'arguments.
int bio_hedged_configure(uint32_t replica_count, uint64_t channel_stride_bytes);

// Ecrit des réplicas d'une même valeur logique dans des canaux mémoire distincts.
// Retourne 0 si OK, -1 sur erreur.
int bio_hedged_replicate_word(uint32_t logical_index, uint64_t value);

// Lit toutes les réplicas d'une valeur logique et renvoie la plus rapide.
// Retourne 0 si OK, -1 sur erreur.
int bio_hedged_read_word(uint32_t logical_index, bio_hedged_result_t* out_result);

// Lit la valeur gagnante pour un index logique donné.
// Retourne 0 si OK, -1 sur erreur.
int bio_hedged_read_value(uint32_t logical_index, uint64_t* out_value);

/// --- HEDGED MALLOC (ALLOCATION AVEC RÉPLICAS MÉMOIRE) ---

// Inclure le header hedged_malloc pour l'API complète
// void* hedged_malloc(size_t size_in_bytes, uint32_t replica_count);
// int hedged_replicate_data(void* ptr, const void* data, size_t size_in_bytes);
// int hedged_read(void* ptr, void* out_buffer, size_t size_in_bytes);
// int hedged_get_stats(void* ptr, uint64_t* out_avg_latency_ns, uint32_t* out_read_count);
// void hedged_free(void* ptr);

#endif // BIO_MEM_H
