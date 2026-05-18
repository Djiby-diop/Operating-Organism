/**
 * BOT-BAREMETAL — Neural Territory Map
 *
 * "L'animal connaît son territoire. Toute présence étrangère est
 *  détectée immédiatement, sans réflexion, par pure familiarité."
 *
 * Le Bot cartographie le système en continu :
 *   - Mémoire légitime vs inconnue
 *   - Processus normaux vs suspects
 *   - Trafic réseau habituel vs anormal
 *   - Fichiers critiques et leur état
 *
 * Cette carte est la base de toutes les détections instinctives.
 * Tout ce qui n'est PAS sur la carte est suspect par définition.
 */

#ifndef TERRITORY_MAP_H
#define TERRITORY_MAP_H

#include <stdint.h>
#include "bot_dna.h"   /* pour ThreatLevel */

/* ─── Limites des Pools (pas d'allocation dynamique) ─────────────────── */

#define TERRITORY_MAX_PROCESSES     512
#define TERRITORY_MAX_CONNECTIONS   256
#define TERRITORY_MAX_WATCHED_FILES  64
#define TERRITORY_MAX_MEM_REGIONS   128
#define TERRITORY_MAX_ANOMALIES     256

/* ─── Statut d'un Élément ────────────────────────────────────────────── */

typedef enum {
    ELEM_UNKNOWN    = 0,   /* Jamais vu — suspect par définition     */
    ELEM_TRUSTED    = 1,   /* Validé — connu et légitime             */
    ELEM_GREY       = 2,   /* Gris — vu mais pas encore validé       */
    ELEM_SUSPICIOUS = 3,   /* Suspect — comportement anormal         */
    ELEM_HOSTILE    = 4,   /* Hostile — menace confirmée             */
    ELEM_QUARANTINE = 5,   /* Isolé par le Bot                       */
} ElemStatus;

/* ─── Processus Cartographié ─────────────────────────────────────────── */

typedef struct {
    uint32_t    pid;
    uint32_t    ppid;              /* PID parent                       */
    char        name[64];          /* Nom du processus                 */
    char        path[256];         /* Chemin de l'exécutable           */
    uint64_t    first_seen;        /* Timestamp première apparition    */
    uint64_t    last_seen;         /* Timestamp dernière activité      */
    uint32_t    observation_count; /* Nombre de fois observé           */
    ElemStatus  status;
    uint8_t     has_network;       /* 1 = a des connexions réseau      */
    uint8_t     has_children;      /* 1 = a des processus fils         */
    uint8_t     _pad[2];
} MappedProcess;

/* ─── Connexion Réseau Cartographiée ─────────────────────────────────── */

typedef struct {
    uint8_t     remote_ip[16];     /* IPv4/IPv6 destination            */
    uint16_t    remote_port;
    uint16_t    local_port;
    uint8_t     protocol;          /* 6=TCP, 17=UDP                    */
    ElemStatus  status;
    uint32_t    owning_pid;
    uint64_t    bytes_sent;
    uint64_t    bytes_recv;
    uint64_t    connection_count;  /* Fois qu'on a vu cette dest.      */
    uint64_t    last_seen;
    char        domain[128];       /* DNS résolu si disponible         */
} MappedConnection;

/* ─── Fichier Critique Surveillé ─────────────────────────────────────── */

typedef struct {
    char        path[256];
    uint8_t     expected_hash[32]; /* SHA256 attendu                   */
    uint8_t     current_hash[32];  /* SHA256 actuel                    */
    uint64_t    size_bytes;
    uint64_t    last_verified;
    uint8_t     is_intact;         /* 1 = OK, 0 = modifié             */
    uint8_t     is_critical;       /* 1 = critique OO (boot, kernel)  */
    uint8_t     _pad[6];
} WatchedFile;

/* ─── Région Mémoire Cartographiée ───────────────────────────────────── */

typedef struct {
    uint64_t    base_addr;
    uint64_t    size;
    uint32_t    owning_pid;
    uint8_t     is_executable;
    uint8_t     is_writable;
    uint8_t     is_shared;
    ElemStatus  status;
    char        label[32];         /* Label lisible (ex: "ntdll.dll") */
} MappedMemRegion;

/* ─── Anomalie Enregistrée ───────────────────────────────────────────── */

typedef struct {
    uint64_t    timestamp;
    uint32_t    pid;               /* Processus concerné (0 si global) */
    uint8_t     anomaly_type;      /* Type (voir constantes ci-dessous)*/
    uint8_t     severity;          /* 0-10                             */
    uint8_t     _pad[2];
    char        description[128];
} TerritoryAnomaly;

/* Types d'anomalies */
#define ANOMALY_NEW_PROCESS          0x01
#define ANOMALY_UNKNOWN_CONNECTION   0x02
#define ANOMALY_FILE_MODIFIED        0x03
#define ANOMALY_MEM_EXEC_WRITE       0x04
#define ANOMALY_UNUSUAL_PARENT       0x05
#define ANOMALY_MASS_FILE_OPS        0x06
#define ANOMALY_HIGH_NET_VOLUME      0x07
#define ANOMALY_BEACON_PATTERN       0x08

/* ─── La Carte du Territoire ─────────────────────────────────────────── */

typedef struct {
    /* Processus */
    MappedProcess   processes[TERRITORY_MAX_PROCESSES];
    uint32_t        process_count;

    /* Réseau */
    MappedConnection connections[TERRITORY_MAX_CONNECTIONS];
    uint32_t        connection_count;

    /* Fichiers critiques */
    WatchedFile     watched_files[TERRITORY_MAX_WATCHED_FILES];
    uint32_t        watched_file_count;

    /* Mémoire */
    MappedMemRegion mem_regions[TERRITORY_MAX_MEM_REGIONS];
    uint32_t        mem_region_count;

    /* Anomalies (historique circulaire) */
    TerritoryAnomaly anomalies[TERRITORY_MAX_ANOMALIES];
    uint32_t        anomaly_head;
    uint32_t        anomaly_count;

    /* Baseline établie ? */
    uint8_t         baseline_ready;   /* 1 = carte de base établie    */
    uint64_t        baseline_time;     /* Quand la baseline a été prise*/

    /* Statistiques */
    uint64_t        total_observations;
    uint64_t        total_anomalies;
} TerritoryMap;

/* ─── Fonctions Principales ──────────────────────────────────────────── */

/** Initialise la carte vide. */
void territory_map_init(TerritoryMap *map);

/**
 * Enregistre un processus observé.
 * Si nouveau → anomalie ANOMALY_NEW_PROCESS.
 * Retourne pointeur vers l'entrée (ou NULL si pool plein).
 */
MappedProcess *territory_observe_process(TerritoryMap *map, uint32_t pid,
                                          uint32_t ppid, const char *name,
                                          const char *path);

/**
 * Enregistre une connexion réseau observée.
 * Si nouvelle destination → anomalie ANOMALY_UNKNOWN_CONNECTION.
 */
MappedConnection *territory_observe_connection(TerritoryMap *map,
                                                uint32_t pid,
                                                const uint8_t *remote_ip,
                                                uint16_t remote_port,
                                                uint64_t bytes_sent);

/**
 * Ajoute un fichier critique à surveiller.
 * Calcule son hash initial comme référence.
 */
int territory_watch_file(TerritoryMap *map, const char *path, uint8_t is_critical);

/**
 * Vérifie l'intégrité de tous les fichiers surveillés.
 * Retourne le nombre de fichiers modifiés.
 */
int territory_verify_files(TerritoryMap *map);

/**
 * Enregistre une région mémoire observée.
 */
MappedMemRegion *territory_observe_mem(TerritoryMap *map, uint64_t base,
                                        uint64_t size, uint32_t pid,
                                        uint8_t exec, uint8_t write);

/**
 * Marque le statut d'un processus.
 */
void territory_mark_process(TerritoryMap *map, uint32_t pid, ElemStatus status);

/**
 * Enregistre une anomalie dans l'historique circulaire.
 */
void territory_record_anomaly(TerritoryMap *map, uint8_t type, uint8_t severity,
                               uint32_t pid, const char *description);

/**
 * Retourne la dernière anomalie enregistrée.
 */
const TerritoryAnomaly *territory_last_anomaly(const TerritoryMap *map);

/**
 * Établit la baseline (prend un snapshot du système actuel comme référence).
 * À appeler au boot, avant l'activité normale.
 */
void territory_establish_baseline(TerritoryMap *map);

/**
 * Affiche un résumé de la carte.
 */
void territory_print_summary(const TerritoryMap *map);

/* ─── Macros Utilitaires ─────────────────────────────────────────────── */

#define TERRITORY_IS_READY(map)      ((map)->baseline_ready != 0)
#define TERRITORY_IS_HOSTILE(e)      ((e)->status == ELEM_HOSTILE)
#define TERRITORY_IS_TRUSTED(e)      ((e)->status == ELEM_TRUSTED)
#define TERRITORY_IS_SUSPICIOUS(e)   ((e)->status == ELEM_SUSPICIOUS)

#endif /* TERRITORY_MAP_H */
