/**
 * BOT-BAREMETAL — Threat Levels
 * Définition des niveaux de menace et transitions d'état du système.
 *
 * Le système immunitaire de l'OO vit dans l'un de ces 6 états.
 * Les transitions sont strictement définies.
 */

#ifndef THREAT_LEVELS_H
#define THREAT_LEVELS_H

#include <stdint.h>

/* ─── Définition des États ───────────────────────────────────────────── */

/**
 * DORMANT (0) — Veille
 *
 * Le Bot est en surveillance passive ultra-légère.
 * Consommation CPU et mémoire minimale.
 * Agents de surveillance actifs en mode économique.
 *
 * Transitions possibles :
 *   → VIGILANCE si anomalie détectée
 */
#define THREAT_DORMANT          0

/**
 * VIGILANCE (1) — Alerte Faible
 *
 * Une activité inhabituelle a été détectée.
 * Peut être légitime (nouvelle app, mise à jour, activité réseau normale).
 * Le Bot augmente sa surveillance sans agir.
 * HoneyTrap peut être pré-activé discrètement.
 *
 * Transitions possibles :
 *   → DORMANT si l'activité se normalise
 *   → ALERT si le comportement persiste ou s'intensifie
 */
#define THREAT_VIGILANCE        1

/**
 * ALERT (2) — Alerte Haute
 *
 * Le comportement est clairement suspect.
 * Pattern partiellement reconnu ou heuristique forte.
 * Quarantine préventive possible.
 * MimicryAgent activé pour capturer les techniques.
 * Tous les agents en stand-by actif.
 *
 * Transitions possibles :
 *   → VIGILANCE si la menace n'est pas confirmée après analyse
 *   → COMBAT si la menace est confirmée
 */
#define THREAT_ALERT            2

/**
 * COMBAT (3) — Combat
 *
 * Intrusion confirmée par preuve mesurable.
 * SwarmMind coordonne toute la flotte.
 * Neutralizer autorisé à agir.
 * MimicryAgent capture en temps réel.
 * OOBridgeAgent notifie le LLM-Baremetal.
 * Journalisation complète activée.
 *
 * Transitions possibles :
 *   → ALERT si la menace est contenue mais pas encore éliminée
 *   → DORMANT si la menace est neutralisée et système propre
 *   → SURVIVAL si le Bot lui-même est attaqué
 */
#define THREAT_COMBAT           3

/**
 * SURVIVAL (4) — Survie Critique
 *
 * Le Bot lui-même est ciblé.
 * Priorité absolue : survivre avant de protéger.
 * RegenAgent en priorité maximale.
 * ChameleonAgent actif (invisibilité maximale).
 * Agents non essentiels suspendus pour économiser les ressources.
 * OOBridgeAgent : alerte critique au LLM.
 *
 * Transitions possibles :
 *   → COMBAT une fois la menace contre le Bot contenue
 *   → CONFINEMENT si la menace est existentielle pour l'OO
 */
#define THREAT_SURVIVAL         4

/**
 * CONFINEMENT (5) — Confinement Total
 *
 * Menace existentielle pour l'Operating Organism.
 * Isolation complète du système.
 * Préservation de l'ADN du Bot (snapshot complet).
 * Journalisation maximale pour analyse post-mortem.
 * Coupure réseau totale.
 * Aucune action offensive — pure survie et préservation.
 *
 * Transitions possibles :
 *   → SURVIVAL après validation que le cœur OO est intact
 */
#define THREAT_CONFINEMENT      5

/* ─── Structure d'État du Système ───────────────────────────────────── */

typedef struct {
    uint8_t     current_level;          /* Niveau actuel (0-5)          */
    uint8_t     previous_level;         /* Niveau précédent             */
    uint64_t    level_entered_at;       /* Timestamp d'entrée           */
    uint64_t    transition_count;       /* Nombre de transitions totales*/

    /* Raison de la dernière transition */
    char        reason[128];

    /* Statistiques par niveau */
    uint64_t    time_in_level[6];       /* Temps total passé (ns)       */
    uint32_t    entries_per_level[6];   /* Nombre d'entrées par niveau  */
} ThreatState;

/* ─── Fonctions de Transition ───────────────────────────────────────── */

/**
 * Initialise l'état de menace (commence en DORMANT).
 */
void threat_state_init(ThreatState *state);

/**
 * Tente une transition vers un nouveau niveau.
 * Vérifie la validité de la transition.
 * Retourne 1 si la transition est autorisée, 0 sinon.
 *
 * Note : Une transition de DORMANT à COMBAT directement
 * est INTERDITE. L'instinct force toujours le passage
 * par les états intermédiaires sauf en SURVIVAL/CONFINEMENT.
 */
int threat_transition(ThreatState *state, uint8_t new_level, const char *reason);

/**
 * Retourne 1 si une transition est valide selon la matrice de transition.
 */
int threat_transition_is_valid(uint8_t from, uint8_t to);

/**
 * Affiche le résumé de l'état de menace.
 */
void threat_state_print(const ThreatState *state);

/* ─── Matrice de Transition (règles strictes) ──────────────────────── */

/*
 * Matrice de transitions autorisées :
 *
 *          TO:  0    1    2    3    4    5
 * FROM 0:       -    ✓    ✗    ✗    ✗    ✗
 * FROM 1:       ✓    -    ✓    ✗    ✗    ✗
 * FROM 2:       ✗    ✓    -    ✓    ✗    ✗
 * FROM 3:       ✓    ✗    ✓    -    ✓    ✓
 * FROM 4:       ✗    ✗    ✗    ✓    -    ✓
 * FROM 5:       ✗    ✗    ✗    ✗    ✓    -
 *
 * Le système ne peut pas sauter plusieurs niveaux
 * sauf transitions d'urgence COMBAT→SURVIVAL/CONFINEMENT.
 *
 * Cette règle prévient les faux positifs catastrophiques.
 */

#define THREAT_TRANSITION_MATRIX { \
    /* 0→ */ {0, 1, 0, 0, 0, 0}, \
    /* 1→ */ {1, 0, 1, 0, 0, 0}, \
    /* 2→ */ {0, 1, 0, 1, 0, 0}, \
    /* 3→ */ {1, 0, 1, 0, 1, 1}, \
    /* 4→ */ {0, 0, 0, 1, 0, 1}, \
    /* 5→ */ {0, 0, 0, 0, 1, 0}, \
}

/* ─── Macros de Vérification Rapide ────────────────────────────────── */

#define THREAT_IS_ACTIVE(state)     ((state)->current_level >= THREAT_ALERT)
#define THREAT_IS_CRITICAL(state)   ((state)->current_level >= THREAT_SURVIVAL)
#define THREAT_IS_COMBAT(state)     ((state)->current_level == THREAT_COMBAT)
#define THREAT_IS_DORMANT(state)    ((state)->current_level == THREAT_DORMANT)

#endif /* THREAT_LEVELS_H */
