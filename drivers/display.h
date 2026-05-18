#pragma once

#include <stdint.h>

/**
 * Display Driver — Sovereign Visuals
 */

typedef struct {
    uint32_t *framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    int      active;
} DisplayCtx;

/**
 * Initializes the display using the provided framebuffer address.
 */
void display_init(DisplayCtx *ctx, void *addr, uint32_t w, uint32_t h, uint32_t p);

/**
 * Plots a pixel (ARGB format).
 */
static inline void display_put_pixel(DisplayCtx *ctx, uint32_t x, uint32_t y, uint32_t color) {
    if (x >= ctx->width || y >= ctx->height) return;
    ctx->framebuffer[y * ctx->pitch + x] = color;
}

/**
 * Clears the screen.
 */
void display_clear(DisplayCtx *ctx, uint32_t color);
