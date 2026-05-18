#ifndef OO_GENETICS_H
#define OO_GENETICS_H

#include <stdint.h>
#include <stddef.h>

/// Définition d'une mutation (Adaptation LoRA)
typedef struct {
    uint32_t mutation_id;
    uint32_t generation;
    uint8_t confidence;
    size_t weight_size;
} oo_mutation_t;

/// Initialise le moteur d'évolution génétique
void evolution_init(void);

/// Mutation : Applique un adaptateur LoRA au Cortex (LLM)
/// Cela permet de modifier le comportement sans changer le kernel.
int evolution_apply_mutation(const uint8_t* lora_weights, size_t size);

/// Sélection Naturelle : Analyse la performance d'un pattern
/// Si un pattern extrait par le Dream-Baremetal est efficace, il est "sacralisé".
void evolution_evaluate_fitness(uint32_t pattern_id, uint8_t success_rate);

#endif // OO_GENETICS_H
