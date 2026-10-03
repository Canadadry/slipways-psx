#include "p8/assets.h"
#include "p8/mem.h"

#include <assert.h>
#include <stdio.h>

static void test_gfx_json_loads_known_pixel(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    int ok = p8_load_gfx_json(&mem, "pico8/gfx.json");
    assert(ok);

    assert(p8_peek(&mem, P8_ADDR_SPRITE_SHEET + 0 * 128 + 16) == 3);
}

static void test_gfx_json_loads_full_128x128_range(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    int ok = p8_load_gfx_json(&mem, "pico8/gfx.json");
    assert(ok);

    assert(p8_peek(&mem, P8_ADDR_SPRITE_SHEET + 64 * 128 + 2) == 1);
    assert(p8_peek(&mem, P8_ADDR_SPRITE_SHEET + 127 * 128 + 0) == 3);
}

static void test_gfx_json_missing_file_returns_failure(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    int ok = p8_load_gfx_json(&mem, "pico8/does_not_exist.json");
    assert(!ok);
}

int main(void) {
    test_gfx_json_loads_known_pixel();
    test_gfx_json_loads_full_128x128_range();
    test_gfx_json_missing_file_returns_failure();

    printf("test_asset_loading: all tests passed\n");
    return 0;
}
