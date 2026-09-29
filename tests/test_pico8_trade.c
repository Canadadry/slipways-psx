/* The export route `trade`, built by `jo:ib` and `kg` in a game built
 * directly (no events file), or by hand on that game's planets and
 * slipways, read only through pico8_entity_get_field and `to`/`kg`/`eq`. */

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
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_trade.h"
#include "../src/pico8_research.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_menu.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_trade.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

/* `gb[3]`: a project with an `o` and no `i`, whose planet exports to any
 * planet a slipway joins it to. */
#define EXPORTER 3

#define GAME_ROUTE_HASH 0x93A4DA45u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static const char *const absent[] = {"to", "kg", "me", "nu", "id"};

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
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

/* The instance reader keeps a field written nil (TROU-9), the struct reader
 * reads a nil member as absent: nil and absent are one state here. */
static int lookup(const void *e, const char *name, pico8_value_t *out) {
    return pico8_entity_get_field(e, name, out) && !(out->kind == PICO8_VALUE_REF && out->as.ref == NULL);
}

static int has(const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v);
}

static void *get_ref(const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v) && v.kind == PICO8_VALUE_REF ? v.as.ref : NULL;
}

static const pico8_list_t *get_list(const void *e, const char *name) {
    const pico8_list_t *l = get_ref(e, name);
    assert(l == NULL || pico8_type_tag_of(l) == PICO8_TYPE_TAG_LIST);
    return l;
}

static int same_value(pico8_value_t a, pico8_value_t b) {
    return pico8_value_equal(a, b);
}

/* Kind, then the string's bytes, the number, the boolean or the rank in
 * `hv` of what the value holds. */
static uint32_t fold_value(uint32_t h, pico8_value_t v) {
    const char *s;
    h = fnv(h, (uint32_t)v.kind);
    switch (v.kind) {
    case PICO8_VALUE_FIXED:
        return fnv(h, (uint32_t)v.as.fixed);
    case PICO8_VALUE_BOOL:
        return fnv(h, (uint32_t)v.as.boolean);
    case PICO8_VALUE_STRING:
        for (s = v.as.string; *s != '\0'; s++) {
            h = fnv(h, (uint32_t)(unsigned char)*s);
        }
        return h;
    case PICO8_VALUE_REF:
        h = fnv(h, (uint32_t)(v.as.ref != NULL));
        return fnv(h, (uint32_t)rank_in(pico8_hv(), v.as.ref));
    default:
        return h;
    }
}

static uint32_t fold_field(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    if (!lookup(e, name, &v)) {
        return fnv(h, 0xFFFFFFFFu);
    }
    return fold_value(h, v);
}

static uint32_t fold_ranks(uint32_t h, const pico8_list_t *l) {
    int i;
    if (l == NULL) {
        return fnv(h, 0xFFFFFFFFu);
    }
    h = fnv(h, (uint32_t)l->count);
    for (i = 0; i < l->count; i++) {
        h = fold_value(h, l->items[i]);
    }
    return h;
}

/* A route: the ranks in `hv` of its `lp` planets and `rl` slipways, its
 * `qe`, and the rank of `to()`. */
static uint32_t fold_route(uint32_t h, const void *t) {
    pico8_value_t to;
    h = fold_ranks(h, get_list(t, "lp"));
    h = fold_ranks(h, get_list(t, "rl"));
    h = fold_field(h, t, "qe");
    if (pico8_trade_to((void *)t, &to)) {
        return fold_value(fnv(h, 1u), to);
    }
    return fnv(h, 0u);
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

static pico8_fixed_t must_fixed(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static pico8_hp_t *project(int k) {
    assert(pico8_gb_at(k) != NULL);
    return pico8_gb_at(k);
}

/* `jc:hq` reads the hovered planet off `ok.o_`; the slipway is paid by `ey`
 * as a drag onto `re` pays it. */
static void join(void *qu, void *re) {
    pico8_jc_keys_t jc_keys = {0};
    pico8_vec_t *mp = malloc(sizeof *mp);
    pico8_value_t out;
    void *jc;

    jc_keys.qu = qu;
    jc = pico8_jc_new(&jc_keys);
    *mp = pico8_vec_add(pico8_vec_mul(pos_of(re), pico8_fixed_from_int(128)),
                        pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(1)));
    pico8_ok()->o_ = re;
    assert(pico8_mv(jc, "hq", 1, pico8_value_ref(mp), pico8_value_ref(NULL), &out));
    pico8_ok()->o_ = NULL;
    while (must_fixed(jc, "t") < pico8_fixed_from_int(15)) {
        pico8_dg();
    }
    assert(pico8_mv(jc, "ey", 1, pico8_value_ref(re), pico8_value_ref(NULL), &out));
}

static pico8_value_t nil(void) {
    return pico8_value_ref(NULL);
}

static pico8_value_t ref(const void *p) {
    return pico8_value_ref((void *)p);
}

static pico8_list_t *list_of(int n, const pico8_value_t *items) {
    pico8_list_t *l = malloc(sizeof *l);
    int i;
    pico8_list_init(l);
    for (i = 0; i < n; i++) {
        pico8_list_add(l, items[i]);
    }
    return l;
}

/* NULL `lp`/`rl` are nil; `has_qe` 0 is a nil `qe`. */
static void *route(pico8_list_t *lp, pico8_list_t *rl, int has_qe, int qe_is_bool, pico8_fixed_t qe) {
    pico8_trade_keys_t keys = {0};

    keys.lp = lp;
    keys.rl = rl;
    keys.has_qe = has_qe;
    keys.qe_is_bool = qe_is_bool;
    keys.qe = qe;
    return pico8_trade_new(&keys);
}

static void *planet(int i) {
    const pico8_list_t *jo = pico8_cg("jo");
    assert(jo != NULL && i < jo->count && jo->items[i].kind == PICO8_VALUE_REF);
    return jo->items[i].as.ref;
}

/* A generated planet colonized by EXPORTER, its nearest one by a bare
 * project, the two joined by a paid slipway, then left to settle; and the
 * first route: the first element of `by` on the first planet of `cg.jo`
 * that exports. */
static void *game_route(void) {
    const pico8_list_t *jo;
    void *qu;
    void *re = NULL;
    int i;

    start_game();
    jo = pico8_cg("jo");
    assert(jo != NULL && jo->count > 3);
    qu = jo->items[0].as.ref;
    for (i = 1; i < jo->count; i++) {
        void *e = jo->items[i].as.ref;
        if (re == NULL || dist2(pos_of(e), pos_of(qu)) < dist2(pos_of(re), pos_of(qu))) {
            re = e;
        }
    }
    pico8_jo_hu(qu, project(EXPORTER));
    pico8_jo_hu(re, pico8_hp_new(NULL));
    join(qu, re);
    for (i = 0; i < 120; i++) {
        pico8_dg();
    }
    for (i = 0; jo != NULL && i < jo->count; i++) {
        const pico8_list_t *by = get_list(jo->items[i].as.ref, "by");
        if (by != NULL && by->count > 0) {
            return by->items[0].as.ref;
        }
    }
    assert(0 && "no route in the game");
    return NULL;
}

/* A route built from the game's route's own `lp`/`rl`/`qe` reads them back
 * by identity; with no key, or `lp` alone, the others are absent. */
static void test_construction(void) {
    uint32_t h = 2166136261u;
    void *g = game_route();
    const pico8_trade_t *g_route = g;
    pico8_list_t *lp = get_ref(g, "lp");
    pico8_list_t *rl = get_ref(g, "rl");
    pico8_value_t qe;
    pico8_value_t v;
    void *t;
    size_t i;

    assert(lp != NULL && rl != NULL && lookup(g, "qe", &qe));
    h = fold_route(h, g);
    t = route(lp, rl, g_route->has_qe, g_route->qe_is_bool, g_route->qe);
    assert(t != NULL && t != g);
    assert(get_ref(t, "lp") == lp && get_ref(t, "rl") == rl);
    assert(lookup(t, "qe", &v) && v.kind == qe.kind && same_value(v, qe));
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(t, absent[i]) && !has(g, absent[i]));
    }
    h = fold_route(h, t);

    t = pico8_trade_new(NULL);
    assert(t != NULL && !has(t, "lp") && !has(t, "rl") && !has(t, "qe"));
    t = route(lp, NULL, 0, 0, 0);
    assert(get_ref(t, "lp") == lp && !has(t, "rl") && !has(t, "qe"));
    check("game route", h, GAME_ROUTE_HASH);
}

/* `to` is the last of `lp`; an empty or missing `lp` gives nothing and
 * leaves the output alone. */
static void test_to(void) {
    pico8_value_t items[3];
    pico8_value_t out;
    const pico8_value_t sentinel = pico8_value_fixed(pico8_fixed_from_int(7));
    void *t;

    (void)game_route();
    items[0] = ref(planet(0));
    items[1] = ref(planet(1));
    items[2] = ref(planet(2));
    t = route(list_of(3, items), NULL, 0, 0, 0);
    assert(pico8_trade_to(t, &out) && out.kind == PICO8_VALUE_REF && out.as.ref == planet(2));
    t = route(list_of(1, items), NULL, 0, 0, 0);
    assert(pico8_trade_to(t, &out) && out.as.ref == planet(0));

    out = sentinel;
    assert(!pico8_trade_to(route(list_of(0, items), NULL, 0, 0, 0), &out) && same_value(out, sentinel));
    assert(!pico8_trade_to(pico8_trade_new(NULL), &out) && same_value(out, sentinel));
    assert(!pico8_trade_to(NULL, &out) && same_value(out, sentinel));
}

static int list_is(const pico8_list_t *l, int n, const pico8_value_t *items) {
    int i;
    if (l == NULL || l->count != n) {
        return 0;
    }
    for (i = 0; i < n; i++) {
        if (!same_value(l->items[i], items[i]) || l->items[i].kind != items[i].kind) {
            return 0;
        }
    }
    return 1;
}

/* `kg` builds a new route and leaves `self` alone: `np` appended to `lp`,
 * `jc` to `rl` unless nil, the same `qe`; `self` with no `qe` gives the new
 * route `qe` false. */
static void test_kg(void) {
    void *g = game_route();
    const pico8_list_t *lp = get_list(g, "lp");
    const pico8_list_t *rl = get_list(g, "rl");
    const int lp_n = lp->count;
    const int rl_n = rl->count;
    void *np = planet(3);
    void *jc = rl->items[0].as.ref;
    pico8_value_t want[16];
    pico8_value_t qe;
    pico8_value_t v;
    void *k;
    int i;

    assert(lookup(g, "qe", &qe) && lp_n + 1 < 16 && rl_n + 1 < 16);
    k = pico8_trade_kg(g, ref(np), ref(jc));
    assert(k != NULL && k != g);
    assert(get_list(g, "lp") == lp && get_list(g, "rl") == rl && lp->count == lp_n && rl->count == rl_n);
    assert(get_list(k, "lp") != lp && get_list(k, "rl") != rl);
    for (i = 0; i < lp_n; i++) {
        want[i] = lp->items[i];
    }
    want[lp_n] = ref(np);
    assert(list_is(get_list(k, "lp"), lp_n + 1, want));
    for (i = 0; i < rl_n; i++) {
        want[i] = rl->items[i];
    }
    want[rl_n] = ref(jc);
    assert(list_is(get_list(k, "rl"), rl_n + 1, want));
    assert(lookup(k, "qe", &v) && v.kind == qe.kind && same_value(v, qe));

    k = pico8_trade_kg(g, ref(np), nil());
    assert(get_list(k, "lp")->count == lp_n + 1 && list_is(get_list(k, "rl"), rl_n, rl->items));
    assert(get_list(k, "rl") != rl);
    assert(lookup(k, "qe", &v) && same_value(v, qe));

    k = pico8_trade_kg(g, nil(), nil());
    assert(list_is(get_list(k, "lp"), lp_n, lp->items) && list_is(get_list(k, "rl"), rl_n, rl->items));

    k = pico8_trade_kg(route((pico8_list_t *)lp, NULL, 0, 0, 0), ref(np), nil());
    assert(get_list(k, "lp")->count == lp_n + 1 && get_list(k, "rl")->count == 0);
    assert(lookup(k, "qe", &v) && v.kind == PICO8_VALUE_BOOL && !v.as.boolean);

    k = pico8_trade_kg(pico8_trade_new(NULL), ref(np), ref(jc));
    want[0] = ref(np);
    assert(list_is(get_list(k, "lp"), 1, want));
    want[0] = ref(jc);
    assert(list_is(get_list(k, "rl"), 1, want));
    assert(lookup(k, "qe", &v) && v.kind == PICO8_VALUE_BOOL && !v.as.boolean);

    assert(pico8_trade_kg(NULL, ref(np), ref(jc)) == NULL);
}

/* Equal when they start and end at the same planet, whatever the length. */
static void test_eq(void) {
    pico8_value_t a[2];
    pico8_value_t b[3];
    pico8_value_t c[2];
    pico8_value_t d[2];
    void *ta;
    void *tb;

    (void)game_route();
    a[0] = ref(planet(0));
    a[1] = ref(planet(1));
    b[0] = ref(planet(0));
    b[1] = ref(planet(2));
    b[2] = ref(planet(1));
    c[0] = ref(planet(2));
    c[1] = ref(planet(1));
    d[0] = ref(planet(0));
    d[1] = ref(planet(2));
    ta = route(list_of(2, a), NULL, 0, 0, 0);
    tb = route(list_of(3, b), NULL, 0, 0, 0);
    assert(pico8_trade_eq(ta, tb) && pico8_trade_eq(tb, ta) && pico8_trade_eq(ta, ta));
    assert(pico8_trade_eq(ta, route(list_of(2, a), list_of(0, a), 1, 1, 1)));
    assert(!pico8_trade_eq(ta, route(list_of(2, c), NULL, 0, 0, 0)));
    assert(!pico8_trade_eq(ta, route(list_of(2, d), NULL, 0, 0, 0)));
    assert(!pico8_trade_eq(ta, route(list_of(0, a), NULL, 0, 0, 0)));
    assert(!pico8_trade_eq(route(list_of(0, a), NULL, 0, 0, 0), ta));
    assert(!pico8_trade_eq(ta, pico8_trade_new(NULL)) && !pico8_trade_eq(pico8_trade_new(NULL), ta));
    assert(!pico8_trade_eq(ta, NULL) && !pico8_trade_eq(NULL, ta));
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

static void write_qe(void *e) {
    pico8_entity_set_field(e, "qe", pico8_value_bool(1));
}

static void build_with_bool_qe_2(void *e) {
    (void)e;
    (void)route(NULL, NULL, 1, 1, 2);
}

static void build_with_nil_qe_holding_1(void *e) {
    (void)e;
    (void)route(NULL, NULL, 0, 0, 1);
}

static void build_with_nil_qe_as_bool(void *e) {
    (void)e;
    (void)route(NULL, NULL, 0, 1, 0);
}

static void to_on_other(void *e) {
    pico8_value_t out;
    (void)pico8_trade_to(e, &out);
}

static void kg_on_other(void *e) {
    (void)pico8_trade_kg(e, nil(), nil());
}

static void eq_left_other(void *e) {
    (void)pico8_trade_eq(e, pico8_trade_new(NULL));
}

static void eq_right_other(void *e) {
    (void)pico8_trade_eq(pico8_trade_new(NULL), e);
}

/* A game's route is a struct whose descriptor has no method and no writer;
 * a `qe` key its member could not hold stops the run, and `to`/`kg`/`eq`
 * refuse anything that is not a route. */
static void test_trade_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_trade_struct_class();
    pico8_trade_t *g = game_route();
    pico8_trade_t *t = pico8_trade_new(NULL);
    int j;

    assert(pico8_type_tag_of(g) == PICO8_TYPE_TAG_STRUCT && g->header.cls == cls);
    assert(pico8_type_tag_of(t) == PICO8_TYPE_TAG_STRUCT && t->header.cls == cls);
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        assert(cls->methods[j] == NULL);
    }
    assert(cls->field != NULL && cls->set_field == NULL);
    assert(pico8_trade_ready());
    assert(pico8_trade_new(NULL) != NULL);

    must_abort(write_qe, g);
    must_abort(build_with_bool_qe_2, NULL);
    must_abort(build_with_nil_qe_holding_1, NULL);
    must_abort(build_with_nil_qe_as_bool, NULL);
    must_abort(to_on_other, pico8_ok());
    must_abort(kg_on_other, pico8_ok());
    must_abort(eq_left_other, pico8_ok());
    must_abort(eq_right_other, pico8_ok());
}

static pico8_trade_t *trade_of(pico8_list_t *lp, pico8_list_t *rl, int has_qe, int qe_is_bool, pico8_fixed_t qe) {
    pico8_trade_keys_t keys = {0};

    keys.lp = lp;
    keys.rl = rl;
    keys.has_qe = has_qe;
    keys.qe_is_bool = qe_is_bool;
    keys.qe = qe;
    return pico8_trade_new(&keys);
}

static void trade_misses(const pico8_trade_t *t, const char *name) {
    const pico8_struct_class_t *cls = pico8_trade_struct_class();
    pico8_value_t out = pico8_value_fixed(0x1234);

    assert(cls->field(t, name, &out) == 0);
    assert(pico8_entity_get_field(t, name, &out) == 0);
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == 0x1234);
}

static void trade_qe_is(const pico8_trade_t *t, pico8_value_t want) {
    pico8_value_t out = pico8_value_ref(NULL);

    assert(pico8_entity_get_field(t, "qe", &out) == 1);
    assert(out.kind == want.kind);
    if (want.kind == PICO8_VALUE_FIXED) {
        assert(out.as.fixed == want.as.fixed);
    } else {
        assert(out.as.boolean == want.as.boolean);
    }
}

static void trade_list_is(const pico8_trade_t *t, const char *name, const pico8_list_t *want) {
    pico8_value_t out = pico8_value_ref(NULL);

    assert(pico8_entity_get_field(t, name, &out) == 1);
    assert(out.kind == PICO8_VALUE_REF && out.as.ref == want);
    assert(pico8_type_tag_of(out.as.ref) == PICO8_TYPE_TAG_LIST);
}

static void test_trade_members_by_name(void) {
    pico8_trade_t *g = game_route();
    pico8_list_t *lp = list_of(0, NULL);
    pico8_list_t *rl = list_of(0, NULL);
    pico8_value_t v;
    pico8_trade_t *t;

    assert(pico8_entity_get_field(g, "lp", &v) == 1 && v.kind == PICO8_VALUE_REF);
    assert(v.as.ref != NULL && pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST);
    assert(pico8_entity_get_field(g, "rl", &v) == 1 && v.kind == PICO8_VALUE_REF);
    assert(v.as.ref != NULL && pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST);
    assert(pico8_entity_get_field(g, "qe", &v) == 1 && v.kind == PICO8_VALUE_FIXED);
    trade_misses(g, "to");
    trade_misses(g, "x");
    trade_misses(g, "");

    t = trade_of(lp, rl, 1, 0, pico8_fixed_from_int(14));
    trade_list_is(t, "lp", lp);
    trade_list_is(t, "rl", rl);
    trade_qe_is(t, pico8_value_fixed(pico8_fixed_from_int(14)));
    trade_qe_is(trade_of(lp, rl, 1, 0, 0), pico8_value_fixed(0));
    trade_qe_is(trade_of(lp, rl, 1, 1, 1), pico8_value_bool(1));
    trade_qe_is(trade_of(lp, rl, 1, 1, 0), pico8_value_bool(0));
    trade_misses(trade_of(lp, rl, 0, 0, 0), "qe");
    trade_misses(trade_of(lp, rl, 0, 0, 0), "qe");

    t = trade_of(NULL, NULL, 0, 0, 0);
    trade_misses(t, "lp");
    trade_misses(t, "rl");
    trade_misses(t, "qe");
    trade_misses(t, "to");
    trade_misses(t, "x");
    trade_misses(t, "");
    t = pico8_trade_new(NULL);
    trade_misses(t, "lp");
    trade_misses(t, "rl");
    trade_misses(t, "qe");

    trade_qe_is(pico8_trade_kg(trade_of(lp, rl, 1, 0, 0), nil(), nil()), pico8_value_fixed(0));
    trade_qe_is(pico8_trade_kg(trade_of(lp, rl, 1, 1, 1), nil(), nil()), pico8_value_bool(1));
    trade_qe_is(pico8_trade_kg(trade_of(lp, rl, 0, 0, 0), nil(), nil()), pico8_value_bool(0));
    trade_qe_is(pico8_trade_kg(trade_of(NULL, NULL, 0, 0, 0), nil(), nil()), pico8_value_bool(0));
}

static int g_kf_calls = 0;

static void count_kf(pico8_jo_trade_proposal_t *self, pico8_value_t a) {
    (void)self;
    (void)a;
    g_kf_calls++;
}

/* A want of any resource (`r["?"]`) takes the resource of a trade its
 * planet accepts (`ba`), then answers that trade once. */
static void test_cn_wildcard_want(void) {
    pico8_trade_keys_t keys = {0};
    pico8_jo_trade_proposal_t lw = {0};
    pico8_list_t *jy = malloc(sizeof *jy);
    pico8_qh_t *qh;
    pico8_jy_t *want;
    pico8_fixed_t any;
    int any_qe;

    start_game();
    assert(pico8_r_lookup("?", &any_qe));
    any = pico8_fixed_from_int(any_qe);
    want = pico8_jy_new(&any);
    pico8_list_init(jy);
    pico8_list_add(jy, pico8_value_ref(want));
    qh = pico8_qh_new(NULL);
    qh->jo.jy = jy;
    keys.has_qe = 1;
    keys.qe = pico8_fixed_from_int(qh->ba->items[0]);
    lw.t = pico8_trade_new(&keys);
    lw.kf = count_kf;
    pico8_jo_cn(&qh->jo, &lw);
    assert(want->row.has_qe && want->row.qe == keys.qe && g_kf_calls == 1);
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
    in_fresh_child(test_construction);
    in_fresh_child(test_to);
    in_fresh_child(test_kg);
    in_fresh_child(test_eq);
    in_fresh_child(test_trade_is_a_struct);
    in_fresh_child(test_trade_members_by_name);
    in_fresh_child(test_cn_wildcard_want);

    printf("test_pico8_trade: all tests passed\n");
    return 0;
}
