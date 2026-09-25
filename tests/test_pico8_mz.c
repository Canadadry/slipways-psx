/* The wormhole `mz`, read and written only through the dual-form field API
 * and driven through the real `mv`/`dg`. The frozen values were taken on
 * the code where `mz` was still a class instance and must not be
 * re-taken. */

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
#include "../src/pico8_cheat.h"
#include "../src/pico8_class.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"
#include "../src/pico8_wormhole.h"

#define PLAYTHROUGH "tests/fixtures/phase4_playthrough_events.json"
#define GEOHARVEST "tests/fixtures/geoharvest_burst_events.json"
#define GEOHARVEST_FRAMES 41

#define OX_HASH 0x3FA66D65u
#define HQ_HASH 0xC7BB40AEu
#define KZ_TICKS 21
#define KZ_HASH 0xE16D1D52u
#define EY_HASH 0x89F623D4u
#define PR_HASH 0xAC265666u
#define PR_EXIT_TICK 27
#define DRAW_HASH 0x9A6C2110u
#define PLAYTHROUGH_HASH 0x27C7A366u
#define PLAYTHROUGH_FRAMES 224
#define GEOHARVEST_HASH 0xB6BD5161u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

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
    if (pico8_type_tag_of(et.as.ref) == PICO8_TYPE_TAG_INSTANCE &&
        ((const pico8_instance_t *)et.as.ref)->cls == NULL) {
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

static void start_game(const char *events) {
    pico8_value_t difficulty;

    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
    pico8__init();
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty) && difficulty.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty.as.ref));
    pico8_cheat_init(&cheat);
}

static void play_frame(void) {
    pico8_dispatch_set_input(&input);
    pico8_cheat_step(&cheat, &input);
    pico8__update60();
    pico8__draw();
    if (input.current_frame + 1 < input.frame_count) {
        pico8_input_step(&input);
    }
}

/* The playthrough up to after its slipway and score tick: a probe, two
 * colonized planets joined by a slipway, and unknown planets left. */
static void world(void) {
    int i;

    start_game(PLAYTHROUGH);
    for (i = 0; i < input.frame_count; i++) {
        play_frame();
    }
}

static void *build_mz(void) {
    pico8_class_t *cls = pico8_class_lookup_named("mz");
    assert(cls != NULL);
    return pico8_class_instantiate(cls, NULL);
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
    pico8_value_t out;
    assert(pico8_mv(mz, "hq", 2, mp, rp, &out));
}

static void ey(void *mz) {
    pico8_value_t out;
    assert(pico8_mv(mz, "ey", 1, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
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
    const pico8_ob_table_t *kk;
    const pico8_ob_table_t *nu;
    const pico8_ob_table_t *jj;
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
    assert(kk->count == 9);
    for (i = 0; i < 9; i++) {
        pico8_value_t entry;
        assert(pico8_ob_table_get_index(kk, i + 1, &entry) && entry.kind == PICO8_VALUE_FIXED);
        assert(entry.as.fixed == pico8_fixed_from_int(want_kk[i]));
    }
    assert(pico8_entity_get_field(mz, "nu", &v) && v.kind == PICO8_VALUE_REF);
    nu = v.as.ref;
    assert(pico8_ob_table_get_index(nu, 1, &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, "mz") == 0);
    assert(pico8_entity_get_field(mz, "jj", &v) && v.kind == PICO8_VALUE_REF);
    jj = v.as.ref;
    assert(pico8_ob_table_get_name(jj, "qs", &v) && v.kind == PICO8_VALUE_STRING &&
           strcmp(v.as.string, "-too far-") == 0);
    assert(pico8_mz_jj_wrapper()->fields == jj);

    assert(!has(mz, "ra") && !has(mz, "p") && !has(mz, "et") && !has(mz, "ij") && !has(mz, "p_"));
    assert(!has(mz, "fx") && !has(mz, "qu"));
    assert(has(mz, "ox"));

    assert(listed(pico8_hv(), mz) && listed(pico8_cv("kj"), mz) && listed(pico8_cv("ev"), mz));
    assert(listed(pico8_cg("mz"), mz));
    assert(!listed(pico8_cv("hd"), mz) && !listed(pico8_cv("hk"), mz) && !listed(pico8_cv("hb"), mz));
}

static void test_construction(void) {
    pico8_value_t made;
    const pico8_list_t *mzs;
    int before;

    start_game(PLAYTHROUGH);
    assert(pico8_cg("g_") == NULL || pico8_cg("g_")->count == 0);
    mzs = pico8_cg("mz");
    before = mzs == NULL ? 0 : mzs->count;
    made = pico8_bg_ldown(NULL, pico8_value_ref(NULL));
    assert(made.kind == PICO8_VALUE_REF && made.as.ref != NULL);
    assert(listed(pico8_cg("g_"), made.as.ref));
    mzs = pico8_cg("mz");
    assert((mzs == NULL ? 0 : mzs->count) == before);

    made = pico8_bg_ldown(NULL, pico8_value_ref(NULL));
    assert(made.kind == PICO8_VALUE_REF && made.as.ref != NULL);
    check_fresh_mz(made.as.ref);
    check_fresh_mz(build_mz());
    check_fresh_mz(pico8_mz_new(NULL));
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
    pico8_entity_set_field(pico8_fm(), "nz", pico8_value_fixed(pico8_fixed_from_int(2)));
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
    pico8_value_t out;

    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    assert(pico8_mv(mz, "kj", 1, vec_ref(64, 64), pico8_value_ref(NULL), &out));
    assert(pico8_mv(mz, "ev", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
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

/* Every frame of a replay: each live `mz`'s state and the credits. */
static uint32_t fold_frame(uint32_t h) {
    const pico8_list_t *mzs = pico8_cg("mz");
    int i;

    for (i = 0; mzs != NULL && i < mzs->count; i++) {
        const void *mz = mzs->items[i].as.ref;
        h = fold_string(h, mz, "me");
        h = fold_fixed(h, mz, "t");
        h = fold_fixed(h, mz, "oi");
        h = fold_vec(h, mz, "ra");
        h = fold_vec(h, mz, "p");
        h = fnv(h, (uint32_t)et_kind(mz));
        h = fnv(h, bool_state(mz, "p_"));
    }
    return fnv(h, (uint32_t)credits());
}

static void test_playthrough(void) {
    uint32_t h = 2166136261u;
    int seen = 0;
    int frames;
    int i;

    start_game(PLAYTHROUGH);
    frames = input.frame_count;
    for (i = 0; i < frames; i++) {
        play_frame();
        h = fold_frame(h);
        if (pico8_cg("mz") != NULL && pico8_cg("mz")->count > 0) {
            seen++;
        }
    }
    printf("[playthrough] an mz alive on %d frames\n", seen);
    assert(seen > 0);
    check_int("playthrough frames", frames, PLAYTHROUGH_FRAMES);
    check("playthrough", h, PLAYTHROUGH_HASH);
}

static void test_geoharvest(void) {
    uint32_t h = 2166136261u;
    int seen = 0;
    int i;

    start_game(GEOHARVEST);
    for (i = 0; i < GEOHARVEST_FRAMES; i++) {
        play_frame();
        h = fold_frame(h);
        if (pico8_cg("mz") != NULL && pico8_cg("mz")->count > 0) {
            seen++;
        }
    }
    printf("[geoharvest] an mz alive on %d frames\n", seen);
    assert(seen > 0);
    check("geoharvest", h, GEOHARVEST_HASH);
}

/* Released over a colonized planet or a slipgate, `jz("gv",held)` reads
 * `s.qu` off the `mz`, which it never has. */
static void test_gv(void) {
    void *mz;
    void *ly;
    pico8_instance_t *nr;
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    pico8_value_t r;

    world();
    mz = build_mz();
    ly = first_with_me("jo", "ly");
    assert(ly != NULL);
    r = pico8_jo_gv(ly, mz);
    assert(r.kind == PICO8_VALUE_BOOL && r.as.boolean);

    pico8_ob_table_set_name(overrides, "p", get(ly, "p"));
    pico8_ob_table_set_name(overrides, "ra", get(ly, "ra"));
    nr = pico8_nr_new(overrides);
    pico8_ob_table_free(overrides);
    r = pico8_nr_gv(nr, mz);
    assert(r.kind == PICO8_VALUE_BOOL && r.as.boolean);
}

static int is_an_mz_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_mz_struct_class();
}

/* Built through `bg:ldown`'s `lookup_named` or directly, an `mz` is a
 * struct with its six methods; the `jj` wrapper stays a classless
 * instance on the very table of the `jj` member; a generic write of a
 * name other than `ra`/`p`/`et`/`p_` stops the run instead of growing a
 * member. */
static void test_mz_is_a_struct(void) {
    static const char *const methods[] = {"hq", "ey", "kz", "pr", "kj", "ev"};
    pico8_value_t made;
    pico8_value_t jj;
    void *mz;
    size_t i;
    pid_t child;
    int status;

    start_game(PLAYTHROUGH);
    (void)pico8_bg_ldown(NULL, pico8_value_ref(NULL));
    made = pico8_bg_ldown(NULL, pico8_value_ref(NULL));
    mz = pico8_mz_new(NULL);
    assert(is_an_mz_struct(made.as.ref) && is_an_mz_struct(build_mz()) && is_an_mz_struct(mz));
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        pico8_value_t m;
        assert(pico8_entity_get_field(mz, methods[i], &m) && m.kind == PICO8_VALUE_FUNCTION);
    }
    assert(!has(mz, "ph") && !has(mz, "ldown") && !has(mz, "md") && !has(mz, "ol"));

    assert(pico8_type_tag_of(pico8_mz_jj_wrapper()) == PICO8_TYPE_TAG_INSTANCE);
    assert(pico8_mz_jj_wrapper()->cls == NULL);
    assert(pico8_entity_get_field(mz, "jj", &jj) && jj.as.ref == pico8_mz_jj_wrapper()->fields);

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

/* A replay depends on the rnd state: run each scenario in a child forked
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
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_construction);
    in_fresh_child(test_ox);
    in_fresh_child(test_hq);
    in_fresh_child(test_kz_ticks);
    in_fresh_child(test_ey);
    in_fresh_child(test_pr_ticks);
    in_fresh_child(test_draw);
    in_fresh_child(test_playthrough);
    in_fresh_child(test_geoharvest);
    in_fresh_child(test_gv);
    in_fresh_child(test_mz_is_a_struct);

    printf("test_pico8_mz: all tests passed\n");
    return 0;
}
