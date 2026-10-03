#include "p8/vram.h"

#include <string.h>

/* All zero except the draw palette, which powers on as the identity map
 * with color 0 transparent -- PICO-8's own palt() reset state. An all-zero
 * table would instead mean "every color drawn as 0, nothing transparent". */
void p8_vram_init(p8_vram_t *mem) {
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
