/* The click dispatcher `ui`, the singleton `ok`, built by its only road
 * (`jl()`, `pico8_ui_new()`, on a game started directly: no events
 * file), read only through the dual-form field API and compared with the
 * archetype `pico8_ui_def` and `pico8_ui_le_shared()`, and
 * driven through the real `dg`/`lj`/`ev`/`jz`/`cj`/`pz`. */

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
#include "p8/audio.h"
#include "game/background.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/help.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "p8/mem.h"
#include "game/menu.h"
#include "game/mouse.h"
#include "game/message_box.h"
#include "game/particle.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/probe.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/score_panel.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "engine/vec.h"
#include "game/wormhole.h"

/* Ticks after the slipway is paid, long enough for the colonizing and
 * paying effects to leave `hv`: `dg` skips the entity after one it takes out
 * (TROU-5). */
#define SETTLE_TICKS 120

#define BUILT_HASH 0x86CE257Cu
#define SHARED_HASH 0x690E4805u
#define FRESH_HASH 0xE44E0786u
#define HOVER_HASH 0xB61A6E40u
#define LAYERS_HASH 0xDBA4EEF8u
#define RIGHT_HASH 0xF02024DCu
#define LEFT_HASH 0xBD9BC256u
#define CLEAR_EPHEMERALS_HASH 0xD215FB81u
#define STALE_HASH 0x5B066E86u
#define ENTITY_STATE_HASH 0xE995C6EFu
#define ENTITY_STATE_TIME_HASH 0xDE09A985u

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const char *const not_listed[] = {"draw", "hit_test", "new_year", "refresh"};
static const char *const tags[] = {"planet", "probe", "structure", "slipway", "wormhole", "message_box", "node", "ascension", "targeting", "slipgate", "background"};

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static int entity_list_rank_in(const entity_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int entity_list_listed(const entity_list_t *list, const void *e) {
    return entity_list_rank_in(list, e) >= 0;
}

static const entity_list_t *ephemerals_of(const ui_t *ok) {
    assert(ok->ephemerals != NULL);
    return ok->ephemerals;
}

static vec_t vec_at(const vec_t *p) {
    assert(p != NULL);
    return *p;
}

static const mouse_t *typed_main_mouse(void) {
    const mouse_t *je = main_mouse_typed();
    assert(je != NULL);
    return je;
}

static const camera_t *typed_main_camera(void) {
    const camera_t *mo = main_camera_typed();
    assert(mo != NULL);
    return mo;
}

static planet_t *as_planet(void *e) {
    assert(planet_is_family(e));
    return e;
}

/* 0 absent or false, 2 true. */
static uint32_t is_hovered_state(const planet_t *jo) {
    return jo->is_hovered ? 2 : 0;
}

static uint32_t fold_vec(uint32_t h, vec_t v) {
    return fnv(fnv(h, (uint32_t)v.x), (uint32_t)v.y);
}

/* Presence, then the rank in `hv` of an entity. */
static uint32_t fold_entity(uint32_t h, const void *ref) {
    h = fnv(h, (uint32_t)(ref != NULL));
    return fnv(h, (uint32_t)entity_list_rank_in(live_entities(), ref));
}

static int layer_count(const ui_t *ok, int mf) {
    return ok->hovered_by_layer == NULL ? 0 : ok->hovered_by_layer->layers[mf].count;
}

static int in_layer(const ui_t *ok, int mf, const void *e) {
    return ok->hovered_by_layer != NULL && entity_list_listed(&ok->hovered_by_layer->layers[mf], e);
}

static uint32_t fold_layers(uint32_t h, const ui_t *ok) {
    int mf;
    for (mf = 0; mf <= 15; mf++) {
        h = fnv(h, (uint32_t)layer_count(ok, mf));
    }
    return h;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
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

/* `ev` drawn alone on a cleared screen is exactly sprite `id` at `je.ra`. */
static uint32_t cursor_is(int id) {
    vec_t at = vec_at(typed_main_mouse()->pos);
    uint32_t drawn;

    p8_cls(&mem);
    ui_draw_overlay(&gfx, &mem, main_ui());
    drawn = screen_hash();
    p8_cls(&mem);
    p8_spr_default(&gfx, &mem, id, p8_fixed_flr_int(at.x), p8_fixed_flr_int(at.y));
    assert(drawn == screen_hash());
    return drawn;
}

static int is_of(const void *e, const entity_class_t *cls) {
    return e != NULL && ((const entity_t *)e)->class == cls;
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static ui_t *start_game(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    cart_init();
    cart_start_game(NULL);
    cart_flush_exit_queue();
    assert(main_ui() != NULL);
    return main_ui();
}

static vec_t pos_of(void *e) {
    return vec_at(as_planet(e)->map_pos);
}

static p8_fixed_t dist2(vec_t a, vec_t b) {
    vec_t d = vec_sub(a, b);
    return p8_fixed_add(p8_fixed_mul(d.x, d.x), p8_fixed_mul(d.y, d.y));
}

static int placed(void *e) {
    return as_planet(e)->map_pos != NULL;
}

/* The planet of `cg.jo` nearest to `qu`, among those with a `p`. */
static void *nearest_to(void *qu) {
    const entity_list_t *jos = entities_by_tag("planet");
    void *re = NULL;
    int i;

    for (i = 0; i < jos->count; i++) {
        void *e = jos->items[i];
        if (e != qu && placed(e) && (re == NULL || dist2(pos_of(e), pos_of(qu)) < dist2(pos_of(re), pos_of(qu)))) {
            re = e;
        }
    }
    assert(re != NULL);
    return re;
}

static p8_input_frame_t g_mouse;

/* One frame's mouse, read by `je` through `dg`. */
static void mouse_at(int x, int y, int buttons) {
    memset(&g_mouse, 0, sizeof g_mouse);
    g_mouse.mouse_x = x;
    g_mouse.mouse_y = y;
    g_mouse.mouse_buttons = buttons;
    input.frames = &g_mouse;
    input.frame_count = 1;
    input.current_frame = 0;
    dispatch_context_set_input(&input);
    update_entities();
}

static void *the_background(void) {
    const entity_list_t *hv = live_entities();
    void *bg = NULL;
    int i;
    for (i = 0; i < hv->count; i++) {
        if (is_of(hv->items[i], background_class())) {
            assert(bg == NULL);
            bg = hv->items[i];
        }
    }
    assert(bg != NULL);
    return bg;
}

/* The game's `ok` as `jl()` and `lt` left it: where it registered, its
 * defaults, the names only read elsewhere, its one method, and the class's
 * `le`/`hw` by identity. */
static void test_built(void) {
    uint32_t hash = 2166136261u;
    ui_t *ok = start_game();
    size_t index;

    hash = fnv(hash, (uint32_t)entity_list_rank_in(live_entities(), ok));
    hash = fnv(hash, (uint32_t)live_entities()->count);
    assert(entity_list_listed(entities_by_message("draw_overlay"), ok));
    hash = fnv(hash, (uint32_t)entity_list_rank_in(entities_by_message("draw_overlay"), ok));
    for (index = 0; index < sizeof not_listed / sizeof not_listed[0]; index++) {
        assert(!entity_list_listed(entities_by_message(not_listed[index]), ok));
    }
    for (index = 0; index < sizeof tags / sizeof tags[0]; index++) {
        assert(!entity_list_listed(entities_by_tag(tags[index]), ok));
    }

    assert(ok->archetype->cursor_sprite == p8_fixed_from_int(58));
    assert(ok->archetype->layer == p8_fixed_from_int(15));
    assert(ok->header.state != NULL && strcmp(ok->header.state, "normal") == 0);
    /* The presence word is constant by the struct's shape, kept so that
     * BUILT_HASH still proves the typed read. */
    hash = fnv(fnv(hash, 1u), (uint32_t)ok->header.state_time);
    assert(ok->mouse_world == NULL && ok->hovered_by_layer == NULL && ok->hovered == NULL && ok->previous_hovered == NULL && ok->left_held == NULL &&
           ok->right_held == NULL);
    assert(ui_class()->draw_overlay != NULL);
    assert(ui_class()->states[ENTITY_STATE_NORMAL] == NULL && ui_class()->left_press == NULL &&
           ui_class()->right_press == NULL && ui_class()->hit_test == NULL);

    assert(ok->ephemerals == ui_ephemerals_shared());
    assert(ok->archetype == &ui_archetype);
    assert(ok->archetype->panel_box.left == p8_fixed_from_int(10) && ok->archetype->panel_box.top == p8_fixed_from_int(10));
    assert(ok->archetype->panel_box.right == p8_fixed_from_int(118) && ok->archetype->panel_box.bottom == p8_fixed_from_int(118));
    assert(ephemerals_of(ok)->count == 0);
    check("built", hash, BUILT_HASH);
}

static void call_galaxy_menu_on_click(int i) {
    galaxy_menu_on_click(i)(NULL);
}

/* A second `ui` shares the class's empty `le` until its own `cj`; `pz`
 * replaces `ok.le`, and a `p` with `ff` empties it. */
static void test_shared(void) {
    uint32_t h = 2166136261u;
    ui_t *ok = start_game();
    ui_t *other = ui_new();
    const entity_list_t *le = ok->ephemerals;

    assert(other != NULL && other != ok);
    assert(other->header.cursor_sprite == &other->archetype->cursor_sprite);
    assert(other->ephemerals == le && le == ui_ephemerals_shared());
    h = fnv(h, (uint32_t)entity_list_rank_in(live_entities(), other));
    h = fnv(h, (uint32_t)entity_list_rank_in(entities_by_message("draw_overlay"), other));

    ui_clear_ephemerals(other);
    assert(other->ephemerals != le && ephemerals_of(other)->count == 0);
    assert(ok->ephemerals == le && ui_ephemerals_shared() == le && le->count == 0);

    call_galaxy_menu_on_click(4);
    assert(ok->ephemerals != le && other->ephemerals != ok->ephemerals);
    h = fnv(h, (uint32_t)ephemerals_of(ok)->count);
    h = fnv(h, (uint32_t)entity_list_rank_in(live_entities(), ephemerals_of(ok)->items[0]));
    assert(ui_ephemerals_shared() == le && le->count == 0);

    (void)help_screen_new();
    assert(ephemerals_of(ok)->count == 0 && ok->ephemerals != le);
    h = fnv(h, (uint32_t)live_entities()->count);
    check("shared", h, SHARED_HASH);
}

/* A fresh game's first `lj`: `qa` = `je.ra` + `mo.ra`, `ic` built, `gj` is
 * `o_`, and `ev` draws sprite 58 at `je.ra`. */
static void test_fresh(void) {
    uint32_t h = 2166136261u;
    ui_t *ok = start_game();
    vec_t want;

    mouse_at(64, 100, 0);
    ui_update(&mem, main_ui());
    want = vec_add(vec_at(typed_main_mouse()->pos), vec_at(typed_main_camera()->pos));
    assert(vec_at(ok->mouse_world).x == want.x && vec_at(ok->mouse_world).y == want.y);
    assert(ok->hovered_by_layer != NULL && ok->hovered != NULL);
    assert(ok->previous_hovered == ok->hovered);
    assert((void *)ok->hovered == the_background());
    h = fold_vec(h, vec_at(ok->mouse_world));
    h = fold_layers(h, ok);
    h = fold_entity(h, ok->hovered);
    h = fnv(h, (uint32_t)p8_peek(&mem, 0x5f80));
    h = fnv(h, cursor_is(58));
    assert(ok->ephemerals == ui_ephemerals_shared());
    check("fresh", h, FRESH_HASH);
}

/* The first planet of `cg.jo` whose screen position is well inside. */
static planet_t *planet_on_screen(int *sx, int *sy) {
    const entity_list_t *jo = entities_by_tag("planet");
    vec_t cam = vec_at(typed_main_camera()->pos);
    int i;

    for (i = 0; jo != NULL && i < jo->count; i++) {
        vec_t at = vec_at(as_planet(jo->items[i])->pos);
        int x = p8_fixed_flr_int(p8_fixed_sub(at.x, cam.x));
        int y = p8_fixed_flr_int(p8_fixed_sub(at.y, cam.y));
        if (x >= 16 && x <= 112 && y >= 16 && y <= 112) {
            *sx = x;
            *sy = y;
            return (planet_t *)jo->items[i];
        }
    }
    assert(0 && "no planet on screen");
    return NULL;
}

/* The mouse over a planet: `o_` is the planet, marked `m_`, its `fg` poked
 * at 0x5f80, `gj` on the next pass; the mouse away unmarks it. */
static void test_hover(void) {
    uint32_t h = 2166136261u;
    ui_t *ok = start_game();
    int x;
    int y;
    planet_t *planet;

    mouse_at(64, 100, 0);
    ui_update(&mem, ok);
    planet = planet_on_screen(&x, &y);
    mouse_at(x, y, 0);
    ui_update(&mem, ok);
    assert((void *)ok->hovered == planet);
    assert(is_hovered_state(planet) == 2);
    /* A planet has no `fg`: 0x5f80 is poked with 0. */
    assert(p8_peek(&mem, 0x5f80) == 0);
    assert(in_layer(ok, 6, planet) && in_layer(ok, 1, the_background()));
    h = fnv(h, (uint32_t)entity_list_rank_in(live_entities(), planet));
    h = fnv(h, (uint32_t)p8_peek(&mem, 0x5f80));
    h = fold_layers(h, ok);

    ui_update(&mem, ok);
    assert((void *)ok->previous_hovered == planet && (void *)ok->hovered == planet);
    assert(is_hovered_state(planet) == 2);
    h = fnv(h, cursor_is(58));

    mouse_at(x + 40 > 127 ? x - 40 : x + 40, y, 0);
    ui_update(&mem, ok);
    assert((void *)ok->hovered != planet);
    assert(is_hovered_state(planet) == 0);
    h = fold_entity(h, ok->hovered);
    h = fnv(h, (uint32_t)p8_peek(&mem, 0x5f80));
    check("hover", h, HOVER_HASH);
}

/* `jz` walks the layers from 15 down: over a planet, `hd` answers from the
 * planet (layer 6) before the background (layer 1); `rdown` only from the
 * background, which answers itself. */
static void test_layers(void) {
    uint32_t h = 2166136261u;
    ui_t *ok = start_game();
    entity_t *kv = NULL;
    down_t kn;
    vec_t *qa;
    vec_t *ra;
    planet_t *planet;
    int x;
    int y;

    mouse_at(64, 100, 0);
    ui_update(&mem, ok);
    planet = planet_on_screen(&x, &y);
    mouse_at(x, y, 0);
    ui_update(&mem, ok);
    qa = ok->mouse_world;
    ra = typed_main_mouse()->pos;
    assert(qa != NULL && ra != NULL);

    entity_set_state(&planet->header, "known");
    assert(ui_send_to_hovered(main_ui(), SEND_TO_HOVERED_LEFT_PRESS, qa, ra, NULL, &kv, &kn));
    assert((void *)kv == planet && kn.stop && kn.held == NULL);
    h = fnv(h, (uint32_t)entity_list_rank_in(live_entities(), kv));
    kv = NULL;
    assert(ui_send_to_hovered(main_ui(), SEND_TO_HOVERED_RIGHT_PRESS, qa, ra, NULL, &kv, &kn));
    assert((void *)kv == the_background() && kn.stop && (void *)kn.held == the_background());
    h = fnv(h, (uint32_t)entity_list_rank_in(live_entities(), kv));
    check("layers", h, LAYERS_HASH);
}

/* The right button held scrolls through the background's `rdown`/`hq`:
 * `rheld` is the background and the cursor is sprite 9; released, `rheld`
 * goes. */
static void test_right(void) {
    uint32_t h = 2166136261u;
    ui_t *ok = start_game();
    void *bg;

    mouse_at(64, 100, 0);
    ui_update(&mem, ok);
    bg = the_background();
    mouse_at(64, 100, 2);
    ui_update(&mem, ok);
    assert((void *)ok->right_held == bg && ok->left_held == NULL);
    h = fnv(h, cursor_is(9));

    mouse_at(70, 90, 2);
    ui_update(&mem, ok);
    assert((void *)ok->right_held == bg);
    h = fold_vec(h, vec_at(typed_main_camera()->pos));
    h = fnv(h, cursor_is(9));

    mouse_at(70, 90, 0);
    ui_update(&mem, ok);
    assert(ok->right_held == NULL && ok->left_held == NULL);
    h = fold_vec(h, vec_at(typed_main_camera()->pos));
    h = fnv(h, cursor_is(58));
    check("right", h, RIGHT_HASH);
}

static int slipways_count(void *planet) {
    const entity_list_t *oe = as_planet(planet)->slipways;
    return oe == NULL ? 0 : oe->count;
}

/* Where `e` is on screen under `mo`'s camera. */
static void screen_of(void *e, int *x, int *y) {
    vec_t at = vec_sub(vec_at(as_planet(e)->pos), vec_at(typed_main_camera()->pos));

    *x = p8_fixed_flr_int(at.x);
    *y = p8_fixed_flr_int(at.y);
}

static int on_screen(void *e) {
    int x;
    int y;

    screen_of(e, &x, &y);
    return x >= 8 && x < 120 && y >= 8 && y < 120;
}

/* A started game's first planet on screen whose nearest one is on screen
 * too, both colonized by a bare project, then the mouse by hand: the left
 * button on the one holds a new slipway `jc` in `lheld`; released over the
 * other, `ey` gets that planet and `lheld` goes. */
static void test_left(void) {
    uint32_t hash = 2166136261u;
    const entity_list_t *jos;
    ui_t *dispatcher;
    void *held;
    void *origin = NULL;
    void *target;
    void *other = NULL;
    int pixel_x;
    int pixel_y;
    int index;

    dispatcher = start_game();
    jos = entities_by_tag("planet");
    for (index = 0; index < jos->count && other == NULL; index++) {
        void *e = jos->items[index];
        if (on_screen(e) && on_screen(nearest_to(e))) {
            origin = e;
            other = nearest_to(e);
        }
    }
    assert(other != NULL);
    planet_apply_project(origin, research_project_new(NULL));
    planet_apply_project(other, research_project_new(NULL));
    for (index = 0; index < SETTLE_TICKS; index++) {
        update_entities();
    }

    screen_of(origin, &pixel_x, &pixel_y);
    mouse_at(pixel_x, pixel_y, 0);
    ui_update(&mem, dispatcher);
    mouse_at(pixel_x, pixel_y, 1);
    ui_update(&mem, dispatcher);
    held = dispatcher->left_held;
    assert(held != NULL && is_of(held, slipway_class()) && dispatcher->right_held == NULL);
    assert((void *)((const slipway_t *)held)->start == origin && slipways_count(origin) == 0);
    hash = fnv(hash, (uint32_t)entity_list_rank_in(live_entities(), held));
    hash = fnv(hash, (uint32_t)entity_list_rank_in(live_entities(), origin));
    hash = fnv(hash, cursor_is(58));

    screen_of(other, &pixel_x, &pixel_y);
    mouse_at(pixel_x, pixel_y, 1);
    ui_update(&mem, dispatcher);
    assert((void *)dispatcher->left_held == held);
    target = dispatcher->hovered;
    assert(target == other);
    hash = fnv(hash, (uint32_t)entity_list_rank_in(live_entities(), target));

    mouse_at(pixel_x, pixel_y, 0);
    ui_update(&mem, dispatcher);
    assert(dispatcher->left_held == NULL && dispatcher->right_held == NULL);
    assert(slipways_count(origin) == 1 && slipways_count(target) == 1);
    hash = fnv(hash, (uint32_t)entities_by_tag("slipway")->count);
    check("left", hash, LEFT_HASH);
}

/* `cj` unregisters every entity of the old `le` and starts a new list. */
static void test_clear_ephemerals(void) {
    uint32_t h = 2166136261u;
    ui_t *ok = start_game();
    const entity_list_t *le;
    int count;
    int i;

    call_galaxy_menu_on_click(4);
    le = ephemerals_of(ok);
    count = le->count;
    assert(count > 0);
    for (i = 0; i < count; i++) {
        assert(entity_list_listed(live_entities(), le->items[i]));
        h = fnv(h, (uint32_t)entity_list_rank_in(live_entities(), le->items[i]));
    }
    h = fnv(h, (uint32_t)live_entities()->count);
    ui_clear_ephemerals(main_ui());
    assert(ok->ephemerals != le && ephemerals_of(ok)->count == 0 && ok->ephemerals != ui_ephemerals_shared());
    for (i = 0; i < count; i++) {
        assert(!entity_list_listed(live_entities(), le->items[i]));
    }
    h = fnv(h, (uint32_t)live_entities()->count);
    check("clear_ephemerals", h, CLEAR_EPHEMERALS_HASH);
}

/* On the title screen, a click on a difficulty button runs `eu()` from
 * inside `lj`: `jl()` rebuilds `ok`, and the end of `lj` still runs on the
 * old one, whose `cj` and `gj` land on it, not on the new `ok`. */
static void test_stale(void) {
    uint32_t h = 2166136261u;
    ui_t *old;
    void *button;
    const entity_list_t *le;

    p8_input_init(&input);
    dispatch_context_set_input(&input);
    cart_init();
    old = main_ui();
    mouse_at(76, 71, 0);
    ui_update(&mem, old);
    button = old->hovered;
    le = ephemerals_of(old);
    assert(button != NULL && is_of(button, button_class()) && le->count == 0);
    h = fnv(h, (uint32_t)entity_list_rank_in(live_entities(), button));

    mouse_at(76, 71, 1);
    ui_update(&mem, old);
    assert(main_ui() != old && main_ui() != NULL);
    assert(old->previous_hovered == old->hovered && old->previous_hovered != NULL && is_of(old->previous_hovered, button_class()));
    h = fnv(h, (uint32_t)entity_list_rank_in(live_entities(), old->previous_hovered));
    assert(old->ephemerals != le && ephemerals_of(old)->count == 0);
    assert(main_ui()->previous_hovered == NULL && main_ui()->mouse_world == NULL);
    h = fnv(h, (uint32_t)ephemerals_of(main_ui())->count);
    h = fnv(h, (uint32_t)(main_ui()->ephemerals == ui_ephemerals_shared()));
    h = fnv(h, (uint32_t)entity_list_rank_in(live_entities(), main_ui()));
    h = fnv(h, (uint32_t)live_entities()->count);
    check("stale", h, STALE_HASH);
}

/* With a panel up, `lj` hands the left click to an entity of `le`: the
 * third row of `ha` runs its `my`, which flips `ih`. */
static void test_click_reaches_a_row_of_ephemerals(void) {
    ui_t *ok = start_game();
    void *row;
    int ih;
    int x;
    int y;
    int hx = -1;
    int hy = -1;

    call_galaxy_menu_on_click(4);
    assert(ephemerals_of(ok)->count == 4);
    row = ephemerals_of(ok)->items[2];
    for (y = 0; y < 128 && hx < 0; y += 2) {
        for (x = 0; x < 128 && hx < 0; x += 2) {
            mouse_at(x, y, 0);
            ui_update(&mem, ok);
            if ((void *)ok->hovered == row) {
                hx = x;
                hy = y;
            }
        }
    }
    assert(hx >= 0);
    ih = p8_sfx_muted();
    mouse_at(hx, hy, 1);
    ui_update(&mem, ok);
    assert(p8_sfx_muted() == !ih);
    p8_set_sfx_muted(ih);
}

/* `pico8_ui_init()` empties the shared `le`. */
static void test_init_empties_shared_ephemerals(void) {
    static entity_t mark;

    (void)start_game();
    entity_list_add(ui_ephemerals_shared(), &mark);
    assert(ui_ephemerals_shared()->count == 1);
    ui_init();
    assert(ui_ephemerals_shared()->count == 0);
}

static void must_abort(void (*write)(void *), void *e) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        write(e);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

static void handle_button_on_x(void *e) {
    ui_handle_button(e, "x");
}

/* The game's `ok` is a struct whose descriptor holds only `ev`; it takes
 * no construction key and no button but `l`/`r`. Every
 * new `ui` starts on the class's generated defaults and its shared `le`. */
static void test_ui_is_a_struct(void) {
    const entity_class_t *cls = ui_class();
    ui_t *ok = start_game();
    ui_t *other;
    int j;

    assert(is_of(ok, cls));
    assert(cls->draw_overlay != NULL && cls->draw == NULL && cls->hit_test == NULL && cls->new_year == NULL && cls->refresh == NULL &&
           cls->left_press == NULL && cls->right_press == NULL && cls->accept_drop == NULL && cls->drag == NULL && cls->drop == NULL &&
           cls->attach_slipway == NULL && cls->offer_trade == NULL && cls->draw_at == NULL && cls->spawn_particle == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(ui_ready());
    assert(ok->archetype == &ui_archetype);
    assert(ui_panel_box() == &ok->archetype->panel_box);
    assert(ok->ephemerals == ui_ephemerals_shared() && ok->ephemerals->count == 0);
    other = ui_new();
    assert(is_of(other, cls) && other->ephemerals == ui_ephemerals_shared() && other->archetype == ok->archetype);

    must_abort(handle_button_on_x, ok);
}

typedef struct {
    entity_t header;
} giver_t;

static entity_t *g_given;
static int g_giver_hovered = 1;

static down_t giver_down(entity_t *self, vec_t *qa, vec_t *ra) {
    down_t out;

    (void)self;
    (void)qa;
    (void)ra;
    out.stop = g_given != NULL;
    out.held = g_given;
    return out;
}

static int giver_hit_test(entity_t *self, const vec_t *qa, const vec_t *ra) {
    (void)self;
    (void)qa;
    (void)ra;
    return g_giver_hovered;
}

static const entity_class_t g_giver_class = {
    .hit_test = giver_hit_test,
    .left_press = giver_down,
    .right_press = giver_down,
};

static const p8_fixed_t g_giver_layer = 0xf0000;
static int g_giver_is_hovered;
static giver_t g_giver = {{.class = &g_giver_class, .layer = &g_giver_layer, .is_hovered = &g_giver_is_hovered}};

static void press(void *ok, int buttons) {
    mouse_at(64, 100, 0);
    ui_update(&mem, ok);
    mouse_at(64, 100, buttons);
    ui_update(&mem, ok);
}

static int g_hold_buttons;

static void hold_given(void *ok) {
    press(ok, g_hold_buttons);
}

static void test_ui_held_stops_off_row(void) {
    ui_t *ok = start_game();
    void *left[8];
    void *right[6];
    size_t i;

    entity_register(&g_giver.header);
    left[0] = protostar_new(NULL);
    left[1] = burst_new(NULL);
    left[2] = entities_by_tag("planet")->items[0];
    left[3] = the_background();
    left[4] = help_screen_new();
    left[5] = button_new(NULL);
    left[6] = structure_icon_new(NULL);
    left[7] = wormhole_too_far_target_wrapper();
    right[0] = slipway_new(NULL);
    right[1] = probe_new(NULL);
    right[2] = wormhole_new();
    right[3] = structure_new(NULL);
    right[4] = left[2];
    right[5] = left[4];

    g_hold_buttons = 1;
    for (i = 0; i < 8; i++) {
        g_given = left[i];
        must_abort(hold_given, ok);
    }
    g_hold_buttons = 2;
    for (i = 0; i < 6; i++) {
        g_given = right[i];
        must_abort(hold_given, ok);
    }
}

/* `ks` takes a struct or NULL, and does nothing on NULL. */
static void test_entity_closed(void) {
    (void)start_game();
    entity_set_state(NULL, "normal");
}

/* Every entity struct begins with its header. */
static entity_t *entity_of(void *e) {
    assert(e != NULL);
    return e;
}

static uint32_t fold_string(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0u);
}

static int identity_of(const char *s) {
    if (s == background_archetype.state) {
        return 1;
    }
    if (s == planet_archetype.state) {
        return 2;
    }
    if (s == structure_archetype.state) {
        return 3;
    }
    if (s == protostar_archetype.state) {
        return 4;
    }
    return 0;
}

static uint32_t fold_set_state_back(uint32_t h, void *e) {
    assert(entity_of(e)->state_time == 0);
    return fnv(h, (uint32_t)entity_of(e)->state_time);
}

/* The component a host stores in itself: not shared with `other`, of the
 * same class, and inside the host's own `size` bytes. */
static void assert_own_emitter(entity_t *self, entity_t *other, size_t size) {
    entity_t *e = self;
    entity_t *o = other;
    const char *at = (const char *)e->emitter;

    assert(e->class == o->class);
    assert((const void *)at != (const void *)o->emitter);
    assert(at >= (const char *)self && at < (const char *)self + size);
}

static void assert_own_probe(entity_t *self, entity_t *other, size_t size) {
    entity_t *e = self;
    entity_t *o = other;
    const char *at = (const char *)e->targeting;

    assert(e->class == o->class);
    assert((const void *)at != (const void *)o->targeting);
    assert(at >= (const char *)self && at < (const char *)self + size);
}

/* A second help, built once `first` gave back its list: the qy_fv sort
 * holds one help's list at a time. */
static entity_t *help_alone(entity_t *first) {
    static entity_t *previous;
    entity_t *fresh;

    emitter_release_particles(first->emitter);
    if (previous != NULL) {
        emitter_release_particles(previous->emitter);
    }
    fresh = &help_screen_new()->header;
    previous = fresh;
    return fresh;
}

/* Which of the 24 classes carry a particle emitter (exactly hj, pm, ga, mx,
 * g_, fv) and a probe (qh's line, g_, fv, jc, mz), each in its own storage. */
static void test_components_carried(void) {
    static const int emitter[24] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
    static const int probe[24] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1};
    void *entities[24];
    size_t index;

    (void)start_game();
    entities[0] = background_new();
    entities[1] = mouse_new();
    entities[2] = message_box_new(NULL);
    entities[3] = toast_new(NULL);
    entities[4] = planet_new(NULL);
    entities[5] = structure_new(NULL);
    entities[6] = trading_hub_new(NULL);
    entities[7] = processor_new(NULL);
    entities[8] = ascension_new(NULL);
    entities[9] = protostar_new(NULL);
    entities[10] = burst_new(NULL);
    entities[11] = slipgate_new(NULL);
    entities[12] = infraspace_gate_new(NULL);
    entities[13] = synthesizer_new(NULL);
    entities[14] = probe_new(NULL);
    entities[15] = help_screen_new();
    entities[16] = structure_icon_new(NULL);
    entities[17] = score_panel_new();
    entities[18] = camera_new();
    entities[19] = slipway_new(NULL);
    entities[20] = ui_new();
    entities[21] = button_new(NULL);
    entities[22] = modal_backdrop_new(NULL);
    entities[23] = wormhole_new();
    for (index = 0; index < 24; index++) {
        const entity_t *header = entities[index];

        assert((header->emitter != NULL) == emitter[index]);
        assert((header->targeting != NULL) == probe[index]);
    }

    assert_own_emitter(entities[8], &ascension_new(NULL)->header, sizeof(planet_t));
    assert_own_emitter(entities[9], &protostar_new(NULL)->header, sizeof(planet_t));
    assert_own_emitter(entities[10], &burst_new(NULL)->header, sizeof(planet_t));
    assert_own_emitter(entities[13], &synthesizer_new(NULL)->header, sizeof(planet_t));
    assert_own_emitter(entities[14], &probe_new(NULL)->header, sizeof(probe_t));
    assert_own_emitter(entities[15], help_alone(entities[15]), sizeof(probe_t));
    assert_own_probe(entities[5], &structure_new(NULL)->header, sizeof(planet_t));
    assert_own_probe(entities[6], &trading_hub_new(NULL)->header, sizeof(planet_t));
    assert_own_probe(entities[7], &processor_new(NULL)->header, sizeof(planet_t));
    assert_own_probe(entities[8], &ascension_new(NULL)->header, sizeof(planet_t));
    assert_own_probe(entities[9], &protostar_new(NULL)->header, sizeof(planet_t));
    assert_own_probe(entities[11], &slipgate_new(NULL)->header, sizeof(planet_t));
    assert_own_probe(entities[12], &infraspace_gate_new(NULL)->header, sizeof(planet_t));
    assert_own_probe(entities[13], &synthesizer_new(NULL)->header, sizeof(planet_t));
    assert_own_probe(entities[14], &probe_new(NULL)->header, sizeof(probe_t));
    assert_own_probe(entities[15], help_alone(entities[15]), sizeof(probe_t));
    assert_own_probe(entities[19], &slipway_new(NULL)->header, sizeof(slipway_t));
    assert_own_probe(entities[23], &wormhole_new()->header, sizeof(wormhole_t));
}

/* The kind words the frozen hashes fold before a value: any other value
 * moves them. */
enum { FIXED_KIND = 0, STRING_KIND = 1 };

static void test_entity_state_by_name(void) {
    static const char *const want[24] = {"normal", "normal",      "normal", "normal", "unknown", "placing", "placing", "placing",
                                         "placing", "settled",      "unknown", "placing", "placing", "placing", "normal", "normal",
                                         "normal", "normal",      "normal", "placing", "normal", "normal", "normal", "placing"};
    static const int identity[24] = {1, 1, 1, 1, 2, 3, 3, 3, 3, 4, 2, 3, 3, 3, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0};
    char state_name[] = "vanishing";
    void *entities[24];
    const entity_list_t *live;
    const char *state;
    uint32_t hash = 2166136261u;
    size_t index;

    (void)start_game();
    live = live_entities();
    assert(live != NULL);
    hash = fnv(hash, (uint32_t)live->count);
    /* The kind word is constant by the struct's shape (`me` is a string or
     * absent), kept so that KE_ME_HASH still proves the typed reads. */
    for (index = 0; index < (size_t)live->count; index++) {
        state = entity_of(live->items[index])->state;
        if (state != NULL) {
            hash = fold_string(fnv(fnv(hash, 1u), (uint32_t)STRING_KIND), state);
        } else {
            hash = fnv(hash, 0u);
        }
    }

    entities[0] = background_new();
    entities[1] = mouse_new();
    entities[2] = message_box_new(NULL);
    entities[3] = toast_new(NULL);
    entities[4] = planet_new(NULL);
    entities[5] = structure_new(NULL);
    entities[6] = trading_hub_new(NULL);
    entities[7] = processor_new(NULL);
    entities[8] = ascension_new(NULL);
    entities[9] = protostar_new(NULL);
    entities[10] = burst_new(NULL);
    entities[11] = slipgate_new(NULL);
    entities[12] = infraspace_gate_new(NULL);
    entities[13] = synthesizer_new(NULL);
    entities[14] = probe_new(NULL);
    entities[15] = help_screen_new();
    entities[16] = structure_icon_new(NULL);
    entities[17] = score_panel_new();
    entities[18] = camera_new();
    entities[19] = slipway_new(NULL);
    entities[20] = ui_new();
    entities[21] = button_new(NULL);
    entities[22] = modal_backdrop_new(NULL);
    entities[23] = wormhole_new();
    for (index = 0; index < 24; index++) {
        state = entity_of(entities[index])->state;
        assert(state != NULL);
        assert(strcmp(state, want[index]) == 0);
        assert(identity_of(state) == identity[index]);
        hash = fold_string(fnv(hash, (uint32_t)STRING_KIND), state);
        hash = fnv(hash, (uint32_t)identity_of(state));
    }

    /* The six strangers the reader refused, constant by the struct's shape. */
    for (index = 0; index < 6; index++) {
        hash = fnv(hash, 0u);
    }

    entity_set_state(entities[0], state_name);
    state = entity_of(entities[0])->state;
    assert(state == state_name);
    hash = fold_set_state_back(fold_string(fnv(hash, 1u), state), entities[0]);
    entity_set_state(entities[0], NULL);
    assert(entity_of(entities[0])->state == NULL);
    hash = fold_set_state_back(fnv(hash, 0u), entities[0]);
    entity_set_state(entities[0], "normal");
    state = entity_of(entities[0])->state;
    assert(state != NULL && strcmp(state, "normal") == 0);
    hash = fold_set_state_back(fold_string(fnv(hash, 1u), state), entities[0]);

    entity_set_state(entities[9], state_name);
    state = entity_of(entities[9])->state;
    assert(state == state_name);
    hash = fold_set_state_back(fold_string(fnv(hash, 1u), state), entities[9]);

    check("entity_state", hash, ENTITY_STATE_HASH);
}

/* The kind word is constant by the struct's shape (`t` is always a fixed),
 * kept so that KE_T_HASH still proves the typed reads. */
static uint32_t fold_state_time(uint32_t h, void *e) {
    return fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)entity_of(e)->state_time);
}

static uint32_t fold_live_entities_state_time(uint32_t h) {
    const entity_list_t *hv = live_entities();
    int i;

    assert(hv != NULL);
    h = fnv(h, (uint32_t)hv->count);
    for (i = 0; i < hv->count; i++) {
        h = fold_state_time(h, hv->items[i]);
    }
    return h;
}

static void assert_state_time(void *e, int want) {
    assert(entity_of(e)->state_time == p8_fixed_from_int(want));
}

static void test_entity_state_time_by_name(void) {
    void *entities[24];
    probe_keys_t keyed_keys = {0};
    probe_keys_t zero_keys = {0};
    probe_t *keyed;
    probe_t *zero;
    uint32_t hash = 2166136261u;
    size_t index;

    (void)start_game();
    hash = fold_live_entities_state_time(hash);
    update_entities();
    hash = fold_live_entities_state_time(hash);

    entities[0] = background_new();
    entities[1] = mouse_new();
    entities[2] = message_box_new(NULL);
    entities[3] = toast_new(NULL);
    entities[4] = planet_new(NULL);
    entities[5] = structure_new(NULL);
    entities[6] = trading_hub_new(NULL);
    entities[7] = processor_new(NULL);
    entities[8] = ascension_new(NULL);
    entities[9] = protostar_new(NULL);
    entities[10] = burst_new(NULL);
    entities[11] = slipgate_new(NULL);
    entities[12] = infraspace_gate_new(NULL);
    entities[13] = synthesizer_new(NULL);
    entities[14] = probe_new(NULL);
    entities[15] = help_screen_new();
    entities[16] = structure_icon_new(NULL);
    entities[17] = score_panel_new();
    entities[18] = camera_new();
    entities[19] = slipway_new(NULL);
    entities[20] = ui_new();
    entities[21] = button_new(NULL);
    entities[22] = modal_backdrop_new(NULL);
    entities[23] = wormhole_new();
    for (index = 0; index < 24; index++) {
        assert_state_time(entities[index], 0);
        hash = fold_state_time(hash, entities[index]);
    }

    /* The six strangers the reader refused, constant by the struct's shape. */
    for (index = 0; index < 6; index++) {
        hash = fnv(hash, 0u);
    }

    keyed_keys.has_angle_slices = 1;
    keyed_keys.angle_slices = p8_fixed_from_int(7);
    keyed_keys.state_time = p8_fixed_from_int(3);
    keyed_keys.has_base_radius = 1;
    keyed_keys.base_radius = p8_fixed_from_int(10);
    keyed_keys.has_speed_factor = 1;
    keyed_keys.speed_factor = p8_fixed_from_int(1);
    keyed = probe_new(&keyed_keys);
    assert_state_time(keyed, 3);
    assert(keyed->emitter->angle_slices == p8_fixed_from_int(7));
    assert(keyed->emitter->base_radius == p8_fixed_from_int(10));
    assert(keyed->emitter->speed_factor == p8_fixed_from_int(1));
    hash = fold_state_time(hash, keyed);

    zero_keys.state_time = 0;
    zero = probe_new(&zero_keys);
    assert_state_time(zero, 0);
    assert(zero->emitter->angle_slices == probe_archetype.emitter.angle_slices);
    assert(zero->emitter->base_radius == probe_archetype.emitter.base_radius);
    assert(zero->emitter->speed_factor == probe_archetype.emitter.speed_factor);
    hash = fold_state_time(hash, zero);

    entity_set_state(&keyed->header, "vanishing");
    assert_state_time(keyed, 0);
    assert(keyed->header.state != NULL && strcmp(keyed->header.state, "vanishing") == 0);
    hash = fold_string(fold_state_time(hash, keyed), keyed->header.state);

    entity_set_state(entities[9], "vanishing");
    assert_state_time(entities[9], 0);
    hash = fold_state_time(hash, entities[9]);

    check("entity_state_time", hash, ENTITY_STATE_TIME_HASH);
}

static void test_probe_t_key_stops_off_row(void) {
    probe_keys_t keys = {0};
    void *made;

    (void)start_game();
    keys.state_time = 0;
    made = probe_new(&keys);
    assert_state_time(made, 0);
}

/* Each scenario runs in a child forked right after pico8_cart_load(), as a
 * fresh process would start it: the rnd state is the same for all. */
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

    cart_load();
    in_fresh_child(test_built);
    in_fresh_child(test_shared);
    in_fresh_child(test_fresh);
    in_fresh_child(test_hover);
    in_fresh_child(test_layers);
    in_fresh_child(test_right);
    in_fresh_child(test_left);
    in_fresh_child(test_clear_ephemerals);
    in_fresh_child(test_stale);
    in_fresh_child(test_click_reaches_a_row_of_ephemerals);
    in_fresh_child(test_init_empties_shared_ephemerals);
    in_fresh_child(test_ui_is_a_struct);
    in_fresh_child(test_ui_held_stops_off_row);
    in_fresh_child(test_entity_closed);
    in_fresh_child(test_entity_state_by_name);
    in_fresh_child(test_components_carried);
    in_fresh_child(test_entity_state_time_by_name);
    in_fresh_child(test_probe_t_key_stops_off_row);

    printf("test_ui: all tests passed\n");
    return 0;
}
