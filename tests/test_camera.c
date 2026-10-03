/* The camera `camera_t` (`main_camera()` once `reset_world()` has run),
 * read and written on its members through main_camera_typed() and driven
 * through the real update_entities and draw_layers, in games built directly:
 * no events file. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/planet_family.h"
#include "game/scroll.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

#define SCRIPTED_TICKS 110
#define SCRIPTED_HASH 0xDF9F2872u
#define SHAKE_TICKS 31
#define SHAKE_HASH 0xF1E7A593u
#define GEOHARVEST_WINDOW 60

static p8_memory_t memory;
static p8_graphics_t graphics;
static p8_input_t input;

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static int entity_list_listed(const entity_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return 1;
        }
    }
    return 0;
}

static int in_any_entities_by_message(const void *e) {
    const char *names[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};
    int i;

    for (i = 0; i < 5; i++) {
        if (entity_list_listed(entities_by_message(names[i]), e)) {
            return 1;
        }
    }
    return 0;
}

static camera_t *checked_main_camera(void) {
    assert(main_camera_typed() != NULL);
    return main_camera_typed();
}

static vec_t must_vec(const vec_t *v) {
    assert(v != NULL);
    return *v;
}

static uint32_t fold_vec(uint32_t h, const vec_t *v) {
    if (v == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)v->x);
    return fnv(h, (uint32_t)v->y);
}

static uint32_t fold_main_camera(uint32_t h) {
    const camera_t *mo = checked_main_camera();

    h = fold_vec(h, mo->pos_before_shake);
    h = fold_vec(h, mo->velocity);
    h = fold_vec(h, mo->pos);
    h = fnv(h, (uint32_t)mo->shake);
    h = fnv(h, (uint32_t)graphics.camera_x);
    return fnv(h, (uint32_t)graphics.camera_y);
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static p8_input_frame_t g_frames[SCRIPTED_TICKS];

static void script(int n) {
    input.frames = g_frames;
    input.frame_count = n;
    input.current_frame = 0;
    dispatch_context_set_input(&input);
}

static void unscript(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
}

static void pad(int i, int left, int right, int up, int down) {
    memset(&g_frames[i], 0, sizeof g_frames[i]);
    g_frames[i].dpad[0] = left;
    g_frames[i].dpad[1] = right;
    g_frames[i].dpad[2] = up;
    g_frames[i].dpad[3] = down;
}

static int is_vec(vec_t v, int x, int y) {
    return v.x == p8_fixed_from_int(x) && v.y == p8_fixed_from_int(y);
}

/* `reset_world()` builds the main camera: among the live entities only, the
 * archetype's defaults, the entity's state and state time, the dpad
 * directions in the order left, right, up, down, and the position before
 * shake and the position two distinct vectors. */
static void check_fresh_main_camera(const camera_t *mo) {
    const vec_seq_t *pc;
    vec_t p = must_vec(mo->pos_before_shake);
    vec_t ra = must_vec(mo->pos);
    vec_t v = must_vec(mo->velocity);

    assert(entity_list_listed(live_entities(), mo) && !in_any_entities_by_message(mo));
    assert(p.x == p8_fixed_from_int(192) && p.y == p8_fixed_from_int(192));
    assert(ra.x == p8_fixed_from_int(192) && ra.y == p8_fixed_from_int(192));
    assert(v.x == 0 && v.y == 0);
    assert(mo->shake == 0);
    assert(mo->header.state_time == 0);
    assert(mo->header.state != NULL && strcmp(mo->header.state, "normal") == 0);
    assert(mo->pos_before_shake != mo->pos);

    pc = mo->archetype->dpad_directions;
    assert(pc != NULL);
    assert(pc == &camera_dpad_directions_vecs && pc->count == 4);
    assert(is_vec(pc->items[0], -1, 0));
    assert(is_vec(pc->items[1], 1, 0));
    assert(is_vec(pc->items[2], 0, -1));
    assert(is_vec(pc->items[3], 0, 1));
}

static void test_reset_world_builds_a_camera(void) {
    reset_world();
    check_fresh_main_camera(checked_main_camera());
}

static void apply_camera_to(p8_fixed_t fy, int want_x, int want_y) {
    p8_camera(&graphics, 7, 11);
    assert(main_camera()->class == camera_class());
    camera_apply(dispatch_context_gfx(), (camera_t *)main_camera(), fy);
    assert(graphics.camera_x == want_x && graphics.camera_y == want_y);
}

/* camera_apply puts the camera on the position times the factor. The later
 * tests of this process start from the camera the last call leaves: 192,192. */
static void test_apply_camera_sets_the_camera(void) {
    reset_world();
    apply_camera_to(p8_fixed_from_int(1), 192, 192);
    apply_camera_to(P8_FIXED_LIT(0x00008000, 0.5), 96, 96);
    apply_camera_to(0, 0, 0);
    apply_camera_to(p8_fixed_from_int(1), 192, 192);
}

/* Idle, left, right+up, down held into the 320 bound, then released. */
static void test_scripted_ticks(void) {
    uint32_t h = 2166136261u;
    int bounded = 0;
    int i;

    reset_world();
    for (i = 0; i < SCRIPTED_TICKS; i++) {
        if (i < 3) {
            pad(i, 0, 0, 0, 0);
        } else if (i < 8) {
            pad(i, 1, 0, 0, 0);
        } else if (i < 16) {
            pad(i, 0, 1, 1, 0);
        } else if (i < 90) {
            pad(i, 0, 0, 0, 1);
        } else {
            pad(i, 0, 0, 0, 0);
        }
    }
    script(SCRIPTED_TICKS);
    for (i = 0; i < SCRIPTED_TICKS; i++) {
        update_entities();
        h = fold_main_camera(h);
        h = fnv(h, (uint32_t)checked_main_camera()->header.state_time);
        if (must_vec(checked_main_camera()->pos_before_shake).y == p8_fixed_from_int(320)) {
            bounded++;
        }
        if (i < SCRIPTED_TICKS - 1) {
            p8_input_step(&input);
        }
    }
    unscript();
    printf("[scripted] pos_before_shake.y at the 320 bound on %d ticks\n", bounded);
    assert(bounded > 0);
    check("scripted", h, SCRIPTED_HASH);
}

/* A shake of 3, as a burst's construction writes it, decays by 0.1 per
 * camera_update, shaking the position off the position before shake by
 * entity_geom_shake. */
static void test_shake_decays(void) {
    uint32_t h = 2166136261u;
    int ticks = 0;
    const camera_t *mo;

    reset_world();
    mo = checked_main_camera();
    checked_main_camera()->shake = p8_fixed_from_int(3);
    assert(mo->shake == p8_fixed_from_int(3));
    unscript();
    while (mo->shake != 0) {
        vec_t off;
        update_entities();
        ticks++;
        assert(ticks < 100);
        off = vec_sub(must_vec(mo->pos), must_vec(mo->pos_before_shake));
        h = fnv(h, (uint32_t)mo->shake);
        h = fnv(h, (uint32_t)off.x);
        h = fnv(h, (uint32_t)off.y);
    }
    printf("[shake] shake back to 0 after %d ticks\n", ticks);
    fflush(stdout);
    assert(ticks == SHAKE_TICKS);
    check("shake", h, SHAKE_HASH);
}

/* `cart_start_game()` runs `reset_world()` again over a live main camera; a
 * shutdown and a fresh `reset_world()` after it must not crash either. */
static void test_second_reset_world(void) {
    const void *first;

    reset_world();
    first = main_camera();
    reset_world();
    assert(main_camera() != first);
    assert(!entity_list_listed(live_entities(), first));
    check_fresh_main_camera(checked_main_camera());

    bootstrap_shutdown();
    reset_world();
    check_fresh_main_camera(checked_main_camera());
}

/* Before `reset_world()`, the main camera is the placeholder, not a
 * `camera_t`: nothing is applied and the camera stays put. */
static void test_placeholder_main_camera(void) {
    entity_registry_init();
    p8_camera(&graphics, 5, 6);
    assert(main_camera()->class != camera_class());
    assert(graphics.camera_x == 5 && graphics.camera_y == 6);
    assert(main_camera_typed() == NULL);
}

/* A game started as cart_start_game starts it, the right button pressed over
 * empty space then dragged: `main_ui()` hands the press to the background,
 * whose drag moves the main camera. */
static void test_right_drag_moves_the_camera(void) {
    vec_t before;
    vec_t after;
    int i;

    cart_init();
    cart_start_game(NULL);
    for (i = 0; i < 20; i++) {
        memset(&g_frames[i], 0, sizeof g_frames[i]);
        g_frames[i].mouse_x = 64 + 2 * i;
        g_frames[i].mouse_y = 64 + i;
        g_frames[i].mouse_buttons = i < 18 ? 2 : 0;
    }
    script(20);
    cart_update60();
    before = must_vec(checked_main_camera()->pos_before_shake);
    for (i = 1; i < 20; i++) {
        p8_input_step(&input);
        cart_update60();
    }
    unscript();
    after = must_vec(checked_main_camera()->pos_before_shake);
    assert(after.x != before.x || after.y != before.y);
}

static int burst_alive(void) {
    const entity_list_t *hv = live_entities();
    int i;

    for (i = 0; i < hv->count; i++) {
        const void *e = hv->items[i];
        if (e != NULL &&
            ((const entity_t *)e)->class == burst_class()) {
            return 1;
        }
    }
    return 0;
}

/* A burst's construction shakes the camera: a shake of 3 as soon as it is
 * built, then the decay back to 0 within GEOHARVEST_WINDOW ticks. */
static void test_burst_shakes(void) {
    int i;

    cart_init();
    cart_start_game(NULL);
    assert(!burst_alive());
    (void)burst_new(NULL);
    assert(burst_alive());
    assert(checked_main_camera()->shake == p8_fixed_from_int(3));
    for (i = 0; i < GEOHARVEST_WINDOW; i++) {
        update_entities();
    }
    assert(checked_main_camera()->shake == 0);
}

/* The galaxy depends on the rnd state, which the tests before it consume: run
 * it in a child forked right after cart_load(), as a fresh process
 * would start it. */
static void in_fresh_child(void (*test)(void)) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        test();
        fflush(stdout);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static int is_a_camera_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == camera_class();
}

/* The main camera of `reset_world()` and one built by camera_new are camera
 * structs among the live entities only, with an update and an apply as
 * methods and no draw; the update reads a NULL velocity as zero. */
static void test_camera_is_a_struct(void) {
    camera_t *generic;

    reset_world();
    generic = camera_new();
    assert(is_a_camera_struct(main_camera()) && is_a_camera_struct(generic));
    assert(entity_list_listed(live_entities(), generic) && !in_any_entities_by_message(generic));
    check_fresh_main_camera(generic);
    assert(generic->header.pos == &generic->pos);
    assert(generic->header.map_pos == &generic->pos_before_shake);
    camera_class()->states[ENTITY_STATE_NORMAL](&generic->header, p8_fixed_from_int(0));
    camera_apply(dispatch_context_gfx(), generic, p8_fixed_from_int(1));
    assert(camera_class()->draw == NULL);
    {
        vec_t p_before = must_vec(generic->pos_before_shake);

        generic->velocity = NULL;
        camera_class()->states[ENTITY_STATE_NORMAL](&generic->header, p8_fixed_from_int(0));
        assert(generic->velocity != NULL && generic->velocity->x == 0 && generic->velocity->y == 0);
        assert(generic->pos_before_shake->x == p_before.x && generic->pos_before_shake->y == p_before.y);
    }
}

int main(void) {
    p8_mem_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    assert(p8_load_map_json(&memory, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);

    cart_load();
    in_fresh_child(test_right_drag_moves_the_camera);
    in_fresh_child(test_burst_shakes);

    test_reset_world_builds_a_camera();
    test_apply_camera_sets_the_camera();
    test_scripted_ticks();
    test_shake_decays();
    test_second_reset_world();
    test_placeholder_main_camera();
    test_camera_is_a_struct();

    printf("test_camera: all tests passed\n");
    return 0;
}
