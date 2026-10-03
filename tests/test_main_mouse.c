/* The placeholder main mouse that ui_init() leaves until `reset_world()`
 * replaces it with a `mouse_t`. The frozen mouse world position was taken on
 * the code where the placeholder was still a class-less instance and must
 * not be re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/mouse.h"
#include "p8/mem.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "engine/vec.h"

#define UPDATE_MOUSE_WORLD_X 0
#define UPDATE_MOUSE_WORLD_Y 0

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static int entity_list_listed(const entity_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return 1;
        }
    }
    return 0;
}

/* The messages may be read only once entity_registry_init() has run. */
static int registered(const void *e, int entity_live) {
    const char *buckets[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};
    const char *tags[] = {"mouse", "ui", "camera", "entity"};
    size_t i;

    if (entity_list_listed(live_entities(), e)) {
        return 1;
    }
    for (i = 0; entity_live && i < sizeof buckets / sizeof buckets[0]; i++) {
        if (entity_list_listed(entities_by_message(buckets[i]), e)) {
            return 1;
        }
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        if (entity_list_listed(entities_by_tag(tags[i]), e)) {
            return 1;
        }
    }
    return 0;
}

/* Not a `mouse_t`; no method answers; nothing registers it. */
static void assert_placeholder(int entity_live) {
    entity_t *je = main_mouse();

    assert(je != NULL && main_mouse_typed() == NULL);
    assert(je->class->states[ENTITY_STATE_NORMAL] == NULL);
    assert(je->class->draw == NULL);
    assert(!registered(je, entity_live));
}

static void assert_a_mouse(void) {
    const mouse_t *nf = main_mouse_typed();

    assert(nf != NULL && nf->pos != NULL);
    assert(nf->pos->x == 0 && nf->pos->y == 0);
    assert(mouse_left_button() != NULL && mouse_right_button() != NULL);
}

/* Between cart_load() and cart_init(). */
static void test_placeholder_after_cart_load(void) {
    assert_placeholder(0);
}

/* A ui_update on the placeholder, the `ui_t` built by hand since
 * `reset_world()` has not run: no press, `mouse_world` is the camera
 * position alone. */
static void test_update_on_placeholder(void) {
    ui_t *ok;
    const vec_t *qa;

    entity_registry_init();
    assert_placeholder(1);
    ok = ui_new();
    ui_update(&mem, ok);
    assert(ok->left_held == NULL && ok->right_held == NULL);
    assert(ok->mouse_world != NULL);
    qa = ok->mouse_world;
    printf("[update mouse_world] (%d,%d)\n", (int)qa->x, (int)qa->y);
    fflush(stdout);
    assert(qa->x == UPDATE_MOUSE_WORLD_X && qa->y == UPDATE_MOUSE_WORLD_Y);
}

/* Shutdown clears it; every init brings it back; cart_shutdown() and
 * cart_load() too. */
static void test_shutdown_and_init_cycle(void) {
    ui_shutdown();
    assert(main_mouse() == NULL);
    ui_init();
    assert_placeholder(0);
    ui_init();
    ui_init();
    assert_placeholder(0);
    cart_shutdown();
    assert(main_mouse() == NULL);
    cart_load();
    assert_placeholder(0);
}

/* `reset_world()` puts a `mouse_t` in its place; ui_init() puts it back. */
static void test_replaced_by_reset_world(void) {
    cart_init();
    assert_a_mouse();
    ui_init();
    assert_placeholder(1);
}

/* The path of a caller that never calls cart_load(). */
static void test_reset_world_without_cart_load(void) {
    /* ui_init() after reset_world() freed `main_ui()`, still live:
     * cart_shutdown() would abandon it, so the live entities go first. */
    bootstrap_shutdown();
    cart_shutdown();
    reset_world();
    assert_a_mouse();
    bootstrap_shutdown();
    assert(main_mouse() == NULL);
    reset_world();
    assert_a_mouse();
    bootstrap_shutdown();
}

static int is_a_struct_of(const void *e, const entity_class_t *cls) {
    return e != NULL &&
           ((const entity_t *)e)->class == cls;
}

/* A struct of its own class, one object for the whole process; a
 * `mouse_t` once `reset_world()` has run. */
static void test_placeholder_is_a_struct(void) {
    const void *placeholder;

    cart_load();
    placeholder = main_mouse();
    assert(is_a_struct_of(placeholder, main_mouse_placeholder_class()));
    reset_world();
    assert(is_a_struct_of(main_mouse(), mouse_class()));
    ui_init();
    assert(main_mouse() == placeholder);
    bootstrap_shutdown();
    cart_shutdown();
}

int main(void) {
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);

    cart_load();

    test_placeholder_after_cart_load();
    test_update_on_placeholder();
    test_shutdown_and_init_cycle();
    test_replaced_by_reset_world();
    test_reset_world_without_cart_load();
    test_placeholder_is_a_struct();

    printf("test_main_mouse: all tests passed\n");
    return 0;
}
