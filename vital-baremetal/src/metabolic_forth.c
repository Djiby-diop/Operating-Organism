#include <stdint.h>
#include <string.h>
#include "../include/vital_spark.h"

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - METABOLIC FORTH (Couche 3 - Enhanced)
/// -----------------------------------------------------------------------------
/// Un interpreteur minimaliste pour executer les "Lois de la Vie" dynamiquement.
/// Permet au Cortex (LLM) d'ajuster finement l'homeostasie.
/// -----------------------------------------------------------------------------

#define FORTH_STACK_SIZE 32
#define MAX_WORDS 128

typedef struct {
    char name;
    char code[8];
    uint8_t length;
} forth_word_t;

static forth_word_t dictionary[MAX_WORDS];
static uint8_t word_count = 0;

static int64_t forth_stack[FORTH_STACK_SIZE];
static int8_t sp = -1;

void forth_push(int64_t val) { if (sp < FORTH_STACK_SIZE - 1) forth_stack[++sp] = val; }
int64_t forth_pop() { return (sp >= 0) ? forth_stack[sp--] : 0; }

// Apprentissage d'un nouveau mot (Morphogenèse)
void forth_learn_word(char name, const char* code, uint8_t len) {
    if (word_count < MAX_WORDS) {
        dictionary[word_count].name = name;
        for(int i=0; i<len && i<8; i++) dictionary[word_count].code[i] = code[i];
        dictionary[word_count].length = len;
        word_count++;
        // oo_print("[Forth] 🧬 Nouveau mot appris.\n");
    }
}

// Execution d'une instruction (ADN ou Mot appris)
void vital_forth_exec(const char* instruction) {
    if (!instruction || !instruction[0]) return;

    char c = instruction[0];

    // 1. Literal (Chiffres 0-9)
    if (c >= '0' && c <= '9') {
        forth_push(c - '0');
        return;
    }

    // 2. Mots appris (Morphogenèse)
    for (int i=0; i<word_count; i++) {
        if (dictionary[i].name == c) {
            for (int j=0; j<dictionary[i].length; j++) {
                char sub[2] = {dictionary[i].code[j], 0};
                vital_forth_exec(sub);
            }
            return;
        }
    }

    // 3. Primitives ADN (Operations de base)
    int64_t a, b;
    switch(c) {
        case '+': b = forth_pop(); a = forth_pop(); forth_push(a + b); break;
        case '-': b = forth_pop(); a = forth_pop(); forth_push(a - b); break;
        case '*': b = forth_pop(); a = forth_pop(); forth_push(a * b); break;
        
        // --- HOOKS BIOLOGIQUES ---
        case 'Y': vital_shift_balance(FORCE_YANG, (uint8_t)forth_pop()); break;
        case 'y': vital_shift_balance(FORCE_YIN, (uint8_t)forth_pop()); break;
        
        case 'P': forth_push(vital_get_pulse_count()); break;
        case 'T': forth_push(vital_get_tension()); break;
        case 'L': forth_push(vital_get_pain_level()); break;
        
        case '.': // Print Top of Stack (Debug)
            a = forth_pop();
            // oo_print("[Forth] Stack Top: %ld\n", a); 
            break;

        case 'D': // Distress Beacon
            vital_emit_distress_beacon();
            break;
    }
}

void vital_metabolic_tick() {
    // Exemple d'autonomie : Si la tension > 150, on injecte du Yin automatiquement.
    // Equivalent Forth : "T 150 - y" (si positif)
    if (vital_get_tension() > 150) {
        vital_forth_exec("5"); // Quantité
        vital_forth_exec("y"); // Injecter Yin
    }
}
