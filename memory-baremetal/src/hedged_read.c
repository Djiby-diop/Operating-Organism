#include "../include/bio_mem.h"

#define HEDGED_MAX_REPLICAS 8
#define HEDGED_MAX_WORDS 4096

static uint32_t g_replica_count = 0;
static uint64_t g_channel_stride = 0;
static uint64_t g_replica_store[HEDGED_MAX_REPLICAS][HEDGED_MAX_WORDS];

static uint32_t pseudo_latency_ns(uint32_t logical_index, uint32_t replica_index) {
    uint32_t seed = (logical_index * 1103515245u) ^ (replica_index * 2654435761u);
    seed ^= (seed >> 16);
    return 80u + (seed % 220u);
}

int bio_hedged_configure(uint32_t replica_count, uint64_t channel_stride_bytes) {
    if (replica_count == 0 || replica_count > HEDGED_MAX_REPLICAS) {
        return -1;
    }
    if (channel_stride_bytes == 0) {
        return -1;
    }

    g_replica_count = replica_count;
    g_channel_stride = channel_stride_bytes;
    return 0;
}

int bio_hedged_replicate_word(uint32_t logical_index, uint64_t value) {
    uint32_t i;

    if (g_replica_count == 0 || logical_index >= HEDGED_MAX_WORDS) {
        return -1;
    }

    for (i = 0; i < g_replica_count; i++) {
        g_replica_store[i][logical_index] = value;
    }
    return 0;
}

int bio_hedged_read_word(uint32_t logical_index, bio_hedged_result_t* out_result) {
    uint32_t i;
    uint32_t best_replica = 0;
    uint32_t best_latency = 0xffffffffu;

    if (out_result == NULL || g_replica_count == 0 || logical_index >= HEDGED_MAX_WORDS) {
        return -1;
    }

    for (i = 0; i < g_replica_count; i++) {
        uint32_t latency = pseudo_latency_ns(logical_index, i);
        if (latency < best_latency) {
            best_latency = latency;
            best_replica = i;
        }
    }

    out_result->logical_index = logical_index;
    out_result->replica_index = best_replica;
    out_result->replica_addr = (uint64_t)&g_replica_store[best_replica][logical_index] + (best_replica * g_channel_stride);
    out_result->simulated_latency_ns = best_latency;

    return 0;
}

int bio_hedged_read_value(uint32_t logical_index, uint64_t* out_value) {
    bio_hedged_result_t winner;
    uint64_t value;

    if (out_value == NULL) {
        return -1;
    }
    if (bio_hedged_read_word(logical_index, &winner) != 0) {
        return -1;
    }

    value = g_replica_store[winner.replica_index][logical_index];
    *out_value = value;
    return 0;
}
