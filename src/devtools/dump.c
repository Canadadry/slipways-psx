#include "pico8_dump.h"

#include "p8/gfx.h"

#include <assert.h>

static const char HEX_DIGITS[] = "0123456789abcdef";

int pico8_dump_pixels(const unsigned char *pixels, FILE *out) {
    if (fprintf(out, "%d %d\n", PICO8_SCREEN_WIDTH, PICO8_SCREEN_HEIGHT) < 0) {
        return 0;
    }

    for (int y = 0; y < PICO8_SCREEN_HEIGHT; y++) {
        char line[PICO8_SCREEN_WIDTH + 1];
        for (int x = 0; x < PICO8_SCREEN_WIDTH; x++) {
            unsigned char pixel = pixels[y * PICO8_SCREEN_WIDTH + x];
            assert(pixel <= 0x0f && "one 4-bit color index per byte");
            line[x] = HEX_DIGITS[pixel];
        }
        line[PICO8_SCREEN_WIDTH] = '\n';
        if (fwrite(line, 1, sizeof(line), out) != sizeof(line)) {
            return 0;
        }
    }

    return 1;
}

int pico8_dump_screen(const pico8_mem_t *mem, FILE *out) {
    unsigned char pixels[PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT];
    for (int i = 0; i < (int)sizeof(pixels); i++) {
        pixels[i] = pico8_peek(mem, PICO8_ADDR_SCREEN + i);
    }
    return pico8_dump_pixels(pixels, out);
}
