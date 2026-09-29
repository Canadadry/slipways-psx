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
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_research.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
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
#define MZ_MEMBERS_HASH 0x991FD1CAu

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

static int rank_in(const pico8_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i].kind == PICO8_VALUE_REF && list->items[i].as.ref == e) {
            return i;
        }
    }
    return -1;
}

static int listed(const pico8_list_t *list, const void *e) {
    return rank_in(list, e) >= 0;
}

static int has(const void *e, const char *name) {
    pico8_value_t v;
    return pico8_entity_get_field(e, name, &v);
}

static pico8_value_t get(const void *e, const char *name) {
    pico8_value_t v = pico8_value_ref(NULL);
    (void)pico8_entity_get_field(e, name, &v);
    return v;
}

static pico8_fixed_t must_fixed(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static int is_string(const void *e, const char *name, const char *want) {
    pico8_value_t v;
    return pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, want) == 0;
}

static uint32_t fold_fixed(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    if (!pico8_entity_get_field(e, name, &v)) {
        return fnv(h, 0xDEADu);
    }
    assert(v.kind == PICO8_VALUE_FIXED);
    return fnv(h, (uint32_t)v.as.fixed);
}

static uint32_t fold_vec(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    const pico8_vec_t *p;
    if (!pico8_entity_get_field(e, name, &v)) {
        return fnv(h, 0xDEADu);
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    p = v.as.ref;
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

/* 0 absent, 1 false, 2 true, 3 other. */
static uint32_t bool_state(const void *e, const char *name) {
    pico8_value_t v;
    if (!pico8_entity_get_field(e, name, &v)) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_BOOL) {
        return v.as.boolean ? 2 : 1;
    }
    return 3;
}

static uint32_t fold_string(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    if (!pico8_entity_get_field(e, name, &v)) {
        return fnv(h, 0xDEADu);
    }
    assert(v.kind == PICO8_VALUE_STRING);
    return fnv_str(h, v.as.string);
}

enum { ET_ABSENT, ET_FALSE, ET_JJ, ET_JC, ET_JO, ET_THROWAWAY, ET_OTHER };

static int is_bv_target(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_bv_et_struct_class();
}

static int et_kind(const void *mz) {
    pico8_value_t et;
    if (!pico8_entity_get_field(mz, "et", &et)) {
        return ET_ABSENT;
    }
    if (et.kind == PICO8_VALUE_BOOL && !et.as.boolean) {
        return ET_FALSE;
    }
    if (et.kind != PICO8_VALUE_REF || et.as.ref == NULL) {
        return ET_OTHER;
    }
    if (et.as.ref == (const void *)pico8_mz_jj_wrapper()) {
        return ET_JJ;
    }
    if (listed(pico8_cg("jc"), et.as.ref)) {
        return ET_JC;
    }
    if (listed(pico8_cg("jo"), et.as.ref)) {
        return ET_JO;
    }
    if (is_bv_target(et.as.ref)) {
        return ET_THROWAWAY;
    }
    return ET_OTHER;
}

static pico8_fixed_t credits(void) {
    return must_fixed(pico8_fm(), "nz");
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

static pico8_vec_t pos_of(const void *e) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, "p", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return *(const pico8_vec_t *)v.as.ref;
}

static pico8_fixed_t dist2(pico8_vec_t a, pico8_vec_t b) {
    pico8_vec_t d = pico8_vec_sub(a, b);
    return pico8_fixed_add(pico8_fixed_mul(d.x, d.x), pico8_fixed_mul(d.y, d.y));
}

/* Two generated planets colonized and joined by a paid slipway, the rest
 * of the galaxy left unknown, then left to settle. `jc:hq` reads the hovered planet off
 * `ok.o_`. */
static void world(void) {
    const pico8_list_t *jos;
    pico8_jc_keys_t jc_keys = {0};
    pico8_vec_t *mp = malloc(sizeof *mp);
    void *qu;
    void *re = NULL;
    void *jc;
    int i;

    start_game();
    jos = pico8_cg("jo");
    assert(jos != NULL && jos->count > 1);
    qu = jos->items[0].as.ref;
    for (i = 1; i < jos->count; i++) {
        void *e = jos->items[i].as.ref;
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
    while (must_fixed(jc, "t") < pico8_fixed_from_int(15)) {
        pico8_dg();
    }
    pico8_jc_struct_class()->ey(jc, re);
    assert(get(jc, "re").as.ref == re);
    for (i = 0; i < SETTLE_TICKS; i++) {
        pico8_dg();
    }
}

static void *build_mz(void) {
    return pico8_mz_new();
}

static pico8_value_t vec_ref(int x, int y) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
    return pico8_value_ref(v);
}

static pico8_value_t vec_ref_fixed(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return pico8_value_ref(v);
}

static void hq(void *mz, pico8_value_t mp, pico8_value_t rp) {
    pico8_mz_struct_class()->hq(mz, mp.as.ref, rp.as.ref);
}

static void ey(void *mz) {
    pico8_mz_struct_class()->ey(mz, NULL);
}

/* `dg` skips the entity after one it takes out of `hv` (TROU-5): count
 * ticks by `t`, not by calls. */
static void until_t(const void *mz, int t) {
    while (must_fixed(mz, "t") < pico8_fixed_from_int(t)) {
        pico8_dg();
    }
}

static pico8_vec_t vec_of(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return *(const pico8_vec_t *)v.as.ref;
}

static void *first_with_me(const char *tag, const char *me) {
    const pico8_list_t *list = pico8_cg(tag);
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (is_string(list->items[i].as.ref, "me", me)) {
            return list->items[i].as.ref;
        }
    }
    return NULL;
}

/* A fresh `mz`: the spec's defaults, ke's `t`, nothing written yet but
 * `ox`, registered like any `ke` entity with `kj` and `ev`. */
static void check_fresh_mz(const void *mz) {
    pico8_value_t v;
    const pico8_int_seq_t *kk;
    const pico8_str_seq_t *nu;
    const void *jj;
    static const int want_kk[9] = {7, 7, 6, 13, 5, 1, 1, 1, 0};
    int i;

    assert(is_string(mz, "me", "kz"));
    assert(must_fixed(mz, "t") == 0);
    assert(must_fixed(mz, "dw") == 0);
    assert(is_string(mz, "dq", "pr"));
    assert(must_fixed(mz, "spawn_snd") == pico8_fixed_from_int(52));
    assert(must_fixed(mz, "ik") == pico8_fixed_from_int(49));
    assert(must_fixed(mz, "oi") == 0);
    assert(must_fixed(mz, "od") == pico8_fixed_from_int(3));
    assert(must_fixed(mz, "fp") == pico8_fixed_from_int(1));
    assert(must_fixed(mz, "fa") == pico8_fixed_from_int(5));

    assert(pico8_entity_get_field(mz, "kk", &v) && v.kind == PICO8_VALUE_REF);
    kk = v.as.ref;
    assert(kk == &pico8_mz_kk_seq && kk->count == 9);
    for (i = 0; i < 9; i++) {
        assert(kk->items[i] == want_kk[i]);
    }
    assert(pico8_entity_get_field(mz, "nu", &v) && v.kind == PICO8_VALUE_REF);
    nu = v.as.ref;
    assert(nu == &pico8_mz_nu_tags && nu->count == 1 && strcmp(nu->items[0], "mz") == 0);
    assert(pico8_entity_get_field(mz, "jj", &v) && v.kind == PICO8_VALUE_REF);
    jj = v.as.ref;
    assert(pico8_entity_get_field(jj, "qs", &v) && v.kind == PICO8_VALUE_STRING &&
           strcmp(v.as.string, "-too far-") == 0);
    assert((const void *)pico8_mz_jj_wrapper() == jj);

    assert(!has(mz, "ra") && !has(mz, "p") && !has(mz, "et") && !has(mz, "ij") && !has(mz, "p_"));
    assert(!has(mz, "fx") && !has(mz, "qu"));
    assert(has(mz, "ox"));

    assert(listed(pico8_hv(), mz) && listed(pico8_cv("kj"), mz) && listed(pico8_cv("ev"), mz));
    assert(listed(pico8_cg("mz"), mz));
    assert(!listed(pico8_cv("hd"), mz) && !listed(pico8_cv("hk"), mz) && !listed(pico8_cv("hb"), mz));
}

static void test_construction(void) {
    pico8_down_t made;
    const pico8_list_t *mzs;
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

static uint32_t fold_ox(uint32_t h, const void *mz) {
    pico8_value_t v;
    const pico8_list_t *ox;
    int i;

    assert(pico8_entity_get_field(mz, "ox", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    ox = v.as.ref;
    h = fnv(h, (uint32_t)ox->count);
    for (i = 0; i < ox->count; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), ox->items[i].as.ref));
    }
    return h;
}

/* `ox` holds the `fx` entities of `hv` in its order, never an `mz`. */
static void test_ox(void) {
    uint32_t h = 2166136261u;
    void *first;
    void *second;
    pico8_value_t v;

    world();
    first = build_mz();
    h = fold_ox(h, first);
    second = build_mz();
    h = fold_ox(h, second);
    assert(pico8_entity_get_field(second, "ox", &v));
    assert(!listed(v.as.ref, first));
    printf("[ox] %d entries\n", ((const pico8_list_t *)v.as.ref)->count);
    assert(((const pico8_list_t *)v.as.ref)->count > 0);
    check("ox", h, OX_HASH);
}

static uint32_t fold_hq(uint32_t h, const void *mz, int want_kind) {
    pico8_value_t et;
    pico8_value_t ij;
    int kind = et_kind(mz);

    printf("[hq] et kind %d\n", kind);
    fflush(stdout);
    assert(kind == want_kind);
    h = fold_vec(h, mz, "ra");
    h = fold_vec(h, mz, "p");
    h = fnv(h, (uint32_t)kind);
    if (pico8_entity_get_field(mz, "et", &et) && et.kind == PICO8_VALUE_REF && et.as.ref != NULL) {
        pico8_value_t qs;
        if (pico8_entity_get_field(et.as.ref, "qs", &qs)) {
            h = fnv_str(h, qs.as.string);
        }
        if (kind == ET_JC || kind == ET_THROWAWAY) {
            pico8_value_t fo;
            assert(pico8_entity_get_field(et.as.ref, "fo", &fo) && fo.kind == PICO8_VALUE_REF && fo.as.ref == mz);
        }
    }
    assert(pico8_entity_get_field(mz, "ij", &ij) && ij.kind == PICO8_VALUE_REF);
    return fnv(h, (uint32_t)rank_in(pico8_hv(), ij.as.ref));
}

/* A quarter of the slipway's length past its `qu` end: near that
 * colonized anchor, off the slipway's segment. */
static pico8_vec_t near_anchor(void) {
    const pico8_list_t *jcs = pico8_cg("jc");
    const void *jc;
    pico8_vec_t qu;
    pico8_vec_t re;

    assert(jcs != NULL && jcs->count == 1);
    jc = jcs->items[0].as.ref;
    qu = vec_of(get(jc, "qu").as.ref, "p");
    re = vec_of(get(jc, "re").as.ref, "p");
    return pico8_vec_mul(pico8_vec_add(qu, pico8_vec_div(pico8_vec_sub(qu, re), pico8_fixed_from_int(4))),
                         pico8_fixed_from_int(128));
}

static void test_hq(void) {
    uint32_t h = 2166136261u;
    void *mz;
    const pico8_list_t *jcs;
    void *jc;
    pico8_vec_t mid;

    world();
    mz = build_mz();
    jcs = pico8_cg("jc");
    assert(jcs != NULL && jcs->count > 0);
    jc = jcs->items[0].as.ref;
    mid = pico8_vec_mul(pico8_vec_div(pico8_vec_add(vec_of(get(jc, "qu").as.ref, "p"), vec_of(get(jc, "re").as.ref, "p")),
                                      pico8_fixed_from_int(2)),
                        pico8_fixed_from_int(128));
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
    void *mz;
    int i;

    world();
    mz = build_mz();
    for (i = 0; i < KZ_TICKS; i++) {
        pico8_dg();
        assert(bool_state(mz, "p_") == 0);
        assert(must_fixed(mz, "t") == pico8_fixed_from_int(i + 1));
        h = fold_fixed(h, mz, "oi");
    }
    assert(listed(pico8_hv(), mz));
    check("kz ticks", h, KZ_HASH);
}

static uint32_t fold_ey(uint32_t h, const void *mz) {
    h = fold_string(h, mz, "me");
    h = fold_fixed(h, mz, "t");
    h = fnv(h, bool_state(mz, "p_"));
    return fnv(h, (uint32_t)credits());
}

/* Before tick 15 `d_` never runs; after it, a set `et` refuses, an unset
 * one pays `od` and turns to `pr`, and too few credits refuse. */
static void test_ey(void) {
    uint32_t h = 2166136261u;
    pico8_fixed_t before;
    void *early;
    void *targeted;
    void *paid;
    void *broke;

    world();
    before = credits();

    early = build_mz();
    ey(early);
    assert(bool_state(early, "p_") == 2 && credits() == before);
    h = fold_ey(h, early);

    targeted = build_mz();
    until_t(targeted, 15);
    hq(targeted, vec_ref(-640, -640), vec_ref(64, 64));
    assert(et_kind(targeted) == ET_JJ);
    ey(targeted);
    assert(bool_state(targeted, "p_") == 2 && credits() == before);
    h = fold_ey(h, targeted);

    paid = build_mz();
    until_t(paid, 15);
    hq(paid, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    assert(et_kind(paid) == ET_FALSE);
    ey(paid);
    assert(is_string(paid, "me", "pr") && must_fixed(paid, "t") == 0 && bool_state(paid, "p_") == 1);
    assert(credits() == pico8_fixed_sub(before, pico8_fixed_from_int(3)));
    h = fold_ey(h, paid);

    broke = build_mz();
    until_t(broke, 15);
    hq(broke, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    pico8_fm_hy()->nz = pico8_fixed_from_int(2);
    ey(broke);
    assert(is_string(broke, "me", "kz") && bool_state(broke, "p_") == 2);
    assert(credits() == pico8_fixed_from_int(2));
    h = fold_ey(h, broke);
    check("ey", h, EY_HASH);
}

static int unknown_count(void) {
    const pico8_list_t *jo = pico8_cg("jo");
    int n = 0;
    int i;
    for (i = 0; jo != NULL && i < jo->count; i++) {
        if (is_string(jo->items[i].as.ref, "me", "unknown")) {
            n++;
        }
    }
    return n;
}

/* A paid `mz` put on an unknown planet: `pr` reveals around it until
 * `t>25` takes it out of `hv`. */
static void *paid_mz_on_unknown(void) {
    void *mz = build_mz();
    void *unknown = first_with_me("jo", "unknown");
    pico8_vec_t *p = malloc(sizeof *p);

    assert(unknown != NULL);
    until_t(mz, 15);
    hq(mz, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    assert(et_kind(mz) == ET_FALSE);
    ey(mz);
    assert(is_string(mz, "me", "pr"));
    *p = vec_of(unknown, "p");
    pico8_entity_set_field(mz, "p", pico8_value_ref(p));
    return mz;
}

static void test_pr_ticks(void) {
    uint32_t h = 2166136261u;
    void *mz;
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
        h = fnv(h, bool_state(mz, "p_"));
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
    cls->kj(mz, vec_ref(64, 64).as.ref);
    cls->ev(mz, NULL);
    return screen_hash();
}

/* Ring, anchor line and sprite while `kz`, the "-too far-" label on the
 * `jj` wrapper, the coloured rings of `pr`. */
static void test_draw(void) {
    uint32_t h = 2166136261u;
    uint32_t blank;
    void *bare;
    void *far;
    void *mz;
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
    void *mz;
    void *ly;
    void *nr;
    pico8_jo_keys_t keys;
    pico8_down_t r;

    world();
    mz = build_mz();
    ly = first_with_me("jo", "ly");
    assert(ly != NULL);
    r = pico8_jo_gv(ly, mz);
    assert(r.stop && r.held == NULL);

    keys.id = NULL;
    keys.p = get(ly, "p").as.ref;
    keys.ra = get(ly, "ra").as.ref;
    nr = pico8_nr_new(&keys);
    r = pico8_nr_gv(nr, mz);
    assert(r.stop && r.held == NULL);
}

/* `mz.jj` as the cart has it: `qs` and nothing else, never written. `bv`
 * puts `fo` on the target it has just computed, never on the wrapper a
 * previous `hq` left in `et`. */
static void check_jj_untouched(void) {
    static const char *const absent[] = {"fo", "ra", "p", "me", "t", "nu", "et"};
    void *wrapper = (void *)pico8_mz_jj_wrapper();
    size_t i;

    assert(wrapper != NULL);
    assert(is_string(wrapper, "qs", "-too far-"));
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(wrapper, absent[i]));
    }
}

static void check_fo_is(const void *mz) {
    pico8_value_t et;
    pico8_value_t fo;

    assert(pico8_entity_get_field(mz, "et", &et) && et.kind == PICO8_VALUE_REF && et.as.ref != NULL);
    assert(pico8_entity_get_field(et.as.ref, "fo", &fo) && fo.kind == PICO8_VALUE_REF && fo.as.ref == mz);
}

static void test_jj_wrapper(void) {
    void *wrapper;
    void *mz;
    void *other;
    const pico8_list_t *jcs;
    const void *jc;
    pico8_vec_t mid;
    pico8_value_t et;
    pico8_value_t other_et;

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
    jc = jcs->items[0].as.ref;
    mid = pico8_vec_mul(pico8_vec_div(pico8_vec_add(vec_of(get(jc, "qu").as.ref, "p"), vec_of(get(jc, "re").as.ref, "p")),
                                      pico8_fixed_from_int(2)),
                        pico8_fixed_from_int(128));
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
    assert(pico8_entity_get_field(mz, "et", &et) && pico8_entity_get_field(other, "et", &other_et));
    assert(et.as.ref == other_et.as.ref && et.as.ref == wrapper);
    check_jj_untouched();
}

static int is_a_struct_of(const void *e, const pico8_struct_class_t *cls) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)e)->cls == cls;
}

static int is_an_mz_struct(const void *e) {
    return is_a_struct_of(e, pico8_mz_struct_class());
}

/* Built through `bg:ldown` or directly, an `mz` is a
 * struct with its six methods; the `jj` wrapper is a struct too, the very
 * object of the `jj` member, and has no writer; a generic write of a name
 * other than `ra`/`p`/`et`/`p_` stops the run instead of growing a
 * member. */
static void test_mz_is_a_struct(void) {
    pico8_down_t made;
    pico8_value_t jj;
    void *mz;
    pid_t child;
    int status;

    start_game();
    (void)pico8_bg_ldown(NULL);
    made = pico8_bg_ldown(NULL);
    mz = pico8_mz_new();
    assert(is_an_mz_struct(made.held) && is_an_mz_struct(build_mz()) && is_an_mz_struct(mz));
    assert(pico8_mz_struct_class()->hq != NULL && pico8_mz_struct_class()->ey != NULL && !has(mz, "hq"));
    assert(pico8_mz_struct_class()->ol == NULL);
    assert(pico8_mz_struct_class()->states[PICO8_STATE_KZ] != NULL &&
           pico8_mz_struct_class()->states[PICO8_STATE_PR] != NULL && pico8_mz_struct_class()->kj != NULL &&
           pico8_mz_struct_class()->ev != NULL);
    assert(pico8_mz_struct_class()->states[PICO8_STATE_PH] == NULL && pico8_mz_struct_class()->ldown == NULL);

    assert(is_a_struct_of(pico8_mz_jj_wrapper(), pico8_mz_jj_struct_class()));
    assert(pico8_entity_get_field(mz, "jj", &jj) && jj.as.ref == (const void *)pico8_mz_jj_wrapper());

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        pico8_entity_set_field(pico8_mz_jj_wrapper(), "fo", pico8_value_ref(mz));
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        pico8_entity_set_field(mz, "oi", pico8_value_fixed(0));
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

static void test_jj_qs_by_name(void) {
    static const char *const absent[] = {"fo", "jj", ""};
    const pico8_struct_class_t *cls = pico8_mz_jj_struct_class();
    void *wrapper = (void *)pico8_mz_jj_wrapper();
    pico8_value_t witness = pico8_value_fixed(0x1234);
    pico8_value_t out;
    pico8_value_t again;
    pico8_value_t jj;
    void *mz_a;
    void *mz_b;
    size_t i;
    int k;

    assert(cls->field != NULL && cls->set_field == NULL);

    out = witness;
    assert(pico8_entity_get_field(wrapper, "qs", &out) == 1);
    assert(out.kind == PICO8_VALUE_STRING && strcmp(out.as.string, "-too far-") == 0);
    assert(pico8_entity_get_field(wrapper, "qs", &again) == 1 && again.as.string == out.as.string);

    start_game();
    mz_a = pico8_mz_new();
    mz_b = pico8_mz_new();
    assert(mz_a != mz_b);
    for (k = 0; k < 2; k++) {
        assert(pico8_entity_get_field(k == 0 ? mz_a : mz_b, "jj", &jj) && jj.kind == PICO8_VALUE_REF);
        assert(pico8_entity_get_field(jj.as.ref, "qs", &again) == 1);
        assert(again.kind == PICO8_VALUE_STRING && again.as.string == out.as.string);
    }

    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        out = witness;
        assert(cls->field(wrapper, absent[i], &out) == 0);
        assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == 0x1234);
    }
}

static const char *const mz_names[17] = {"nu", "dw", "dq", "spawn_snd", "ik", "oi", "od", "fp", "fa",
                                         "kk", "jj", "ox", "ra", "p",         "et", "ij", "p_"};

static pico8_value_t zero_union(pico8_value_kind_t kind) {
    pico8_value_t v;

    memset(&v, 0, sizeof v);
    v.kind = kind;
    return v;
}

static int read_mz(const void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(0x1234);
    pico8_value_t via = pico8_value_fixed(0x1234);
    int got_raw = pico8_mz_struct_class()->field(e, name, &raw);
    int got_via = pico8_entity_get_field(e, name, &via);
    assert(got_raw == got_via && raw.kind == via.kind);
    if (!got_raw) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        assert(via.kind == PICO8_VALUE_FIXED && via.as.fixed == 0x1234);
    } else if (raw.kind == PICO8_VALUE_REF) {
        assert(raw.as.ref != NULL && raw.as.ref == via.as.ref);
    } else if (raw.kind == PICO8_VALUE_STRING) {
        assert(raw.as.string != NULL && raw.as.string == via.as.string);
    } else if (raw.kind == PICO8_VALUE_FIXED) {
        assert(raw.as.fixed == via.as.fixed);
    } else if (raw.kind == PICO8_VALUE_BOOL) {
        assert(raw.as.boolean == via.as.boolean);
    }
    *out = raw;
    return got_raw;
}

static void *mz_ref(const void *mz, const char *name) {
    pico8_value_t v;
    if (!read_mz(mz, name, &v)) {
        return NULL;
    }
    assert(v.kind == PICO8_VALUE_REF);
    return v.as.ref;
}

static int mz_bool(const void *mz, const char *name, int want) {
    pico8_value_t v;
    return read_mz(mz, name, &v) && v.kind == PICO8_VALUE_BOOL && v.as.boolean == want;
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == (const void *)&pico8_mz_nu_tags) {
        return fnv(fnv(h, 0x7Au), (uint32_t)pico8_mz_nu_tags.count);
    }
    if (ref == (const void *)&pico8_mz_kk_seq) {
        return fnv(fnv(h, 0x7Au), (uint32_t)pico8_mz_kk_seq.count);
    }
    switch (pico8_type_tag_of(ref)) {
    case PICO8_TYPE_TAG_STRUCT:
        if (ref == (const void *)pico8_mz_jj_wrapper()) {
            return fnv(h, 0xA11u);
        }
        if (is_bv_target(ref)) {
            return fnv(h, 0xB0u);
        }
        return fnv(fnv(h, 0x57u), (uint32_t)rank_in(pico8_hv(), ref));
    case PICO8_TYPE_TAG_LIST:
        return fnv(fnv(h, 0x11u), (uint32_t)((const pico8_list_t *)ref)->count);
    default:
        h = fnv(h, (uint32_t)((const pico8_vec_t *)ref)->x);
        return fnv(h, (uint32_t)((const pico8_vec_t *)ref)->y);
    }
}

static uint32_t fold_mz_member(uint32_t h, const void *mz, const char *name) {
    pico8_value_t v;
    int got = read_mz(mz, name, &v);
    h = fnv(h, (uint32_t)got);
    h = fnv(h, (uint32_t)v.kind);
    switch (v.kind) {
    case PICO8_VALUE_FIXED:
        return fnv(h, (uint32_t)v.as.fixed);
    case PICO8_VALUE_BOOL:
        return fnv(h, (uint32_t)v.as.boolean);
    case PICO8_VALUE_STRING:
        return fnv_str(h, v.as.string);
    case PICO8_VALUE_REF:
        return fold_ref(h, v.as.ref);
    default:
        return fnv(h, 0xF0u);
    }
}

static uint32_t fold_mz_members(uint32_t h, const void *mz) {
    size_t i;
    for (i = 0; i < 17; i++) {
        h = fold_mz_member(h, mz, mz_names[i]);
    }
    return h;
}

static void check_fresh_members(const void *mz, const void *like) {
    static const char *const unset[] = {"ra", "p", "et", "ij", "p_"};
    static const struct {
        const char *name;
        int value;
    } fixed[] = {{"dw", 0}, {"spawn_snd", 52}, {"ik", 49}, {"oi", 0}, {"od", 3}, {"fp", 1}, {"fa", 5}};
    pico8_value_t v;
    pico8_value_t dq;
    size_t i;

    assert(mz_ref(mz, "nu") != NULL && mz_ref(mz, "nu") == mz_ref(like, "nu"));
    assert(mz_ref(mz, "kk") != NULL && mz_ref(mz, "kk") == mz_ref(like, "kk"));
    assert(mz_ref(mz, "jj") == (void *)pico8_mz_jj_wrapper() && mz_ref(like, "jj") == mz_ref(mz, "jj"));
    assert(read_mz(mz, "dq", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, "pr") == 0);
    assert(read_mz(like, "dq", &dq) && dq.as.string == v.as.string);
    assert(mz_ref(mz, "ox") != NULL && (mz == like || mz_ref(mz, "ox") != mz_ref(like, "ox")));
    for (i = 0; i < sizeof fixed / sizeof fixed[0]; i++) {
        assert(read_mz(mz, fixed[i].name, &v) && v.kind == PICO8_VALUE_FIXED);
        assert(v.as.fixed == pico8_fixed_from_int(fixed[i].value));
    }
    for (i = 0; i < sizeof unset / sizeof unset[0]; i++) {
        assert(!read_mz(mz, unset[i], &v));
    }
}

static pico8_vec_t slipway_mid(const void *jc) {
    return pico8_vec_mul(
        pico8_vec_div(pico8_vec_add(vec_of(get(jc, "qu").as.ref, "p"), vec_of(get(jc, "re").as.ref, "p")),
                      pico8_fixed_from_int(2)),
        pico8_fixed_from_int(128));
}

static void test_mz_members_by_name(void) {
    static const char *const unwritten[] = {"", "RA", "p ", "et_", "has_et", "has_p_", "fo", "qu"};
    static const char *const intruders[] = {"", "q", "et ", "ET", "has_et", "has_p_", "header", "ke", "fo", "qs"};
    const pico8_struct_class_t *cls = pico8_mz_struct_class();
    uint32_t h = 2166136261u;
    uint32_t before;
    pico8_value_t mp;
    pico8_value_t v;
    pico8_vec_t want_p;
    void *jc;
    void *mz;
    void *throwaway;
    void *fresh[3];
    void *targets[3];
    const pico8_list_t *ox;
    size_t i;
    int k;

    world();
    jc = pico8_cg("jc")->items[0].as.ref;
    mz = build_mz();
    mp = vec_ref_fixed(slipway_mid(jc));
    hq(mz, mp, vec_ref(64, 64));
    assert(mz_ref(mz, "ra") == mp.as.ref);
    want_p = pico8_vec_div(*(const pico8_vec_t *)mp.as.ref, pico8_fixed_from_int(128));
    assert(mz_ref(mz, "p") != NULL && mz_ref(mz, "p") != mp.as.ref);
    assert(vec_of(mz, "p").x == want_p.x && vec_of(mz, "p").y == want_p.y);
    assert(mz_ref(mz, "et") == jc);
    ox = mz_ref(mz, "ox");
    assert(mz_ref(mz, "ij") != NULL && listed(ox, mz_ref(mz, "ij")));
    h = fold_mz_members(h, mz);
    hq(mz, vec_ref(-640, -640), vec_ref(64, 64));
    assert(mz_ref(mz, "et") == (void *)pico8_mz_jj_wrapper());
    h = fold_mz_members(h, mz);
    hq(mz, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    assert(mz_bool(mz, "et", 0));
    h = fold_mz_members(h, mz);
    hq(mz, vec_ref_fixed(near_anchor()), vec_ref(2, 2));
    throwaway = mz_ref(mz, "et");
    assert(throwaway != NULL && is_bv_target(throwaway));
    assert(mz_ref(mz, "ij") != NULL && listed(ox, mz_ref(mz, "ij")));
    h = fold_mz_members(h, mz);

    mz = build_mz();
    pico8_dg();
    assert(read_mz(mz, "oi", &v) && v.kind == PICO8_VALUE_FIXED);
    ey(mz);
    assert(mz_bool(mz, "p_", 1));
    h = fold_mz_members(h, mz);
    mz = build_mz();
    until_t(mz, 15);
    hq(mz, vec_ref_fixed(near_anchor()), vec_ref(64, 64));
    ey(mz);
    assert(mz_bool(mz, "p_", 0));
    h = fold_mz_member(h, mz, "p_");
    mz = paid_mz_on_unknown();
    pico8_dg();
    assert(mz_bool(mz, "p_", 0));
    h = fold_mz_member(h, mz, "p_");

    mz = build_mz();
    for (k = 0; k < 2; k++) {
        const char *name = k == 0 ? "ra" : "p";
        pico8_value_t a = vec_ref(3, 4);
        pico8_value_t b = vec_ref(5, 6);

        pico8_entity_set_field(mz, name, a);
        assert(mz_ref(mz, name) == a.as.ref);
        h = fold_mz_member(h, mz, name);
        assert(cls->set_field(mz, name, b) == 1);
        assert(mz_ref(mz, name) == b.as.ref);
        pico8_entity_set_field(mz, name, pico8_value_ref(NULL));
        assert(!read_mz(mz, name, &v));
        pico8_entity_set_field(mz, name, a);
        pico8_entity_set_field(mz, name, zero_union(PICO8_VALUE_REF));
        assert(!read_mz(mz, name, &v));
        h = fold_mz_member(h, mz, name);
    }
    pico8_entity_set_field(mz, "et", pico8_value_bool(0));
    assert(mz_bool(mz, "et", 0));
    pico8_entity_set_field(mz, "et", pico8_value_ref(NULL));
    assert(!read_mz(mz, "et", &v));
    pico8_entity_set_field(mz, "et", zero_union(PICO8_VALUE_BOOL));
    assert(mz_bool(mz, "et", 0));
    h = fold_mz_member(h, mz, "et");
    targets[0] = jc;
    targets[1] = throwaway;
    targets[2] = (void *)pico8_mz_jj_wrapper();
    for (k = 0; k < 3; k++) {
        pico8_entity_set_field(mz, "et", pico8_value_ref(targets[k]));
        assert(mz_ref(mz, "et") == targets[k]);
        h = fold_mz_member(h, mz, "et");
        assert(cls->set_field(mz, "et", pico8_value_ref(targets[(k + 1) % 3])) == 1);
        assert(mz_ref(mz, "et") == targets[(k + 1) % 3]);
        pico8_entity_set_field(mz, "et", pico8_value_ref(NULL));
        assert(!read_mz(mz, "et", &v));
    }
    pico8_entity_set_field(mz, "et", pico8_value_ref(jc));
    pico8_entity_set_field(mz, "et", zero_union(PICO8_VALUE_REF));
    assert(!read_mz(mz, "et", &v));

    pico8_entity_set_field(mz, "p_", pico8_value_bool(1));
    assert(mz_bool(mz, "p_", 1));
    h = fold_mz_member(h, mz, "p_");
    pico8_entity_set_field(mz, "p_", pico8_value_ref(NULL));
    assert(!read_mz(mz, "p_", &v));
    pico8_entity_set_field(mz, "p_", pico8_value_bool(0));
    assert(mz_bool(mz, "p_", 0));
    h = fold_mz_member(h, mz, "p_");
    pico8_entity_set_field(mz, "p_", pico8_value_ref(NULL));
    assert(!read_mz(mz, "p_", &v));
    pico8_entity_set_field(mz, "p_", zero_union(PICO8_VALUE_BOOL));
    assert(mz_bool(mz, "p_", 0));
    h = fold_mz_member(h, mz, "p_");
    pico8_entity_set_field(mz, "p_", pico8_value_ref(NULL));
    pico8_entity_set_field(mz, "p_", pico8_value_ref(NULL));
    assert(!read_mz(mz, "p_", &v));
    h = fold_mz_member(h, mz, "p_");

    pico8_entity_set_field(mz, "ra", vec_ref(7, 8));
    pico8_entity_set_field(mz, "p", vec_ref(9, 10));
    pico8_entity_set_field(mz, "et", pico8_value_ref(throwaway));
    pico8_entity_set_field(mz, "p_", pico8_value_bool(1));
    before = fold_mz_members(2166136261u, mz);
    for (i = 0; i < 17; i++) {
        if (strcmp(mz_names[i], "ra") != 0 && strcmp(mz_names[i], "p") != 0 && strcmp(mz_names[i], "et") != 0 &&
            strcmp(mz_names[i], "p_") != 0) {
            assert(cls->set_field(mz, mz_names[i], pico8_value_fixed(pico8_fixed_from_int(9))) == 0);
        }
    }
    for (i = 0; i < sizeof unwritten / sizeof unwritten[0]; i++) {
        assert(cls->set_field(mz, unwritten[i], pico8_value_fixed(pico8_fixed_from_int(9))) == 0);
    }
    assert(fold_mz_members(2166136261u, mz) == before);
    h = fnv(h, before);
    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        pico8_value_t raw = pico8_value_fixed(0x1234);
        assert(cls->field(mz, intruders[i], &raw) == 0);
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        h = fold_mz_member(h, mz, intruders[i]);
    }

    start_game();
    fresh[0] = pico8_mz_new();
    fresh[1] = pico8_mz_new();
    (void)pico8_bg_ldown(NULL);
    fresh[2] = pico8_bg_ldown(NULL).held;
    for (k = 0; k < 3; k++) {
        check_fresh_members(fresh[k], fresh[0]);
        h = fold_mz_members(h, fresh[k]);
    }
    assert(((const pico8_list_t *)mz_ref(fresh[0], "ox"))->count == 0);
    hq(fresh[0], vec_ref(64, 64), vec_ref(64, 64));
    assert(!read_mz(fresh[0], "ij", &v));
    assert(mz_ref(fresh[0], "ra") != NULL && mz_ref(fresh[0], "p") != NULL && read_mz(fresh[0], "et", &v));
    h = fold_mz_members(h, fresh[0]);
    check("mz members", h, MZ_MEMBERS_HASH);
}

static pico8_value_t g_off_row;
static const char *g_off_name;

static void must_abort(void (*write)(void *), void *mz) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        write(mz);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

static void write_off_row(void *mz) {
    pico8_entity_set_field(mz, g_off_name, g_off_row);
}

static void stops_on(void *mz, const char *name, const pico8_value_t *off, size_t n) {
    size_t i;

    g_off_name = name;
    for (i = 0; i < n; i++) {
        g_off_row = off[i];
        must_abort(write_off_row, mz);
    }
}

static void test_mz_writer_stops_off_row(void) {
    pico8_value_t list;
    pico8_value_t vec;
    pico8_value_t jo;
    pico8_value_t other;
    void *mz;

    world();
    mz = build_mz();
    list = pico8_value_ref((void *)pico8_hv());
    vec = vec_ref(1, 1);
    jo = pico8_cg("jo")->items[0];
    other = pico8_value_ref(build_mz());
    {
        const pico8_value_t off[] = {pico8_value_fixed(pico8_fixed_from_int(1)),
                                     zero_union(PICO8_VALUE_FIXED),
                                     pico8_value_bool(1),
                                     zero_union(PICO8_VALUE_BOOL),
                                     pico8_value_string("x"),
                                     zero_union(PICO8_VALUE_STRING),
                                     list,
                                     jo,
                                     other};
        stops_on(mz, "ra", off, sizeof off / sizeof off[0]);
        stops_on(mz, "p", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_bool(1),
                                     pico8_value_fixed(pico8_fixed_from_int(1)),
                                     zero_union(PICO8_VALUE_FIXED),
                                     pico8_value_string("x"),
                                     zero_union(PICO8_VALUE_STRING),
                                     list,
                                     vec,
                                     jo,
                                     pico8_value_ref(pico8_qh_new(NULL)),
                                     pico8_value_ref(pico8_g__new(NULL)),
                                     other,
                                     pico8_value_ref(pico8_bg_new())};
        stops_on(mz, "et", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_fixed(pico8_fixed_from_int(1)),
                                     zero_union(PICO8_VALUE_FIXED),
                                     pico8_value_string("x"),
                                     zero_union(PICO8_VALUE_STRING),
                                     vec,
                                     other};
        stops_on(mz, "p_", off, sizeof off / sizeof off[0]);
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
    in_fresh_child(test_jj_qs_by_name);
    in_fresh_child(test_mz_members_by_name);
    in_fresh_child(test_mz_writer_stops_off_row);

    printf("test_pico8_mz: all tests passed\n");
    return 0;
}
