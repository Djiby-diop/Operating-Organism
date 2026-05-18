#ifndef C5C2DD59_390A_43C7_AEF7_215EC5600649
#define C5C2DD59_390A_43C7_AEF7_215EC5600649
#ifndef HEDGED_MALLOC_H
#define HEDGED_MALLOC_H

#include <stddef.h>
#include <stdint.h>

/**
 * Allocates memory with hedged read replicas.
 * Returns base address (replicas are at base + i*size for i in [0, replica_count)).
 */
void* hedged_malloc(size_t size_in_bytes, uint32_t replica_count);

/**
 * Replicates data to all replicas of an allocation.
 */
int hedged_replicate_data(void* ptr, const void* data, size_t size_in_bytes);

/**
 * Performs a hedged read from fastest replica.
 */
int hedged_read(void* ptr, void* out_buffer, size_t size_in_bytes);

/**
 * Gets read statistics for an allocation.
 */
int hedged_get_stats(void* ptr, uint64_t* out_avg_latency_ns, uint32_t* out_read_count);

/**
 * Frees hedged allocation.
 */
void hedged_free(void* ptr);

#endif // HEDGED_MALLOC_H


#endif /* C5C2DD59_390A_43C7_AEF7_215EC5600649 */
