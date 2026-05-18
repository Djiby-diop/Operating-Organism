/**
 * test_hedged_malloc.c
 * Unit tests for hedged_malloc + hedged_read integration.
 */

#include <stdio.h>
#include <string.h>
#include "../include/hedged_malloc.h"

void test_basic_allocation() {
    printf("[TEST] Basic allocation with 4 replicas...\n");
    
    size_t size = 256;
    void* ptr = hedged_malloc(size, 4);
    
    if (ptr == NULL) {
        printf("  FAIL: hedged_malloc returned NULL\n");
        return;
    }
    
    printf("  PASS: allocated %zu bytes with 4 replicas at %p\n", size, ptr);
    hedged_free(ptr);
}

void test_replication_and_read() {
    printf("[TEST] Data replication and hedged read...\n");
    
    size_t size = 64;
    void* ptr = hedged_malloc(size, 4);
    if (ptr == NULL) {
        printf("  FAIL: hedged_malloc returned NULL\n");
        return;
    }
    
    uint8_t test_data[64];
    for (int i = 0; i < 64; i++) {
        test_data[i] = (i * 7) % 256;
    }
    
    int ret = hedged_replicate_data(ptr, test_data, size);
    if (ret != 0) {
        printf("  FAIL: hedged_replicate_data returned %d\n", ret);
        hedged_free(ptr);
        return;
    }
    
    uint8_t read_buffer[64];
    ret = hedged_read(ptr, read_buffer, size);
    if (ret != 0) {
        printf("  FAIL: hedged_read returned %d\n", ret);
        hedged_free(ptr);
        return;
    }
    
    if (memcmp(read_buffer, test_data, size) != 0) {
        printf("  FAIL: read data mismatch\n");
        hedged_free(ptr);
        return;
    }
    
    printf("  PASS: data replicated and read successfully\n");
    hedged_free(ptr);
}

void test_read_statistics() {
    printf("[TEST] Read statistics tracking...\n");
    
    size_t size = 128;
    void* ptr = hedged_malloc(size, 4);
    if (ptr == NULL) {
        printf("  FAIL: hedged_malloc returned NULL\n");
        return;
    }
    
    uint8_t test_data[128];
    memset(test_data, 0xAA, sizeof(test_data));
    
    hedged_replicate_data(ptr, test_data, size);
    
    uint8_t read_buffer[128];
    for (int i = 0; i < 10; i++) {
        hedged_read(ptr, read_buffer, size);
    }
    
    uint64_t avg_latency = 0;
    uint32_t read_count = 0;
    int ret = hedged_get_stats(ptr, &avg_latency, &read_count);
    
    if (ret != 0) {
        printf("  FAIL: hedged_get_stats returned %d\n", ret);
        hedged_free(ptr);
        return;
    }
    
    printf("  PASS: %u reads, avg latency = %llu ns\n", read_count, avg_latency);
    hedged_free(ptr);
}

void test_multiple_allocations() {
    printf("[TEST] Multiple concurrent allocations...\n");
    
    void* ptr1 = hedged_malloc(64, 2);
    void* ptr2 = hedged_malloc(256, 3);
    void* ptr3 = hedged_malloc(512, 4);
    
    if (ptr1 == NULL || ptr2 == NULL || ptr3 == NULL) {
        printf("  FAIL: one or more allocations failed\n");
        return;
    }
    
    printf("  PASS: allocated 3 hedged regions\n");
    
    hedged_free(ptr1);
    hedged_free(ptr2);
    hedged_free(ptr3);
}

int main() {
    printf("=== Hedged Malloc Unit Tests ===\n\n");
    
    test_basic_allocation();
    printf("\n");
    
    test_replication_and_read();
    printf("\n");
    
    test_read_statistics();
    printf("\n");
    
    test_multiple_allocations();
    printf("\n");
    
    printf("=== Tests Complete ===\n");
    return 0;
}
