/* Phase 5 bug fix -- "in screen game top menu is weird": three separate
 * reported rendering defects on the in-game galaxy screen's top menu bar,
 * all three reachable only through a really booted cart, so this test
 * drives the exact four entry points a human (and src/main_raylib.c:64-74)
 * hits -- pico8_cart_load() / pico8__init() / pico8__update60() /
 * pico8__draw() -- and then asserts on PICO8_ADDR_SCREEN, the very memory
 * src/pico8_raylib_backend.c:139-146 blits to the window one-for-one.
 * Nothing here calls a drawing primitive directly: the last several bugs
 * of this port hid precisely in the gap between "a unit test calls it" and
 * "a human reaches it" -- a pico8_palt()/pico8_print() unit test passes
 * happily with all three of these defects live.
 *
 * The three reported symptoms, and what each assertion below pins:
 *
 *   1. "all icon in top menubar, bottom menu and contextual menu lack a
 *      back outline". The cart's icon art draws its outline in color 0 and
 *      its own transparent background in color 3 (pico8/gfx.json: sprite
 *      37 = the top-left lightbulb, sprite 8 = the bottom-left menu
 *      button). Color 3 is this cart's one real transparent key -- cu()
 *      bakes the 0x80 bit into every palette block for color 3 alone
 *      (pico8/lua.lua:134-140, src/pico8_hud.c) -- so every color-0 pixel
 *      of those sprites MUST reach the screen. verify_icon_sprite_pixels()
 *      therefore compares the whole 8x8 blit against the sprite sheet's
 *      own nibbles, translating only color 3 to the button's own fill
 *      color. This is what fails when a stale, independently-stored
 *      palt(0,true) keeps color 0 invisible.
 *
 *   2. "in top menu technology screen should print 0 (starting point of
 *      tech)". fm.jk starts at 0 (pico8/lua.lua:1200) and pj:hb() copies
 *      it into the drawable slot as a raw number (pico8/lua.lua:847);
 *      io() draws it at x=13,y=2 in color 9. A number argument to Lua's
 *      own print() renders as its decimal string, so exactly one 3x5
 *      glyph cell, at (13,2), must be lit in color 9 -- not a blank chip.
 *
 *   3. "year just be Jan 3401 but we can only see 3 rectangle". The month
 *      ef[fm.lo] is the uppercase literal "JAN" (pico8/lua.lua:797) drawn
 *      at x=85,y=2 in color 13, and fm.yr is the raw number 3401
 *      (pico8/lua.lua:1202) drawn at x=99,y=2. So: three month cells that
 *      each render a real glyph -- lit, but NOT the solid 3x5 block the
 *      font table's own PICO8_FONT_ROWS_PLACEHOLDER draws for an uncovered
 *      character (src/pico8_gfx.c) -- and four year cells that are lit at
 *      all. These assertions deliberately do NOT pin the glyph BITMAPS:
 *      that font table is documented (src/pico8_gfx.c) as a hand-authored
 *      invention rather than a reproduction of PICO-8's real glyphs, so
 *      "a readable glyph, not the placeholder block" is the real
 *      behavioral claim and the exact shapes must stay free to change.
 *
 * The scripted input is deliberately inert (tests/fixtures/
 * top_menubar_events.json: four frames, no clicks, cursor parked outside
 * ui.hw) -- all three defects are visible on the very first in-game frame
 * with no input at all, and an idle script keeps fm.yr/fm.lo at their
 * start-of-game values, so "JAN 3401" is exactly what the bar must show.
 * The difficulty-button pico8_eu() kick is the same one
 * tests/test_pico8_playability.c and src/main.c already use to leave the
 * title screen. */


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

/* pico8/gfx.json, read straight off the sprite sheet (sprite 37 lives at
 * sheet (40,16), sprite 8 at sheet (64,0)) -- one hex nibble per source
 * pixel, exactly as spr() reads them. */
static const char *const MENUBAR_LIGHTBULB_SPRITE37[8] = {
    "33000333", "30ddd033", "0d773d03", "0d333d03", "30333033", "30511033", "30000033", "33333333",
};
static const char *const BOTTOM_MENU_SPRITE8[8] = {
    "00000003", "0ddd5503", "00000003", "0dd55503", "00000003", "0dd55503", "00000003", "33333333",
};

/* js:kj()'s own button chrome fills its box with color 13 before blitting
 * the icon (pico8/lua.lua:273, src/pico8_layout.c), so that is what shows
 * through every transparent (color 3) source pixel. */
#define MENUBAR_BUTTON_FILL 13

/* The cart's one real transparent key: cu() sets 0x80 for color 3 alone
 * (pico8/lua.lua:134-140). */
#define SPRITE_TRANSPARENT_COLOR 3

#define MENUBAR_LIGHTBULB_X 1
#define MENUBAR_LIGHTBULB_Y 2
#define BOTTOM_MENU_ICON_X 1
#define BOTTOM_MENU_ICON_Y 119

/* io()'s own draw coordinates for pj.ou's technology / month / year slots,
 * plus the font's 3x5 glyph in a 4px cell (src/pico8_gfx.h). */
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

/* Symptom 1. Every source pixel of the blit must land on screen in its own
 * color, EXCEPT color 3 (the cart's transparent key), through which the
 * button's own fill shows. A missing black outline shows up here as
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

    /* Guards the assertion above against silently degenerating: both of
     * these sprites really do carry color-0 outline art. */
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

/* The 15 pixels of one 3x5 glyph cell as a bitmask (bit 0 = top-left,
 * row-major), counting only pixels lit in the text's own color -- io()'s
 * eight black offset passes are deliberately not part of it. */
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

/* A real glyph: something is lit, and it is not the solid 3x5
 * PICO8_FONT_ROWS_PLACEHOLDER block an uncovered character draws. */
static void verify_glyph_cell_is_a_real_glyph(const char *name, int index, unsigned int mask) {
    if (mask == 0) {
        fprintf(stderr, "[%s] glyph cell %d is blank -- nothing drawn at all\n", name, index);
    } else if (mask == GLYPH_FULL_MASK) {
        fprintf(stderr, "[%s] glyph cell %d is a solid 3x5 block -- the font's placeholder, not a letter\n", name, index);
    }
    assert(mask != 0);
    assert(mask != GLYPH_FULL_MASK);
}

/* Symptom 2: fm.jk is 0 at game start, and a number argument to print()
 * renders as its decimal string -- so the chip shows exactly one lit glyph
 * cell, at io()'s own (13,2), in color 9. A blank chip (the reported bug)
 * fails the first assertion; a stray extra character fails the second. */
static void verify_technology_counter(void) {
    int in_cell = count_color_in_rect(TECH_TEXT_X, TECH_TEXT_Y, TECH_TEXT_X + GLYPH_W - 1, TECH_TEXT_Y + GLYPH_H - 1,
                                      TECH_TEXT_COLOR);
    /* The whole technology chip's own text slot, between the lightbulb
     * button and the growth chip that follows it. */
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

/* Symptom 3a: "JAN" is three uppercase letters, so three real, not-all-
 * identical glyphs -- not three solid placeholder blocks. */
static void verify_month(void) {
    unsigned int mask[3];
    int i;

    for (i = 0; i < 3; i++) {
        mask[i] = glyph_cell_mask(MONTH_TEXT_X + i * GLYPH_CELL_W, DATE_TEXT_Y, DATE_TEXT_COLOR);
        verify_glyph_cell_is_a_real_glyph("month", i, mask[i]);
    }

    /* J, A and N are three different letters. */
    assert(!(mask[0] == mask[1] && mask[1] == mask[2]));
    printf("[month] three distinct real glyphs drawn from x=%d in color %d\n", MONTH_TEXT_X, DATE_TEXT_COLOR);
}

/* Symptom 3b: fm.yr is the raw number 3401, so four lit glyph cells from
 * x=99 and nothing beyond them. A dropped number argument (the reported
 * bug) leaves the whole slot empty. */
static void verify_year(void) {
    int i;
    int in_cells = 0;
    int in_slot;

    for (i = 0; i < 4; i++) {
        int x = YEAR_TEXT_X + i * GLYPH_CELL_W;
        verify_glyph_cell_is_a_real_glyph("year", i, glyph_cell_mask(x, DATE_TEXT_Y, DATE_TEXT_COLOR));
        in_cells += count_color_in_rect(x, DATE_TEXT_Y, x + GLYPH_W - 1, DATE_TEXT_Y + GLYPH_H - 1, DATE_TEXT_COLOR);
    }

    /* The date chip's own year slot, up to the right-hand menu button. */
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
