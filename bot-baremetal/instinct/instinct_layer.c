/**
 * BOT-BAREMETAL — InstinctLayer Implementation
 *
 * "L'instinct ne raisonne pas. Il agit. La raison vient après."
 *
 * Cette implémentation est la plus critique du Bot.
 * Chaque fonction doit terminer en < 1ms.
 * Zéro allocation dynamique. Zéro appel bloquant.
 */

#include "instinct_layer.h"
#include <string.h>
#include <stdio.h>

/* ─── Table de Réaction Instinctive ─────────────────────────────────── */
/*
 * Pour chaque (TriggerType, ThreatLevel) → ReactionType recommandée.
 * C'est la "moelle épinière" du Bot : un simple lookup table.
 * Pas de raisonnement. Pas de LLM. Que de la réaction pure.
 */

typedef struct {
    TriggerType  trigger;
    uint8_t      min_threat_level; /* Niveau minimum pour cette réaction */
    uint32_t     min_confidence;   /* Confiance minimum requise          */
    ReactionType reaction;
    uint8_t      new_threat_level; /* Niveau de menace recommandé après  */
} InstinctRule;

static const InstinctRule INSTINCT_RULES[] = {
    /* ── Mémoire ── */
    { TRIGGER_MEM_WRITE_EXEC,      0, 60, REACTION_INTENSIFY_WATCH,   THREAT_VIGILANCE  },
    { TRIGGER_MEM_WRITE_EXEC,      1, 75, REACTION_CAPTURE_MEMORY,    THREAT_ALERT      },
    { TRIGGER_MEM_SHELLCODE_SIG,   0, 70, REACTION_QUARANTINE_PROC,   THREAT_ALERT      },
    { TRIGGER_MEM_SHELLCODE_SIG,   2, 85, REACTION_LOG_FULL,          THREAT_COMBAT     },
    { TRIGGER_MEM_SPRAY,           0, 65, REACTION_INTENSIFY_WATCH,   THREAT_VIGILANCE  },
    { TRIGGER_MEM_ROP_CHAIN,       1, 70, REACTION_CAPTURE_MEMORY,    THREAT_ALERT      },

    /* ── Processus ── */
    { TRIGGER_PROC_HOLLOW,         0, 75, REACTION_QUARANTINE_PROC,   THREAT_ALERT      },
    { TRIGGER_PROC_HOLLOW,         2, 85, REACTION_LOG_FULL,          THREAT_COMBAT     },
    { TRIGGER_PROC_INJECT,         0, 70, REACTION_QUARANTINE_PROC,   THREAT_ALERT      },
    { TRIGGER_PROC_PRIV_ESC,       0, 65, REACTION_INTENSIFY_WATCH,   THREAT_ALERT      },
    { TRIGGER_PROC_PRIV_ESC,       2, 80, REACTION_QUARANTINE_PROC,   THREAT_COMBAT     },
    { TRIGGER_PROC_ABNORMAL_CHILD, 0, 60, REACTION_INTENSIFY_WATCH,   THREAT_VIGILANCE  },
    { TRIGGER_PROC_DEBUGGER,       0, 75, REACTION_CAMOUFLAGE_SELF,   THREAT_ALERT      },
    { TRIGGER_PROC_DEBUGGER,       2, 85, REACTION_ALERT_SWARM_MIND,  THREAT_COMBAT     },

    /* ── Réseau ── */
    { TRIGGER_NET_C2_BEACON,       0, 70, REACTION_DEPLOY_HONEYTRAP,  THREAT_ALERT      },
    { TRIGGER_NET_C2_BEACON,       2, 85, REACTION_CUT_NETWORK,       THREAT_COMBAT     },
    { TRIGGER_NET_EXFIL_VOLUME,    0, 65, REACTION_LOG_FULL,          THREAT_ALERT      },
    { TRIGGER_NET_EXFIL_VOLUME,    2, 80, REACTION_CUT_NETWORK,       THREAT_COMBAT     },
    { TRIGGER_NET_UNUSUAL_PORT,    0, 55, REACTION_INTENSIFY_WATCH,   THREAT_VIGILANCE  },
    { TRIGGER_NET_DNS_TUNNEL,      0, 70, REACTION_DEPLOY_HONEYTRAP,  THREAT_ALERT      },

    /* ── Fichiers ── */
    { TRIGGER_FS_MASS_ENCRYPT,     0, 80, REACTION_QUARANTINE_PROC,   THREAT_COMBAT     },
    { TRIGGER_FS_MASS_ENCRYPT,     3, 90, REACTION_CUT_NETWORK_ALL,   THREAT_COMBAT     },
    { TRIGGER_FS_CRITICAL_MODIF,   0, 75, REACTION_ALERT_SWARM_MIND,  THREAT_ALERT      },
    { TRIGGER_FS_BOOT_MODIF,       0, 80, REACTION_ALERT_SWARM_MIND,  THREAT_COMBAT     },
    { TRIGGER_FS_RAPID_DELETE,     0, 70, REACTION_QUARANTINE_PROC,   THREAT_ALERT      },

    /* ── Kernel ── */
    { TRIGGER_KERN_SYSCALL_HOOK,   0, 80, REACTION_ALERT_SWARM_MIND,  THREAT_COMBAT     },
    { TRIGGER_KERN_ROOTKIT_SIG,    0, 75, REACTION_ALERT_SWARM_MIND,  THREAT_COMBAT     },
    { TRIGGER_KERN_DKOM,           0, 85, REACTION_CUT_NETWORK_ALL,   THREAT_COMBAT     },

    /* ── Survie du Bot ── */
    { TRIGGER_BOT_SELF_SCAN,       0, 60, REACTION_CAMOUFLAGE_SELF,   THREAT_VIGILANCE  },
    { TRIGGER_BOT_KILL_ATTEMPT,    0, 70, REACTION_BACKUP_DNA,        THREAT_SURVIVAL   },
    { TRIGGER_BOT_KILL_ATTEMPT,    4, 85, REACTION_REGEN_AGENT,       THREAT_SURVIVAL   },
    { TRIGGER_BOT_CORRUPT_DETECT,  0, 80, REACTION_REGEN_AGENT,       THREAT_SURVIVAL   },
};

#define INSTINCT_RULES_COUNT  (sizeof(INSTINCT_RULES) / sizeof(INSTINCT_RULES[0]))

/* ─── Implémentation ─────────────────────────────────────────────────── */

void instinct_init(InstinctLayer *inst) {
    if (!inst) return;
    memset(inst, 0, sizeof(InstinctLayer));
    threat_state_init(&inst->threat_state);
}

ReactionType instinct_get_reaction(TriggerType trigger,
                                   uint8_t current_threat_level,
                                   uint32_t confidence) {
    /*
     * Cherche la règle la plus spécifique pour ce (trigger, level, confidence).
     * Priorité aux règles avec le niveau de menace le plus élevé qui correspond.
     */
    ReactionType best = REACTION_INTENSIFY_WATCH; /* Défaut minimal */
    uint8_t      best_level = 255;                /* Cherche le plus haut */

    for (uint32_t i = 0; i < INSTINCT_RULES_COUNT; i++) {
        const InstinctRule *r = &INSTINCT_RULES[i];
        if (r->trigger != trigger) continue;
        if (current_threat_level < r->min_threat_level) continue;
        if (confidence < r->min_confidence) continue;
        if (r->min_threat_level <= best_level) {
            best_level = r->min_threat_level;
            best = r->reaction;
        }
    }
    return best;
}

void instinct_trigger(InstinctLayer *inst,
                      TriggerType trigger,
                      uint32_t pid,
                      uint64_t addr,
                      uint32_t confidence) {
    if (!inst) return;

    uint8_t current = inst->threat_state.current_level;

    /* 1. Trouver la réaction (lookup table, < 10µs) */
    ReactionType reaction = instinct_get_reaction(trigger, current, confidence);

    /* 2. Trouver le nouveau niveau recommandé */
    uint8_t new_level = current;
    for (uint32_t i = 0; i < INSTINCT_RULES_COUNT; i++) {
        const InstinctRule *r = &INSTINCT_RULES[i];
        if (r->trigger == trigger
            && current >= r->min_threat_level
            && confidence >= r->min_confidence)
        {
            if (r->new_threat_level > new_level)
                new_level = r->new_threat_level;
        }
    }

    /* 3. Enregistrer dans l'historique circulaire */
    uint32_t idx = inst->history_head % INSTINCT_HISTORY_SIZE;
    InstinctEvent *ev = &inst->history[idx];
    ev->trigger            = trigger;
    ev->timestamp          = inst->total_triggers; /* placeholder sans horloge */
    ev->target_addr        = addr;
    ev->target_pid         = pid;
    ev->confidence         = confidence;
    ev->reaction_taken     = reaction;
    ev->threat_level_before = current;
    ev->threat_level_after  = new_level;

    inst->history_head++;
    if (inst->history_count < INSTINCT_HISTORY_SIZE)
        inst->history_count++;

    inst->total_triggers++;

    /* 4. Transiter si besoin */
    if (new_level != current) {
        char reason[64];
        snprintf(reason, sizeof(reason),
                 "trigger=0x%02x pid=%u conf=%u", trigger, pid, confidence);
        threat_transition(&inst->threat_state, new_level, reason);
    }

    /* 5. Exécuter la réaction via les callbacks (aucun appel bloquant) */
    inst->total_reactions++;

    switch (reaction) {
        case REACTION_QUARANTINE_PROC:
            if (inst->on_quarantine) inst->on_quarantine(pid, addr);
            break;
        case REACTION_CUT_NETWORK:
            if (inst->on_network_cut) inst->on_network_cut(0);
            break;
        case REACTION_CUT_NETWORK_ALL:
            if (inst->on_network_cut) inst->on_network_cut(1);
            break;
        case REACTION_DEPLOY_HONEYTRAP:
            if (inst->on_honeytrap_deploy) inst->on_honeytrap_deploy(addr);
            break;
        case REACTION_REGEN_AGENT:
            /* Demande de régénération : rôle encodé dans addr pour simplifier */
            if (inst->on_regen_request) inst->on_regen_request((uint8_t)addr);
            break;
        case REACTION_ALERT_SWARM_MIND:
            if (inst->on_swarm_alert) inst->on_swarm_alert(new_level, trigger);
            break;
        case REACTION_ALERT_OO_BRIDGE:
            if (inst->on_oo_bridge_alert) {
                inst->on_oo_bridge_alert(new_level, "instinct trigger");
            }
            break;
        case REACTION_BACKUP_DNA:
            /* Géré par l'appelant après retour */
            if (inst->on_swarm_alert) inst->on_swarm_alert(new_level, trigger);
            break;
        default:
            /* INTENSIFY_WATCH, CAPTURE_MEMORY, LOG_FULL, CAMOUFLAGE_SELF */
            /* → SwarmMind gère ces réactions en mode actif               */
            if (new_level >= THREAT_COMBAT && inst->on_swarm_alert)
                inst->on_swarm_alert(new_level, trigger);
            break;
    }
}

const InstinctEvent *instinct_last_event(const InstinctLayer *inst) {
    if (!inst || inst->history_count == 0) return NULL;
    uint32_t last = (inst->history_head - 1 + INSTINCT_HISTORY_SIZE)
                    % INSTINCT_HISTORY_SIZE;
    return &inst->history[last];
}

void instinct_dump_history(const InstinctLayer *inst, uint32_t n) {
    if (!inst) return;
    if (n > inst->history_count) n = inst->history_count;
    printf("[InstinctLayer] Last %u events (total=%llu reactions=%llu):\n",
           n,
           (unsigned long long)inst->total_triggers,
           (unsigned long long)inst->total_reactions);
    for (uint32_t i = 0; i < n; i++) {
        uint32_t idx = (inst->history_head - 1 - i + INSTINCT_HISTORY_SIZE)
                       % INSTINCT_HISTORY_SIZE;
        const InstinctEvent *ev = &inst->history[idx];
        printf("  [%2u] trigger=0x%02x pid=%5u conf=%3u "
               "reaction=0x%02x level: %u→%u\n",
               i, ev->trigger, ev->target_pid, ev->confidence,
               ev->reaction_taken, ev->threat_level_before,
               ev->threat_level_after);
    }
}

int instinct_reset_to_dormant(InstinctLayer *inst, const char *reason) {
    if (!inst) return -1;
    /* Le reset vers DORMANT n'est autorisé que depuis VIGILANCE ou COMBAT */
    uint8_t cur = inst->threat_state.current_level;
    if (cur == THREAT_DORMANT) return 0;
    return threat_transition(&inst->threat_state, THREAT_DORMANT, reason);
}
