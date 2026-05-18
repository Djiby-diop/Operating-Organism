/**
 * BOT-BAREMETAL — ThreatState Implementation
 * Gestion des transitions de niveau de menace.
 */

#include "threat_levels.h"
#include <string.h>
#include <stdio.h>

/* Matrice de transitions autorisées [from][to] */
static const uint8_t TRANSITION_MATRIX[6][6] = {
    /* TO: 0  1  2  3  4  5  */
    {0, 1, 0, 0, 0, 0},   /* FROM 0: DORMANT    */
    {1, 0, 1, 0, 0, 0},   /* FROM 1: VIGILANCE  */
    {0, 1, 0, 1, 0, 0},   /* FROM 2: ALERT      */
    {1, 0, 1, 0, 1, 1},   /* FROM 3: COMBAT     */
    {0, 0, 0, 1, 0, 1},   /* FROM 4: SURVIVAL   */
    {0, 0, 0, 0, 1, 0},   /* FROM 5: CONFINEMENT*/
};

static const char *LEVEL_NAMES[6] = {
    "DORMANT", "VIGILANCE", "ALERT", "COMBAT", "SURVIVAL", "CONFINEMENT"
};

void threat_state_init(ThreatState *state) {
    if (!state) return;
    memset(state, 0, sizeof(ThreatState));
    state->current_level  = THREAT_DORMANT;
    state->previous_level = THREAT_DORMANT;
    snprintf(state->reason, sizeof(state->reason), "boot");
}

int threat_transition_is_valid(uint8_t from, uint8_t to) {
    if (from > 5 || to > 5) return 0;
    return TRANSITION_MATRIX[from][to];
}

int threat_transition(ThreatState *state, uint8_t new_level, const char *reason) {
    if (!state) return 0;
    if (new_level > 5) return 0;
    if (new_level == state->current_level) return 1;

    if (!threat_transition_is_valid(state->current_level, new_level)) {
        fprintf(stderr, "[ThreatState] BLOCKED: %s → %s (invalid transition)\n",
                LEVEL_NAMES[state->current_level], LEVEL_NAMES[new_level]);
        return 0;
    }

    fprintf(stderr, "[ThreatState] %s → %s | %s\n",
            LEVEL_NAMES[state->current_level],
            LEVEL_NAMES[new_level],
            reason ? reason : "");

    state->previous_level = state->current_level;
    state->current_level  = new_level;
    state->transition_count++;
    state->entries_per_level[new_level]++;

    if (reason) {
        strncpy(state->reason, reason, sizeof(state->reason) - 1);
        state->reason[sizeof(state->reason) - 1] = '\0';
    }

    return 1;
}

void threat_state_print(const ThreatState *state) {
    if (!state) return;
    printf("[ThreatState] Level=%s (was=%s) transitions=%llu\n",
           LEVEL_NAMES[state->current_level],
           LEVEL_NAMES[state->previous_level],
           (unsigned long long)state->transition_count);
    printf("  Reason: %s\n", state->reason);
    printf("  Entries per level: ");
    for (int i = 0; i < 6; i++)
        printf("%s=%u ", LEVEL_NAMES[i], state->entries_per_level[i]);
    printf("\n");
}
