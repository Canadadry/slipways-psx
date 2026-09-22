/* The in-game top menu bar, asserted on `PICO8_ADDR_SCREEN` after a real
 * boot rather than by calling drawing primitives: the defects this pins
 * live in the gap between "a unit test calls it" and "a human reaches it",
 * and a `palt`/`print` unit test passes happily with all of them live.
 *
 * What each check is grounded in:
 *
 *   1. The icon art draws its outline in colour 0 over a colour-3
 *      background, and colour 3 is the cart's one transparent key --
 *      `cu()` bakes the 0x80 bit into every palette block for colour 3
 *      alone. So every colour-0 pixel must reach the screen, and the whole
 *      8x8 blit is compared against the sheet's nibbles with only colour 3
 *      translated to the button fill.
 *   2. `fm.jk` starts at 0 and reaches `io()` as a raw number, which Lua's
 *      `print` renders as its decimal string: exactly one lit glyph cell.
 *   3. The month is the literal "JAN" and `fm.yr` the raw number 3401:
 *      three month cells and four year cells, each a real glyph rather
 *      than the font's placeholder block. The glyph BITMAPS are
 *      deliberately not pinned -- this port's font table is hand-authored,
 *      not a reproduction of PICO-8's, so the shapes must stay free to
 *      change.
 *
 * The scripted input is inert on purpose: every defect shows on the first
 * in-game frame, and an idle script leaves `fm.yr`/`fm.lo` at their
 * start-of-game values, so "JAN 3401" is exactly what must show. */


#include <stdio.h>
#include <assert.h>

#include "../src/pico8_mem.h"
#include "../src/pico8_assets.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_help.h"
#include "../src/pico8_ui_button.h"

#define TOP_MENUBAR_EVENTS_PATH "tests/fixtures/top_menubar_events.json"

/* Read straight off the sprite sheet -- sprite 37 at sheet (40,16), sprite
 * 8 at (64,0) -- one hex nibble per source pixel, as `spr()` reads them. */
static const char *const MENUBAR_LIGHTBULB_SPRITE37[8] = {
    "33000333", "30ddd033", "0d773d03", "0d333d03", "30333033", "30511033", "30000033", "33333333",
};
static const char *const BOTTOM_MENU_SPRITE8[8] = {
    "00000003", "0ddd5503", "00000003", "0dd55503", "00000003", "0dd55503", "00000003", "33333333",
};

/* `js:kj()` fills the button box with 13 before blitting the icon, so that
 * is what shows through every transparent source pixel. */
#define MENUBAR_BUTTON_FILL 13

/* The cart's one transparent key: `cu()` sets 0x80 for colour 3 alone. */
#define SPRITE_TRANSPARENT_COLOR 3

#define MENUBAR_LIGHTBULB_X 1
#define MENUBAR_LIGHTBULB_Y 2
#define BOTTOM_MENU_ICON_X 1
#define BOTTOM_MENU_ICON_Y 119

/* `io()`'s draw coordinates for `pj.ou`'s technology, month and year
 * slots, and the font's 3x5 glyph inside a 4px cell. */
#define TECH_TEXT_X 13
#define TECH_TEXT_Y 2
#define TECH_TEXT_COLOR 9
#define DATE_TEXT_Y 2
#define DATE_TEXT_COLOR 13
#define MONTH_TEXT_X 85
#define YEAR_TEXT_X 99
#define GLYPH_W 3
#define GLYPH_H 5
#define GLYPH_CELL_W 4

static pico8_mem_t *g_screen_mem;

static int screen_px(int x, int y) {
    return pico8_peek(g_screen_mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x);
}

static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    return 10 + (c - 'a');
}

/* Every source pixel must land in its own colour, except colour 3, through
 * which the button fill shows. A missing outline appears here as
 * `background` where the sheet says 0. */
static void verify_icon_sprite_pixels(const char *name, const char *const sheet_rows[8], int x0, int y0,
                                      int background) {
    int dx, dy;
    int outline_pixels = 0;

    for (dy = 0; dy < 8; dy++) {
        for (dx = 0; dx < 8; dx++) {
            int src = hex_nibble(sheet_rows[dy][dx]);
            int expected = (src == SPRITE_TRANSPARENT_COLOR) ? background : src;
            int actual = screen_px(x0 + dx, y0 + dy);

            if (actual != expected) {
                fprintf(stderr, "[%s] pixel (%d,%d): expected %d (sheet nibble %d), got %d\n", name, x0 + dx, y0 + dy,
                       expected, src, actual);
            }
            assert(actual == expected);
            if (src == 0) {
                outline_pixels++;
            }
        }
    }

    /* Guards the loop above against degenerating: both sprites really do
     * carry colour-0 outline art. */
    assert(outline_pixels > 0);
    printf("[icon] %-22s at (%d,%d): all 64 blitted pixels match the sheet (%d outline pixels)\n", name, x0, y0,
           outline_pixels);
}

static int count_color_in_rect(int x0, int y0, int x1, int y1, int col) {
    int x, y;
    int n = 0;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++) {
            if (screen_px(x, y) == col) {
                n++;
            }
        }
    }
    return n;
}

/* One 3x5 glyph cell as a bitmask, bit 0 top-left, row-major, counting only
 * pixels in the text's own colour -- `io()`'s eight black offset passes are
 * deliberately excluded. */
static unsigned int glyph_cell_mask(int x0, int y0, int col) {
    unsigned int mask = 0;
    int gx, gy;
    for (gy = 0; gy < GLYPH_H; gy++) {
        for (gx = 0; gx < GLYPH_W; gx++) {
            if (screen_px(x0 + gx, y0 + gy) == col) {
                mask |= 1u << (gy * GLYPH_W + gx);
            }
        }
    }
    return mask;
}

#define GLYPH_FULL_MASK ((1u << (GLYPH_W * GLYPH_H)) - 1u)

/* A real glyph: something is lit, and it is not the solid 3x5 block the
 * font draws for an uncovered character. */
static void verify_glyph_cell_is_a_real_glyph(const char *name, int index, unsigned int mask) {
    if (mask == 0) {
        fprintf(stderr, "[%s] glyph cell %d is blank -- nothing drawn at all\n", name, index);
    } else if (mask == GLYPH_FULL_MASK) {
        fprintf(stderr, "[%s] glyph cell %d is a solid 3x5 block -- the font's placeholder, not a letter\n", name, index);
    }
    assert(mask != 0);
    assert(mask != GLYPH_FULL_MASK);
}

/* `fm.jk` is 0 at game start, so the chip shows exactly one lit glyph cell.
 * A blank chip fails the first assertion, a stray extra character the
 * second. */
static void verify_technology_counter(void) {
    int in_cell = count_color_in_rect(TECH_TEXT_X, TECH_TEXT_Y, TECH_TEXT_X + GLYPH_W - 1, TECH_TEXT_Y + GLYPH_H - 1,
                                      TECH_TEXT_COLOR);
    /* The whole text slot, between the lightbulb button and the growth
     * chip after it. */
    int in_slot = count_color_in_rect(10, 0, 27, 8, TECH_TEXT_COLOR);

    if (in_cell == 0) {
        fprintf(stderr, "[technology] nothing drawn in color %d at (%d,%d) -- the counter is blank\n", TECH_TEXT_COLOR,
               TECH_TEXT_X, TECH_TEXT_Y);
    }
    assert(in_cell > 0);
    assert(in_slot == in_cell);
    verify_glyph_cell_is_a_real_glyph("technology", 0, glyph_cell_mask(TECH_TEXT_X, TECH_TEXT_Y, TECH_TEXT_COLOR));
    printf("[technology] one real glyph drawn at (%d,%d) in color %d\n", TECH_TEXT_X, TECH_TEXT_Y, TECH_TEXT_COLOR);
}

/* "JAN" is three uppercase letters: three real glyphs, and not all three
 * the same. */
static void verify_month(void) {
    unsigned int mask[3];
    int i;

    for (i = 0; i < 3; i++) {
        mask[i] = glyph_cell_mask(MONTH_TEXT_X + i * GLYPH_CELL_W, DATE_TEXT_Y, DATE_TEXT_COLOR);
        verify_glyph_cell_is_a_real_glyph("month", i, mask[i]);
    }

    assert(!(mask[0] == mask[1] && mask[1] == mask[2]));
    printf("[month] three distinct real glyphs drawn from x=%d in color %d\n", MONTH_TEXT_X, DATE_TEXT_COLOR);
}

/* `fm.yr` is the raw number 3401: four lit glyph cells from x=99 and
 * nothing beyond them. A dropped number argument leaves the slot empty. */
static void verify_year(void) {
    int i;
    int in_cells = 0;
    int in_slot;

    for (i = 0; i < 4; i++) {
        int x = YEAR_TEXT_X + i * GLYPH_CELL_W;
        verify_glyph_cell_is_a_real_glyph("year", i, glyph_cell_mask(x, DATE_TEXT_Y, DATE_TEXT_COLOR));
        in_cells += count_color_in_rect(x, DATE_TEXT_Y, x + GLYPH_W - 1, DATE_TEXT_Y + GLYPH_H - 1, DATE_TEXT_COLOR);
    }

    /* The year slot, up to the right-hand menu button. */
    in_slot = count_color_in_rect(YEAR_TEXT_X - 1, 0, 115, 8, DATE_TEXT_COLOR);
    assert(in_slot == in_cells);
    printf("[year] four real glyphs drawn from x=%d in color %d\n", YEAR_TEXT_X, DATE_TEXT_COLOR);
}

int main(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    pico8_value_t difficulty_entry;
    pico8_instance_t *difficulty_button;
    int frame_count;
    int i;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    g_screen_mem = &mem;

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, TOP_MENUBAR_EVENTS_PATH));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    difficulty_button = pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref);
    pico8_eu(difficulty_button);

    frame_count = input.frame_count;
    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8__update60();
        pico8__draw();
        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    verify_icon_sprite_pixels("top menubar lightbulb", MENUBAR_LIGHTBULB_SPRITE37, MENUBAR_LIGHTBULB_X,
                              MENUBAR_LIGHTBULB_Y, MENUBAR_BUTTON_FILL);
    verify_icon_sprite_pixels("bottom menu button", BOTTOM_MENU_SPRITE8, BOTTOM_MENU_ICON_X, BOTTOM_MENU_ICON_Y,
                              MENUBAR_BUTTON_FILL);
    verify_technology_counter();
    verify_month();
    verify_year();

    pico8_input_free(&input);

    printf("pico8 top menubar: %d frame(s) run against %s; icon outlines, technology counter and JAN 3401 date all "
           "render\n",
           frame_count, TOP_MENUBAR_EVENTS_PATH);
    return 0;
}
