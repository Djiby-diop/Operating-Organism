#ifndef VITAL_SYNAPSE_H
#define VITAL_SYNAPSE_H

#include <stdint.h>

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - SYNAPSE NETWORK
/// -----------------------------------------------------------------------------
/// Un réseau de connexions pondérées entre les sous-systèmes internes du Vital.
/// Contrairement au bus United (qui relie les organes entre eux),
/// ce réseau relie les neurones INTERNES de l'étincelle vitale.
/// Les poids évoluent par apprentissage hebbien ("ce qui fire ensemble, wire ensemble").
/// -----------------------------------------------------------------------------

#define SYNAPSE_MAX_NODES    16
#define SYNAPSE_MAX_LINKS    64

/// Un nœud du réseau interne
typedef struct {
    uint8_t  node_id;
    int16_t  activation;     // -128 à +127 (inhibiteur ou excitateur)
    uint8_t  threshold;      // Seuil de déclenchement
    uint8_t  refractory;     // Période réfractaire restante (ne peut pas re-fire)
    uint32_t fire_count;     // Nombre total de déclenchements
} oo_synapse_node_t;

/// Une connexion pondérée entre deux nœuds
typedef struct {
    uint8_t from_node;
    uint8_t to_node;
    int8_t  weight;          // -128 (inhibiteur fort) à +127 (excitateur fort)
    uint8_t plasticity;      // Capacité d'apprentissage 0-255
} oo_synapse_link_t;

/// Initialise le réseau synaptique
void synapse_init(void);

/// Ajoute un nœud au réseau
int synapse_add_node(uint8_t node_id, uint8_t threshold);

/// Connecte deux nœuds avec un poids initial
int synapse_connect(uint8_t from, uint8_t to, int8_t weight, uint8_t plasticity);

/// Stimule un nœud (injection de signal externe)
void synapse_stimulate(uint8_t node_id, int16_t signal);

/// Propage les signaux dans tout le réseau (un tick de simulation neuronale)
void synapse_propagate(void);

/// Apprentissage Hebbien : renforce les connexions qui fire ensemble
void synapse_hebbian_learning(void);

/// Retourne l'activation d'un nœud
int16_t synapse_get_activation(uint8_t node_id);

/// --- PERSISTENCE ---
int synapse_export_links(void* buffer, int max_size);
void synapse_import_links(const void* buffer, int size);

#endif // VITAL_SYNAPSE_H
