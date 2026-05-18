#include "../include/vocal.h"
#include <stdint.h>

extern void outb(uint16_t port, uint8_t val);
extern uint8_t inb(uint16_t port);
extern void oo_print(const char* msg);

void vocal_init(void) {
    oo_print("[Vocal] Cordes vocales pretes. Port 0x61 arme.\n");
}

void vocal_emit_tone(uint32_t frequency, uint32_t duration_ms) {
    if (frequency == 0) return;
    
    uint32_t div = 1193180 / frequency;
    
    // Commande du timer 8254 PIT
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(div & 0xFF));
    outb(0x42, (uint8_t)((div >> 8) & 0xFF));
    
    // Activation du haut-parleur
    uint8_t tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
    
    // En baremetal pur, on attendrait ici via un timer hardware
    oo_print("[Vocal] 🔊 Emission d'un son d'alerte...\n");
}

void vocal_speak(const char* message) {
    // Redirection vers le port série UART
    oo_print(message);
}
