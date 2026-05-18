#ifndef OO_SENSORY_RECEPTORS_H
#define OO_SENSORY_RECEPTORS_H

#include <stdint.h>

typedef enum {
    STIMULUS_TOUCH,
    STIMULUS_VISION,
    STIMULUS_THERMAL,
    STIMULUS_AUDITORY,
    STIMULUS_SERIAL,   /* UART / serial data stream */
    STIMULUS_TIMER,    /* periodic heartbeat tick   */
    STIMULUS_NET       /* network packet arrival    */
} oo_stimulus_type_t;

typedef struct {
    oo_stimulus_type_t type;
    uint32_t intensity; // 0-100
    uint64_t timestamp;
    uint8_t raw_data[16];
} oo_stimulus_t;

/// Initialise l'ensemble des récepteurs sensoriels matériels
void sense_init(void);

/// --- LE TOUCHER (Clavier / Souris / I2C) ---
/// Convertit une frappe clavier brute en influx nerveux (Globule Rouge)
void sense_transduce_keystroke(uint8_t scancode);

/// --- LA RÉTINE (Vision / Écran) ---
/// Met à jour la perception visuelle de l'organisme (Framebuffer / Texte)
void sense_update_retina(const char* visual_buffer, uint32_t size);

/// --- THERMOCEPTION (Douleur / Chaleur) ---
/// Lit la température des cœurs CPU. Déclenche le reflexe si > Seuil.
void sense_feel_temperature(void);

/// --- UART / SERIAL ---
/// Transmet une trame série comme influx nerveux vers le cortex (Globule Rouge)
void sense_transduce_serial(const char* data, uint32_t len);

/// --- TIMER TICK ---
/// Émet un Globule Jaune broadcast à chaque tick horloge
void sense_transduce_timer(uint64_t tick);

/// Retourne le dernier stimulus capturé
oo_stimulus_t sense_get_last_stimulus(void);

#endif // OO_SENSORY_RECEPTORS_H
