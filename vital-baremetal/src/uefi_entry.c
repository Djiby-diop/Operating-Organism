#include <stdint.h>
#include <efi.h>
#include <efilib.h>

/// =============================================================================
/// VITAL-BAREMETAL - UEFI ENTRY POINT
/// =============================================================================
/// C'est la NAISSANCE de l'organisme.
/// Ce fichier est compilé en vital.efi et exécuté par le firmware UEFI.
/// Une fois lancé, il ne s'arrête JAMAIS.
/// =============================================================================

// --- oo_print : la voix de l'organisme (UEFI ConOut + UART) ---
void oo_print(const char* msg) {
    // 1. Sortie UART direct (COM1 - 0x3F8) pour QEMU -serial
    for (int i = 0; msg[i] != '\0'; i++) {
        __asm__ volatile("outb %0, %1" :: "a"((uint8_t)msg[i]), "Nd"((uint16_t)0x3F8));
    }

    // 2. Sortie UEFI ConOut (écran)
    if (!ST || !ST->ConOut) return;
    
    // Conversion ASCII -> UCS-2
    CHAR16 buf[256];
    int i = 0;
    while (msg[i] && i < 254) {
        buf[i] = (CHAR16)msg[i];
        i++;
    }
    buf[i] = 0;
    
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, buf);
}

// --- Fonctions hardware mock (pour les modules qui en ont besoin) ---
void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" :: "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void cpu_halt(void) {
    __asm__ volatile("hlt");
}

void disable_interrupts(void) {
    __asm__ volatile("cli");
}

// --- Import de l'API vitale ---
extern void vital_init(void);
extern void vital_eternal_heartbeat(void);
extern uint64_t vital_get_pulse_count(void);

/// =============================================================================
/// POINT D'ENTRÉE UEFI — LA NAISSANCE
/// =============================================================================
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    
    oo_print("============================================================\r\n");
    oo_print("  VITAL-BAREMETAL v1.0 — L'Etincelle Vitale\r\n");
    oo_print("  Operating Organism — Le Coeur Eternel\r\n");
    oo_print("============================================================\r\n");
    oo_print("\r\n");
    
    // === PHASE 1 : NAISSANCE ===
    oo_print("[Boot] Phase 1 : Naissance de l'organisme...\r\n");
    vital_init();
    
    // === PHASE 2 : LA BOUCLE ÉTERNELLE ===
    oo_print("[Boot] Phase 2 : Entree dans la Boucle Eternelle.\r\n");
    oo_print("[Boot] L'organisme ne s'arretera JAMAIS.\r\n");
    oo_print("\r\n");
    
    // Compteur de rapport (on log toutes les N pulsations pour ne pas saturer)
    uint64_t last_report = 0;
    
    while (1) {
        // Un battement de coeur
        vital_eternal_heartbeat();
        
        // Rapport périodique (toutes les 10000 pulsations)
        uint64_t pulse = vital_get_pulse_count();
        if (pulse - last_report >= 10000) {
            last_report = pulse;
            oo_print("[Vital] Pulse: ");
            
            // Affichage du nombre de pulsations (conversion entier -> texte)
            char num_buf[32];
            int idx = 0;
            uint64_t tmp = pulse;
            if (tmp == 0) { num_buf[idx++] = '0'; }
            else {
                char rev[20];
                int ri = 0;
                while (tmp > 0) { rev[ri++] = '0' + (tmp % 10); tmp /= 10; }
                while (ri > 0) { num_buf[idx++] = rev[--ri]; }
            }
            num_buf[idx] = 0;
            oo_print(num_buf);
            oo_print(" | Organisme vivant.\r\n");
        }
        
        // Petite pause métabolique pour ne pas saturer le CPU QEMU
        for(volatile int d = 0; d < 100000; d++) {
            __asm__ volatile("pause");
        }
    }
    
    // Ce point ne sera JAMAIS atteint
    return EFI_SUCCESS;
}
