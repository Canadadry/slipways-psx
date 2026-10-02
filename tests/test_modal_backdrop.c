/* The click veil `fq`, built bare (`pj:ph`) and with a `my` (`ix[3]` with
 * `de`), lived through the real `dg`, its `ldown` and the hover loop of
 * `ok`, in a game built directly: no events file. */

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
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/score_panel_display.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "game/ui_zone.h"

#define ON_CLICK_TRUE_TICKS 6
#define ON_CLICK_TRUE_SEQUENCE_HASH 0xA6A43C27u
#define BARE_SEQUENCE_HASH 0x52BE382Au
#define HOVER_SEQUENCE_HASH 0x689321F9u
#define HOVER_CURSOR_HASH 0x04C40AB9u

#define LEFT_PRESS_TICK 5
#define IDLE_TICKS 30

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

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

/* The kind words the frozen hashes fold before a value: any other value
 * moves them. */
enum { FIXED_KIND = 0, BOOL_KIND = 2 };

static uint32_t fold_bool_member(uint32_t h, int has, int b) {
    if (!has) {
        return fnv(h, 0xDEADu);
    }
    return fnv(fnv(h, (uint32_t)BOOL_KIND), (uint32_t)b);
}

/* The kind words are constant by the struct's shape, kept so that the
 * sequence hashes still prove the typed reads. */
static uint32_t fold_veil(uint32_t h, const modal_backdrop_t *e) {
    h = fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)e->header.state_time);
    h = fold_bool_member(h, (e->is_dead != 0), e->is_dead);
    h = fold_bool_member(h, (e->is_hovered != 0), e->is_hovered);
    h = fnv(h, (uint32_t)entity_list_listed(live_entities(), e));
    return fnv(h, (uint32_t)entity_list_listed(entities_by_message("hit_test"), e));
}

static const void *main_ui_hovered(void) {
    return main_ui()->hovered;
}

static int g_on_click_calls = 0;
static int g_on_click_answer = 1;

static int counting_on_click(modal_backdrop_t *self) {
    (void)self;
    g_on_click_calls++;
    return g_on_click_answer;
}

static modal_backdrop_t *veil_with_on_click(int answer) {
    modal_backdrop_keys_t keys = {0};

    g_on_click_calls = 0;
    g_on_click_answer = answer;
    keys.on_click = counting_on_click;
    return modal_backdrop_new(&keys);
}

static int left_press(void *veil) {
    down_t out = modal_backdrop_class()->left_press(veil, NULL, NULL);

    assert(out.held == NULL);
    return out.stop;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

/* `fq(de)`: the click at tick LDOWN_TICK stores `my()` in `p_`, and the
 * next `dg` takes the veil out. */
static void test_left_press_with_a_true_on_click_removes_the_veil(void) {
    uint32_t h = 2166136261u;
    modal_backdrop_t *veil;
    int ticks = 0;

    reset_world();
    cart_flush_exit_queue();
    veil = veil_with_on_click(1);
    h = fold_veil(h, veil);
    while (entity_list_listed(live_entities(), veil)) {
        if (ticks == LEFT_PRESS_TICK) {
            assert(left_press(veil));
            assert(g_on_click_calls == 1);
            h = fold_veil(h, veil);
        }
        update_entities();
        ticks++;
        h = fold_veil(h, veil);
        assert(ticks < 1000);
    }
    assert(g_on_click_calls == 1);
    assert(!entity_list_listed(entities_by_message("hit_test"), veil));
    printf("[on_click true] ticks=%d\n", ticks);
    fflush(stdout);
    assert(ticks == ON_CLICK_TRUE_TICKS);
    check("on_click true", h, ON_CLICK_TRUE_SEQUENCE_HASH);
}

/* `fq()`: a click is swallowed, `p_` stays nil, the veil stays. */
static void test_bare_veil_swallows_the_click_and_stays(void) {
    uint32_t h = 2166136261u;
    modal_backdrop_t *veil;
    int i;

    reset_world();
    cart_flush_exit_queue();
    veil = modal_backdrop_new(NULL);
    for (i = 0; i < IDLE_TICKS; i++) {
        if (i == LEFT_PRESS_TICK) {
            assert(left_press(veil));
            assert(!veil->is_dead);
        }
        update_entities();
        h = fold_veil(h, veil);
    }
    assert(entity_list_listed(live_entities(), veil) && entity_list_listed(entities_by_message("hit_test"), veil));
    check("bare", h, BARE_SEQUENCE_HASH);
}

/* A `my` answering false leaves `p_` false, which reads as nil: the veil
 * stays, as a bare one. */
static void test_false_on_click_keeps_the_veil(void) {
    uint32_t h = 2166136261u;
    modal_backdrop_t *veil;
    int i;

    reset_world();
    cart_flush_exit_queue();
    veil = veil_with_on_click(0);
    for (i = 0; i < IDLE_TICKS; i++) {
        if (i == LEFT_PRESS_TICK) {
            assert(left_press(veil));
            assert(g_on_click_calls == 1);
            assert(!veil->is_dead);
        }
        update_entities();
        h = fold_veil(h, veil);
    }
    assert(entity_list_listed(live_entities(), veil) && entity_list_listed(entities_by_message("hit_test"), veil));
    check("on_click false", h, BARE_SEQUENCE_HASH);
}

static int in_layer(int layer, const void *e) {
    const ui_hovered_by_layer_t *ic = main_ui()->hovered_by_layer;

    if (ic == NULL) {
        return 0;
    }
    return entity_list_listed(&ic->layers[layer], e);
}

/* A started game, then `fq()`: `ok`'s hover loop makes it `o_` (hd=1 is
 * always a hit) in layer 14, marks it `m_`, takes `ldown` but not `rdown`,
 * and, once the veil is gone, unmarks it through `gj`. */
static void test_hover_loop_on_the_veil(void) {
    uint32_t h = 2166136261u;
    down_t kn;
    entity_t *kv = NULL;
    modal_backdrop_t *veil;

    cart_init();
    cart_start_game(NULL);
    veil = modal_backdrop_new(NULL);

    ui_update(&mem, main_ui());
    assert(main_ui_hovered() == veil);
    assert(in_layer(14, veil) && !in_layer(15, veil));
    assert(veil->is_hovered);
    h = fnv(h, (uint32_t)p8_peek(&mem, 0x5f80));
    h = fold_veil(h, veil);

    p8_gfx_init(&gfx);
    p8_cls(&mem);
    ui_draw_overlay(&gfx, &mem, main_ui());
    check("hover cursor", screen_hash(), HOVER_CURSOR_HASH);

    assert(ui_send_to_hovered(main_ui(), SEND_TO_HOVERED_LEFT_PRESS, NULL, NULL, NULL, &kv, &kn));
    assert((void *)kv == veil && kn.stop && kn.held == NULL);
    kv = NULL;
    if (ui_send_to_hovered(main_ui(), SEND_TO_HOVERED_RIGHT_PRESS, NULL, NULL, NULL, &kv, &kn)) {
        assert((void *)kv != veil);
    }
    h = fold_veil(h, veil);

    entity_unregister(&veil->header);
    ui_update(&mem, main_ui());
    assert(main_ui_hovered() != veil && !in_layer(14, veil));
    assert(!veil->is_hovered);
    h = fnv(h, (uint32_t)p8_peek(&mem, 0x5f80));
    h = fold_veil(h, veil);
    check("hover", h, HOVER_SEQUENCE_HASH);
}

/* The veil's defaults are the class's generated defaults. */
static void test_defaults_match_the_parsed_spec(void) {
    modal_backdrop_t *veil;

    reset_world();
    cart_flush_exit_queue();
    veil = modal_backdrop_new(NULL);
    assert(veil->archetype == &modal_backdrop_archetype);
    assert(veil->archetype->layer == p8_fixed_from_int(14));
    assert(veil->archetype->hit_test == p8_fixed_from_int(1));
    assert(veil->header.state != NULL && strcmp(veil->header.state, "normal") == 0);
    assert(modal_backdrop_ready());
    modal_backdrop_shutdown();
    assert(!modal_backdrop_ready());
    modal_backdrop_init();
}

static int is_a_veil_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == modal_backdrop_class();
}

/* Built by `pico8_fq_new` or through its class, a veil is a struct in hv
 * and cv.hd only; its `hd` is the constant hit of `hd=1`, and it has
 * neither `ph` nor `rdown`. */
static void test_modal_backdrop_is_a_struct_registered_under_hit_test(void) {
    const char *others[] = {"draw", "draw_overlay", "new_year", "refresh"};
    const entity_class_t *cls = modal_backdrop_class();
    modal_backdrop_keys_t keys = {0};
    void *built;
    void *generic;
    int i;

    reset_world();
    cart_flush_exit_queue();
    keys.on_click = score_panel_toggle_on_click();
    built = modal_backdrop_new(&keys);
    generic = modal_backdrop_new(NULL);

    assert(is_a_veil_struct(built) && is_a_veil_struct(generic));
    assert(entity_list_listed(live_entities(), built) && entity_list_listed(entities_by_message("hit_test"), built));
    assert(entity_list_listed(live_entities(), generic) && entity_list_listed(entities_by_message("hit_test"), generic));
    for (i = 0; i < 4; i++) {
        assert(!entity_list_listed(entities_by_message(others[i]), built) && !entity_list_listed(entities_by_message(others[i]), generic));
    }
    assert(cls->hit_test == entity_registry_hit_test_always && cls->hit_test(generic, NULL, NULL) == 1);
    assert(cls->states[ENTITY_STATE_NORMAL] == NULL && cls->right_press == NULL);
    assert(((const modal_backdrop_t *)built)->on_click == score_panel_toggle_on_click());
}

/* A veil takes a place; once its `my` lifts `p_` and `dg` takes it out,
 * emptying the exit queue gives the place back. */
static void test_veil_gives_its_slot_back(void) {
    modal_backdrop_t *veil;
    int base;
    int ticks = 0;

    reset_world();
    cart_flush_exit_queue();
    base = ui_zone_live(SLOT_KIND_MODAL_BACKDROP);
    veil = veil_with_on_click(1);
    assert(ui_zone_live(SLOT_KIND_MODAL_BACKDROP) == base + 1);
    assert(left_press(veil));
    while (entity_list_listed(live_entities(), veil)) {
        update_entities();
        assert(++ticks < 1000);
    }
    assert(ui_zone_live(SLOT_KIND_MODAL_BACKDROP) == base + 1 && "out of hv, still read this frame");
    cart_flush_exit_queue();
    assert(ui_zone_live(SLOT_KIND_MODAL_BACKDROP) == base);
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

    test_left_press_with_a_true_on_click_removes_the_veil();
    test_bare_veil_swallows_the_click_and_stays();
    test_false_on_click_keeps_the_veil();
    test_hover_loop_on_the_veil();
    test_defaults_match_the_parsed_spec();
    test_modal_backdrop_is_a_struct_registered_under_hit_test();
    test_veil_gives_its_slot_back();

    printf("test_modal_backdrop: all tests passed\n");
    return 0;
}
