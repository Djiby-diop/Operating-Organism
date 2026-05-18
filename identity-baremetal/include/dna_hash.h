#ifndef OO_DNA_HASH_H
#define OO_DNA_HASH_H

#include <stdint.h>
#include <stddef.h>

/// Signature ADN d'un organe (Hash cryptographique)
typedef struct {
    uint8_t hash[32]; // SHA-256
} oo_dna_signature_t;

/// Initialise la glande du Thymus (Validation TPM)
void identity_init(void);

/// Calcule l'ADN (Hash) d'une région mémoire
void identity_calculate_dna(const void* memory_region, size_t size, oo_dna_signature_t* out_signature);

/// Compare une signature mémoire avec le génome officiel (Le "Soi")
/// Retourne 1 si l'organe appartient à l'organisme, 0 si c'est un corps étranger (Virus)
int identity_is_self(const oo_dna_signature_t* signature);

/// Enregistre une nouvelle signature de confiance (ex: après une mise à jour d'évolution)
void identity_trust_dna(const oo_dna_signature_t* new_signature);

#endif // OO_DNA_HASH_H
