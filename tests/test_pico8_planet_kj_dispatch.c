#include "../src/pico8_planet.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/pico8_bootstrap.h"
#include "../src/pico8_class.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

/* Phase 3, Chunk 13 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md), sub-lot 5 -- the PRD's own explicitly flagged "first real,
 * non-dead dc('kj') exercise" (Testing Decisions section): once `jo` has a
 * real "kj" field AND `mo` is a real `qb` instance (both conditions became
 * true only in this sub-lot -- `mo` has been real since Chunk 6, `jo`'s
 * own "kj" field is this sub-lot's own work), `pico8_dc("kj")` finally
 * reaches something real for the first time in this whole port. This is
 * NOT a structurally new form of the generic `kv:jr`/`hv`/`cv`/`cg`/`dc`/
 * `mv`/`lt`/`he` mechanism itself -- `tests/test_pico8_entity_smoke.c`'s
 * own existing coverage already exercises `dc()`'s general per-bm-name,
 * per-layer-bucket dispatch loop (for "ev") and its own `cv["kj"]`
 * membership-vs-absence checks; sub-lots 1-4 of this same chunk already
 * established, repeatedly, that wiring one more `bm` name (`hb`,`hk`, now
 * `kj`) as a class-default field is a DATA change, not a MECHANISM change.
 * What genuinely is new here is that `dc("kj")`'s own `mo:md(ci[o])`
 * special case (`pico8_entity.c`'s own `pico8_dc`) has been dead-but-
 * compiled code since Phase 2 -- no existing test anywhere in this port
 * ever calls `pico8_dc("kj")` with a real `mo` AND a real registered "kj"
 * entity at the same time.
 *
 * This lives in its OWN test file/Makefile target, not
 * `tests/test_pico8_entity_smoke.c`, deliberately: that file's minimal
 * 8-source dependency graph (entity/class/ke/ob/list/value/str/fixed) is
 * itself a considered design choice, already reaffirmed once in this same
 * chunk (sub-lot 3's own `test_closure_context_survives_instantiate_field_
 * copy` used a generic fake class instead of a real `jo`/`hp` specifically
 * to avoid widening that file's own Makefile dependency graph). Proving
 * this exact moment for real needs a real `jo` instance, a real `mo=qb()`
 * singleton (`pico8_jl()`), and real gfx/mem -- the same dependency
 * closure `tests/test_pico8_data_init.c`'s own Makefile target already
 * pulls in for unrelated reasons (data decoding), reused here for a
 * topically distinct, entity-dispatch-focused permanent regression test
 * instead of bloating that file's own scope either. */

static char g_order[8];
static int g_order_n = 0;

/* Spy on mo's own "md": records call order, then forwards to the REAL,
 * exported pico8_qb_md (pico8_scroll.h) -- proves the real function still
 * runs, not just that something was invoked in its place. Installed as an
 * INSTANCE-level override on `mo` itself (pico8_instance_get_field checks
 * instance fields before the class chain, pico8_class.c), so this doesn't
 * change WHAT class-level "md" would otherwise resolve to for any other
 * qb instance, only lets this one test observe order for this one mo. */
static pico8_value_t spy_md(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t fy = (arg0.kind == PICO8_VALUE_FIXED) ? arg0.as.fixed : pico8_fixed_from_int(1);
    (void)argc;
    (void)arg1;
    assert(g_order_n < 8);
    g_order[g_order_n++] = 'M';
    pico8_qb_md(pico8_dispatch_gfx(), (pico8_instance_t *)self, fy);
    return pico8_value_ref(NULL);
}

/* Same idea for the planet's own "kj": records order, forwards to the
 * REAL, exported pico8_jo_kj (pico8_planet_display.h). */
static pico8_value_t spy_kj(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    assert(g_order_n < 8);
    g_order[g_order_n++] = 'K';
    pico8_jo_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self, p);
    return pico8_value_ref(NULL);
}

static void test_dc_kj_dispatches_mo_md_then_real_jo_kj(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_instance_t *planet;
    pico8_instance_t *mo;
    pico8_vec_t *ra_heap;
    int screen_pixel_addr;
    unsigned char pixel_before;
    unsigned char pixel_after;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);

    g_order_n = 0;

    /* Real jl(): builds real hv/cv/cg AND a real mo=qb() singleton --
     * exactly the condition this chunk's own PRD text says is one half of
     * "dc('kj') finally does something real" (the other half is `jo`
     * itself, wired below). */
    pico8_jl();
    mo = pico8_mo();
    assert(mo != NULL);

    /* Real jo class + a real jo instance -- the other half. */
    pico8_jo_init();
    planet = pico8_jo_new(NULL);
    assert(planet != NULL);

    /* Confirm real, non-dead cv["kj"] membership: this specific entity IS
     * registered under the real "kj" bm bucket now that jo wires it as a
     * class-default field. */
    {
        const pico8_list_t *bucket = pico8_cv("kj");
        int found = 0;
        int i;
        assert(bucket != NULL);
        for (i = 0; i < bucket->count; i++) {
            if (bucket->items[i].kind == PICO8_VALUE_REF && bucket->items[i].as.ref == planet) {
                found = 1;
            }
        }
        assert(found);
    }

    /* Give the planet a real "ra" inside mo's own default (192,192)
     * viewport (pico8_gd's own 76px-box visibility test), so kj's own
     * `gd(self)` early-return is NOT taken -- this test wants the real
     * draw body to run, not the early-out. */
    ra_heap = malloc(sizeof *ra_heap);
    *ra_heap = pico8_vec_v(pico8_fixed_from_int(256), pico8_fixed_from_int(256));
    pico8_ob_table_set_name(planet->fields, "ra", pico8_value_ref(ra_heap));

    /* Install order-recording spies -- both forward to the real, exported
     * function, so this proves order without stubbing out real behavior. */
    pico8_ob_table_set_name(mo->fields, "md", pico8_value_function(spy_md));
    pico8_ob_table_set_name(planet->fields, "kj", pico8_value_function(spy_kj));

    /* Sentinel: pre-poke the exact SCREEN framebuffer byte kj's own
     * circfill(256,256,7,0) is expected to paint, so that reading back
     * color 0 afterward proves a real write happened. This matters
     * because color 0 is ALSO this framebuffer's own zero-initialized
     * default (pico8_mem_init memsets to 0, pico8_mem.c) -- reading 0
     * there without ever having poked something else first would prove
     * nothing (this is exactly the bug this fix replaces: the previous
     * version of this assertion read pico8_sget(&mem,256,256), the
     * SPRITE SHEET at PICO8_ADDR_SPRITE_SHEET, not the screen at
     * PICO8_ADDR_SCREEN -- a different memory region entirely, and
     * (256,256) is out of sget's own 128x128 bounds anyway, so it
     * returned 0 unconditionally regardless of what kj actually drew).
     * The real screen address is world (256,256) minus the camera
     * (192,192) mo:md sets moments earlier in this same pico8_dc("kj")
     * call -- pico8_pset's own sx=x-camera_x,sy=y-camera_y convention
     * (pico8_gfx.c) -- hardcoded here (before the camera is actually set)
     * from the same reasoning assertion (b) below spells out, then
     * cross-checked against the real gfx.camera_x/camera_y right after
     * the call. */
    screen_pixel_addr = PICO8_ADDR_SCREEN + (256 - 192) * PICO8_SCREEN_WIDTH + (256 - 192);
    pico8_poke(&mem, screen_pixel_addr, 9);
    pixel_before = pico8_peek(&mem, screen_pixel_addr);
    assert(pixel_before == 9);

    /* THE call this whole sub-lot exists for. */
    pico8_dc("kj");

    /* (a) exact order: mo:md(ci[o]) once for the non-empty bucket, THEN
     * this entity's own "kj" -- never the reverse (pico8_entity.c's own
     * pico8_dc, read directly: the "kj" special case runs before the
     * per-entity loop within the same layer). */
    assert(g_order_n == 2);
    assert(g_order[0] == 'M');
    assert(g_order[1] == 'K');

    /* (b) mo:md really ran for real: camera(192,192) -- mo.ra's own
     * default value (qb's class spec `ra=v(192,192)`, pico8_ma's own
     * decay leaving self.ma at its own default 0, so p+qm(0,...)=p
     * unchanged), fy=ci[6]=1 for a "jo" instance's own default fa=6
     * layer bucket. */
    assert(gfx.camera_x == 192);
    assert(gfx.camera_y == 192);

    /* Cross-check the hardcoded sentinel address above against the real,
     * now-known camera state, so this test can't silently drift out of
     * sync with the (b) assertion it depends on. */
    assert(screen_pixel_addr == PICO8_ADDR_SCREEN + (256 - gfx.camera_y) * PICO8_SCREEN_WIDTH + (256 - gfx.camera_x));

    /* (c) the planet's own real kj really drew something:
     * circfill(p.x,p.y,7,0) paints a black disc centered on world
     * (256,256), landing on SCREEN (256-192,256-192)=(64,64) once the
     * camera shift is applied -- read back via pico8_peek at the real
     * PICO8_ADDR_SCREEN framebuffer address (not pico8_sget's unrelated
     * sprite-sheet region, see the sentinel comment above for the full
     * trace of why that was wrong). The sentinel poke proves this 0 is a
     * genuine overwrite by the real circfill call, not just the
     * framebuffer's own untouched zero default. */
    pixel_after = pico8_peek(&mem, screen_pixel_addr);
    assert(pixel_after == 0);

    assert(pico8_mo() == mo);

    pico8_jo_shutdown();
    pico8_bootstrap_shutdown();
}

int main(void) {
    test_dc_kj_dispatches_mo_md_then_real_jo_kj();

    printf("test_pico8_planet_kj_dispatch: all tests passed\n");
    return 0;
}
