/* The wormhole `mz`, read and written only through the dual-form field API
 * and driven through the real `mv`/`dg`, in a game built directly: no
 * events file. */

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
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
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
 * paying effects to leave `hv`: `dg` skips the entity after one it takes out
 * (TROU-5). */
#define SETTLE_TICKS 120

#define OX_HASH 0xC6D8AF55u
#define HQ_HASH 0x9DAA980Cu
#define KZ_TICKS 21
#define KZ_HASH 0xE16D1D52u
#define EY_HASH 0x86F499FFu
#define PR_HASH 0x85EB1EC6u
#define PR_EXIT_TICK 27
#define DRAW_HASH 0xD124A50Cu

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

static uint32_t fnv_str(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFu);
}

static int ents_rank_in(const entity_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int ents_listed(const entity_list_t *list, const void *e) {
    return ents_rank_in(list, e) >= 0;
}

static int is_me(const entity_t *ke, const char *want) {
    return ke->state != NULL && strcmp(ke->state, want) == 0;
}

static pico8_mz_t *as_mz(void *e) {
    assert(e != NULL &&
           ((const entity_t *)e)->class == pico8_mz_struct_class());
    return e;
}

static pico8_jc_t *as_jc(void *e) {
    assert(e != NULL &&
           ((const entity_t *)e)->class == pico8_jc_struct_class());
    return e;
}

static pico8_jo_t *as_jo(void *e) {
    assert(pico8_jo_is_family(e));
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
static uint32_t p__state(const pico8_mz_t *mz) {
    return mz->p_ ? 2 : 0;
}

static uint32_t fold_string(uint32_t h, const char *s) {
    if (s == NULL) {
        return fnv(h, 0xDEADu);
    }
    return fnv_str(h, s);
}

enum { ET_ABSENT, ET_FALSE, ET_JJ, ET_JC, ET_JO, ET_THROWAWAY, ET_OTHER };

static int is_bv_target(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_bv_et_struct_class();
}

/* The throwaway target's `fo`, NULL when absent. */
static const void *bv_et_fo(const void *t) {
    assert(is_bv_target(t));
    return ((const pico8_bv_et_t *)t)->fo;
}

static int et_kind(const pico8_mz_t *mz) {
    const void *et = mz->probe->et;

    if (et == NULL) {
        return ET_FALSE;
    }
    if (et == (const void *)pico8_mz_jj_wrapper()) {
        return ET_JJ;
    }
    if (ents_listed(entities_by_tag("slipway"), et)) {
        return ET_JC;
    }
    if (ents_listed(entities_by_tag("planet"), et)) {
        return ET_JO;
    }
    if (is_bv_target(et)) {
        return ET_THROWAWAY;
    }
    return ET_OTHER;
}

static p8_fixed_t credits(void) {
    const pico8_hy_t *hy = pico8_fm_hy();
    assert(hy != NULL);
    return hy->nz;
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

static vec_t pos_of(void *e) {
    const pico8_jo_t *jo = as_jo(e);
    assert(jo->p != NULL);
    return *jo->p;
}

static p8_fixed_t dist2(vec_t a, vec_t b) {
    vec_t d = vec_sub(a, b);
    return p8_fixed_add(p8_fixed_mul(d.x, d.x), p8_fixed_mul(d.y, d.y));
}

/* Two generated planets colonized and joined by a paid slipway, the rest
 * of the galaxy left unknown, then left to settle. `jc:hq` reads the hovered planet off
 * `ok.o_`. */
static void world(void) {
    const entity_list_t *jos;
    pico8_jc_keys_t jc_keys = {0};
    vec_t *mp = malloc(sizeof *mp);
    void *qu;
    void *re = NULL;
    pico8_jc_t *jc;
    int i;

    start_game();
    jos = entities_by_tag("planet");
    assert(jos != NULL && jos->count > 1);
    qu = jos->items[0];
    for (i = 1; i < jos->count; i++) {
        void *e = jos->items[i];
        if (re == NULL || dist2(pos_of(e), pos_of(qu)) < dist2(pos_of(re), pos_of(qu))) {
            re = e;
        }
    }
    pico8_jo_hu(qu, pico8_hp_new(NULL));
    pico8_jo_hu(re, pico8_hp_new(NULL));

    jc_keys.qu = qu;
    jc = pico8_jc_new(&jc_keys);
    *mp = vec_add(vec_mul(pos_of(re), p8_fixed_from_int(128)),
                        vec_make(p8_fixed_from_int(1), p8_fixed_from_int(1)));
    pico8_ok()->o_ = re;
    pico8_jc_struct_class()->drag(&jc->header, mp, NULL);
    pico8_ok()->o_ = NULL;
    while (jc->header.state_time < p8_fixed_from_int(15)) {
        update_entities();
    }
    pico8_jc_struct_class()->drop(&jc->header, re);
    assert((void *)jc->re == re);
    for (i = 0; i < SETTLE_TICKS; i++) {
        update_entities();
    }
}

static pico8_mz_t *build_mz(void) {
    return pico8_mz_new();
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

static void hq(void *mz, vec_t *mp, vec_t *rp) {
    pico8_mz_struct_class()->drag(mz, mp, rp);
}

static void ey(void *mz) {
    pico8_mz_struct_class()->drop(mz, NULL);
}

/* `dg` skips the entity after one it takes out of `hv` (TROU-5): count
 * ticks by `t`, not by calls. */
static void until_t(const pico8_mz_t *mz, int t) {
    while (mz->header.state_time < p8_fixed_from_int(t)) {
        update_entities();
    }
}

static pico8_jo_t *first_jo_with_me(const char *me) {
    const entity_list_t *list = entities_by_tag("planet");
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        pico8_jo_t *jo = as_jo(list->items[i]);
        if (is_me(&jo->header, me)) {
            return jo;
        }
    }
    return NULL;
}

/* The middle of `jc`, in screen units. */
static vec_t slipway_mid(pico8_jc_t *jc) {
    return vec_mul(vec_div(vec_add(pos_of(jc->qu), pos_of(jc->re)), p8_fixed_from_int(2)),
                         p8_fixed_from_int(128));
}

/* A fresh `mz`: the spec's defaults, ke's `t`, nothing written yet but
 * `ox`, registered like any `ke` entity with `kj` and `ev`. */
static void check_fresh_mz(void *e) {
    const pico8_mz_t *mz = as_mz(e);
    const int_seq_t *kk;
    const pico8_mz_jj_t *jj;
    static const int want_kk[9] = {7, 7, 6, 13, 5, 1, 1, 1, 0};
    int i;

    assert(is_me(&mz->header, "placing"));
    assert(mz->header.state_time == 0);
    assert(mz->probe->def->attach_distance == 0);
    assert(mz->probe->def->confirmed_state != NULL && strcmp(mz->probe->def->confirmed_state, "unfolding") == 0);
    assert(mz->def->spawn_snd == p8_fixed_from_int(52));
    assert(mz->probe->def->status_sprite == p8_fixed_from_int(49) && mz->probe->def->has_status_sprite == 1);
    assert(mz->oi == 0);
    assert(mz->probe->od == p8_fixed_from_int(3));
    assert(mz->probe->def->clock_ticks == p8_fixed_from_int(1));
    assert(mz->def->layer == p8_fixed_from_int(5));
    assert(mz->header.layer == &mz->def->layer);

    kk = mz->def->color_ramp;
    assert(kk == &wormhole_color_ramp_seq && kk->count == 9);
    for (i = 0; i < 9; i++) {
        assert(kk->items[i] == want_kk[i]);
    }
    assert(mz->header.class->tags == TAG(TAG_WORMHOLE));
    jj = mz->jj;
    assert(jj != NULL && jj->qs != NULL && strcmp(jj->qs, "-too far-") == 0);
    assert(pico8_mz_jj_wrapper() == jj);

    assert(mz->ra == NULL && mz->p == NULL && mz->probe->et == NULL && mz->ij == NULL && !mz->p_);
    assert(mz->ox != NULL);

    assert(ents_listed(live_entities(), mz) && ents_listed(entities_by_message("draw"), mz) && ents_listed(entities_by_message("draw_overlay"), mz));
    assert(ents_listed(entities_by_tag("wormhole"), mz));
    assert(!ents_listed(entities_by_message("hit_test"), mz) && !ents_listed(entities_by_message("new_year"), mz) && !ents_listed(entities_by_message("refresh"), mz));
}

static void test_construction(void) {
    down_t made;
    const entity_list_t *mzs;
    int before;

    start_game();
    assert(entities_by_tag("probe") == NULL || entities_by_tag("probe")->count == 0);
    mzs = entities_by_tag("wormhole");
    before = mzs == NULL ? 0 : mzs->count;
    made = pico8_bg_ldown(NULL);
    assert(made.stop && made.held != NULL);
    assert(ents_listed(entities_by_tag("probe"), made.held));
    mzs = entities_by_tag("wormhole");
    assert((mzs == NULL ? 0 : mzs->count) == before);

    made = pico8_bg_ldown(NULL);
    assert(made.stop && made.held != NULL);
    check_fresh_mz(made.held);
    check_fresh_mz(build_mz());
    check_fresh_mz(pico8_mz_new());
}

static uint32_t fold_ox(uint32_t h, const pico8_mz_t *mz) {
    const entity_list_t *ox;
    int i;

    assert(mz->ox != NULL);
    ox = mz->ox;
    h = fnv(h, (uint32_t)ox->count);
    for (i = 0; i < ox->count; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(live_entities(), ox->items[i]));
    }
    return h;
}

/* `ox` holds the `fx` entities of `hv` in its order, never an `mz`. */
static void test_ox(void) {
    uint32_t h = 2166136261u;
    pico8_mz_t *first;
    pico8_mz_t *second;
    pico8_jo_t *unlit = NULL;
    pico8_g__t *probe;
    pico8_jo_t *lit;
    const entity_list_t *jos;
    int i;

    world();
    jos = entities_by_tag("planet");
    for (i = 0; unlit == NULL && jos != NULL && i < jos->count; i++) {
        pico8_jo_t *jo = as_jo(jos->items[i]);
        if (jo->fx == 0) {
            unlit = jo;
        }
    }
    assert(unlit != NULL);
    first = build_mz();
    h = fold_ox(h, first);
    second = build_mz();
    h = fold_ox(h, second);
    assert(second->ox != NULL);
    assert(!ents_listed(second->ox, first) && !ents_listed(second->ox, unlit));
    printf("[ox] %d entries\n", second->ox->count);
    assert(second->ox->count > 0);
    check("ox", h, OX_HASH);

    /* A probe's `fx` is a fixed, true in Lua even at 0: it joins `ox`. */
    probe = pico8_g__new(NULL);
    assert(ents_listed(build_mz()->ox, probe));

    lit = pico8_qh_new(NULL);
    lit->fx = p8_fixed_from_int(1);
    assert(ents_listed(build_mz()->ox, lit));
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
    return game_zone_used() - registry_bytes(live_entities()) - registry_bytes(entities_by_message("draw")) -
           registry_bytes(entities_by_message("draw_overlay")) - registry_bytes(entities_by_tag("wormhole"));
}

/* A built `mz` takes one grab arena, `ox` reserved to the exact count;
 * once given back, the next `mz` takes that arena again. */
static void test_arena(void) {
    pico8_mz_t *mz;
    size_t before;
    int i;

    world();
    pico8_cart_vider_file_sortie();
    assert(!game_zone_pending());
    build_mz();
    before = generation_but_registries();
    mz = build_mz();
    printf("[arena] generation +%lu, ox %d/%d\n", (unsigned long)(generation_but_registries() - before),
           mz->ox->count, mz->ox->capacity);
    assert(generation_but_registries() == before + PICO8_JO_PRISE_OCTETS);
    assert(mz->ox->count > 0 && mz->ox->count == mz->ox->capacity);
    for (i = 1; i < mz->ox->count; i++) {
        assert(ents_rank_in(live_entities(), mz->ox->items[i - 1]) < ents_rank_in(live_entities(), mz->ox->items[i]));
    }

    ey(mz);
    assert(mz->p_);
    update_entities();
    assert(!ents_listed(live_entities(), mz));
    pico8_cart_vider_file_sortie();
    before = generation_but_registries();
    build_mz();
    assert(generation_but_registries() == before && "a dead mz gives its grab arena back to the next grab");
}

/* The `fo` of the `mz`'s `et`, a slipway or the throwaway. */
static const void *et_fo(const pico8_mz_t *mz) {
    entity_t *et = mz->probe->et;

    assert(et != NULL);
    if (et->class == pico8_jc_struct_class()) {
        return ((const pico8_jc_t *)et)->fo;
    }
    return bv_et_fo(et);
}

static uint32_t fold_hq(uint32_t h, const pico8_mz_t *mz, int want_kind) {
    const entity_t *et = mz->probe->et;
    int kind = et_kind(mz);

    printf("[hq] et kind %d\n", kind);
    fflush(stdout);
    assert(kind == want_kind);
    assert(mz->header.pos == &mz->ra && mz->header.map_pos == &mz->p);
    h = fold_vec(h, mz->ra);
    h = fold_vec(h, mz->p);
    h = fnv(h, (uint32_t)kind);
    if (et != NULL) {
        if (et->class == pico8_mz_jj_struct_class() && ((const pico8_mz_jj_t *)et)->qs != NULL) {
            h = fnv_str(h, ((const pico8_mz_jj_t *)et)->qs);
        }
        if (et->class == pico8_jc_too_far_struct_class() && ((const pico8_jc_too_far_t *)et)->qs != NULL) {
            h = fnv_str(h, ((const pico8_jc_too_far_t *)et)->qs);
        }
        if (kind == ET_JC || kind == ET_THROWAWAY) {
            assert(et_fo(mz) == mz);
        }
    }
    assert(mz->ij != NULL);
    return fnv(h, (uint32_t)ents_rank_in(live_entities(), mz->ij));
}

/* A quarter of the slipway's length past its `qu` end: near that
 * colonized anchor, off the slipway's segment. */
static vec_t near_anchor(void) {
    const entity_list_t *jcs = entities_by_tag("slipway");
    const pico8_jc_t *jc;
    vec_t qu;
    vec_t re;

    assert(jcs != NULL && jcs->count == 1);
    jc = as_jc(jcs->items[0]);
    qu = pos_of(jc->qu);
    re = pos_of(jc->re);
    return vec_mul(vec_add(qu, vec_div(vec_sub(qu, re), p8_fixed_from_int(4))),
                         p8_fixed_from_int(128));
}

static void test_hq(void) {
    uint32_t h = 2166136261u;
    pico8_mz_t *mz;
    const entity_list_t *jcs;
    vec_t mid;

    world();
    mz = build_mz();
    jcs = entities_by_tag("slipway");
    assert(jcs != NULL && jcs->count > 0);
    mid = slipway_mid(as_jc(jcs->items[0]));
    hq(mz, vec_ref_fixed(mid), vec_ref(64, 64));
    h = fold_hq(h, mz, ET_JC);
    hq(mz, vec_ref(-640, -640), vec_ref(64, 64));
    h = fold_hq(h, mz, ET_JJ);
    hq(mz, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    h = fold_hq(h, mz, ET_FALSE);
    hq(mz, vec_ref_fixed(near_anchor()), vec_ref(2, 2));
    h = fold_hq(h, mz, ET_THROWAWAY);
    check("drag", h, HQ_HASH);
}

/* `kz` grows `oi` each `dg` tick; nothing sets `p_`. */
static void test_kz_ticks(void) {
    uint32_t h = 2166136261u;
    pico8_mz_t *mz;
    int i;

    world();
    mz = build_mz();
    for (i = 0; i < KZ_TICKS; i++) {
        update_entities();
        assert(p__state(mz) == 0);
        assert(mz->header.state_time == p8_fixed_from_int(i + 1));
        h = fnv(h, (uint32_t)mz->oi);
    }
    assert(ents_listed(live_entities(), mz));
    check("kz ticks", h, KZ_HASH);
}

static uint32_t fold_ey(uint32_t h, const pico8_mz_t *mz) {
    h = fold_string(h, mz->header.state);
    h = fnv(h, (uint32_t)mz->header.state_time);
    h = fnv(h, p__state(mz));
    return fnv(h, (uint32_t)credits());
}

/* Before tick 15 `d_` never runs; after it, a set `et` refuses, an unset
 * one pays `od` and turns to `pr`, and too few credits refuse. */
static void test_ey(void) {
    uint32_t h = 2166136261u;
    p8_fixed_t before;
    pico8_mz_t *early;
    pico8_mz_t *targeted;
    pico8_mz_t *paid;
    pico8_mz_t *broke;

    world();
    before = credits();

    early = build_mz();
    ey(early);
    assert(p__state(early) == 2 && credits() == before);
    h = fold_ey(h, early);

    targeted = build_mz();
    until_t(targeted, 15);
    hq(targeted, vec_ref(-640, -640), vec_ref(64, 64));
    assert(et_kind(targeted) == ET_JJ);
    ey(targeted);
    assert(p__state(targeted) == 2 && credits() == before);
    h = fold_ey(h, targeted);

    paid = build_mz();
    until_t(paid, 15);
    hq(paid, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    assert(et_kind(paid) == ET_FALSE);
    ey(paid);
    assert(is_me(&paid->header, "unfolding") && paid->header.state_time == 0 && p__state(paid) == 0);
    assert(credits() == p8_fixed_sub(before, p8_fixed_from_int(3)));
    h = fold_ey(h, paid);

    broke = build_mz();
    until_t(broke, 15);
    hq(broke, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    pico8_fm_hy()->nz = p8_fixed_from_int(2);
    ey(broke);
    assert(is_me(&broke->header, "placing") && p__state(broke) == 2);
    assert(credits() == p8_fixed_from_int(2));
    h = fold_ey(h, broke);
    check("drop", h, EY_HASH);
}

static int unknown_count(void) {
    const entity_list_t *jo = entities_by_tag("planet");
    int n = 0;
    int i;
    for (i = 0; jo != NULL && i < jo->count; i++) {
        if (is_me(&as_jo(jo->items[i])->header, "unknown")) {
            n++;
        }
    }
    return n;
}

/* A paid `mz` put on an unknown planet: `pr` reveals around it until
 * `t>25` takes it out of `hv`. */
static pico8_mz_t *paid_mz_on_unknown(void) {
    pico8_mz_t *mz = build_mz();
    pico8_jo_t *unknown = first_jo_with_me("unknown");
    vec_t *p = malloc(sizeof *p);

    assert(unknown != NULL);
    until_t(mz, 15);
    hq(mz, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    assert(et_kind(mz) == ET_FALSE);
    ey(mz);
    assert(is_me(&mz->header, "unfolding"));
    assert(unknown->p != NULL);
    *p = *unknown->p;
    mz->p = p;
    return mz;
}

static void test_pr_ticks(void) {
    uint32_t h = 2166136261u;
    pico8_mz_t *mz;
    int tick = 0;
    int revealed = 0;

    world();
    mz = paid_mz_on_unknown();
    while (ents_listed(live_entities(), mz)) {
        int before = unknown_count();
        update_entities();
        tick++;
        assert(tick < 100);
        revealed += before - unknown_count();
        h = fnv(h, (uint32_t)(before - unknown_count()));
        h = fnv(h, p__state(mz));
        h = fnv(h, (uint32_t)ents_listed(entities_by_message("draw"), mz));
        h = fnv(h, (uint32_t)ents_listed(entities_by_tag("wormhole"), mz));
    }
    printf("[pr] left hv after %d ticks, %d planets revealed\n", tick, revealed);
    assert(revealed > 0);
    assert(!ents_listed(entities_by_message("draw"), mz) && !ents_listed(entities_by_message("draw_overlay"), mz) && !ents_listed(entities_by_tag("wormhole"), mz));
    check_int("pr exit tick", tick, PR_EXIT_TICK);
    check("pr ticks", h, PR_HASH);
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

/* `kj` is given the screen centre rather than `ra`, which is in world
 * coordinates, off the uncamera'd screen. */
static uint32_t draw(void *mz) {
    const entity_class_t *cls = ((const entity_t *)mz)->class;

    p8_gfx_init(&gfx);
    p8_cls(&mem);
    cls->draw(mz, vec_ref(64, 64));
    cls->draw_overlay(mz, NULL);
    return screen_hash();
}

/* Ring, anchor line and sprite while `kz`, the "-too far-" label on the
 * `jj` wrapper, the coloured rings of `pr`. */
static void test_draw(void) {
    uint32_t h = 2166136261u;
    uint32_t blank;
    pico8_mz_t *bare;
    pico8_mz_t *far;
    pico8_mz_t *mz;
    int t;

    world();
    p8_gfx_init(&gfx);
    p8_cls(&mem);
    blank = screen_hash();

    bare = build_mz();
    until_t(bare, 5);
    hq(bare, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    assert(et_kind(bare) == ET_FALSE);
    assert(draw(bare) != blank);
    h = fnv(h, draw(bare));

    far = build_mz();
    until_t(far, 5);
    hq(far, vec_ref(-640, -640), vec_ref(64, 64));
    assert(et_kind(far) == ET_JJ);
    assert(draw(far) != draw(bare));
    h = fnv(h, draw(far));

    mz = paid_mz_on_unknown();
    for (t = 1; t <= 20; t++) {
        update_entities();
        if (t == 4 || t == 9 || t == 20) {
            uint32_t d = draw(mz);
            assert(d != blank);
            h = fnv(h, d);
        }
    }
    check("draw", h, DRAW_HASH);
}

/* Released over a colonized planet or a slipgate, `jz("gv",held)` reads
 * `s.qu` off the `mz`, which it never has. */
static void test_gv(void) {
    pico8_mz_t *mz;
    pico8_jo_t *ly;
    void *nr;
    planet_keys_t keys;
    down_t r;

    world();
    mz = build_mz();
    ly = first_jo_with_me("settled");
    assert(ly != NULL);
    r = pico8_jo_gv(ly, &mz->header);
    assert(r.stop && r.held == NULL);

    keys.planet_id = NULL;
    keys.map_pos = ly->p;
    keys.pos = ly->ra;
    nr = pico8_nr_new(&keys);
    r = pico8_nr_gv(nr, &mz->header);
    assert(r.stop && r.held == NULL);
}

/* `mz.jj` as the cart has it: `qs` and nothing else, never written. `bv`
 * puts `fo` on the target it has just computed, never on the wrapper a
 * previous `hq` left in `et`. */
static void check_jj_untouched(void) {
    const pico8_mz_jj_t *wrapper = pico8_mz_jj_wrapper();

    assert(wrapper != NULL);
    assert(wrapper->qs != NULL && strcmp(wrapper->qs, "-too far-") == 0);
}

static void check_fo_is(const pico8_mz_t *mz) {
    assert(et_fo(mz) == mz);
}

static void test_jj_wrapper(void) {
    void *wrapper;
    pico8_mz_t *mz;
    pico8_mz_t *other;
    const entity_list_t *jcs;
    vec_t mid;

    world();
    wrapper = (void *)pico8_mz_jj_wrapper();
    check_jj_untouched();
    assert(((const entity_t *)wrapper)->class->draw == NULL &&
           ((const entity_t *)wrapper)->class->draw_overlay == NULL &&
           ((const entity_t *)wrapper)->class->drag == NULL);

    mz = build_mz();
    hq(mz, vec_ref(-640, -640), vec_ref(64, 64));
    assert(et_kind(mz) == ET_JJ);
    check_jj_untouched();

    jcs = entities_by_tag("slipway");
    assert(jcs != NULL && jcs->count > 0);
    mid = slipway_mid(as_jc(jcs->items[0]));
    hq(mz, vec_ref_fixed(mid), vec_ref(64, 64));
    assert(et_kind(mz) == ET_JC);
    check_fo_is(mz);
    check_jj_untouched();

    hq(mz, vec_ref_fixed(near_anchor()), vec_ref(2, 2));
    assert(et_kind(mz) == ET_THROWAWAY);
    check_fo_is(mz);
    check_jj_untouched();

    hq(mz, vec_ref(-640, -640), vec_ref(64, 64));
    assert(et_kind(mz) == ET_JJ);
    check_jj_untouched();

    other = build_mz();
    hq(other, vec_ref(640, 640), vec_ref(64, 64));
    assert(et_kind(other) == ET_JJ);
    assert(mz->probe->et != NULL && other->probe->et != NULL);
    assert(mz->probe->et == other->probe->et && (void *)mz->probe->et == wrapper);
    check_jj_untouched();
}

static int is_a_struct_of(const void *e, const entity_class_t *cls) {
    return e != NULL && ((const entity_t *)e)->class == cls;
}

static int is_an_mz_struct(const void *e) {
    return is_a_struct_of(e, pico8_mz_struct_class());
}

/* Built through `bg:ldown` or directly, an `mz` is a
 * struct with its six methods; the `jj` wrapper is a struct too, the very
 * object of the `jj` member. */
static void test_mz_is_a_struct(void) {
    down_t made;
    pico8_mz_t *mz;

    start_game();
    (void)pico8_bg_ldown(NULL);
    made = pico8_bg_ldown(NULL);
    mz = pico8_mz_new();
    assert(is_an_mz_struct(made.held) && is_an_mz_struct(build_mz()) && is_an_mz_struct(mz));
    assert(pico8_mz_struct_class()->drag != NULL && pico8_mz_struct_class()->drop != NULL);
    assert(pico8_mz_struct_class()->draw_at == NULL);
    assert(pico8_mz_struct_class()->states[ENTITY_STATE_PLACING] != NULL &&
           pico8_mz_struct_class()->states[ENTITY_STATE_UNFOLDING] != NULL && pico8_mz_struct_class()->draw != NULL &&
           pico8_mz_struct_class()->draw_overlay != NULL);
    assert(pico8_mz_struct_class()->states[ENTITY_STATE_NORMAL] == NULL && pico8_mz_struct_class()->left_press == NULL);

    assert(is_a_struct_of(pico8_mz_jj_wrapper(), pico8_mz_jj_struct_class()));
    assert(mz->jj == pico8_mz_jj_wrapper());
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
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);

    pico8_cart_load();
    in_fresh_child(test_construction);
    in_fresh_child(test_ox);
    in_fresh_child(test_arena);
    in_fresh_child(test_hq);
    in_fresh_child(test_kz_ticks);
    in_fresh_child(test_ey);
    in_fresh_child(test_pr_ticks);
    in_fresh_child(test_draw);
    in_fresh_child(test_gv);
    in_fresh_child(test_jj_wrapper);
    in_fresh_child(test_mz_is_a_struct);

    printf("test_pico8_mz: all tests passed\n");
    return 0;
}
