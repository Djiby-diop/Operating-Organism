/**
 * BOT-BAREMETAL — OO Protocol (C interface)
 *
 * Protocole de communication entre le Bot-Baremetal et le LLM-Baremetal
 * dans l'Operating Organism, du point de vue du code C bas niveau.
 *
 * Ce header est utilisé par l'InstinctLayer et le Soma du Bot
 * pour formater et envoyer des messages OO sans dépendances Python.
 *
 * Compatible : UEFI bare-metal (pas de stdlib complète)
 */

#ifndef OO_PROTOCOL_H
#define OO_PROTOCOL_H

#include <stdint.h>

/* ─── Magic et versions ──────────────────────────────────────────────────── */

#define OO_PROTO_MAGIC      0x4F4F424F  /* "OOBO" — OO Bot message          */
#define OO_PROTO_VERSION    1

/* ─── Types de Messages ──────────────────────────────────────────────────── */

typedef enum {
    OO_MSG_BOT_EVENT     = 0x01,   /* Bot → LLM : événement de sécurité     */
    OO_MSG_BOT_ALERT     = 0x02,   /* Bot → LLM : alerte critique           */
    OO_MSG_BOT_HEARTBEAT = 0x03,   /* Bot → LLM : signe de vie régulier     */
    OO_MSG_BOT_GENOME    = 0x04,   /* Bot → LLM : nouveau pattern appris    */
    OO_MSG_LLM_DIRECTIVE = 0x10,   /* LLM → Bot : directive stratégique     */
    OO_MSG_LLM_ACK       = 0x11,   /* LLM → Bot : acquittement              */
    OO_MSG_SYSTEM_STATUS = 0x20,   /* Interne : rapport d'état global       */
} OOMsgType;

/* ─── Niveaux de Priorité ────────────────────────────────────────────────── */

typedef enum {
    OO_PRIO_LOW      = 0,
    OO_PRIO_NORMAL   = 1,
    OO_PRIO_HIGH     = 2,
    OO_PRIO_CRITICAL = 3,
} OOPriority;

/* ─── En-tête de Message ─────────────────────────────────────────────────── */

#define OO_SOURCE_BOT_NAME  "bot-baremetal"
#define OO_TARGET_LLM_NAME  "llm-baremetal"
#define OO_MSG_ID_LEN       12
#define OO_NAME_LEN         32
#define OO_DESC_LEN         256

typedef struct {
    uint32_t    magic;                      /* OO_PROTO_MAGIC               */
    uint32_t    version;                    /* OO_PROTO_VERSION             */
    uint8_t     msg_type;                   /* OOMsgType                    */
    uint8_t     priority;                   /* OOPriority                   */
    uint8_t     _pad[2];
    char        msg_id[OO_MSG_ID_LEN];     /* ID unique du message          */
    char        from[OO_NAME_LEN];         /* Source                        */
    char        to[OO_NAME_LEN];           /* Destination                   */
    uint64_t    timestamp;                 /* Timestamp (ns depuis boot)    */
    uint32_t    payload_size;              /* Taille du payload en bytes    */
    uint32_t    payload_crc;              /* CRC32 du payload              */
} OOMsgHeader;

/* ─── Payload : Événement de Sécurité ────────────────────────────────────── */

typedef struct {
    uint8_t     threat_level;              /* 0-5                           */
    uint8_t     confidence;               /* 0-100                          */
    uint8_t     agent_role;               /* AgentRole du déclarant        */
    uint8_t     action_taken;             /* ReactionType effectué          */
    char        description[OO_DESC_LEN]; /* Description lisible            */
    char        pattern_id[OO_MSG_ID_LEN]; /* ID pattern appris (si applic.)*/
    uint32_t    target_pid;               /* PID concerné (0 si N/A)        */
    uint64_t    target_addr;              /* Adresse mémoire (0 si N/A)     */
} OOBotEvent;

/* ─── Payload : Alerte Critique ──────────────────────────────────────────── */

typedef struct {
    uint8_t     threat_level;             /* Toujours >= 4 (SURVIVAL)       */
    uint8_t     _pad[3];
    char        description[OO_DESC_LEN];
    char        action_taken[64];         /* Ce que le Bot a déjà fait      */
    uint8_t     request_ack;              /* 1 = LLM doit acquitter         */
    uint8_t     _pad2[7];
} OOBotAlert;

/* ─── Payload : Heartbeat ────────────────────────────────────────────────── */

typedef struct {
    uint8_t     current_threat_level;     /* 0-5                            */
    uint8_t     fleet_health;             /* % agents sains 0-100           */
    uint8_t     genome_count;             /* Nb patterns dans génome        */
    uint8_t     is_cloaked;               /* 1 = Bot en mode camouflage     */
    uint32_t    threats_today;            /* Menaces détectées aujourd'hui  */
    uint32_t    uptime_s;                 /* Secondes depuis démarrage      */
} OOBotHeartbeat;

/* ─── Payload : Directive LLM ────────────────────────────────────────────── */

typedef struct {
    uint8_t     action;                   /* BotDirectiveAction             */
    uint8_t     priority;                 /* OOPriority                     */
    uint8_t     _pad[2];
    char        target[64];              /* Cible de l'action              */
    char        context[128];            /* Contexte fourni par le LLM     */
    uint64_t    expires_at;              /* Expiration (0 = jamais)        */
} OOLLMDirective;

/* ─── Message Complet (header + payload inline) ──────────────────────────── */

#define OO_MAX_PAYLOAD_SIZE     512

typedef struct {
    OOMsgHeader header;
    uint8_t     payload[OO_MAX_PAYLOAD_SIZE];
} OOMessage;

/* ─── Fonctions ──────────────────────────────────────────────────────────── */

/**
 * Initialise un message OO avec les valeurs par défaut.
 */
void oo_msg_init(OOMessage *msg, OOMsgType type, OOPriority prio);

/**
 * Remplit le payload d'un événement de sécurité.
 * Retourne -1 si le payload ne rentre pas.
 */
int oo_msg_set_event(OOMessage *msg, const OOBotEvent *event);

/**
 * Remplit le payload d'une alerte critique.
 */
int oo_msg_set_alert(OOMessage *msg, const OOBotAlert *alert);

/**
 * Remplit le payload d'un heartbeat.
 */
int oo_msg_set_heartbeat(OOMessage *msg, const OOBotHeartbeat *hb);

/**
 * Sérialise un message complet en bytes pour écriture dans le journal OO.
 * Format de sortie : JSON ligne unique (compatible avec OOJOUR.LOG).
 *
 * @param msg    Message à sérialiser
 * @param buf    Buffer de sortie
 * @param buflen Taille du buffer
 * @return Nombre de bytes écrits, ou -1 si buffer trop petit
 */
int oo_msg_to_json_line(const OOMessage *msg, char *buf, uint32_t buflen);

/**
 * Calcule le CRC32 d'un buffer (pour vérification payload).
 */
uint32_t oo_crc32(const uint8_t *data, uint32_t len);

/**
 * Vérifie l'intégrité d'un message reçu.
 * Retourne 1 si valide, 0 sinon.
 */
int oo_msg_verify(const OOMessage *msg);

/* ─── Macros Utilitaires ─────────────────────────────────────────────────── */

#define OO_MSG_IS_FROM_LLM(msg) \
    (strncmp((msg)->header.from, OO_TARGET_LLM_NAME, OO_NAME_LEN) == 0)

#define OO_MSG_IS_ALERT(msg) \
    ((msg)->header.msg_type == OO_MSG_BOT_ALERT)

#define OO_MSG_IS_CRITICAL(msg) \
    ((msg)->header.priority == OO_PRIO_CRITICAL)

#define OO_MSG_VALID(msg) \
    ((msg) != NULL && (msg)->header.magic == OO_PROTO_MAGIC && oo_msg_verify(msg))

#endif /* OO_PROTOCOL_H */
