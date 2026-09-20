#include "../src/pico8_assets.h"
#include "../src/pico8_mem.h"

#include <assert.h>
#include <stdio.h>

static void test_gfx_json_loads_known_pixel(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    int ok = pico8_load_gfx_json(&mem, "pico8/gfx.json");
    assert(ok);

    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_SHEET + 0 * 128 + 16) == 3);
}

static void test_gfx_json_loads_full_128x128_range(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    int ok = pico8_load_gfx_json(&mem, "pico8/gfx.json");
    assert(ok);

    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_SHEET + 64 * 128 + 2) == 1);
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_SHEET + 127 * 128 + 0) == 3);
}

static void test_map_json_loads_base_region(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    int ok = pico8_load_map_json(&mem, "pico8/map.json");
    assert(ok);

    assert(pico8_peek(&mem, PICO8_ADDR_MAP + 0 * 128 + 1) == 81);
    assert(pico8_peek(&mem, PICO8_ADDR_MAP + 0 * 128 + 9) == 213);
    assert(pico8_peek(&mem, PICO8_ADDR_MAP + 31 * 128 + 0) == 51);
}

static void test_map_json_loads_shared_region_for_rows_32_to_63(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    int ok = pico8_load_map_json(&mem, "pico8/map.json");
    assert(ok);

    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_MAP_SHARED + 0 * 128 + 0) == 51);
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_MAP_SHARED + 0 * 128 + 1) == 6);
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_MAP_SHARED + 31 * 128 + 0) == 41);
}

static void test_gfx_and_map_loaded_into_same_instance(void) {
    /* Regression test: gfx.json's sprite sheet load and map.json's map
     * load must not clobber each other when both land in the same
     * pico8_mem_t, since the real cart loads both into one address
     * space. This is the scenario the original packed-hardware-sized
     * (0x2000-byte) sprite sheet/screen regions got wrong: a full
     * unpacked 128x128 sprite sheet needs 0x4000 bytes, which used to
     * spill into the map's 0x1000-0x2fff span. */
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    int gfx_ok = pico8_load_gfx_json(&mem, "pico8/gfx.json");
    assert(gfx_ok);
    int map_ok = pico8_load_map_json(&mem, "pico8/map.json");
    assert(map_ok);

    /* Sprite sheet pixels must still read back correctly after the map
     * load, including rows that would fall inside the map's address
     * range under the old (buggy) packed-hardware-sized layout. */
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_SHEET + 0 * 128 + 16) == 3);
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_SHEET + 64 * 128 + 2) == 1);
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_SHEET + 127 * 128 + 0) == 3);

    /* Map cells (base region and the shared upper region for rows
     * 32-63) must be correct too. */
    assert(pico8_peek(&mem, PICO8_ADDR_MAP + 0 * 128 + 1) == 81);
    assert(pico8_peek(&mem, PICO8_ADDR_MAP + 0 * 128 + 9) == 213);
    assert(pico8_peek(&mem, PICO8_ADDR_MAP + 31 * 128 + 0) == 51);
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_MAP_SHARED + 0 * 128 + 0) == 51);
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_MAP_SHARED + 0 * 128 + 1) == 6);
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_MAP_SHARED + 31 * 128 + 0) == 41);
}

static void test_gfx_json_missing_file_returns_failure(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    int ok = pico8_load_gfx_json(&mem, "pico8/does_not_exist.json");
    assert(!ok);
}

static void test_map_json_missing_file_returns_failure(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    int ok = pico8_load_map_json(&mem, "pico8/does_not_exist.json");
    assert(!ok);
}

int main(void) {
    test_gfx_json_loads_known_pixel();
    test_gfx_json_loads_full_128x128_range();
    test_map_json_loads_base_region();
    test_map_json_loads_shared_region_for_rows_32_to_63();
    test_gfx_and_map_loaded_into_same_instance();
    test_gfx_json_missing_file_returns_failure();
    test_map_json_missing_file_returns_failure();

    printf("test_asset_loading: all tests passed\n");
    return 0;
}
