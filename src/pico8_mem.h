#ifndef PICO8_MEM_H
#define PICO8_MEM_H


#include <stddef.h>

#define PICO8_MEM_SIZE 0x8000

#define PICO8_ADDR_SPRITE_SHEET 0x0000
#define PICO8_ADDR_SPRITE_MAP_SHARED 0x1000
#define PICO8_ADDR_MAP 0x2000
#define PICO8_ADDR_SCREEN 0x6000

typedef struct {
    unsigned char data[PICO8_MEM_SIZE];
} pico8_mem_t;

void pico8_mem_init(pico8_mem_t *mem);

unsigned char pico8_peek(const pico8_mem_t *mem, int addr);

void pico8_poke(pico8_mem_t *mem, int addr, unsigned char val);

void pico8_memcpy(pico8_mem_t *mem, int dest_addr, int src_addr, int len);

#endif
