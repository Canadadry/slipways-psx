/* The probe `g_`, read and written only through the dual-form field API,
 * its defaults compared to `pico8_g__start`/`pico8_fv_start`, built by
 * its three roads (`bg:ldown`, `pico8_g__new(NULL)`, `pico8_g__new` with the
 * 4 keys of probe_mm_rnd_order) and driven through the real `mv`/`dc`/`dg`.
 * Its methods are compared to the public functions. Its daughter `fv` is
 * read on the title screen. No events file. */

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
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_particle.h"
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


/* The tag class instances carried, which the frozen hashes fold: any
 * other value moves every *_HASH. */
#define FROZEN_INSTANCE_TAG 0x9E5C1451u

#define BUILT_HASH 0xD5E726A8u
#define LIFE_HASH 0x26179978u
#define EY_ON_HASH 0x7D81D210u
#define EY_OFF_HASH 0x196FE04Du
#define FV_HASH 0xA5CD3A2Du
#define BV_ET_MEMBERS_HASH 0x2E520916u
#define G__MEMBERS_HASH 0x545D7F21u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

/* g_'s 18 spec defaults, in spec order. */
static const char *const defaults[] = {"nu", "fx", "dq", "od", "fp", "fa", "dw", "py", "ki",
                                       "pn", "pk", "fc", "k_", "qo", "ro", "qg", "q_", "du"};
static const char *const scalars[] = {"fx", "dq", "od", "fp", "fa", "dw", "py", "ki", "pn",
                                      "pk", "fc", "qo", "ro", "qg", "q_", "du", "me", "t"};
static const char *const not_yet[] = {"ra", "p", "et", "p_", "fo", "m_", "ma", "ik"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};

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

static int list_count(const pico8_list_t *list) {
    return list != NULL ? list->count : 0;
}

/* The instance reader keeps a field written nil (TROU-9), the struct reader
 * reads a nil member as absent: nil and absent are one state here. */
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

static int is_string(const void *e, const char *name, const char *want) {
    pico8_value_t v;
    return lookup(e, name, &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, want) == 0;
}

static pico8_value_function_t method(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_FUNCTION);
    return v.as.function;
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

static int is_bv_target(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_bv_et_struct_class();
}

static uint32_t fold_value(uint32_t h, pico8_value_t v) {
    switch (v.kind) {
    case PICO8_VALUE_BOOL:
        return fnv(fnv(h, 0xB0u), (uint32_t)v.as.boolean);
    case PICO8_VALUE_FIXED:
        return fnv(fnv(h, 0xF1u), (uint32_t)v.as.fixed);
    case PICO8_VALUE_STRING:
        return fnv_str(fnv(h, 0x57u), v.as.string);
    case PICO8_VALUE_FUNCTION:
        return fnv(h, 0xFCu);
    case PICO8_VALUE_REF:
        if (v.as.ref == NULL) {
            return fnv(h, 0xDEADu);
        }
        if (listed(pico8_hv(), v.as.ref)) {
            return fnv(fnv(h, 0x4Eu), (uint32_t)rank_in(pico8_hv(), v.as.ref));
        }
        /* The frozen hashes folded the INSTANCE tag the throwaway had. */
        if (is_bv_target(v.as.ref)) {
            return fnv(fnv(h, 0xEEEEu), FROZEN_INSTANCE_TAG);
        }
        return fnv(fnv(h, 0xEEEEu), (uint32_t)pico8_type_tag_of(v.as.ref));
    default:
        return fnv(h, 0xF0u);
    }
}

static uint32_t fold_field(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v) ? fold_value(fnv(h, 1u), v) : fnv(h, 0u);
}

/* A tag sequence: each position, then its string, then the count. */
static uint32_t fold_tags(uint32_t h, const pico8_str_seq_t *s) {
    int i;

    for (i = 0; i < s->count; i++) {
        h = fnv(h, (uint32_t)(i + 1));
        h = fold_value(h, pico8_value_string(s->items[i]));
    }
    return fnv(h, (uint32_t)s->count);
}

/* An integer sequence: each position, then its value, then the count. */
static uint32_t fold_seq(uint32_t h, const pico8_int_seq_t *s) {
    int i;

    for (i = 0; i < s->count; i++) {
        h = fnv(h, (uint32_t)(i + 1));
        h = fold_value(h, pico8_value_fixed(pico8_fixed_from_int(s->items[i])));
    }
    return fnv(h, (uint32_t)s->count);
}

/* The item count of a list field, -1 when absent. */
static int size_of(const void *e, const char *name) {
    pico8_value_t v;
    if (!lookup(e, name, &v)) {
        return -1;
    }
    assert(v.kind == PICO8_VALUE_REF && pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST);
    return ((const pico8_list_t *)v.as.ref)->count;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
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

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
}

static pico8_value_t vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return pico8_value_ref(v);
}

static pico8_vec_t vec_xy(int x, int y) {
    return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
}

static pico8_vec_t vec_of(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return *(const pico8_vec_t *)v.as.ref;
}

static const pico8_struct_class_t *cls_of(const void *e) {
    return ((const pico8_struct_header_t *)e)->cls;
}

static pico8_value_t call(void *e, const char *name, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, argc, arg0, arg1, &out));
    return out;
}

static pico8_value_t nil(void) {
    return pico8_value_ref(NULL);
}

/* The background is the entity of cv.hd whose `hd` and `fa` are both the
 * plain value 1. */
static void *the_bg(void) {
    const pico8_list_t *hd = pico8_cv("hd");
    void *found = NULL;
    int count = 0;
    int i;

    for (i = 0; hd != NULL && i < hd->count; i++) {
        pico8_value_t v;
        void *e = hd->items[i].as.ref;

        if (pico8_entity_get_field(e, "hd", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(1) &&
            pico8_entity_get_field(e, "fa", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(1)) {
            found = e;
            count++;
        }
    }
    assert(count == 1);
    return found;
}

/* A probe, whatever its form: `g_`'s `nu` and `fa`=2 (an `fv` has 10). */
static int is_probe(const void *e) {
    pico8_value_t fa;
    return e != NULL && get_ref(e, "nu") == &pico8_g__nu_tags && lookup(e, "fa", &fa) &&
           fa.kind == PICO8_VALUE_FIXED && fa.as.fixed == pico8_fixed_from_int(2);
}

/* Road (a): `bg:ldown` while cg.g_ is empty. */
static void *ldown(void) {
    void *bg = the_bg();
    pico8_down_t out;

    assert(list_count(pico8_cg("g_")) == 0);
    out = cls_of(bg)->ldown(bg, vec_ref(vec_xy(1, 1)).as.ref, vec_ref(vec_xy(2, 2)).as.ref);
    assert(out.stop && out.held != NULL && is_probe(out.held));
    return out.held;
}

/* Road (c): the 4 keys of probe_mm_rnd_order. */
static void *keyed(void) {
    pico8_g__keys_t keys = {0};

    keys.has_pn = 1;
    keys.pn = pico8_fixed_from_int(7);
    keys.t = pico8_fixed_from_int(3);
    keys.has_du = 1;
    keys.du = pico8_fixed_from_int(10);
    keys.has_py = 1;
    keys.py = pico8_fixed_from_int(1);
    return pico8_g__new(&keys);
}

/* A probe as its road and `f_`, `lt` left it: its 18 defaults, `me`, `t`,
 * its tables' contents, its empty `qy`; none of the names only `bv`/`d_`
 * write; its 5 methods, the public functions; where `lt` registered it. */
static uint32_t fold_probe(uint32_t h, const void *e) {
    size_t i;

    assert(is_probe(e));
    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    for (i = 0; i < sizeof defaults / sizeof defaults[0]; i++) {
        assert(has(e, defaults[i]));
    }
    h = fold_tags(h, get_ref(e, "nu"));
    h = fold_seq(h, get_ref(e, "k_"));
    assert(get_ref(e, "nu") == &pico8_g__nu_tags);
    assert(get_ref(e, "k_") == &pico8_g__k__seq);
    h = fnv(h, (uint32_t)size_of(e, "qy"));
    assert(size_of(e, "qy") == 0);
    for (i = 0; i < sizeof not_yet / sizeof not_yet[0]; i++) {
        assert(!has(e, not_yet[i]));
    }
    assert(cls_of(e)->hd == NULL && cls_of(e)->hk == NULL && cls_of(e)->hb == NULL &&
           cls_of(e)->states[PICO8_STATE_LY] == NULL && cls_of(e)->states[PICO8_STATE_PH] == NULL);
    assert(method(e, "hq") == pico8_bv_dispatch);
    assert(method(e, "ey") == pico8_d__dispatch);
    assert(cls_of(e)->ev == pico8_dl_dispatch);
    assert(cls_of(e)->kj == pico8_ei_dispatch);
    assert(method(e, "mm") == pico8_g__mm);
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cv(bm[i]), e));
    }
    assert(listed(pico8_cv("kj"), e) && listed(pico8_cv("ev"), e));
    assert(!listed(pico8_cv("hd"), e) && !listed(pico8_cv("hk"), e) && !listed(pico8_cv("hb"), e));
    h = fnv(h, (uint32_t)rank_in(pico8_cg("g_"), e));
    h = fnv(h, (uint32_t)rank_in(pico8_cg("prober"), e));
    assert(listed(pico8_cg("g_"), e) && listed(pico8_cg("prober"), e));
    return h;
}

/* One probe by each road. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;
    void *c;

    start_game();
    a = ldown();
    h = fold_probe(h, a);
    b = pico8_g__new(NULL);
    h = fold_probe(h, b);
    c = keyed();
    h = fold_probe(h, c);
    assert(a != b && b != c && a != c);
    assert(get_ref(a, "qy") != get_ref(b, "qy") && get_ref(b, "qy") != get_ref(c, "qy"));
    assert(get(a, "fa").as.fixed == pico8_fixed_from_int(2) && get(a, "ki").as.fixed == pico8_fixed_from_int(9));
    assert(get(a, "dw").as.fixed == pico8_g__start.dw);
    assert(get(c, "pn").as.fixed == pico8_fixed_from_int(7) && get(c, "t").as.fixed == pico8_fixed_from_int(3) &&
           get(c, "du").as.fixed == pico8_fixed_from_int(10) && get(c, "py").as.fixed == pico8_fixed_from_int(1));
    assert(get(b, "pn").as.fixed == pico8_fixed_from_int(3) && get(b, "t").as.fixed == pico8_fixed_from_int(0));
    check("built", h, BUILT_HASH);
}

static uint32_t fold_particle(uint32_t h, pico8_value_t pt) {
    const pico8_qy_t *q;

    assert(pt.kind == PICO8_VALUE_REF && pt.as.ref != NULL);
    q = pt.as.ref;
    assert(!q->has_a);
    h = fnv(fnv(h, (uint32_t)q->p.x), (uint32_t)q->p.y);
    h = fnv(fnv(h, (uint32_t)q->v.x), (uint32_t)q->v.y);
    return fnv(h, (uint32_t)q->l);
}

/* The kind of the probe's `et`: absent, `false`, a planet, a slipway, the
 * throwaway struct. */
static uint32_t et_kind(const void *e) {
    pico8_value_t et;
    const void *ref;

    if (!lookup(e, "et", &et)) {
        return 0;
    }
    if (et.kind == PICO8_VALUE_BOOL) {
        assert(et.as.boolean == 0);
        return 1;
    }
    assert(et.kind == PICO8_VALUE_REF);
    ref = et.as.ref;
    if (listed(pico8_cg("np"), ref)) {
        return 2;
    }
    if (pico8_type_tag_of(ref) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)ref)->cls == pico8_jc_struct_class()) {
        return 3;
    }
    assert(is_bv_target(ref));
    return 4;
}

/* The first planet of cg.np. */
static void *a_planet(void) {
    assert(list_count(pico8_cg("np")) > 0);
    return pico8_cg("np")->items[0].as.ref;
}

/* A slipway between two bare planets far from each other and from the
 * rest, its `re` set directly; its middle is the returned point. */
static void *a_slipway(pico8_vec_t *middle) {
    pico8_jo_keys_t keys;
    pico8_jc_keys_t jc_keys = {0};
    void *qu;
    void *re;
    void *jc;
    pico8_vec_t base = vec_of(a_planet(), "p");
    pico8_vec_t qu_p = pico8_vec_add(base, vec_xy(0, 1));
    pico8_vec_t re_p = pico8_vec_add(base, vec_xy(1, 1));

    keys.p = vec_ref(qu_p).as.ref;
    keys.ra = NULL;
    keys.id = "s91";
    qu = pico8_jo_new(&keys);
    keys.p = vec_ref(re_p).as.ref;
    keys.id = "s92";
    re = pico8_jo_new(&keys);
    jc_keys.qu = qu;
    jc = pico8_jc_new(&jc_keys);
    ((pico8_jc_t *)jc)->re = re;
    *middle = pico8_vec_div(pico8_vec_add(qu_p, re_p), pico8_fixed_from_int(2));
    return jc;
}

/* The mouse one pixel off `at` (world units, as `p`): `ij` skips an entity
 * at the very position. */
static pico8_value_t mouse_at(pico8_vec_t at) {
    return vec_ref(pico8_vec_add(pico8_vec_mul(at, pico8_fixed_from_int(128)), vec_xy(1, 1)));
}

/* `hq` at `at`, the raw mouse at `rp`: `et`'s kind, the target's `fo`,
 * `ra` the very REF passed, `p`. */
static uint32_t fold_hq(uint32_t h, void *e, pico8_vec_t at, int rp, uint32_t want_kind) {
    pico8_value_t mp = mouse_at(at);
    uint32_t kind;

    (void)call(e, "hq", 2, mp, vec_ref(vec_xy(rp, rp)));
    kind = et_kind(e);
    printf("[hq] et kind %u\n", (unsigned)kind);
    fflush(stdout);
    assert(kind == want_kind);
    assert(get_ref(e, "ra") == mp.as.ref);
    assert(kind == 1 || get_ref(get_ref(e, "et"), "fo") == e);
    h = fnv(h, kind);
    h = fold_value(h, get(e, "et"));
    h = fold_vec(h, e, "p");
    return h;
}

/* `mm`; `hq` on a planet, a slipway, the void; `ev` with a target and
 * without; `dc("kj")` with `qy`'s size; `dg` steps, then `p_` set by `ey`
 * taking it out of `hv` and cg. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;
    void *jc;
    pico8_vec_t middle;
    int i;

    start_game();
    a = ldown();
    b = pico8_g__new(NULL);
    jc = a_slipway(&middle);
    h = fold_particle(h, call(a, "mm", 0, nil(), nil()));
    h = fold_particle(h, call(b, "mm", 0, nil(), nil()));

    h = fold_hq(h, a, vec_of(a_planet(), "p"), 64, 2);
    h = fold_hq(h, a, middle, 64, 3);
    assert(get_ref(jc, "fo") == a);
    h = fold_hq(h, a, pico8_vec_add(middle, vec_xy(0, 3)), 64, 1);
    h = fold_hq(h, a, pico8_vec_add(middle, vec_xy(0, 3)), 2, 4);

    pico8_ke_ks(a, pico8_value_string("kz"));
    pico8_ke_ks(b, pico8_value_string("kz"));
    cls_of(a)->ev(a, NULL);
    h = fnv(h, screen_hash());
    cls_of(b)->ev(b, NULL);
    h = fnv(h, screen_hash());
    (void)call(a, "hq", 2, mouse_at(vec_of(a_planet(), "p")),
               vec_ref(vec_xy(64, 64)));
    cls_of(a)->ev(a, NULL);
    h = fnv(h, screen_hash());

    for (i = 0; i < 3; i++) {
        pico8_dc("kj");
        h = fnv(h, screen_hash());
        h = fnv(h, (uint32_t)size_of(a, "qy"));
        h = fnv(h, (uint32_t)size_of(b, "qy"));
    }
    for (i = 0; i < 3; i++) {
        pico8_dg();
        h = fold_field(h, a, "t");
        h = fold_field(h, b, "t");
        h = fold_field(h, a, "me");
        assert(listed(pico8_hv(), a) && listed(pico8_hv(), b));
    }
    assert(call(a, "ey", 0, nil(), nil()).kind == PICO8_VALUE_REF);
    assert(pico8_value_equal(get(a, "p_"), pico8_value_bool(1)));
    pico8_dg();
    assert(!listed(pico8_hv(), a) && !listed(pico8_cg("g_"), a) && !listed(pico8_cg("prober"), a));
    assert(listed(pico8_hv(), b) && listed(pico8_cg("g_"), b));
    h = fnv(h, (uint32_t)pico8_hv()->count);
    check("life", h, LIFE_HASH);
}

/* `ey` with a target: `p_`, nil returned. */
static void test_ey_on(void) {
    uint32_t h = 2166136261u;
    void *a;
    pico8_value_t out;

    start_game();
    a = ldown();
    (void)call(a, "hq", 2, mouse_at(vec_of(a_planet(), "p")),
               vec_ref(vec_xy(64, 64)));
    out = call(a, "ey", 0, nil(), nil());
    assert(out.kind == PICO8_VALUE_REF && out.as.ref == NULL);
    assert(pico8_value_equal(get(a, "p_"), pico8_value_bool(1)));
    h = fold_field(h, a, "me");
    h = fold_field(h, a, "t");
    h = fold_field(h, a, "p_");
    h = fold_field(h, pico8_fm(), "nz");
    check("ey on", h, EY_ON_HASH);
}

/* `ey` with no target and enough credits: `ks(dq)`, true returned. */
static void test_ey_off(void) {
    uint32_t h = 2166136261u;
    void *a;
    pico8_value_t out;

    start_game();
    a = ldown();
    out = call(a, "ey", 0, nil(), nil());
    assert(pico8_value_equal(out, pico8_value_bool(1)));
    assert(!has(a, "p_"));
    assert(is_string(a, "me", "ly"));
    assert(get(a, "t").as.fixed == pico8_fixed_from_int(0));
    h = fold_field(h, a, "me");
    h = fold_field(h, a, "t");
    h = fold_field(h, pico8_fm(), "nz");
    check("ey off", h, EY_OFF_HASH);
}

/* `ey` over the void, off the panel: `et` is the throwaway target, which is
 * truthy, so the drop is refused (`p_`, nil returned) and costs nothing. */
static void test_ey_void(void) {
    void *a;
    pico8_vec_t middle;
    pico8_value_t nz;
    pico8_value_t me;
    pico8_value_t out;

    start_game();
    a = ldown();
    (void)a_slipway(&middle);
    (void)call(a, "hq", 2, mouse_at(pico8_vec_add(middle, vec_xy(0, 3))), vec_ref(vec_xy(2, 2)));
    assert(et_kind(a) == 4);
    assert(get_ref(get_ref(a, "et"), "fo") == a);
    assert(!has(a, "p_"));
    nz = get(pico8_fm(), "nz");
    me = get(a, "me");
    out = call(a, "ey", 0, nil(), nil());
    assert(out.kind == PICO8_VALUE_REF && out.as.ref == NULL);
    assert(pico8_value_equal(get(a, "p_"), pico8_value_bool(1)));
    assert(pico8_value_equal(get(pico8_fm(), "nz"), nz));
    assert(pico8_value_equal(get(a, "me"), me));
    assert(et_kind(a) == 4);
    printf("[ey void] refused\n");
}

/* The help: `g_`'s `nu` and `fa`=10. */
static int is_help(const void *e) {
    pico8_value_t fa;
    return e != NULL && get_ref(e, "nu") == &pico8_g__nu_tags && lookup(e, "fa", &fa) &&
           fa.kind == PICO8_VALUE_FIXED && fa.as.fixed == pico8_fixed_from_int(10);
}

/* The title screen's `fv`, whose class's parent is `g_`'s class: `nu`,
 * `fx`, `dq` inherited, `ev`, `mm` the functions of `g_`. */
static void test_fv(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *hv;
    void *fv = NULL;
    size_t d;
    int i;

    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    hv = pico8_hv();
    for (i = 0; i < hv->count; i++) {
        void *e = hv->items[i].as.ref;
        if (is_help(e)) {
            assert(fv == NULL);
            fv = e;
        }
    }
    assert(fv != NULL && !is_probe(fv));
    assert(get_ref(fv, "nu") == &pico8_g__nu_tags);
    assert(get(fv, "fx").as.fixed == pico8_fixed_from_int(1));
    assert(is_string(fv, "dq", "ly"));
    assert(get(fv, "fa").as.fixed == pico8_fixed_from_int(10));
    assert(get_ref(fv, "k_") == &pico8_fv_k__seq);
    assert(get_ref(fv, "k_") != &pico8_g__k__seq);
    assert(cls_of(fv)->ev == pico8_dl_dispatch);
    assert(method(fv, "mm") == pico8_g__mm);
    assert(method(fv, "hq") == pico8_bv_dispatch);
    assert(method(fv, "ey") == pico8_d__dispatch);
    assert(cls_of(fv)->kj != pico8_ei_dispatch);
    assert(cls_of(fv)->kj == pico8_fv_kj_dispatch);
    assert(cls_of(fv)->hd != NULL);
    assert(pico8_g__ready());
    assert(size_of(fv, "qy") >= 0);
    h = fnv(h, (uint32_t)size_of(fv, "qy"));
    h = fnv(h, (uint32_t)rank_in(hv, fv));
    h = fnv(h, (uint32_t)rank_in(pico8_cg("g_"), fv));
    h = fnv(h, (uint32_t)rank_in(pico8_cg("prober"), fv));
    assert(listed(pico8_cg("g_"), fv) && listed(pico8_cg("prober"), fv));
    for (d = 0; d < sizeof defaults / sizeof defaults[0]; d++) {
        h = fold_field(h, fv, defaults[d]);
    }
    for (i = 0; i < 3; i++) {
        pico8_dc("kj");
        h = fnv(h, screen_hash());
        h = fnv(h, (uint32_t)size_of(fv, "qy"));
    }
    check("fv", h, FV_HASH);
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

static void write_fixed(void *e, const char *name) {
    pico8_entity_set_field(e, name, pico8_value_fixed(pico8_fixed_from_int(9)));
}

static void write_dw(void *e) {
    write_fixed(e, "dw");
}

static void write_fx(void *e) {
    write_fixed(e, "fx");
}

static void write_nu(void *e) {
    write_fixed(e, "nu");
}

static void write_k_(void *e) {
    write_fixed(e, "k_");
}

static void write_fo(void *e) {
    write_fixed(e, "fo");
}

static void write_m_(void *e) {
    write_fixed(e, "m_");
}

static void write_me(void *e) {
    write_fixed(e, "me");
}

static void write_qs(void *e) {
    write_fixed(e, "qs");
}

static int is_a_g__struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_g__struct_class();
}

/* Built by any road, a probe is a struct whose descriptor holds the 5
 * public functions of `g_`, which its class no longer installs. The title's
 * `fv` is a struct of its own. */
static void test_g__is_a_struct(void) {
    static const char *const entries[] = {"hq", "ey", "mm"};
    static const pico8_value_function_t fns[] = {pico8_bv_dispatch, pico8_d__dispatch, pico8_g__mm};
    const pico8_struct_class_t *cls = pico8_g__struct_class();
    const pico8_list_t *hv;
    void *roads[3];
    pico8_value_t v;
    size_t i;
    int j;

    start_game();
    roads[0] = ldown();
    roads[1] = pico8_g__new(NULL);
    roads[2] = keyed();
    for (i = 0; i < 3; i++) {
        assert(is_a_g__struct(roads[i]));
    }
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        int named = 0;
        for (i = 0; i < sizeof entries / sizeof entries[0]; i++) {
            if (j == pico8_method_of(entries[i])) {
                assert(cls->methods[j] == fns[i]);
                named = 1;
            }
        }
        assert(named || cls->methods[j] == NULL);
    }
    assert(cls->methods[PICO8_METHOD_HQ] == pico8_bv_dispatch && cls->methods[PICO8_METHOD_EY] == pico8_d__dispatch &&
           cls->ev == pico8_dl_dispatch && cls->kj == pico8_ei_dispatch &&
           cls->methods[PICO8_METHOD_MM] == pico8_g__mm);
    assert(cls->hd == NULL && cls->hk == NULL && cls->hb == NULL && cls->has_hb == NULL && cls->ldown == NULL &&
           cls->rdown == NULL && cls->gv == NULL);
    for (j = 0; j < PICO8_STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(cls->field != NULL && cls->set_field != NULL);
    assert(pico8_g__ready());

    must_abort(write_dw, roads[0]);
    must_abort(write_fx, roads[0]);
    must_abort(write_nu, roads[0]);
    must_abort(write_k_, roads[0]);
    must_abort(write_fo, roads[0]);
    must_abort(write_m_, roads[0]);
    must_abort(write_me, roads[0]);
    ((pico8_g__t *)roads[1])->fa = 0;
    assert(cls->field(roads[1], "fa", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0 && !is_probe(roads[1]));

    pico8__init();
    hv = pico8_hv();
    for (j = 0; j < hv->count; j++) {
        const void *e = hv->items[j].as.ref;
        if (is_help(e)) {
            break;
        }
    }
    assert(j < hv->count);
    assert(pico8_type_tag_of(hv->items[j].as.ref) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)hv->items[j].as.ref)->cls == pico8_fv_struct_class());
}

/* The throwaway target is a struct that keeps `fo` only: no method, no
 * other field, and a write of another name stops the run. */
static void test_bv_et_is_a_struct(void) {
    static const char *const absent[] = {"qs", "hq", "p", "me"};
    void *a;
    void *et;
    pico8_vec_t middle;
    size_t i;

    start_game();
    a = ldown();
    (void)a_slipway(&middle);
    (void)call(a, "hq", 2, mouse_at(pico8_vec_add(middle, vec_xy(0, 3))), vec_ref(vec_xy(2, 2)));
    et = get_ref(a, "et");
    assert(is_bv_target(et) && get_ref(et, "fo") == a);
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(et, absent[i]));
    }
    assert(cls_of(et)->ev == NULL);
    must_abort(write_qs, et);
    pico8_entity_set_field(et, "fo", pico8_value_ref(NULL));
    assert(!has(et, "fo"));
}

/* `fo` of the throwaway target read by the raw reader and by name, which
 * agree; absent leaves the witness untouched. Returns presence, then kind,
 * then the rank in `holders` of the REF read (-1 off the list). */
static uint32_t fold_bv_et_fo(uint32_t h, void *t, void *const *holders, int n, void *want) {
    pico8_value_t raw = pico8_value_fixed(0x1234);
    pico8_value_t named = pico8_value_fixed(0x1234);
    int got_raw = pico8_bv_et_struct_class()->field(t, "fo", &raw);
    int got_named = pico8_entity_get_field(t, "fo", &named);
    int i;

    assert(got_raw == got_named && raw.kind == named.kind);
    if (!got_raw) {
        assert(want == NULL && raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        assert(named.kind == PICO8_VALUE_FIXED && named.as.fixed == 0x1234);
        return fnv(fnv(h, 0), 0);
    }
    assert(raw.kind == PICO8_VALUE_REF && raw.as.ref == want && named.as.ref == want);
    for (i = 0; i < n && holders[i] != raw.as.ref; i++) {
    }
    h = fnv(h, 1);
    h = fnv(h, (uint32_t)raw.kind);
    return fnv(h, (uint32_t)(i < n ? i : -1));
}

static void *bv_et_target(void **holder_out, void **jc_out, pico8_vec_t *at_out) {
    void *a;
    void *t;
    pico8_vec_t middle;

    start_game();
    a = ldown();
    *jc_out = a_slipway(&middle);
    *at_out = pico8_vec_add(middle, vec_xy(0, 3));
    (void)call(a, "hq", 2, mouse_at(*at_out), vec_ref(vec_xy(2, 2)));
    t = get_ref(a, "et");
    assert(is_bv_target(t));
    *holder_out = a;
    return t;
}

/* The state of `mz`'s `et` after its `hq`: absent, the throwaway target,
 * the `jj` wrapper, another value. */
static uint32_t mz_et_state(const void *mz, const void *t) {
    pico8_value_t et;

    if (!lookup(mz, "et", &et)) {
        return 0;
    }
    if (et.kind != PICO8_VALUE_REF) {
        return 10 + (uint32_t)et.kind;
    }
    if (et.as.ref == t) {
        return 1;
    }
    if (et.as.ref == (const void *)pico8_mz_jj_wrapper()) {
        return 2;
    }
    return 3;
}

/* `fo` by name with each of the eleven classes whose `hq` runs `bv`: the
 * classes of the typed member, plus `fv`, `nl`, `pm`. */
static void test_bv_et_members_by_name(void) {
    static const char *const strangers[] = {"", "x", "FO", "fo ", "qs", "et", "header", "cls"};
    static const char *const unwritten[] = {"qs", "et", "", "fo ", "FO"};
    const pico8_struct_class_t *want_cls[11];
    void *holders[11];
    void *a;
    void *t;
    void *jc;
    pico8_vec_t at;
    pico8_value_t zero_ref;
    uint32_t h = 2166136261u;
    size_t i;
    int k;

    t = bv_et_target(&a, &jc, &at);

    h = fold_bv_et_fo(h, t, &a, 1, a);
    for (i = 0; i < sizeof strangers / sizeof strangers[0]; i++) {
        pico8_value_t raw = pico8_value_fixed(0x1234);
        pico8_value_t named = pico8_value_fixed(0x1234);
        int got_raw = pico8_bv_et_struct_class()->field(t, strangers[i], &raw);
        int got_named = pico8_entity_get_field(t, strangers[i], &named);
        assert(!got_raw && !got_named);
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        assert(named.kind == PICO8_VALUE_FIXED && named.as.fixed == 0x1234);
        h = fnv(h, (uint32_t)got_raw);
    }

    holders[0] = a;
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
    want_cls[0] = pico8_g__struct_class();
    want_cls[1] = pico8_fv_struct_class();
    want_cls[2] = pico8_mz_struct_class();
    want_cls[3] = pico8_qh_struct_class();
    want_cls[4] = pico8_nl_struct_class();
    want_cls[5] = pico8_os_struct_class();
    want_cls[6] = pico8_hj_struct_class();
    want_cls[7] = pico8_pm_struct_class();
    want_cls[8] = pico8_nr_struct_class();
    want_cls[9] = pico8_fr_struct_class();
    want_cls[10] = pico8_mx_struct_class();
    for (k = 0; k < 11; k++) {
        assert(pico8_type_tag_of(holders[k]) == PICO8_TYPE_TAG_STRUCT &&
               ((const pico8_struct_header_t *)holders[k])->cls == want_cls[k]);
    }

    for (k = 0; k < 11; k++) {
        (void)call(holders[k], "hq", 2, mouse_at(at), vec_ref(vec_xy(2, 2)));
        h = fold_bv_et_fo(h, t, holders, 11, holders[k]);
        if (k == 2) {
            h = fnv(h, mz_et_state(holders[k], t));
        } else {
            assert(get_ref(holders[k], "et") == t);
        }
    }

    for (k = 10; k >= 0; k--) {
        pico8_entity_set_field(t, "fo", pico8_value_ref(holders[k]));
        h = fold_bv_et_fo(h, t, holders, 11, holders[k]);
        h = fnv(h, (uint32_t)pico8_bv_et_struct_class()->set_field(t, "fo", pico8_value_ref(a)));
        h = fold_bv_et_fo(h, t, holders, 11, a);
    }
    pico8_entity_set_field(t, "fo", pico8_value_ref(NULL));
    h = fold_bv_et_fo(h, t, holders, 11, NULL);
    memset(&zero_ref, 0, sizeof zero_ref);
    zero_ref.kind = PICO8_VALUE_REF;
    pico8_entity_set_field(t, "fo", zero_ref);
    h = fold_bv_et_fo(h, t, holders, 11, NULL);
    pico8_entity_set_field(t, "fo", pico8_value_ref(NULL));
    h = fold_bv_et_fo(h, t, holders, 11, NULL);
    pico8_entity_set_field(t, "fo", pico8_value_ref(holders[0]));
    h = fold_bv_et_fo(h, t, holders, 11, holders[0]);

    for (i = 0; i < sizeof unwritten / sizeof unwritten[0]; i++) {
        h = fnv(h, (uint32_t)pico8_bv_et_struct_class()->set_field(t, unwritten[i], pico8_value_ref(a)));
        h = fold_bv_et_fo(h, t, holders, 11, holders[0]);
    }

    assert(get_ref(a, "et") == t);
    check("bv_et members", h, BV_ET_MEMBERS_HASH);
}

static void write_fo_by_name(void *e, pico8_value_t v) {
    pico8_entity_set_field(e, "fo", v);
}

static void must_abort_value(void *e, void (*write)(void *, pico8_value_t), pico8_value_t v) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        write(e, v);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

/* `fo` takes nil or an object whose `hq` runs `bv` only. */
static void test_bv_et_writer_stops_off_row(void) {
    pico8_value_t zero_fixed;
    pico8_value_t zero_false;
    pico8_value_t zero_string;
    pico8_value_t zero_function;
    pico8_vec_t at;
    void *a;
    void *t;
    void *jc;

    t = bv_et_target(&a, &jc, &at);
    memset(&zero_fixed, 0, sizeof zero_fixed);
    zero_fixed.kind = PICO8_VALUE_FIXED;
    memset(&zero_false, 0, sizeof zero_false);
    zero_false.kind = PICO8_VALUE_BOOL;
    memset(&zero_string, 0, sizeof zero_string);
    zero_string.kind = PICO8_VALUE_STRING;
    memset(&zero_function, 0, sizeof zero_function);
    zero_function.kind = PICO8_VALUE_FUNCTION;

    must_abort_value(t, write_fo_by_name, pico8_value_fixed(pico8_fixed_from_int(1)));
    must_abort_value(t, write_fo_by_name, zero_fixed);
    must_abort_value(t, write_fo_by_name, pico8_value_bool(1));
    must_abort_value(t, write_fo_by_name, zero_false);
    must_abort_value(t, write_fo_by_name, pico8_value_string("x"));
    must_abort_value(t, write_fo_by_name, zero_string);
    must_abort_value(t, write_fo_by_name, zero_function);
    must_abort_value(t, write_fo_by_name, vec_ref(vec_xy(1, 1)));
    must_abort_value(t, write_fo_by_name, pico8_value_ref((void *)pico8_hv()));
    must_abort_value(t, write_fo_by_name, get(jc, "qu"));
    must_abort_value(t, write_fo_by_name, pico8_value_ref(jc));
    must_abort_value(t, write_fo_by_name, pico8_value_ref(t));
    must_abort_value(t, write_fo_by_name, pico8_value_ref(pico8_mz_jj_wrapper()));
}

static const char *const g__names[23] = {"nu", "fx", "dq", "od", "fp", "fa", "dw", "py", "ki", "pn", "pk", "fc",
                                         "k_", "qo", "ro", "qg", "q_", "du", "qy", "ra", "p",  "et", "p_"};

static pico8_value_t zero_union(pico8_value_kind_t kind) {
    pico8_value_t v;

    memset(&v, 0, sizeof v);
    v.kind = kind;
    return v;
}

static int is_of(const void *e, const pico8_struct_class_t *cls) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)e)->cls == cls;
}

/* The raw reader and the name reader agree; absent leaves both witnesses
 * untouched. */
static int read_both(const void *e, const char *name, pico8_value_t *out) {
    const pico8_struct_header_t *header = e;
    pico8_value_t raw = pico8_value_fixed(0x1234);
    pico8_value_t named = pico8_value_fixed(0x1234);
    int got_raw = header->cls->field(e, name, &raw);
    int got_named = pico8_entity_get_field(e, name, &named);

    assert(got_raw == got_named);
    if (!got_raw) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        assert(named.kind == PICO8_VALUE_FIXED && named.as.fixed == 0x1234);
        return 0;
    }
    assert(pico8_value_equal(raw, named));
    *out = raw;
    return 1;
}

static void write_by_name(void *e, const char *name, pico8_value_t v) {
    const pico8_struct_header_t *header = e;

    assert(header->cls->set_field(e, name, v) == 1);
    pico8_entity_set_field(e, name, v);
}

/* A fresh planet of the k-th class of `cg.np`: jo, then qh, nl, os, hj, pm,
 * nr, fr, mx, at `at`. */
static void *np_target_at(int k, pico8_vec_t at) {
    pico8_jo_keys_t keys;
    void *t;

    switch (k) {
    case 0:
        keys.id = NULL;
        keys.p = vec_ref(at).as.ref;
        keys.ra = NULL;
        return pico8_jo_new(&keys);
    case 1:
        t = pico8_qh_new(NULL);
        break;
    case 2:
        t = pico8_nl_new(NULL);
        break;
    case 3:
        t = pico8_os_new(NULL);
        break;
    case 4:
        t = pico8_hj_new(NULL);
        break;
    case 5:
        t = pico8_pm_new(NULL);
        break;
    case 6:
        t = pico8_nr_new(NULL);
        break;
    case 7:
        t = pico8_fr_new(NULL);
        break;
    default:
        t = pico8_mx_new(NULL);
        break;
    }
    pico8_entity_set_field(t, "p", vec_ref(at));
    return t;
}

/* The eleven classes `bv` aims at: jo, qh, nl, os, hj, pm, nr, fr, mx, jc,
 * the throwaway target. */
static uint32_t target_rank(const void *t) {
    const pico8_struct_class_t *cls[11];
    uint32_t k;

    cls[0] = pico8_jo_struct_class();
    cls[1] = pico8_qh_struct_class();
    cls[2] = pico8_nl_struct_class();
    cls[3] = pico8_os_struct_class();
    cls[4] = pico8_hj_struct_class();
    cls[5] = pico8_pm_struct_class();
    cls[6] = pico8_nr_struct_class();
    cls[7] = pico8_fr_struct_class();
    cls[8] = pico8_mx_struct_class();
    cls[9] = pico8_jc_struct_class();
    cls[10] = pico8_bv_et_struct_class();
    for (k = 0; k < 11; k++) {
        if (is_of(t, cls[k])) {
            return k;
        }
    }
    assert(0 && "et holds one of the eleven classes bv aims at");
    return 11;
}

/* Presence, kind, value; a REF by what it points to, never its address. */
static uint32_t fold_member(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    const pico8_vec_t *p;

    if (!read_both(e, name, &v)) {
        return fnv(h, 0u);
    }
    h = fnv(h, 1u + (uint32_t)v.kind);
    switch (v.kind) {
    case PICO8_VALUE_FIXED:
        return fnv(h, (uint32_t)v.as.fixed);
    case PICO8_VALUE_BOOL:
        return fnv(h, (uint32_t)v.as.boolean);
    case PICO8_VALUE_STRING:
        return fnv_str(h, v.as.string);
    case PICO8_VALUE_REF:
        assert(v.as.ref != NULL);
        if (v.as.ref == (const void *)&pico8_g__nu_tags) {
            return fold_tags(h, v.as.ref);
        }
        if (v.as.ref == (const void *)&pico8_g__k__seq || v.as.ref == (const void *)&pico8_fv_k__seq) {
            return fold_seq(h, v.as.ref);
        }
        h = fnv(h, (uint32_t)pico8_type_tag_of(v.as.ref));
        switch (pico8_type_tag_of(v.as.ref)) {
        case PICO8_TYPE_TAG_LIST:
            return fnv(h, (uint32_t)((const pico8_list_t *)v.as.ref)->count);
        case PICO8_TYPE_TAG_STRUCT:
            return fnv(h, target_rank(v.as.ref));
        default:
            p = v.as.ref;
            return fnv(fnv(h, (uint32_t)p->x), (uint32_t)p->y);
        }
    default:
        assert(0 && "no member of g_ holds a function");
        return h;
    }
}

static uint32_t fold_g__all(uint32_t h, const void *e) {
    size_t i;

    for (i = 0; i < sizeof g__names / sizeof g__names[0]; i++) {
        h = fold_member(h, e, g__names[i]);
    }
    return h;
}

static void assert_same(const void *e, const char *name, pico8_value_t want) {
    pico8_value_t v;

    assert(read_both(e, name, &v) && pico8_value_equal(v, want));
}

static void assert_absent(const void *e, const char *name) {
    pico8_value_t v;

    assert(!read_both(e, name, &v));
}

/* A fresh probe: `pico8_g__start` at the very pointers but for the keys,
 * an empty `qy` of its own, nothing `bv`/`d_` write. */
static uint32_t fold_fresh_probe(uint32_t h, const void *e, int keyed) {
    const pico8_g__t *g_ = e;

    h = fold_g__all(h, e);
    assert(g_->fx == pico8_g__start.fx);
    assert(g_->dq == pico8_g__start.dq);
    assert(g_->od == pico8_g__start.od);
    assert(g_->fp == pico8_g__start.fp);
    assert(g_->fa == pico8_g__start.fa);
    assert(g_->dw == pico8_g__start.dw);
    assert(keyed || g_->py == pico8_g__start.py);
    assert(g_->ki == pico8_g__start.ki);
    assert(keyed || g_->pn == pico8_g__start.pn);
    assert(g_->pk == pico8_g__start.pk);
    assert(g_->fc == pico8_g__start.fc);
    assert(g_->qo == pico8_g__start.qo);
    assert(g_->ro == pico8_g__start.ro);
    assert(g_->qg == pico8_g__start.qg);
    assert(g_->q_ == pico8_g__start.q_);
    assert(keyed || g_->du == pico8_g__start.du);
    assert(get_ref(e, "nu") == pico8_g__start.nu && pico8_g__start.nu == &pico8_g__nu_tags);
    assert(get_ref(e, "k_") == pico8_g__start.k_ && pico8_g__start.k_ == &pico8_g__k__seq);
    assert(size_of(e, "qy") == 0);
    assert_absent(e, "ra");
    assert_absent(e, "p");
    assert_absent(e, "et");
    assert_absent(e, "p_");
    return h;
}

/* A fresh fv: its 13 overriding defaults from `pico8_fv_start`, `ra` its
 * very vector, the other 5 from `pico8_g__start`. */
static uint32_t fold_fresh_fv(uint32_t h, const void *e) {
    const pico8_g__t *g_ = e;

    h = fold_g__all(h, e);
    assert(g_->fa == pico8_fv_start.fa);
    assert(g_->ra == &pico8_fv_start.ra);
    assert_same(e, "ra", pico8_value_ref((void *)&pico8_fv_start.ra));
    assert(g_->fc == pico8_fv_start.fc);
    assert(g_->qo == pico8_fv_start.qo);
    assert(g_->ro == pico8_fv_start.ro);
    assert(g_->qg == pico8_fv_start.qg);
    assert(g_->q_ == pico8_fv_start.q_);
    assert(g_->pn == pico8_fv_start.pn);
    assert(g_->du == pico8_fv_start.du);
    assert(g_->py == pico8_fv_start.py);
    assert(g_->pk == pico8_fv_start.pk);
    assert(g_->ki == pico8_fv_start.ki);
    assert(g_->fx == pico8_g__start.fx);
    assert(g_->dq == pico8_g__start.dq);
    assert(g_->od == pico8_g__start.od);
    assert(g_->fp == pico8_g__start.fp);
    assert(g_->dw == pico8_g__start.dw);
    assert(get_ref(e, "nu") == pico8_g__start.nu);
    assert(get_ref(e, "k_") == pico8_fv_start.k_ && pico8_fv_start.k_ == &pico8_fv_k__seq);
    assert(size_of(e, "qy") >= 0);
    assert_absent(e, "p");
    assert_absent(e, "et");
    assert_absent(e, "p_");
    return h;
}

/* `hq` of `e` with the mouse one pixel off `at`, the raw mouse at `rp`:
 * `ra` is the very REF passed. */
static uint32_t hq_at(uint32_t h, void *e, pico8_vec_t at, int rp) {
    pico8_value_t mp = mouse_at(at);

    (void)call(e, "hq", 2, mp, vec_ref(vec_xy(rp, rp)));
    assert(get_ref(e, "ra") == mp.as.ref);
    h = fold_member(h, e, "et");
    h = fold_member(h, e, "ra");
    return fold_member(h, e, "p");
}

/* What `bv` writes by name into a probe and into an fv, and what the
 * writer takes and refuses by name. */
static uint32_t fold_written_by_name(uint32_t h, void *e, void *const *targets) {
    pico8_list_t *list = malloc(sizeof *list);
    pico8_value_t vec = vec_ref(vec_xy(3, 4));
    const char *const vecs[2] = {"ra", "p"};
    size_t i;

    pico8_list_init(list);
    write_by_name(e, "qy", pico8_value_ref(list));
    assert(get_ref(e, "qy") == list);
    h = fold_member(h, e, "qy");
    write_by_name(e, "qy", pico8_value_ref(NULL));
    assert_absent(e, "qy");
    h = fold_member(h, e, "qy");

    for (i = 0; i < 2; i++) {
        write_by_name(e, vecs[i], vec);
        assert(get_ref(e, vecs[i]) == vec.as.ref);
        h = fold_member(h, e, vecs[i]);
        write_by_name(e, vecs[i], pico8_value_ref(NULL));
        assert_absent(e, vecs[i]);
        h = fold_member(h, e, vecs[i]);
    }

    for (i = 0; i < 11; i++) {
        write_by_name(e, "et", pico8_value_ref(targets[i]));
        assert(get_ref(e, "et") == targets[i]);
        h = fold_member(h, e, "et");
    }
    write_by_name(e, "et", pico8_value_bool(0));
    assert_same(e, "et", pico8_value_bool(0));
    h = fold_member(h, e, "et");
    write_by_name(e, "et", pico8_value_ref(NULL));
    assert_absent(e, "et");
    h = fold_member(h, e, "et");
    write_by_name(e, "et", zero_union(PICO8_VALUE_BOOL));
    assert_same(e, "et", pico8_value_bool(0));
    h = fold_member(h, e, "et");
    write_by_name(e, "et", pico8_value_ref(NULL));
    assert_absent(e, "et");

    write_by_name(e, "p_", pico8_value_bool(1));
    assert_same(e, "p_", pico8_value_bool(1));
    h = fold_member(h, e, "p_");
    write_by_name(e, "p_", pico8_value_bool(0));
    assert_same(e, "p_", pico8_value_bool(0));
    h = fold_member(h, e, "p_");
    write_by_name(e, "p_", zero_union(PICO8_VALUE_BOOL));
    assert_same(e, "p_", pico8_value_bool(0));
    h = fold_member(h, e, "p_");
    write_by_name(e, "p_", pico8_value_bool(1));
    write_by_name(e, "p_", pico8_value_ref(NULL));
    assert_absent(e, "p_");
    return fold_member(h, e, "p_");
}

/* g_'s 23 names by name, on the old and on the typed form alike. */
static void test_g__members_by_name(void) {
    static const char *const unwritten[] = {"me", "t", "fo", "m_", "", "qy ", "P_", "has_et", "has_p_"};
    static const char *const strangers[] = {"", "n", "nu ", "NU", "et_", "has_et", "has_p_", "p__", "q"};
    uint32_t h = 2166136261u;
    const pico8_list_t *hv;
    pico8_g__keys_t zero_keys = {0};
    void *targets[11];
    void *fv[2] = {NULL, NULL};
    void *probe[3];
    void *e;
    pico8_vec_t middle;
    pico8_value_t v;
    size_t i;
    int k;

    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    hv = pico8_hv();
    for (k = 0; k < hv->count; k++) {
        if (is_help(hv->items[k].as.ref)) {
            fv[0] = hv->items[k].as.ref;
        }
    }
    fv[1] = pico8_fv_new();
    for (k = 0; k < 2; k++) {
        assert(fv[k] != NULL && is_of(fv[k], pico8_fv_struct_class()));
        h = fold_fresh_fv(h, fv[k]);
    }
    assert(get_ref(fv[0], "qy") != get_ref(fv[1], "qy"));

    start_game();
    probe[0] = ldown();
    probe[1] = pico8_g__new(NULL);
    probe[2] = keyed();
    for (k = 0; k < 3; k++) {
        h = fold_fresh_probe(h, probe[k], k == 2);
    }
    assert(get_ref(probe[0], "qy") != get_ref(probe[1], "qy") && get_ref(probe[1], "qy") != get_ref(probe[2], "qy"));
    assert_same(probe[2], "pn", pico8_value_fixed(pico8_fixed_from_int(7)));
    assert_same(probe[2], "du", pico8_value_fixed(pico8_fixed_from_int(10)));
    assert_same(probe[2], "py", pico8_value_fixed(pico8_fixed_from_int(1)));
    assert_same(probe[2], "t", pico8_value_fixed(pico8_fixed_from_int(3)));

    targets[9] = a_slipway(&middle);
    e = pico8_g__new(NULL);
    for (k = 0; k < 9; k++) {
        pico8_vec_t at = vec_xy(100 + 2 * k, 100);
        targets[k] = np_target_at(k, at);
        h = hq_at(h, e, at, 2);
        assert(get_ref(e, "et") == targets[k] && get_ref(targets[k], "fo") == e);
    }
    h = hq_at(h, e, middle, 2);
    assert(get_ref(e, "et") == targets[9] && get_ref(targets[9], "fo") == e);
    h = hq_at(h, e, vec_xy(200, 200), 2);
    targets[10] = get_ref(e, "et");
    assert(is_bv_target(targets[10]) && get_ref(targets[10], "fo") == e);
    h = hq_at(h, e, vec_xy(200, 200), 64);
    assert_same(e, "et", pico8_value_bool(0));
    {
        pico8_vec_t at = vec_xy(130, 100);
        void *t = np_target_at(0, at);
        h = hq_at(h, fv[1], at, 2);
        assert(get_ref(fv[1], "et") == t && get_ref(t, "fo") == fv[1]);
    }

    h = fold_written_by_name(h, e, targets);
    h = fold_written_by_name(h, fv[1], targets);
    e = pico8_g__new(NULL);
    for (i = 0; i < 18; i++) {
        h = fnv(h, (uint32_t)pico8_g__struct_class()->set_field(e, g__names[i], pico8_value_fixed(7)));
    }
    for (i = 0; i < sizeof unwritten / sizeof unwritten[0]; i++) {
        h = fnv(h, (uint32_t)pico8_g__struct_class()->set_field(e, unwritten[i], pico8_value_fixed(7)));
    }
    h = fold_fresh_probe(h, e, 0);

    zero_keys.has_pn = 1;
    zero_keys.pn = 0;
    zero_keys.has_du = 1;
    zero_keys.du = 0;
    zero_keys.has_py = 1;
    zero_keys.py = 0;
    e = pico8_g__new(&zero_keys);
    assert_same(e, "pn", pico8_value_fixed(0));
    assert_same(e, "du", pico8_value_fixed(0));
    assert_same(e, "py", pico8_value_fixed(0));
    h = fold_g__all(h, e);

    for (i = 0; i < sizeof strangers / sizeof strangers[0]; i++) {
        h = fnv(h, (uint32_t)read_both(e, strangers[i], &v));
        assert(!read_both(e, strangers[i], &v));
    }

    start_game();
    probe[0] = ldown();
    h = hq_at(h, probe[0], vec_xy(200, 200), 64);
    h = fold_value(h, call(probe[0], "ey", 0, nil(), nil()));
    h = fold_field(h, probe[0], "me");
    h = fold_field(h, pico8_fm(), "nz");
    h = fold_g__all(h, probe[0]);
    probe[1] = pico8_g__new(NULL);
    h = hq_at(h, probe[1], vec_xy(200, 200), 2);
    h = fold_value(h, call(probe[1], "ey", 0, nil(), nil()));
    assert_same(probe[1], "p_", pico8_value_bool(1));
    h = fold_field(h, pico8_fm(), "nz");
    h = fold_g__all(h, probe[1]);
    cls_of(probe[0])->ev(probe[0], NULL);
    h = fnv(h, screen_hash());
    cls_of(probe[1])->ev(probe[1], NULL);
    h = fnv(h, screen_hash());

    check("g_ members", h, G__MEMBERS_HASH);
}

static const char *g_off_row_name;

static void write_off_row(void *e, pico8_value_t v) {
    pico8_entity_set_field(e, g_off_row_name, v);
}

static void must_abort_all(void *e, const char *name, void (*write)(void *, pico8_value_t), const pico8_value_t *off,
                           size_t n) {
    size_t i;

    g_off_row_name = name;
    for (i = 0; i < n; i++) {
        must_abort_value(e, write, off[i]);
    }
}

/* Each written member takes nil or a value of its type only; `fv` delegates
 * `et` and `p_` to g_'s writer. */
static void test_g__writer_stops_off_row(void) {
    pico8_value_t off[20];
    pico8_value_t structs[8];
    pico8_value_t vec;
    pico8_value_t gb_1;
    pico8_list_t *hv = (pico8_list_t *)pico8_hv();
    void *holders[2];
    void *jc;
    pico8_vec_t middle;
    size_t n;
    size_t i;

    start_game();
    holders[0] = pico8_g__new(NULL);
    holders[1] = pico8_fv_new();
    jc = a_slipway(&middle);
    vec = vec_ref(vec_xy(1, 1));
    gb_1 = pico8_value_ref(pico8_gb_at(1));
    assert(gb_1.as.ref != NULL);

    n = 0;
    off[n++] = pico8_value_fixed(pico8_fixed_from_int(1));
    off[n++] = zero_union(PICO8_VALUE_FIXED);
    off[n++] = pico8_value_bool(1);
    off[n++] = zero_union(PICO8_VALUE_BOOL);
    off[n++] = pico8_value_string("x");
    off[n++] = zero_union(PICO8_VALUE_STRING);
    off[n++] = zero_union(PICO8_VALUE_FUNCTION);
    off[n++] = pico8_value_ref(jc);
    off[n++] = vec;
    must_abort_all(holders[0], "qy", write_off_row, off, n);
    off[n - 1] = pico8_value_ref(hv);
    must_abort_all(holders[0], "ra", write_off_row, off, n);
    must_abort_all(holders[0], "p", write_off_row, off, n);

    n = 0;
    off[n++] = pico8_value_fixed(pico8_fixed_from_int(1));
    off[n++] = zero_union(PICO8_VALUE_FIXED);
    off[n++] = pico8_value_bool(1);
    off[n++] = pico8_value_string("x");
    off[n++] = zero_union(PICO8_VALUE_STRING);
    off[n++] = zero_union(PICO8_VALUE_FUNCTION);
    off[n++] = pico8_value_ref(hv);
    off[n++] = vec;
    structs[0] = pico8_value_ref(pico8_ga_new(NULL));
    structs[1] = pico8_value_ref(pico8_g__new(NULL));
    structs[2] = pico8_value_ref(pico8_fv_new());
    structs[3] = pico8_value_ref(pico8_mz_new());
    structs[4] = pico8_value_ref(the_bg());
    structs[5] = pico8_value_ref(pico8_ok());
    structs[6] = gb_1;
    structs[7] = pico8_value_ref(pico8_mz_jj_wrapper());
    for (i = 0; i < 2; i++) {
        must_abort_all(holders[i], "et", write_off_row, off, n);
        must_abort_all(holders[i], "et", write_off_row, structs, 8);
    }

    n = 0;
    off[n++] = pico8_value_fixed(pico8_fixed_from_int(1));
    off[n++] = zero_union(PICO8_VALUE_FIXED);
    off[n++] = pico8_value_string("x");
    off[n++] = zero_union(PICO8_VALUE_STRING);
    off[n++] = zero_union(PICO8_VALUE_FUNCTION);
    off[n++] = vec;
    for (i = 0; i < 2; i++) {
        must_abort_all(holders[i], "p_", write_off_row, off, n);
    }
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
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_ey_on);
    in_fresh_child(test_ey_off);
    in_fresh_child(test_ey_void);
    in_fresh_child(test_fv);
    in_fresh_child(test_g__is_a_struct);
    in_fresh_child(test_g__members_by_name);
    in_fresh_child(test_g__writer_stops_off_row);
    in_fresh_child(test_bv_et_is_a_struct);
    in_fresh_child(test_bv_et_members_by_name);
    in_fresh_child(test_bv_et_writer_stops_off_row);

    printf("test_pico8_g_: all tests passed\n");
    return 0;
}
