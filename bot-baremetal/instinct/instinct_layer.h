/**
 * BOT-BAREMETAL — InstinctLayer
 * Réflexes primitifs ultra-rapides. Pas de pensée, que de l'action.
 *
 * Règle d'or : aucune fonction ici ne doit prendre plus de 1ms.
 * Aucune allocation dynamique. Aucun appel bloquant.
 * Ce module est le système nerveux autonome du Bot.
 *
 * "L'instinct ne demande pas la permission."
 */

#ifndef INSTINCT_LAYER_H
#define INSTINCT_LAYER_H

#include <stdint.h>
#include "threat_levels.h"

/* ─── Taille des Pools Fixes ─────────────────────────────────────────── */

#define INSTINCT_MAX_TRIGGERS       32
#define INSTINCT_MAX_REACTIONS      32
#define INSTINCT_HISTORY_SIZE       256

/* ─── Types de Déclencheurs ─────────────────────────────────────────── */

typedef enum {
    /* Mémoire */
    TRIGGER_MEM_WRITE_EXEC      = 0x01,   /* Écriture dans zone exécutable */
    TRIGGER_MEM_SPRAY           = 0x02,   /* Pattern de heap spray         */
    TRIGGER_MEM_SHELLCODE_SIG   = 0x03,   /* Signature shellcode détectée  */
    TRIGGER_MEM_ROP_CHAIN       = 0x04,   /* Chaîne ROP suspectée          */

    /* Processus */
    TRIGGER_PROC_HOLLOW         = 0x10,   /* Process hollowing             */
    TRIGGER_PROC_INJECT         = 0x11,   /* Injection de code             */
    TRIGGER_PROC_PRIV_ESC       = 0x12,   /* Escalade de privilèges        */
    TRIGGER_PROC_ABNORMAL_CHILD = 0x13,   /* Processus enfant anormal      */
    TRIGGER_PROC_DEBUGGER       = 0x14,   /* Débogueur non autorisé        */

    /* Réseau */
    TRIGGER_NET_C2_BEACON       = 0x20,   /* Beacon C&C                    */
    TRIGGER_NET_EXFIL_VOLUME    = 0x21,   /* Volume de sortie anormal      */
    TRIGGER_NET_UNUSUAL_PORT    = 0x22,   /* Port inhabituel               */
    TRIGGER_NET_DNS_TUNNEL      = 0x23,   /* Tunneling DNS                 */

    /* Système de fichiers */
    TRIGGER_FS_MASS_ENCRYPT     = 0x30,   /* Chiffrement de masse          */
    TRIGGER_FS_CRITICAL_MODIF   = 0x31,   /* Modification fichier critique */
    TRIGGER_FS_BOOT_MODIF       = 0x32,   /* Modification du secteur boot  */
    TRIGGER_FS_RAPID_DELETE     = 0x33,   /* Suppression massive rapide    */

    /* Kernel */
    TRIGGER_KERN_SYSCALL_HOOK   = 0x40,   /* Hook de syscall               */
    TRIGGER_KERN_ROOTKIT_SIG    = 0x41,   /* Signature rootkit             */
    TRIGGER_KERN_DKOM           = 0x42,   /* Manipulation objet kernel     */

    /* Sécurité du Bot */
    TRIGGER_BOT_SELF_SCAN       = 0x50,   /* Quelqu'un scanne le Bot       */
    TRIGGER_BOT_KILL_ATTEMPT    = 0x51,   /* Tentative de tuer un agent    */
    TRIGGER_BOT_CORRUPT_DETECT  = 0x52,   /* Corruption d'un agent détectée*/
} TriggerType;

/* ─── Types de Réactions ────────────────────────────────────────────── */

typedef enum {
    REACTION_NONE               = 0x00,

    /* Isolation */
    REACTION_QUARANTINE_PROC    = 0x01,   /* Isoler un processus           */
    REACTION_QUARANTINE_FILE    = 0x02,   /* Isoler un fichier             */
    REACTION_CUT_NETWORK        = 0x03,   /* Coupure réseau ciblée         */
    REACTION_CUT_NETWORK_ALL    = 0x04,   /* Coupure réseau totale         */

    /* Leurre */
    REACTION_DEPLOY_HONEYTRAP   = 0x10,   /* Déployer un leurre            */
    REACTION_FAKE_RESOURCES     = 0x11,   /* Fausser les ressources visibles*/

    /* Surveillance renforcée */
    REACTION_INTENSIFY_WATCH    = 0x20,   /* Augmenter surveillance        */
    REACTION_CAPTURE_MEMORY     = 0x21,   /* Capturer snapshot mémoire     */
    REACTION_LOG_FULL           = 0x22,   /* Journalisation complète       */

    /* Survie */
    REACTION_REGEN_AGENT        = 0x30,   /* Régénérer un agent corrompu   */
    REACTION_CAMOUFLAGE_SELF    = 0x31,   /* S'invisibiliser               */
    REACTION_BACKUP_DNA         = 0x32,   /* Sauvegarder son ADN           */

    /* Escalade */
    REACTION_ESCALATE_THREAT    = 0x40,   /* Monter le niveau de menace    */
    REACTION_ALERT_SWARM_MIND   = 0x41,   /* Alerter le SwarmMind          */
    REACTION_ALERT_OO_BRIDGE    = 0x42,   /* Alerter le LLM via OOBridge   */
} ReactionType;

/* ─── Structure d'un Événement Instinctif ───────────────────────────── */

typedef struct {
    TriggerType trigger;            /* Ce qui a déclenché               */
    uint64_t    timestamp;          /* Quand (nanosecondes)             */
    uint64_t    target_addr;        /* Adresse concernée (si applicable)*/
    uint32_t    target_pid;         /* PID concerné (si applicable)     */
    uint32_t    confidence;         /* Confiance 0-100                  */
    ReactionType reaction_taken;    /* Réaction effectuée               */
    uint8_t     threat_level_before;/* Niveau avant                     */
    uint8_t     threat_level_after; /* Niveau après                     */
    uint8_t     _pad[6];
} InstinctEvent;

/* ─── Structure Principale de l'InstinctLayer ───────────────────────── */

typedef struct {
    /* État courant */
    ThreatState             threat_state;

    /* Historique circulaire des événements */
    InstinctEvent           history[INSTINCT_HISTORY_SIZE];
    uint32_t                history_head;
    uint32_t                history_count;

    /* Callbacks vers les agents (pas d'allocations) */
    void (*on_quarantine)(uint32_t pid, uint64_t addr);
    void (*on_network_cut)(int full);
    void (*on_honeytrap_deploy)(uint64_t target_addr);
    void (*on_regen_request)(uint8_t agent_role);
    void (*on_swarm_alert)(uint8_t threat_level, TriggerType trigger);
    void (*on_oo_bridge_alert)(uint8_t threat_level, const char *summary);

    /* Statistiques */
    uint64_t                total_triggers;
    uint64_t                total_reactions;
    uint64_t                false_positive_count;
} InstinctLayer;

/* ─── Fonctions Principales ──────────────────────────────────────────── */

/**
 * Initialise l'InstinctLayer.
 * Tous les callbacks sont optionnels mais fortement recommandés.
 */
void instinct_init(InstinctLayer *inst);

/**
 * Point d'entrée principal : signal de déclencheur.
 * 
 * C'est la fonction la plus critique du Bot.
 * Elle doit répondre en < 1ms dans tous les cas.
 * Aucune allocation. Aucun appel bloquant.
 *
 * @param trigger   Type de déclencheur
 * @param pid       PID concerné (0 si non applicable)
 * @param addr      Adresse concernée (0 si non applicable)
 * @param confidence Confiance 0-100
 */
void instinct_trigger(InstinctLayer *inst,
                      TriggerType trigger,
                      uint32_t pid,
                      uint64_t addr,
                      uint32_t confidence);

/**
 * Retourne la réaction recommandée pour un déclencheur donné.
 * Pure computation, aucun effet de bord.
 */
ReactionType instinct_get_reaction(TriggerType trigger,
                                   uint8_t current_threat_level,
                                   uint32_t confidence);

/**
 * Retourne le dernier événement enregistré.
 * Retourne NULL si aucun événement.
 */
const InstinctEvent *instinct_last_event(const InstinctLayer *inst);

/**
 * Dump des N derniers événements pour journal OO.
 */
void instinct_dump_history(const InstinctLayer *inst, uint32_t n);

/**
 * Retourne le niveau de menace actuel.
 */
static inline uint8_t instinct_current_level(const InstinctLayer *inst) {
    return inst->threat_state.current_level;
}

/**
 * Force un reset vers DORMANT (usage : après neutralisation complète).
 * Nécessite la validation du SwarmMind.
 */
int instinct_reset_to_dormant(InstinctLayer *inst, const char *reason);

#endif /* INSTINCT_LAYER_H */
