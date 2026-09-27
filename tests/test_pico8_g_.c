/* The probe `g_`, read and written only through the dual-form field API and
 * `pico8_g__default(...)` for spec defaults, built by
 * its three roads (`bg:ldown`, `pico8_g__new(NULL)`, `pico8_g__new` with the
 * 4 keys of probe_mm_rnd_order) and driven through the real `mv`/`dc`/`dg`.
 * Its methods are compared to the public functions. Its daughter `fv` is
 * read on the title screen. The frozen values were taken on the code where
 * `g_` was still a class instance and must not be re-taken. */

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
#include "../src/pico8_cheat.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_particle.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define DRAG "tests/fixtures/slipway_drag_events.json"

/* The tag class instances carried, which the frozen hashes fold: any
 * other value moves every *_HASH. */
#define FROZEN_INSTANCE_TAG 0x9E5C1451u

#define BUILT_HASH 0xD5E726A8u
#define LIFE_HASH 0x26179978u
#define EY_ON_HASH 0x7D81D210u
#define EY_OFF_HASH 0x196FE04Du
#define DRAG_HASH 0xBF801C2Fu
#define DRAG_PROBES 2
#define FV_HASH 0xE9230D69u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

/* g_'s 18 spec defaults, in spec order. */
static const char *const defaults[] = {"nu", "fx", "dq", "od", "fp", "fa", "dw", "py", "ki",
                                       "pn", "pk", "fc", "k_", "qo", "ro", "qg", "q_", "du"};
static const char *const scalars[] = {"fx", "dq", "od", "fp", "fa", "dw", "py", "ki", "pn",
                                      "pk", "fc", "qo", "ro", "qg", "q_", "du", "me", "t"};
static const char *const not_yet[] = {"ra", "p", "et", "p_", "fo", "m_", "ma", "hd",
                                      "hk", "hb", "ik", "ly", "ph"};
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

static pico8_value_t class_value(int (*default_of)(const char *, pico8_value_t *), const char *name) {
    pico8_value_t v;
    assert(default_of(name, &v));
    return v;
}

static void *class_ref(int (*default_of)(const char *, pico8_value_t *), const char *name) {
    pico8_value_t v = class_value(default_of, name);
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
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

/* A table's entries, in order. */
static uint32_t fold_table(uint32_t h, const pico8_ob_table_t *t) {
    int i;

    assert(pico8_type_tag_of(t) == PICO8_TYPE_TAG_OB_TABLE);
    for (i = 0; i < t->count; i++) {
        const pico8_ob_key_t *key = &t->entries[i].key;
        h = key->kind == PICO8_OB_KEY_NAME ? fnv_str(h, key->name) : fnv(h, (uint32_t)key->index);
        h = fold_value(h, t->entries[i].value);
    }
    return fnv(h, (uint32_t)t->count);
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

static void check_int(const char *label, int got, int want) {
    printf("[%s] %d\n", label, got);
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

static const pico8_ob_table_t *bo_row(int i) {
    pico8_value_t row;
    assert(pico8_ob_table_get_index(pico8_bo(), i, &row) && row.kind == PICO8_VALUE_REF);
    return row.as.ref;
}

static void start_game(const char *events) {
    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_cheat_init(&cheat);
    pico8_eu(pico8_js_new(bo_row(2)));
}

static void play_frame(int i) {
    pico8_dispatch_set_input(&input);
    pico8_cheat_step(&cheat, &input);
    pico8__update60();
    pico8__draw();
    if (i < input.frame_count - 1) {
        pico8_input_step(&input);
    }
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
    return e != NULL && get_ref(e, "nu") == class_ref(pico8_g__default, "nu") && lookup(e, "fa", &fa) &&
           fa.kind == PICO8_VALUE_FIXED && fa.as.fixed == pico8_fixed_from_int(2);
}

/* Road (a): `bg:ldown` while cg.g_ is empty. */
static void *ldown(void) {
    pico8_value_t out;

    assert(list_count(pico8_cg("g_")) == 0);
    out = call(the_bg(), "ldown", 2, vec_ref(vec_xy(1, 1)), vec_ref(vec_xy(2, 2)));
    assert(out.kind == PICO8_VALUE_REF && out.as.ref != NULL && is_probe(out.as.ref));
    return out.as.ref;
}

/* Road (c): the 4 keys of probe_mm_rnd_order. */
static void *keyed(void) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    void *made;

    pico8_ob_table_set_name(b, "pn", pico8_value_fixed(pico8_fixed_from_int(7)));
    pico8_ob_table_set_name(b, "t", pico8_value_fixed(pico8_fixed_from_int(3)));
    pico8_ob_table_set_name(b, "du", pico8_value_fixed(pico8_fixed_from_int(10)));
    pico8_ob_table_set_name(b, "py", pico8_value_fixed(pico8_fixed_from_int(1)));
    made = pico8_g__new(b);
    pico8_ob_table_free(b);
    return made;
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
    h = fold_table(h, get_ref(e, "nu"));
    h = fold_table(h, get_ref(e, "k_"));
    assert(get_ref(e, "nu") == class_ref(pico8_g__default, "nu"));
    assert(get_ref(e, "k_") == class_ref(pico8_g__default, "k_"));
    h = fnv(h, (uint32_t)size_of(e, "qy"));
    assert(size_of(e, "qy") == 0);
    for (i = 0; i < sizeof not_yet / sizeof not_yet[0]; i++) {
        assert(!has(e, not_yet[i]));
    }
    assert(method(e, "hq") == pico8_bv_dispatch);
    assert(method(e, "ey") == pico8_d__dispatch);
    assert(method(e, "ev") == pico8_dl_dispatch);
    assert(method(e, "kj") == pico8_ei_dispatch);
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

    start_game(DRAG);
    a = ldown();
    h = fold_probe(h, a);
    b = pico8_g__new(NULL);
    h = fold_probe(h, b);
    c = keyed();
    h = fold_probe(h, c);
    assert(a != b && b != c && a != c);
    assert(get_ref(a, "qy") != get_ref(b, "qy") && get_ref(b, "qy") != get_ref(c, "qy"));
    assert(get(a, "fa").as.fixed == pico8_fixed_from_int(2) && get(a, "ki").as.fixed == pico8_fixed_from_int(9));
    assert(get(a, "dw").as.fixed == class_value(pico8_g__default, "dw").as.fixed);
    assert(get(c, "pn").as.fixed == pico8_fixed_from_int(7) && get(c, "t").as.fixed == pico8_fixed_from_int(3) &&
           get(c, "du").as.fixed == pico8_fixed_from_int(10) && get(c, "py").as.fixed == pico8_fixed_from_int(1));
    assert(get(b, "pn").as.fixed == pico8_fixed_from_int(3) && get(b, "t").as.fixed == pico8_fixed_from_int(0));
    check("built", h, BUILT_HASH);
}

static uint32_t fold_particle(uint32_t h, pico8_value_t pt) {
    static const char *const vecs[] = {"p", "v"};
    pico8_value_t v;
    size_t i;

    assert(pt.kind == PICO8_VALUE_REF && pt.as.ref != NULL && pico8_type_tag_of(pt.as.ref) == PICO8_TYPE_TAG_OB_TABLE);
    for (i = 0; i < sizeof vecs / sizeof vecs[0]; i++) {
        const pico8_vec_t *p;
        assert(pico8_ob_table_get_name(pt.as.ref, vecs[i], &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
        p = v.as.ref;
        h = fnv(fnv(h, (uint32_t)p->x), (uint32_t)p->y);
    }
    assert(pico8_ob_table_get_name(pt.as.ref, "l", &v) && v.kind == PICO8_VALUE_FIXED);
    return fnv(h, (uint32_t)v.as.fixed);
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
    pico8_ob_table_t *b;
    void *qu;
    void *re;
    void *jc;
    pico8_vec_t base = vec_of(a_planet(), "p");
    pico8_vec_t qu_p = pico8_vec_add(base, vec_xy(0, 1));
    pico8_vec_t re_p = pico8_vec_add(base, vec_xy(1, 1));

    b = pico8_ob_table_new();
    pico8_ob_table_set_name(b, "p", vec_ref(qu_p));
    pico8_ob_table_set_name(b, "id", pico8_value_string("s91"));
    qu = pico8_jo_new(b);
    pico8_ob_table_free(b);
    b = pico8_ob_table_new();
    pico8_ob_table_set_name(b, "p", vec_ref(re_p));
    pico8_ob_table_set_name(b, "id", pico8_value_string("s92"));
    re = pico8_jo_new(b);
    pico8_ob_table_free(b);
    b = pico8_ob_table_new();
    pico8_ob_table_set_name(b, "qu", pico8_value_ref(qu));
    jc = pico8_jc_new(b);
    pico8_ob_table_free(b);
    ((pico8_jc_t *)jc)->re = pico8_value_ref(re);
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

    start_game(DRAG);
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
    (void)call(a, "ev", 0, nil(), nil());
    h = fnv(h, screen_hash());
    (void)call(b, "ev", 0, nil(), nil());
    h = fnv(h, screen_hash());
    (void)call(a, "hq", 2, mouse_at(vec_of(a_planet(), "p")),
               vec_ref(vec_xy(64, 64)));
    (void)call(a, "ev", 0, nil(), nil());
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

    start_game(DRAG);
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

    start_game(DRAG);
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

    start_game(DRAG);
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

/* The slipway drag fixture, started as main.c starts it: per frame, each
 * live probe's rank, `me`, `t`, `p_`, `et`'s kind, `qy`'s size and `ra`;
 * each probe's birth and death frames; the last screen. */
static void test_drag(void) {
    uint32_t h = 2166136261u;
    const void *seen[8];
    int birth[8];
    int death[8];
    int probes = 0;
    int i;
    int k;

    start_game(DRAG);
    for (i = 0; i < input.frame_count; i++) {
        const pico8_list_t *hv;
        int j;

        play_frame(i);
        hv = pico8_hv();
        for (k = 0; k < probes; k++) {
            if (death[k] < 0 && !listed(hv, seen[k])) {
                death[k] = i;
                h = fnv(fnv(h, 0xD1Eu), (uint32_t)i);
            }
        }
        for (j = 0; j < hv->count; j++) {
            const void *e = hv->items[j].as.ref;
            if (!is_probe(e)) {
                continue;
            }
            for (k = 0; k < probes && seen[k] != e; k++) {
            }
            if (k == probes) {
                assert(probes < 8);
                seen[probes] = e;
                birth[probes] = i;
                death[probes] = -1;
                probes++;
                h = fnv(fnv(h, 0xB1Eu), (uint32_t)i);
            }
            h = fnv(h, (uint32_t)i);
            h = fnv(h, (uint32_t)j);
            h = fold_field(h, e, "me");
            h = fold_field(h, e, "t");
            h = fold_field(h, e, "p_");
            h = fnv(h, et_kind(e));
            h = fnv(h, (uint32_t)size_of(e, "qy"));
            h = fold_vec(h, e, "ra");
        }
    }
    for (k = 0; k < probes; k++) {
        printf("[drag] probe %d born %d died %d\n", k, birth[k], death[k]);
    }
    h = fnv(h, screen_hash());
    check_int("drag probes", probes, DRAG_PROBES);
    check("drag", h, DRAG_HASH);
}

/* The help: `g_`'s `nu` and `fa`=10. */
static int is_help(const void *e) {
    pico8_value_t fa;
    return e != NULL && get_ref(e, "nu") == class_ref(pico8_g__default, "nu") && lookup(e, "fa", &fa) &&
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
    assert(pico8_input_load(&input, DRAG));
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
    assert(class_ref(pico8_fv_default, "nu") == class_ref(pico8_g__default, "nu"));
    assert(get_ref(fv, "nu") == class_ref(pico8_g__default, "nu"));
    assert(get(fv, "fx").as.fixed == pico8_fixed_from_int(1));
    assert(is_string(fv, "dq", "ly"));
    assert(get(fv, "fa").as.fixed == pico8_fixed_from_int(10));
    assert(get_ref(fv, "k_") == class_ref(pico8_fv_default, "k_"));
    assert(get_ref(fv, "k_") != class_ref(pico8_g__default, "k_"));
    assert(method(fv, "ev") == pico8_dl_dispatch);
    assert(method(fv, "mm") == pico8_g__mm);
    assert(method(fv, "hq") == pico8_bv_dispatch);
    assert(method(fv, "ey") == pico8_d__dispatch);
    assert(method(fv, "kj") != pico8_ei_dispatch);
    assert(method(fv, "kj") == pico8_fv_kj_dispatch);
    assert(method(fv, "hd") != NULL);
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

static void build_with_unknown_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "ki", pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_g__new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_g__new(b);
}

static int is_a_g__struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_g__struct_class();
}

/* Built by any road, a probe is a struct whose descriptor holds the 5
 * public functions of `g_`, which its class no longer installs; it
 * takes exactly `qy`, `ra`, `p`, `et`, `p_`, and a construction key outside
 * the 4 measured stops the run. The title's `fv` is a struct of its own. */
static void test_g__is_a_struct(void) {
    static const char *const written[] = {"qy", "ra", "p", "et", "p_"};
    static const char *const entries[] = {"hq", "ey", "ev", "kj", "mm"};
    static const pico8_value_function_t fns[] = {pico8_bv_dispatch, pico8_d__dispatch, pico8_dl_dispatch,
                                                  pico8_ei_dispatch, pico8_g__mm};
    const pico8_struct_class_t *cls = pico8_g__struct_class();
    const pico8_list_t *hv;
    void *roads[3];
    pico8_value_t v;
    size_t i;
    int j;

    start_game(DRAG);
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
                assert(!pico8_g__default(entries[i], &v));
                named = 1;
            }
        }
        assert(named || cls->methods[j] == NULL);
    }
    assert(cls->methods[PICO8_METHOD_HQ] == pico8_bv_dispatch && cls->methods[PICO8_METHOD_EY] == pico8_d__dispatch &&
           cls->methods[PICO8_METHOD_EV] == pico8_dl_dispatch && cls->methods[PICO8_METHOD_KJ] == pico8_ei_dispatch &&
           cls->methods[PICO8_METHOD_MM] == pico8_g__mm);
    assert(cls->field != NULL && cls->set_field != NULL);
    assert(pico8_g__ready());

    must_abort(write_dw, roads[0]);
    must_abort(write_fx, roads[0]);
    must_abort(write_nu, roads[0]);
    must_abort(write_k_, roads[0]);
    must_abort(write_fo, roads[0]);
    must_abort(write_m_, roads[0]);
    must_abort(write_me, roads[0]);
    must_abort(build_with_unknown_key, NULL);
    must_abort(build_with_positional_key, NULL);
    for (i = 0; i < sizeof written / sizeof written[0]; i++) {
        pico8_entity_set_field(roads[1], written[i], pico8_value_fixed(pico8_fixed_from_int((int)i + 1)));
        assert(lookup(roads[1], written[i], &v) && v.kind == PICO8_VALUE_FIXED &&
               v.as.fixed == pico8_fixed_from_int((int)i + 1));
    }
    ((pico8_g__t *)roads[1])->fa = pico8_value_ref(NULL);
    assert(!cls->field(roads[1], "fa", &v) && !is_probe(roads[1]));

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
    static const char *const absent[] = {"qs", "hq", "ev", "p", "me"};
    void *a;
    void *et;
    pico8_vec_t middle;
    size_t i;

    start_game(DRAG);
    a = ldown();
    (void)a_slipway(&middle);
    (void)call(a, "hq", 2, mouse_at(pico8_vec_add(middle, vec_xy(0, 3))), vec_ref(vec_xy(2, 2)));
    et = get_ref(a, "et");
    assert(is_bv_target(et) && get_ref(et, "fo") == a);
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(et, absent[i]));
    }
    must_abort(write_qs, et);
    pico8_entity_set_field(et, "fo", pico8_value_ref(NULL));
    assert(!has(et, "fo"));
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
    in_fresh_child(test_drag);
    in_fresh_child(test_fv);
    in_fresh_child(test_g__is_a_struct);
    in_fresh_child(test_bv_et_is_a_struct);

    printf("test_pico8_g_: all tests passed\n");
    return 0;
}
