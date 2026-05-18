#ifndef OO_NERVOUS_SYSTEM_H
#define OO_NERVOUS_SYSTEM_H

#include <stdint.h>

/// Initialise la moelle épinière (IDT - Interrupt Descriptor Table)
void reflex_init(void);

/// Attache un réflexe musculaire à un stimulus matériel
/// @param interrupt_vector Le numéro d'interruption matérielle (IRQ)
/// @param reflex_action Le pointeur de fonction à exécuter instantanément (sans passer par le LLM)
void reflex_bind_action(uint8_t interrupt_vector, void (*reflex_action)(void));

/// Déclencheurs matériels bruts
void reflex_on_thermal_burn(void); // Surchauffe
void reflex_on_pain(void);         // Erreur matérielle critique (NMI)

/* Vecteurs de réflexe standard */
#define OO_REFLEX_PAIN           0x02
#define OO_REFLEX_WATCHDOG       0x03
#define OO_REFLEX_THERMAL        0x04
#define OO_REFLEX_STACK_OVERFLOW 0x08
#define OO_REFLEX_MEMORY_FAULT   0x0E

#define ORGAN_REFLEX_SOURCE      8

typedef struct {
    uint32_t armed_count;
    uint32_t triggered_count;
    uint8_t  last_vector;
} reflex_status_t;

void           reflex_watchdog_kick(void);
void           reflex_thermal_check(uint32_t temp_celsius);
reflex_status_t reflex_get_status(void);

#endif // OO_NERVOUS_SYSTEM_H
