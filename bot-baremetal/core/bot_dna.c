/**
 * BOT-BAREMETAL — BotDNA Implementation
 *
 * "Chaque agent sait qui il est, ce qu'il peut faire, et ce qu'il a appris."
 *
 * Contraintes strictes :
 *   - Aucune allocation dynamique (pools fixes uniquement)
 *   - Compatible bare-metal (pas de libc complète requise)
 *   - Sérialisable pour le journal OO
 */

#include "bot_dna.h"
#include <string.h>
#include <stdio.h>

/* ─── Implémentation SHA256 minimaliste (standalone) ─────────────────── */
/* Évite la dépendance à OpenSSL sur bare-metal.                          */

typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t  buf[64];
} sha256_ctx_t;

static const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
    0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
    0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
    0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
    0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
    0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define ROR32(x,n) (((x)>>(n))|((x)<<(32-(n))))
#define S0(x)  (ROR32(x,2)^ROR32(x,13)^ROR32(x,22))
#define S1(x)  (ROR32(x,6)^ROR32(x,11)^ROR32(x,25))
#define R0(x)  (ROR32(x,7)^ROR32(x,18)^((x)>>3))
#define R1(x)  (ROR32(x,17)^ROR32(x,19)^((x)>>10))
#define Ch(x,y,z) (((x)&(y))^(~(x)&(z)))
#define Maj(x,y,z) (((x)&(y))^((x)&(z))^((y)&(z)))

static void sha256_transform(sha256_ctx_t *ctx, const uint8_t *data) {
    uint32_t a,b,c,d,e,f,g,h,t1,t2,w[64];
    int i;
    for (i = 0; i < 16; i++)
        w[i] = ((uint32_t)data[i*4]<<24)|((uint32_t)data[i*4+1]<<16)
              |((uint32_t)data[i*4+2]<<8)|((uint32_t)data[i*4+3]);
    for (; i < 64; i++)
        w[i] = R1(w[i-2])+w[i-7]+R0(w[i-15])+w[i-16];
    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];
    for (i = 0; i < 64; i++) {
        t1 = h+S1(e)+Ch(e,f,g)+K[i]+w[i];
        t2 = S0(a)+Maj(a,b,c);
        h=g; g=f; f=e; e=d+t1;
        d=c; c=b; b=a; a=t1+t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_init(sha256_ctx_t *ctx) {
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85;
    ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c;
    ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
    ctx->count = 0;
}

static void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, uint32_t len) {
    uint32_t i, j = (uint32_t)(ctx->count & 63);
    ctx->count += len;
    for (i = 0; i < len; i++) {
        ctx->buf[j++] = data[i];
        if (j == 64) { sha256_transform(ctx, ctx->buf); j = 0; }
    }
}

static void sha256_final(sha256_ctx_t *ctx, uint8_t *digest) {
    uint64_t bits = ctx->count * 8;
    uint8_t pad[64]; int i;
    uint32_t j = (uint32_t)(ctx->count & 63);
    pad[0] = 0x80;
    uint32_t plen = (j < 56) ? (56 - j) : (120 - j);
    memset(pad+1, 0, plen-1);
    for (i = 0; i < 8; i++) pad[plen+i] = (uint8_t)(bits >> (56-8*i));
    sha256_update(ctx, pad, plen + 8);
    for (i = 0; i < 8; i++) {
        digest[i*4]   = (uint8_t)(ctx->state[i]>>24);
        digest[i*4+1] = (uint8_t)(ctx->state[i]>>16);
        digest[i*4+2] = (uint8_t)(ctx->state[i]>>8);
        digest[i*4+3] = (uint8_t)(ctx->state[i]);
    }
}

/* ─── Helpers internes ────────────────────────────────────────────────── */

/**
 * Calcule le hash SHA256 d'un BotDNA en excluant le champ integrity_hash.
 * Le hash est calculé sur tout sauf les 32 derniers bytes du hash lui-même.
 */
static void _bot_dna_compute_hash(const BotDNA *dna, uint8_t out[32]) {
    sha256_ctx_t ctx;
    sha256_init(&ctx);

    /* Hash tout le struct sauf integrity_hash et _reserved */
    size_t offset_hash = (uint8_t*)dna->integrity_hash - (uint8_t*)dna;
    sha256_update(&ctx, (const uint8_t*)dna, (uint32_t)offset_hash);

    /* Finalise */
    sha256_final(&ctx, out);
}

/* ─── Directives Primaires (scellées à la création) ──────────────────── */

static const char BOT_PRIME_DIRECTIVES[] =
    "1:SURVIVE_BEFORE_ALL\n"
    "2:PROTECT_OPERATING_ORGANISM\n"
    "3:LEARN_FROM_EVERY_THREAT\n"
    "4:NEVER_CORRUPT_SOMA\n"
    "5:NEVER_ACT_WITHOUT_EVIDENCE\n"
    "6:JOURNAL_EVERY_ACTION\n"
    "7:RESPECT_OO_HIERARCHY\n"
    "8:SHARE_KNOWLEDGE_WITH_SWARM\n"
    "9:REGENERATE_FALLEN_AGENTS\n"
    "10:ISOLATE_BEFORE_NEUTRALIZE\n";

/* ─── Implémentations Publiques ───────────────────────────────────────── */

void bot_dna_init(BotDNA *dna, AgentRole role, const char *name) {
    if (!dna) return;
    memset(dna, 0, sizeof(BotDNA));

    dna->magic   = BOT_DNA_MAGIC;
    dna->version = BOT_DNA_VERSION;
    dna->role    = role;
    dna->generation = 1;
    dna->is_healthy = 1;

    /* Copie du nom */
    strncpy(dna->name, name ? name : "unnamed", sizeof(dna->name) - 1);

    /* Capacités par défaut selon le rôle */
    switch (role) {
        case AGENT_ROLE_MEM_WATCH:
            dna->threat_domains = THREAT_DOMAIN_ANTIVIRUS | THREAT_DOMAIN_ANTIROOTKIT;
            break;
        case AGENT_ROLE_NET_WATCH:
            dna->threat_domains = THREAT_DOMAIN_ANTITHEFT | THREAT_DOMAIN_ANTIBOT;
            break;
        case AGENT_ROLE_FS_WATCH:
            dna->threat_domains = THREAT_DOMAIN_ANTIRANSOM | THREAT_DOMAIN_ANTITHEFT;
            break;
        case AGENT_ROLE_PROC_WATCH:
            dna->threat_domains = THREAT_DOMAIN_ANTIVIRUS | THREAT_DOMAIN_ANTIPIRACY;
            break;
        case AGENT_ROLE_KERNEL_WATCH:
            dna->threat_domains = THREAT_DOMAIN_ANTIROOTKIT | THREAT_DOMAIN_SELF_PROTECT;
            break;
        case AGENT_ROLE_BOOT_WATCH:
            dna->threat_domains = THREAT_DOMAIN_ANTIROOTKIT | THREAT_DOMAIN_SELF_PROTECT;
            break;
        case AGENT_ROLE_SWARM_MIND:
            dna->threat_domains = 0xFF; /* Voit tout */
            break;
        default:
            dna->threat_domains = 0;
            break;
    }

    /* Sceller les prime_directives (immuables après init) */
    size_t pd_len = strlen(BOT_PRIME_DIRECTIVES);
    if (pd_len >= BOT_PRIME_DIR_LEN)
        pd_len = BOT_PRIME_DIR_LEN - 1;
    memcpy(dna->prime_directives, BOT_PRIME_DIRECTIVES, pd_len);

    /* Calculer le hash initial */
    _bot_dna_compute_hash(dna, dna->integrity_hash);
}

int bot_dna_verify(const BotDNA *dna) {
    if (!dna) return 0;
    if (dna->magic != BOT_DNA_MAGIC) return 0;
    if (dna->version != BOT_DNA_VERSION) return 0;

    uint8_t computed[32];
    _bot_dna_compute_hash(dna, computed);

    return (memcmp(computed, dna->integrity_hash, 32) == 0) ? 1 : 0;
}

int bot_dna_add_pattern(BotDNA *dna, const BehaviorPattern *pattern) {
    if (!dna || !pattern) return -1;
    if (dna->pattern_count >= BOT_MAX_PATTERNS) return -1;

    memcpy(&dna->learned_patterns[dna->pattern_count], pattern,
           sizeof(BehaviorPattern));
    dna->pattern_count++;

    /* Recalcule le hash après modification */
    _bot_dna_compute_hash(dna, dna->integrity_hash);
    return 0;
}

int bot_dna_serialize(const BotDNA *dna, uint8_t *buf, uint32_t buf_len) {
    if (!dna || !buf) return -1;
    if (buf_len < sizeof(BotDNA)) return -1;
    memcpy(buf, dna, sizeof(BotDNA));
    return (int)sizeof(BotDNA);
}

int bot_dna_deserialize(BotDNA *dna, const uint8_t *buf, uint32_t buf_len) {
    if (!dna || !buf) return -1;
    if (buf_len < sizeof(BotDNA)) return -1;
    memcpy(dna, buf, sizeof(BotDNA));
    /* Vérification immédiate après désérialisation */
    if (!bot_dna_verify(dna)) {
        memset(dna, 0, sizeof(BotDNA));
        return -1;
    }
    return 0;
}

void bot_dna_print_summary(const BotDNA *dna) {
    if (!dna) { printf("[BotDNA] NULL\n"); return; }
    printf("[BotDNA] %-16s | role=0x%02x | gen=%u | healthy=%s\n",
           dna->name, (unsigned)dna->role, dna->generation,
           dna->is_healthy ? "YES" : "CORRUPTED");
    printf("         patterns=%u | threats=%llu | neutralized=%llu | FP=%llu\n",
           dna->pattern_count,
           (unsigned long long)dna->threats_detected,
           (unsigned long long)dna->threats_neutralized,
           (unsigned long long)dna->false_positives);
    printf("         domains=0x%08x | hash=%02x%02x%02x%02x...\n",
           dna->threat_domains,
           dna->integrity_hash[0], dna->integrity_hash[1],
           dna->integrity_hash[2], dna->integrity_hash[3]);
}
