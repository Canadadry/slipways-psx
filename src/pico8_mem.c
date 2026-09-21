#include "pico8_mem.h"

#include <string.h>

void pico8_mem_init(pico8_mem_t *mem) {
    memset(mem->data, 0, sizeof(mem->data));
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
