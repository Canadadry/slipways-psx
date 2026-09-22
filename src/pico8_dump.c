#include "pico8_dump.h"

#include "pico8_gfx.h"

#include <assert.h>

static const char HEX_DIGITS[] = "0123456789abcdef";

int pico8_dump_screen(const pico8_mem_t *mem, FILE *out) {
    if (fprintf(out, "%d %d\n", PICO8_SCREEN_WIDTH, PICO8_SCREEN_HEIGHT) < 0) {
        return 0;
    }

    for (int y = 0; y < PICO8_SCREEN_HEIGHT; y++) {
        char line[PICO8_SCREEN_WIDTH + 1];
        for (int x = 0; x < PICO8_SCREEN_WIDTH; x++) {
            unsigned char pixel = pico8_peek(mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x);
            assert(pixel <= 0x0f && "screen memory holds one 4-bit color index per byte");
            line[x] = HEX_DIGITS[pixel];
        }
        line[PICO8_SCREEN_WIDTH] = '\n';
        if (fwrite(line, 1, sizeof(line), out) != sizeof(line)) {
            return 0;
        }
    }

    return 1;
}
