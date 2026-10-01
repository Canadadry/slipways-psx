
#include "p8/mem.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "../src/pico8_dump.h"

#include <assert.h>
#include <stdio.h>

#define DEMO_EVENTS_PATH "tests/fixtures/demo_events.json"

static unsigned char screen_pixel(const pico8_mem_t *mem, int x, int y) {
    return pico8_peek(mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x);
}

int main(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    pico8_gfx_t gfx;
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    assert(pico8_sget(&mem, 16, 0) == 3);
    /* map.json's cells row 0 lands in PICO8_ADDR_SPRITE_MAP_SHARED on the
     * real cart (confirmed against pico8/original.p8.png's decoded ROM). */
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_MAP_SHARED + 1) == 81);

    pico8_cls(&mem);
    assert(screen_pixel(&mem, 0, 0) == 0);
    assert(screen_pixel(&mem, 127, 127) == 0);

    pico8_line(&gfx, &mem, 0, 5, 4, 5, 8);
    for (int x = 0; x <= 4; x++) {
        assert(screen_pixel(&mem, x, 5) == 8);
    }
    assert(screen_pixel(&mem, 5, 5) == 0);

    pico8_pset(&gfx, &mem, 10, 20, 7);
    assert(screen_pixel(&mem, 10, 20) == 7);
    assert(screen_pixel(&mem, 11, 20) == 0);

    pico8_rect(&gfx, &mem, 30, 30, 34, 34, 9);
    for (int x = 30; x <= 34; x++) {
        assert(screen_pixel(&mem, x, 30) == 9);
        assert(screen_pixel(&mem, x, 34) == 9);
    }
    for (int y = 30; y <= 34; y++) {
        assert(screen_pixel(&mem, 30, y) == 9);
        assert(screen_pixel(&mem, 34, y) == 9);
    }
    assert(screen_pixel(&mem, 32, 32) == 0);

    pico8_circ(&gfx, &mem, 60, 60, 3, 10);
    assert(screen_pixel(&mem, 63, 60) == 10);
    assert(screen_pixel(&mem, 57, 60) == 10);
    assert(screen_pixel(&mem, 60, 63) == 10);
    assert(screen_pixel(&mem, 60, 57) == 10);
    assert(screen_pixel(&mem, 60, 60) == 0);

    pico8_spr(&gfx, &mem, 2, 0, 90, 1, 1);
    assert(screen_pixel(&mem, 0, 90) == 3);
    assert(screen_pixel(&mem, 7, 90) == 3);
    assert(screen_pixel(&mem, 2, 91) == 4);
    assert(screen_pixel(&mem, 1, 90) == 0);

    pico8_map(&gfx, &mem, 0, 0, 0, 100, 4, 1);
    /* The cart's map cell (0,0) is 0, and `map()` takes sprite 0 to mean
     * empty and does not draw it, so the whole first 8x8 block stays
     * cleared -- this cart's sprite 0 is palette data, not a tile. */
    for (int y = 100; y <= 107; y++) {
        for (int x = 0; x <= 7; x++) {
            assert(screen_pixel(&mem, x, y) == 0);
        }
    }
    assert(screen_pixel(&mem, 8, 100) == 0);
    assert(screen_pixel(&mem, 16, 100) == 1);
    assert(screen_pixel(&mem, 31, 107) == 6);

    pico8_print(&gfx, &mem, "hi", 0, 115, 11);
    assert(screen_pixel(&mem, 0, 115) == 11);
    assert(screen_pixel(&mem, 1, 115) == 0);
    assert(screen_pixel(&mem, 2, 117) == 11);
    assert(screen_pixel(&mem, 5, 115) == 11);
    assert(screen_pixel(&mem, 4, 116) == 0);
    assert(screen_pixel(&mem, 5, 116) == 11);

    /* This block asserts the Phase-1 four-frame scenario's exact stat()/
     * btn() values, so it needs that exact scenario -- not whatever the
     * root events.json happens to hold. It read "events.json" until
     * 741d633 ("chore: use segfault recording as root events.json")
     * replaced that file with a 9729-frame playtest recording, which
     * aborted this test on its first assertion. The scenario now lives in
     * a fixture of its own, where a recording swap cannot reach it; the
     * root recording is still covered, for shape and loadability rather
     * than fixed values, by tests/test_pico8_input.c. */
    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, DEMO_EVENTS_PATH));

    assert(pico8_stat(&input, 32) == 64);
    assert(pico8_stat(&input, 33) == 32);
    assert(pico8_stat(&input, 34) == 1);
    assert(pico8_btn(&input, 0) == 0);
    assert(pico8_btn(&input, 1) == 1);
    assert(pico8_btn(&input, 2) == 0);
    assert(pico8_btn(&input, 3) == 0);

    pico8_input_step(&input);

    assert(pico8_stat(&input, 32) == 70);
    assert(pico8_stat(&input, 33) == 40);
    assert(pico8_stat(&input, 34) == 0);
    assert(pico8_btn(&input, 0) == 0);
    assert(pico8_btn(&input, 1) == 0);
    assert(pico8_btn(&input, 2) == 1);
    assert(pico8_btn(&input, 3) == 0);

    pico8_input_free(&input);

    FILE *dump_out = fopen("tmp/pico8_demo_dump.txt", "w");
    assert(dump_out != NULL);
    int dump_ok = pico8_dump_screen(&mem, dump_out);
    fclose(dump_out);
    assert(dump_ok);

    printf("pico8 demo: all assertions passed; framebuffer dumped to tmp/pico8_demo_dump.txt\n");
    return 0;
}
