#include "p8/mem.h"

#include <string.h>

/* All zero except the draw palette, which powers on as the identity map
 * with color 0 transparent -- PICO-8's own palt() reset state. An all-zero
 * table would instead mean "every color drawn as 0, nothing transparent". */
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
