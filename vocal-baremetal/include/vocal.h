#ifndef OO_VOCAL_H
#define OO_VOCAL_H

#include <stdint.h>

/// Initialise les cordes vocales
void vocal_init(void);

/// Émet un son via le PC Speaker (Fréquence en Hz)
void vocal_emit_tone(uint32_t frequency, uint32_t duration_ms);

/// Chante un log via le port série
void vocal_speak(const char* message);

#endif // OO_VOCAL_H
