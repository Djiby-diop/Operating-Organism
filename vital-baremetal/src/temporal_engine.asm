; =============================================================================
; VITAL-BAREMETAL - TEMPORAL ENGINE (x86_64 SSE/AVX)
; =============================================================================
; Ce module mesure la "Flèche du Temps" de l'organisme.
; Il détecte les anomalies temporelles (ralentissements, accélérations anormales)
; et protège l'organisme contre les attaques de timing.
; =============================================================================

[BITS 64]

section .data
align 64
temporal_baseline   dq 0       ; Valeur TSC de référence
temporal_jitter     dq 0       ; Accumulation du jitter
temporal_anomalies  dq 0       ; Compteur d'anomalies temporelles

; Masque AVX pour détection parallèle d'anomalies
align 32
jitter_threshold    dq 50000, 50000, 50000, 50000

section .text
global vital_temporal_tick
global vital_temporal_get_jitter
global vital_temporal_reset

vital_temporal_tick:
    ; --- LECTURE DU TEMPS (Cycle précis) ---
    lfence                      ; Barrière mémoire (anti-spéculation)
    rdtsc
    lfence
    shl rdx, 32
    or  rax, rdx                ; RAX = TSC 64-bit

    ; --- CALCUL DU DELTA ---
    mov rcx, [rel temporal_baseline]
    cmp rcx, 0
    je  .init_baseline          ; Premier appel

    sub rax, rcx                ; Delta TSC
    
    ; --- DETECTION D'ANOMALIE TEMPORELLE ---
    ; Si le delta est hors norme (trop grand = ralentissement, trop petit = saut)
    mov rcx, 100000             ; Seuil normal max
    cmp rax, rcx
    ja  .anomaly_detected       ; Saut temporel anormal
    
    mov rcx, 100                ; Seuil normal min
    cmp rax, rcx
    jb  .anomaly_detected       ; Accélération anormale
    
    add [rel temporal_jitter], rax
    jmp .update_baseline

.anomaly_detected:
    inc qword [rel temporal_anomalies]
    ; Émission d'un signal d'urgence (simulé)
    ; En réalité, on appellerait vital_rust_guardian ici

.init_baseline:
.update_baseline:
    rdtsc
    shl rdx, 32
    or  rax, rdx
    mov [rel temporal_baseline], rax
    ret

vital_temporal_get_jitter:
    mov rax, [rel temporal_jitter]
    ret

vital_temporal_reset:
    xor rax, rax
    mov [rel temporal_baseline], rax
    mov [rel temporal_jitter], rax
    ret

; GNU stack note — no executable stack
section .note.GNU-stack noalloc noexec nowrite progbits
