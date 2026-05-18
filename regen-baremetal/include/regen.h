#ifndef OO_REGEN_H
#define OO_REGEN_H

#include <stdint.h>

/// Initialise les cellules souches
void regen_init(void);

/// Patch une fonction en mémoire vive
/// @param target_addr L'adresse de la fonction à remplacer
/// @param replacement_addr L'adresse de la nouvelle fonction
void regen_hotpatch(void* target_addr, void* replacement_addr);

#endif // OO_REGEN_H
