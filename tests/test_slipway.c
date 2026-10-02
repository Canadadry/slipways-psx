/* The slipway `jc`, read and written on its members and driven through the
 * real `mv`/`dg`/`dc`/`ok`, in games built directly: no events file. */

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
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/probe.h"
#include "game/research.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "engine/vec.h"
#include "game/wormhole.h"
#include "engine/game_zone.h"

/* Ticks after the slipway is paid, long enough for the colonizing and
 * paying effects to leave `hv`: `dg` skips the entity after one it takes out
 * (TROU-5). */
#define SETTLE_TICKS 120

#define ATTACH_DISTANCE_BITS 0x00000106
#define DROP_HASH 0xEC58A304u
#define DRAG_HASH 0xCD175E6Bu
#define OTHER_END_HASH 0x19ADC88Cu
#define DRAW_HASH 0x92EAE4C5u
#define TARGETING_TRACK_HASH 0xB930691Fu
#define TOO_FAR_HASH 0x65AE0C50u
#define MOUSE_POINT_HASH 0x4C9D6519u

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const int want_trail_colors[20] = {0, 0, 7, 7, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 4, 4, 4, 2, 2, 1};

static const entity_class_t *class_of(const void *e) {
    return ((const entity_t *)e)->class;
}

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static uint32_t fnv_str(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFu);
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

static slipway_t *as_slipway(const void *e) {
    assert(e != NULL && class_of(e) == slipway_class());
    return (slipway_t *)e;
}

static planet_t *as_planet(const void *e) {
    assert(planet_is_family(e));
    return (planet_t *)e;
}

static vec_t vec_of(const vec_t *v) {
    assert(v != NULL);
    return *v;
}

/* 0 absent, 1 false, 2 true. */
static uint32_t is_dead_state(const slipway_t *jc) {
    if (!jc->is_dead) {
        return 0;
    }
    return jc->is_dead ? 2u : 1u;
}

static int is_throwaway(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == slipway_mouse_point_class();
}

static int is_too_far(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == slipway_too_far_class() &&
           ((const slipway_too_far_t *)e)->status_text != NULL && strcmp(((const slipway_too_far_t *)e)->status_text, "-too far-") == 0;
}

static slipway_too_far_t *as_too_far(const void *e) {
    assert(is_too_far(e));
    return (slipway_too_far_t *)e;
}

/* The mouse point's `ra` and `p`, NULL when absent. */
static const vec_t *mouse_point_pos(const void *re) {
    assert(is_throwaway(re));
    return ((const slipway_mouse_point_t *)re)->pos;
}

static const vec_t *mouse_point_map_pos(const void *re) {
    assert(is_throwaway(re));
    return ((const slipway_mouse_point_t *)re)->map_pos;
}

/* The word the frozen hashes fold before a boolean: 0xB0, then 0 or 1; any
 * other moves every *_HASH. */
static uint32_t fold_bool(uint32_t h, int b) {
    return fnv(fnv(h, 0xB0u), b ? 1u : 0u);
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (entity_list_listed(live_entities(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)entity_list_rank_in(live_entities(), ref));
    }
    /* The frozen hashes folded the `{qs}` as a class-less throwaway. */
    return fnv(h, is_too_far(ref) || is_throwaway(ref) ? 0x7Au : 0xEEEEu);
}

/* A member as the frozen hashes fold what the name reader gave: nil when
 * absent; the 0xF1 of a fixed and the 0x57 of a string are constant by the
 * struct's shape, kept so that the hashes still prove the typed reads. */
static uint32_t fold_fixed(uint32_t h, int has, p8_fixed_t f) {
    return has ? fnv(fnv(h, 0xF1u), (uint32_t)f) : fnv(h, 0xDEADu);
}

static uint32_t fold_string(uint32_t h, const char *s) {
    return s != NULL ? fnv_str(fnv(h, 0x57u), s) : fnv(h, 0xDEADu);
}

static uint32_t fold_vec(uint32_t h, const vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

enum { END_ABSENT, END_PLANET, END_THROWAWAY, END_OTHER };

static int end_kind(const void *jc) {
    const void *re = as_slipway(jc)->end;
    if (re == NULL) {
        return END_ABSENT;
    }
    if (entity_list_listed(entities_by_tag("node"), re)) {
        return END_PLANET;
    }
    if (is_throwaway(re)) {
        return END_THROWAWAY;
    }
    return END_OTHER;
}

enum { TARGET_ABSENT, TARGET_FALSE, TARGET_PLANET, TARGET_SLIPWAY, TARGET_STATUS_TEXT, TARGET_OTHER };

static int target_kind(const void *jc) {
    const void *et = as_slipway(jc)->targeting->target;
    if (et == NULL) {
        return TARGET_ABSENT;
    }
    if (entity_list_listed(entities_by_tag("node"), et)) {
        return TARGET_PLANET;
    }
    if (entity_list_listed(entities_by_tag("slipway"), et)) {
        return TARGET_SLIPWAY;
    }
    if (is_too_far(et)) {
        return TARGET_STATUS_TEXT;
    }
    return TARGET_OTHER;
}

enum { TARGETED_BY_ABSENT, TARGETED_BY_SLIPWAY, TARGETED_BY_PROBE, TARGETED_BY_WORMHOLE, TARGETED_BY_OTHER };

static int holder_kind(const void *fo) {
    if (fo == NULL) {
        return TARGETED_BY_ABSENT;
    }
    if (entity_list_listed(entities_by_tag("slipway"), fo)) {
        return TARGETED_BY_SLIPWAY;
    }
    if (entity_list_listed(entities_by_tag("probe"), fo)) {
        return TARGETED_BY_PROBE;
    }
    if (entity_list_listed(entities_by_tag("wormhole"), fo)) {
        return TARGETED_BY_WORMHOLE;
    }
    return TARGETED_BY_OTHER;
}

static int pointer_list_count(const pointer_list_t *l) {
    return l == NULL ? -1 : l->count;
}

static int entity_list_pointer_list_count(const entity_list_t *l) {
    return l == NULL ? -1 : l->count;
}

/* The count, then each `id` and `true`. */
static uint32_t fold_served_planets(uint32_t h, const void *jc) {
    const slipway_served_planets_t *hg = as_slipway(jc)->served_planets;
    int i;
    if (hg == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)hg->count);
    for (i = 0; i < hg->count; i++) {
        h = fnv_str(h, hg->ids[i]);
        h = fold_bool(h, 1);
    }
    return h;
}

static uint32_t fold_slipway(uint32_t h, const void *e) {
    const slipway_t *jc = as_slipway(e);

    h = fold_fixed(h, jc->targeting->has_cost, jc->targeting->cost);
    h = fold_fixed(h, jc->has_infraspace, jc->infraspace);
    h = fold_fixed(h, 1, jc->layer);
    h = fold_fixed(h, 1, jc->fade_in_step);
    h = fnv(h, is_dead_state(jc));
    h = fold_string(h, jc->header.state);
    h = fold_fixed(h, 1, jc->header.state_time);
    h = fold_ref(h, jc->start);
    h = fnv(h, (uint32_t)end_kind(jc));
    h = fold_ref(h, jc->end);
    if (end_kind(jc) == END_THROWAWAY) {
        h = fold_vec(h, mouse_point_map_pos(jc->end));
    }
    h = fnv(h, (uint32_t)target_kind(jc));
    h = fold_ref(h, jc->targeting->target);
    h = fold_ref(h, jc->targeted_by);
    h = fnv(h, (uint32_t)holder_kind(jc->targeted_by));
    h = fnv(h, (uint32_t)pointer_list_count(jc->transit_dots));
    return fold_served_planets(h, jc);
}

static p8_fixed_t credits(void) {
    assert(main_economy_typed() != NULL);
    return main_economy_typed()->nz;
}

static uint32_t fold_planets(uint32_t h, int with_other_end) {
    const entity_list_t *np = entities_by_tag("node");
    int i;

    for (i = 0; np != NULL && i < np->count; i++) {
        void *p = np->items[i];
        const entity_list_t *oe = as_planet(p)->slipways;
        h = fnv(h, (uint32_t)entity_list_pointer_list_count(oe));
        h = fold_ref(h, as_planet(p)->targeted_by);
        h = fnv(h, (uint32_t)holder_kind(as_planet(p)->targeted_by));
        if (with_other_end && oe != NULL) {
            int j;
            for (j = 0; j < oe->count; j++) {
                assert(class_of(oe->items[j]) == slipway_class());
                h = fold_ref(h, slipway_other_end((const slipway_t *)oe->items[j], p));
            }
        }
    }
    return h;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
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

static vec_t pos_of(const void *e) {
    return vec_of(as_planet(e)->map_pos);
}

static p8_fixed_t dist2(vec_t a, vec_t b) {
    vec_t d = vec_sub(a, b);
    return p8_fixed_add(p8_fixed_mul(d.x, d.x), p8_fixed_mul(d.y, d.y));
}

/* A started game's first generated planet and its nearest one, both
 * colonized by a bare project. */
static void colonized_pair(void **qu, void **re) {
    const entity_list_t *jos;
    int i;

    start_game();
    jos = entities_by_tag("planet");
    assert(jos != NULL && jos->count > 1);
    *qu = jos->items[0];
    *re = NULL;
    for (i = 1; i < jos->count; i++) {
        void *e = jos->items[i];
        if (*re == NULL || dist2(pos_of(e), pos_of(*qu)) < dist2(pos_of(*re), pos_of(*qu))) {
            *re = e;
        }
    }
    planet_apply_project(*qu, pico8_hp_new(NULL));
    planet_apply_project(*re, pico8_hp_new(NULL));
}

static void *build_slipway(void *qu);
static void drag_over(void *jc, void *hovered, vec_t *mp);

/* A slipway from `qu` dragged over `re` as `jc:hq` sees a drag (it reads the
 * hovered planet off `ok.o_`), held until its `t` reaches 15, not paid. */
static void *dragged(void *qu, void *re) {
    vec_t *mp = malloc(sizeof *mp);
    void *jc = build_slipway(qu);

    *mp = vec_add(vec_mul(pos_of(re), p8_fixed_from_int(128)),
                        vec_make(p8_fixed_from_int(1), p8_fixed_from_int(1)));
    drag_over(jc, re, mp);
    pico8_ok()->o_ = NULL;
    while (as_slipway(jc)->header.state_time < p8_fixed_from_int(15)) {
        update_entities();
    }
    return jc;
}

/* Two colonized planets joined by one slipway, paid by `ey` as a drag pays
 * it, then left to settle. */
static void world(void) {
    void *qu;
    void *re;
    void *jc;
    int i;

    colonized_pair(&qu, &re);
    jc = dragged(qu, re);
    slipway_class()->drop(jc, re);
    for (i = 0; i < SETTLE_TICKS; i++) {
        update_entities();
    }
    assert(entities_by_tag("slipway") != NULL && entities_by_tag("slipway")->count == 1);
}

static void *the_slipway(void) {
    return entities_by_tag("slipway")->items[0];
}

static void *build_slipway(void *qu) {
    slipway_keys_t keys = {0};

    keys.start = qu;
    return slipway_new(&keys);
}

static vec_t *vec_ref(int x, int y) {
    vec_t *v = malloc(sizeof *v);
    *v = vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y));
    return v;
}

static vec_t *vec_ref_fixed(vec_t at) {
    vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

/* `jc:hq` reads the hovered entity off `ok.o_`. */
static void drag_over(void *jc, void *hovered, vec_t *mp) {
    pico8_ok()->o_ = hovered;
    class_of(jc)->drag(jc, mp, NULL);
}

/* Every entity of `cg` begins with its header. */
static const entity_t *entity_of(const void *e) {
    assert(e != NULL);
    return e;
}

static void *first_with_state(const char *tag, const char *me) {
    const entity_list_t *list = entities_by_tag(tag);
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        const entity_t *ke = entity_of(list->items[i]);
        if (ke->state != NULL && strcmp(ke->state, me) == 0) {
            return list->items[i];
        }
    }
    return NULL;
}

/* A fresh `jc`: the spec's defaults, ke's `t`, `qu`, and the `ts`/`hg` its
 * `pq` made; registered like any `ke` entity with `kj` and `ev`. */
static void check_fresh_slipway(const void *e, const void *qu) {
    const slipway_t *jc = as_slipway(e);
    const int_seq_t *k_;
    const slipway_served_planets_t *hg;
    size_t i;

    assert(jc->archetype == &slipway_archetype && jc->targeting->archetype == &slipway_archetype.targeting);
    assert(jc->header.state != NULL && strcmp(jc->header.state, "placing") == 0);
    assert(jc->header.state_time == 0);
    assert(jc->layer == p8_fixed_from_int(2));
    assert(jc->targeting->archetype->clock_ticks == 0);
    assert(jc->targeting->archetype->status_sprite == p8_fixed_from_int(48) && jc->targeting->archetype->has_status_sprite == 1);
    printf("[dw] 0x%08X\n", (unsigned)jc->targeting->archetype->attach_distance);
    assert(jc->targeting->archetype->attach_distance == (p8_fixed_t)ATTACH_DISTANCE_BITS);
    assert(jc->fade_in_step == p8_fixed_from_int(20));

    k_ = jc->archetype->trail_colors;
    assert(k_ == &slipway_trail_colors_seq && k_->count == 20);
    for (i = 0; i < 20; i++) {
        assert(k_->items[i] == want_trail_colors[i]);
    }
    assert(jc->header.class->tags == TAG(TAG_SLIPWAY));
    assert(jc->start == qu);

    assert(pointer_list_count(jc->transit_dots) == 0);
    hg = jc->served_planets;
    assert(hg == &jc->served_planets_own && hg->count == 0);

    assert(jc->end == NULL && !jc->targeting->has_cost && jc->targeting->cost == 0 && !jc->has_infraspace && jc->targeting->target == NULL);
    assert(jc->header.infraspace == NULL);
    assert(jc->targeted_by == NULL && !jc->is_dead);
    assert(class_of(jc)->drag != NULL && class_of(jc)->drop != NULL);
    assert(class_of(jc)->draw_overlay != NULL && class_of(jc)->draw != NULL);
    assert(class_of(jc)->hit_test == NULL && class_of(jc)->states[ENTITY_STATE_PLACING] == NULL &&
           class_of(jc)->states[ENTITY_STATE_NORMAL] == NULL && class_of(jc)->left_press == NULL && class_of(jc)->accept_drop == NULL);

    assert(entity_list_listed(live_entities(), jc) && entity_list_listed(entities_by_message("draw"), jc) && entity_list_listed(entities_by_message("draw_overlay"), jc));
    assert(entity_list_listed(entities_by_tag("slipway"), jc));
    assert(!entity_list_listed(entities_by_message("hit_test"), jc) && !entity_list_listed(entities_by_message("new_year"), jc) && !entity_list_listed(entities_by_message("refresh"), jc));
}

static void test_construction(void) {
    void *ly;
    void *by_left_press;
    void *by_lookup;
    down_t made;

    world();
    ly = first_with_state("planet", "settled");
    assert(ly != NULL);
    made = class_of(ly)->left_press(ly, NULL, NULL);
    assert(made.stop && made.held != NULL);
    by_left_press = made.held;
    check_fresh_slipway(by_left_press, ly);
    by_lookup = build_slipway(ly);
    check_fresh_slipway(by_lookup, ly);

    assert(as_slipway(by_left_press)->archetype == as_slipway(by_lookup)->archetype);
    assert(as_slipway(by_left_press)->header.class->tags == as_slipway(by_lookup)->header.class->tags);
    assert(as_slipway(by_left_press)->transit_dots != as_slipway(by_lookup)->transit_dots);
    assert(as_slipway(by_left_press)->served_planets != as_slipway(by_lookup)->served_planets);
}

/* Without a target, `ey` only raises `p_`; with one, the slipway joins both
 * `oe` and `d_` charges `od`. */
static void test_drop(void) {
    uint32_t hash = 2166136261u;
    void *held;
    void *origin;
    void *target;
    void *refused;
    p8_fixed_t before;
    p8_fixed_t od;

    colonized_pair(&origin, &target);
    held = dragged(origin, target);
    assert(entity_list_listed(entities_by_tag("slipway"), held));
    assert(as_slipway(held)->start == origin && as_slipway(held)->end == target);
    assert(end_kind(held) == END_PLANET && target_kind(held) == TARGET_ABSENT);
    assert(entity_list_pointer_list_count(as_planet(origin)->slipways) == 0 && entity_list_pointer_list_count(as_planet(target)->slipways) == 0);
    before = credits();
    assert(as_slipway(held)->targeting->has_cost);
    od = as_slipway(held)->targeting->cost;
    hash = fold_slipway(hash, held);

    refused = build_slipway(origin);
    class_of(refused)->drop(refused, NULL);
    assert(is_dead_state(as_slipway(refused)) == 2 && credits() == before);
    assert(entity_list_pointer_list_count(as_planet(origin)->slipways) == 0);
    hash = fold_slipway(hash, refused);

    class_of(held)->drop(held, target);
    assert(as_slipway(held)->fade_in_step == p8_fixed_from_int(1));
    assert(as_slipway(held)->end == target);
    assert(is_dead_state(as_slipway(held)) == 0);
    assert(entity_list_pointer_list_count(as_planet(origin)->slipways) == 1 && entity_list_pointer_list_count(as_planet(target)->slipways) == 1);
    assert(credits() == p8_fixed_sub(before, od));
    hash = fold_slipway(hash, held);
    hash = fold_planets(hash, 1);
    hash = fnv(hash, (uint32_t)credits());
    check("drop", hash, DROP_HASH);
}

/* Beyond range `et` is a fresh `{qs}`; over the pair an existing slipway
 * already joins, `et` is that slipway; either way its `fo` is the new one. */
static void test_drag(void) {
    uint32_t h = 2166136261u;
    void *old;
    void *origin;
    void *target;
    void *far;
    void *twin;

    world();
    old = the_slipway();
    origin = as_slipway(old)->start;
    target = as_slipway(old)->end;

    far = build_slipway(origin);
    drag_over(far, NULL, vec_ref(-640, -640));
    assert(end_kind(far) == END_THROWAWAY && target_kind(far) == TARGET_STATUS_TEXT);
    assert(as_too_far(as_slipway(far)->targeting->target)->targeted_by == far);
    h = fold_slipway(h, far);

    twin = build_slipway(origin);
    drag_over(twin, target, vec_ref_fixed(vec_of(as_planet(target)->pos)));
    assert(end_kind(twin) == END_PLANET && as_slipway(twin)->end == target);
    assert(target_kind(twin) == TARGET_SLIPWAY && as_slipway(twin)->targeting->target == old);
    assert(as_slipway(old)->targeted_by == twin);
    h = fold_slipway(h, twin);
    h = fold_slipway(h, old);
    check("drag", h, DRAG_HASH);
}

static void test_drag_takes_infraspace_from_its_start(void) {
    void *from_planet;
    void *from_infraspace_gate;

    world();
    from_planet = build_slipway(planet_new(NULL));
    drag_over(from_planet, NULL, vec_ref(-640, -640));
    assert(!as_slipway(from_planet)->has_infraspace && as_slipway(from_planet)->header.infraspace == NULL);
    from_infraspace_gate = build_slipway(infraspace_gate_new(NULL));
    drag_over(from_infraspace_gate, NULL, vec_ref(-640, -640));
    assert(as_slipway(from_infraspace_gate)->has_infraspace && as_slipway(from_infraspace_gate)->infraspace == infraspace_gate_archetype.infraspace);
    assert(as_slipway(from_infraspace_gate)->header.infraspace == &as_slipway(from_infraspace_gate)->infraspace);
}

/* `A and B or C`: a falsy `re` falls back to `qu`. */
static void test_other_end(void) {
    uint32_t h = 2166136261u;
    void *old;
    void *origin;
    void *target;
    void *fresh;

    world();
    old = the_slipway();
    origin = as_slipway(old)->start;
    target = as_slipway(old)->end;
    fresh = build_slipway(origin);

    assert(slipway_other_end(old, origin) == target);
    assert(slipway_other_end(old, target) == origin);
    assert(slipway_other_end(fresh, origin) == origin);
    assert(slipway_other_end(fresh, target) == origin);
    h = fold_ref(h, slipway_other_end(old, origin));
    h = fold_ref(h, slipway_other_end(old, target));
    h = fold_ref(h, slipway_other_end(fresh, origin));
    h = fold_ref(h, slipway_other_end(fresh, NULL));
    check("other_end", h, OTHER_END_HASH);
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

/* Under the game's camera: the slipway alone on a cleared screen, `fo` cleared after drawing; then a
 * planet whose `fo` is a slipway, circled at `dw`'s radius. */
static void test_draw(void) {
    uint32_t hash = 2166136261u;
    uint32_t blank;
    uint32_t plain;
    uint32_t circled;
    vec_t main_camera_pos;
    void *old;
    void *origin;
    int i;

    world();
    old = the_slipway();
    origin = as_slipway(old)->start;
    assert(pico8_mo_qb() != NULL);
    main_camera_pos = vec_of(pico8_mo_qb()->ra);
    p8_camera(&gfx, p8_fixed_flr_int(main_camera_pos.x), p8_fixed_flr_int(main_camera_pos.y));
    p8_cls(&mem);
    blank = screen_hash();

    for (i = 0; i < 3; i++) {
        as_slipway(old)->targeted_by = old;
        p8_cls(&mem);
        class_of(old)->draw(old, NULL);
        assert(as_slipway(old)->targeted_by == NULL);
        assert(screen_hash() != blank);
        hash = fnv(hash, screen_hash());
        hash = fold_slipway(hash, old);
    }

    p8_cls(&mem);
    class_of(origin)->draw(origin, vec_ref_fixed(vec_of(as_planet(origin)->pos)));
    plain = screen_hash();
    as_planet(origin)->targeted_by = old;
    p8_cls(&mem);
    class_of(origin)->draw(origin, vec_ref_fixed(vec_of(as_planet(origin)->pos)));
    circled = screen_hash();
    assert(as_planet(origin)->targeted_by == NULL);
    assert(circled != plain);
    hash = fnv(hash, plain);
    hash = fnv(hash, circled);
    check("draw", hash, DRAW_HASH);
}

/* Beyond range: `kj` draws the line under the game's camera, `ev` writes
 * `qs` at the mouse where one with another `et` draws a sprite and one
 * without writes its cost, and `ey` onto a real planet is refused without
 * spending or joining anything. */
static void test_too_far(void) {
    uint32_t hash = 2166136261u;
    uint32_t blank;
    uint32_t too_far;
    uint32_t sprite;
    uint32_t costed;
    vec_t main_camera_pos;
    vec_t mouse;
    p8_fixed_t fade_in_step;
    p8_fixed_t before;
    void *old;
    void *origin;
    void *target;
    void *far;
    void *twin;
    void *near;
    void *end;

    world();
    old = the_slipway();
    origin = as_slipway(old)->start;
    target = as_slipway(old)->end;
    far = build_slipway(origin);
    drag_over(far, NULL, vec_ref(-640, -640));
    assert(end_kind(far) == END_THROWAWAY && target_kind(far) == TARGET_STATUS_TEXT);
    assert(target_kind(old) == TARGET_ABSENT);
    assert(pico8_mo_qb() != NULL);
    main_camera_pos = vec_of(pico8_mo_qb()->ra);
    p8_camera(&gfx, p8_fixed_flr_int(main_camera_pos.x), p8_fixed_flr_int(main_camera_pos.y));
    p8_cls(&mem);
    blank = screen_hash();

    class_of(far)->draw(far, NULL);
    assert(screen_hash() != blank);
    hash = fnv(hash, screen_hash());

    twin = build_slipway(origin);
    drag_over(twin, target, vec_ref_fixed(vec_of(as_planet(target)->pos)));
    assert(target_kind(twin) == TARGET_SLIPWAY);
    near = build_slipway(origin);
    drag_over(near, NULL,
            vec_ref_fixed(vec_add(vec_of(as_planet(origin)->pos), vec_make(p8_fixed_from_int(16), 0))));
    assert(end_kind(near) == END_THROWAWAY && target_kind(near) == TARGET_ABSENT && as_slipway(near)->targeting->has_cost);
    assert(pico8_je_nf() != NULL);
    mouse = vec_of(pico8_je_nf()->ra);
    p8_camera(&gfx, p8_fixed_flr_int(mouse.x) - 64, p8_fixed_flr_int(mouse.y) - 64);
    p8_cls(&mem);
    class_of(far)->draw_overlay(far, NULL);
    too_far = screen_hash();
    p8_cls(&mem);
    class_of(twin)->draw_overlay(twin, NULL);
    sprite = screen_hash();
    p8_cls(&mem);
    class_of(near)->draw_overlay(near, NULL);
    costed = screen_hash();
    assert(too_far != blank && sprite != blank && costed != blank);
    assert(too_far != sprite && too_far != costed);
    hash = fnv(hash, too_far);
    hash = fnv(hash, sprite);
    hash = fnv(hash, costed);

    before = credits();
    fade_in_step = as_slipway(far)->fade_in_step;
    end = as_slipway(far)->end;
    assert(entity_list_pointer_list_count(as_planet(origin)->slipways) == 1 && entity_list_pointer_list_count(as_planet(target)->slipways) == 1);
    class_of(far)->drop(far, target);
    assert(is_dead_state(as_slipway(far)) == 2);
    assert(credits() == before);
    assert(entity_list_pointer_list_count(as_planet(origin)->slipways) == 1 && entity_list_pointer_list_count(as_planet(target)->slipways) == 1);
    assert(as_slipway(far)->end == end && end_kind(far) == END_THROWAWAY);
    assert(as_slipway(far)->fade_in_step == fade_in_step);
    assert(target_kind(far) == TARGET_STATUS_TEXT);
    hash = fold_slipway(hash, far);
    hash = fnv(hash, (uint32_t)credits());
    check("too far", hash, TOO_FAR_HASH);
}

static int same_vec(vec_t a, vec_t b) {
    return a.x == b.x && a.y == b.y;
}

/* The cart keeps the box of `mp`, the port a copy of it. */
static int holds_copy_of(const vec_t *ra, const vec_t *m) {
    return ra != NULL && ra != m && same_vec(*ra, *m);
}

/* Over nothing, `hq` hangs on `re` the slipway's own `{ra,p}`, rewritten in
 * place at each call: `ra` a copy of the `mp` it got, `p` its own `mp/128`,
 * registered nowhere; another `jc` has its own; the generation does not
 * move. */
static void test_drag_mouse_point(void) {
    uint32_t hash = 2166136261u;
    vec_t *first_mouse;
    vec_t *second_mouse;
    vec_t first_pos;
    vec_t on_camera;
    void *origin;
    void *target;
    void *first;
    void *second;
    void *first_end;
    void *second_end;
    void *rows;
    size_t grab_arena;
    int index;

    world();
    origin = as_slipway(the_slipway())->start;
    target = as_slipway(the_slipway())->end;
    first = build_slipway(origin);
    first_mouse = vec_ref(-640, -640);
    drag_over(first, NULL, first_mouse);
    first_end = as_slipway(first)->end;
    assert(first_end != NULL && end_kind(first) == END_THROWAWAY);
    assert(holds_copy_of(mouse_point_pos(first_end), first_mouse));
    first_pos = vec_of(mouse_point_map_pos(first_end));
    assert(same_vec(first_pos, vec_div(vec_of(mouse_point_pos(first_end)), p8_fixed_from_int(128))));
    assert(mouse_point_map_pos(first_end) != first_mouse);
    assert(*((const entity_t *)first_end)->map_pos == mouse_point_map_pos(first_end));
    assert(!entity_geom_is_on_screen(first_end));
    assert(!entity_list_listed(live_entities(), first_end) && !entity_list_listed(entities_by_tag("slipway"), first_end) && !entity_list_listed(entities_by_tag("node"), first_end));
    hash = fnv(hash, (uint32_t)first_pos.x);
    hash = fnv(hash, (uint32_t)first_pos.y);
    hash = fold_slipway(hash, first);

    second_mouse = vec_ref(40, -24);
    drag_over(first, NULL, second_mouse);
    second_end = as_slipway(first)->end;
    assert(second_end == first_end && end_kind(first) == END_THROWAWAY && "the mouse point is the slipway's own, rewritten in place");
    assert(holds_copy_of(mouse_point_pos(second_end), second_mouse));
    assert(same_vec(vec_of(mouse_point_map_pos(second_end)), vec_div(*second_mouse, p8_fixed_from_int(128))));
    assert(!same_vec(vec_of(mouse_point_map_pos(second_end)), first_pos));
    hash = fold_slipway(hash, first);

    grab_arena = game_zone_used();
    for (index = 0; index < 10; index++) {
        drag_over(first, NULL, index % 2 ? first_mouse : second_mouse);
        assert(as_slipway(first)->end == first_end);
    }
    assert(game_zone_used() == grab_arena && "a held slipway allocates nothing per hq");
    drag_over(first, NULL, second_mouse);

    second = build_slipway(origin);
    drag_over(second, NULL, first_mouse);
    rows = as_slipway(second)->end;
    assert(rows != NULL && rows != first_end && as_slipway(first)->end == first_end);
    assert(holds_copy_of(mouse_point_pos(rows), first_mouse) && same_vec(vec_of(mouse_point_map_pos(rows)), first_pos));
    assert(holds_copy_of(mouse_point_pos(first_end), second_mouse) && "another slipway's hq leaves this one's point as it was");
    hash = fold_slipway(hash, second);

    drag_over(first, target, vec_ref_fixed(vec_of(as_planet(target)->pos)));
    assert(as_slipway(first)->end == target && end_kind(first) == END_PLANET);
    assert(holds_copy_of(mouse_point_pos(rows), first_mouse));
    hash = fold_slipway(hash, first);
    check("mouse point", hash, MOUSE_POINT_HASH);

    on_camera = vec_add(vec_of(pico8_mo_qb()->ra),
                              vec_make(p8_fixed_from_int(64), p8_fixed_from_int(64)));
    drag_over(second, NULL, vec_ref_fixed(on_camera));
    assert(entity_geom_is_on_screen(as_slipway(second)->end));
}

/* `jc:ce` falls through to `cg("jc")` when no planet lies on the drag: a
 * drag from a nearby planet to just past the slipway's middle crosses it. */
static void test_obstruction_crosses_a_slipway(void) {
    const entity_list_t *jos;
    void *old;
    vec_t mid;
    int i;
    int crossed = 0;

    world();
    jos = entities_by_tag("planet");
    old = the_slipway();
    mid = vec_div(vec_add(pos_of(as_slipway(old)->start), pos_of(as_slipway(old)->end)), p8_fixed_from_int(2));
    for (i = 0; i < jos->count && !crossed; i++) {
        void *q = jos->items[i];
        vec_t past;
        void *a;

        if (q == as_slipway(old)->start || q == as_slipway(old)->end) {
            continue;
        }
        past = vec_add(mid, vec_div(vec_sub(mid, pos_of(q)), p8_fixed_from_int(8)));
        a = build_slipway(q);
        drag_over(a, NULL, vec_ref_fixed(vec_mul(past, p8_fixed_from_int(128))));
        crossed = as_slipway(a)->targeting->target == old;
        if (!crossed) {
            entity_unregister(a);
        }
    }
    printf("[ce] crossed from planet %d\n", i - 1);
    assert(crossed);
}

/* g_'s archetype with the probe's `dw` zeroed, as `mz`'s is: every point of
 * this short slipway lies within a `g_`'s `dw` of one of its own planets,
 * which would win. */
static probe_archetype_t g_attach_distance_zero;

/* A `g_` built as `pico8_g__new(NULL)` builds one, on `g_dw_zero`. */
static probe_t *probe_with_zero_attach_distance(void) {
    probe_t *probe = malloc(sizeof *probe);

    g_attach_distance_zero = probe_archetype;
    g_attach_distance_zero.targeting.attach_distance = 0;
    probe_init_instance(probe, probe_class(), &g_attach_distance_zero);
    emitter_init(&probe->header);
    entity_construct(&probe->header);
    return probe;
}

/* A probe over the slipway targets it and hangs `fo` on it. */
static void test_targeting_track(void) {
    uint32_t h = 2166136261u;
    void *old;
    void *probe;
    vec_t qu;
    vec_t re;
    int k;

    world();
    old = the_slipway();
    probe = probe_with_zero_attach_distance();
    qu = pos_of(as_slipway(old)->start);
    re = pos_of(as_slipway(old)->end);
    for (k = 1; k < 8; k++) {
        vec_t at = vec_add(qu, vec_div(vec_mul(vec_sub(re, qu), p8_fixed_from_int(k)),
                                                         p8_fixed_from_int(8)));
        class_of(probe)->drag(probe, vec_ref_fixed(vec_mul(at, p8_fixed_from_int(128))),
                          vec_ref(64, 64));
        h = fold_ref(h, ((probe_t *)probe)->targeting->target);
        if (((probe_t *)probe)->targeting->target == old) {
            break;
        }
    }
    printf("[targeting_track] on the slipway at %d/8\n", k);
    assert(((probe_t *)probe)->targeting->target == old);
    assert(as_slipway(old)->targeted_by == probe);
    assert(holder_kind(as_slipway(old)->targeted_by) == TARGETED_BY_PROBE);
    h = fold_slipway(h, old);
    h = fold_ref(h, ((probe_t *)probe)->targeting->target);
    check("targeting_track", h, TARGETING_TRACK_HASH);
}

/* `bv` aims at the first slipway of `cg("jc")` under the probe (`jp(...)[1]`),
 * not the last: a second slipway laid over the first, registered after it,
 * never wins. */
static void test_targeting_track_takes_the_first_slipway_of_entities_by_tag(void) {
    void *old;
    void *twin;
    void *probe;
    vec_t qu;
    vec_t re;
    vec_t at;

    world();
    old = the_slipway();
    twin = build_slipway(as_slipway(old)->end);
    as_slipway(twin)->end = as_slipway(old)->start;
    assert(entities_by_tag("slipway")->count == 2 && entities_by_tag("slipway")->items[0] == old &&
           entities_by_tag("slipway")->items[1] == twin);
    probe = probe_with_zero_attach_distance();
    qu = pos_of(as_slipway(old)->start);
    re = pos_of(as_slipway(old)->end);
    at = vec_div(vec_add(qu, re), p8_fixed_from_int(2));
    class_of(probe)->drag(probe, vec_ref_fixed(vec_mul(at, p8_fixed_from_int(128))), vec_ref(64, 64));
    assert(((probe_t *)probe)->targeting->target == old);
    assert(as_slipway(old)->targeted_by == probe && as_slipway(twin)->targeted_by != probe);
}

static int is_a_slipway_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == slipway_class();
}

static void must_abort(void (*write)(void *), void *jc) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        write(jc);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

static void served_planets_add_nil(void *hg) {
    slipway_served_planets_add(hg, NULL);
}

static void served_planets_add_one_more(void *hg) {
    slipway_served_planets_add(hg, "one more");
}

/* `hg[id]=true` keeps each string `id` once and in order, reads anything
 * else as absent, refuses a nil `id` and a 91st planet. */
static void test_served_planets_is_a_set_of_ids(void) {
    static char ids[SLIPWAY_SERVED_PLANETS_CAP][8];
    slipway_served_planets_t hg = {0, {NULL}};
    int i;

    slipway_served_planets_add(&hg, "p1");
    slipway_served_planets_add(&hg, "p2");
    slipway_served_planets_add(&hg, "p1");
    assert(hg.count == 2 && strcmp(hg.ids[0], "p1") == 0 && strcmp(hg.ids[1], "p2") == 0);
    assert(slipway_served_planets_has(&hg, "p2") && !slipway_served_planets_has(&hg, "p3"));
    assert(!slipway_served_planets_has(&hg, NULL) && !slipway_served_planets_has(NULL, "p1"));
    must_abort(served_planets_add_nil, &hg);

    hg.count = 0;
    for (i = 0; i < SLIPWAY_SERVED_PLANETS_CAP; i++) {
        snprintf(ids[i], sizeof ids[i], "p%d", i + 1);
        slipway_served_planets_add(&hg, ids[i]);
    }
    assert(hg.count == SLIPWAY_SERVED_PLANETS_CAP && slipway_served_planets_has(&hg, "p90"));
    must_abort(served_planets_add_one_more, &hg);
}

/* Built by `jo:ldown`/`nr:ldown` or directly, a `jc`
 * is a struct with its five methods. */
static void test_slipway_is_a_struct(void) {
    void *old;
    void *jc;
    slipway_keys_t keys = {0};
    void *direct;

    world();
    old = the_slipway();
    jc = build_slipway(as_slipway(old)->start);
    keys.start = as_slipway(old)->start;
    direct = slipway_new(&keys);
    assert(as_slipway(direct)->start == as_slipway(old)->start);
    assert(is_a_slipway_struct(old) && is_a_slipway_struct(jc) && is_a_slipway_struct(direct));
    assert(class_of(jc)->drag != NULL && class_of(jc)->drop != NULL);
    assert(class_of(jc)->draw_overlay != NULL && class_of(jc)->draw != NULL);
    assert(class_of(jc)->states[ENTITY_STATE_PLACING] == NULL && class_of(jc)->states[ENTITY_STATE_NORMAL] == NULL &&
           class_of(jc)->left_press == NULL && class_of(jc)->right_press == NULL && class_of(jc)->accept_drop == NULL);
}

/* Beyond range, every `jc` gets the same struct: `qs` fixed, `fo` the last
 * `hq` to write it, no method. */
static void test_too_far_is_a_struct(void) {
    void *origin;
    void *far;
    void *other;
    slipway_too_far_t *et;

    world();
    origin = as_slipway(the_slipway())->start;
    far = build_slipway(origin);
    drag_over(far, NULL, vec_ref(-640, -640));
    et = as_too_far(as_slipway(far)->targeting->target);
    assert(et->targeted_by == far);
    assert(class_of(et)->draw_overlay == NULL && class_of(et)->draw == NULL && class_of(et)->drag == NULL);

    other = build_slipway(origin);
    drag_over(other, NULL, vec_ref(-640, -640));
    assert(as_slipway(other)->targeting->target == &et->header && et->targeted_by == other);
    assert(strcmp(et->status_text, "-too far-") == 0);
}

/* Over nothing, `re` is a `{ra,p}` struct without method; a nil `mp`
 * leaves `ra` absent and `p` zero. */
static void test_mouse_point_is_a_struct(void) {
    void *origin;
    void *jc;
    void *re;
    vec_t p;

    world();
    origin = as_slipway(the_slipway())->start;
    jc = build_slipway(origin);
    drag_over(jc, NULL, vec_ref(-640, -640));
    re = as_slipway(jc)->end;
    assert(re != NULL &&
           ((const entity_t *)re)->class == slipway_mouse_point_class());
    assert(mouse_point_pos(re) != NULL && mouse_point_map_pos(re) != NULL);
    assert(class_of(re)->attach_slipway == NULL && class_of(re)->drag == NULL);

    drag_over(jc, NULL, NULL);
    re = as_slipway(jc)->end;
    assert(is_throwaway(re) && mouse_point_pos(re) == NULL);
    p = vec_of(mouse_point_map_pos(re));
    assert(p.x == 0 && p.y == 0);
}

static void *g_off_start;

static void build_off_row(void *e) {
    slipway_keys_t keys = {0};

    (void)e;
    keys.start = g_off_start;
    (void)slipway_new(&keys);
}

/* A construction's `qu` that is not a planet stops the run. */
static void test_slipway_new_stops_off_start(void) {
    void *off[5];
    void *old;
    void *far;
    size_t i;

    world();
    old = the_slipway();
    far = build_slipway(as_slipway(old)->start);
    drag_over(far, NULL, vec_ref(-640, -640));
    assert(is_throwaway(as_slipway(far)->end));
    off[0] = pico8_ga_new(NULL);
    off[1] = probe_new(NULL);
    off[2] = wormhole_new();
    off[3] = far;
    off[4] = as_slipway(far)->end;
    for (i = 0; i < sizeof off / sizeof off[0]; i++) {
        g_off_start = off[i];
        must_abort(build_off_row, NULL);
    }
}

/* The galaxy depends on the rnd state: run each scenario in a child forked
 * right after pico8_cart_load(), as a fresh process would start it. */
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
    in_fresh_child(test_drop);
    in_fresh_child(test_drag);
    in_fresh_child(test_drag_takes_infraspace_from_its_start);
    in_fresh_child(test_other_end);
    in_fresh_child(test_draw);
    in_fresh_child(test_targeting_track);
    in_fresh_child(test_targeting_track_takes_the_first_slipway_of_entities_by_tag);
    in_fresh_child(test_too_far);
    in_fresh_child(test_drag_mouse_point);
    in_fresh_child(test_obstruction_crosses_a_slipway);
    in_fresh_child(test_slipway_is_a_struct);
    in_fresh_child(test_served_planets_is_a_set_of_ids);
    in_fresh_child(test_too_far_is_a_struct);
    in_fresh_child(test_mouse_point_is_a_struct);
    in_fresh_child(test_slipway_new_stops_off_start);

    printf("test_slipway: all tests passed\n");
    return 0;
}
