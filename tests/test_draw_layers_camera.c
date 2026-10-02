#include "game/planet.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "engine/bootstrap.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/planet_display.h"
#include "game/scroll.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* `dc("kj")`'s `mo:md(ci[o])` special case, which needs both a real `mo`
 * and a real entity registered under "kj" to run at all: `jl()`, a real
 * `jo` and real gfx/mem. */

static int g_draw_calls = 0;
static int g_camera_x_at_draw;
static int g_camera_y_at_draw;

/* Records the camera `mo:md` has left by the time the planet's "kj" runs,
 * then forwards to the real `pico8_jo_kj`, so the order is observed without
 * stubbing the behaviour out. Installed on the planet alone, through a copy
 * of jo's class whose `kj` is this spy, so no other `jo` sees it. */
static void spy_draw(entity_t *self, const vec_t *ra) {
    vec_t p = ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));

    g_draw_calls++;
    g_camera_x_at_draw = dispatch_context_gfx()->camera_x;
    g_camera_y_at_draw = dispatch_context_gfx()->camera_y;
    planet_draw(dispatch_context_gfx(), dispatch_context_mem(), (planet_t *)self, p);
}

static entity_class_t g_spied_planet;

static void test_draw_layers_draw_dispatches_main_camera_apply_camera_then_real_planet_draw(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    planet_t *planet;
    entity_t *camera;
    vec_t *pos_heap;
    int screen_pixel_addr;
    unsigned char pixel_before;
    unsigned char pixel_after;

    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);

    g_draw_calls = 0;

    /* `jl()` builds the real `hv`/`cv`/`cg` and the real `mo=qb()`. */
    reset_world();
    camera = main_camera();
    assert(camera != NULL);

    planet_init();
    planet = planet_new(NULL);
    assert(planet != NULL);

    /* `jo` wires "kj" as a class default, so the instance must land in the
     * real `cv["kj"]` bucket. */
    {
        const entity_list_t *bucket = entities_by_message("draw");
        int found = 0;
        int i;
        assert(bucket != NULL);
        for (i = 0; i < bucket->count; i++) {
            if (bucket->items[i] == (entity_t *)planet) {
                found = 1;
            }
        }
        assert(found);
    }

    /* An "ra" inside `mo`'s default (192,192) viewport, so `kj`'s `gd(self)`
     * early return is not taken and the real draw body runs. */
    pos_heap = malloc(sizeof *pos_heap);
    *pos_heap = vec_make(p8_fixed_from_int(256), p8_fixed_from_int(256));
    planet->pos = pos_heap;

    assert(camera->class == camera_class());
    g_spied_planet = *planet_class();
    g_spied_planet.draw = spy_draw;
    planet->header.class = &g_spied_planet;

    /* `kj` paints colour 0, which is also the framebuffer's cleared value,
     * so reading 0 back proves nothing unless the byte held something else
     * first -- hence the sentinel 9. The address is world (256,256) minus
     * the camera (192,192) that `mo:md` sets inside this same `dc("kj")`,
     * hardcoded here because the camera is not set yet, and cross-checked
     * against the real camera below. */
    screen_pixel_addr = P8_ADDR_SCREEN + (256 - 192) * P8_SCREEN_WIDTH + (256 - 192);
    p8_poke(&mem, screen_pixel_addr, 9);
    pixel_before = p8_peek(&mem, screen_pixel_addr);
    assert(pixel_before == 9);

    assert(gfx.camera_x == 0 && gfx.camera_y == 0);
    draw_layers("draw");

    /* Order, never the reverse: within a layer, the "kj" special case runs
     * before the per-entity loop, so the real `kj` already sees its camera. */
    assert(g_draw_calls == 1);
    assert(g_camera_x_at_draw == 192 && g_camera_y_at_draw == 192);

    /* `mo:md` really ran: the camera lands on `qb`'s spec `ra=v(192,192)`,
     * `self.ma` having decayed to its 0 default so the shake adds nothing. */
    assert(gfx.camera_x == 192);
    assert(gfx.camera_y == 192);

    /* The sentinel address against the camera actually set, so the two
     * cannot drift apart. */
    assert(screen_pixel_addr == P8_ADDR_SCREEN + (256 - gfx.camera_y) * P8_SCREEN_WIDTH + (256 - gfx.camera_x));

    /* `circfill(p.x,p.y,7,0)` paints a black disc on world (256,256), i.e.
     * screen (64,64): the sentinel is gone. */
    pixel_after = p8_peek(&mem, screen_pixel_addr);
    assert(pixel_after == 0);

    assert(main_camera() == camera);

    planet->header.class = planet_class();
    planet_shutdown();
    bootstrap_shutdown();
}

int main(void) {
    test_draw_layers_draw_dispatches_main_camera_apply_camera_then_real_planet_draw();

    printf("test_draw_layers_camera: all tests passed\n");
    return 0;
}
