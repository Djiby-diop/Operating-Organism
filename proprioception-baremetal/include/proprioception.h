#ifndef OO_PROPRIOCEPTION_H
#define OO_PROPRIOCEPTION_H

#include <stdint.h>

/// Initialise la conscience corporelle
void proprioception_init(void);

/// Vérifie l'intégrité de la posture de l'organisme
/// Scanne les piles et les zones mémoires critiques
void proprioception_check_posture(void);

#endif // OO_PROPRIOCEPTION_H
