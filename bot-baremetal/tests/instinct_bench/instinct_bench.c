/**
 * BOT-BAREMETAL — Instinct Bench
 *
 * Benchmark < 1ms de la couche InstinctLayer.
 * Vérifie que la réaction aux triggers est instantanée.
 */

#include "instinct_layer.h"
#include "bot_dna.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/* Helper pour la précision (Standard C fallback) */
static double get_time_ms(void) {
    return (double)clock() * 1000.0 / CLOCKS_PER_SEC;
}

int main(void) {
    printf("=== InstinctLayer Benchmark ===\n");

    InstinctLayer il;
    instinct_init(&il);

    const int iterations = 100000;
    double start_time = get_time_ms();

    for (int i = 0; i < iterations; i++) {
        /* On déclenche aléatoirement des triggers qui correspondent aux règles par défaut */
        instinct_trigger(&il, TRIGGER_FS_CRITICAL_MODIF, 1234, 0, 100);
        instinct_trigger(&il, TRIGGER_NET_C2_BEACON, 5678, 0, 80);
        instinct_trigger(&il, TRIGGER_KERN_DKOM, 0, 0xFFFF0000, 100);
    }

    double end_time = get_time_ms();
    double total_ms = end_time - start_time;
    double ms_per_trigger = total_ms / (iterations * 3);

    printf("Total triggers : %d\n", iterations * 3);
    printf("Total time     : %.3f ms\n", total_ms);
    printf("Avg time/trig  : %.6f ms\n", ms_per_trigger);

    if (ms_per_trigger < 1.0) {
        printf("[PASS] InstinctLayer is operating well below the 1ms threshold.\n");
    } else {
        printf("[FAIL] InstinctLayer is too slow.\n");
    }

    return 0;
}
