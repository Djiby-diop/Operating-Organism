#ifndef OO_PHEROMONES_H
#define OO_PHEROMONES_H

#include <stdint.h>
#include <stddef.h>

/// Initialise l'intelligence collective (Swarm)
void swarm_init(void);

/// Émission de phéromones (Broadcast vers d'autres OO sur le réseau)
/// Permet de partager des anticorps ou des connaissances.
void swarm_emit_pheromone(uint8_t type, const void* data, size_t size);

/// Réception de phéromones externes
void swarm_on_pheromone_received(const uint8_t* sender_dna, const void* data, size_t size);

#endif // OO_PHEROMONES_H
