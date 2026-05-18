#include "display.h"
#include <string.h>

void display_init(DisplayCtx *ctx, void *addr, uint32_t w, uint32_t h, uint32_t p) {
    if (!ctx) return;
    ctx->framebuffer = (uint32_t*)addr;
    ctx->width = w;
    ctx->height = h;
    ctx->pitch = p;
    ctx->active = 1;
}

void display_clear(DisplayCtx *ctx, uint32_t color) {
    if (!ctx || !ctx->active) return;
    
    uint32_t total_pixels = ctx->height * ctx->pitch;
    for (uint32_t i = 0; i < total_pixels; i++) {
        ctx->framebuffer[i] = color;
    }
}
