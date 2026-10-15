#include "game/hud.h"
#include "p8/assets.h"
#include "p8/draw_backend.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

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

static void test_bar_reaches_the_right_edge(void) {
    p8_vram_t mem;
    p8_graphics_t gfx;
    p8_vram_init(&mem);
    p8_gfx_init(&gfx);

    hud_draw_bar(&gfx, &mem, p8_fixed_from_int(5), p8_fixed_from_int(6), 7);

    for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
        assert(mem.screen[5][x] == 7 && mem.screen[6][x] == 7);
    }
}

static int screen_equal(const p8_vram_t *a, const p8_vram_t *b) {
    return memcmp(a->screen, b->screen, sizeof a->screen) == 0;
}

static int screen_has_ink(const p8_vram_t *mem) {
    for (int y = 0; y < P8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
            if (mem->screen[y][x] != 0) {
                return 1;
            }
        }
    }
    return 0;
}

static void test_alignment_zero_prints_as_the_default(void) {
    p8_vram_t aligned, plain;
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_vram_init(&aligned);
    p8_vram_init(&plain);

    hud_print_outlined(&gfx, &aligned, "ore", p8_fixed_from_int(40), p8_fixed_from_int(30), 7, p8_fixed_from_int(0));
    hud_print_outlined_default(&gfx, &plain, "ore", p8_fixed_from_int(40), p8_fixed_from_int(30), 7);

    assert(screen_has_ink(&plain));
    assert(screen_equal(&aligned, &plain));
}

static void test_half_alignment_shifts_by_two_pixels_per_letter(void) {
    p8_vram_t centered, shifted;
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_vram_init(&centered);
    p8_vram_init(&shifted);

    hud_print_outlined(&gfx, &centered, "ore", p8_fixed_from_int(40), p8_fixed_from_int(30), 7, P8_FIXED_LIT(0x00008000, 0.5));
    hud_print_outlined_default(&gfx, &shifted, "ore", p8_fixed_from_int(34), p8_fixed_from_int(30), 7);

    assert(screen_equal(&centered, &shifted));
}

typedef struct {
    int prints;
    int outlined_prints;
    const char *str;
    int x, y, col;
} outlined_record_t;

static outlined_record_t outlined_record;

static void count_print(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col) {
    (void)gfx, (void)mem, (void)str, (void)x, (void)y, (void)col;
    outlined_record.prints++;
}

static void record_print_outlined(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col) {
    (void)gfx, (void)mem;
    outlined_record.outlined_prints++;
    outlined_record.str = str;
    outlined_record.x = x;
    outlined_record.y = y;
    outlined_record.col = col;
}

static void test_outlined_text_is_one_outlined_print_at_the_aligned_position(void) {
    static p8_draw_backend_t recording;
    p8_vram_t mem;
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_vram_init(&mem);
    recording = *p8_gfx_software_backend();
    recording.print = count_print;
    recording.print_outlined = record_print_outlined;
    memset(&outlined_record, 0, sizeof outlined_record);
    const p8_draw_backend_t *previous = p8_draw_backend_set(&recording);
    static const char text[] = "ore";

    hud_print_outlined(&gfx, &mem, text, p8_fixed_from_int(40), p8_fixed_from_int(30), 0x1b, P8_FIXED_LIT(0x00008000, 0.5));

    p8_draw_backend_set(previous);
    assert(outlined_record.prints == 0);
    assert(outlined_record.outlined_prints == 1);
    assert(outlined_record.str == text);
    assert(outlined_record.x == 34 && outlined_record.y == 30);
    assert(outlined_record.col == 0x0b);
}

int main(void) {
    test_each_block_matches_the_sprite_sheet_pixels();
    test_select_needs_no_sprite_sheet();
    test_fractional_index_selects_its_floor();
    test_bar_reaches_the_right_edge();
    test_alignment_zero_prints_as_the_default();
    test_half_alignment_shifts_by_two_pixels_per_letter();
    test_outlined_text_is_one_outlined_print_at_the_aligned_position();

    printf("test_hud: OK\n");
    return 0;
}
