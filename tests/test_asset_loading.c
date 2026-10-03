#include "p8/assets.h"
#include "p8/mem.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_gfx_json_loads_known_pixel(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    int ok = p8_load_gfx_json(&mem, "pico8/gfx.json");
    assert(ok);

    assert(mem.sprite_sheet[0][16] == 3);
}

static void test_gfx_json_loads_full_128x128_range(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    int ok = p8_load_gfx_json(&mem, "pico8/gfx.json");
    assert(ok);

    assert(mem.sprite_sheet[64][2] == 1);
    assert(mem.sprite_sheet[127][0] == 3);
}

static void test_gfx_json_missing_file_returns_failure(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    int ok = p8_load_gfx_json(&mem, "pico8/does_not_exist.json");
    assert(!ok);
}

/* The fixture holds 129 rows of 128 values, value i being (i % 15) + 1. */
static void test_gfx_json_values_past_the_sheet_touch_nothing_else(void) {
    static p8_memory_t mem;
    static p8_memory_t expected;
    p8_mem_init(&mem);
    p8_mem_init(&expected);
    for (int y = 0; y < P8_SPRITE_SHEET_HEIGHT; y++) {
        for (int x = 0; x < P8_SPRITE_SHEET_WIDTH; x++) {
            expected.sprite_sheet[y][x] = (unsigned char)((y * P8_SPRITE_SHEET_WIDTH + x) % 15 + 1);
        }
    }

    assert(p8_load_gfx_json(&mem, "tests/fixtures/gfx_oversized.json"));

    assert(memcmp(&mem, &expected, sizeof(mem)) == 0);
}

int main(void) {
    test_gfx_json_loads_known_pixel();
    test_gfx_json_loads_full_128x128_range();
    test_gfx_json_missing_file_returns_failure();
    test_gfx_json_values_past_the_sheet_touch_nothing_else();

    printf("test_asset_loading: all tests passed\n");
    return 0;
}
