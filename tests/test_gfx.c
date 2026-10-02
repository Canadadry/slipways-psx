#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/mem.h"

#include <assert.h>
#include <stdio.h>

static void test_cls_clears_whole_screen_to_zero(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    for (int i = 0; i < P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT; i++) {
        p8_poke(&mem, P8_ADDR_SCREEN + i, 9);
    }

    p8_cls(&mem);

    for (int i = 0; i < P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT; i++) {
        assert(p8_peek(&mem, P8_ADDR_SCREEN + i) == 0);
    }
}

static void test_pset_plots_pixel_at_coordinate(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_pset(&gfx, &mem, 10, 20, 7);

    assert(p8_peek(&mem, P8_ADDR_SCREEN + 20 * P8_SCREEN_WIDTH + 10) == 7);
    /* Neighboring pixels are untouched. */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 20 * P8_SCREEN_WIDTH + 11) == 0);
}

static void test_pset_off_screen_is_clipped_silently(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_pset(&gfx, &mem, -1, 0, 7);
    p8_pset(&gfx, &mem, 0, -1, 7);
    p8_pset(&gfx, &mem, P8_SCREEN_WIDTH, 0, 7);
    p8_pset(&gfx, &mem, 0, P8_SCREEN_HEIGHT, 7);

    for (int i = 0; i < P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT; i++) {
        assert(p8_peek(&mem, P8_ADDR_SCREEN + i) == 0);
    }
}

static void test_camera_offset_shifts_subsequent_pset(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_camera(&gfx, 10, 5);
    p8_pset(&gfx, &mem, 15, 15, 7);

    /* camera(10,5) shifts the world left/up by (10,5): world (15,15)
     * lands at screen (5,10). */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 10 * P8_SCREEN_WIDTH + 5) == 7);
    /* It did NOT land at the unshifted coordinate. */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 15 * P8_SCREEN_WIDTH + 15) == 0);
}

static void test_camera_reset_restores_zero_offset(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_camera(&gfx, 10, 5);
    p8_camera_reset(&gfx);
    p8_pset(&gfx, &mem, 3, 4, 6);

    assert(p8_peek(&mem, P8_ADDR_SCREEN + 4 * P8_SCREEN_WIDTH + 3) == 6);
}

/* `palt()` stores into the draw palette in memory, so the PICO-8 default --
 * every colour opaque but colour 0 -- is `pico8_mem_init`'s power-on state,
 * not a gfx-side flag array. */
static void test_mem_init_default_transparency_is_color_0_only(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    assert(p8_gfx_is_transparent(&mem, 0));
    for (int c = 1; c < 16; c++) {
        assert(!p8_gfx_is_transparent(&mem, c));
    }
}

static void test_palt_marks_color_transparent_and_opaque(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    p8_palt(&mem, 3, 1);
    assert(p8_gfx_is_transparent(&mem, 3));

    p8_palt(&mem, 0, 0);
    assert(!p8_gfx_is_transparent(&mem, 0));

    p8_palt(&mem, 3, 0);
    assert(!p8_gfx_is_transparent(&mem, 3));
}

/* The consequence of that single store, which this cart depends on: `dv()`'s
 * memcpy into 0x5f00 replaces whatever `palt()` last said. The cart undoes
 * its `palt(0,true)` with a bare `dv()`, never with `palt(0,false)`. */
static void test_draw_palette_write_overrides_a_previous_palt(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    p8_palt(&mem, 0, 1);
    assert(p8_gfx_is_transparent(&mem, 0));

    for (int c = 0; c < 16; c++) {
        p8_poke(&mem, 0x5000 + c, (unsigned char)(c == 3 ? (c | P8_DRAW_PALETTE_TRANSPARENT_BIT) : c));
    }
    p8_memcpy(&mem, P8_ADDR_DRAW_PALETTE, 0x5000, 16);

    assert(!p8_gfx_is_transparent(&mem, 0));
    assert(p8_gfx_is_transparent(&mem, 3));
}

static void test_palt_out_of_range_col_is_noop_and_queries_as_opaque(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    p8_palt(&mem, -1, 1);
    p8_palt(&mem, 16, 1);

    assert(!p8_gfx_is_transparent(&mem, -1));
    assert(!p8_gfx_is_transparent(&mem, 16));
}

static void test_palt_affects_a_subsequent_transparent_color_draw(void) {
    /* `pset` never consults `palt`: as in real PICO-8, transparency only
     * affects the sprite primitives. What is exercised here is the "does
     * this pixel draw" query `spr()` makes per source pixel. */
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_palt(&mem, 3, 1);

    int col = 3;
    if (!p8_gfx_is_transparent(&mem, col)) {
        p8_pset(&gfx, &mem, 5, 5, col);
    }
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 5 * P8_SCREEN_WIDTH + 5) == 0);

    p8_palt(&mem, 3, 0);
    if (!p8_gfx_is_transparent(&mem, col)) {
        p8_pset(&gfx, &mem, 5, 5, col);
    }
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 5 * P8_SCREEN_WIDTH + 5) == 3);
}

static void test_line_draws_diagonal_pixels(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_line(&gfx, &mem, 2, 2, 5, 5, 8);

    for (int i = 2; i <= 5; i++) {
        assert(p8_peek(&mem, P8_ADDR_SCREEN + i * P8_SCREEN_WIDTH + i) == 8);
    }
    /* Off the diagonal is untouched. */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 2 * P8_SCREEN_WIDTH + 3) == 0);
}

static void test_line_zero_length_draws_single_pixel(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_line(&gfx, &mem, 7, 9, 7, 9, 11);

    assert(p8_peek(&mem, P8_ADDR_SCREEN + 9 * P8_SCREEN_WIDTH + 7) == 11);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 9 * P8_SCREEN_WIDTH + 8) == 0);
}

static void test_rect_draws_unfilled_outline(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_rect(&gfx, &mem, 2, 2, 5, 4, 8);

    /* Corners and edges are drawn. */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 2 * P8_SCREEN_WIDTH + 2) == 8);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 2 * P8_SCREEN_WIDTH + 5) == 8);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 4 * P8_SCREEN_WIDTH + 2) == 8);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 4 * P8_SCREEN_WIDTH + 5) == 8);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 2 * P8_SCREEN_WIDTH + 3) == 8);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 3 * P8_SCREEN_WIDTH + 2) == 8);
    /* Interior is untouched (unfilled). */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 3 * P8_SCREEN_WIDTH + 3) == 0);
}

static void test_rect_single_pixel_draws_one_point(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_rect(&gfx, &mem, 6, 6, 6, 6, 12);

    assert(p8_peek(&mem, P8_ADDR_SCREEN + 6 * P8_SCREEN_WIDTH + 6) == 12);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 6 * P8_SCREEN_WIDTH + 7) == 0);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 7 * P8_SCREEN_WIDTH + 6) == 0);
}

static void test_rectfill_fills_every_pixel_in_bounds(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_rectfill(&gfx, &mem, 2, 2, 4, 3, 9);

    for (int y = 2; y <= 3; y++) {
        for (int x = 2; x <= 4; x++) {
            assert(p8_peek(&mem, P8_ADDR_SCREEN + y * P8_SCREEN_WIDTH + x) == 9);
        }
    }
    /* Just outside the rectangle is untouched. */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 4 * P8_SCREEN_WIDTH + 2) == 0);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 2 * P8_SCREEN_WIDTH + 5) == 0);
}

static void test_circ_draws_unfilled_outline(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_circ(&gfx, &mem, 10, 10, 3, 8);

    /* Cardinal points of the outline. */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 10 * P8_SCREEN_WIDTH + 13) == 8);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 10 * P8_SCREEN_WIDTH + 7) == 8);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 13 * P8_SCREEN_WIDTH + 10) == 8);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 7 * P8_SCREEN_WIDTH + 10) == 8);
    /* Center is untouched (unfilled). */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 10 * P8_SCREEN_WIDTH + 10) == 0);
}

static void test_circ_zero_radius_draws_single_pixel(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_circ(&gfx, &mem, 5, 5, 0, 6);

    assert(p8_peek(&mem, P8_ADDR_SCREEN + 5 * P8_SCREEN_WIDTH + 5) == 6);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 5 * P8_SCREEN_WIDTH + 6) == 0);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 4 * P8_SCREEN_WIDTH + 5) == 0);
}

static void test_circfill_fills_the_disc(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_circfill(&gfx, &mem, 10, 10, 3, 8);

    /* Center and outline are both filled. */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 10 * P8_SCREEN_WIDTH + 10) == 8);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 10 * P8_SCREEN_WIDTH + 13) == 8);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 10 * P8_SCREEN_WIDTH + 7) == 8);
    /* Well outside the disc is untouched. */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 10 * P8_SCREEN_WIDTH + 14) == 0);
}

static void test_circfill_zero_radius_draws_single_pixel(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_circfill(&gfx, &mem, 5, 5, 0, 6);

    assert(p8_peek(&mem, P8_ADDR_SCREEN + 5 * P8_SCREEN_WIDTH + 5) == 6);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 5 * P8_SCREEN_WIDTH + 6) == 0);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 4 * P8_SCREEN_WIDTH + 5) == 0);
}

static void test_fillp_pattern_alters_rectfill_output(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    /* Rows alternate solid/skip in groups of 4: row%4==0..2 draws (bits
     * clear), row%4==3 skips (bits set) -- 0b0000000000001111 marks only
     * the pattern's last row as "skip". */
    p8_fillp(&gfx, 0x000F);
    p8_rectfill(&gfx, &mem, 0, 0, 3, 3, 8);

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 4; x++) {
            assert(p8_peek(&mem, P8_ADDR_SCREEN + y * P8_SCREEN_WIDTH + x) == 8);
        }
    }
    for (int x = 0; x < 4; x++) {
        assert(p8_peek(&mem, P8_ADDR_SCREEN + 3 * P8_SCREEN_WIDTH + x) == 0);
    }

    /* fillp() with no pattern resets to solid (always draw). */
    p8_fillp_reset(&gfx);
    p8_rectfill(&gfx, &mem, 0, 0, 3, 3, 8);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            assert(p8_peek(&mem, P8_ADDR_SCREEN + y * P8_SCREEN_WIDTH + x) == 8);
        }
    }
}

static void test_sget_reads_known_pixel_from_loaded_cart_data(void) {
    /* The three known-good values, in raw sheet pixel coordinates, not
     * sprite index plus offset: (16,0) is sprite 2's top-left, (2,64) is
     * sprite 128's (dx=2,dy=0), (0,127) is sprite 240's bottom-left. */
    p8_memory_t mem;
    p8_mem_init(&mem);
    int ok = p8_load_gfx_json(&mem, "pico8/gfx.json");
    assert(ok);

    assert(p8_sget(&mem, 16, 0) == 3);
    assert(p8_sget(&mem, 2, 64) == 1);
    assert(p8_sget(&mem, 0, 127) == 3);
}

static void test_sget_out_of_range_returns_zero(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);
    /* Poke a nonzero value adjacent to the sheet's bounds to make sure a
     * naive/unclamped read wouldn't accidentally also read 0 here. */
    p8_poke(&mem, P8_ADDR_SPRITE_SHEET, 9);

    assert(p8_sget(&mem, -1, 0) == 0);
    assert(p8_sget(&mem, 0, -1) == 0);
    assert(p8_sget(&mem, P8_SPRITE_SHEET_WIDTH, 0) == 0);
    assert(p8_sget(&mem, 0, P8_SPRITE_SHEET_HEIGHT) == 0);
}

static void test_spr_default_draws_1x1_sprite_at_world_position(void) {
    /* Sprite 5 lives at sheet pixel (40,0) (col 5*8, row 0*8, since 16
     * sprites/row). Poke two of its pixels directly and draw it with the
     * 3-arg convenience wrapper (defaults w=h=1). */
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_poke(&mem, P8_ADDR_SPRITE_SHEET + 0 * 128 + 40, 7);
    p8_poke(&mem, P8_ADDR_SPRITE_SHEET + 1 * 128 + 41, 8);

    p8_spr_default(&gfx, &mem, 5, 20, 30);

    assert(p8_peek(&mem, P8_ADDR_SCREEN + 30 * P8_SCREEN_WIDTH + 20) == 7);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 31 * P8_SCREEN_WIDTH + 21) == 8);
    /* Outside the 8x8 block is untouched. */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 38 * P8_SCREEN_WIDTH + 20) == 0);
}

static void test_spr_with_explicit_width_height_draws_multi_block_sprite(void) {
    /* Sprite 0 spans a 2x2-sprite (16x16 pixel) area starting at sheet
     * (0,0) when w=2,h=2: it reads pixels from sprite 0's own block plus
     * sprite 1 (to its right), sprite 16 (below), and sprite 17
     * (diagonal) -- real PICO-8's multi-cell sprite behavior. */
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_poke(&mem, P8_ADDR_SPRITE_SHEET + 0 * 128 + 0, 1);   /* sprite 0, corner */
    p8_poke(&mem, P8_ADDR_SPRITE_SHEET + 0 * 128 + 8, 2);   /* sprite 1's corner */
    p8_poke(&mem, P8_ADDR_SPRITE_SHEET + 8 * 128 + 0, 3);   /* sprite 16's corner */
    p8_poke(&mem, P8_ADDR_SPRITE_SHEET + 15 * 128 + 15, 4); /* sprite 17's far corner */

    p8_spr(&gfx, &mem, 0, 0, 0, 2, 2);

    assert(p8_peek(&mem, P8_ADDR_SCREEN + 0 * P8_SCREEN_WIDTH + 0) == 1);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 0 * P8_SCREEN_WIDTH + 8) == 2);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 8 * P8_SCREEN_WIDTH + 0) == 3);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 15 * P8_SCREEN_WIDTH + 15) == 4);
    /* Beyond the 16x16 area is untouched. */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 16 * P8_SCREEN_WIDTH + 0) == 0);
}

static void test_spr_skips_pixels_marked_transparent_by_palt(void) {
    /* Unlike pset/shapes, spr must consult palt: a source pixel whose
     * color is marked transparent is skipped (not drawn), leaving
     * whatever was already on screen underneath. */
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_poke(&mem, P8_ADDR_SPRITE_SHEET + 0 * 128 + 0, 3); /* sprite 0, corner pixel */
    p8_poke(&mem, P8_ADDR_SCREEN + 0 * P8_SCREEN_WIDTH + 0, 9); /* pre-existing content */

    p8_palt(&mem, 3, 1);
    p8_spr_default(&gfx, &mem, 0, 0, 0);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 0 * P8_SCREEN_WIDTH + 0) == 9);

    p8_palt(&mem, 3, 0);
    p8_spr_default(&gfx, &mem, 0, 0, 0);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 0 * P8_SCREEN_WIDTH + 0) == 3);
}

static void test_mset_writes_base_region_cell(void) {
    /* Cel rows 0-31 (the "base" JSON/map rows) land in the real cart's
     * PICO8_ADDR_SPRITE_MAP_SHARED region, confirmed by decoding
     * pico8/original.p8.png's cart ROM byte-for-byte. */
    p8_memory_t mem;
    p8_mem_init(&mem);

    p8_mset(&mem, 9, 0, 213);
    p8_mset(&mem, 0, 31, 51);

    assert(p8_peek(&mem, P8_ADDR_SPRITE_MAP_SHARED + 0 * 128 + 9) == 213);
    assert(p8_peek(&mem, P8_ADDR_SPRITE_MAP_SHARED + 31 * 128 + 0) == 51);
}

static void test_mset_writes_shared_upper_region_cell(void) {
    /* Cel rows 32-63 land in the real cart's PICO8_ADDR_MAP region. */
    p8_memory_t mem;
    p8_mem_init(&mem);

    p8_mset(&mem, 1, 32, 6);
    p8_mset(&mem, 0, 63, 41);

    assert(p8_peek(&mem, P8_ADDR_MAP + 0 * 128 + 1) == 6);
    assert(p8_peek(&mem, P8_ADDR_MAP + 31 * 128 + 0) == 41);
}

static void test_mset_out_of_range_is_noop(void) {
    p8_memory_t mem;
    p8_mem_init(&mem);

    p8_mset(&mem, -1, 0, 5);
    p8_mset(&mem, 0, -1, 5);
    p8_mset(&mem, P8_MAP_WIDTH, 0, 5);
    p8_mset(&mem, 0, P8_MAP_HEIGHT, 5);

    for (int i = 0; i < P8_MAP_WIDTH * 32; i++) {
        assert(p8_peek(&mem, P8_ADDR_MAP + i) == 0);
    }
    for (int i = 0; i < P8_MAP_WIDTH * 32; i++) {
        assert(p8_peek(&mem, P8_ADDR_SPRITE_MAP_SHARED + i) == 0);
    }
}

static void test_map_draws_cells_via_sprite_drawing_logic(void) {
    /* A 2x2-cell region, cell (1,0) pointing to sprite 5, the rest to
     * sprite 0. map() draws each cell as a 1x1 spr() call at
     * sx+c*8, sy+r*8 -- confirmed by cell (1,0) landing sprite 5's known
     * pixel at screen column 8 (the second cell), not column 0. */
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_poke(&mem, P8_ADDR_SPRITE_SHEET + 0 * 128 + 40, 7); /* sprite 5's corner */
    p8_mset(&mem, 1, 0, 5);

    p8_map(&gfx, &mem, 0, 0, 0, 0, 2, 2);

    assert(p8_peek(&mem, P8_ADDR_SCREEN + 0 * P8_SCREEN_WIDTH + 8) == 7);
    /* Cell (0,0) is sprite 0, whose corner pixel was never poked (stays
     * palette index 0, which is transparent by default and so is
     * skipped -- screen stays at its cleared 0 value either way). */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 0 * P8_SCREEN_WIDTH + 0) == 0);
}

static void test_map_treats_sprite_0_cells_as_empty(void) {
    /* Real PICO-8's map() "takes sprite 0 to mean empty and does not draw
     * it" (PICO-8 manual, MAP()); the opt-out POKE(0x5F36,0x8) is never
     * poked by this cart. So a cell holding 0 leaves its 8x8 block alone
     * even when sprite 0's own pixels are opaque -- which they are in this
     * cart, whose sprite 0 is palette data, not artwork. Direct spr(0,...)
     * is unaffected by that rule and still draws. */
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    /* Sprite 0's top-left pixel, opaque colour 7. */
    p8_poke(&mem, P8_ADDR_SPRITE_SHEET + 0, 7);
    p8_mset(&mem, 0, 0, 0);

    p8_map(&gfx, &mem, 0, 0, 0, 0, 1, 1);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 0) == 0);

    p8_spr(&gfx, &mem, 0, 0, 0, 1, 1);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 0) == 7);
}

static void test_map_and_mset_round_trip_against_real_cart_data(void) {
    /* `sget`'s read of a sprite-sheet pixel and the framebuffer `map()`
     * draws from that same sprite must agree, on real cart data. */
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    int ok = p8_load_gfx_json(&mem, "pico8/gfx.json");
    assert(ok);

    /* Sprite 2's top-left pixel is known-good color 3 (see
     * test_sget_reads_known_pixel_from_loaded_cart_data). */
    unsigned char expected = p8_sget(&mem, 16, 0);
    assert(expected == 3);

    p8_mset(&mem, 4, 40, 2); /* row 40 >= 32 lands in PICO8_ADDR_MAP */
    p8_map(&gfx, &mem, 4, 40, 0, 0, 1, 1);

    assert(p8_peek(&mem, P8_ADDR_MAP + (40 - 32) * 128 + 4) == 2);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 0 * P8_SCREEN_WIDTH + 0) == expected);
}

static void test_print_draws_known_glyph_pixels(void) {
    /* Spot-check the hand-authored 'a' glyph: a 3-wide x 5-tall bitmap
     * (see PICO8_FONT_GLYPH_WIDTH/HEIGHT) left-aligned at (x,y), with
     * column x+3 (the 4th column of the 4px advance cell) always blank. */
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_print(&gfx, &mem, "a", 10, 20, 7);

    /* 'a' row-by-row (# = lit, . = blank): .#. / #.# / ### / #.# / #.# */
    int expected[5][3] = {
        {0, 1, 0},
        {1, 0, 1},
        {1, 1, 1},
        {1, 0, 1},
        {1, 0, 1},
    };
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            unsigned char pixel = p8_peek(&mem, P8_ADDR_SCREEN + (20 + row) * P8_SCREEN_WIDTH + (10 + col));
            assert(pixel == (expected[row][col] ? 7 : 0));
        }
        /* 4th column of the advance cell is always blank spacing. */
        assert(p8_peek(&mem, P8_ADDR_SCREEN + (20 + row) * P8_SCREEN_WIDTH + 13) == 0);
    }
}

static void test_print_advances_cursor_by_cell_width_between_chars(void) {
    /* 'b' row-by-row: #.. / #.. / ##. / #.# / ##. */
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_print(&gfx, &mem, "ab", 0, 0, 6);

    /* Second glyph ('b') starts 4px to the right of the first, at x=4. */
    int expected_b[5][3] = {
        {1, 0, 0},
        {1, 0, 0},
        {1, 1, 0},
        {1, 0, 1},
        {1, 1, 0},
    };
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            unsigned char pixel = p8_peek(&mem, P8_ADDR_SCREEN + row * P8_SCREEN_WIDTH + (4 + col));
            assert(pixel == (expected_b[row][col] ? 6 : 0));
        }
    }
}

static void test_print_unmapped_character_draws_placeholder_block(void) {
    /* A character outside the documented covered set (see pico8_gfx.c)
     * draws a solid 3x5 block rather than staying blank, so a missing
     * mapping is visually obvious. */
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_print(&gfx, &mem, "@", 0, 0, 8);

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            assert(p8_peek(&mem, P8_ADDR_SCREEN + row * P8_SCREEN_WIDTH + col) == 8);
        }
    }
}

static void test_print_space_is_blank(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_print(&gfx, &mem, " ", 0, 0, 5);

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 4; col++) {
            assert(p8_peek(&mem, P8_ADDR_SCREEN + row * P8_SCREEN_WIDTH + col) == 0);
        }
    }
}

static void test_print_respects_camera_offset(void) {
    /* Consistent with every other pico8_gfx primitive: print goes through
     * pico8_pset, so a camera offset shifts world coordinates the same
     * way it would for pset/line/etc. */
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    p8_camera(&gfx, 10, 20);

    p8_print(&gfx, &mem, "a", 10, 20, 7);

    /* World (10,20) minus camera (10,20) lands at screen (0,0); the 'a'
     * glyph's top-left pixel (row0,col1) should be lit there. */
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 0 * P8_SCREEN_WIDTH + 1) == 7);
    assert(p8_peek(&mem, P8_ADDR_SCREEN + 0 * P8_SCREEN_WIDTH + 0) == 0);
}

static void test_print_digit_and_punctuation_glyphs(void) {
    /* Spot-check a digit and a punctuation glyph from the covered set
     * documented in pico8_gfx.c (this cart's UI text uses digits for
     * scores/years/percentages and ':' for labels like "population:"). */
    p8_memory_t mem;
    p8_graphics_t gfx;
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);

    p8_print(&gfx, &mem, "1:", 0, 0, 9);

    /* '1' row-by-row: .#. / ##. / .#. / .#. / ### */
    int expected_1[5][3] = {
        {0, 1, 0},
        {1, 1, 0},
        {0, 1, 0},
        {0, 1, 0},
        {1, 1, 1},
    };
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            unsigned char pixel = p8_peek(&mem, P8_ADDR_SCREEN + row * P8_SCREEN_WIDTH + col);
            assert(pixel == (expected_1[row][col] ? 9 : 0));
        }
    }

    /* ':' (second cell, starting at x=4) row-by-row: ... / .#. / ... / .#. / ... */
    int expected_colon[5][3] = {
        {0, 0, 0},
        {0, 1, 0},
        {0, 0, 0},
        {0, 1, 0},
        {0, 0, 0},
    };
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            unsigned char pixel = p8_peek(&mem, P8_ADDR_SCREEN + row * P8_SCREEN_WIDTH + (4 + col));
            assert(pixel == (expected_colon[row][col] ? 9 : 0));
        }
    }
}

int main(void) {
    test_cls_clears_whole_screen_to_zero();
    test_pset_plots_pixel_at_coordinate();
    test_pset_off_screen_is_clipped_silently();
    test_camera_offset_shifts_subsequent_pset();
    test_camera_reset_restores_zero_offset();
    test_mem_init_default_transparency_is_color_0_only();
    test_palt_marks_color_transparent_and_opaque();
    test_draw_palette_write_overrides_a_previous_palt();
    test_palt_out_of_range_col_is_noop_and_queries_as_opaque();
    test_palt_affects_a_subsequent_transparent_color_draw();
    test_line_draws_diagonal_pixels();
    test_line_zero_length_draws_single_pixel();
    test_rect_draws_unfilled_outline();
    test_rect_single_pixel_draws_one_point();
    test_rectfill_fills_every_pixel_in_bounds();
    test_circ_draws_unfilled_outline();
    test_circ_zero_radius_draws_single_pixel();
    test_circfill_fills_the_disc();
    test_circfill_zero_radius_draws_single_pixel();
    test_fillp_pattern_alters_rectfill_output();
    test_sget_reads_known_pixel_from_loaded_cart_data();
    test_sget_out_of_range_returns_zero();
    test_spr_default_draws_1x1_sprite_at_world_position();
    test_spr_with_explicit_width_height_draws_multi_block_sprite();
    test_spr_skips_pixels_marked_transparent_by_palt();
    test_mset_writes_base_region_cell();
    test_mset_writes_shared_upper_region_cell();
    test_mset_out_of_range_is_noop();
    test_map_draws_cells_via_sprite_drawing_logic();
    test_map_treats_sprite_0_cells_as_empty();
    test_map_and_mset_round_trip_against_real_cart_data();
    test_print_draws_known_glyph_pixels();
    test_print_advances_cursor_by_cell_width_between_chars();
    test_print_unmapped_character_draws_placeholder_block();
    test_print_space_is_blank();
    test_print_respects_camera_offset();
    test_print_digit_and_punctuation_glyphs();

    printf("test_gfx: all tests passed\n");
    return 0;
}
