#include "../src/pico8_assets.h"
#include "../src/pico8_bg.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_mem.h"

#include <assert.h>
#include <stdio.h>


#define BG_STAR_CELL_X 2
#define BG_STAR_CELL_Y 1
#define BG_STAR_SPRITE 72

static unsigned char screen_at(const pico8_mem_t *mem, int x, int y) {
    return pico8_peek(mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x);
}

static void setup_sky(pico8_mem_t *mem, pico8_gfx_t *gfx) {
    int ok;
    pico8_mem_init(mem);
    pico8_gfx_init(gfx);
    ok = pico8_load_gfx_json(mem, "pico8/gfx.json");
    assert(ok);
    pico8_mset(mem, BG_STAR_CELL_X, BG_STAR_CELL_Y, BG_STAR_SPRITE);
}

static void test_bg_kj_leaves_empty_map_cells_black(void) {
    static const int probe_cells[][2] = {{1, 1}, {6, 2}, {2, 5}, {13, 6}, {10, 10}, {15, 15}};
    const int probe_count = (int)(sizeof(probe_cells) / sizeof(probe_cells[0]));
    pico8_mem_t mem;
    pico8_gfx_t gfx;

    setup_sky(&mem, &gfx);

    pico8_bg_kj(&gfx, &mem, NULL);

    for (int i = 0; i < probe_count; i++) {
        int cx = probe_cells[i][0];
        int cy = probe_cells[i][1];
        for (int dy = 0; dy < PICO8_SPRITE_SIZE; dy++) {
            for (int dx = 0; dx < PICO8_SPRITE_SIZE; dx++) {
                assert(screen_at(&mem, cx * PICO8_SPRITE_SIZE + dx, cy * PICO8_SPRITE_SIZE + dy) == 0);
            }
        }
    }
}

static void test_bg_kj_still_draws_non_empty_map_cells(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    int star_x = BG_STAR_CELL_X * PICO8_SPRITE_SIZE;
    int star_y = BG_STAR_CELL_Y * PICO8_SPRITE_SIZE;
    int lit = 0;

    setup_sky(&mem, &gfx);

    pico8_bg_kj(&gfx, &mem, NULL);

    for (int dy = 0; dy < PICO8_SPRITE_SIZE; dy++) {
        for (int dx = 0; dx < PICO8_SPRITE_SIZE; dx++) {
            unsigned char expected = pico8_sget(&mem, (BG_STAR_SPRITE % 16) * PICO8_SPRITE_SIZE + dx,
                                                (BG_STAR_SPRITE / 16) * PICO8_SPRITE_SIZE + dy);
            assert(screen_at(&mem, star_x + dx, star_y + dy) == expected);
            if (expected != 0) {
                lit++;
            }
        }
    }
    assert(lit == 1);
}

int main(void) {
    test_bg_kj_leaves_empty_map_cells_black();
    test_bg_kj_still_draws_non_empty_map_cells();

    printf("test_pico8_bg_kj: all tests passed\n");
    return 0;
}
