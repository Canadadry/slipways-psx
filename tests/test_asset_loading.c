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

static void test_map_json_loads_base_region(void) {
    /* map.json's cells rows 0-31 (the "base" JSON rows) land in the
     * P8_ADDR_SPRITE_MAP_SHARED region (0x1000), confirmed by decoding the
     * ROM of pico8/original.p8.png byte for byte. */
    p8_memory_t mem;
    p8_mem_init(&mem);

    int ok = p8_load_map_json(&mem, "pico8/map.json");
    assert(ok);

    assert(p8_peek(&mem, P8_ADDR_SPRITE_MAP_SHARED + 0 * 128 + 1) == 81);
    assert(p8_peek(&mem, P8_ADDR_SPRITE_MAP_SHARED + 0 * 128 + 9) == 213);
    assert(p8_peek(&mem, P8_ADDR_SPRITE_MAP_SHARED + 31 * 128 + 0) == 51);
}

static void test_map_json_loads_shared_region_for_rows_32_to_63(void) {
    /* map.json's cells rows 32-63 land in the P8_ADDR_MAP region (0x2000),
     * confirmed the same way. */
    p8_memory_t mem;
    p8_mem_init(&mem);

    int ok = p8_load_map_json(&mem, "pico8/map.json");
    assert(ok);

    assert(p8_peek(&mem, P8_ADDR_MAP + 0 * 128 + 0) == 51);
    assert(p8_peek(&mem, P8_ADDR_MAP + 0 * 128 + 1) == 6);
    assert(p8_peek(&mem, P8_ADDR_MAP + 31 * 128 + 0) == 41);
}

static void test_gfx_and_map_loaded_into_same_instance(void) {
    /* Both loads land in one `p8_memory_t`, as they do on a real PICO-8,
     * and must not clobber each other: an unpacked 128x128 sprite sheet
     * needs 0x4000 bytes, and a packed hardware-sized region spilled into
     * the map's own span. */
    p8_memory_t mem;
    p8_mem_init(&mem);

    int gfx_ok = p8_load_gfx_json(&mem, "pico8/gfx.json");
    assert(gfx_ok);
    int map_ok = p8_load_map_json(&mem, "pico8/map.json");
    assert(map_ok);

    /* Sprite rows that a packed layout would have put inside the map's
     * address range still read back correctly after the map load. */
    assert(p8_peek(&mem, P8_ADDR_SPRITE_SHEET + 0 * 128 + 16) == 3);
    assert(p8_peek(&mem, P8_ADDR_SPRITE_SHEET + 64 * 128 + 2) == 1);
    assert(p8_peek(&mem, P8_ADDR_SPRITE_SHEET + 127 * 128 + 0) == 3);

    /* And the map cells, both regions. */
    assert(p8_peek(&mem, P8_ADDR_SPRITE_MAP_SHARED + 0 * 128 + 1) == 81);
    assert(p8_peek(&mem, P8_ADDR_SPRITE_MAP_SHARED + 0 * 128 + 9) == 213);
    assert(p8_peek(&mem, P8_ADDR_SPRITE_MAP_SHARED + 31 * 128 + 0) == 51);
    assert(p8_peek(&mem, P8_ADDR_MAP + 0 * 128 + 0) == 51);
    assert(p8_peek(&mem, P8_ADDR_MAP + 0 * 128 + 1) == 6);
    assert(p8_peek(&mem, P8_ADDR_MAP + 31 * 128 + 0) == 41);
}

static void test_gfx_json_missing_file_returns_failure(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    int ok = p8_load_gfx_json(&mem, "pico8/does_not_exist.json");
    assert(!ok);
}

static void test_map_json_missing_file_returns_failure(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    int ok = p8_load_map_json(&mem, "pico8/does_not_exist.json");
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
