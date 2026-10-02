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

#define DW_BITS 0x00000106
#define EY_HASH 0xEC58A304u
#define HQ_HASH 0xCD175E6Bu
#define EB_HASH 0x19ADC88Cu
#define DRAW_HASH 0x92EAE4C5u
#define BV_HASH 0xB930691Fu
#define TOO_FAR_HASH 0x65AE0C50u
#define MOUSE_POINT_HASH 0x4C9D6519u

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const int want_k_[20] = {0, 0, 7, 7, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 4, 4, 4, 2, 2, 1};

static const entity_class_t *cls_of(const void *e) {
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

static pico8_jc_t *as_jc(const void *e) {
    assert(e != NULL && cls_of(e) == pico8_jc_struct_class());
    return (pico8_jc_t *)e;
}

static pico8_jo_t *as_jo(const void *e) {
    assert(pico8_jo_is_family(e));
    return (pico8_jo_t *)e;
}

static vec_t vec_of(const vec_t *v) {
    assert(v != NULL);
    return *v;
}

/* 0 absent, 1 false, 2 true. */
static uint32_t p__state(const pico8_jc_t *jc) {
    if (!jc->p_) {
        return 0;
    }
    return jc->p_ ? 2u : 1u;
}

static int is_throwaway(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_jc_mouse_point_struct_class();
}

static int is_too_far(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_jc_too_far_struct_class() &&
           ((const pico8_jc_too_far_t *)e)->qs != NULL && strcmp(((const pico8_jc_too_far_t *)e)->qs, "-too far-") == 0;
}

static pico8_jc_too_far_t *as_too_far(const void *e) {
    assert(is_too_far(e));
    return (pico8_jc_too_far_t *)e;
}

/* The mouse point's `ra` and `p`, NULL when absent. */
static const vec_t *mouse_point_ra(const void *re) {
    assert(is_throwaway(re));
    return ((const pico8_jc_mouse_point_t *)re)->ra;
}

static const vec_t *mouse_point_p(const void *re) {
    assert(is_throwaway(re));
    return ((const pico8_jc_mouse_point_t *)re)->p;
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
    if (ents_listed(live_entities(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)ents_rank_in(live_entities(), ref));
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

enum { RE_ABSENT, RE_PLANET, RE_THROWAWAY, RE_OTHER };

static int re_kind(const void *jc) {
    const void *re = as_jc(jc)->re;
    if (re == NULL) {
        return RE_ABSENT;
    }
    if (ents_listed(entities_by_tag("node"), re)) {
        return RE_PLANET;
    }
    if (is_throwaway(re)) {
        return RE_THROWAWAY;
    }
    return RE_OTHER;
}

enum { ET_ABSENT, ET_FALSE, ET_PLANET, ET_JC, ET_QS, ET_OTHER };

static int et_kind(const void *jc) {
    const void *et = as_jc(jc)->probe->et;
    if (et == NULL) {
        return ET_ABSENT;
    }
    if (ents_listed(entities_by_tag("node"), et)) {
        return ET_PLANET;
    }
    if (ents_listed(entities_by_tag("slipway"), et)) {
        return ET_JC;
    }
    if (is_too_far(et)) {
        return ET_QS;
    }
    return ET_OTHER;
}

enum { FO_ABSENT, FO_JC, FO_PROBE, FO_MZ, FO_OTHER };

static int holder_kind(const void *fo) {
    if (fo == NULL) {
        return FO_ABSENT;
    }
    if (ents_listed(entities_by_tag("slipway"), fo)) {
        return FO_JC;
    }
    if (ents_listed(entities_by_tag("probe"), fo)) {
        return FO_PROBE;
    }
    if (ents_listed(entities_by_tag("wormhole"), fo)) {
        return FO_MZ;
    }
    return FO_OTHER;
}

static int ptrs_count(const pointer_list_t *l) {
    return l == NULL ? -1 : l->count;
}

static int ents_ptrs_count(const entity_list_t *l) {
    return l == NULL ? -1 : l->count;
}

/* The count, then each `id` and `true`. */
static uint32_t fold_hg(uint32_t h, const void *jc) {
    const pico8_jc_hg_t *hg = as_jc(jc)->hg;
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

static uint32_t fold_jc(uint32_t h, const void *e) {
    const pico8_jc_t *jc = as_jc(e);

    h = fold_fixed(h, jc->probe->has_od, jc->probe->od);
    h = fold_fixed(h, jc->has_mt, jc->mt);
    h = fold_fixed(h, 1, jc->fa);
    h = fold_fixed(h, 1, jc->nh);
    h = fnv(h, p__state(jc));
    h = fold_string(h, jc->header.state);
    h = fold_fixed(h, 1, jc->header.state_time);
    h = fold_ref(h, jc->qu);
    h = fnv(h, (uint32_t)re_kind(jc));
    h = fold_ref(h, jc->re);
    if (re_kind(jc) == RE_THROWAWAY) {
        h = fold_vec(h, mouse_point_p(jc->re));
    }
    h = fnv(h, (uint32_t)et_kind(jc));
    h = fold_ref(h, jc->probe->et);
    h = fold_ref(h, jc->fo);
    h = fnv(h, (uint32_t)holder_kind(jc->fo));
    h = fnv(h, (uint32_t)ptrs_count(jc->ts));
    return fold_hg(h, jc);
}

static p8_fixed_t credits(void) {
    assert(pico8_fm_hy() != NULL);
    return pico8_fm_hy()->nz;
}

static uint32_t fold_planets(uint32_t h, int with_eb) {
    const entity_list_t *np = entities_by_tag("node");
    int i;

    for (i = 0; np != NULL && i < np->count; i++) {
        void *p = np->items[i];
        const entity_list_t *oe = as_jo(p)->oe;
        h = fnv(h, (uint32_t)ents_ptrs_count(oe));
        h = fold_ref(h, as_jo(p)->fo);
        h = fnv(h, (uint32_t)holder_kind(as_jo(p)->fo));
        if (with_eb && oe != NULL) {
            int j;
            for (j = 0; j < oe->count; j++) {
                assert(cls_of(oe->items[j]) == pico8_jc_struct_class());
                h = fold_ref(h, pico8_jc_eb((const pico8_jc_t *)oe->items[j], p));
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
    return vec_of(as_jo(e)->p);
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
    pico8_jo_hu(*qu, pico8_hp_new(NULL));
    pico8_jo_hu(*re, pico8_hp_new(NULL));
}

static void *build_jc(void *qu);
static void hq_over(void *jc, void *hovered, vec_t *mp);

/* A slipway from `qu` dragged over `re` as `jc:hq` sees a drag (it reads the
 * hovered planet off `ok.o_`), held until its `t` reaches 15, not paid. */
static void *dragged(void *qu, void *re) {
    vec_t *mp = malloc(sizeof *mp);
    void *jc = build_jc(qu);

    *mp = vec_add(vec_mul(pos_of(re), p8_fixed_from_int(128)),
                        vec_make(p8_fixed_from_int(1), p8_fixed_from_int(1)));
    hq_over(jc, re, mp);
    pico8_ok()->o_ = NULL;
    while (as_jc(jc)->header.state_time < p8_fixed_from_int(15)) {
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
    pico8_jc_struct_class()->drop(jc, re);
    for (i = 0; i < SETTLE_TICKS; i++) {
        update_entities();
    }
    assert(entities_by_tag("slipway") != NULL && entities_by_tag("slipway")->count == 1);
}

static void *the_slipway(void) {
    return entities_by_tag("slipway")->items[0];
}

static void *build_jc(void *qu) {
    pico8_jc_keys_t keys = {0};

    keys.qu = qu;
    return pico8_jc_new(&keys);
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
static void hq_over(void *jc, void *hovered, vec_t *mp) {
    pico8_ok()->o_ = hovered;
    cls_of(jc)->drag(jc, mp, NULL);
}

/* Every entity of `cg` begins with its header. */
static const entity_t *ke_of(const void *e) {
    assert(e != NULL);
    return e;
}

static void *first_with_me(const char *tag, const char *me) {
    const entity_list_t *list = entities_by_tag(tag);
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        const entity_t *ke = ke_of(list->items[i]);
        if (ke->state != NULL && strcmp(ke->state, me) == 0) {
            return list->items[i];
        }
    }
    return NULL;
}

/* A fresh `jc`: the spec's defaults, ke's `t`, `qu`, and the `ts`/`hg` its
 * `pq` made; registered like any `ke` entity with `kj` and `ev`. */
static void check_fresh_jc(const void *e, const void *qu) {
    const pico8_jc_t *jc = as_jc(e);
    const pico8_int_seq_t *k_;
    const pico8_jc_hg_t *hg;
    size_t i;

    assert(jc->def == &pico8_jc_def && jc->probe->def == &pico8_jc_def.probe);
    assert(jc->header.state != NULL && strcmp(jc->header.state, "placing") == 0);
    assert(jc->header.state_time == 0);
    assert(jc->fa == p8_fixed_from_int(2));
    assert(jc->probe->def->fp == 0);
    assert(jc->probe->def->ik == p8_fixed_from_int(48) && jc->probe->def->has_ik == 1);
    printf("[dw] 0x%08X\n", (unsigned)jc->probe->def->dw);
    assert(jc->probe->def->dw == (p8_fixed_t)DW_BITS);
    assert(jc->nh == p8_fixed_from_int(20));

    k_ = jc->def->k_;
    assert(k_ == &pico8_jc_k__seq && k_->count == 20);
    for (i = 0; i < 20; i++) {
        assert(k_->items[i] == want_k_[i]);
    }
    assert(jc->header.class->tags == TAG(TAG_SLIPWAY));
    assert(jc->qu == qu);

    assert(ptrs_count(jc->ts) == 0);
    hg = jc->hg;
    assert(hg == &jc->hg_own && hg->count == 0);

    assert(jc->re == NULL && !jc->probe->has_od && jc->probe->od == 0 && !jc->has_mt && jc->probe->et == NULL);
    assert(jc->header.infraspace == NULL);
    assert(jc->fo == NULL && !jc->p_);
    assert(cls_of(jc)->drag != NULL && cls_of(jc)->drop != NULL);
    assert(cls_of(jc)->draw_overlay != NULL && cls_of(jc)->draw != NULL);
    assert(cls_of(jc)->hit_test == NULL && cls_of(jc)->states[ENTITY_STATE_PLACING] == NULL &&
           cls_of(jc)->states[ENTITY_STATE_NORMAL] == NULL && cls_of(jc)->left_press == NULL && cls_of(jc)->accept_drop == NULL);

    assert(ents_listed(live_entities(), jc) && ents_listed(entities_by_message("draw"), jc) && ents_listed(entities_by_message("draw_overlay"), jc));
    assert(ents_listed(entities_by_tag("slipway"), jc));
    assert(!ents_listed(entities_by_message("hit_test"), jc) && !ents_listed(entities_by_message("new_year"), jc) && !ents_listed(entities_by_message("refresh"), jc));
}

static void test_construction(void) {
    void *ly;
    void *by_ldown;
    void *by_lookup;
    down_t made;

    world();
    ly = first_with_me("planet", "settled");
    assert(ly != NULL);
    made = cls_of(ly)->left_press(ly, NULL, NULL);
    assert(made.stop && made.held != NULL);
    by_ldown = made.held;
    check_fresh_jc(by_ldown, ly);
    by_lookup = build_jc(ly);
    check_fresh_jc(by_lookup, ly);

    assert(as_jc(by_ldown)->def == as_jc(by_lookup)->def);
    assert(as_jc(by_ldown)->header.class->tags == as_jc(by_lookup)->header.class->tags);
    assert(as_jc(by_ldown)->ts != as_jc(by_lookup)->ts);
    assert(as_jc(by_ldown)->hg != as_jc(by_lookup)->hg);
}

/* Without a target, `ey` only raises `p_`; with one, the slipway joins both
 * `oe` and `d_` charges `od`. */
static void test_ey(void) {
    uint32_t h = 2166136261u;
    void *held;
    void *origin;
    void *target;
    void *refused;
    p8_fixed_t before;
    p8_fixed_t od;

    colonized_pair(&origin, &target);
    held = dragged(origin, target);
    assert(ents_listed(entities_by_tag("slipway"), held));
    assert(as_jc(held)->qu == origin && as_jc(held)->re == target);
    assert(re_kind(held) == RE_PLANET && et_kind(held) == ET_ABSENT);
    assert(ents_ptrs_count(as_jo(origin)->oe) == 0 && ents_ptrs_count(as_jo(target)->oe) == 0);
    before = credits();
    assert(as_jc(held)->probe->has_od);
    od = as_jc(held)->probe->od;
    h = fold_jc(h, held);

    refused = build_jc(origin);
    cls_of(refused)->drop(refused, NULL);
    assert(p__state(as_jc(refused)) == 2 && credits() == before);
    assert(ents_ptrs_count(as_jo(origin)->oe) == 0);
    h = fold_jc(h, refused);

    cls_of(held)->drop(held, target);
    assert(as_jc(held)->nh == p8_fixed_from_int(1));
    assert(as_jc(held)->re == target);
    assert(p__state(as_jc(held)) == 0);
    assert(ents_ptrs_count(as_jo(origin)->oe) == 1 && ents_ptrs_count(as_jo(target)->oe) == 1);
    assert(credits() == p8_fixed_sub(before, od));
    h = fold_jc(h, held);
    h = fold_planets(h, 1);
    h = fnv(h, (uint32_t)credits());
    check("drop", h, EY_HASH);
}

/* Beyond range `et` is a fresh `{qs}`; over the pair an existing slipway
 * already joins, `et` is that slipway; either way its `fo` is the new one. */
static void test_hq(void) {
    uint32_t h = 2166136261u;
    void *old;
    void *origin;
    void *target;
    void *far;
    void *twin;

    world();
    old = the_slipway();
    origin = as_jc(old)->qu;
    target = as_jc(old)->re;

    far = build_jc(origin);
    hq_over(far, NULL, vec_ref(-640, -640));
    assert(re_kind(far) == RE_THROWAWAY && et_kind(far) == ET_QS);
    assert(as_too_far(as_jc(far)->probe->et)->fo == far);
    h = fold_jc(h, far);

    twin = build_jc(origin);
    hq_over(twin, target, vec_ref_fixed(vec_of(as_jo(target)->ra)));
    assert(re_kind(twin) == RE_PLANET && as_jc(twin)->re == target);
    assert(et_kind(twin) == ET_JC && as_jc(twin)->probe->et == old);
    assert(as_jc(old)->fo == twin);
    h = fold_jc(h, twin);
    h = fold_jc(h, old);
    check("drag", h, HQ_HASH);
}

static void test_hq_takes_mt_from_its_start(void) {
    void *from_jo;
    void *from_fr;

    world();
    from_jo = build_jc(pico8_jo_new(NULL));
    hq_over(from_jo, NULL, vec_ref(-640, -640));
    assert(!as_jc(from_jo)->has_mt && as_jc(from_jo)->header.infraspace == NULL);
    from_fr = build_jc(pico8_fr_new(NULL));
    hq_over(from_fr, NULL, vec_ref(-640, -640));
    assert(as_jc(from_fr)->has_mt && as_jc(from_fr)->mt == pico8_fr_def.mt);
    assert(as_jc(from_fr)->header.infraspace == &as_jc(from_fr)->mt);
}

/* `A and B or C`: a falsy `re` falls back to `qu`. */
static void test_eb(void) {
    uint32_t h = 2166136261u;
    void *old;
    void *origin;
    void *target;
    void *fresh;

    world();
    old = the_slipway();
    origin = as_jc(old)->qu;
    target = as_jc(old)->re;
    fresh = build_jc(origin);

    assert(pico8_jc_eb(old, origin) == target);
    assert(pico8_jc_eb(old, target) == origin);
    assert(pico8_jc_eb(fresh, origin) == origin);
    assert(pico8_jc_eb(fresh, target) == origin);
    h = fold_ref(h, pico8_jc_eb(old, origin));
    h = fold_ref(h, pico8_jc_eb(old, target));
    h = fold_ref(h, pico8_jc_eb(fresh, origin));
    h = fold_ref(h, pico8_jc_eb(fresh, NULL));
    check("eb", h, EB_HASH);
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
    uint32_t h = 2166136261u;
    uint32_t blank;
    uint32_t plain;
    uint32_t circled;
    vec_t mo_ra;
    void *old;
    void *origin;
    int i;

    world();
    old = the_slipway();
    origin = as_jc(old)->qu;
    assert(pico8_mo_qb() != NULL);
    mo_ra = vec_of(pico8_mo_qb()->ra);
    p8_camera(&gfx, p8_fixed_flr_int(mo_ra.x), p8_fixed_flr_int(mo_ra.y));
    p8_cls(&mem);
    blank = screen_hash();

    for (i = 0; i < 3; i++) {
        as_jc(old)->fo = old;
        p8_cls(&mem);
        cls_of(old)->draw(old, NULL);
        assert(as_jc(old)->fo == NULL);
        assert(screen_hash() != blank);
        h = fnv(h, screen_hash());
        h = fold_jc(h, old);
    }

    p8_cls(&mem);
    cls_of(origin)->draw(origin, vec_ref_fixed(vec_of(as_jo(origin)->ra)));
    plain = screen_hash();
    as_jo(origin)->fo = old;
    p8_cls(&mem);
    cls_of(origin)->draw(origin, vec_ref_fixed(vec_of(as_jo(origin)->ra)));
    circled = screen_hash();
    assert(as_jo(origin)->fo == NULL);
    assert(circled != plain);
    h = fnv(h, plain);
    h = fnv(h, circled);
    check("draw", h, DRAW_HASH);
}

/* Beyond range: `kj` draws the line under the game's camera, `ev` writes
 * `qs` at the mouse where one with another `et` draws a sprite and one
 * without writes its cost, and `ey` onto a real planet is refused without
 * spending or joining anything. */
static void test_too_far(void) {
    uint32_t h = 2166136261u;
    uint32_t blank;
    uint32_t too_far;
    uint32_t sprite;
    uint32_t costed;
    vec_t mo_ra;
    vec_t mouse;
    p8_fixed_t nh;
    p8_fixed_t before;
    void *old;
    void *origin;
    void *target;
    void *far;
    void *twin;
    void *near;
    void *re;

    world();
    old = the_slipway();
    origin = as_jc(old)->qu;
    target = as_jc(old)->re;
    far = build_jc(origin);
    hq_over(far, NULL, vec_ref(-640, -640));
    assert(re_kind(far) == RE_THROWAWAY && et_kind(far) == ET_QS);
    assert(et_kind(old) == ET_ABSENT);
    assert(pico8_mo_qb() != NULL);
    mo_ra = vec_of(pico8_mo_qb()->ra);
    p8_camera(&gfx, p8_fixed_flr_int(mo_ra.x), p8_fixed_flr_int(mo_ra.y));
    p8_cls(&mem);
    blank = screen_hash();

    cls_of(far)->draw(far, NULL);
    assert(screen_hash() != blank);
    h = fnv(h, screen_hash());

    twin = build_jc(origin);
    hq_over(twin, target, vec_ref_fixed(vec_of(as_jo(target)->ra)));
    assert(et_kind(twin) == ET_JC);
    near = build_jc(origin);
    hq_over(near, NULL,
            vec_ref_fixed(vec_add(vec_of(as_jo(origin)->ra), vec_make(p8_fixed_from_int(16), 0))));
    assert(re_kind(near) == RE_THROWAWAY && et_kind(near) == ET_ABSENT && as_jc(near)->probe->has_od);
    assert(pico8_je_nf() != NULL);
    mouse = vec_of(pico8_je_nf()->ra);
    p8_camera(&gfx, p8_fixed_flr_int(mouse.x) - 64, p8_fixed_flr_int(mouse.y) - 64);
    p8_cls(&mem);
    cls_of(far)->draw_overlay(far, NULL);
    too_far = screen_hash();
    p8_cls(&mem);
    cls_of(twin)->draw_overlay(twin, NULL);
    sprite = screen_hash();
    p8_cls(&mem);
    cls_of(near)->draw_overlay(near, NULL);
    costed = screen_hash();
    assert(too_far != blank && sprite != blank && costed != blank);
    assert(too_far != sprite && too_far != costed);
    h = fnv(h, too_far);
    h = fnv(h, sprite);
    h = fnv(h, costed);

    before = credits();
    nh = as_jc(far)->nh;
    re = as_jc(far)->re;
    assert(ents_ptrs_count(as_jo(origin)->oe) == 1 && ents_ptrs_count(as_jo(target)->oe) == 1);
    cls_of(far)->drop(far, target);
    assert(p__state(as_jc(far)) == 2);
    assert(credits() == before);
    assert(ents_ptrs_count(as_jo(origin)->oe) == 1 && ents_ptrs_count(as_jo(target)->oe) == 1);
    assert(as_jc(far)->re == re && re_kind(far) == RE_THROWAWAY);
    assert(as_jc(far)->nh == nh);
    assert(et_kind(far) == ET_QS);
    h = fold_jc(h, far);
    h = fnv(h, (uint32_t)credits());
    check("too far", h, TOO_FAR_HASH);
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
static void test_hq_mouse_point(void) {
    uint32_t h = 2166136261u;
    vec_t *m1;
    vec_t *m2;
    vec_t p1;
    vec_t on_camera;
    void *origin;
    void *target;
    void *a;
    void *b;
    void *r1;
    void *r2;
    void *rb;
    size_t prise;
    int i;

    world();
    origin = as_jc(the_slipway())->qu;
    target = as_jc(the_slipway())->re;
    a = build_jc(origin);
    m1 = vec_ref(-640, -640);
    hq_over(a, NULL, m1);
    r1 = as_jc(a)->re;
    assert(r1 != NULL && re_kind(a) == RE_THROWAWAY);
    assert(holds_copy_of(mouse_point_ra(r1), m1));
    p1 = vec_of(mouse_point_p(r1));
    assert(same_vec(p1, vec_div(vec_of(mouse_point_ra(r1)), p8_fixed_from_int(128))));
    assert(mouse_point_p(r1) != m1);
    assert(*((const entity_t *)r1)->map_pos == mouse_point_p(r1));
    assert(!entity_geom_is_on_screen(r1));
    assert(!ents_listed(live_entities(), r1) && !ents_listed(entities_by_tag("slipway"), r1) && !ents_listed(entities_by_tag("node"), r1));
    h = fnv(h, (uint32_t)p1.x);
    h = fnv(h, (uint32_t)p1.y);
    h = fold_jc(h, a);

    m2 = vec_ref(40, -24);
    hq_over(a, NULL, m2);
    r2 = as_jc(a)->re;
    assert(r2 == r1 && re_kind(a) == RE_THROWAWAY && "the mouse point is the slipway's own, rewritten in place");
    assert(holds_copy_of(mouse_point_ra(r2), m2));
    assert(same_vec(vec_of(mouse_point_p(r2)), vec_div(*m2, p8_fixed_from_int(128))));
    assert(!same_vec(vec_of(mouse_point_p(r2)), p1));
    h = fold_jc(h, a);

    prise = game_zone_used();
    for (i = 0; i < 10; i++) {
        hq_over(a, NULL, i % 2 ? m1 : m2);
        assert(as_jc(a)->re == r1);
    }
    assert(game_zone_used() == prise && "a held slipway allocates nothing per hq");
    hq_over(a, NULL, m2);

    b = build_jc(origin);
    hq_over(b, NULL, m1);
    rb = as_jc(b)->re;
    assert(rb != NULL && rb != r1 && as_jc(a)->re == r1);
    assert(holds_copy_of(mouse_point_ra(rb), m1) && same_vec(vec_of(mouse_point_p(rb)), p1));
    assert(holds_copy_of(mouse_point_ra(r1), m2) && "another slipway's hq leaves this one's point as it was");
    h = fold_jc(h, b);

    hq_over(a, target, vec_ref_fixed(vec_of(as_jo(target)->ra)));
    assert(as_jc(a)->re == target && re_kind(a) == RE_PLANET);
    assert(holds_copy_of(mouse_point_ra(rb), m1));
    h = fold_jc(h, a);
    check("mouse point", h, MOUSE_POINT_HASH);

    on_camera = vec_add(vec_of(pico8_mo_qb()->ra),
                              vec_make(p8_fixed_from_int(64), p8_fixed_from_int(64)));
    hq_over(b, NULL, vec_ref_fixed(on_camera));
    assert(entity_geom_is_on_screen(as_jc(b)->re));
}

/* `jc:ce` falls through to `cg("jc")` when no planet lies on the drag: a
 * drag from a nearby planet to just past the slipway's middle crosses it. */
static void test_ce_crosses_a_slipway(void) {
    const entity_list_t *jos;
    void *old;
    vec_t mid;
    int i;
    int crossed = 0;

    world();
    jos = entities_by_tag("planet");
    old = the_slipway();
    mid = vec_div(vec_add(pos_of(as_jc(old)->qu), pos_of(as_jc(old)->re)), p8_fixed_from_int(2));
    for (i = 0; i < jos->count && !crossed; i++) {
        void *q = jos->items[i];
        vec_t past;
        void *a;

        if (q == as_jc(old)->qu || q == as_jc(old)->re) {
            continue;
        }
        past = vec_add(mid, vec_div(vec_sub(mid, pos_of(q)), p8_fixed_from_int(8)));
        a = build_jc(q);
        hq_over(a, NULL, vec_ref_fixed(vec_mul(past, p8_fixed_from_int(128))));
        crossed = as_jc(a)->probe->et == old;
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
static pico8_g__def_t g_dw_zero;

/* A `g_` built as `pico8_g__new(NULL)` builds one, on `g_dw_zero`. */
static pico8_g__t *probe_with_zero_dw(void) {
    pico8_g__t *probe = malloc(sizeof *probe);

    g_dw_zero = pico8_g__def;
    g_dw_zero.probe.dw = 0;
    pico8_g__struct_init(probe, pico8_g__struct_class(), &g_dw_zero);
    pico8_f_(&probe->header);
    entity_construct(&probe->header);
    return probe;
}

/* A probe over the slipway targets it and hangs `fo` on it. */
static void test_bv(void) {
    uint32_t h = 2166136261u;
    void *old;
    void *probe;
    vec_t qu;
    vec_t re;
    int k;

    world();
    old = the_slipway();
    probe = probe_with_zero_dw();
    qu = pos_of(as_jc(old)->qu);
    re = pos_of(as_jc(old)->re);
    for (k = 1; k < 8; k++) {
        vec_t at = vec_add(qu, vec_div(vec_mul(vec_sub(re, qu), p8_fixed_from_int(k)),
                                                         p8_fixed_from_int(8)));
        cls_of(probe)->drag(probe, vec_ref_fixed(vec_mul(at, p8_fixed_from_int(128))),
                          vec_ref(64, 64));
        h = fold_ref(h, ((pico8_g__t *)probe)->probe->et);
        if (((pico8_g__t *)probe)->probe->et == old) {
            break;
        }
    }
    printf("[bv] on the slipway at %d/8\n", k);
    assert(((pico8_g__t *)probe)->probe->et == old);
    assert(as_jc(old)->fo == probe);
    assert(holder_kind(as_jc(old)->fo) == FO_PROBE);
    h = fold_jc(h, old);
    h = fold_ref(h, ((pico8_g__t *)probe)->probe->et);
    check("bv", h, BV_HASH);
}

/* `bv` aims at the first slipway of `cg("jc")` under the probe (`jp(...)[1]`),
 * not the last: a second slipway laid over the first, registered after it,
 * never wins. */
static void test_bv_takes_the_first_slipway_of_cg(void) {
    void *old;
    void *twin;
    void *probe;
    vec_t qu;
    vec_t re;
    vec_t at;

    world();
    old = the_slipway();
    twin = build_jc(as_jc(old)->re);
    as_jc(twin)->re = as_jc(old)->qu;
    assert(entities_by_tag("slipway")->count == 2 && entities_by_tag("slipway")->items[0] == old &&
           entities_by_tag("slipway")->items[1] == twin);
    probe = probe_with_zero_dw();
    qu = pos_of(as_jc(old)->qu);
    re = pos_of(as_jc(old)->re);
    at = vec_div(vec_add(qu, re), p8_fixed_from_int(2));
    cls_of(probe)->drag(probe, vec_ref_fixed(vec_mul(at, p8_fixed_from_int(128))), vec_ref(64, 64));
    assert(((pico8_g__t *)probe)->probe->et == old);
    assert(as_jc(old)->fo == probe && as_jc(twin)->fo != probe);
}

static int is_a_jc_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_jc_struct_class();
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

static void hg_add_nil(void *hg) {
    pico8_jc_hg_add(hg, NULL);
}

static void hg_add_one_more(void *hg) {
    pico8_jc_hg_add(hg, "one more");
}

/* `hg[id]=true` keeps each string `id` once and in order, reads anything
 * else as absent, refuses a nil `id` and a 91st planet. */
static void test_hg_is_a_set_of_ids(void) {
    static char ids[PICO8_JC_HG_CAP][8];
    pico8_jc_hg_t hg = {0, {NULL}};
    int i;

    pico8_jc_hg_add(&hg, "p1");
    pico8_jc_hg_add(&hg, "p2");
    pico8_jc_hg_add(&hg, "p1");
    assert(hg.count == 2 && strcmp(hg.ids[0], "p1") == 0 && strcmp(hg.ids[1], "p2") == 0);
    assert(pico8_jc_hg_has(&hg, "p2") && !pico8_jc_hg_has(&hg, "p3"));
    assert(!pico8_jc_hg_has(&hg, NULL) && !pico8_jc_hg_has(NULL, "p1"));
    must_abort(hg_add_nil, &hg);

    hg.count = 0;
    for (i = 0; i < PICO8_JC_HG_CAP; i++) {
        snprintf(ids[i], sizeof ids[i], "p%d", i + 1);
        pico8_jc_hg_add(&hg, ids[i]);
    }
    assert(hg.count == PICO8_JC_HG_CAP && pico8_jc_hg_has(&hg, "p90"));
    must_abort(hg_add_one_more, &hg);
}

/* Built by `jo:ldown`/`nr:ldown` or directly, a `jc`
 * is a struct with its five methods. */
static void test_jc_is_a_struct(void) {
    void *old;
    void *jc;
    pico8_jc_keys_t keys = {0};
    void *direct;

    world();
    old = the_slipway();
    jc = build_jc(as_jc(old)->qu);
    keys.qu = as_jc(old)->qu;
    direct = pico8_jc_new(&keys);
    assert(as_jc(direct)->qu == as_jc(old)->qu);
    assert(is_a_jc_struct(old) && is_a_jc_struct(jc) && is_a_jc_struct(direct));
    assert(cls_of(jc)->drag != NULL && cls_of(jc)->drop != NULL);
    assert(cls_of(jc)->draw_overlay != NULL && cls_of(jc)->draw != NULL);
    assert(cls_of(jc)->states[ENTITY_STATE_PLACING] == NULL && cls_of(jc)->states[ENTITY_STATE_NORMAL] == NULL &&
           cls_of(jc)->left_press == NULL && cls_of(jc)->right_press == NULL && cls_of(jc)->accept_drop == NULL);
}

/* Beyond range, every `jc` gets the same struct: `qs` fixed, `fo` the last
 * `hq` to write it, no method. */
static void test_too_far_is_a_struct(void) {
    void *origin;
    void *far;
    void *other;
    pico8_jc_too_far_t *et;

    world();
    origin = as_jc(the_slipway())->qu;
    far = build_jc(origin);
    hq_over(far, NULL, vec_ref(-640, -640));
    et = as_too_far(as_jc(far)->probe->et);
    assert(et->fo == far);
    assert(cls_of(et)->draw_overlay == NULL && cls_of(et)->draw == NULL && cls_of(et)->drag == NULL);

    other = build_jc(origin);
    hq_over(other, NULL, vec_ref(-640, -640));
    assert(as_jc(other)->probe->et == &et->header && et->fo == other);
    assert(strcmp(et->qs, "-too far-") == 0);
}

/* Over nothing, `re` is a `{ra,p}` struct without method; a nil `mp`
 * leaves `ra` absent and `p` zero. */
static void test_mouse_point_is_a_struct(void) {
    void *origin;
    void *jc;
    void *re;
    vec_t p;

    world();
    origin = as_jc(the_slipway())->qu;
    jc = build_jc(origin);
    hq_over(jc, NULL, vec_ref(-640, -640));
    re = as_jc(jc)->re;
    assert(re != NULL &&
           ((const entity_t *)re)->class == pico8_jc_mouse_point_struct_class());
    assert(mouse_point_ra(re) != NULL && mouse_point_p(re) != NULL);
    assert(cls_of(re)->attach_slipway == NULL && cls_of(re)->drag == NULL);

    hq_over(jc, NULL, NULL);
    re = as_jc(jc)->re;
    assert(is_throwaway(re) && mouse_point_ra(re) == NULL);
    p = vec_of(mouse_point_p(re));
    assert(p.x == 0 && p.y == 0);
}

static void *g_off_qu;

static void build_off_row(void *e) {
    pico8_jc_keys_t keys = {0};

    (void)e;
    keys.qu = g_off_qu;
    (void)pico8_jc_new(&keys);
}

/* A construction's `qu` that is not a planet stops the run. */
static void test_jc_new_stops_off_qu(void) {
    void *off[5];
    void *old;
    void *far;
    size_t i;

    world();
    old = the_slipway();
    far = build_jc(as_jc(old)->qu);
    hq_over(far, NULL, vec_ref(-640, -640));
    assert(is_throwaway(as_jc(far)->re));
    off[0] = pico8_ga_new(NULL);
    off[1] = pico8_g__new(NULL);
    off[2] = pico8_mz_new();
    off[3] = far;
    off[4] = as_jc(far)->re;
    for (i = 0; i < sizeof off / sizeof off[0]; i++) {
        g_off_qu = off[i];
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
    in_fresh_child(test_ey);
    in_fresh_child(test_hq);
    in_fresh_child(test_hq_takes_mt_from_its_start);
    in_fresh_child(test_eb);
    in_fresh_child(test_draw);
    in_fresh_child(test_bv);
    in_fresh_child(test_bv_takes_the_first_slipway_of_cg);
    in_fresh_child(test_too_far);
    in_fresh_child(test_hq_mouse_point);
    in_fresh_child(test_ce_crosses_a_slipway);
    in_fresh_child(test_jc_is_a_struct);
    in_fresh_child(test_hg_is_a_set_of_ids);
    in_fresh_child(test_too_far_is_a_struct);
    in_fresh_child(test_mouse_point_is_a_struct);
    in_fresh_child(test_jc_new_stops_off_qu);

    printf("test_pico8_jc: all tests passed\n");
    return 0;
}
