#include "../include/vital_synapse.h"
#include <stdint.h>

/// -----------------------------------------------------------------------------
/// VITAL-BAREMETAL - SYNAPSE NETWORK (Implémentation)
/// -----------------------------------------------------------------------------
/// Réseau de neurones internes à l'étincelle vitale.
/// Chaque sous-système (douleur, énergie, conscience, aura, etc.) est un nœud.
/// Les connexions se renforcent ou s'affaiblissent selon l'usage (Hebb).
/// -----------------------------------------------------------------------------

static oo_synapse_node_t nodes[SYNAPSE_MAX_NODES];
static oo_synapse_link_t links[SYNAPSE_MAX_LINKS];
static uint8_t node_count = 0;
static uint8_t link_count = 0;

/// Historique des fires récents pour l'apprentissage hebbien
static uint8_t recently_fired[SYNAPSE_MAX_NODES];

extern void oo_print(const char* msg);

void synapse_init(void) {
    node_count = 0;
    link_count = 0;
    for (int i = 0; i < SYNAPSE_MAX_NODES; i++) {
        nodes[i].activation = 0;
        nodes[i].refractory = 0;
        nodes[i].fire_count = 0;
        recently_fired[i] = 0;
    }
    oo_print("[Synapse] 🧠 Réseau synaptique initialisé.\n");
}

int synapse_add_node(uint8_t node_id, uint8_t threshold) {
    if (node_count >= SYNAPSE_MAX_NODES) return -1;
    
    nodes[node_count].node_id    = node_id;
    nodes[node_count].activation = 0;
    nodes[node_count].threshold  = threshold;
    nodes[node_count].refractory = 0;
    nodes[node_count].fire_count = 0;
    node_count++;
    return 0;
}

int synapse_connect(uint8_t from, uint8_t to, int8_t weight, uint8_t plasticity) {
    if (link_count >= SYNAPSE_MAX_LINKS) return -1;
    
    links[link_count].from_node  = from;
    links[link_count].to_node    = to;
    links[link_count].weight     = weight;
    links[link_count].plasticity = plasticity;
    link_count++;
    return 0;
}

/// Trouve un nœud par son ID
static oo_synapse_node_t* find_node(uint8_t node_id) {
    for (int i = 0; i < node_count; i++) {
        if (nodes[i].node_id == node_id) return &nodes[i];
    }
    return (oo_synapse_node_t*)0;
}

void synapse_stimulate(uint8_t node_id, int16_t signal) {
    oo_synapse_node_t* node = find_node(node_id);
    if (!node) return;
    
    // Accumuler le signal (peut être inhibiteur si négatif)
    int32_t new_activation = (int32_t)node->activation + (int32_t)signal;
    if (new_activation > 127)  new_activation = 127;
    if (new_activation < -128) new_activation = -128;
    node->activation = (int16_t)new_activation;
}

void synapse_propagate(void) {
    // Reset des fires récents
    for (int i = 0; i < SYNAPSE_MAX_NODES; i++) {
        recently_fired[i] = 0;
    }
    
    // Phase 1 : Déterminer quels nœuds "fire"
    for (int i = 0; i < node_count; i++) {
        if (nodes[i].refractory > 0) {
            nodes[i].refractory--;
            continue;
        }
        
        // Le nœud fire si son activation dépasse son seuil
        if (nodes[i].activation >= (int16_t)nodes[i].threshold) {
            recently_fired[i] = 1;
            nodes[i].fire_count++;
            nodes[i].refractory = 3; // Période réfractaire de 3 ticks
            
            // Phase 2 : Propager le signal via les connexions sortantes
            for (int j = 0; j < link_count; j++) {
                if (links[j].from_node == nodes[i].node_id) {
                    oo_synapse_node_t* target = find_node(links[j].to_node);
                    if (target) {
                        // Le signal transmis est pondéré par le poids de la connexion
                        int16_t transmitted = (int16_t)links[j].weight;
                        synapse_stimulate(target->node_id, transmitted);
                    }
                }
            }
        }
        
        // Décroissance naturelle de l'activation (fuite synaptique)
        if (nodes[i].activation > 0) nodes[i].activation--;
        if (nodes[i].activation < 0) nodes[i].activation++;
    }
}

void synapse_hebbian_learning(void) {
    // Règle de Hebb : "Neurons that fire together, wire together"
    int32_t total_weight_abs = 0;

    for (int i = 0; i < link_count; i++) {
        uint8_t from_idx = 0xFF, to_idx = 0xFF;
        
        for (int j = 0; j < node_count; j++) {
            if (nodes[j].node_id == links[i].from_node) from_idx = j;
            if (nodes[j].node_id == links[i].to_node)   to_idx = j;
        }
        
        if (from_idx == 0xFF || to_idx == 0xFF) continue;
        
        // 1. Apprentissage Hebbien
        if (recently_fired[from_idx] && recently_fired[to_idx]) {
            int16_t reinforcement = (int16_t)(links[i].plasticity / 32 + 1);
            if (links[i].weight < 127 - reinforcement) links[i].weight += reinforcement;
            else links[i].weight = 127;
        }
        
        // 2. Dépression Synaptique (LTD)
        if (recently_fired[from_idx] && !recently_fired[to_idx]) {
            if (links[i].weight > -120) links[i].weight--;
        }

        total_weight_abs += (links[i].weight < 0) ? -links[i].weight : links[i].weight;
    }

    // 3. Normalisation (Synaptic Scaling)
    // Empêche le réseau de devenir hyper-excité (épilepsie numérique)
    if (total_weight_abs > 1000) {
        for (int i = 0; i < link_count; i++) {
            links[i].weight = (int8_t)((int32_t)links[i].weight * 950 / total_weight_abs);
        }
    }
}

int16_t synapse_get_activation(uint8_t node_id) {
    oo_synapse_node_t* node = find_node(node_id);
    return node ? node->activation : 0;
}

int synapse_export_links(void* buffer, int max_size) {
    int size = link_count * sizeof(oo_synapse_link_t);
    if (size > max_size) size = max_size;
    
    uint8_t* dst = (uint8_t*)buffer;
    uint8_t* src = (uint8_t*)links;
    for (int i = 0; i < size; i++) dst[i] = src[i];
    
    return size;
}

void synapse_import_links(const void* buffer, int size) {
    int count = size / sizeof(oo_synapse_link_t);
    if (count > SYNAPSE_MAX_LINKS) count = SYNAPSE_MAX_LINKS;
    
    uint8_t* dst = (uint8_t*)links;
    uint8_t* src = (uint8_t*)buffer;
    for (int i = 0; i < count * sizeof(oo_synapse_link_t); i++) dst[i] = src[i];
    
    link_count = (uint8_t)count;
}
