#ifndef PICO8_MEM_H
#define PICO8_MEM_H


#include <stddef.h>

#define PICO8_ADDR_SPRITE_SHEET 0x6000
#define PICO8_ADDR_SPRITE_MAP_SHARED 0x1000
#define PICO8_ADDR_MAP 0x2000
#define PICO8_ADDR_SCREEN 0xA000

/* Real PICO-8's own active draw-palette table (`pal()`'s backing memory,
 * `dv()`'s own literal `memcpy(0x5f00,...)` target -- pico8_hud.c). Each
 * of the 16 bytes here is one source color 0-15's current mapping: bit
 * 0x80 set means "this color is not drawn at all" (the same bit `palt()`
 * itself would toggle on real hardware -- this cart instead bakes it
 * straight into the table `cu()` builds, see pico8_hud.c's own top
 * comment), the low bits give the color it's actually drawn as. */
#define PICO8_ADDR_DRAW_PALETTE 0x5f00
#define PICO8_DRAW_PALETTE_TRANSPARENT_BIT 0x80

#define PICO8_MEM_SIZE 0xE000

typedef struct {
    unsigned char data[PICO8_MEM_SIZE];
} pico8_mem_t;

void pico8_mem_init(pico8_mem_t *mem);

unsigned char pico8_peek(const pico8_mem_t *mem, int addr);

void pico8_poke(pico8_mem_t *mem, int addr, unsigned char val);

void pico8_memcpy(pico8_mem_t *mem, int dest_addr, int src_addr, int len);

#endif
