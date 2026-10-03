#include "p8/mem.h"

#include <string.h>

/* All zero except the draw palette, which powers on as the identity map
 * with color 0 transparent -- PICO-8's own palt() reset state. An all-zero
 * table would instead mean "every color drawn as 0, nothing transparent". */
void p8_mem_init(p8_memory_t *mem) {
    int c;

    memset(mem, 0, sizeof(*mem));

    for (c = 0; c < 16; c++) {
        unsigned char entry = (unsigned char)c;
        if (c == 0) {
            entry = (unsigned char)(entry | P8_DRAW_PALETTE_TRANSPARENT_BIT);
        }
        mem->draw_palette[c] = entry;
    }
}

static int addr_in_range(int addr) {
    return addr >= 0 && addr < P8_MEM_SIZE;
}

unsigned char p8_peek(const p8_memory_t *mem, int addr) {
    if (!addr_in_range(addr)) {
        return 0;
    }
    return mem->data[addr];
}

void p8_poke(p8_memory_t *mem, int addr, unsigned char val) {
    if (!addr_in_range(addr)) {
        return;
    }
    mem->data[addr] = val;
}

void p8_memcpy(p8_memory_t *mem, int dest_addr, int src_addr, int len) {
    if (len <= 0) {
        return;
    }
    if (!addr_in_range(dest_addr) || !addr_in_range(dest_addr + len - 1)) {
        return;
    }
    if (!addr_in_range(src_addr) || !addr_in_range(src_addr + len - 1)) {
        return;
    }
    memmove(mem->data + dest_addr, mem->data + src_addr, (size_t)len);
}
