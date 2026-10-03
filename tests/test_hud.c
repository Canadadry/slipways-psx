#include "game/hud.h"
#include "p8/assets.h"

#include <assert.h>
#include <stdio.h>

static const unsigned char block_8[16] = {0, 1, 2, 131, 4, 5, 6, 7, 10, 9, 0, 4, 2, 13, 0, 0};

static void assert_draw_palette_is_block_8(const p8_vram_t *mem) {
    for (int c = 0; c < 16; c++) {
        assert(mem->draw_palette[c] == block_8[c]);
    }
}

/* The cart encodes block n, color c as sprite-sheet pixel (n, c), color 3
 * with the transparency bit. */
static void test_each_block_matches_the_sprite_sheet_pixels(void) {
    p8_vram_t mem;
    p8_vram_init(&mem);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));

    for (int n = 0; n < 16; n++) {
        hud_select_palette_block(&mem, p8_fixed_from_int(n));
        for (int c = 0; c < 16; c++) {
            unsigned char expected = (unsigned char)(p8_sget(&mem, n, c) | (c == 3 ? P8_DRAW_PALETTE_TRANSPARENT_BIT : 0));
            if (mem.draw_palette[c] != expected) {
                fprintf(stderr, "block %d, color %d: %d, expected %d\n", n, c,
                        mem.draw_palette[c], expected);
                assert(0);
            }
        }
    }
}

static void test_select_needs_no_sprite_sheet(void) {
    p8_vram_t mem;
    p8_vram_init(&mem);

    hud_select_palette_block(&mem, p8_fixed_from_int(8));

    assert_draw_palette_is_block_8(&mem);
}

static void test_fractional_index_selects_its_floor(void) {
    p8_vram_t mem;
    p8_vram_init(&mem);

    hud_select_palette_block(&mem, P8_FIXED_LIT(0x0008e666, 8.9));

    assert_draw_palette_is_block_8(&mem);
}

int main(void) {
    test_each_block_matches_the_sprite_sheet_pixels();
    test_select_needs_no_sprite_sheet();
    test_fractional_index_selects_its_floor();

    printf("test_hud: OK\n");
    return 0;
}
