/* `lq({nil})` must draw nothing at all: in `lq:mn` a nil item is neither a
 * string nor truthy, so both branches are skipped and `fn or function()end`
 * hands `lq:ol` a closure that draws nothing -- the item still takes its
 * default w=6 of layout width, invisible rather than absent. The trap this
 * pins is kept at the guard itself, in src/pico8_layout.c.
 *
 * WHY THIS TEST EXISTS beside the e2e tests/e2e/colony_panel.json, which
 * replays the reported bug end to end: that one pins only the msgbox half
 * of the fix (`pz()`'s own `if p.ep then`). Measured when the bug was fixed
 * -- reverting src/pico8_layout.c's REF(NULL) clause on its own left the
 * whole suite green, because once `pz()` stops building the bogus row no
 * nil ever reaches `lq:mn` again: the single nil in the entire replay
 * corpus (the 9729-frame events.json plus the 278-frame built_planet_panel
 * fixture) was manufactured by the msgbox bug itself. Without this test
 * the `lq:mn` guard is correct today and free to rot the moment another
 * caller hands `lq()` a nil item.
 *
 * `pico8_lq_mn` is static to src/pico8_layout.c, so this test never names
 * it: it builds a real `lq` holding ONE positional item -- the shape of
 * every `lq({...})` call site -- and renders it with pico8_lq_ol_default(),
 * which is how the cart reaches `lq:mn` too. */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_hud.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

/* Where the row is rendered. Any position clear of the screen edges does;
 * these put the whole 8x8 sprite the faulty build drew well inside the
 * framebuffer. */
#define DRAW_X 20
#define DRAW_Y 30

/* Sprite #0's bottom pixel row. Row 7 and not row 0: row 0 is all zeros, so
 * it would match blank background at every position on screen and make the
 * search vacuous. Row 7 is six distinct colours and occurs exactly once in
 * the whole 128x128 sheet. */
#define SPRITE0_ROW_PIXELS 8
#define SPRITE0_BOTTOM_ROW_Y 7

static pico8_mem_t mem;
static pico8_gfx_t gfx;

static int screen_px(int x, int y) {
    return pico8_peek(&mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x);
}

/* How many places sprite #0's bottom row appears in the framebuffer, with
 * the first one's top-left x/y written through the out-params. The wanted
 * run is read back out of the loaded sprite sheet, never hard-coded. */
static int sprite0_bottom_row_hits(int *out_x, int *out_y) {
    int wanted[SPRITE0_ROW_PIXELS];
    int i, x, y, hits = 0;

    for (i = 0; i < SPRITE0_ROW_PIXELS; i++) {
        wanted[i] = pico8_sget(&mem, i, SPRITE0_BOTTOM_ROW_Y);
    }

    for (y = 0; y < PICO8_SCREEN_HEIGHT; y++) {
        for (x = 0; x + SPRITE0_ROW_PIXELS <= PICO8_SCREEN_WIDTH; x++) {
            int match = 1;
            for (i = 0; i < SPRITE0_ROW_PIXELS; i++) {
                if (screen_px(x + i, y) != wanted[i]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                if (hits == 0) {
                    *out_x = x;
                    *out_y = y;
                }
                hits++;
            }
        }
    }
    return hits;
}

/* How many framebuffer pixels `cls()` did NOT leave behind -- i.e. how much
 * this render actually drew. */
static int pixels_drawn(void) {
    int x, y, n = 0;
    for (y = 0; y < PICO8_SCREEN_HEIGHT; y++) {
        for (x = 0; x < PICO8_SCREEN_WIDTH; x++) {
            if (screen_px(x, y) != 0) {
                n++;
            }
        }
    }
    return n;
}

/* `lq({item}):ol(v(DRAW_X,DRAW_Y))` -- one real row holding one
 * positional item, rendered onto a cleared screen. */
static void render_one_item(pico8_value_t item) {
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    pico8_lq_t *line;

    pico8_ob_table_set_index(overrides, 1, item);
    line = pico8_lq_new(overrides);
    pico8_ob_table_free(overrides);
    assert(line != NULL);

    pico8_cls(&mem);
    pico8_lq_ol_default(&gfx, &mem, line, pico8_vec_v(pico8_fixed_from_int(DRAW_X), pico8_fixed_from_int(DRAW_Y)));
}

/* THE SUBJECT: `lq({nil})` must draw nothing at all. */
static void test_nil_item_draws_nothing(void) {
    int hit_x = -1, hit_y = -1;
    int hits, drawn;

    render_one_item(pico8_value_ref(NULL));

    hits = sprite0_bottom_row_hits(&hit_x, &hit_y);
    drawn = pixels_drawn();
    printf("[nil]     lq({nil}):ol() drew %d pixel(s); sprite #0's bottom row appears %d time(s)\n", drawn, hits);
    if (hits > 0) {
        printf("[nil]     first occurrence at (%d,%d)\n", hit_x, hit_y);
    }
    /* assert() aborts, and stdout is fully buffered when piped (it is, under
     * `make test`): flush so a failure still shows its measurement. */
    fflush(stdout);
    assert(hits == 0);
    assert(drawn == 0);
}

/* THE POSITIVE CONTROL, without which "draws nothing, ever" would pass the
 * assertions above: the same path, given a genuine numeric sprite id, really
 * does draw that sprite, exactly where the layout put it. The item is the
 * number 0 -- truthy in Lua, where only nil and false are not -- so control
 * and subject differ in nothing but nil-ness and both go down the
 * sprite-id-0 path. */
static void test_numeric_item_still_draws_its_sprite(void) {
    int hit_x = -1, hit_y = -1;
    int hits, drawn;

    render_one_item(pico8_value_fixed(pico8_fixed_from_int(0)));

    hits = sprite0_bottom_row_hits(&hit_x, &hit_y);
    drawn = pixels_drawn();
    printf("[control] lq({0}):ol() drew %d pixel(s); sprite #0's bottom row appears %d time(s) at (%d,%d)\n", drawn,
           hits, hit_x, hit_y);
    fflush(stdout);
    assert(hits == 1);
    assert(hit_x == DRAW_X);
    assert(hit_y == DRAW_Y + SPRITE0_BOTTOM_ROW_Y);
    assert(drawn > 0);
}

int main(void) {
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));

    /* `cu()` fills the 0x5000 palette-block table that `dv()` copies slices
     * of. `lq:mn`'s sprite branch calls `dv(qw)`/`dv()` around its `spr()`,
     * so the control only draws what the cart would draw once it has run. */
    pico8_cu(&mem);

    pico8_layout_init();

    test_nil_item_draws_nothing();
    test_numeric_item_still_draws_its_sprite();

    printf("lq nil-item tests passed\n");
    return 0;
}
