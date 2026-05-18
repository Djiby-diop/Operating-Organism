#include "../include/genetics.h"
#include "../../identity-baremetal/include/dna_hash.h"

extern void oo_print(const char* msg);

void evolution_init(void) {
    oo_print("[EvolutionBaremetal] 🧬 Moteur genetique pret. En attente de mutations...\n");
}

int evolution_apply_mutation(const uint8_t* lora_weights, size_t size) {
    if (!lora_weights || size == 0) return -1;
    
    oo_mutation_t mutation;
    mutation.mutation_id = 0; // Mock ID
    mutation.generation = 1;
    mutation.confidence = 95;
    mutation.weight_size = size;
    
    oo_print("[EvolutionBaremetal] 🧪 Tentative de mutation (Application LoRA)...\n");
    
    // 1. Calcul de la signature ADN du nouveau trait
    oo_dna_signature_t sig;
    identity_calculate_dna(lora_weights, size, &sig);
    
    // 2. Validation par le Thymus (Identity)
    // Seules les mutations "reconnues" ou signées peuvent être appliquées.
    if (identity_is_self(&sig)) {
        oo_print("[EvolutionBaremetal] ✅ Mutation validee. Integration au Cortex.\n");
        // On "sacralise" cette nouvelle signature dans le genome de confiance
        identity_trust_dna(&sig);
        return 0;
    } else {
        oo_print("[EvolutionBaremetal] ❌ Rejet de greffe ! Mutation non autorisee.\n");
        return -1;
    }
}

void evolution_evaluate_fitness(uint32_t pattern_id, uint8_t success_rate) {
    if (success_rate > 90) {
        oo_print("[EvolutionBaremetal] 🌟 Pattern performant detecte. Passage au genome permanent.\n");
        // Inscription dans la Flash/SSD pour le prochain boot
    }
}
