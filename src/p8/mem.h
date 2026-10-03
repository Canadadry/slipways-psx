#ifndef P8_MEM_H
#define P8_MEM_H


#include <stddef.h>

#define P8_ADDR_SPRITE_MAP_SHARED 0x1000
#define P8_ADDR_MAP 0x2000
#define P8_ADDR_SCREEN 0xA000

#define P8_DRAW_PALETTE_TRANSPARENT_BIT 0x80

#define P8_SPRITE_SHEET_WIDTH 128
#define P8_SPRITE_SHEET_HEIGHT 128

#define P8_MEM_SIZE 0xE000

typedef struct {
    unsigned char data[P8_MEM_SIZE];
    /* One byte per source color 0-15, low bits giving the color it is
     * actually drawn as, P8_DRAW_PALETTE_TRANSPARENT_BIT meaning "not drawn
     * at all". p8_palt() and hud_select_palette_block() write these same
     * bytes. */
    unsigned char draw_palette[16];
    unsigned char sprite_sheet[P8_SPRITE_SHEET_HEIGHT][P8_SPRITE_SHEET_WIDTH];
} p8_memory_t;

void p8_mem_init(p8_memory_t *mem);

/* An out-of-range address is ignored rather than an error: p8_peek
 * returns 0, p8_poke and p8_memcpy write nothing at all. Overlapping
 * source and destination are allowed. */
unsigned char p8_peek(const p8_memory_t *mem, int addr);

void p8_poke(p8_memory_t *mem, int addr, unsigned char val);

void p8_memcpy(p8_memory_t *mem, int dest_addr, int src_addr, int len);

#endif
