#include "p8/mem.h"

#include <assert.h>
#include <stdio.h>

/* The draw palette comes up in PICO-8's documented power-on state: the
 * identity map, with colour 0 and only colour 0 transparent. */
static void test_init_zeroes_everything_but_the_default_draw_palette(void) {
    p8_memory_t mem;
    for (int i = 0; i < P8_MEM_SIZE; i += 4096) {
        mem.data[i] = 0xAB;
    }
    for (int c = 0; c < 16; c++) {
        mem.draw_palette[c] = 0xAB;
    }
    mem.sprite_sheet[0][0] = 0xAB;
    mem.sprite_sheet[P8_SPRITE_SHEET_HEIGHT - 1][P8_SPRITE_SHEET_WIDTH - 1] = 0xAB;
    p8_mem_init(&mem);
    for (int i = 0; i < P8_MEM_SIZE; i++) {
        assert(mem.data[i] == 0);
    }
    for (int y = 0; y < P8_SPRITE_SHEET_HEIGHT; y++) {
        for (int x = 0; x < P8_SPRITE_SHEET_WIDTH; x++) {
            assert(mem.sprite_sheet[y][x] == 0);
        }
    }

    assert(mem.draw_palette[0] == (0 | P8_DRAW_PALETTE_TRANSPARENT_BIT));
    for (int c = 1; c < 16; c++) {
        assert(mem.draw_palette[c] == (unsigned char)c);
    }
}

static void test_peek_poke_roundtrip(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    p8_poke(&mem, 0x1234, 42);
    assert(p8_peek(&mem, 0x1234) == 42);
}

static void test_shared_region_boundary(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    assert(P8_ADDR_SPRITE_MAP_SHARED == 0x1000);
    p8_poke(&mem, P8_ADDR_SPRITE_MAP_SHARED, 3);
    assert(p8_peek(&mem, P8_ADDR_SPRITE_MAP_SHARED) == 3);

    p8_poke(&mem, P8_ADDR_MAP - 1, 4);
    assert(p8_peek(&mem, P8_ADDR_MAP - 1) == 4);
}

static void test_map_region_boundary(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    assert(P8_ADDR_MAP == 0x2000);
    p8_poke(&mem, P8_ADDR_MAP, 5);
    assert(p8_peek(&mem, P8_ADDR_MAP) == 5);

    p8_poke(&mem, P8_ADDR_MAP + 0x1000 - 1, 6);
    assert(p8_peek(&mem, P8_ADDR_MAP + 0x1000 - 1) == 6);
}

static void test_screen_region_boundary(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    assert(P8_ADDR_SCREEN == 0xA000);
    p8_poke(&mem, P8_ADDR_SCREEN, 7);
    assert(p8_peek(&mem, P8_ADDR_SCREEN) == 7);

    p8_poke(&mem, P8_MEM_SIZE - 1, 8);
    assert(p8_peek(&mem, P8_MEM_SIZE - 1) == 8);
}

static void test_out_of_range_peek_returns_zero(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    assert(p8_peek(&mem, -1) == 0);
    assert(p8_peek(&mem, P8_MEM_SIZE) == 0);
    assert(p8_peek(&mem, P8_MEM_SIZE + 100) == 0);
}

static void test_out_of_range_poke_is_noop(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    p8_poke(&mem, -1, 99);
    p8_poke(&mem, P8_MEM_SIZE, 99);
    p8_poke(&mem, P8_MEM_SIZE + 100, 99);

    for (int i = 0; i < P8_MEM_SIZE; i++) {
        assert(mem.data[i] == 0);
    }
}

static void test_memcpy_basic(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    for (int i = 0; i < 16; i++) {
        p8_poke(&mem, 0x5000 + i, (unsigned char)(i + 1));
    }
    p8_memcpy(&mem, 0x5f00, 0x5000, 16);
    for (int i = 0; i < 16; i++) {
        assert(p8_peek(&mem, 0x5f00 + i) == (unsigned char)(i + 1));
    }
}

static void test_memcpy_overlapping_forward(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    for (int i = 0; i < 10; i++) {
        p8_poke(&mem, 0x100 + i, (unsigned char)(i + 1));
    }

    p8_memcpy(&mem, 0x102, 0x100, 8);

    unsigned char expected[10] = {1, 2, 1, 2, 3, 4, 5, 6, 7, 8};
    for (int i = 0; i < 10; i++) {
        assert(p8_peek(&mem, 0x100 + i) == expected[i]);
    }
}

static void test_memcpy_overlapping_backward(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    for (int i = 0; i < 10; i++) {
        p8_poke(&mem, 0x200 + i, (unsigned char)(i + 1));
    }

    p8_memcpy(&mem, 0x200, 0x202, 8);

    unsigned char expected[10] = {3, 4, 5, 6, 7, 8, 9, 10, 9, 10};
    for (int i = 0; i < 10; i++) {
        assert(p8_peek(&mem, 0x200 + i) == expected[i]);
    }
}

static void test_memcpy_out_of_range_dest_is_noop(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    p8_poke(&mem, 0x300, 111);
    p8_memcpy(&mem, P8_MEM_SIZE - 4, 0x300, 16);

    for (int i = 0; i < P8_MEM_SIZE; i++) {
        if (i == 0x300) {
            assert(mem.data[i] == 111);
        } else {
            assert(mem.data[i] == 0);
        }
    }
}

static void test_memcpy_out_of_range_src_is_noop(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    p8_memcpy(&mem, 0x400, P8_MEM_SIZE - 4, 16);

    for (int i = 0; i < P8_MEM_SIZE; i++) {
        assert(mem.data[i] == 0);
    }
}

static void test_memcpy_negative_addr_is_noop(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    p8_memcpy(&mem, -1, 0x400, 16);
    p8_memcpy(&mem, 0x400, -1, 16);

    for (int i = 0; i < P8_MEM_SIZE; i++) {
        assert(mem.data[i] == 0);
    }
}

int main(void) {
    test_init_zeroes_everything_but_the_default_draw_palette();
    test_peek_poke_roundtrip();
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

    printf("test_mem: all tests passed\n");
    return 0;
}
