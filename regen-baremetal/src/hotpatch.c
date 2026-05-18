#include "../include/regen.h"
#include <stdint.h>

extern void oo_print(const char* msg);

void regen_init(void) {
    oo_print("[Regen] Cellules souches pretes pour reparation.\n");
}

void regen_hotpatch(void* target_addr, void* replacement_addr) {
    // Technique de détournement par JMP relatif (x86_64)
    // On remplace les 5 premiers octets de target_addr par :
    // E9 <offset_32_bit>
    
    uint8_t* target = (uint8_t*)target_addr;
    int32_t offset = (int32_t)((int64_t)replacement_addr - (int64_t)target_addr - 5);
    
    oo_print("[Regen] Hotpatching fonction... Mutation a chaud.\n");
    
    // Dans un vrai OS, il faudrait déprotéger la page (WP bit dans CR0)
    target[0] = 0xE9; // JMP
    *(int32_t*)(target + 1) = offset;
    
    // Invalidation du cache d'instructions (CPU Flush)
    // __asm__ volatile("wbinvd" ::: "memory");
}
