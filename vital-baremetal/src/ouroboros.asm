; -----------------------------------------------------------------------------
; VITAL-BAREMETAL - THE OUROBOROS (Couche 6 - Circular Watchdog)
; -----------------------------------------------------------------------------

[BITS 64]
section .text
global vital_ouroboros_check
extern get_organism_vitality ; Rust
extern vital_steel_tick ; Asm self

vital_ouroboros_check:
    ; 1. Verifie si le Guardian Rust est en vie
    call get_organism_vitality
    cmp rax, 0
    jne .rust_alive
    
    ; Si Rust est mort, on tente une reanimation d'urgence
    ; (C'est ici qu'on appellerait la couche Regen)
    mov rdi, 0xBAD00001
    jmp .panic_reboot

.rust_alive:
    ; 2. Verifie si le compteur de pulsations avance
    ; Si le coeur d'acier s'est fige, on force un saut vers son entree
    mov rdx, [rel vital_pulse_counter]
    pause
    cmp rdx, [rel vital_pulse_counter]
    je .heart_frozen
    
    ret

.heart_frozen:
    ; Note: vital_steel_tick attend des arguments, mais ici on reset brutalement
    jmp vital_steel_tick ; Re-allumage du coeur

.panic_reboot:
    ; Logique de dernier recours
    ud2 ; Genere une exception non-valide pour forcer un handler de secours

global vital_ouroboros_panic
vital_ouroboros_panic:
    cli
.die:
    hlt
    jmp .die
    
section .data
vital_pulse_counter dq 0

; GNU stack note — no executable stack
section .note.GNU-stack noalloc noexec nowrite progbits
