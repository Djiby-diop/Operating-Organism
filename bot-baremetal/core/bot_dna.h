/**
 * BOT-BAREMETAL — BotDNA
 * L'ADN de chaque agent : identité, capacités, mémoire génétique.
 *
 * Règle : cette structure est la source de vérité de tout agent.
 * Elle ne peut pas croître de façon incontrôlée.
 * Chaque champ doit être justifié.
 */

#ifndef BOT_DNA_H
#define BOT_DNA_H

#include <stdint.h>

/* ─── Constantes ──────────────────────────────────────────────────────── */

#define BOT_DNA_MAGIC       0x424F5444  /* "BOTD" en ASCII                */
#define BOT_AGENT_ID_LEN    16          /* UUID 128 bits                  */
#define BOT_MAX_PATTERNS    64          /* Patterns comportementaux max    */
#define BOT_PRIME_DIR_LEN   256         /* Taille des directives immuables */
#define BOT_DNA_VERSION     1

/* ─── Rôles des Agents ────────────────────────────────────────────────── */

typedef enum {
    AGENT_ROLE_NONE         = 0x00,

    /* Surveillance (Sensor Net) */
    AGENT_ROLE_MEM_WATCH    = 0x01,   /* Mémoire RAM                    */
    AGENT_ROLE_NET_WATCH    = 0x02,   /* Réseau                         */
    AGENT_ROLE_FS_WATCH     = 0x03,   /* Système de fichiers            */
    AGENT_ROLE_PROC_WATCH   = 0x04,   /* Processus                      */
    AGENT_ROLE_KERNEL_WATCH = 0x05,   /* Kernel / syscalls              */
    AGENT_ROLE_BOOT_WATCH   = 0x06,   /* Boot UEFI                      */

    /* Réponse (Immune Response) */
    AGENT_ROLE_QUARANTINE   = 0x10,   /* Isolation                      */
    AGENT_ROLE_HONEY_TRAP   = 0x11,   /* Leurre / piège                 */
    AGENT_ROLE_NEUTRALIZER  = 0x12,   /* Neutralisation confirmée       */
    AGENT_ROLE_CHAMELEON    = 0x13,   /* Camouflage du Bot              */

    /* Adaptation */
    AGENT_ROLE_MIMICRY      = 0x20,   /* Apprentissage des attaques     */
    AGENT_ROLE_MUTATION     = 0x21,   /* Évolution des agents           */

    /* Infrastructure */
    AGENT_ROLE_REGEN        = 0x30,   /* Auto-reconstruction            */
    AGENT_ROLE_OO_BRIDGE    = 0x31,   /* Interface LLM-Baremetal        */
    AGENT_ROLE_SWARM_MIND   = 0xFF,   /* Chef d'État — coordinateur     */
} AgentRole;

/* ─── Niveaux de Menace ───────────────────────────────────────────────── */

typedef enum {
    THREAT_LEVEL_DORMANT     = 0,   /* Surveillance passive            */
    THREAT_LEVEL_VIGILANCE   = 1,   /* Activité suspecte              */
    THREAT_LEVEL_ALERT       = 2,   /* Comportement malveillant        */
    THREAT_LEVEL_COMBAT      = 3,   /* Intrusion confirmée             */
    THREAT_LEVEL_SURVIVAL    = 4,   /* Le Bot lui-même est attaqué     */
    THREAT_LEVEL_CONFINEMENT = 5,   /* Menace existentielle OO         */
} ThreatLevel;

/* ─── Domaines de Protection ──────────────────────────────────────────── */

#define THREAT_DOMAIN_ANTIVIRUS     (1 << 0)   /* Malwares comportementaux  */
#define THREAT_DOMAIN_ANTIPIRACY    (1 << 1)   /* Reverse engineering       */
#define THREAT_DOMAIN_ANTITHEFT     (1 << 2)   /* Exfiltration de données   */
#define THREAT_DOMAIN_ANTIROOTKIT   (1 << 3)   /* Rootkits / bootkits       */
#define THREAT_DOMAIN_ANTIRANSOM    (1 << 4)   /* Ransomware                */
#define THREAT_DOMAIN_ANTISOCIAL    (1 << 5)   /* Social engineering        */
#define THREAT_DOMAIN_ANTIBOT       (1 << 6)   /* Botnets entrants          */
#define THREAT_DOMAIN_SELF_PROTECT  (1 << 7)   /* Protection du Bot lui-même*/

/* ─── Pattern Comportemental (Anticorps) ─────────────────────────────── */

#define PATTERN_SIG_LEN     32

typedef struct {
    uint8_t     signature[PATTERN_SIG_LEN]; /* Signature comportementale  */
    uint32_t    threat_domain;              /* Domaine de menace           */
    uint32_t    confidence;                 /* Confiance 0-100             */
    uint64_t    learned_at;                 /* Timestamp d'apprentissage   */
    uint64_t    hit_count;                  /* Nombre de détections        */
    uint8_t     active;                     /* 1 = actif, 0 = en test      */
    uint8_t     _pad[7];
} BehaviorPattern;

/* ─── ADN Principal de l'Agent ───────────────────────────────────────── */

typedef struct {
    /* En-tête */
    uint32_t        magic;                          /* BOT_DNA_MAGIC       */
    uint32_t        version;                        /* BOT_DNA_VERSION     */

    /* Identité */
    uint8_t         agent_id[BOT_AGENT_ID_LEN];    /* UUID 128-bit unique */
    AgentRole       role;                           /* Rôle spécialisé     */
    uint32_t        generation;                     /* Génération (mute)   */
    char            name[32];                       /* Nom lisible         */

    /* Capacités */
    uint64_t        capabilities;                   /* Bitmap capacités    */
    uint32_t        threat_domains;                 /* Domaines actifs     */
    ThreatLevel     current_threat_level;           /* Niveau actuel       */

    /* Mémoire comportementale */
    BehaviorPattern learned_patterns[BOT_MAX_PATTERNS];
    uint32_t        pattern_count;

    /* Statistiques de vie */
    uint64_t        threats_detected;               /* Menaces détectées   */
    uint64_t        threats_neutralized;            /* Menaces neutralisées*/
    uint64_t        false_positives;                /* Erreurs commises    */
    uint64_t        birth_time;                     /* Timestamp création  */
    uint64_t        last_regen_time;                /* Dernière régénérat. */
    uint32_t        regen_count;                    /* Nombre de régénérat.*/

    /* Santé */
    uint8_t         integrity_hash[32];             /* SHA256 de soi-même  */
    uint8_t         is_healthy;                     /* 1 = sain, 0 = corru.*/

    /* Directives immuables (scellées à la création, NE PAS MODIFIER) */
    uint8_t         prime_directives[BOT_PRIME_DIR_LEN];

    /* Padding pour alignement futur */
    uint8_t         _reserved[64];
} BotDNA;

/* ─── Fonctions Principales ───────────────────────────────────────────── */

/**
 * Initialise un BotDNA avec les valeurs par défaut pour un rôle donné.
 * Les prime_directives sont scellées à ce moment et ne peuvent plus changer.
 */
void bot_dna_init(BotDNA *dna, AgentRole role, const char *name);

/**
 * Vérifie l'intégrité d'un agent (hash SHA256 de lui-même).
 * Retourne 1 si sain, 0 si corrompu.
 */
int bot_dna_verify(const BotDNA *dna);

/**
 * Ajoute un pattern comportemental appris à l'ADN.
 * Retourne 0 si succès, -1 si pool plein.
 */
int bot_dna_add_pattern(BotDNA *dna, const BehaviorPattern *pattern);

/**
 * Sérialise un BotDNA en bytes pour persistance.
 * Compatible avec le journal OO.
 */
int bot_dna_serialize(const BotDNA *dna, uint8_t *buf, uint32_t buf_len);

/**
 * Désérialise un BotDNA depuis des bytes.
 * Vérifie l'intégrité après désérialisation.
 */
int bot_dna_deserialize(BotDNA *dna, const uint8_t *buf, uint32_t buf_len);

/**
 * Affiche un résumé lisible du BotDNA (pour debug/journal).
 */
void bot_dna_print_summary(const BotDNA *dna);

/* ─── Macros Utilitaires ─────────────────────────────────────────────── */

#define BOT_DNA_IS_VALID(dna) \
    ((dna) != NULL && (dna)->magic == BOT_DNA_MAGIC && bot_dna_verify(dna))

#define BOT_HAS_DOMAIN(dna, domain) \
    (((dna)->threat_domains & (domain)) != 0)

#define BOT_IS_ROLE(dna, r) \
    ((dna)->role == (r))

#endif /* BOT_DNA_H */
