; -----------------------------------------------------------------------------
; VITAL-BAREMETAL - THE STEEL HEARTBEAT (x86_64)
; -----------------------------------------------------------------------------

[BITS 64]

section .text
global vital_steel_tick

; void vital_steel_tick(uint64_t* p_pulse, uint64_t* p_jitter)
vital_steel_tick:
    push rbp
    mov rbp, rsp

    mov rax, [rdi]
    inc rax
    mov [rdi], rax

    rdtsc
    mov r8, rax
    mov rcx, 50
.burn:
    pause
    loop .burn
    rdtsc
    sub rax, r8
    add [rsi], rax

    rdrand rax

    pop rbp
    ret

; GNU stack note — no executable stack
section .note.GNU-stack noalloc noexec nowrite progbits
