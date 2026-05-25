/* oo_weak_stubs.c — weak stubs for all remaining undefined symbols in KERNEL.EFI
 * These are overridden by real implementations when available.
 * Purpose: eliminate all R_X86_64_JUMP_SLOT relocations for undefined symbols
 * that would cause #GP at UEFI runtime (no dynamic linker exists). */
#include <stdint.h>
#include <stddef.h>

/* Large weak buffers — prevents .data overflow when real structs absent.
 * soma_dual.c (compiled) accesses g_sia.trust.level, .rhythm.current_mode,
 * g_sm_sia.initialized, .solar_chosen at deep offsets. 4-byte stubs overflow.
 * Real strong definitions override these weak symbols at link time. */
__attribute__((weak)) __attribute__((aligned(64))) char g_evolvion[4096] = {0};
__attribute__((weak)) __attribute__((aligned(64))) char g_sia[65536]     = {0};
__attribute__((weak)) __attribute__((aligned(64))) char g_sm_sia[1024]   = {0};

/* BPE tokenizer */
__attribute__((weak)) void* bpe_load(const char *p, int v) { (void)p;(void)v; return (void*)0; }
__attribute__((weak)) int   bpe_encode(void *b, const char *s, int *t, int n) { (void)b;(void)s;(void)t;(void)n; return 0; }
__attribute__((weak)) int   bpe_decode_token(void *b, int t, char *o, int n) { (void)b;(void)t;(void)o;(void)n; return 0; }

/* djiblas AVX2 */
__attribute__((weak)) void djiblas_sgemm_avx2(int m,int n,int k,float a,const float*A,const float*B,float b,float*C)
  { (void)m;(void)n;(void)k;(void)a;(void)A;(void)B;(void)b;(void)C; }

/* GGUF */
__attribute__((weak)) void* llmk_gguf_build_plan(const void *h, size_t s) { (void)h;(void)s; return (void*)0; }
__attribute__((weak)) size_t llmk_gguf_calc_llama2_q8_0_blob_bytes(const void *p) { (void)p; return 0; }
__attribute__((weak)) void  llmk_gguf_free_plan(void *p) { (void)p; }
__attribute__((weak)) int   llmk_gguf_load_into_llama2_layout(void *p, void *l) { (void)p;(void)l; return -1; }
__attribute__((weak)) int   llmk_gguf_load_into_llama2_q8_0_blob(void *p, void *b) { (void)p;(void)b; return -1; }
__attribute__((weak)) int   llmk_gguf_plan_supports_q8_0_blob(const void *p) { (void)p; return 0; }

/* Mamba SSM */
__attribute__((weak)) void mamba_conv1d_step(void *s, const void *c, int d) { (void)s;(void)c;(void)d; }
__attribute__((weak)) void mamba_matmul(float *o, const float *a, const float *b, int r, int c, int k) 
  { (void)o;(void)a;(void)b;(void)r;(void)c;(void)k; }
__attribute__((weak)) void mamba_rmsnorm(float *o, const float *x, const float *w, int n)
  { (void)o;(void)x;(void)w;(void)n; }

/* Hardware drivers */
__attribute__((weak)) int hda_init(void *c)            { (void)c; return -1; }
__attribute__((weak)) int nic_e1000_init(void *c)      { (void)c; return -1; }
__attribute__((weak)) int nvme_init(void *c)           { (void)c; return -1; }
__attribute__((weak)) int xhci_init(void *c)           { (void)c; return -1; }
__attribute__((weak)) int oo_virtio_net_init(void *c)  { (void)c; return -1; }

/* Bus */
__attribute__((weak)) void oo_bus_init(void *hermes, void *boot) { (void)hermes;(void)boot; }

/* Module table */
__attribute__((weak)) void oo_module_table_tick_all(void *ctx) { (void)ctx; }

/* Network packets */
__attribute__((weak)) void* oo_net_pkt_build(void *c, int t)          { (void)c;(void)t; return (void*)0; }
__attribute__((weak)) void* oo_net_pkt_from_bytes(const void *b, int n){ (void)b;(void)n; return (void*)0; }
__attribute__((weak)) void* oo_net_pkt_get_dna(const void *p)         { (void)p; return (void*)0; }
__attribute__((weak)) void  oo_net_pkt_set_dna(void *p, const void *d){ (void)p;(void)d; }
__attribute__((weak)) void  oo_net_pkt_set_text(void *p, const char *t){ (void)p;(void)t; }
__attribute__((weak)) int   oo_net_pkt_to_bytes(const void *p, void *b, int n){ (void)p;(void)b;(void)n; return 0; }

/* Swarm */
__attribute__((weak)) void  oo_swarm_packet_broadcast(void *p)        { (void)p; }
__attribute__((weak)) void* oo_swarm_packet_init(void *c)             { (void)c; return (void*)0; }
__attribute__((weak)) int   oo_swarm_packet_poll(void *c, void *p)    { (void)c;(void)p; return 0; }

/* Soma DNA */
__attribute__((weak)) uint32_t soma_dna_hash(const void *d, size_t n) { (void)d;(void)n; return 0; }
__attribute__((weak)) void soma_dna_init_default(void *d)             { (void)d; }
__attribute__((weak)) void soma_dna_mutate(void *d, uint32_t s)       { (void)d;(void)s; }
__attribute__((weak)) int  soma_dna_validate(const void *d)           { (void)d; return 1; }

/* Symbion */
__attribute__((weak)) int symbion_init(void *ctx) { (void)ctx; return 0; }

/* Log causal */
__attribute__((weak)) void _log_causal(const char *fmt, ...) { (void)fmt; }