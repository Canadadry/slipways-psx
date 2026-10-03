#include "p8/assets.h"
#include "game/background.h"
#include "p8/gfx.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdio.h>


#define BACKGROUND_STAR_CELL_X 2
#define BACKGROUND_STAR_CELL_Y 1
#define BACKGROUND_STAR_SPRITE 72

static unsigned char screen_at(const p8_vram_t *mem, int x, int y) {
    return mem->screen[y][x];
}

static void setup_sky(p8_vram_t *mem, p8_graphics_t *gfx) {
    int ok;
    p8_vram_init(mem);
    p8_gfx_init(gfx);
    ok = p8_load_gfx_json(mem, "pico8/gfx.json");
    assert(ok);
    p8_mset(mem, BACKGROUND_STAR_CELL_X, BACKGROUND_STAR_CELL_Y, BACKGROUND_STAR_SPRITE);
}

static void test_background_draw_leaves_empty_map_cells_black(void) {
    static const int probe_cells[][2] = {{1, 1}, {6, 2}, {2, 5}, {13, 6}, {10, 10}, {15, 15}};
    const int probe_count = (int)(sizeof(probe_cells) / sizeof(probe_cells[0]));
    p8_vram_t mem;
    p8_graphics_t gfx;

    setup_sky(&mem, &gfx);

    background_draw(&gfx, &mem, NULL);

    for (int i = 0; i < probe_count; i++) {
        int cx = probe_cells[i][0];
        int cy = probe_cells[i][1];
        for (int dy = 0; dy < P8_SPRITE_SIZE; dy++) {
            for (int dx = 0; dx < P8_SPRITE_SIZE; dx++) {
                assert(screen_at(&mem, cx * P8_SPRITE_SIZE + dx, cy * P8_SPRITE_SIZE + dy) == 0);
            }
        }
    }
}

static void test_background_draw_still_draws_non_empty_map_cells(void) {
    p8_vram_t mem;
    p8_graphics_t gfx;
    int star_x = BACKGROUND_STAR_CELL_X * P8_SPRITE_SIZE;
    int star_y = BACKGROUND_STAR_CELL_Y * P8_SPRITE_SIZE;
    int lit = 0;

    setup_sky(&mem, &gfx);

    background_draw(&gfx, &mem, NULL);

    for (int dy = 0; dy < P8_SPRITE_SIZE; dy++) {
        for (int dx = 0; dx < P8_SPRITE_SIZE; dx++) {
            unsigned char expected = p8_sget(&mem, (BACKGROUND_STAR_SPRITE % 16) * P8_SPRITE_SIZE + dx,
                                                (BACKGROUND_STAR_SPRITE / 16) * P8_SPRITE_SIZE + dy);
            assert(screen_at(&mem, star_x + dx, star_y + dy) == expected);
            if (expected != 0) {
                lit++;
            }
        }
    }
    assert(lit == 1);
}

int main(void) {
    test_background_draw_leaves_empty_map_cells_black();
    test_background_draw_still_draws_non_empty_map_cells();

    printf("test_background_draw: all tests passed\n");
    return 0;
}
