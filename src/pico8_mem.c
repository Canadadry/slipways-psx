#include "pico8_mem.h"

#include <string.h>

/* Zeroed memory, plus PICO-8's own documented power-on DRAW PALETTE at
 * 0x5f00: the identity map (color c drawn as color c), with color 0 -- and
 * only color 0 -- carrying the transparency bit. That is what the PICO-8
 * manual states palt() resets to ("all colours opaque except colour 0"),
 * and since this port's own palt()/spr() now share that one table
 * (pico8_gfx.h's own comment on pico8_palt) rather than a separate
 * gfx-side flag array, this is where that default has to live. In this
 * cart it is only ever observed before the first `cu()`/`dv()` pair runs
 * (pico8__init() calls cu() and _draw() opens with a bare dv(),
 * pico8_cart.c), which is precisely why a plain all-zero table -- "every
 * color maps to 0, nothing transparent" -- would be the wrong start. */
void pico8_mem_init(pico8_mem_t *mem) {
    int c;

    memset(mem->data, 0, sizeof(mem->data));

    for (c = 0; c < 16; c++) {
        unsigned char entry = (unsigned char)c;
        if (c == 0) {
            entry = (unsigned char)(entry | PICO8_DRAW_PALETTE_TRANSPARENT_BIT);
        }
        mem->data[PICO8_ADDR_DRAW_PALETTE + c] = entry;
    }
}

static int pico8_addr_in_range(int addr) {
    return addr >= 0 && addr < PICO8_MEM_SIZE;
}

unsigned char pico8_peek(const pico8_mem_t *mem, int addr) {
    if (!pico8_addr_in_range(addr)) {
        return 0;
    }
    return mem->data[addr];
}

void pico8_poke(pico8_mem_t *mem, int addr, unsigned char val) {
    if (!pico8_addr_in_range(addr)) {
        return;
    }
    mem->data[addr] = val;
}

void pico8_memcpy(pico8_mem_t *mem, int dest_addr, int src_addr, int len) {
    if (len <= 0) {
        return;
    }
    if (!pico8_addr_in_range(dest_addr) || !pico8_addr_in_range(dest_addr + len - 1)) {
        return;
    }
    if (!pico8_addr_in_range(src_addr) || !pico8_addr_in_range(src_addr + len - 1)) {
        return;
    }
    memmove(mem->data + dest_addr, mem->data + src_addr, (size_t)len);
}
