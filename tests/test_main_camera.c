/* The placeholder main camera that entity_registry_init() leaves until
 * `reset_world()` replaces it with a `camera_t`, read through
 * main_camera_typed(). The frozen values were taken on the code where the
 * placeholder was still a class-less instance and must not be re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/scroll.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "engine/vec.h"

#define IS_ON_SCREEN_NEAR 1
#define IS_ON_SCREEN_FAR 0
#define UPDATE_MOUSE_WORLD_X 0
#define UPDATE_MOUSE_WORLD_Y 0
#define CAMERA_POS 192

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

/* Not a `camera_t`; no method answers and the camera stays put; the live
 * entities and the messages are empty. */
static void assert_placeholder(void) {
    const char *buckets[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};
    entity_t *mo = main_camera();
    int i;

    assert(mo != NULL && main_camera_typed() == NULL);
    p8_camera(&gfx, 5, 6);
    assert(mo->class != camera_class());
    assert(mo->class->states[ENTITY_STATE_NORMAL] == NULL && mo->class->draw == NULL);
    assert(gfx.camera_x == 5 && gfx.camera_y == 6);
    assert(live_entities()->count == 0);
    for (i = 0; i < 5; i++) {
        assert(entities_by_message(buckets[i])->count == 0);
    }
}

static void assert_a_camera(void) {
    const vec_t *ra;

    assert(main_camera_typed() != NULL && main_camera_typed()->pos != NULL);
    ra = main_camera_typed()->pos;
    assert(ra->x == p8_fixed_from_int(CAMERA_POS) && ra->y == p8_fixed_from_int(CAMERA_POS));
}

static void test_placeholder_after_init(void) {
    entity_registry_init();
    assert_placeholder();
    assert(!entity_list_listed(live_entities(), main_camera()));
}

typedef struct {
    entity_t header;
    const vec_t *pos;
} pos_only_t;

static const entity_class_t pos_only_class_def = {0};

/* entity_geom_is_on_screen reads the camera position as (0,0): 64 off is
 * inside the 76 margin, 77 is not. */
static void test_is_on_screen_on_placeholder(void) {
    vec_t near_pos = vec_make(p8_fixed_from_int(64), p8_fixed_from_int(64));
    vec_t far_pos = vec_make(p8_fixed_from_int(64 + 77), p8_fixed_from_int(64));
    pos_only_t near_e = {{.class = &pos_only_class_def, .pos = &near_e.pos}, NULL};
    pos_only_t far_e = {{.class = &pos_only_class_def, .pos = &far_e.pos}, NULL};
    int near_hit;
    int far_hit;

    entity_registry_init();
    near_e.pos = &near_pos;
    far_e.pos = &far_pos;
    near_hit = entity_geom_is_on_screen(&near_e.header);
    far_hit = entity_geom_is_on_screen(&far_e.header);
    printf("[is_on_screen] near %d far %d\n", near_hit, far_hit);
    fflush(stdout);
    assert(near_hit == IS_ON_SCREEN_NEAR && far_hit == IS_ON_SCREEN_FAR);
}

/* A ui_update on the placeholder, the `ui_t` built by hand since
 * `reset_world()` has not run: `mouse_world` is the mouse position plus the
 * camera position. */
static void test_update_on_placeholder(void) {
    ui_t *ok;
    const vec_t *qa;

    entity_registry_init();
    ok = ui_new();
    ui_update(&mem, ok);
    assert(ok->mouse_world != NULL);
    qa = ok->mouse_world;
    printf("[update mouse_world] (%d,%d)\n", (int)qa->x, (int)qa->y);
    fflush(stdout);
    assert(qa->x == UPDATE_MOUSE_WORLD_X && qa->y == UPDATE_MOUSE_WORLD_Y);
}

static void test_shutdown_and_init_cycle(void) {
    entity_registry_shutdown();
    assert(main_camera() == NULL);
    entity_registry_init();
    assert_placeholder();
    entity_registry_init();
    entity_registry_init();
    assert_placeholder();
    entity_registry_shutdown();
    entity_registry_shutdown();
    assert(main_camera() == NULL);
}

/* `reset_world()` puts a `camera_t` in its place; entity_registry_init()
 * puts it back. */
static void test_replaced_by_reset_world(void) {
    reset_world();
    assert_a_camera();
    entity_registry_init();
    assert_placeholder();
    reset_world();
    bootstrap_shutdown();
    assert(main_camera() == NULL);
    reset_world();
    assert_a_camera();
}

static int is_a_struct_of(const void *e, const entity_class_t *cls) {
    return e != NULL &&
           ((const entity_t *)e)->class == cls;
}

/* A struct of its own class, one object for the whole process. A
 * `camera_t` once `reset_world()` has run. */
static void test_placeholder_is_a_struct(void) {
    const void *placeholder;

    entity_registry_init();
    placeholder = main_camera();
    assert(is_a_struct_of(placeholder, main_camera_placeholder_class()));
    reset_world();
    assert(is_a_struct_of(main_camera(), camera_class()));
    entity_registry_init();
    assert(main_camera() == placeholder);
    bootstrap_shutdown();
}

/* A shaken camera: its position is off its position before shake, and off
 * its own diagonal. entity_geom_is_on_screen reads the camera's position,
 * never its position before shake, and each axis on its own. */
static void test_is_on_screen_under_a_shaken_camera(void) {
    static const vec_t zero_pos_before_shake = {0, 0};
    static vec_t shaken_pos;
    vec_t in_pos = vec_make(p8_fixed_from_int(200 + 64), p8_fixed_from_int(100 + 64));
    vec_t below_pos = vec_make(p8_fixed_from_int(200 + 64), p8_fixed_from_int(100 + 64 + 77));
    pos_only_t in_e = {{.class = &pos_only_class_def, .pos = &in_e.pos}, NULL};
    pos_only_t below_e = {{.class = &pos_only_class_def, .pos = &below_e.pos}, NULL};
    camera_t *qb;
    const vec_t *saved_p;
    const vec_t *saved_pos;

    reset_world();
    qb = main_camera_typed();
    assert(qb != NULL);
    saved_p = qb->pos_before_shake;
    saved_pos = qb->pos;
    shaken_pos = vec_make(p8_fixed_from_int(200), p8_fixed_from_int(100));
    qb->pos_before_shake = &zero_pos_before_shake;
    qb->pos = &shaken_pos;
    in_e.pos = &in_pos;
    below_e.pos = &below_pos;
    assert(entity_geom_is_on_screen(&in_e.header) == IS_ON_SCREEN_NEAR);
    assert(entity_geom_is_on_screen(&below_e.header) == IS_ON_SCREEN_FAR);
    qb->pos_before_shake = saved_p;
    qb->pos = saved_pos;
    bootstrap_shutdown();
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

    test_placeholder_after_init();
    test_is_on_screen_on_placeholder();
    test_update_on_placeholder();
    test_shutdown_and_init_cycle();
    test_replaced_by_reset_world();
    test_placeholder_is_a_struct();
    test_is_on_screen_under_a_shaken_camera();

    printf("test_main_camera: all tests passed\n");
    return 0;
}
