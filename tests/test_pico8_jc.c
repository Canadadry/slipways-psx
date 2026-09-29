/* The slipway `jc`, read and written only through the dual-form field API
 * and driven through the real `mv`/`dg`/`dc`/`ok`, in games built directly:
 * no events file. */

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
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
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

#define DW_BITS 0x00000106
#define EY_HASH 0x2D4E976Fu
#define HQ_HASH 0x369116ECu
#define EB_HASH 0x19ADC88Cu
#define DRAW_HASH 0xABD825A2u
#define BV_HASH 0x731DE4D8u
#define TOO_FAR_HASH 0xBFB9FB9Bu
#define MOUSE_POINT_HASH 0x78A94031u
#define MEMBERS_HASH 0x27A294B4u
#define FAR_MEMBERS_HASH 0x34D70700u
#define JC_MEMBERS_HASH 0xB314E25Fu

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static const int want_k_[20] = {0, 0, 7, 7, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 4, 4, 4, 2, 2, 1};

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

/* The instance reader keeps a field written nil, the struct reader reads a
 * nil member as absent: nil and absent are one state here. */
static int lookup(const void *e, const char *name, pico8_value_t *out) {
    return pico8_entity_get_field(e, name, out) && !(out->kind == PICO8_VALUE_REF && out->as.ref == NULL);
}

static int has(const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v);
}

static pico8_value_t get(const void *e, const char *name) {
    pico8_value_t v = pico8_value_ref(NULL);
    (void)pico8_entity_get_field(e, name, &v);
    return v;
}

static void *get_ref(const void *e, const char *name) {
    pico8_value_t v = get(e, name);
    return v.kind == PICO8_VALUE_REF ? v.as.ref : NULL;
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

/* 0 absent, 1 false, 2 true, 3 other. */
static uint32_t bool_state(const void *e, const char *name) {
    pico8_value_t v;
    if (!lookup(e, name, &v)) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_BOOL) {
        return v.as.boolean ? 2 : 1;
    }
    return 3;
}

static int is_throwaway(const void *e) {
    return e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_jc_mouse_point_struct_class();
}

static int is_too_far(const void *e) {
    return e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_jc_too_far_struct_class() &&
           is_string(e, "qs", "-too far-");
}

/* A value by what it is, a referenced entity by its rank in `hv`. */
static uint32_t fold_value(uint32_t h, pico8_value_t v) {
    switch (v.kind) {
    case PICO8_VALUE_BOOL:
        return fnv(fnv(h, 0xB0u), (uint32_t)v.as.boolean);
    case PICO8_VALUE_FIXED:
        return fnv(fnv(h, 0xF1u), (uint32_t)v.as.fixed);
    case PICO8_VALUE_STRING:
        return fnv_str(fnv(h, 0x57u), v.as.string);
    case PICO8_VALUE_REF:
        if (v.as.ref == NULL) {
            return fnv(h, 0xDEADu);
        }
        if (listed(pico8_hv(), v.as.ref)) {
            return fnv(fnv(h, 0x4Eu), (uint32_t)rank_in(pico8_hv(), v.as.ref));
        }
        /* The frozen hashes folded the `{qs}` as a class-less throwaway. */
        return fnv(h, is_too_far(v.as.ref) || is_throwaway(v.as.ref) ? 0x7Au : 0xEEEEu);
    default:
        return fnv(h, 0xF0u);
    }
}

static uint32_t fold_field(uint32_t h, const void *e, const char *name) {
    return fold_value(h, get(e, name));
}

static uint32_t fold_vec(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    const pico8_vec_t *p;
    if (!lookup(e, name, &v)) {
        return fnv(h, 0xDEADu);
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    p = v.as.ref;
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

enum { RE_ABSENT, RE_PLANET, RE_THROWAWAY, RE_OTHER };

static int re_kind(const void *jc) {
    void *re = get_ref(jc, "re");
    if (re == NULL) {
        return RE_ABSENT;
    }
    if (listed(pico8_cg("np"), re)) {
        return RE_PLANET;
    }
    if (is_throwaway(re)) {
        return RE_THROWAWAY;
    }
    return RE_OTHER;
}

enum { ET_ABSENT, ET_FALSE, ET_PLANET, ET_JC, ET_QS, ET_OTHER };

static int et_kind(const void *jc) {
    pico8_value_t et;
    if (!lookup(jc, "et", &et)) {
        return ET_ABSENT;
    }
    if (et.kind == PICO8_VALUE_BOOL && !et.as.boolean) {
        return ET_FALSE;
    }
    if (et.kind != PICO8_VALUE_REF || et.as.ref == NULL) {
        return ET_OTHER;
    }
    if (listed(pico8_cg("np"), et.as.ref)) {
        return ET_PLANET;
    }
    if (listed(pico8_cg("jc"), et.as.ref)) {
        return ET_JC;
    }
    if (is_too_far(et.as.ref)) {
        return ET_QS;
    }
    return ET_OTHER;
}

enum { FO_ABSENT, FO_JC, FO_PROBE, FO_MZ, FO_OTHER };

static int holder_kind(const void *e) {
    void *fo = get_ref(e, "fo");
    if (fo == NULL) {
        return FO_ABSENT;
    }
    if (listed(pico8_cg("jc"), fo)) {
        return FO_JC;
    }
    if (listed(pico8_cg("g_"), fo)) {
        return FO_PROBE;
    }
    if (listed(pico8_cg("mz"), fo)) {
        return FO_MZ;
    }
    return FO_OTHER;
}

static int list_count(const void *e, const char *name) {
    void *l = get_ref(e, name);
    return l == NULL ? -1 : ((const pico8_list_t *)l)->count;
}

/* The count, then each `id` and `true`. */
static uint32_t fold_hg(uint32_t h, const void *jc) {
    const pico8_jc_hg_t *hg = get_ref(jc, "hg");
    int i;
    if (hg == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)hg->count);
    for (i = 0; i < hg->count; i++) {
        h = fnv_str(h, hg->ids[i]);
        h = fold_value(h, pico8_value_bool(1));
    }
    return h;
}

static uint32_t fold_jc(uint32_t h, const void *jc) {
    pico8_value_t et;

    h = fold_field(h, jc, "od");
    h = fold_field(h, jc, "mt");
    h = fold_field(h, jc, "fa");
    h = fold_field(h, jc, "nh");
    h = fnv(h, bool_state(jc, "p_"));
    h = fold_field(h, jc, "me");
    h = fold_field(h, jc, "t");
    h = fold_field(h, jc, "qu");
    h = fnv(h, (uint32_t)re_kind(jc));
    h = fold_field(h, jc, "re");
    if (re_kind(jc) == RE_THROWAWAY) {
        h = fold_vec(h, get_ref(jc, "re"), "p");
    }
    h = fnv(h, (uint32_t)et_kind(jc));
    et = get(jc, "et");
    h = fold_value(h, et);
    h = fold_field(h, jc, "fo");
    h = fnv(h, (uint32_t)holder_kind(jc));
    h = fnv(h, (uint32_t)list_count(jc, "ts"));
    return fold_hg(h, jc);
}

static pico8_fixed_t credits(void) {
    return must_fixed(pico8_fm(), "nz");
}

static uint32_t fold_planets(uint32_t h, int with_eb) {
    const pico8_list_t *np = pico8_cg("np");
    int i;

    for (i = 0; np != NULL && i < np->count; i++) {
        void *p = np->items[i].as.ref;
        const pico8_list_t *oe = get_ref(p, "oe");
        h = fnv(h, (uint32_t)list_count(p, "oe"));
        h = fold_field(h, p, "fo");
        h = fnv(h, (uint32_t)holder_kind(p));
        if (with_eb && oe != NULL) {
            int j;
            for (j = 0; j < oe->count; j++) {
                pico8_value_t out;
                assert(pico8_mv(oe->items[j].as.ref, "eb", 1, pico8_value_ref(p), pico8_value_ref(NULL), &out));
                h = fold_value(h, out);
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

/* A started game's first generated planet and its nearest one, both
 * colonized by a bare project. */
static void colonized_pair(void **qu, void **re) {
    const pico8_list_t *jos;
    int i;

    start_game();
    jos = pico8_cg("jo");
    assert(jos != NULL && jos->count > 1);
    *qu = jos->items[0].as.ref;
    *re = NULL;
    for (i = 1; i < jos->count; i++) {
        void *e = jos->items[i].as.ref;
        if (*re == NULL || dist2(pos_of(e), pos_of(*qu)) < dist2(pos_of(*re), pos_of(*qu))) {
            *re = e;
        }
    }
    pico8_jo_hu(*qu, pico8_hp_new(NULL));
    pico8_jo_hu(*re, pico8_hp_new(NULL));
}

static void *build_jc(void *qu);
static void hq_over(void *jc, void *hovered, pico8_value_t mp);

/* A slipway from `qu` dragged over `re` as `jc:hq` sees a drag (it reads the
 * hovered planet off `ok.o_`), held until its `t` reaches 15, not paid. */
static void *dragged(void *qu, void *re) {
    pico8_vec_t *mp = malloc(sizeof *mp);
    void *jc = build_jc(qu);

    *mp = pico8_vec_add(pico8_vec_mul(pos_of(re), pico8_fixed_from_int(128)),
                        pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(1)));
    hq_over(jc, re, pico8_value_ref(mp));
    pico8_ok()->o_ = NULL;
    while (must_fixed(jc, "t") < pico8_fixed_from_int(15)) {
        pico8_dg();
    }
    return jc;
}

/* Two colonized planets joined by one slipway, paid by `ey` as a drag pays
 * it, then left to settle. */
static void world(void) {
    pico8_value_t out;
    void *qu;
    void *re;
    void *jc;
    int i;

    colonized_pair(&qu, &re);
    jc = dragged(qu, re);
    assert(pico8_mv(jc, "ey", 1, pico8_value_ref(re), pico8_value_ref(NULL), &out));
    for (i = 0; i < SETTLE_TICKS; i++) {
        pico8_dg();
    }
    assert(pico8_cg("jc") != NULL && pico8_cg("jc")->count == 1);
}

static void *the_slipway(void) {
    return pico8_cg("jc")->items[0].as.ref;
}

static void *build_jc(void *qu) {
    pico8_jc_keys_t keys = {0};

    keys.qu = qu;
    return pico8_jc_new(&keys);
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

static pico8_vec_t vec_of(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return *(const pico8_vec_t *)v.as.ref;
}

static const pico8_struct_class_t *cls_of(const void *e) {
    return ((const pico8_struct_header_t *)e)->cls;
}

static pico8_value_t call(void *e, const char *name, pico8_value_t arg0) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, 1, arg0, pico8_value_ref(NULL), &out));
    return out;
}

/* `jc:hq` reads the hovered entity off `ok.o_`. */
static void hq_over(void *jc, void *hovered, pico8_value_t mp) {
    pico8_ok()->o_ = hovered;
    (void)call(jc, "hq", mp);
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

/* A fresh `jc`: the spec's defaults, ke's `t`, `qu`, and the `ts`/`hg` its
 * `pq` made; registered like any `ke` entity with `kj` and `ev`. */
static void check_fresh_jc(const void *jc, const void *qu) {
    static const char *const absent[] = {"re", "od", "mt", "et", "fo", "p_", "ra", "hd", "ql", "fx", "dq"};
    static const char *const methods[] = {"hq", "ey", "eb"};
    pico8_value_t v;
    const pico8_int_seq_t *k_;
    const pico8_str_seq_t *nu;
    const pico8_jc_hg_t *hg;
    size_t i;

    assert(is_string(jc, "me", "kz"));
    assert(must_fixed(jc, "t") == 0);
    assert(must_fixed(jc, "fa") == pico8_fixed_from_int(2));
    assert(pico8_entity_get_field(jc, "fp", &v) && v.kind == PICO8_VALUE_BOOL && !v.as.boolean);
    assert(must_fixed(jc, "ik") == pico8_fixed_from_int(48));
    printf("[dw] 0x%08X\n", (unsigned)must_fixed(jc, "dw"));
    assert(must_fixed(jc, "dw") == (pico8_fixed_t)DW_BITS);
    assert(must_fixed(jc, "nh") == pico8_fixed_from_int(20));

    k_ = get_ref(jc, "k_");
    assert(k_ == &pico8_jc_k__seq && k_->count == 20);
    for (i = 0; i < 20; i++) {
        assert(k_->items[i] == want_k_[i]);
    }
    assert(pico8_entity_get_field(jc, "nu", &v) && v.kind == PICO8_VALUE_REF);
    nu = v.as.ref;
    assert(nu == &pico8_jc_nu_tags && nu->count == 1 && strcmp(nu->items[0], "jc") == 0);
    assert(get_ref(jc, "qu") == qu);

    assert(list_count(jc, "ts") == 0);
    hg = get_ref(jc, "hg");
    assert(hg == &((const pico8_jc_t *)jc)->hg_own && hg->count == 0);

    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(jc, absent[i]));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        assert(pico8_entity_get_field(jc, methods[i], &v) && v.kind == PICO8_VALUE_FUNCTION);
    }
    assert(cls_of(jc)->ev != NULL && cls_of(jc)->kj != NULL);
    assert(cls_of(jc)->hd == NULL && cls_of(jc)->states[PICO8_STATE_KZ] == NULL &&
           cls_of(jc)->states[PICO8_STATE_PH] == NULL && cls_of(jc)->ldown == NULL && cls_of(jc)->gv == NULL);

    assert(listed(pico8_hv(), jc) && listed(pico8_cv("kj"), jc) && listed(pico8_cv("ev"), jc));
    assert(listed(pico8_cg("jc"), jc));
    assert(!listed(pico8_cv("hd"), jc) && !listed(pico8_cv("hk"), jc) && !listed(pico8_cv("hb"), jc));
}

static void test_construction(void) {
    void *ly;
    void *by_ldown;
    void *by_lookup;
    pico8_down_t made;

    world();
    ly = first_with_me("jo", "ly");
    assert(ly != NULL);
    made = cls_of(ly)->ldown(ly, NULL, NULL);
    assert(made.stop && made.held != NULL);
    by_ldown = made.held;
    check_fresh_jc(by_ldown, ly);
    by_lookup = build_jc(ly);
    check_fresh_jc(by_lookup, ly);

    assert(get_ref(by_ldown, "k_") == get_ref(by_lookup, "k_"));
    assert(get_ref(by_ldown, "nu") == get_ref(by_lookup, "nu"));
    assert(get_ref(by_ldown, "ts") != get_ref(by_lookup, "ts"));
    assert(get_ref(by_ldown, "hg") != get_ref(by_lookup, "hg"));
}

/* Without a target, `ey` only raises `p_`; with one, the slipway joins both
 * `oe` and `d_` charges `od`. */
static void test_ey(void) {
    uint32_t h = 2166136261u;
    void *held;
    void *origin;
    void *target;
    void *refused;
    pico8_fixed_t before;
    pico8_fixed_t od;

    colonized_pair(&origin, &target);
    held = dragged(origin, target);
    assert(listed(pico8_cg("jc"), held));
    assert(get_ref(held, "qu") == origin && get_ref(held, "re") == target);
    assert(re_kind(held) == RE_PLANET && et_kind(held) == ET_ABSENT);
    assert(list_count(origin, "oe") == 0 && list_count(target, "oe") == 0);
    before = credits();
    od = must_fixed(held, "od");
    h = fold_jc(h, held);

    refused = build_jc(origin);
    (void)call(refused, "ey", pico8_value_ref(NULL));
    assert(bool_state(refused, "p_") == 2 && credits() == before);
    assert(list_count(origin, "oe") == 0);
    h = fold_jc(h, refused);

    (void)call(held, "ey", pico8_value_ref(target));
    assert(must_fixed(held, "nh") == pico8_fixed_from_int(1));
    assert(get_ref(held, "re") == target);
    assert(bool_state(held, "p_") == 0);
    assert(list_count(origin, "oe") == 1 && list_count(target, "oe") == 1);
    assert(credits() == pico8_fixed_sub(before, od));
    h = fold_jc(h, held);
    h = fold_planets(h, 1);
    h = fnv(h, (uint32_t)credits());
    check("ey", h, EY_HASH);
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
    origin = get_ref(old, "qu");
    target = get_ref(old, "re");

    far = build_jc(origin);
    hq_over(far, NULL, vec_ref(-640, -640));
    assert(re_kind(far) == RE_THROWAWAY && et_kind(far) == ET_QS);
    assert(get_ref(get_ref(far, "et"), "fo") == far);
    h = fold_jc(h, far);

    twin = build_jc(origin);
    hq_over(twin, target, vec_ref_fixed(vec_of(target, "ra")));
    assert(re_kind(twin) == RE_PLANET && get_ref(twin, "re") == target);
    assert(et_kind(twin) == ET_JC && get_ref(twin, "et") == old);
    assert(get_ref(old, "fo") == twin);
    h = fold_jc(h, twin);
    h = fold_jc(h, old);
    check("hq", h, HQ_HASH);
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
    origin = get_ref(old, "qu");
    target = get_ref(old, "re");
    fresh = build_jc(origin);

    assert(call(old, "eb", pico8_value_ref(origin)).as.ref == target);
    assert(call(old, "eb", pico8_value_ref(target)).as.ref == origin);
    assert(call(fresh, "eb", pico8_value_ref(origin)).as.ref == origin);
    assert(call(fresh, "eb", pico8_value_ref(target)).as.ref == origin);
    h = fold_value(h, call(old, "eb", pico8_value_ref(origin)));
    h = fold_value(h, call(old, "eb", pico8_value_ref(target)));
    h = fold_value(h, call(fresh, "eb", pico8_value_ref(origin)));
    h = fold_value(h, call(fresh, "eb", pico8_value_ref(NULL)));
    check("eb", h, EB_HASH);
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

/* Under the game's camera: the slipway alone on a cleared screen, `fo` cleared after drawing; then a
 * planet whose `fo` is a slipway, circled at `dw`'s radius. */
static void test_draw(void) {
    uint32_t h = 2166136261u;
    uint32_t blank;
    uint32_t plain;
    uint32_t circled;
    pico8_vec_t mo_ra;
    void *old;
    void *origin;
    int i;

    world();
    old = the_slipway();
    origin = get_ref(old, "qu");
    mo_ra = vec_of(pico8_mo(), "ra");
    pico8_camera(&gfx, pico8_fixed_flr_int(mo_ra.x), pico8_fixed_flr_int(mo_ra.y));
    pico8_cls(&mem);
    blank = screen_hash();

    for (i = 0; i < 3; i++) {
        pico8_entity_set_field(old, "fo", pico8_value_ref(old));
        pico8_cls(&mem);
        cls_of(old)->kj(old, NULL);
        assert(!has(old, "fo"));
        assert(screen_hash() != blank);
        h = fnv(h, screen_hash());
        h = fold_jc(h, old);
    }

    pico8_cls(&mem);
    cls_of(origin)->kj(origin, (vec_ref_fixed(vec_of(origin, "ra"))).as.ref);
    plain = screen_hash();
    pico8_entity_set_field(origin, "fo", pico8_value_ref(old));
    pico8_cls(&mem);
    cls_of(origin)->kj(origin, (vec_ref_fixed(vec_of(origin, "ra"))).as.ref);
    circled = screen_hash();
    assert(!has(origin, "fo"));
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
    pico8_vec_t mo_ra;
    pico8_vec_t mouse;
    pico8_value_t nh;
    pico8_fixed_t before;
    void *old;
    void *origin;
    void *target;
    void *far;
    void *twin;
    void *near;
    void *re;

    world();
    old = the_slipway();
    origin = get_ref(old, "qu");
    target = get_ref(old, "re");
    far = build_jc(origin);
    hq_over(far, NULL, vec_ref(-640, -640));
    assert(re_kind(far) == RE_THROWAWAY && et_kind(far) == ET_QS);
    assert(et_kind(old) == ET_ABSENT);
    mo_ra = vec_of(pico8_mo(), "ra");
    pico8_camera(&gfx, pico8_fixed_flr_int(mo_ra.x), pico8_fixed_flr_int(mo_ra.y));
    pico8_cls(&mem);
    blank = screen_hash();

    cls_of(far)->kj(far, NULL);
    assert(screen_hash() != blank);
    h = fnv(h, screen_hash());

    twin = build_jc(origin);
    hq_over(twin, target, vec_ref_fixed(vec_of(target, "ra")));
    assert(et_kind(twin) == ET_JC);
    near = build_jc(origin);
    hq_over(near, NULL, vec_ref_fixed(pico8_vec_add(vec_of(origin, "ra"), pico8_vec_v(pico8_fixed_from_int(16), 0))));
    assert(re_kind(near) == RE_THROWAWAY && et_kind(near) == ET_ABSENT && has(near, "od"));
    mouse = vec_of(pico8_je(), "ra");
    pico8_camera(&gfx, pico8_fixed_flr_int(mouse.x) - 64, pico8_fixed_flr_int(mouse.y) - 64);
    pico8_cls(&mem);
    cls_of(far)->ev(far, NULL);
    too_far = screen_hash();
    pico8_cls(&mem);
    cls_of(twin)->ev(twin, NULL);
    sprite = screen_hash();
    pico8_cls(&mem);
    cls_of(near)->ev(near, NULL);
    costed = screen_hash();
    assert(too_far != blank && sprite != blank && costed != blank);
    assert(too_far != sprite && too_far != costed);
    h = fnv(h, too_far);
    h = fnv(h, sprite);
    h = fnv(h, costed);

    before = credits();
    nh = get(far, "nh");
    re = get_ref(far, "re");
    assert(list_count(origin, "oe") == 1 && list_count(target, "oe") == 1);
    (void)call(far, "ey", pico8_value_ref(target));
    assert(bool_state(far, "p_") == 2);
    assert(credits() == before);
    assert(list_count(origin, "oe") == 1 && list_count(target, "oe") == 1);
    assert(get_ref(far, "re") == re && re_kind(far) == RE_THROWAWAY);
    assert(pico8_value_equal(get(far, "nh"), nh));
    assert(et_kind(far) == ET_QS);
    h = fold_jc(h, far);
    h = fnv(h, (uint32_t)credits());
    check("too far", h, TOO_FAR_HASH);
}

static int same_vec(pico8_vec_t a, pico8_vec_t b) {
    return a.x == b.x && a.y == b.y;
}

/* Over nothing, each `hq` hangs a fresh `{ra,p}` on `re`: `ra` the very
 * `mp` it got, `p` its own `mp/128`, registered nowhere; a later `hq`, of
 * this `jc` or another, leaves an earlier one as it was. */
static void test_hq_mouse_point(void) {
    uint32_t h = 2166136261u;
    pico8_value_t m1;
    pico8_value_t m2;
    pico8_vec_t p1;
    void *origin;
    void *target;
    void *a;
    void *b;
    void *r1;
    void *r2;
    void *rb;

    world();
    origin = get_ref(the_slipway(), "qu");
    target = get_ref(the_slipway(), "re");
    a = build_jc(origin);
    m1 = vec_ref(-640, -640);
    hq_over(a, NULL, m1);
    r1 = get_ref(a, "re");
    assert(r1 != NULL && re_kind(a) == RE_THROWAWAY);
    assert(get_ref(r1, "ra") == m1.as.ref);
    p1 = vec_of(r1, "p");
    assert(same_vec(p1, pico8_vec_div(vec_of(r1, "ra"), pico8_fixed_from_int(128))));
    assert(get_ref(r1, "p") != m1.as.ref);
    assert(!listed(pico8_hv(), r1) && !listed(pico8_cg("jc"), r1) && !listed(pico8_cg("np"), r1));
    assert(!has(r1, "mt") && !has(r1, "id"));
    h = fnv(h, (uint32_t)p1.x);
    h = fnv(h, (uint32_t)p1.y);
    h = fold_jc(h, a);

    m2 = vec_ref(40, -24);
    hq_over(a, NULL, m2);
    r2 = get_ref(a, "re");
    assert(r2 != NULL && r2 != r1 && re_kind(a) == RE_THROWAWAY);
    assert(get_ref(r2, "ra") == m2.as.ref && get_ref(r1, "ra") == m1.as.ref);
    assert(same_vec(vec_of(r1, "p"), p1));
    assert(!same_vec(vec_of(r2, "p"), p1));
    h = fold_jc(h, a);

    b = build_jc(origin);
    hq_over(b, NULL, m1);
    rb = get_ref(b, "re");
    assert(rb != NULL && rb != r1 && rb != r2 && get_ref(a, "re") == r2);
    assert(get_ref(rb, "ra") == m1.as.ref && same_vec(vec_of(rb, "p"), p1));
    h = fold_jc(h, b);

    hq_over(a, target, vec_ref_fixed(vec_of(target, "ra")));
    assert(get_ref(a, "re") == target && re_kind(a) == RE_PLANET);
    assert(get_ref(r1, "ra") == m1.as.ref && same_vec(vec_of(r1, "p"), p1));
    assert(get_ref(r2, "ra") == m2.as.ref && get_ref(rb, "ra") == m1.as.ref);
    h = fold_jc(h, a);
    check("mouse point", h, MOUSE_POINT_HASH);
}

/* A probe over the slipway targets it and hangs `fo` on it. Its `dw` is
 * zeroed, as `mz`'s is: every point of this short slipway lies within a
 * `g_`'s `dw` of one of its own planets, which would win. */
static void test_bv(void) {
    uint32_t h = 2166136261u;
    void *old;
    void *probe;
    pico8_vec_t qu;
    pico8_vec_t re;
    pico8_value_t out;
    int k;

    world();
    old = the_slipway();
    probe = pico8_g__new(NULL);
    assert(probe != NULL);
    ((pico8_g__t *)probe)->dw = 0;
    qu = vec_of(get_ref(old, "qu"), "p");
    re = vec_of(get_ref(old, "re"), "p");
    for (k = 1; k < 8; k++) {
        pico8_vec_t at = pico8_vec_add(qu, pico8_vec_div(pico8_vec_mul(pico8_vec_sub(re, qu), pico8_fixed_from_int(k)),
                                                         pico8_fixed_from_int(8)));
        assert(pico8_mv(probe, "hq", 2, vec_ref_fixed(pico8_vec_mul(at, pico8_fixed_from_int(128))), vec_ref(64, 64),
                        &out));
        h = fold_field(h, probe, "et");
        if (get_ref(probe, "et") == old) {
            break;
        }
    }
    printf("[bv] on the slipway at %d/8\n", k);
    assert(get_ref(probe, "et") == old);
    assert(get_ref(old, "fo") == probe);
    assert(holder_kind(old) == FO_PROBE);
    h = fold_jc(h, old);
    h = fold_field(h, probe, "et");
    check("bv", h, BV_HASH);
}

static int is_a_jc_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_jc_struct_class();
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

static void hg_add_fixed(void *hg) {
    pico8_jc_hg_add(hg, pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void hg_add_one_more(void *hg) {
    pico8_jc_hg_add(hg, pico8_value_string("one more"));
}

/* `hg[id]=true` keeps each string `id` once and in order, reads anything
 * else as absent, refuses a non-string `id` and a 91st planet. */
static void test_hg_is_a_set_of_ids(void) {
    static char ids[PICO8_JC_HG_CAP][8];
    pico8_jc_hg_t hg = {0, {NULL}};
    int i;

    pico8_jc_hg_add(&hg, pico8_value_string("p1"));
    pico8_jc_hg_add(&hg, pico8_value_string("p2"));
    pico8_jc_hg_add(&hg, pico8_value_string("p1"));
    assert(hg.count == 2 && strcmp(hg.ids[0], "p1") == 0 && strcmp(hg.ids[1], "p2") == 0);
    assert(pico8_jc_hg_has(&hg, pico8_value_string("p2")) && !pico8_jc_hg_has(&hg, pico8_value_string("p3")));
    assert(!pico8_jc_hg_has(&hg, pico8_value_fixed(pico8_fixed_from_int(1))));
    assert(!pico8_jc_hg_has(&hg, pico8_value_ref(NULL)) && !pico8_jc_hg_has(NULL, pico8_value_string("p1")));
    must_abort(hg_add_fixed, &hg);

    hg.count = 0;
    for (i = 0; i < PICO8_JC_HG_CAP; i++) {
        snprintf(ids[i], sizeof ids[i], "p%d", i + 1);
        pico8_jc_hg_add(&hg, pico8_value_string(ids[i]));
    }
    assert(hg.count == PICO8_JC_HG_CAP && pico8_jc_hg_has(&hg, pico8_value_string("p90")));
    must_abort(hg_add_one_more, &hg);
}

static void write_re(void *jc) {
    pico8_entity_set_field(jc, "re", pico8_value_ref(NULL));
}

static void write_nh(void *jc) {
    pico8_entity_set_field(jc, "nh", pico8_value_fixed(0));
}

static void write_fa(void *jc) {
    pico8_entity_set_field(jc, "fa", pico8_value_fixed(0));
}

/* Built by `jo:ldown`/`nr:ldown` or directly, a `jc`
 * is a struct with its five methods; a generic write of a name other than
 * `fo`/`p_` stops the run instead of growing a member. */
static void test_jc_is_a_struct(void) {
    static const char *const methods[] = {"hq", "ey", "eb"};
    void *old;
    void *jc;
    void *others[3];
    pico8_jc_keys_t keys = {0};
    void *direct;
    size_t i;

    world();
    old = the_slipway();
    jc = build_jc(get_ref(old, "qu"));
    keys.qu = get_ref(old, "qu");
    direct = pico8_jc_new(&keys);
    assert(get_ref(direct, "qu") == get_ref(old, "qu"));
    assert(is_a_jc_struct(old) && is_a_jc_struct(jc) && is_a_jc_struct(direct));
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        pico8_value_t m;
        assert(pico8_entity_get_field(jc, methods[i], &m) && m.kind == PICO8_VALUE_FUNCTION);
    }
    assert(cls_of(jc)->ev != NULL && cls_of(jc)->kj != NULL);
    assert(cls_of(jc)->states[PICO8_STATE_KZ] == NULL && cls_of(jc)->states[PICO8_STATE_PH] == NULL &&
           cls_of(jc)->ldown == NULL && cls_of(jc)->rdown == NULL && cls_of(jc)->gv == NULL);

    pico8_entity_set_field(jc, "fo", pico8_value_ref(old));
    assert(get_ref(jc, "fo") == old);
    pico8_entity_set_field(jc, "p_", pico8_value_bool(1));
    assert(bool_state(jc, "p_") == 2);
    must_abort(write_re, jc);
    must_abort(write_nh, jc);
    must_abort(write_fa, jc);

    others[0] = pico8_bg_new();
    others[1] = pico8_fq_new(NULL);
    others[2] = pico8_mz_new();
    for (i = 0; i < 3; i++) {
        assert(!has(others[i], "eb"));
    }
}

static void write_qs(void *e) {
    pico8_entity_set_field(e, "qs", pico8_value_string("-too far-"));
}

static void write_ra(void *e) {
    pico8_entity_set_field(e, "ra", pico8_value_ref(NULL));
}

/* Beyond range, every `jc` gets the same struct: `qs` fixed, `fo` the last
 * `hq` to write it, no method, and a write of another name stops the run. */
static void test_too_far_is_a_struct(void) {
    static const char *const absent[] = {"ra", "p", "me", "nu", "t", "hq"};
    void *origin;
    void *far;
    void *other;
    void *et;
    size_t i;

    world();
    origin = get_ref(the_slipway(), "qu");
    far = build_jc(origin);
    hq_over(far, NULL, vec_ref(-640, -640));
    et = get_ref(far, "et");
    assert(is_too_far(et) && get_ref(et, "fo") == far);
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(et, absent[i]));
    }
    {
        pico8_value_t out;
        assert(!pico8_mv(et, "hq", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    }
    assert(cls_of(et)->ev == NULL && cls_of(et)->kj == NULL);

    other = build_jc(origin);
    hq_over(other, NULL, vec_ref(-640, -640));
    assert(get_ref(other, "et") == et && get_ref(et, "fo") == other);

    must_abort(write_qs, et);
    must_abort(write_ra, et);
    pico8_entity_set_field(et, "fo", pico8_value_ref(NULL));
    assert(!has(et, "fo"));
    assert(is_string(et, "qs", "-too far-"));
}

static void write_p(void *e) {
    pico8_entity_set_field(e, "p", pico8_value_ref(NULL));
}

static void write_fo(void *e) {
    pico8_entity_set_field(e, "fo", pico8_value_ref(NULL));
}

/* Over nothing, `re` is a `{ra,p}` struct: those two members only, no
 * method, no generic write; a nil `mp` leaves `ra` absent and `p` zero. */
static void test_mouse_point_is_a_struct(void) {
    static const char *const absent[] = {"mt", "id", "qs", "fo", "me", "nu", "t", "kf", "eb", "hq"};
    static const char *const not_methods[] = {"kf", "hq", "eb"};
    void *origin;
    void *jc;
    void *re;
    pico8_vec_t p;
    size_t i;

    world();
    origin = get_ref(the_slipway(), "qu");
    jc = build_jc(origin);
    hq_over(jc, NULL, vec_ref(-640, -640));
    re = get_ref(jc, "re");
    assert(re != NULL && pico8_type_tag_of(re) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)re)->cls == pico8_jc_mouse_point_struct_class());
    assert(has(re, "ra") && has(re, "p"));
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(re, absent[i]));
    }
    for (i = 0; i < sizeof not_methods / sizeof not_methods[0]; i++) {
        pico8_value_t out;
        assert(!pico8_mv(re, not_methods[i], 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    }
    must_abort(write_ra, re);
    must_abort(write_p, re);
    must_abort(write_fo, re);

    hq_over(jc, NULL, pico8_value_ref(NULL));
    re = get_ref(jc, "re");
    assert(is_throwaway(re) && !has(re, "ra"));
    p = vec_of(re, "p");
    assert(p.x == 0 && p.y == 0);
}

/* Reads `ra` and `p` through the raw reader and the entity reader, which must
 * agree; a miss leaves the witness as it was. */
static int read_member(const void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(0x1234);
    pico8_value_t via = pico8_value_fixed(0x1234);
    int got_raw = pico8_jc_mouse_point_struct_class()->field(e, name, &raw);
    int got_via = pico8_entity_get_field(e, name, &via);
    assert(got_raw == got_via && raw.kind == via.kind);
    if (!got_raw) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
    } else {
        assert(raw.kind != PICO8_VALUE_REF || raw.as.ref == via.as.ref);
    }
    *out = raw;
    return got_raw;
}

static uint32_t fold_member(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    int got = read_member(e, name, &v);
    h = fnv(h, (uint32_t)got);
    h = fnv(h, (uint32_t)v.kind);
    if (got && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        const pico8_vec_t *p = v.as.ref;
        h = fnv(h, (uint32_t)p->x);
        h = fnv(h, (uint32_t)p->y);
    }
    return h;
}

static void test_mouse_point_members_by_name(void) {
    static const char *const intruders[] = {"", "x", "mt", "id", "re", "fo", "header", "has_ra"};
    uint32_t h = 2166136261u;
    pico8_value_t m1;
    pico8_value_t m2;
    pico8_value_t v;
    pico8_vec_t p1;
    void *origin;
    void *a;
    void *b;
    void *r1;
    void *r2;
    void *rb;
    void *p1_at;
    size_t i;

    world();
    origin = get_ref(the_slipway(), "qu");
    a = build_jc(origin);
    m1 = vec_ref(-640, -640);
    hq_over(a, NULL, m1);
    r1 = get_ref(a, "re");
    assert(is_throwaway(r1));
    assert(read_member(r1, "ra", &v) == 1 && v.kind == PICO8_VALUE_REF && v.as.ref == m1.as.ref);
    assert(read_member(r1, "p", &v) == 1 && v.kind == PICO8_VALUE_REF && v.as.ref != NULL && v.as.ref != m1.as.ref);
    p1_at = v.as.ref;
    p1 = *(const pico8_vec_t *)p1_at;
    assert(same_vec(p1, pico8_vec_div(*(const pico8_vec_t *)m1.as.ref, pico8_fixed_from_int(128))));
    assert(read_member(r1, "p", &v) == 1 && v.as.ref == p1_at);
    h = fold_member(h, r1, "ra");
    h = fold_member(h, r1, "p");
    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        assert(read_member(r1, intruders[i], &v) == 0);
        h = fold_member(h, r1, intruders[i]);
    }

    hq_over(a, NULL, pico8_value_ref(NULL));
    r2 = get_ref(a, "re");
    assert(is_throwaway(r2) && r2 != r1);
    assert(read_member(r2, "ra", &v) == 0);
    assert(read_member(r2, "p", &v) == 1 && v.kind == PICO8_VALUE_REF && v.as.ref != NULL && v.as.ref != p1_at);
    assert(((const pico8_vec_t *)v.as.ref)->x == 0 && ((const pico8_vec_t *)v.as.ref)->y == 0);
    h = fold_member(h, r2, "ra");
    h = fold_member(h, r2, "p");

    m2 = vec_ref(40, -24);
    hq_over(a, NULL, m2);
    b = build_jc(origin);
    hq_over(b, NULL, m1);
    rb = get_ref(b, "re");
    assert(is_throwaway(rb) && rb != r1 && rb != r2 && rb != get_ref(a, "re"));
    assert(read_member(get_ref(a, "re"), "ra", &v) == 1 && v.as.ref == m2.as.ref);
    assert(read_member(get_ref(a, "re"), "p", &v) == 1 && v.as.ref != p1_at);
    h = fold_member(h, get_ref(a, "re"), "ra");
    h = fold_member(h, get_ref(a, "re"), "p");
    assert(read_member(rb, "ra", &v) == 1 && v.as.ref == m1.as.ref);
    assert(read_member(rb, "p", &v) == 1 && v.as.ref != p1_at && same_vec(*(const pico8_vec_t *)v.as.ref, p1));
    h = fold_member(h, rb, "ra");
    h = fold_member(h, rb, "p");
    assert(read_member(r1, "ra", &v) == 1 && v.as.ref == m1.as.ref);
    assert(read_member(r1, "p", &v) == 1 && v.as.ref == p1_at && same_vec(*(const pico8_vec_t *)p1_at, p1));
    h = fold_member(h, r1, "ra");
    h = fold_member(h, r1, "p");
    check("mouse point members", h, MEMBERS_HASH);
}

/* Reads `qs` and `fo` through the raw reader and the entity reader, which must
 * agree; a miss leaves the witness as it was. */
static int read_far(const void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(0x1234);
    pico8_value_t via = pico8_value_fixed(0x1234);
    int got_raw = pico8_jc_too_far_struct_class()->field(e, name, &raw);
    int got_via = pico8_entity_get_field(e, name, &via);
    assert(got_raw == got_via && raw.kind == via.kind);
    if (!got_raw) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
    } else if (raw.kind == PICO8_VALUE_REF) {
        assert(raw.as.ref == via.as.ref);
    } else if (raw.kind == PICO8_VALUE_STRING) {
        assert(raw.as.string == via.as.string);
    }
    *out = raw;
    return got_raw;
}

static uint32_t fold_far(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    int got = read_far(e, name, &v);
    return fold_value(fnv(h, (uint32_t)got), v);
}

static void test_too_far_members_by_name(void) {
    static const char *const intruders[] = {"", "x", "ra", "p", "et", "header", "has_fo"};
    static const char *const unwritten[] = {"qs", "ra", "", "fo "};
    const pico8_struct_class_t *cls = pico8_jc_too_far_struct_class();
    uint32_t h = 2166136261u;
    const char *qs_at;
    pico8_value_t v;
    void *origin;
    void *a;
    void *b;
    void *c;
    void *et;
    size_t i;

    world();
    origin = get_ref(the_slipway(), "qu");
    a = build_jc(origin);
    hq_over(a, NULL, vec_ref(-640, -640));
    et = get_ref(a, "et");
    assert(is_too_far(et));
    assert(read_far(et, "qs", &v) == 1 && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, "-too far-") == 0);
    qs_at = v.as.string;
    assert(read_far(et, "qs", &v) == 1 && v.as.string == qs_at);
    assert(read_far(et, "fo", &v) == 1 && v.kind == PICO8_VALUE_REF && v.as.ref == a);
    h = fold_far(h, et, "qs");
    h = fold_far(h, et, "fo");
    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        assert(read_far(et, intruders[i], &v) == 0);
        h = fold_far(h, et, intruders[i]);
    }

    b = build_jc(origin);
    pico8_entity_set_field(et, "fo", pico8_value_ref(b));
    assert(read_far(et, "fo", &v) == 1 && v.kind == PICO8_VALUE_REF && v.as.ref == b);
    h = fold_far(h, et, "fo");
    assert(cls->set_field(et, "fo", pico8_value_ref(a)) == 1);
    assert(read_far(et, "fo", &v) == 1 && v.kind == PICO8_VALUE_REF && v.as.ref == a);
    h = fold_far(h, et, "fo");
    pico8_entity_set_field(et, "fo", pico8_value_ref(NULL));
    assert(read_far(et, "fo", &v) == 0);
    h = fold_far(h, et, "fo");
    assert(read_far(et, "qs", &v) == 1 && v.as.string == qs_at);
    pico8_entity_set_field(et, "fo", pico8_value_ref(b));
    assert(read_far(et, "fo", &v) == 1 && v.as.ref == b);
    h = fold_far(h, et, "fo");

    for (i = 0; i < sizeof unwritten / sizeof unwritten[0]; i++) {
        assert(cls->set_field(et, unwritten[i], pico8_value_fixed(pico8_fixed_from_int(1))) == 0);
    }
    assert(read_far(et, "qs", &v) == 1 && v.as.string == qs_at);
    assert(read_far(et, "fo", &v) == 1 && v.as.ref == b);
    h = fold_far(h, et, "qs");
    h = fold_far(h, et, "fo");

    c = build_jc(origin);
    hq_over(c, NULL, vec_ref(-640, -640));
    assert(get_ref(c, "et") == et);
    assert(read_far(et, "fo", &v) == 1 && v.as.ref == c);
    h = fold_far(h, et, "fo");
    check("too far members", h, FAR_MEMBERS_HASH);
}

static void *g_planet;

static void write_fo_fixed(void *e) {
    pico8_entity_set_field(e, "fo", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void write_fo_false(void *e) {
    pico8_entity_set_field(e, "fo", pico8_value_bool(0));
}

/* A false whose whole union is zero: its `as.ref` reads as NULL, so a
 * nil test that forgets to check the kind would take it for nil. */
static void write_fo_zero_false(void *e) {
    pico8_value_t v;

    memset(&v, 0, sizeof v);
    v.kind = PICO8_VALUE_BOOL;
    pico8_entity_set_field(e, "fo", v);
}

static void write_fo_string(void *e) {
    pico8_entity_set_field(e, "fo", pico8_value_string("x"));
}

static void write_fo_planet(void *e) {
    pico8_entity_set_field(e, "fo", pico8_value_ref(g_planet));
}

static void write_fo_vec(void *e) {
    pico8_entity_set_field(e, "fo", vec_ref(1, 1));
}

static void test_too_far_writer_stops_off_row(void) {
    void *a;
    void *et;

    world();
    g_planet = get_ref(the_slipway(), "qu");
    a = build_jc(g_planet);
    hq_over(a, NULL, vec_ref(-640, -640));
    et = get_ref(a, "et");
    assert(is_too_far(et));
    must_abort(write_fo_fixed, et);
    must_abort(write_fo_false, et);
    must_abort(write_fo_zero_false, et);
    must_abort(write_fo_string, et);
    must_abort(write_fo_planet, et);
    must_abort(write_fo_vec, et);
}

static const char *const jc_names[16] = {"nu", "fa", "fp", "ik", "dw", "k_", "nh", "qu",
                                         "ts", "hg", "re", "od", "mt", "et", "fo", "p_"};

static int is_of_class(const void *e, const pico8_struct_class_t *cls) {
    return e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)e)->cls == cls;
}

static pico8_value_t zero_union(pico8_value_kind_t kind) {
    pico8_value_t v;

    memset(&v, 0, sizeof v);
    v.kind = kind;
    return v;
}

static int read_jc(const void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(0x1234);
    pico8_value_t via = pico8_value_fixed(0x1234);
    int got_raw = pico8_jc_struct_class()->field(e, name, &raw);
    int got_via = pico8_entity_get_field(e, name, &via);
    assert(got_raw == got_via && raw.kind == via.kind);
    if (!got_raw) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        assert(via.kind == PICO8_VALUE_FIXED && via.as.fixed == 0x1234);
    } else if (raw.kind == PICO8_VALUE_REF) {
        assert(raw.as.ref != NULL && raw.as.ref == via.as.ref);
    } else if (raw.kind == PICO8_VALUE_FIXED) {
        assert(raw.as.fixed == via.as.fixed);
    } else if (raw.kind == PICO8_VALUE_BOOL) {
        assert(raw.as.boolean == via.as.boolean);
    }
    *out = raw;
    return got_raw;
}

static void *jc_ref(const void *jc, const char *name) {
    pico8_value_t v;
    if (!read_jc(jc, name, &v)) {
        return NULL;
    }
    assert(v.kind == PICO8_VALUE_REF);
    return v.as.ref;
}

static uint32_t fold_jc_member(uint32_t h, const void *jc, const char *name) {
    pico8_value_t v;
    int got = read_jc(jc, name, &v);
    h = fnv(h, (uint32_t)got);
    h = fnv(h, (uint32_t)v.kind);
    switch (v.kind) {
    case PICO8_VALUE_FIXED:
        return fnv(h, (uint32_t)v.as.fixed);
    case PICO8_VALUE_BOOL:
        return fnv(h, (uint32_t)v.as.boolean);
    case PICO8_VALUE_REF:
        return fnv(h, (uint32_t)rank_in(pico8_hv(), v.as.ref));
    default:
        return fnv(h, 0xF0u);
    }
}

static uint32_t fold_jc_members(uint32_t h, const void *jc) {
    size_t i;
    for (i = 0; i < 16; i++) {
        h = fold_jc_member(h, jc, jc_names[i]);
    }
    return h;
}

static void check_fresh_members(const void *jc, const void *qu, const void *like) {
    static const char *const unset[] = {"re", "od", "mt", "et", "fo", "p_"};
    pico8_value_t v;
    size_t i;

    assert(jc_ref(jc, "nu") != NULL && jc_ref(jc, "nu") == jc_ref(like, "nu"));
    assert(jc_ref(jc, "k_") != NULL && jc_ref(jc, "k_") == jc_ref(like, "k_"));
    assert(jc_ref(jc, "ts") != NULL && jc_ref(jc, "hg") != NULL);
    assert(jc == like || (jc_ref(jc, "ts") != jc_ref(like, "ts") && jc_ref(jc, "hg") != jc_ref(like, "hg")));
    assert(read_jc(jc, "fp", &v) && v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
    assert(read_jc(jc, "fa", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(2));
    assert(read_jc(jc, "ik", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(48));
    assert(read_jc(jc, "dw", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == (pico8_fixed_t)DW_BITS);
    assert(read_jc(jc, "nh", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(20));
    assert(jc_ref(jc, "qu") == qu);
    for (i = 0; i < sizeof unset / sizeof unset[0]; i++) {
        assert(!read_jc(jc, unset[i], &v));
    }
}

static void *bare_jo(pico8_vec_t p) {
    pico8_jo_keys_t keys;

    keys.id = NULL;
    keys.p = vec_ref_fixed(p).as.ref;
    keys.ra = NULL;
    return pico8_jo_new(&keys);
}

static pico8_vec_t units(int x, int y) {
    return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
}

static pico8_vec_t shifted(pico8_vec_t p, pico8_fixed_t dx, pico8_fixed_t dy) {
    return pico8_vec_add(p, pico8_vec_v(dx, dy));
}

static pico8_value_t mouse_at(pico8_vec_t at) {
    return vec_ref_fixed(pico8_vec_add(pico8_vec_mul(at, pico8_fixed_from_int(128)), units(1, 1)));
}

static void over(void *jc, void *re) {
    hq_over(jc, re, mouse_at(vec_of(re, "p")));
}

static void *lone_jc(void *qu, void **re_out) {
    void *re = bare_jo(shifted(vec_of(qu, "p"), 0x8000, 0));
    void *jc = build_jc(qu);

    over(jc, re);
    assert(jc_ref(jc, "re") == re && jc_ref(jc, "et") == NULL);
    if (re_out != NULL) {
        *re_out = re;
    }
    return jc;
}

static pico8_value_t call2(void *e, const char *name, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, 2, arg0, arg1, &out));
    return out;
}

static void test_jc_members_by_name(void) {
    static const char *const unwritten[] = {"", "FO", "fo ", "p", "has_p_", "has_od", "has_mt"};
    static const char *const intruders[] = {"", "q", "qu ", "QU", "has_od", "has_mt", "has_p_", "header", "ke", "nu_"};
    const pico8_struct_class_t *cls = pico8_jc_struct_class();
    const pico8_struct_class_t *want_cls[9];
    uint32_t h = 2166136261u;
    uint32_t before;
    pico8_jc_keys_t keys = {0};
    pico8_value_t v;
    void *fresh[4];
    void *ly;
    void *qu_of[9];
    void *holders[12];
    void *jc;
    void *re;
    void *far;
    void *fr;
    void *mid;
    void *cross;
    size_t i;
    int k;

    world();
    ly = first_with_me("jo", "ly");
    assert(ly != NULL);
    fresh[0] = pico8_jc_new(NULL);
    fresh[1] = pico8_jc_new(&keys);
    fresh[2] = build_jc(ly);
    fresh[3] = cls_of(ly)->ldown(ly, NULL, NULL).held;
    for (k = 0; k < 4; k++) {
        assert(is_a_jc_struct(fresh[k]));
        check_fresh_members(fresh[k], k < 2 ? NULL : ly, fresh[k == 0 ? 1 : 0]);
        h = fold_jc_members(h, fresh[k]);
    }

    qu_of[0] = ly;
    qu_of[1] = pico8_qh_new(NULL);
    qu_of[2] = pico8_nl_new(NULL);
    qu_of[3] = pico8_os_new(NULL);
    qu_of[4] = pico8_hj_new(NULL);
    qu_of[5] = pico8_pm_new(NULL);
    qu_of[6] = pico8_mx_new(NULL);
    qu_of[7] = pico8_nr_new(NULL);
    qu_of[8] = pico8_fr_new(NULL);
    want_cls[0] = pico8_jo_struct_class();
    want_cls[1] = pico8_qh_struct_class();
    want_cls[2] = pico8_nl_struct_class();
    want_cls[3] = pico8_os_struct_class();
    want_cls[4] = pico8_hj_struct_class();
    want_cls[5] = pico8_pm_struct_class();
    want_cls[6] = pico8_mx_struct_class();
    want_cls[7] = pico8_nr_struct_class();
    want_cls[8] = pico8_fr_struct_class();
    assert(is_string(qu_of[5], "me", "ly"));
    for (k = 1; k < 7; k++) {
        if (k != 5) {
            pico8_ke_ks(qu_of[k], pico8_value_string("ly"));
        }
    }
    for (k = 0; k < 9; k++) {
        void *made;
        assert(is_of_class(qu_of[k], want_cls[k]));
        made = cls_of(qu_of[k])->ldown(qu_of[k], NULL, NULL).held;
        assert(is_a_jc_struct(made));
        check_fresh_members(made, qu_of[k], fresh[0]);
        h = fold_jc_members(h, made);
        made = build_jc(qu_of[k]);
        check_fresh_members(made, qu_of[k], fresh[0]);
        h = fold_jc_members(h, made);
    }
    keys.qu = NULL;
    jc = pico8_jc_new(&keys);
    check_fresh_members(jc, NULL, fresh[0]);
    h = fold_jc_members(h, jc);

    jc = lone_jc(bare_jo(units(-40, -40)), &re);
    assert(read_jc(jc, "od", &v) && v.kind == PICO8_VALUE_FIXED);
    assert(!read_jc(jc, "mt", &v));
    assert(read_jc(jc, "fa", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(4));
    h = fold_jc_members(h, jc);
    (void)call(jc, "ey", pico8_value_ref(NULL));
    assert(read_jc(jc, "p_", &v) && v.kind == PICO8_VALUE_BOOL && v.as.boolean == 1);
    assert(jc_ref(jc, "re") == re);
    h = fold_jc_members(h, jc);

    far = build_jc(jc_ref(jc, "qu"));
    hq_over(far, NULL, vec_ref(-640, -640));
    assert(is_throwaway(jc_ref(far, "re")) && is_too_far(jc_ref(far, "et")));
    assert(get_ref(jc_ref(far, "et"), "fo") == far);
    h = fold_jc_members(h, far);

    fr = pico8_fr_new(NULL);
    (void)call2(fr, "hq", mouse_at(units(-36, -40)), vec_ref(2, 2));
    far = lone_jc(fr, NULL);
    assert(read_jc(far, "mt", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(1));
    assert(read_jc(far, "fa", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(2));
    h = fold_jc_members(h, far);

    far = build_jc(bare_jo(units(-32, -40)));
    re = bare_jo(shifted(units(-32, -40), 0x8000, 0));
    mid = bare_jo(shifted(units(-32, -40), 0x4000, 0));
    over(far, re);
    assert(jc_ref(far, "et") == mid && get_ref(mid, "fo") == far);
    h = fold_jc_members(h, far);

    far = lone_jc(bare_jo(units(-28, -40)), NULL);
    cross = build_jc(bare_jo(shifted(units(-28, -40), 0x4000, -0x4000)));
    over(cross, bare_jo(shifted(units(-28, -40), 0x4000, 0x4000)));
    assert(jc_ref(cross, "et") == far && jc_ref(far, "fo") == cross);
    h = fold_jc_members(h, cross);
    h = fold_jc_members(h, far);

    holders[0] = pico8_g__new(NULL);
    holders[1] = pico8_fv_new();
    holders[2] = pico8_mz_new();
    holders[3] = pico8_qh_new(NULL);
    holders[4] = pico8_nl_new(NULL);
    holders[5] = pico8_os_new(NULL);
    holders[6] = pico8_hj_new(NULL);
    holders[7] = pico8_pm_new(NULL);
    holders[8] = pico8_nr_new(NULL);
    holders[9] = pico8_fr_new(NULL);
    holders[10] = pico8_mx_new(NULL);
    holders[11] = cross;
    for (k = 0; k < 11; k++) {
        pico8_vec_t base = units(-40 + 4 * k, -32);
        jc = lone_jc(bare_jo(base), NULL);
        (void)call2(holders[k], "hq", mouse_at(shifted(base, 0x4000, 0)), vec_ref(2, 2));
        assert(jc_ref(jc, "fo") == holders[k]);
        h = fold_jc_members(h, jc);
    }
    for (k = 11; k >= 0; k--) {
        pico8_entity_set_field(jc, "fo", pico8_value_ref(holders[k]));
        assert(jc_ref(jc, "fo") == holders[k]);
        h = fold_jc_member(h, jc, "fo");
        assert(cls->set_field(jc, "fo", pico8_value_ref(holders[(k + 1) % 12])) == 1);
        assert(jc_ref(jc, "fo") == holders[(k + 1) % 12]);
    }
    pico8_entity_set_field(jc, "fo", pico8_value_ref(NULL));
    assert(!read_jc(jc, "fo", &v));
    pico8_entity_set_field(jc, "fo", pico8_value_ref(holders[0]));
    pico8_entity_set_field(jc, "fo", zero_union(PICO8_VALUE_REF));
    assert(!read_jc(jc, "fo", &v));
    pico8_entity_set_field(jc, "fo", pico8_value_ref(holders[2]));
    cls_of(jc)->kj(jc, NULL);
    assert(!read_jc(jc, "fo", &v));
    h = fold_jc_members(h, jc);

    jc = lone_jc(bare_jo(units(-40, -24)), NULL);
    pico8_entity_set_field(jc, "p_", pico8_value_bool(1));
    assert(read_jc(jc, "p_", &v) && v.kind == PICO8_VALUE_BOOL && v.as.boolean == 1);
    h = fold_jc_member(h, jc, "p_");
    pico8_entity_set_field(jc, "p_", pico8_value_ref(NULL));
    assert(!read_jc(jc, "p_", &v));
    pico8_entity_set_field(jc, "p_", pico8_value_bool(0));
    assert(read_jc(jc, "p_", &v) && v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
    h = fold_jc_member(h, jc, "p_");
    pico8_entity_set_field(jc, "p_", pico8_value_ref(NULL));
    assert(!read_jc(jc, "p_", &v));
    pico8_entity_set_field(jc, "p_", zero_union(PICO8_VALUE_BOOL));
    assert(read_jc(jc, "p_", &v) && v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
    h = fold_jc_member(h, jc, "p_");
    pico8_entity_set_field(jc, "p_", pico8_value_ref(NULL));
    pico8_entity_set_field(jc, "p_", pico8_value_ref(NULL));
    assert(!read_jc(jc, "p_", &v));
    h = fold_jc_member(h, jc, "p_");

    pico8_entity_set_field(jc, "fo", pico8_value_ref(holders[3]));
    pico8_entity_set_field(jc, "p_", pico8_value_bool(1));
    before = fold_jc_members(2166136261u, jc);
    for (i = 0; i < 16; i++) {
        if (strcmp(jc_names[i], "fo") != 0 && strcmp(jc_names[i], "p_") != 0) {
            assert(cls->set_field(jc, jc_names[i], pico8_value_fixed(pico8_fixed_from_int(9))) == 0);
        }
    }
    for (i = 0; i < sizeof unwritten / sizeof unwritten[0]; i++) {
        assert(cls->set_field(jc, unwritten[i], pico8_value_fixed(pico8_fixed_from_int(9))) == 0);
    }
    assert(fold_jc_members(2166136261u, jc) == before);
    h = fnv(h, before);
    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        pico8_value_t raw = pico8_value_fixed(0x1234);
        assert(cls->field(jc, intruders[i], &raw) == 0);
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        h = fold_jc_member(h, jc, intruders[i]);
    }
    check("jc members", h, JC_MEMBERS_HASH);
}

static pico8_value_t g_off_row;
static const char *g_off_name;

static void write_off_row(void *e) {
    pico8_entity_set_field(e, g_off_name, g_off_row);
}

static void build_off_row(void *e) {
    pico8_jc_keys_t keys = {0};

    (void)e;
    keys.qu = g_off_row.as.ref;
    (void)pico8_jc_new(&keys);
}

static void stops_on(void (*write)(void *), void *e, const char *name, const pico8_value_t *off, size_t n) {
    size_t i;

    g_off_name = name;
    for (i = 0; i < n; i++) {
        g_off_row = off[i];
        must_abort(write, e);
    }
}

static void test_jc_writer_stops_off_row(void) {
    pico8_value_t vec;
    pico8_value_t list;
    pico8_value_t jo;
    pico8_value_t ga;
    pico8_value_t other;
    pico8_value_t qs;
    pico8_value_t point;
    void *old;
    void *far;

    world();
    old = the_slipway();
    vec = vec_ref(1, 1);
    list = pico8_value_ref((void *)pico8_hv());
    jo = get(old, "qu");
    ga = pico8_value_ref(pico8_ga_new(NULL));
    far = build_jc(jo.as.ref);
    hq_over(far, NULL, vec_ref(-640, -640));
    other = pico8_value_ref(far);
    qs = get(far, "et");
    point = get(far, "re");
    assert(is_too_far(qs.as.ref) && is_throwaway(point.as.ref));
    {
        const pico8_value_t off[] = {pico8_value_fixed(pico8_fixed_from_int(1)),
                                     zero_union(PICO8_VALUE_FIXED),
                                     pico8_value_bool(1),
                                     zero_union(PICO8_VALUE_BOOL),
                                     pico8_value_string("x"),
                                     zero_union(PICO8_VALUE_STRING),
                                     zero_union(PICO8_VALUE_FUNCTION),
                                     list,
                                     vec,
                                     jo,
                                     ga,
                                     pico8_value_ref(pico8_bg_new()),
                                     qs,
                                     point};
        stops_on(write_off_row, old, "fo", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_fixed(pico8_fixed_from_int(1)),
                                     zero_union(PICO8_VALUE_FIXED),
                                     pico8_value_string("x"),
                                     zero_union(PICO8_VALUE_STRING),
                                     zero_union(PICO8_VALUE_FUNCTION),
                                     vec,
                                     other};
        stops_on(write_off_row, old, "p_", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {vec,
                                     ga,
                                     pico8_value_ref(pico8_g__new(NULL)),
                                     pico8_value_ref(pico8_mz_new()),
                                     other,
                                     point};
        stops_on(build_off_row, NULL, "qu", off, sizeof off / sizeof off[0]);
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
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_construction);
    in_fresh_child(test_ey);
    in_fresh_child(test_hq);
    in_fresh_child(test_eb);
    in_fresh_child(test_draw);
    in_fresh_child(test_bv);
    in_fresh_child(test_too_far);
    in_fresh_child(test_hq_mouse_point);
    in_fresh_child(test_jc_is_a_struct);
    in_fresh_child(test_hg_is_a_set_of_ids);
    in_fresh_child(test_too_far_is_a_struct);
    in_fresh_child(test_mouse_point_is_a_struct);
    in_fresh_child(test_mouse_point_members_by_name);
    in_fresh_child(test_too_far_members_by_name);
    in_fresh_child(test_too_far_writer_stops_off_row);
    in_fresh_child(test_jc_members_by_name);
    in_fresh_child(test_jc_writer_stops_off_row);

    printf("test_pico8_jc: all tests passed\n");
    return 0;
}
