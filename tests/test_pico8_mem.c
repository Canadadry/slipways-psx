#include "../src/pico8_mem.h"

#include <assert.h>
#include <stdio.h>

/* `pico8_mem_init` leaves exactly one region non-zero, the 16-byte draw
 * palette. Every "nothing was written" whole-memory scan here skips it. */
static int is_default_draw_palette_addr(int addr) {
    return addr >= PICO8_ADDR_DRAW_PALETTE && addr < PICO8_ADDR_DRAW_PALETTE + 16;
}

/* That palette comes up in PICO-8's documented power-on state: the identity
 * map, with colour 0 and only colour 0 transparent. */
static void test_init_zeroes_everything_but_the_default_draw_palette(void) {
    pico8_mem_t mem;
    for (int i = 0; i < PICO8_MEM_SIZE; i += 4096) {
        mem.data[i] = 0xAB;
    }
    pico8_mem_init(&mem);
    assert(pico8_peek(&mem, 0) == 0);
    assert(pico8_peek(&mem, PICO8_MEM_SIZE - 1) == 0);

    assert(pico8_peek(&mem, PICO8_ADDR_DRAW_PALETTE) == (0 | PICO8_DRAW_PALETTE_TRANSPARENT_BIT));
    for (int c = 1; c < 16; c++) {
        assert(pico8_peek(&mem, PICO8_ADDR_DRAW_PALETTE + c) == (unsigned char)c);
    }
}

static void test_peek_poke_roundtrip(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    pico8_poke(&mem, 0x1234, 42);
    assert(pico8_peek(&mem, 0x1234) == 42);
}

static void test_sprite_sheet_region_boundary(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    assert(PICO8_ADDR_SPRITE_SHEET == 0x6000);
    pico8_poke(&mem, PICO8_ADDR_SPRITE_SHEET, 1);
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_SHEET) == 1);

    pico8_poke(&mem, PICO8_ADDR_SPRITE_SHEET + 0x4000 - 1, 2);
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_SHEET + 0x4000 - 1) == 2);
}

static void test_shared_region_boundary(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    assert(PICO8_ADDR_SPRITE_MAP_SHARED == 0x1000);
    pico8_poke(&mem, PICO8_ADDR_SPRITE_MAP_SHARED, 3);
    assert(pico8_peek(&mem, PICO8_ADDR_SPRITE_MAP_SHARED) == 3);

    pico8_poke(&mem, PICO8_ADDR_MAP - 1, 4);
    assert(pico8_peek(&mem, PICO8_ADDR_MAP - 1) == 4);
}

static void test_map_region_boundary(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    assert(PICO8_ADDR_MAP == 0x2000);
    pico8_poke(&mem, PICO8_ADDR_MAP, 5);
    assert(pico8_peek(&mem, PICO8_ADDR_MAP) == 5);

    pico8_poke(&mem, PICO8_ADDR_MAP + 0x1000 - 1, 6);
    assert(pico8_peek(&mem, PICO8_ADDR_MAP + 0x1000 - 1) == 6);
}

static void test_screen_region_boundary(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    assert(PICO8_ADDR_SCREEN == 0xA000);
    pico8_poke(&mem, PICO8_ADDR_SCREEN, 7);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN) == 7);

    pico8_poke(&mem, PICO8_MEM_SIZE - 1, 8);
    assert(pico8_peek(&mem, PICO8_MEM_SIZE - 1) == 8);
}

static void test_out_of_range_peek_returns_zero(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    assert(pico8_peek(&mem, -1) == 0);
    assert(pico8_peek(&mem, PICO8_MEM_SIZE) == 0);
    assert(pico8_peek(&mem, PICO8_MEM_SIZE + 100) == 0);
}

static void test_out_of_range_poke_is_noop(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    pico8_poke(&mem, -1, 99);
    pico8_poke(&mem, PICO8_MEM_SIZE, 99);
    pico8_poke(&mem, PICO8_MEM_SIZE + 100, 99);

    for (int i = 0; i < PICO8_MEM_SIZE; i++) {
        if (!is_default_draw_palette_addr(i)) {
            assert(mem.data[i] == 0);
        }
    }
}

static void test_memcpy_basic(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    for (int i = 0; i < 16; i++) {
        pico8_poke(&mem, 0x5000 + i, (unsigned char)(i + 1));
    }
    pico8_memcpy(&mem, 0x5f00, 0x5000, 16);
    for (int i = 0; i < 16; i++) {
        assert(pico8_peek(&mem, 0x5f00 + i) == (unsigned char)(i + 1));
    }
}

static void test_memcpy_overlapping_forward(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    for (int i = 0; i < 10; i++) {
        pico8_poke(&mem, 0x100 + i, (unsigned char)(i + 1));
    }

    pico8_memcpy(&mem, 0x102, 0x100, 8);

    unsigned char expected[10] = {1, 2, 1, 2, 3, 4, 5, 6, 7, 8};
    for (int i = 0; i < 10; i++) {
        assert(pico8_peek(&mem, 0x100 + i) == expected[i]);
    }
}

static void test_memcpy_overlapping_backward(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    for (int i = 0; i < 10; i++) {
        pico8_poke(&mem, 0x200 + i, (unsigned char)(i + 1));
    }

    pico8_memcpy(&mem, 0x200, 0x202, 8);

    unsigned char expected[10] = {3, 4, 5, 6, 7, 8, 9, 10, 9, 10};
    for (int i = 0; i < 10; i++) {
        assert(pico8_peek(&mem, 0x200 + i) == expected[i]);
    }
}

static void test_memcpy_out_of_range_dest_is_noop(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    pico8_poke(&mem, 0x300, 111);
    pico8_memcpy(&mem, PICO8_MEM_SIZE - 4, 0x300, 16);

    for (int i = 0; i < PICO8_MEM_SIZE; i++) {
        if (i == 0x300) {
            assert(mem.data[i] == 111);
        } else if (!is_default_draw_palette_addr(i)) {
            assert(mem.data[i] == 0);
        }
    }
}

static void test_memcpy_out_of_range_src_is_noop(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    pico8_memcpy(&mem, 0x400, PICO8_MEM_SIZE - 4, 16);

    for (int i = 0; i < PICO8_MEM_SIZE; i++) {
        if (!is_default_draw_palette_addr(i)) {
            assert(mem.data[i] == 0);
        }
    }
}

static void test_memcpy_negative_addr_is_noop(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    pico8_memcpy(&mem, -1, 0x400, 16);
    pico8_memcpy(&mem, 0x400, -1, 16);

    for (int i = 0; i < PICO8_MEM_SIZE; i++) {
        if (!is_default_draw_palette_addr(i)) {
            assert(mem.data[i] == 0);
        }
    }
}

int main(void) {
    test_init_zeroes_everything_but_the_default_draw_palette();
    test_peek_poke_roundtrip();
    test_sprite_sheet_region_boundary();
    test_shared_region_boundary();
    test_map_region_boundary();
    test_screen_region_boundary();
    test_out_of_range_peek_returns_zero();
    test_out_of_range_poke_is_noop();
    test_memcpy_basic();
    test_memcpy_overlapping_forward();
    test_memcpy_overlapping_backward();
    test_memcpy_out_of_range_dest_is_noop();
    test_memcpy_out_of_range_src_is_noop();
    test_memcpy_negative_addr_is_noop();

    printf("test_pico8_mem: all tests passed\n");
    return 0;
}
