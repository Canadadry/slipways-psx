/* The wormhole, read through its struct and driven through its class's
 * methods and update_entities, in a game built directly: no events file. */

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
#include "engine/entity.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/vram.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/planet.h"
#include "game/research.h"
#include "game/probe.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "engine/vec.h"
#include "game/wormhole.h"
#include "engine/zone.h"
#include "engine/game_zone.h"

/* Ticks after the slipway is paid, long enough for the colonizing and
 * paying effects to leave the live entities: update_entities skips the
 * entity after one it takes out (GAP-5). */
#define SETTLE_TICKS 120

#define ANCHORS_HASH 0xC6D8AF55u
#define DRAG_HASH 0x9DAA980Cu
#define UPDATE_PLACING_TICKS 21
#define UPDATE_PLACING_HASH 0xE16D1D52u
#define DROP_HASH 0x86F499FFu
#define UPDATE_UNFOLDING_HASH 0x85EB1EC6u
#define UPDATE_UNFOLDING_EXIT_TICK 27
#define DRAW_HASH 0xD124A50Cu

static p8_vram_t memory;
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

static int is_state(const entity_t *ke, state_t want) {
    return ke->state == want;
}

static wormhole_t *as_wormhole(void *e) {
    assert(e != NULL &&
           ((const entity_t *)e)->class == wormhole_class());
    return e;
}

static slipway_t *as_slipway(void *e) {
    assert(e != NULL &&
           ((const entity_t *)e)->class == slipway_class());
    return e;
}

static planet_t *as_planet(void *e) {
    assert(planet_is_family(e));
    return e;
}

static uint32_t fold_vec(uint32_t h, const vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

/* 0 absent or false, 2 true. */
static uint32_t is_dead_state(const wormhole_t *mz) {
    return mz->is_dead ? 2 : 0;
}

static uint32_t fold_string(uint32_t h, const char *s) {
    if (s == NULL) {
        return fnv(h, 0xDEADu);
    }
    return fnv_str(h, s);
}

enum { TARGET_ABSENT, TARGET_FALSE, TARGET_TOO_FAR, TARGET_SLIPWAY, TARGET_PLANET, TARGET_THROWAWAY, TARGET_OTHER };

static int is_targeting_track_target(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == targeting_track_target_class();
}

/* The throwaway target's targeted_by, NULL when absent. */
static const void *throwaway_targeted_by(const void *t) {
    assert(is_targeting_track_target(t));
    return ((const targeting_track_target_t *)t)->targeted_by;
}

static int target_kind(const wormhole_t *mz) {
    const void *et = mz->targeting->target;

    if (et == NULL) {
        return TARGET_FALSE;
    }
    if (et == (const void *)wormhole_too_far_target_wrapper()) {
        return TARGET_TOO_FAR;
    }
    if (entity_list_listed(entities_by_tag(TAG_SLIPWAY), et)) {
        return TARGET_SLIPWAY;
    }
    if (entity_list_listed(entities_by_tag(TAG_PLANET), et)) {
        return TARGET_PLANET;
    }
    if (is_targeting_track_target(et)) {
        return TARGET_THROWAWAY;
    }
    return TARGET_OTHER;
}

static p8_fixed_t credits(void) {
    const economy_t *hy = main_economy_typed();
    assert(hy != NULL);
    return hy->credits;
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

/* A game started as cart_start_game starts it, with no difficulty picked:
 * a NULL difficulty reads as difficulty 0. */
static void start_game(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    cart_init();
    cart_start_game(NULL);
}

static vec_t pos_of(void *e) {
    const planet_t *jo = as_planet(e);
    assert(jo->map_pos != NULL);
    return *jo->map_pos;
}

static p8_fixed_t dist2(vec_t a, vec_t b) {
    vec_t d = vec_sub(a, b);
    return p8_fixed_add(p8_fixed_mul(d.x, d.x), p8_fixed_mul(d.y, d.y));
}

/* Two generated planets colonized and joined by a paid slipway, the rest
 * of the galaxy left unknown, then left to settle. The slipway's drag reads
 * the hovered planet off main_ui()->hovered. */
static void world(void) {
    const entity_list_t *jos;
    slipway_keys_t slipway_keys = {0};
    vec_t *mp = malloc(sizeof *mp);
    void *qu;
    void *re = NULL;
    slipway_t *jc;
    int i;

    start_game();
    jos = entities_by_tag(TAG_PLANET);
    assert(jos != NULL && jos->count > 1);
    qu = jos->items[0];
    for (i = 1; i < jos->count; i++) {
        void *e = jos->items[i];
        if (re == NULL || dist2(pos_of(e), pos_of(qu)) < dist2(pos_of(re), pos_of(qu))) {
            re = e;
        }
    }
    planet_apply_project(qu, research_project_new(NULL));
    planet_apply_project(re, research_project_new(NULL));

    slipway_keys.start = qu;
    jc = slipway_new(&slipway_keys);
    *mp = vec_add(vec_mul(pos_of(re), p8_fixed_from_int(128)),
                        vec_make(p8_fixed_from_int(1), p8_fixed_from_int(1)));
    main_ui()->hovered = re;
    slipway_class()->drag(&jc->header, mp, NULL);
    main_ui()->hovered = NULL;
    while (jc->header.state_time < p8_fixed_from_int(15)) {
        update_entities();
    }
    slipway_class()->drop(&jc->header, re);
    assert((void *)jc->end == re);
    for (i = 0; i < SETTLE_TICKS; i++) {
        update_entities();
    }
}

static wormhole_t *build_wormhole(void) {
    return wormhole_new();
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

static void drag(void *mz, vec_t *mp, vec_t *rp) {
    wormhole_class()->drag(mz, mp, rp);
}

static void drop(void *mz) {
    wormhole_class()->drop(mz, NULL);
}

/* update_entities skips the entity after one it takes out of the live
 * entities (GAP-5): count ticks by state time, not by calls. */
static void until_state_time(const wormhole_t *mz, int t) {
    while (mz->header.state_time < p8_fixed_from_int(t)) {
        update_entities();
    }
}

static planet_t *first_planet_with_state(state_t state) {
    const entity_list_t *list = entities_by_tag(TAG_PLANET);
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        planet_t *jo = as_planet(list->items[i]);
        if (is_state(&jo->header, state)) {
            return jo;
        }
    }
    return NULL;
}

/* The middle of the slipway, in world units. */
static vec_t slipway_mid(slipway_t *jc) {
    return vec_mul(vec_div(vec_add(pos_of(jc->start), pos_of(jc->end)), p8_fixed_from_int(2)),
                         p8_fixed_from_int(128));
}

/* A fresh wormhole: the archetype's defaults, a zero state time, nothing
 * written yet but its anchors list, registered like any entity with draw
 * and draw_overlay. */
static void check_fresh_wormhole(void *e) {
    const wormhole_t *mz = as_wormhole(e);
    const int_seq_t *kk;
    const wormhole_too_far_target_t *jj;
    static const int want_color_ramp[9] = {7, 7, 6, 13, 5, 1, 1, 1, 0};
    int i;

    assert(is_state(&mz->header, ENTITY_STATE_PLACING));
    assert(mz->header.state_time == 0);
    assert(mz->targeting->archetype->attach_distance == 0);
    assert(mz->targeting->archetype->confirmed_state == ENTITY_STATE_UNFOLDING);
    assert(mz->archetype->spawn_snd == p8_fixed_from_int(52));
    assert(mz->radius == 0);
    assert(mz->targeting->cost == p8_fixed_from_int(3));
    assert(mz->targeting->archetype->clock_ticks == p8_fixed_from_int(1));
    assert(mz->archetype->layer == p8_fixed_from_int(5));
    assert(mz->header.layer == &mz->archetype->layer);

    kk = mz->archetype->color_ramp;
    assert(kk == &wormhole_color_ramp_seq && kk->count == 9);
    for (i = 0; i < 9; i++) {
        assert(kk->items[i] == want_color_ramp[i]);
    }
    assert(mz->header.class->tags == TAG(TAG_WORMHOLE));
    jj = mz->too_far_target;
    assert(jj != NULL && jj->status_text != NULL && strcmp(jj->status_text, "-too far-") == 0);
    assert(wormhole_too_far_target_wrapper() == jj);

    assert(mz->pos == NULL && mz->map_pos == NULL && mz->targeting->target == NULL && mz->anchor == NULL && !mz->is_dead);
    assert(mz->anchors != NULL);

    assert(entity_list_listed(live_entities(), mz) && entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW), mz) && entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW_OVERLAY), mz));
    assert(entity_list_listed(entities_by_tag(TAG_WORMHOLE), mz));
    assert(!entity_list_listed(entities_by_message(ENTITY_MESSAGE_HIT_TEST), mz) && !entity_list_listed(entities_by_message(ENTITY_MESSAGE_NEW_YEAR), mz) && !entity_list_listed(entities_by_message(ENTITY_MESSAGE_REFRESH), mz));
}

static void test_construction(void) {
    down_t made;
    const entity_list_t *mzs;
    int before;

    start_game();
    assert(entities_by_tag(TAG_PROBE) == NULL || entities_by_tag(TAG_PROBE)->count == 0);
    mzs = entities_by_tag(TAG_WORMHOLE);
    before = mzs == NULL ? 0 : mzs->count;
    made = background_left_press(NULL);
    assert(made.stop && made.held != NULL);
    assert(entity_list_listed(entities_by_tag(TAG_PROBE), made.held));
    mzs = entities_by_tag(TAG_WORMHOLE);
    assert((mzs == NULL ? 0 : mzs->count) == before);

    made = background_left_press(NULL);
    assert(made.stop && made.held != NULL);
    check_fresh_wormhole(made.held);
    check_fresh_wormhole(build_wormhole());
    check_fresh_wormhole(wormhole_new());
}

static uint32_t fold_anchors(uint32_t h, const wormhole_t *mz) {
    const entity_list_t *ox;
    int i;

    assert(mz->anchors != NULL);
    ox = mz->anchors;
    h = fnv(h, (uint32_t)ox->count);
    for (i = 0; i < ox->count; i++) {
        h = fnv(h, (uint32_t)entity_list_rank_in(live_entities(), ox->items[i]));
    }
    return h;
}

/* The anchors list holds the live entities with a non-zero fold_anchor,
 * in their order, never a wormhole. */
static void test_anchors(void) {
    uint32_t h = 2166136261u;
    wormhole_t *first;
    wormhole_t *second;
    planet_t *unlit = NULL;
    probe_t *probe;
    planet_t *lit;
    const entity_list_t *jos;
    int i;

    world();
    jos = entities_by_tag(TAG_PLANET);
    for (i = 0; unlit == NULL && jos != NULL && i < jos->count; i++) {
        planet_t *jo = as_planet(jos->items[i]);
        if (jo->fold_anchor == 0) {
            unlit = jo;
        }
    }
    assert(unlit != NULL);
    first = build_wormhole();
    h = fold_anchors(h, first);
    second = build_wormhole();
    h = fold_anchors(h, second);
    assert(second->anchors != NULL);
    assert(!entity_list_listed(second->anchors, first) && !entity_list_listed(second->anchors, unlit));
    printf("[anchors] %d entries\n", second->anchors->count);
    assert(second->anchors->count > 0);
    check("anchors", h, ANCHORS_HASH);

    /* A probe's fold_anchor is 1, from its archetype: it joins the anchors
     * list. */
    probe = probe_new(NULL);
    assert(entity_list_listed(build_wormhole()->anchors, probe));

    lit = structure_new(NULL);
    lit->fold_anchor = p8_fixed_from_int(1);
    assert(entity_list_listed(build_wormhole()->anchors, lit));
}

/* Bytes a registry has taken from the generation: capacities 4, 8, ... up
 * to its own, each growth leaving the old block dead. */
static size_t registry_bytes(const entity_list_t *list) {
    size_t bytes = 0;
    int k;
    for (k = 4; list != NULL && k <= list->capacity; k *= 2) {
        bytes += ZONE_CELLS((size_t)k * sizeof(entity_t *)) * sizeof(zone_cell_t);
    }
    return bytes;
}

static size_t generation_but_registries(void) {
    return game_zone_used() - registry_bytes(live_entities()) - registry_bytes(entities_by_message(ENTITY_MESSAGE_DRAW)) -
           registry_bytes(entities_by_message(ENTITY_MESSAGE_DRAW_OVERLAY)) - registry_bytes(entities_by_tag(TAG_WORMHOLE));
}

/* A built wormhole takes one grab arena, its anchors list reserved to the
 * exact count; once given back, the next wormhole takes that arena again. */
static void test_arena(void) {
    wormhole_t *mz;
    size_t before;
    int i;

    world();
    cart_flush_exit_queue();
    assert(!game_zone_pending());
    build_wormhole();
    before = generation_but_registries();
    mz = build_wormhole();
    printf("[arena] generation +%lu, anchors %d/%d\n", (unsigned long)(generation_but_registries() - before),
           mz->anchors->count, mz->anchors->capacity);
    assert(generation_but_registries() == before + arena_size(mz->anchors->zone));
    assert(arena_used(mz->anchors->zone) == arena_size(mz->anchors->zone) && "the arena holds the wormhole exactly");
    assert(mz->anchors->count > 0 && mz->anchors->count == mz->anchors->capacity);
    for (i = 1; i < mz->anchors->count; i++) {
        assert(entity_list_rank_in(live_entities(), mz->anchors->items[i - 1]) < entity_list_rank_in(live_entities(), mz->anchors->items[i]));
    }

    drop(mz);
    assert(mz->is_dead);
    update_entities();
    assert(!entity_list_listed(live_entities(), mz));
    cart_flush_exit_queue();
    before = generation_but_registries();
    build_wormhole();
    assert(generation_but_registries() == before && "a dead wormhole gives its grab arena back to the next grab");
}

/* The targeted_by of the wormhole's target, a slipway or the throwaway. */
static const void *target_targeted_by(const wormhole_t *mz) {
    entity_t *et = mz->targeting->target;

    assert(et != NULL);
    if (et->class == slipway_class()) {
        return ((const slipway_t *)et)->targeted_by;
    }
    return throwaway_targeted_by(et);
}

static uint32_t fold_drag(uint32_t h, const wormhole_t *mz, int want_kind) {
    const entity_t *et = mz->targeting->target;
    int kind = target_kind(mz);

    printf("[drag] target kind %d\n", kind);
    fflush(stdout);
    assert(kind == want_kind);
    assert(mz->header.pos == &mz->pos && mz->header.map_pos == &mz->map_pos);
    h = fold_vec(h, mz->pos);
    h = fold_vec(h, mz->map_pos);
    h = fnv(h, (uint32_t)kind);
    if (et != NULL) {
        if (et->class == wormhole_too_far_target_class() && ((const wormhole_too_far_target_t *)et)->status_text != NULL) {
            h = fnv_str(h, ((const wormhole_too_far_target_t *)et)->status_text);
        }
        if (et->class == slipway_too_far_class() && ((const slipway_too_far_t *)et)->status_text != NULL) {
            h = fnv_str(h, ((const slipway_too_far_t *)et)->status_text);
        }
        if (kind == TARGET_SLIPWAY || kind == TARGET_THROWAWAY) {
            assert(target_targeted_by(mz) == mz);
        }
    }
    assert(mz->anchor != NULL);
    return fnv(h, (uint32_t)entity_list_rank_in(live_entities(), mz->anchor));
}

/* A quarter of the slipway's length past its start end: near that
 * colonized anchor, off the slipway's segment. */
static vec_t near_anchor(void) {
    const entity_list_t *jcs = entities_by_tag(TAG_SLIPWAY);
    const slipway_t *jc;
    vec_t qu;
    vec_t re;

    assert(jcs != NULL && jcs->count == 1);
    jc = as_slipway(jcs->items[0]);
    qu = pos_of(jc->start);
    re = pos_of(jc->end);
    return vec_mul(vec_add(qu, vec_div(vec_sub(qu, re), p8_fixed_from_int(4))),
                         p8_fixed_from_int(128));
}

static void test_drag(void) {
    uint32_t h = 2166136261u;
    wormhole_t *mz;
    const entity_list_t *jcs;
    vec_t mid;

    world();
    mz = build_wormhole();
    jcs = entities_by_tag(TAG_SLIPWAY);
    assert(jcs != NULL && jcs->count > 0);
    mid = slipway_mid(as_slipway(jcs->items[0]));
    drag(mz, vec_ref_fixed(mid), vec_ref(64, 64));
    h = fold_drag(h, mz, TARGET_SLIPWAY);
    drag(mz, vec_ref(-640, -640), vec_ref(64, 64));
    h = fold_drag(h, mz, TARGET_TOO_FAR);
    drag(mz, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    h = fold_drag(h, mz, TARGET_FALSE);
    drag(mz, vec_ref_fixed(near_anchor()), vec_ref(2, 2));
    h = fold_drag(h, mz, TARGET_THROWAWAY);
    check("drag", h, DRAG_HASH);
}

/* The placing state grows the radius each update_entities tick; nothing
 * sets is_dead. */
static void test_update_placing_ticks(void) {
    uint32_t h = 2166136261u;
    wormhole_t *mz;
    int i;

    world();
    mz = build_wormhole();
    for (i = 0; i < UPDATE_PLACING_TICKS; i++) {
        update_entities();
        assert(is_dead_state(mz) == 0);
        assert(mz->header.state_time == p8_fixed_from_int(i + 1));
        h = fnv(h, (uint32_t)mz->radius);
    }
    assert(entity_list_listed(live_entities(), mz));
    check("update_placing", h, UPDATE_PLACING_HASH);
}

static uint32_t fold_drop(uint32_t h, const wormhole_t *mz) {
    h = fold_string(h, entity_state_name(mz->header.state));
    h = fnv(h, (uint32_t)mz->header.state_time);
    h = fnv(h, is_dead_state(mz));
    return fnv(h, (uint32_t)credits());
}

/* Before tick 15 targeting_confirm never runs; after it, a set target
 * refuses, an unset one pays the cost and turns to unfolding, and too few
 * credits refuse. */
static void test_drop(void) {
    uint32_t hash = 2166136261u;
    p8_fixed_t before;
    wormhole_t *early;
    wormhole_t *targeted;
    wormhole_t *paid;
    wormhole_t *broke;

    world();
    before = credits();

    early = build_wormhole();
    drop(early);
    assert(is_dead_state(early) == 2 && credits() == before);
    hash = fold_drop(hash, early);

    targeted = build_wormhole();
    until_state_time(targeted, 15);
    drag(targeted, vec_ref(-640, -640), vec_ref(64, 64));
    assert(target_kind(targeted) == TARGET_TOO_FAR);
    drop(targeted);
    assert(is_dead_state(targeted) == 2 && credits() == before);
    hash = fold_drop(hash, targeted);

    paid = build_wormhole();
    until_state_time(paid, 15);
    drag(paid, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    assert(target_kind(paid) == TARGET_FALSE);
    drop(paid);
    assert(is_state(&paid->header, ENTITY_STATE_UNFOLDING) && paid->header.state_time == 0 && is_dead_state(paid) == 0);
    assert(credits() == p8_fixed_sub(before, p8_fixed_from_int(3)));
    hash = fold_drop(hash, paid);

    broke = build_wormhole();
    until_state_time(broke, 15);
    drag(broke, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    main_economy_typed()->credits = p8_fixed_from_int(2);
    drop(broke);
    assert(is_state(&broke->header, ENTITY_STATE_PLACING) && is_dead_state(broke) == 2);
    assert(credits() == p8_fixed_from_int(2));
    hash = fold_drop(hash, broke);
    check("drop", hash, DROP_HASH);
}

static int unknown_count(void) {
    const entity_list_t *jo = entities_by_tag(TAG_PLANET);
    int n = 0;
    int i;
    for (i = 0; jo != NULL && i < jo->count; i++) {
        if (is_state(&as_planet(jo->items[i])->header, ENTITY_STATE_UNKNOWN)) {
            n++;
        }
    }
    return n;
}

/* A paid wormhole put on an unknown planet: the unfolding state reveals
 * around it until a state time past 25 takes it out of the live entities. */
static wormhole_t *paid_wormhole_on_unknown(void) {
    wormhole_t *mz = build_wormhole();
    planet_t *unknown = first_planet_with_state(ENTITY_STATE_UNKNOWN);
    vec_t *p = malloc(sizeof *p);

    assert(unknown != NULL);
    until_state_time(mz, 15);
    drag(mz, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    assert(target_kind(mz) == TARGET_FALSE);
    drop(mz);
    assert(is_state(&mz->header, ENTITY_STATE_UNFOLDING));
    assert(unknown->map_pos != NULL);
    *p = *unknown->map_pos;
    mz->map_pos = p;
    return mz;
}

static void test_update_unfolding_ticks(void) {
    uint32_t h = 2166136261u;
    wormhole_t *mz;
    int tick = 0;
    int revealed = 0;

    world();
    mz = paid_wormhole_on_unknown();
    while (entity_list_listed(live_entities(), mz)) {
        int before = unknown_count();
        update_entities();
        tick++;
        assert(tick < 100);
        revealed += before - unknown_count();
        h = fnv(h, (uint32_t)(before - unknown_count()));
        h = fnv(h, is_dead_state(mz));
        h = fnv(h, (uint32_t)entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW), mz));
        h = fnv(h, (uint32_t)entity_list_listed(entities_by_tag(TAG_WORMHOLE), mz));
    }
    printf("[unfolding] left live_entities after %d ticks, %d planets revealed\n", tick, revealed);
    assert(revealed > 0);
    assert(!entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW), mz) && !entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW_OVERLAY), mz) && !entity_list_listed(entities_by_tag(TAG_WORMHOLE), mz));
    check_int("update_unfolding_exit_tick", tick, UPDATE_UNFOLDING_EXIT_TICK);
    check("update_unfolding", h, UPDATE_UNFOLDING_HASH);
}

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    for (int y = 0; y < P8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
            h ^= (uint32_t)memory.screen[y][x];
            h *= 16777619u;
        }
    }
    return h;
}

/* The draw method is given the screen centre rather than pos, which is in
 * world coordinates, off the screen without a camera. */
static uint32_t draw(void *mz) {
    const entity_class_t *cls = ((const entity_t *)mz)->class;

    p8_gfx_init(&graphics);
    p8_cls(&memory);
    cls->draw(mz, vec_ref(64, 64));
    cls->draw_overlay(mz, NULL);
    return screen_hash();
}

/* Ring, anchor line and sprite while placing, the "-too far-" label of the
 * too-far target, the coloured rings while unfolding. */
static void test_draw(void) {
    uint32_t hash = 2166136261u;
    uint32_t blank;
    wormhole_t *bare;
    wormhole_t *far;
    wormhole_t *mz;
    int t;

    world();
    p8_gfx_init(&graphics);
    p8_cls(&memory);
    blank = screen_hash();

    bare = build_wormhole();
    until_state_time(bare, 5);
    drag(bare, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    assert(target_kind(bare) == TARGET_FALSE);
    assert(draw(bare) != blank);
    hash = fnv(hash, draw(bare));

    far = build_wormhole();
    until_state_time(far, 5);
    drag(far, vec_ref(-640, -640), vec_ref(64, 64));
    assert(target_kind(far) == TARGET_TOO_FAR);
    assert(draw(far) != draw(bare));
    hash = fnv(hash, draw(far));

    mz = paid_wormhole_on_unknown();
    for (t = 1; t <= 20; t++) {
        update_entities();
        if (t == 4 || t == 9 || t == 20) {
            uint32_t d = draw(mz);
            assert(d != blank);
            hash = fnv(hash, d);
        }
    }
    check("draw", hash, DRAW_HASH);
}

/* Released over a colonized planet or a slipgate, accept_drop looks for a
 * held slipway's start, which a wormhole never has: stopped, nothing held. */
static void test_accept_drop(void) {
    wormhole_t *mz;
    planet_t *ly;
    void *nr;
    planet_keys_t keys;
    down_t r;

    world();
    mz = build_wormhole();
    ly = first_planet_with_state(ENTITY_STATE_SETTLED);
    assert(ly != NULL);
    r = planet_accept_drop(ly, &mz->header);
    assert(r.stop && r.held == NULL);

    keys.planet_id = NULL;
    keys.map_pos = ly->map_pos;
    keys.pos = ly->pos;
    nr = slipgate_new(&keys);
    r = slipgate_accept_drop(nr, &mz->header);
    assert(r.stop && r.held == NULL);
}

/* The wormhole's too-far target: status_text and nothing else, never
 * written. targeting_track puts targeted_by on the target it has just
 * computed, never on the too-far target a previous drag left there. */
static void check_too_far_target_untouched(void) {
    const wormhole_too_far_target_t *wrapper = wormhole_too_far_target_wrapper();

    assert(wrapper != NULL);
    assert(wrapper->status_text != NULL && strcmp(wrapper->status_text, "-too far-") == 0);
}

static void check_targeted_by_is(const wormhole_t *mz) {
    assert(target_targeted_by(mz) == mz);
}

static void test_too_far_target_wrapper(void) {
    void *wrapper;
    wormhole_t *wormhole;
    wormhole_t *other;
    const entity_list_t *jcs;
    vec_t mid;

    world();
    wrapper = (void *)wormhole_too_far_target_wrapper();
    check_too_far_target_untouched();
    assert(((const entity_t *)wrapper)->class->draw == NULL &&
           ((const entity_t *)wrapper)->class->draw_overlay == NULL &&
           ((const entity_t *)wrapper)->class->drag == NULL);

    wormhole = build_wormhole();
    drag(wormhole, vec_ref(-640, -640), vec_ref(64, 64));
    assert(target_kind(wormhole) == TARGET_TOO_FAR);
    check_too_far_target_untouched();

    jcs = entities_by_tag(TAG_SLIPWAY);
    assert(jcs != NULL && jcs->count > 0);
    mid = slipway_mid(as_slipway(jcs->items[0]));
    drag(wormhole, vec_ref_fixed(mid), vec_ref(64, 64));
    assert(target_kind(wormhole) == TARGET_SLIPWAY);
    check_targeted_by_is(wormhole);
    check_too_far_target_untouched();

    drag(wormhole, vec_ref_fixed(near_anchor()), vec_ref(2, 2));
    assert(target_kind(wormhole) == TARGET_THROWAWAY);
    check_targeted_by_is(wormhole);
    check_too_far_target_untouched();

    drag(wormhole, vec_ref(-640, -640), vec_ref(64, 64));
    assert(target_kind(wormhole) == TARGET_TOO_FAR);
    check_too_far_target_untouched();

    other = build_wormhole();
    drag(other, vec_ref(640, 640), vec_ref(64, 64));
    assert(target_kind(other) == TARGET_TOO_FAR);
    assert(wormhole->targeting->target != NULL && other->targeting->target != NULL);
    assert(wormhole->targeting->target == other->targeting->target && (void *)wormhole->targeting->target == wrapper);
    check_too_far_target_untouched();
}

static int is_a_struct_of(const void *e, const entity_class_t *cls) {
    return e != NULL && ((const entity_t *)e)->class == cls;
}

static int is_a_wormhole_struct(const void *e) {
    return is_a_struct_of(e, wormhole_class());
}

/* Built through the background's left_press or directly, a wormhole is a
 * struct whose class has drag, drop, draw, draw_overlay and its two state
 * methods; the too-far target is a struct too, the very object of the
 * too_far_target member. */
static void test_wormhole_is_a_struct(void) {
    down_t made;
    wormhole_t *mz;

    start_game();
    (void)background_left_press(NULL);
    made = background_left_press(NULL);
    mz = wormhole_new();
    assert(is_a_wormhole_struct(made.held) && is_a_wormhole_struct(build_wormhole()) && is_a_wormhole_struct(mz));
    assert(wormhole_class()->drag != NULL && wormhole_class()->drop != NULL);
    assert(wormhole_class()->draw_at == NULL);
    assert(wormhole_class()->states[ENTITY_STATE_PLACING] != NULL &&
           wormhole_class()->states[ENTITY_STATE_UNFOLDING] != NULL && wormhole_class()->draw != NULL &&
           wormhole_class()->draw_overlay != NULL);
    assert(wormhole_class()->states[ENTITY_STATE_NORMAL] == NULL && wormhole_class()->left_press == NULL);

    assert(is_a_struct_of(wormhole_too_far_target_wrapper(), wormhole_too_far_target_class()));
    assert(mz->too_far_target == wormhole_too_far_target_wrapper());
}

/* The galaxy depends on the rnd state: run each scenario in a child forked
 * right after cart_load(), as a fresh process would start it. */
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
    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);

    cart_load();
    in_fresh_child(test_construction);
    in_fresh_child(test_anchors);
    in_fresh_child(test_arena);
    in_fresh_child(test_drag);
    in_fresh_child(test_update_placing_ticks);
    in_fresh_child(test_drop);
    in_fresh_child(test_update_unfolding_ticks);
    in_fresh_child(test_draw);
    in_fresh_child(test_accept_drop);
    in_fresh_child(test_too_far_target_wrapper);
    in_fresh_child(test_wormhole_is_a_struct);

    printf("test_wormhole: all tests passed\n");
    return 0;
}
