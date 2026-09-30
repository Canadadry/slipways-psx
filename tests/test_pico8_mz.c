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

#include "../src/pico8_assets.h"
#include "../src/pico8_bg.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_research.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_vec.h"
#include "../src/pico8_wormhole.h"

/* Ticks after the slipway is paid, long enough for the colonizing and
 * paying effects to leave `hv`: `dg` skips the entity after one it takes out
 * (TROU-5). */
#define SETTLE_TICKS 120

#define OX_HASH 0xC6D8AF55u
#define HQ_HASH 0x9DAA980Cu
#define KZ_TICKS 21
#define KZ_HASH 0xE16D1D52u
#define EY_HASH 0x2D563CE9u
#define PR_HASH 0xAC265666u
#define PR_EXIT_TICK 27
#define DRAW_HASH 0xD124A50Cu

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

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

static int rank_in(const pico8_ptrs_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int listed(const pico8_ptrs_t *list, const void *e) {
    return rank_in(list, e) >= 0;
}

static int ents_rank_in(const pico8_ents_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int ents_listed(const pico8_ents_t *list, const void *e) {
    return ents_rank_in(list, e) >= 0;
}

static int is_me(const pico8_struct_header_t *ke, const char *want) {
    return ke->me != NULL && strcmp(ke->me, want) == 0;
}

static pico8_mz_t *as_mz(void *e) {
    assert(e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_mz_struct_class());
    return e;
}

static pico8_jc_t *as_jc(void *e) {
    assert(e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_jc_struct_class());
    return e;
}

static pico8_jo_t *as_jo(void *e) {
    assert(pico8_jo_is_family(e));
    return e;
}

static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

/* 0 absent, 1 false, 2 true. */
static uint32_t p__state(const pico8_mz_t *mz) {
    if (!mz->has_p_) {
        return 0;
    }
    return mz->p_ ? 2 : 1;
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
           ((const pico8_struct_header_t *)e)->cls == pico8_bv_et_struct_class();
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
    if (listed(pico8_cg("jc"), et)) {
        return ET_JC;
    }
    if (listed(pico8_cg("jo"), et)) {
        return ET_JO;
    }
    if (is_bv_target(et)) {
        return ET_THROWAWAY;
    }
    return ET_OTHER;
}

static pico8_fixed_t credits(void) {
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
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
}

static pico8_vec_t pos_of(void *e) {
    const pico8_jo_t *jo = as_jo(e);
    assert(jo->p != NULL);
    return *jo->p;
}

static pico8_fixed_t dist2(pico8_vec_t a, pico8_vec_t b) {
    pico8_vec_t d = pico8_vec_sub(a, b);
    return pico8_fixed_add(pico8_fixed_mul(d.x, d.x), pico8_fixed_mul(d.y, d.y));
}

/* Two generated planets colonized and joined by a paid slipway, the rest
 * of the galaxy left unknown, then left to settle. `jc:hq` reads the hovered planet off
 * `ok.o_`. */
static void world(void) {
    const pico8_ptrs_t *jos;
    pico8_jc_keys_t jc_keys = {0};
    pico8_vec_t *mp = malloc(sizeof *mp);
    void *qu;
    void *re = NULL;
    pico8_jc_t *jc;
    int i;

    start_game();
    jos = pico8_cg("jo");
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
    *mp = pico8_vec_add(pico8_vec_mul(pos_of(re), pico8_fixed_from_int(128)),
                        pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(1)));
    pico8_ok()->o_ = re;
    pico8_jc_struct_class()->hq(jc, mp, NULL);
    pico8_ok()->o_ = NULL;
    while (jc->header.t < pico8_fixed_from_int(15)) {
        pico8_dg();
    }
    pico8_jc_struct_class()->ey(jc, re);
    assert((void *)jc->re == re);
    for (i = 0; i < SETTLE_TICKS; i++) {
        pico8_dg();
    }
}

static pico8_mz_t *build_mz(void) {
    return pico8_mz_new();
}

static pico8_vec_t *vec_ref(int x, int y) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
    return v;
}

static pico8_vec_t *vec_ref_fixed(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

static void hq(void *mz, pico8_vec_t *mp, pico8_vec_t *rp) {
    pico8_mz_struct_class()->hq(mz, mp, rp);
}

static void ey(void *mz) {
    pico8_mz_struct_class()->ey(mz, NULL);
}

/* `dg` skips the entity after one it takes out of `hv` (TROU-5): count
 * ticks by `t`, not by calls. */
static void until_t(const pico8_mz_t *mz, int t) {
    while (mz->header.t < pico8_fixed_from_int(t)) {
        pico8_dg();
    }
}

static pico8_jo_t *first_jo_with_me(const char *me) {
    const pico8_ptrs_t *list = pico8_cg("jo");
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
static pico8_vec_t slipway_mid(pico8_jc_t *jc) {
    return pico8_vec_mul(pico8_vec_div(pico8_vec_add(pos_of(jc->qu), pos_of(jc->re)), pico8_fixed_from_int(2)),
                         pico8_fixed_from_int(128));
}

/* A fresh `mz`: the spec's defaults, ke's `t`, nothing written yet but
 * `ox`, registered like any `ke` entity with `kj` and `ev`. */
static void check_fresh_mz(void *e) {
    const pico8_mz_t *mz = as_mz(e);
    const pico8_int_seq_t *kk;
    const pico8_mz_jj_t *jj;
    pico8_fixed_t fa;
    static const int want_kk[9] = {7, 7, 6, 13, 5, 1, 1, 1, 0};
    int i;

    assert(is_me(&mz->header, "kz"));
    assert(mz->header.t == 0);
    assert(mz->probe->dw == 0);
    assert(mz->probe->dq != NULL && strcmp(mz->probe->dq, "pr") == 0);
    assert(mz->spawn_snd == pico8_fixed_from_int(52));
    assert(mz->probe->ik == pico8_fixed_from_int(49));
    assert(mz->oi == 0);
    assert(mz->probe->od == pico8_fixed_from_int(3));
    assert(mz->probe->fp == pico8_fixed_from_int(1));
    assert(mz->fa == pico8_fixed_from_int(5));
    assert(mz->header.cls->get_fa(mz, &fa) && fa == mz->fa);

    kk = mz->kk;
    assert(kk == &pico8_mz_kk_seq && kk->count == 9);
    for (i = 0; i < 9; i++) {
        assert(kk->items[i] == want_kk[i]);
    }
    assert(mz->header.cls->tags == PICO8_TAG(PICO8_TAG_MZ));
    jj = mz->jj;
    assert(jj != NULL && jj->qs != NULL && strcmp(jj->qs, "-too far-") == 0);
    assert(pico8_mz_jj_wrapper() == jj);

    assert(mz->ra == NULL && mz->p == NULL && mz->probe->et == NULL && mz->ij == NULL && !mz->has_p_);
    assert(mz->ox != NULL);

    assert(listed(pico8_hv(), mz) && listed(pico8_cv("kj"), mz) && listed(pico8_cv("ev"), mz));
    assert(listed(pico8_cg("mz"), mz));
    assert(!listed(pico8_cv("hd"), mz) && !listed(pico8_cv("hk"), mz) && !listed(pico8_cv("hb"), mz));
}

static void test_construction(void) {
    pico8_down_t made;
    const pico8_ptrs_t *mzs;
    int before;

    start_game();
    assert(pico8_cg("g_") == NULL || pico8_cg("g_")->count == 0);
    mzs = pico8_cg("mz");
    before = mzs == NULL ? 0 : mzs->count;
    made = pico8_bg_ldown(NULL);
    assert(made.stop && made.held != NULL);
    assert(listed(pico8_cg("g_"), made.held));
    mzs = pico8_cg("mz");
    assert((mzs == NULL ? 0 : mzs->count) == before);

    made = pico8_bg_ldown(NULL);
    assert(made.stop && made.held != NULL);
    check_fresh_mz(made.held);
    check_fresh_mz(build_mz());
    check_fresh_mz(pico8_mz_new());
}

static uint32_t fold_ox(uint32_t h, const pico8_mz_t *mz) {
    const pico8_ents_t *ox;
    int i;

    assert(mz->ox != NULL);
    ox = mz->ox;
    h = fnv(h, (uint32_t)ox->count);
    for (i = 0; i < ox->count; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), ox->items[i]));
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
    pico8_qh_t *lit;
    const pico8_ptrs_t *jos;
    int i;

    world();
    jos = pico8_cg("jo");
    for (i = 0; unlit == NULL && jos != NULL && i < jos->count; i++) {
        pico8_jo_t *jo = as_jo(jos->items[i]);
        if (!jo->has_fx) {
            unlit = jo;
        }
    }
    assert(unlit != NULL);
    unlit->fx = 0;
    unlit->has_fx = 1;
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
    lit->jo.fx = 1;
    lit->jo.has_fx = 1;
    assert(ents_listed(build_mz()->ox, lit));
}

/* The `fo` of the `mz`'s `et`, a slipway or the throwaway. */
static const void *et_fo(const pico8_mz_t *mz) {
    pico8_struct_header_t *et = mz->probe->et;

    assert(et != NULL);
    if (et->cls == pico8_jc_struct_class()) {
        return ((const pico8_jc_t *)et)->fo;
    }
    return bv_et_fo(et);
}

static uint32_t fold_hq(uint32_t h, const pico8_mz_t *mz, int want_kind) {
    const pico8_struct_header_t *et = mz->probe->et;
    int kind = et_kind(mz);

    printf("[hq] et kind %d\n", kind);
    fflush(stdout);
    assert(kind == want_kind);
    assert(mz->header.cls->get_ra(mz) == mz->ra && mz->header.cls->get_p(mz) == mz->p);
    h = fold_vec(h, mz->ra);
    h = fold_vec(h, mz->p);
    h = fnv(h, (uint32_t)kind);
    if (et != NULL) {
        if (et->cls == pico8_mz_jj_struct_class() && ((const pico8_mz_jj_t *)et)->qs != NULL) {
            h = fnv_str(h, ((const pico8_mz_jj_t *)et)->qs);
        }
        if (et->cls == pico8_jc_too_far_struct_class() && ((const pico8_jc_too_far_t *)et)->qs != NULL) {
            h = fnv_str(h, ((const pico8_jc_too_far_t *)et)->qs);
        }
        if (kind == ET_JC || kind == ET_THROWAWAY) {
            assert(et_fo(mz) == mz);
        }
    }
    assert(mz->ij != NULL);
    return fnv(h, (uint32_t)rank_in(pico8_hv(), mz->ij));
}

/* A quarter of the slipway's length past its `qu` end: near that
 * colonized anchor, off the slipway's segment. */
static pico8_vec_t near_anchor(void) {
    const pico8_ptrs_t *jcs = pico8_cg("jc");
    const pico8_jc_t *jc;
    pico8_vec_t qu;
    pico8_vec_t re;

    assert(jcs != NULL && jcs->count == 1);
    jc = as_jc(jcs->items[0]);
    qu = pos_of(jc->qu);
    re = pos_of(jc->re);
    return pico8_vec_mul(pico8_vec_add(qu, pico8_vec_div(pico8_vec_sub(qu, re), pico8_fixed_from_int(4))),
                         pico8_fixed_from_int(128));
}

static void test_hq(void) {
    uint32_t h = 2166136261u;
    pico8_mz_t *mz;
    const pico8_ptrs_t *jcs;
    pico8_vec_t mid;

    world();
    mz = build_mz();
    jcs = pico8_cg("jc");
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
    check("hq", h, HQ_HASH);
}

/* `kz` grows `oi` each `dg` tick; nothing sets `p_`. */
static void test_kz_ticks(void) {
    uint32_t h = 2166136261u;
    pico8_mz_t *mz;
    int i;

    world();
    mz = build_mz();
    for (i = 0; i < KZ_TICKS; i++) {
        pico8_dg();
        assert(p__state(mz) == 0);
        assert(mz->header.t == pico8_fixed_from_int(i + 1));
        h = fnv(h, (uint32_t)mz->oi);
    }
    assert(listed(pico8_hv(), mz));
    check("kz ticks", h, KZ_HASH);
}

static uint32_t fold_ey(uint32_t h, const pico8_mz_t *mz) {
    h = fold_string(h, mz->header.me);
    h = fnv(h, (uint32_t)mz->header.t);
    h = fnv(h, p__state(mz));
    return fnv(h, (uint32_t)credits());
}

/* Before tick 15 `d_` never runs; after it, a set `et` refuses, an unset
 * one pays `od` and turns to `pr`, and too few credits refuse. */
static void test_ey(void) {
    uint32_t h = 2166136261u;
    pico8_fixed_t before;
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
    assert(is_me(&paid->header, "pr") && paid->header.t == 0 && p__state(paid) == 1);
    assert(credits() == pico8_fixed_sub(before, pico8_fixed_from_int(3)));
    h = fold_ey(h, paid);

    broke = build_mz();
    until_t(broke, 15);
    hq(broke, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    pico8_fm_hy()->nz = pico8_fixed_from_int(2);
    ey(broke);
    assert(is_me(&broke->header, "kz") && p__state(broke) == 2);
    assert(credits() == pico8_fixed_from_int(2));
    h = fold_ey(h, broke);
    check("ey", h, EY_HASH);
}

static int unknown_count(void) {
    const pico8_ptrs_t *jo = pico8_cg("jo");
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
    pico8_vec_t *p = malloc(sizeof *p);

    assert(unknown != NULL);
    until_t(mz, 15);
    hq(mz, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    assert(et_kind(mz) == ET_FALSE);
    ey(mz);
    assert(is_me(&mz->header, "pr"));
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
    while (listed(pico8_hv(), mz)) {
        int before = unknown_count();
        pico8_dg();
        tick++;
        assert(tick < 100);
        revealed += before - unknown_count();
        h = fnv(h, (uint32_t)(before - unknown_count()));
        h = fnv(h, p__state(mz));
        h = fnv(h, (uint32_t)listed(pico8_cv("kj"), mz));
        h = fnv(h, (uint32_t)listed(pico8_cg("mz"), mz));
    }
    printf("[pr] left hv after %d ticks, %d planets revealed\n", tick, revealed);
    assert(revealed > 0);
    assert(!listed(pico8_cv("kj"), mz) && !listed(pico8_cv("ev"), mz) && !listed(pico8_cg("mz"), mz));
    check_int("pr exit tick", tick, PR_EXIT_TICK);
    check("pr ticks", h, PR_HASH);
}

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)pico8_peek(&mem, PICO8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
}

/* `kj` is given the screen centre rather than `ra`, which is in world
 * coordinates, off the uncamera'd screen. */
static uint32_t draw(void *mz) {
    const pico8_struct_class_t *cls = ((const pico8_struct_header_t *)mz)->cls;

    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    cls->kj(mz, vec_ref(64, 64));
    cls->ev(mz, NULL);
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
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
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
        pico8_dg();
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
    pico8_jo_keys_t keys;
    pico8_down_t r;

    world();
    mz = build_mz();
    ly = first_jo_with_me("ly");
    assert(ly != NULL);
    r = pico8_jo_gv(ly, &mz->header);
    assert(r.stop && r.held == NULL);

    keys.id = NULL;
    keys.p = ly->p;
    keys.ra = ly->ra;
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
    const pico8_ptrs_t *jcs;
    pico8_vec_t mid;

    world();
    wrapper = (void *)pico8_mz_jj_wrapper();
    check_jj_untouched();
    assert(((const pico8_struct_header_t *)wrapper)->cls->kj == NULL &&
           ((const pico8_struct_header_t *)wrapper)->cls->ev == NULL &&
           ((const pico8_struct_header_t *)wrapper)->cls->hq == NULL);

    mz = build_mz();
    hq(mz, vec_ref(-640, -640), vec_ref(64, 64));
    assert(et_kind(mz) == ET_JJ);
    check_jj_untouched();

    jcs = pico8_cg("jc");
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

static int is_a_struct_of(const void *e, const pico8_struct_class_t *cls) {
    return e != NULL && ((const pico8_struct_header_t *)e)->cls == cls;
}

static int is_an_mz_struct(const void *e) {
    return is_a_struct_of(e, pico8_mz_struct_class());
}

/* Built through `bg:ldown` or directly, an `mz` is a
 * struct with its six methods; the `jj` wrapper is a struct too, the very
 * object of the `jj` member. */
static void test_mz_is_a_struct(void) {
    pico8_down_t made;
    pico8_mz_t *mz;

    start_game();
    (void)pico8_bg_ldown(NULL);
    made = pico8_bg_ldown(NULL);
    mz = pico8_mz_new();
    assert(is_an_mz_struct(made.held) && is_an_mz_struct(build_mz()) && is_an_mz_struct(mz));
    assert(pico8_mz_struct_class()->hq != NULL && pico8_mz_struct_class()->ey != NULL);
    assert(pico8_mz_struct_class()->ol == NULL);
    assert(pico8_mz_struct_class()->states[PICO8_STATE_KZ] != NULL &&
           pico8_mz_struct_class()->states[PICO8_STATE_PR] != NULL && pico8_mz_struct_class()->kj != NULL &&
           pico8_mz_struct_class()->ev != NULL);
    assert(pico8_mz_struct_class()->states[PICO8_STATE_PH] == NULL && pico8_mz_struct_class()->ldown == NULL);

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
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);

    pico8_cart_load();
    in_fresh_child(test_construction);
    in_fresh_child(test_ox);
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
