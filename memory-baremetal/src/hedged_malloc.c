/**
 * hedged_malloc.c
 * Integrates hedged reads with neural tissue allocation.
 * Allocates replica sets using bio_allocate_neural_tissue,
 * maintains read statistics, and returns fastest replica on hedged read.
 */

#include "../include/bio_mem.h"
#include <string.h>

#define HEDGED_MALLOC_MAX_ALLOCATIONS 256
#define HEDGED_MALLOC_REPLICA_COUNT 4

typedef struct {
    uint64_t base_address;
    size_t allocation_size;
    uint32_t replica_count;
    uint64_t read_latency_total_ns;
    uint32_t read_count;
} hedged_allocation_t;

static hedged_allocation_t allocations[HEDGED_MALLOC_MAX_ALLOCATIONS];
static uint32_t allocation_count = 0;

/**
 * Allocates memory with hedged replicas.
 * Each replica is placed at base + (i * allocation_size),
 * using neural tissue for spatial locality and alignment.
 */
void* hedged_malloc(size_t size_in_bytes, uint32_t replica_count) {
    if (allocation_count >= HEDGED_MALLOC_MAX_ALLOCATIONS) {
        return NULL;
    }
    if (replica_count == 0 || replica_count > HEDGED_MALLOC_REPLICA_COUNT) {
        return NULL;
    }

    size_t total_size = size_in_bytes * replica_count;
    void* block = bio_allocate_neural_tissue(total_size);
    if (block == NULL) {
        return NULL;
    }

    hedged_allocation_t* alloc = &allocations[allocation_count];
    alloc->base_address = (uint64_t)block;
    alloc->allocation_size = size_in_bytes;
    alloc->replica_count = replica_count;
    alloc->read_latency_total_ns = 0;
    alloc->read_count = 0;

    allocation_count++;
    return block;
}

/**
 * Replicates data to all replicas of an allocation.
 */
int hedged_replicate_data(void* ptr, const void* data, size_t size_in_bytes) {
    if (ptr == NULL || data == NULL) {
        return -1;
    }

    hedged_allocation_t* alloc = NULL;
    for (uint32_t i = 0; i < allocation_count; i++) {
        if (allocations[i].base_address == (uint64_t)ptr) {
            alloc = &allocations[i];
            break;
        }
    }

    if (alloc == NULL) {
        return -1;
    }
    if (size_in_bytes > alloc->allocation_size) {
        return -1;
    }

    for (uint32_t r = 0; r < alloc->replica_count; r++) {
        uint8_t* replica_addr = (uint8_t*)(alloc->base_address + (r * alloc->allocation_size));
        memcpy(replica_addr, data, size_in_bytes);
    }

    return 0;
}

/**
 * Simulated latency for a read from a given replica.
 * In real hardware, this would measure actual memory latency from DRAM controllers.
 */
static uint32_t simulate_replica_latency(uint32_t replica_index) {
    uint32_t base_latency = 80;
    uint32_t jitter = (replica_index * 13579u) % 120u;
    return base_latency + jitter;
}

/**
 * Performs a hedged read: reads from all replicas and returns fastest result.
 */
int hedged_read(void* ptr, void* out_buffer, size_t size_in_bytes) {
    if (ptr == NULL || out_buffer == NULL) {
        return -1;
    }

    hedged_allocation_t* alloc = NULL;
    for (uint32_t i = 0; i < allocation_count; i++) {
        if (allocations[i].base_address == (uint64_t)ptr) {
            alloc = &allocations[i];
            break;
        }
    }

    if (alloc == NULL) {
        return -1;
    }
    if (size_in_bytes > alloc->allocation_size) {
        return -1;
    }

    uint32_t best_replica = 0;
    uint32_t best_latency = 0xffffffffu;

    for (uint32_t r = 0; r < alloc->replica_count; r++) {
        uint32_t latency = simulate_replica_latency(r);
        if (latency < best_latency) {
            best_latency = latency;
            best_replica = r;
        }
    }

    uint8_t* replica_addr = (uint8_t*)(alloc->base_address + (best_replica * alloc->allocation_size));
    memcpy(out_buffer, replica_addr, size_in_bytes);

    alloc->read_latency_total_ns += best_latency;
    alloc->read_count++;

    return 0;
}

/**
 * Returns allocation statistics (for performance analysis).
 */
int hedged_get_stats(void* ptr, uint64_t* out_avg_latency_ns, uint32_t* out_read_count) {
    if (ptr == NULL) {
        return -1;
    }

    for (uint32_t i = 0; i < allocation_count; i++) {
        if (allocations[i].base_address == (uint64_t)ptr) {
            hedged_allocation_t* alloc = &allocations[i];
            if (alloc->read_count > 0) {
                *out_avg_latency_ns = alloc->read_latency_total_ns / alloc->read_count;
            } else {
                *out_avg_latency_ns = 0;
            }
            *out_read_count = alloc->read_count;
            return 0;
        }
    }

    return -1;
}

/**
 * Frees hedged allocation and underlying neural tissue.
 */
void hedged_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    for (uint32_t i = 0; i < allocation_count; i++) {
        if (allocations[i].base_address == (uint64_t)ptr) {
            size_t total_size = allocations[i].allocation_size * allocations[i].replica_count;
            bio_apoptosis(ptr, total_size);
            
            if (i < allocation_count - 1) {
                allocations[i] = allocations[allocation_count - 1];
            }
            allocation_count--;
            return;
        }
    }
}
