/* The background `bg`, read and written only through the dual-form field
 * API and driven through the real `mv`/`dg`/`dc`/`ok`, in games built
 * directly: no events file. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "p8/assets.h"
#include "game/background.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/scroll.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "engine/vec.h"
#include "game/wormhole.h"


#define MAP_HASH 0xBBDC5BFAu
#define MAP_STARS 4
#define MAP_ZEROS 625
#define SECOND_MAP_HASH 0x2E2CECCAu
#define DRAG_HASH 0x2CB62643u
#define LEFT_PRESS_PROBE_BEFORE 0
#define LEFT_PRESS_PROBE_AFTER 1
#define LEFT_PRESS_WORMHOLE_BEFORE 0
#define LEFT_PRESS_WORMHOLE_AFTER 1
#define HOVER_HASH 0x477E2027u
#define DRAW_HASH 0xE18E0472u

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const int want_tile_choices[15] = {0, 0, 0, 72, 73, 74, 75, 88, 89, 90, 91, 88, 91, 89, 91};

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)p8_peek(&mem, P8_ADDR_SCREEN + i);
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

static int entity_list_list_count(const entity_list_t *list) {
    return list == NULL ? 0 : list->count;
}

static int is_state(const entity_t *ke, const char *want) {
    return ke->state != NULL && strcmp(ke->state, want) == 0;
}

static uint32_t fold_vec(uint32_t h, const vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

/* `mq` as the box the cart keeps: NULL while nil. */
static const vec_t *drag_mouse_of(const background_t *bg) {
    return bg->has_drag_mouse ? &bg->drag_mouse : NULL;
}

/* `mq` holds the value of `rp`, a copy. */
static int drag_mouse_is(const background_t *bg, const vec_t *rp) {
    return bg->has_drag_mouse && &bg->drag_mouse != rp && bg->drag_mouse.x == rp->x && bg->drag_mouse.y == rp->y;
}

/* 0 absent or false, 2 true. */
static uint32_t is_hovered_state(const background_t *bg) {
    return bg->is_hovered ? 2 : 0;
}

static const camera_t *typed_main_camera(void) {
    const camera_t *mo = main_camera_typed();
    assert(mo != NULL);
    return mo;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static void check_int(const char *label, int got, int want) {
    printf("[%s] %d\n", label, got);
    fflush(stdout);
    assert(got == want);
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
}

/* The background is the `bg` of cv.hd whose `hd` and `fa` are both 1. */
static background_t *find_background(int *count) {
    const entity_list_t *hd = entities_by_message("hit_test");
    background_t *found = NULL;
    int i;

    *count = 0;
    for (i = 0; hd != NULL && i < hd->count; i++) {
        const entity_t *e = hd->items[i];
        background_t *bg;

        if (e->class != background_class()) {
            continue;
        }
        bg = (background_t *)e;
        if (bg->archetype->hit_test == p8_fixed_from_int(1) && bg->archetype->layer == p8_fixed_from_int(1)) {
            found = bg;
            (*count)++;
        }
    }
    return found;
}

static background_t *the_background(void) {
    int count;
    background_t *bg = find_background(&count);
    assert(count == 1 && bg != NULL);
    return bg;
}

static vec_t *vec_ref(int x, int y) {
    vec_t *v = malloc(sizeof *v);
    *v = vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y));
    return v;
}

static int mget(int x, int y) {
    return p8_peek(&mem, P8_ADDR_SPRITE_MAP_SHARED + y * P8_MAP_WIDTH + x);
}

/* A fresh `bg`: ke's `me`/`t`, the spec's `fa`/`hd`/`pl`, the four
 * methods, nothing written yet, registered in hv, cv.kj and cv.hd only. */
static void check_fresh_background(const background_t *bg) {
    const entity_class_t *cls = bg->header.class;
    const int_seq_t *pl;
    size_t i;

    assert(is_state(&bg->header, "normal"));
    assert(bg->header.state_time == 0);
    assert(bg->archetype == &background_archetype);
    assert(bg->archetype->layer == p8_fixed_from_int(1));
    assert(bg->archetype->hit_test == p8_fixed_from_int(1));
    pl = bg->archetype->tile_choices;
    assert(pl == &background_tile_choices_seq && pl->count == 15);
    for (i = 0; i < 15; i++) {
        assert(pl->items[i] == want_tile_choices[i]);
    }
    assert(!bg->has_drag_mouse && !bg->is_hovered);
    assert(cls->accept_drop == NULL && cls->states[ENTITY_STATE_NORMAL] == NULL);
    assert(cls->draw != NULL && cls->left_press != NULL && cls->right_press != NULL && cls->drag != NULL);
    assert(cls->drop == NULL);
    assert(entity_list_listed(live_entities(), bg) && entity_list_listed(entities_by_message("draw"), bg) && entity_list_listed(entities_by_message("hit_test"), bg));
    assert(!entity_list_listed(entities_by_message("draw_overlay"), bg) && !entity_list_listed(entities_by_message("new_year"), bg) && !entity_list_listed(entities_by_message("refresh"), bg));
}

static void test_construction(void) {
    background_t *from_start_game;
    background_t *built;
    background_t *generic;

    start_game();
    from_start_game = the_background();
    check_fresh_background(from_start_game);
    built = background_new();
    check_fresh_background(built);
    generic = background_new();
    check_fresh_background(generic);
    assert(entities_by_tag("background") == NULL);
    assert(from_start_game->archetype == built->archetype && built->archetype == generic->archetype);
}

static int in_tile_choices(int tile) {
    int i;
    for (i = 0; i < 15; i++) {
        if (want_tile_choices[i] == tile) {
            return 1;
        }
    }
    return 0;
}

static uint32_t map_hash(int *stars, int *zeros) {
    uint32_t h = 2166136261u;
    int x, y;

    *stars = 0;
    *zeros = 0;
    for (y = 0; y < 32; y++) {
        for (x = 0; x < 32; x++) {
            int tile = mget(x, y);
            assert(tile == 0 || in_tile_choices(tile) || (tile >= 122 && tile <= 126));
            *stars += tile >= 122 && tile <= 126;
            *zeros += tile == 0;
            h = fnv(h, (uint32_t)tile);
        }
    }
    return h;
}

static void test_map(void) {
    int stars;
    int zeros;
    uint32_t first;

    start_game();
    first = map_hash(&stars, &zeros);
    check("map", first, MAP_HASH);
    check_int("map stars", stars, MAP_STARS);
    check_int("map zeros", zeros, MAP_ZEROS);
    (void)background_new();
    check("second map", map_hash(&stars, &zeros), SECOND_MAP_HASH);
}

static void drag(void *bg, vec_t *rp) {
    background_class()->drag(bg, vec_ref(1, 2), rp);
}

/* `hq` with no `mq` yet reads the zero vector; `rdown` keeps a copy of `rp`
 * and hands back the background; each `hq` then moves `mo.v` by the drag
 * delta and replaces `mq`. */
static void test_drag(void) {
    uint32_t h = 2166136261u;
    background_t *bg;
    vec_t *rp;
    down_t out;
    vec_t v;
    int i;

    start_game();
    bg = the_background();
    rp = vec_ref(10, 20);
    drag(bg, rp);
    assert(typed_main_camera()->velocity != NULL);
    v = *typed_main_camera()->velocity;
    assert(v.x == p8_fixed_from_int(-10) && v.y == p8_fixed_from_int(-20));
    assert(drag_mouse_is(bg, rp));
    h = fold_vec(h, typed_main_camera()->velocity);

    rp = vec_ref(30, 40);
    out = background_class()->right_press(&bg->header, vec_ref(5, 6), rp);
    assert(out.stop && (void *)out.held == bg);
    assert(drag_mouse_is(bg, rp));
    h = fold_vec(h, drag_mouse_of(bg));

    for (i = 0; i < 3; i++) {
        rp = vec_ref(33 - 7 * i, 41 + 5 * i * i);
        drag(bg, rp);
        assert(drag_mouse_is(bg, rp));
        h = fold_vec(h, typed_main_camera()->velocity);
        h = fold_vec(h, drag_mouse_of(bg));
    }
    check("drag", h, DRAG_HASH);
}

/* `ldown` builds a `g_` while cg.g_ is empty, an `mz` after. */
static void test_left_press(void) {
    void *bg;
    down_t out;
    int g_before;
    int wormhole_before;

    start_game();
    bg = the_background();
    g_before = entity_list_list_count(entities_by_tag("probe"));
    wormhole_before = entity_list_list_count(entities_by_tag("wormhole"));
    check_int("left_press probe before", g_before, LEFT_PRESS_PROBE_BEFORE);
    check_int("left_press wormhole before", wormhole_before, LEFT_PRESS_WORMHOLE_BEFORE);

    out = background_class()->left_press(bg, vec_ref(1, 1), vec_ref(2, 2));
    assert(out.stop && out.held != NULL);
    assert(out.held->class == probe_class());
    assert(entity_list_listed(entities_by_tag("probe"), out.held));

    out = background_class()->left_press(bg, vec_ref(1, 1), vec_ref(2, 2));
    assert(out.stop && out.held != NULL);
    assert(out.held->class == wormhole_class());
    assert(entity_list_listed(entities_by_tag("wormhole"), out.held));
    assert(is_state(&((const wormhole_t *)out.held)->header, "placing"));

    check_int("left_press probe after", entity_list_list_count(entities_by_tag("probe")), LEFT_PRESS_PROBE_AFTER);
    check_int("left_press wormhole after", entity_list_list_count(entities_by_tag("wormhole")), LEFT_PRESS_WORMHOLE_AFTER);
}

static int in_layer(int layer, const void *e) {
    const ui_hovered_by_layer_t *ic = main_ui()->hovered_by_layer;

    if (ic == NULL) {
        return 0;
    }
    return entity_list_listed(&ic->layers[layer], e);
}

/* Nothing else hovered: the background is `o_`, marked `m_`, in layer 1,
 * and answers both `ldown` and `rdown`; a veil built over it takes `o_`,
 * and the next hover pass unmarks it through `gj`. */
static p8_input_frame_t g_void;

static void test_hover(void) {
    uint32_t h = 2166136261u;
    background_t *bg;
    void *veil;
    entity_t *kv = NULL;
    down_t kn;

    start_game();
    bg = the_background();
    memset(&g_void, 0, sizeof g_void);
    g_void.mouse_x = 64;
    g_void.mouse_y = 100;
    input.frames = &g_void;
    input.frame_count = 1;
    input.current_frame = 0;
    dispatch_context_set_input(&input);
    update_entities();

    ui_update(&mem, main_ui());
    assert((void *)main_ui()->hovered == bg);
    assert(is_hovered_state(bg) == 2);
    assert(in_layer(1, bg));
    h = fnv(h, (uint32_t)p8_peek(&mem, 0x5f80));
    h = fnv(h, is_hovered_state(bg));

    assert(ui_send_to_hovered(main_ui(), SEND_TO_HOVERED_LEFT_PRESS, vec_ref(1, 1), vec_ref(2, 2), NULL, &kv, &kn));
    assert((void *)kv == bg && kn.stop && kn.held != NULL && (void *)kn.held != bg);
    kv = NULL;
    assert(ui_send_to_hovered(main_ui(), SEND_TO_HOVERED_RIGHT_PRESS, vec_ref(1, 1), vec_ref(2, 2), NULL, &kv, &kn));
    assert((void *)kv == bg && kn.stop && (void *)kn.held == bg);
    h = fold_vec(h, drag_mouse_of(bg));

    veil = (void *)modal_backdrop_new(NULL);
    ui_update(&mem, main_ui());
    assert((void *)main_ui()->hovered == veil);
    assert(is_hovered_state(bg) == 0);
    h = fnv(h, (uint32_t)p8_peek(&mem, 0x5f80));
    h = fnv(h, is_hovered_state(bg));
    check("hover", h, HOVER_HASH);
}

static planet_t *first_planet_with_state(const char *me) {
    const entity_list_t *list = entities_by_tag("planet");
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        entity_t *e = list->items[i];

        assert(planet_is_family(e));
        if (is_state(&((const planet_t *)e)->header, me)) {
            return (planet_t *)e;
        }
    }
    return NULL;
}

/* Released over a colonized planet or a slipgate, `jz("gv",held)` reads
 * `s.qu` off the background, which it never has. */
static void test_accept_drop(void) {
    background_t *bg;
    planet_t *ly;
    void *nr;
    planet_keys_t keys;
    down_t r;

    start_game();
    planet_apply_project((planet_t *)entities_by_tag("planet")->items[0], research_project_new(NULL));
    bg = the_background();
    ly = first_planet_with_state("settled");
    assert(ly != NULL);
    r = planet_accept_drop(ly, &bg->header);
    assert(r.stop && r.held == NULL);
    assert(is_state(&ly->header, "settled"));

    keys.planet_id = NULL;
    keys.map_pos = ly->map_pos;
    keys.pos = ly->pos;
    nr = slipgate_new(&keys);
    r = slipgate_accept_drop(nr, &bg->header);
    assert(r.stop && r.held == NULL);
}

/* `bg` alone in cv.kj, drawn by `dc` with the camera `mo` puts at its
 * initial position. */
static void test_draw(void) {
    void *bg;

    reset_world();
    bg = (void *)background_new();
    assert(entity_list_list_count(entities_by_message("draw")) == 1 && entity_list_listed(entities_by_message("draw"), bg));
    p8_gfx_init(&gfx);
    p8_cls(&mem);
    assert(main_camera()->class == camera_class());
    camera_apply(dispatch_context_gfx(), (camera_t *)main_camera(), p8_fixed_from_int(1));
    draw_layers("draw");
    check("draw", screen_hash(), DRAW_HASH);
}

enum { HELD_ABSENT, HELD_PROBE_STRUCT, HELD_WORMHOLE_STRUCT, HELD_OTHER };

static uint32_t left_held_kind(void) {
    const void *held = main_ui()->left_held;

    if (held == NULL) {
        return HELD_ABSENT;
    }
    if (held != NULL &&
        ((const entity_t *)held)->class == probe_class()) {
        return HELD_PROBE_STRUCT;
    }
    if (held != NULL &&
        ((const entity_t *)held)->class == wormhole_class()) {
        return HELD_WORMHOLE_STRUCT;
    }
    return HELD_OTHER;
}

static p8_input_frame_t g_drag[8];

static void drag_frame(int i, int x, int y, int buttons) {
    memset(&g_drag[i], 0, sizeof g_drag[i]);
    g_drag[i].mouse_x = x;
    g_drag[i].mouse_y = y;
    g_drag[i].mouse_buttons = buttons;
}

/* Over empty space, the right button held makes the background `ok.rheld`
 * until released; the left button pressed there holds the `g_` its `ldown`
 * builds. */
static void test_scripted_drags(void) {
    background_t *bg;
    int i;

    start_game();
    bg = the_background();
    for (i = 0; i < 4; i++) {
        drag_frame(i, 64 + 3 * i, 100 - 2 * i, i < 3 ? 2 : 0);
    }
    drag_frame(4, 64, 100, 0);
    drag_frame(5, 64, 100, 1);
    drag_frame(6, 70, 96, 1);
    input.frames = g_drag;
    input.frame_count = 7;
    input.current_frame = 0;
    dispatch_context_set_input(&input);
    for (i = 0; i < 7; i++) {
        int count;

        pico8__update60();
        pico8__draw();
        assert(find_background(&count) == bg && count == 1);
        assert((main_ui()->right_held == bg) == (i < 3));
        if (i == 6) {
            assert(left_held_kind() == HELD_PROBE_STRUCT);
        }
        if (i < 6) {
            p8_input_step(&input);
        }
    }
    p8_input_init(&input);
    dispatch_context_set_input(&input);
}

static int is_a_background_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == background_class();
}

/* Built by `eu()`, `pico8_bg_new` or through its class, a background is a
 * struct with `kj`, `ldown`, `rdown` and `hq` as methods. */
static void test_background_is_a_struct(void) {
    const entity_class_t *cls;
    void *from_start_game;
    void *built;
    void *generic;
    void *veil;

    start_game();
    from_start_game = the_background();
    built = (void *)background_new();
    generic = (void *)background_new();
    assert(is_a_background_struct(from_start_game) && is_a_background_struct(built) && is_a_background_struct(generic));
    cls = ((const entity_t *)built)->class;
    assert(cls->draw != NULL && cls->left_press != NULL && cls->right_press != NULL && cls->drag != NULL);
    assert(cls->states[ENTITY_STATE_NORMAL] == NULL && cls->accept_drop == NULL && cls->draw_overlay == NULL);
    assert(cls->drop == NULL && cls->draw_at == NULL);

    veil = (void *)modal_backdrop_new(NULL);
    assert(((const entity_t *)veil)->class->right_press == NULL);
}

/* A scenario depends on the rnd state: run each in a child forked right
 * after pico8_cart_load(), as a fresh process would start it. */
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

int main(void) {
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_construction);
    in_fresh_child(test_map);
    in_fresh_child(test_drag);
    in_fresh_child(test_left_press);
    in_fresh_child(test_hover);
    in_fresh_child(test_accept_drop);
    in_fresh_child(test_draw);
    in_fresh_child(test_scripted_drags);
    in_fresh_child(test_background_is_a_struct);

    printf("test_background: all tests passed\n");
    return 0;
}
