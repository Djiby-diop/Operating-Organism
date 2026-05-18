#ifndef OO_STEALTH_H
#define OO_STEALTH_H

#include <stdint.h>
#include <stddef.h>

/// Initialise l'instinct de furtivité
void shadow_init(void);

/// Active le camouflage (Hook les tables de pages pour se cacher)
/// @param threat_severity De 1 (Suspicion) à 10 (Mort imminente)
void shadow_activate_camouflage(uint8_t threat_severity);

/// Désactive la présence de l'organisme (Fake Halt)
/// Fait croire à l'OS externe / Hyperviseur que la machine est plantée
void shadow_simulate_death(void);

/// Zéroïsation d'urgence des clés et mémoires neuronales
void shadow_panic_purge(void);

/// Nécrose : Simule la mort d'une partie de l'organisme (Data Decoy)
/// Détourne l'attention de l'attaquant vers une zone mémoire factice.
void shadow_necrosis(void* fake_organ_ptr, size_t size);

#endif // OO_STEALTH_H
