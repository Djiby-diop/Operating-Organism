#ifndef OO_LUNGS_H
#define OO_LUNGS_H

#include <stdint.h>
#include <stddef.h>

/// Initialise le système respiratoire (Carte réseau / Pile TCP/IP)
void network_init(void);

/// Inspiration : Réception d'un paquet brut depuis le hardware (NIC)
void network_inhale(const uint8_t* frame, size_t size);

/// Expiration : Envoi de données vers l'extérieur
void network_exhale(const uint8_t* data, size_t size);

/// Types de respiration
typedef enum {
    BREATH_INHALED_DATA,
    BREATH_INHALED_CONTROL,
    BREATH_EXHALED_REPLY
} oo_breath_type_t;

/// État de la respiration (Link status, congestion)
typedef struct {
    uint8_t link_up;
    uint32_t breath_rate; // Packets per second
} oo_respiration_stats_t;

void network_get_stats(oo_respiration_stats_t* stats);

#endif // OO_LUNGS_H
