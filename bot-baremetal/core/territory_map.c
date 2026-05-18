/**
 * BOT-BAREMETAL — TerritoryMap Implementation
 *
 * "L'animal connaît son territoire.
 *  Il n'a pas besoin de réfléchir pour détecter un intrus."
 */

#include "territory_map.h"
#include <string.h>
#include <stdio.h>

/* ─── Helpers internes ───────────────────────────────────────────────────── */

static uint64_t _now_placeholder(void) {
    /* En production : syscall ou UEFI GetTime */
    /* Placeholder incrémental pour tests */
    static uint64_t t = 0;
    return ++t;
}

/* ─── Init ───────────────────────────────────────────────────────────────── */

void territory_map_init(TerritoryMap *map) {
    if (!map) return;
    memset(map, 0, sizeof(TerritoryMap));
    /* Tous les pools à zéro = ELEM_UNKNOWN pour les status */
}

/* ─── Processus ──────────────────────────────────────────────────────────── */

MappedProcess *territory_observe_process(TerritoryMap *map, uint32_t pid,
                                          uint32_t ppid, const char *name,
                                          const char *path) {
    if (!map) return NULL;

    /* Chercher si déjà connu */
    for (uint32_t i = 0; i < map->process_count; i++) {
        if (map->processes[i].pid == pid) {
            map->processes[i].last_seen = _now_placeholder();
            map->processes[i].observation_count++;
            map->total_observations++;
            return &map->processes[i];
        }
    }

    /* Nouveau processus */
    if (map->process_count >= TERRITORY_MAX_PROCESSES) {
        /* Pool plein : on écrase le plus ancien (ring buffer simple) */
        /* Pour l'instant : retourner NULL */
        return NULL;
    }

    MappedProcess *proc = &map->processes[map->process_count++];
    memset(proc, 0, sizeof(MappedProcess));

    proc->pid  = pid;
    proc->ppid = ppid;
    proc->status = ELEM_UNKNOWN;
    proc->first_seen = _now_placeholder();
    proc->last_seen  = proc->first_seen;
    proc->observation_count = 1;

    if (name) strncpy(proc->name, name, sizeof(proc->name) - 1);
    if (path) strncpy(proc->path, path, sizeof(proc->path) - 1);

    map->total_observations++;

    /* Enregistrer anomalie : nouveau processus inconnu */
    territory_record_anomaly(map, ANOMALY_NEW_PROCESS, 3, pid,
                             name ? name : "unknown");

    return proc;
}

void territory_mark_process(TerritoryMap *map, uint32_t pid, ElemStatus status) {
    if (!map) return;
    for (uint32_t i = 0; i < map->process_count; i++) {
        if (map->processes[i].pid == pid) {
            map->processes[i].status = status;
            return;
        }
    }
}

/* ─── Connexions Réseau ───────────────────────────────────────────────────── */

MappedConnection *territory_observe_connection(TerritoryMap *map,
                                                uint32_t pid,
                                                const uint8_t *remote_ip,
                                                uint16_t remote_port,
                                                uint64_t bytes_sent) {
    if (!map || !remote_ip) return NULL;

    /* Chercher connexion existante vers ce (ip, port) */
    for (uint32_t i = 0; i < map->connection_count; i++) {
        MappedConnection *c = &map->connections[i];
        if (c->remote_port == remote_port
            && memcmp(c->remote_ip, remote_ip, 4) == 0)
        {
            c->connection_count++;
            c->bytes_sent += bytes_sent;
            c->last_seen   = _now_placeholder();
            c->owning_pid  = pid;
            map->total_observations++;
            return c;
        }
    }

    /* Nouvelle connexion */
    if (map->connection_count >= TERRITORY_MAX_CONNECTIONS) return NULL;

    MappedConnection *conn = &map->connections[map->connection_count++];
    memset(conn, 0, sizeof(MappedConnection));

    memcpy(conn->remote_ip, remote_ip, 4); /* IPv4 pour l'instant */
    conn->remote_port     = remote_port;
    conn->owning_pid      = pid;
    conn->bytes_sent      = bytes_sent;
    conn->connection_count = 1;
    conn->status          = ELEM_UNKNOWN;
    conn->last_seen       = _now_placeholder();

    map->total_observations++;

    /* Anomalie : nouvelle destination jamais vue */
    char desc[64];
    snprintf(desc, sizeof(desc), "New connection to %u.%u.%u.%u:%u",
             remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3],
             remote_port);
    territory_record_anomaly(map, ANOMALY_UNKNOWN_CONNECTION, 2, pid, desc);

    return conn;
}

/* ─── Fichiers Surveillés ─────────────────────────────────────────────────── */

int territory_watch_file(TerritoryMap *map, const char *path, uint8_t is_critical) {
    if (!map || !path) return -1;
    if (map->watched_file_count >= TERRITORY_MAX_WATCHED_FILES) return -1;

    WatchedFile *f = &map->watched_files[map->watched_file_count++];
    memset(f, 0, sizeof(WatchedFile));

    strncpy(f->path, path, sizeof(f->path) - 1);
    f->is_critical  = is_critical;
    f->is_intact    = 1;
    f->last_verified = _now_placeholder();

    /* Hash attendu = 0 pour l'instant (à remplir au premier scan sain) */
    memset(f->expected_hash, 0, sizeof(f->expected_hash));
    memset(f->current_hash,  0, sizeof(f->current_hash));

    return 0;
}

int territory_verify_files(TerritoryMap *map) {
    if (!map) return 0;
    int modified = 0;

    for (uint32_t i = 0; i < map->watched_file_count; i++) {
        WatchedFile *f = &map->watched_files[i];
        if (!f->is_intact) modified++;
    }
    return modified;
}

/* ─── Régions Mémoire ─────────────────────────────────────────────────────── */

MappedMemRegion *territory_observe_mem(TerritoryMap *map, uint64_t base,
                                        uint64_t size, uint32_t pid,
                                        uint8_t exec, uint8_t write) {
    if (!map) return NULL;

    /* Chercher région existante */
    for (uint32_t i = 0; i < map->mem_region_count; i++) {
        MappedMemRegion *r = &map->mem_regions[i];
        if (r->base_addr == base && r->owning_pid == pid) {
            /* Détecter changement de permissions : write+exec = DANGEREUX */
            if (exec && write && (!r->is_executable || !r->is_writable)) {
                r->status = ELEM_SUSPICIOUS;
                territory_record_anomaly(map, ANOMALY_MEM_EXEC_WRITE, 7,
                                         pid, "Region became W+X");
            }
            r->is_executable = exec;
            r->is_writable   = write;
            return r;
        }
    }

    /* Nouvelle région */
    if (map->mem_region_count >= TERRITORY_MAX_MEM_REGIONS) return NULL;

    MappedMemRegion *region = &map->mem_regions[map->mem_region_count++];
    memset(region, 0, sizeof(MappedMemRegion));

    region->base_addr    = base;
    region->size         = size;
    region->owning_pid   = pid;
    region->is_executable = exec;
    region->is_writable  = write;
    region->status       = (exec && write) ? ELEM_SUSPICIOUS : ELEM_UNKNOWN;

    if (exec && write) {
        territory_record_anomaly(map, ANOMALY_MEM_EXEC_WRITE, 8,
                                  pid, "New W+X region — possible shellcode");
    }

    map->total_observations++;
    return region;
}

/* ─── Anomalies ──────────────────────────────────────────────────────────── */

void territory_record_anomaly(TerritoryMap *map, uint8_t type, uint8_t severity,
                               uint32_t pid, const char *description) {
    if (!map) return;

    uint32_t idx = map->anomaly_head % TERRITORY_MAX_ANOMALIES;
    TerritoryAnomaly *a = &map->anomalies[idx];

    a->timestamp    = _now_placeholder();
    a->pid          = pid;
    a->anomaly_type = type;
    a->severity     = severity;

    if (description)
        strncpy(a->description, description, sizeof(a->description) - 1);

    map->anomaly_head++;
    if (map->anomaly_count < TERRITORY_MAX_ANOMALIES)
        map->anomaly_count++;

    map->total_anomalies++;
}

const TerritoryAnomaly *territory_last_anomaly(const TerritoryMap *map) {
    if (!map || map->anomaly_count == 0) return NULL;
    uint32_t last = (map->anomaly_head - 1 + TERRITORY_MAX_ANOMALIES)
                    % TERRITORY_MAX_ANOMALIES;
    return &map->anomalies[last];
}

/* ─── Baseline ───────────────────────────────────────────────────────────── */

void territory_establish_baseline(TerritoryMap *map) {
    if (!map) return;
    map->baseline_ready = 1;
    map->baseline_time  = _now_placeholder();

    /* Valider tout ce qui est déjà connu comme TRUSTED */
    for (uint32_t i = 0; i < map->process_count; i++) {
        if (map->processes[i].status == ELEM_UNKNOWN)
            map->processes[i].status = ELEM_TRUSTED;
    }
    for (uint32_t i = 0; i < map->connection_count; i++) {
        if (map->connections[i].status == ELEM_UNKNOWN)
            map->connections[i].status = ELEM_TRUSTED;
    }
    for (uint32_t i = 0; i < map->mem_region_count; i++) {
        if (map->mem_regions[i].status == ELEM_UNKNOWN)
            map->mem_regions[i].status = ELEM_TRUSTED;
    }

    printf("[Territory] Baseline established: %u procs / %u conns / "
           "%u files / %u mem-regions\n",
           map->process_count, map->connection_count,
           map->watched_file_count, map->mem_region_count);
}

/* ─── Résumé ─────────────────────────────────────────────────────────────── */

void territory_print_summary(const TerritoryMap *map) {
    if (!map) { printf("[TerritoryMap] NULL\n"); return; }

    printf("[TerritoryMap] baseline=%s | procs=%u | conns=%u | "
           "files=%u | mem=%u | anomalies=%llu\n",
           map->baseline_ready ? "READY" : "NOT_SET",
           map->process_count,
           map->connection_count,
           map->watched_file_count,
           map->mem_region_count,
           (unsigned long long)map->total_anomalies);

    /* Processus hostiles */
    uint32_t hostile = 0;
    for (uint32_t i = 0; i < map->process_count; i++)
        if (map->processes[i].status >= ELEM_SUSPICIOUS) hostile++;
    if (hostile > 0)
        printf("[TerritoryMap] !! %u hostile/suspicious process(es) !!\n", hostile);

    /* Dernière anomalie */
    const TerritoryAnomaly *a = territory_last_anomaly(map);
    if (a) {
        printf("[TerritoryMap] Last anomaly: type=0x%02x sev=%u pid=%u — %s\n",
               a->anomaly_type, a->severity, a->pid, a->description);
    }
}
