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
#include "../src/pico8_struct.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

/* `dc("kj")`'s `mo:md(ci[o])` special case, which needs both a real `mo`
 * and a real entity registered under "kj" to run at all: `jl()`, a real
 * `jo` and real gfx/mem. */

static char g_order[8];
static int g_order_n = 0;

/* Records the call order, then forwards to the real `pico8_qb_md`, so the
 * order is observed without stubbing the behaviour out. Installed on `mo`
 * alone, through a copy of qb's method table whose `md` entry is this spy,
 * so no other `qb` sees it. */
static pico8_value_t spy_md(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t fy = (arg0.kind == PICO8_VALUE_FIXED) ? arg0.as.fixed : pico8_fixed_from_int(1);
    (void)argc;
    (void)arg1;
    assert(g_order_n < 8);
    g_order[g_order_n++] = 'M';
    pico8_qb_md(pico8_dispatch_gfx(), (pico8_qb_t *)self, fy);
    return pico8_value_ref(NULL);
}

/* Same, for the planet's "kj". */
static pico8_value_t spy_kj(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    assert(g_order_n < 8);
    g_order[g_order_n++] = 'K';
    pico8_jo_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jo_t *)self, p);
    return pico8_value_ref(NULL);
}

static pico8_struct_class_t g_spied_qb;
static pico8_struct_class_t g_spied_jo;

static void test_dc_kj_dispatches_mo_md_then_real_jo_kj(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_jo_t *planet;
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

    /* `jl()` builds the real `hv`/`cv`/`cg` and the real `mo=qb()`. */
    pico8_jl();
    mo = pico8_mo();
    assert(mo != NULL);

    pico8_jo_init();
    planet = pico8_jo_new(NULL);
    assert(planet != NULL);

    /* `jo` wires "kj" as a class default, so the instance must land in the
     * real `cv["kj"]` bucket. */
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

    /* An "ra" inside `mo`'s default (192,192) viewport, so `kj`'s `gd(self)`
     * early return is not taken and the real draw body runs. */
    ra_heap = malloc(sizeof *ra_heap);
    *ra_heap = pico8_vec_v(pico8_fixed_from_int(256), pico8_fixed_from_int(256));
    planet->ra = pico8_value_ref(ra_heap);

    g_spied_qb = *pico8_qb_struct_class();
    g_spied_qb.methods[PICO8_METHOD_MD] = spy_md;
    ((pico8_struct_header_t *)mo)->cls = &g_spied_qb;
    g_spied_jo = *pico8_jo_struct_class();
    g_spied_jo.methods[PICO8_METHOD_KJ] = spy_kj;
    planet->ke.header.cls = &g_spied_jo;

    /* `kj` paints colour 0, which is also the framebuffer's cleared value,
     * so reading 0 back proves nothing unless the byte held something else
     * first -- hence the sentinel 9. The address is world (256,256) minus
     * the camera (192,192) that `mo:md` sets inside this same `dc("kj")`,
     * hardcoded here because the camera is not set yet, and cross-checked
     * against the real camera below. */
    screen_pixel_addr = PICO8_ADDR_SCREEN + (256 - 192) * PICO8_SCREEN_WIDTH + (256 - 192);
    pico8_poke(&mem, screen_pixel_addr, 9);
    pixel_before = pico8_peek(&mem, screen_pixel_addr);
    assert(pixel_before == 9);

    pico8_dc("kj");

    /* Order, never the reverse: within a layer, the "kj" special case runs
     * before the per-entity loop. */
    assert(g_order_n == 2);
    assert(g_order[0] == 'M');
    assert(g_order[1] == 'K');

    /* `mo:md` really ran: the camera lands on `qb`'s spec `ra=v(192,192)`,
     * `self.ma` having decayed to its 0 default so the shake adds nothing. */
    assert(gfx.camera_x == 192);
    assert(gfx.camera_y == 192);

    /* The sentinel address against the camera actually set, so the two
     * cannot drift apart. */
    assert(screen_pixel_addr == PICO8_ADDR_SCREEN + (256 - gfx.camera_y) * PICO8_SCREEN_WIDTH + (256 - gfx.camera_x));

    /* `circfill(p.x,p.y,7,0)` paints a black disc on world (256,256), i.e.
     * screen (64,64): the sentinel is gone. */
    pixel_after = pico8_peek(&mem, screen_pixel_addr);
    assert(pixel_after == 0);

    assert(pico8_mo() == mo);

    ((pico8_struct_header_t *)mo)->cls = pico8_qb_struct_class();
    planet->ke.header.cls = pico8_jo_struct_class();
    pico8_jo_shutdown();
    pico8_bootstrap_shutdown();
}

int main(void) {
    test_dc_kj_dispatches_mo_md_then_real_jo_kj();

    printf("test_pico8_planet_kj_dispatch: all tests passed\n");
    return 0;
}
