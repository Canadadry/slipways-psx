#ifndef PICO8_MEM_H
#define PICO8_MEM_H


#include <stddef.h>

#define PICO8_ADDR_SPRITE_SHEET 0x6000
#define PICO8_ADDR_SPRITE_MAP_SHARED 0x1000
#define PICO8_ADDR_MAP 0x2000
#define PICO8_ADDR_SCREEN 0xA000

/* The live draw palette: one byte per source color 0-15, low bits giving
 * the color it is actually drawn as, bit 0x80 meaning "not drawn at all".
 * pico8_palt() and dv() write these same bytes. */
#define PICO8_ADDR_DRAW_PALETTE 0x5f00
#define PICO8_DRAW_PALETTE_TRANSPARENT_BIT 0x80

#define PICO8_MEM_SIZE 0xE000

typedef struct {
    unsigned char data[PICO8_MEM_SIZE];
} pico8_mem_t;

void pico8_mem_init(pico8_mem_t *mem);

/* An out-of-range address is ignored rather than an error: pico8_peek
 * returns 0, pico8_poke and pico8_memcpy write nothing at all. Overlapping
 * source and destination are allowed. */
unsigned char pico8_peek(const pico8_mem_t *mem, int addr);

void pico8_poke(pico8_mem_t *mem, int addr, unsigned char val);

void pico8_memcpy(pico8_mem_t *mem, int dest_addr, int src_addr, int len);

#endif
