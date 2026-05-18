#include "../include/dna_hash.h"
#include "../../united-baremetal/include/united_bus.h"

#define MAX_TRUSTED_DNA 64

static oo_dna_signature_t trusted_genome[MAX_TRUSTED_DNA];
static int trusted_count = 0;

extern void oo_print(const char* msg);

// FNV-1a Hash (64-bit simplified to 32-bit for the ADN signature)
static void identity_hash_fnv1a(const uint8_t* data, size_t size, uint8_t* out_hash) {
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < size; i++) {
        hash ^= data[i];
        hash *= 0x100000001b3ULL;
    }
    // On remplit le buffer de 32 octets avec des variations du hash
    for (int i = 0; i < 32; i += 8) {
        uint64_t sub_hash = hash ^ (i * 0x9E3779B97F4A7C15ULL);
        for (int j = 0; j < 8; j++) {
            out_hash[i + j] = (sub_hash >> (j * 8)) & 0xFF;
        }
    }
}

void identity_init(void) {
    trusted_count = 0;
    oo_print("[IdentityBaremetal] 👤 Glande du Thymus activee. Generation du Genome de base...\n");
    // En situation réelle, on interroge le PCR0 du TPM (Trusted Platform Module)
}

void identity_calculate_dna(const void* memory_region, size_t size, oo_dna_signature_t* out_signature) {
    if (!out_signature) return;
    identity_hash_fnv1a((const uint8_t*)memory_region, size, out_signature->hash);
}

int identity_is_self(const oo_dna_signature_t* signature) {
    for (int i = 0; i < trusted_count; i++) {
        int match = 1;
        for (int j = 0; j < 32; j++) {
            if (trusted_genome[i].hash[j] != signature->hash[j]) {
                match = 0;
                break;
            }
        }
        if (match) return 1; // Fait partie du Soi
    }
    
    // Alerte le Bot-Baremetal (Non-Soi détecté !)
    globule_t alert;
    alert.type = GLOBULE_WHITE;
    alert.target_organ = 1; // ORGAN_TYPE_IMMUNE
    alert.payload_addr = (void*)signature;
    united_bus_pump(alert);
    
    return 0; // Corps étranger
}

void identity_trust_dna(const oo_dna_signature_t* new_signature) {
    if (trusted_count < MAX_TRUSTED_DNA) {
        for (int i = 0; i < 32; i++) {
            trusted_genome[trusted_count].hash[i] = new_signature->hash[i];
        }
        trusted_count++;
        oo_print("[IdentityBaremetal] 🧬 Nouvelle sequence ADN incorporee (Evolution du Soi).\n");
    }
}
