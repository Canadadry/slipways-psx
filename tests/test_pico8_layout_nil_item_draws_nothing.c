/* Phase 5 bug fix, second half: `lq:mn`'s own `elseif f then` must treat
 * this port's nil (`pico8_value_ref(NULL)`, src/pico8_value.h) as FALSE.
 *
 * THE REAL LUA (pico8/lua.lua:300-312):
 *
 *   function lq:mn(f)local w,qw,qv,fn=6,0
 *   ...
 *   if type(f)=="string" then ... return function(p)io(f,p.x+2,p.y+1,c)end,#f*4+3,1
 *   elseif f then
 *   fn=function(p)dv(qw)spr(f,p.x,p.y)dv()end
 *   end
 *   return fn or function()end,w,qv or 2
 *
 * A nil item is neither a string nor truthy, so BOTH branches are skipped
 * and `fn` stays nil: `fn or function()end` hands lq:ol a closure that
 * DRAWS NOTHING. The item still occupies its default `w=6` of layout
 * width -- it is invisible, not absent.
 *
 * WHAT WENT WRONG: src/pico8_layout.c's own truthiness helper reported
 * this port's nil as TRUE, so `elseif f then` was taken, the sprite id
 * fell back to 0, and the row drew PICO-8 sprite #0 -- an 8x8 block of
 * raw sprite-sheet colour-ramp data -- straight into the framebuffer.
 *
 * WHY THIS TEST EXISTS ALONGSIDE tests/test_pico8_built_planet_panel_nil_
 * row.c: that test is the end-to-end reproduction of the reported bug, and
 * it pins the MSGBOX half of the fix (`pz()`'s own `if p.ep then`,
 * pico8/lua.lua:280). It does not pin this half. Measured: reverting
 * src/pico8_layout.c's own `REF(NULL)` clause on its own leaves the whole
 * suite green, because once `pz()` stops building the bogus row, no nil
 * ever reaches `lq:mn` again on any replay in the repo -- the single nil
 * in the entire corpus (9729-frame events.json plus the 278-frame fixture)
 * was manufactured by the msgbox bug itself. The `lq:mn` guard would have
 * been silently un-tested: correct today, and free to rot the moment any
 * other caller hands `lq()` a nil item. This test closes exactly that gap
 * and nothing else.
 *
 * HOW IT REACHES THE REAL CODE: through the ordinary public interface
 * only. `lq:mn` is private by design (src/pico8_layout.h says plainly that
 * nothing outside pico8_layout.c calls it), so this test never names it:
 * it builds a real `lq` instance with pico8_lq_new() holding ONE
 * positional item -- exactly the shape every real `lq({...})` call site
 * has -- and renders it with pico8_lq_ol_default() into a real
 * pico8_mem_t, then reads the framebuffer back. `lq:mn` runs because
 * `lq:pq()` (the class's own constructor) runs it, the way it does in the
 * cart.
 *
 * WHAT IT ASSERTS, on observable output rather than on the shape of the
 * fix:
 *   - NIL ITEM: the framebuffer is bit-for-bit as `cls()` left it -- not
 *     one pixel written -- and in particular sprite #0's own bottom pixel
 *     row appears nowhere on screen. That 8-pixel run is read back out of
 *     the loaded sprite sheet with pico8_sget(), never hard-coded. Row
 *     y=7 is used rather than sprite #0's row y=0 ON PURPOSE: row 0 is all
 *     zeros, so it matches blank background at every position on screen
 *     and would make the assertion vacuous. Row 7 is six distinct colours
 *     and occurs exactly once in the whole 128x128 sheet.
 *   - POSITIVE CONTROL, so that "draws nothing, ever" cannot pass: the
 *     SAME rendering path, given a genuine numeric item, really does draw
 *     that sprite -- the run appears exactly once, at exactly the pixel
 *     the layout put it. The control's item is the number 0, which is
 *     TRUE in Lua (only nil and false are not), so control and subject
 *     differ in nothing but nil-ness and both go down the sprite-id-0
 *     path: the control proves sprite #0 is drawable here, which is what
 *     makes the subject's zero hits mean something. */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_class.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_hud.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

/* Where the row is rendered. Any position clear of the screen edges does;
 * these are chosen so the whole 8x8 sprite the faulty build drew lands
 * well inside the framebuffer. */
#define DRAW_X 20
#define DRAW_Y 30

/* Sprite #0's bottom pixel row -- see this file's own top comment for why
 * it is row 7 and not row 0. */
#define SPRITE0_ROW_PIXELS 8
#define SPRITE0_BOTTOM_ROW_Y 7

static pico8_mem_t mem;
static pico8_gfx_t gfx;

static int screen_px(int x, int y) {
    return pico8_peek(&mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x);
}

/* Scans the whole framebuffer for the 8 pixels `spr(0)` lays down for
 * sprite #0's bottom row, read back out of the loaded sprite sheet.
 * Returns how many places it appears, and writes the first one's
 * top-left x/y through the out-params. */
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

/* How many framebuffer pixels `cls()` did NOT leave behind -- i.e. how
 * much this render actually drew. */
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

/* `lq({item}):ol(v(DRAW_X,DRAW_Y))` -- one real `lq` instance holding one
 * positional item, rendered through the public interface onto a cleared
 * screen. Exactly the shape of every real `lq({...})` call site in the
 * cart; nothing private is touched. */
static void render_one_item(pico8_value_t item) {
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    pico8_instance_t *line;

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
    /* assert() aborts, and stdout is fully buffered when this test's
     * output is piped (it is, under `make test`) -- flush so a failure
     * still shows the measurement that produced it. */
    fflush(stdout);
    assert(hits == 0);
    assert(drawn == 0);
}

/* THE POSITIVE CONTROL: the same path, given a genuine numeric sprite id,
 * really does draw that sprite -- otherwise "draws nothing, ever" would
 * pass the assertion above. */
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

    /* `cu()` -- the cart's own boot step (pico8/lua.lua's `_init`, ported
     * in src/pico8_cart.c) that fills the 0x5000 palette-block table
     * `dv()` copies slices of. `lq:mn`'s own sprite branch really does
     * call `dv(qw)`/`dv()` around its `spr()`, so the block table has to
     * be the real one for the control to draw what the cart would draw. */
    pico8_cu(&mem);

    pico8_layout_init();

    test_nil_item_draws_nothing();
    test_numeric_item_still_draws_its_sprite();

    printf("lq nil-item tests passed\n");
    return 0;
}
