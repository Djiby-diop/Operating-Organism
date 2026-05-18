#include "../include/vital_spark.h"
#include <stdint.h>

/// =============================================================================
/// VITAL-BAREMETAL - PINEAL GLAND (Hardware RTC Link)
/// =============================================================================
/// La Glande Pinéale de l'Operating Organism.
/// Cette couche lie la boucle infinie du CPU avec le cycle solaire du
/// monde physique réel via le CMOS (Real Time Clock).
/// L'humeur du système dépend donc de l'heure du monde réel.
/// =============================================================================

// Ports standards du RTC
#define CMOS_ADDRESS_PORT 0x70
#define CMOS_DATA_PORT    0x71

// Registres RTC
#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x02
#define RTC_HOURS   0x04

extern void outb(uint16_t port, uint8_t val);
extern uint8_t inb(uint16_t port);

static int get_update_in_progress_flag(void) {
    outb(CMOS_ADDRESS_PORT, 0x0A);
    return (inb(CMOS_DATA_PORT) & 0x80);
}

static uint8_t get_rtc_register(int reg) {
    outb(CMOS_ADDRESS_PORT, reg);
    return inb(CMOS_DATA_PORT);
}

/// Convertit le format BCD (Binary Coded Decimal) en binaire
static uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0x0F);
}

void vital_pineal_gland_sync(void) {
    // Attend que le RTC soit stable
    while (get_update_in_progress_flag());
    
    uint8_t hours = get_rtc_register(RTC_HOURS);
    uint8_t reg_b = get_rtc_register(0x0B);
    
    // Si le RTC est en mode BCD, on le convertit
    if (!(reg_b & 0x04)) {
        hours = bcd_to_binary(hours);
    }
    
    // Si le RTC est en mode 12h et que c'est PM, on ajuste
    if (!(reg_b & 0x02) && (hours & 0x80)) {
        hours = ((hours & 0x7F) + 12) % 24;
    }
    
    // -------------------------------------------------------------
    // ALIGNEMENT CIRCADIEN PHYSIQUE
    // -------------------------------------------------------------
    // L'heure réelle influence l'équilibre Yin/Yang
    
    if (hours >= 6 && hours < 12) {
        // Matin (Aube -> Zenith) : Forte poussée Yang (Éveil)
        vital_shift_balance(FORCE_YANG, 20);
        oo_print("[Pineal] Synchronisation RTC: Matin detecte. Stimulation Yang.\n");
    } 
    else if (hours >= 12 && hours < 18) {
        // Après-midi (Zenith) : Régulation de maintien Yang
        vital_shift_balance(FORCE_YANG, 5);
    }
    else if (hours >= 18 && hours < 22) {
        // Soir (Crépuscule) : Le Yin commence à s'installer
        vital_shift_balance(FORCE_YIN, 15);
        oo_print("[Pineal] Synchronisation RTC: Crepuscule detecte. MonTEE Yin.\n");
    }
    else {
        // Nuit (Nadir) : Phase onirique et réparatrice pure Yin
        vital_shift_balance(FORCE_YIN, 30);
    }
}
