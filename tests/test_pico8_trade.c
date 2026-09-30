/* The export route `trade`, built by `jo:ib` and `kg` in a game built
 * directly (no events file), or by hand on that game's planets and
 * slipways, read on its struct members and through `to`/`kg`/`eq`. */

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
#include "../src/pico8_economy_tiers.h"
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
#include "../src/pico8_mem.h"
#include "../src/pico8_menu.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_trade.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_vec.h"

/* `gb[3]`: a project with an `o` and no `i`, whose planet exports to any
 * planet a slipway joins it to. */
#define EXPORTER 3

#define GAME_ROUTE_HASH 0x93A4DA45u

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

static int ents_rank_in(const pico8_ents_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static pico8_trade_t *as_trade(void *e) {
    assert(e != NULL && ((const pico8_struct_header_t *)e)->cls == pico8_trade_struct_class());
    return e;
}

static pico8_jo_t *as_jo(void *e) {
    assert(pico8_jo_is_family(e));
    return e;
}

/* Both `qe` set, of one kind and one value. */
static int same_qe(const pico8_trade_t *a, const pico8_trade_t *b) {
    return a->has_qe && b->has_qe && a->qe_is_bool == b->qe_is_bool && a->qe == b->qe;
}

/* The kind words the frozen hashes fold before a value: any other value
 * moves them. */
enum { FIXED_KIND = 0, BOOL_KIND = 2, REF_KIND = 3 };

/* The kind word a REF value folded, then presence and the rank in `hv`. */
static uint32_t fold_ref(uint32_t h, const void *p) {
    h = fnv(h, (uint32_t)REF_KIND);
    h = fnv(h, (uint32_t)(p != NULL));
    return fnv(h, (uint32_t)ents_rank_in(pico8_hv(), p));
}

/* A BOOL or FIXED value's kind word and payload; an unset `qe` folds as
 * absent. */
static uint32_t fold_qe(uint32_t h, const pico8_trade_t *t) {
    if (!t->has_qe) {
        return fnv(h, 0xFFFFFFFFu);
    }
    if (t->qe_is_bool) {
        return fnv(fnv(h, (uint32_t)BOOL_KIND), (uint32_t)(t->qe != 0));
    }
    return fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)t->qe);
}

static uint32_t fold_ranks(uint32_t h, const pico8_ents_t *l) {
    int i;
    if (l == NULL) {
        return fnv(h, 0xFFFFFFFFu);
    }
    h = fnv(h, (uint32_t)l->count);
    for (i = 0; i < l->count; i++) {
        h = fold_ref(h, l->items[i]);
    }
    return h;
}

/* A route: the ranks in `hv` of its `lp` planets and `rl` slipways, its
 * `qe`, and the rank of `to()`. */
static uint32_t fold_route(uint32_t h, const pico8_trade_t *t) {
    pico8_struct_header_t *to;
    h = fold_ranks(h, t->lp);
    h = fold_ranks(h, t->rl);
    h = fold_qe(h, t);
    if (pico8_trade_to(t, &to)) {
        return fold_ref(fnv(h, 1u), to);
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

static pico8_vec_t pos_of(void *e) {
    const pico8_vec_t *p = as_jo(e)->p;
    assert(p != NULL);
    return *p;
}

static pico8_fixed_t dist2(pico8_vec_t a, pico8_vec_t b) {
    pico8_vec_t d = pico8_vec_sub(a, b);
    return pico8_fixed_add(pico8_fixed_mul(d.x, d.x), pico8_fixed_mul(d.y, d.y));
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
    pico8_jc_t *jc;

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
}

static pico8_ents_t *list_of(int n, void *const *items) {
    pico8_ents_t *l = malloc(sizeof *l);
    int i;
    pico8_ents_init(l);
    for (i = 0; i < n; i++) {
        pico8_ents_add(l, items[i]);
    }
    return l;
}

/* NULL `lp`/`rl` are nil; `has_qe` 0 is a nil `qe`. */
static pico8_trade_t *route(pico8_ents_t *lp, pico8_ents_t *rl, int has_qe, int qe_is_bool, pico8_fixed_t qe) {
    pico8_trade_keys_t keys = {0};

    keys.lp = lp;
    keys.rl = rl;
    keys.has_qe = has_qe;
    keys.qe_is_bool = qe_is_bool;
    keys.qe = qe;
    return pico8_trade_new(&keys);
}

static void *planet(int i) {
    const pico8_ents_t *jo = pico8_cg("jo");
    assert(jo != NULL && i < jo->count);
    return jo->items[i];
}

/* A generated planet colonized by EXPORTER, its nearest one by a bare
 * project, the two joined by a paid slipway, then left to settle; and the
 * first route: the first element of `by` on the first planet of `cg.jo`
 * that exports. */
static pico8_trade_t *game_route(void) {
    const pico8_ents_t *jo;
    void *qu;
    void *re = NULL;
    int i;

    start_game();
    jo = pico8_cg("jo");
    assert(jo != NULL && jo->count > 3);
    qu = jo->items[0];
    for (i = 1; i < jo->count; i++) {
        void *e = jo->items[i];
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
        const pico8_ents_t *by = as_jo(jo->items[i])->by;
        if (by != NULL && by->count > 0) {
            return as_trade(by->items[0]);
        }
    }
    assert(0 && "no route in the game");
    return NULL;
}

/* A route built from the game's route's own `lp`/`rl`/`qe` reads them back
 * by identity; with no key, or `lp` alone, the others are absent. */
static void test_construction(void) {
    uint32_t h = 2166136261u;
    pico8_trade_t *g = game_route();
    pico8_ents_t *lp = g->lp;
    pico8_ents_t *rl = g->rl;
    pico8_trade_t *t;

    assert(lp != NULL && rl != NULL && g->has_qe);
    h = fold_route(h, g);
    t = route(lp, rl, g->has_qe, g->qe_is_bool, g->qe);
    assert(t != NULL && t != g);
    assert(t->lp == lp && t->rl == rl);
    assert(same_qe(t, g));
    h = fold_route(h, t);

    t = pico8_trade_new(NULL);
    assert(t != NULL && t->lp == NULL && t->rl == NULL && !t->has_qe);
    t = route(lp, NULL, 0, 0, 0);
    assert(t->lp == lp && t->rl == NULL && !t->has_qe);
    check("game route", h, GAME_ROUTE_HASH);
}

/* `to` is the last of `lp`; an empty or missing `lp` gives nothing and
 * leaves the output alone. */
static void test_to(void) {
    static pico8_struct_header_t sentinel_target;
    pico8_struct_header_t *const sentinel = &sentinel_target;
    void *items[3];
    pico8_struct_header_t *out;
    pico8_trade_t *t;

    (void)game_route();
    items[0] = planet(0);
    items[1] = planet(1);
    items[2] = planet(2);
    t = route(list_of(3, items), NULL, 0, 0, 0);
    assert(pico8_trade_to(t, &out) && out == planet(2));
    t = route(list_of(1, items), NULL, 0, 0, 0);
    assert(pico8_trade_to(t, &out) && out == planet(0));

    out = sentinel;
    assert(!pico8_trade_to(route(list_of(0, items), NULL, 0, 0, 0), &out) && out == sentinel);
    assert(!pico8_trade_to(pico8_trade_new(NULL), &out) && out == sentinel);
    assert(!pico8_trade_to(NULL, &out) && out == sentinel);
}

static int list_is(const pico8_ents_t *l, int n, pico8_struct_header_t *const *items) {
    int i;
    if (l == NULL || l->count != n) {
        return 0;
    }
    for (i = 0; i < n; i++) {
        if (l->items[i] != items[i]) {
            return 0;
        }
    }
    return 1;
}

/* `kg` builds a new route and leaves `self` alone: `np` appended to `lp`,
 * `jc` to `rl` unless nil, the same `qe`; `self` with no `qe` gives the new
 * route `qe` false. */
static void test_kg(void) {
    pico8_trade_t *g = game_route();
    const pico8_ents_t *lp = g->lp;
    const pico8_ents_t *rl = g->rl;
    const int lp_n = lp->count;
    const int rl_n = rl->count;
    void *np = planet(3);
    void *jc = rl->items[0];
    pico8_struct_header_t *want[16];
    pico8_trade_t *k;
    int i;

    assert(g->has_qe && lp_n + 1 < 16 && rl_n + 1 < 16);
    k = pico8_trade_kg(g, np, jc);
    assert(k != NULL && k != g);
    assert(g->lp == lp && g->rl == rl && lp->count == lp_n && rl->count == rl_n);
    assert(k->lp != lp && k->rl != rl);
    for (i = 0; i < lp_n; i++) {
        want[i] = lp->items[i];
    }
    want[lp_n] = np;
    assert(list_is(k->lp, lp_n + 1, want));
    for (i = 0; i < rl_n; i++) {
        want[i] = rl->items[i];
    }
    want[rl_n] = jc;
    assert(list_is(k->rl, rl_n + 1, want));
    assert(same_qe(k, g));

    k = pico8_trade_kg(g, np, NULL);
    assert(k->lp->count == lp_n + 1 && list_is(k->rl, rl_n, rl->items));
    assert(k->rl != rl);
    assert(same_qe(k, g));

    k = pico8_trade_kg(g, NULL, NULL);
    assert(list_is(k->lp, lp_n, lp->items) && list_is(k->rl, rl_n, rl->items));

    k = pico8_trade_kg(route((pico8_ents_t *)lp, NULL, 0, 0, 0), np, NULL);
    assert(k->lp->count == lp_n + 1 && k->rl->count == 0);
    assert(k->has_qe && k->qe_is_bool && k->qe == 0);

    k = pico8_trade_kg(pico8_trade_new(NULL), np, jc);
    want[0] = np;
    assert(list_is(k->lp, 1, want));
    want[0] = jc;
    assert(list_is(k->rl, 1, want));
    assert(k->has_qe && k->qe_is_bool && k->qe == 0);

    assert(pico8_trade_kg(NULL, np, jc) == NULL);
}

/* Equal when they start and end at the same planet, whatever the length. */
static void test_eq(void) {
    void *a[2];
    void *b[3];
    void *c[2];
    void *d[2];
    pico8_trade_t *ta;
    pico8_trade_t *tb;

    (void)game_route();
    a[0] = planet(0);
    a[1] = planet(1);
    b[0] = planet(0);
    b[1] = planet(2);
    b[2] = planet(1);
    c[0] = planet(2);
    c[1] = planet(1);
    d[0] = planet(0);
    d[1] = planet(2);
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

static void qe_of_true_route(void *e) {
    (void)e;
    (void)pico8_trade_qe(route(NULL, NULL, 1, 1, 1));
}

static void to_on_other(void *e) {
    pico8_struct_header_t *out;
    (void)pico8_trade_to(e, &out);
}

static void kg_on_other(void *e) {
    (void)pico8_trade_kg(e, NULL, NULL);
}

static void eq_left_other(void *e) {
    (void)pico8_trade_eq(e, pico8_trade_new(NULL));
}

static void eq_right_other(void *e) {
    (void)pico8_trade_eq(pico8_trade_new(NULL), e);
}

/* A game's route is a struct whose descriptor has no method;
 * a `qe` key its member could not hold stops the run, as does reading a true
 * `qe`, and `to`/`kg`/`eq` refuse anything that is not a route. */
static void test_trade_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_trade_struct_class();
    pico8_trade_t *g = game_route();
    pico8_trade_t *t = pico8_trade_new(NULL);
    int j;

    assert(g != NULL && g->header.cls == cls);
    assert(t != NULL && t->header.cls == cls);
    assert(cls->kj == NULL && cls->ev == NULL && cls->hd == NULL && cls->hk == NULL && cls->hb == NULL &&
           cls->ldown == NULL && cls->rdown == NULL && cls->gv == NULL && cls->hq == NULL && cls->ey == NULL &&
           cls->kf == NULL && cls->cn == NULL && cls->ol == NULL && cls->mm == NULL);
    for (j = 0; j < PICO8_STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(pico8_trade_ready());
    assert(pico8_trade_new(NULL) != NULL);

    must_abort(build_with_bool_qe_2, NULL);
    must_abort(build_with_nil_qe_holding_1, NULL);
    must_abort(build_with_nil_qe_as_bool, NULL);
    must_abort(qe_of_true_route, NULL);
    must_abort(to_on_other, pico8_ok());
    must_abort(kg_on_other, pico8_ok());
    must_abort(eq_left_other, pico8_ok());
    must_abort(eq_right_other, pico8_ok());
}

static int g_kf_calls = 0;

static void count_kf(pico8_jo_trade_proposal_t *self, pico8_trade_t *a) {
    (void)self;
    (void)a;
    g_kf_calls++;
}

/* A want of any resource (`r["?"]`) takes the resource of a trade its
 * planet accepts (`ba`), then answers that trade once. */
static void test_cn_wildcard_want(void) {
    pico8_trade_keys_t keys = {0};
    pico8_jo_trade_proposal_t lw = {0};
    pico8_ptrs_t *jy = malloc(sizeof *jy);
    pico8_qh_t *qh;
    pico8_jy_t *want;
    pico8_fixed_t any;
    int any_qe;

    start_game();
    assert(pico8_r_lookup("?", &any_qe));
    any = pico8_fixed_from_int(any_qe);
    want = pico8_jy_new(&any);
    pico8_ptrs_init(jy);
    pico8_ptrs_add(jy, want);
    qh = pico8_qh_new(NULL);
    qh->jo.jy = jy;
    keys.has_qe = 1;
    keys.qe = pico8_fixed_from_int(qh->ba->items[0]);
    lw.t = pico8_trade_new(&keys);
    lw.kf = count_kf;
    pico8_jo_cn(&qh->jo, &lw);
    assert(want->row.has_qe && want->row.qe == keys.qe && g_kf_calls == 1);
}

/* A route through `kg` holds false, not nil: it no longer answers a want
 * with no `qe`, which the cart's nil route would answer -- TROU-24. */
static void test_cn_nil_want_after_kg(void) {
    pico8_jo_trade_proposal_t lw = {0};
    pico8_ptrs_t *jy = malloc(sizeof *jy);
    pico8_qh_t *qh;
    pico8_trade_t *nil_route;

    start_game();
    pico8_ptrs_init(jy);
    pico8_ptrs_add(jy, pico8_jy_new(NULL));
    qh = pico8_qh_new(NULL);
    qh->jo.jy = jy;
    nil_route = pico8_trade_new(&(pico8_trade_keys_t){.lp = list_of(0, NULL)});
    lw.kf = count_kf;
    lw.t = nil_route;
    pico8_jo_cn(&qh->jo, &lw);
    assert(g_kf_calls == 1);
    lw.t = pico8_trade_kg(nil_route, NULL, NULL);
    assert(lw.t->has_qe && lw.t->qe_is_bool && lw.t->qe == 0);
    pico8_jo_cn(&qh->jo, &lw);
    assert(g_kf_calls == 1);
}

/* `dh` counts for a want only the routes whose `qe` equals the want's: a
 * route with no `qe` is skipped by the `has_qe` guard, and one through `kg`
 * holds false, which is not the nil of a want with no `qe` -- TROU-24. */
static void test_dh_counts_numbered_routes_only(void) {
    pico8_ptrs_t *jy = malloc(sizeof *jy);
    pico8_ents_t *cc = malloc(sizeof *cc);
    pico8_qh_t *qh;
    pico8_jy_t *nil_want;
    pico8_jy_t *x_want;
    pico8_fixed_t x = pico8_fixed_from_int(2);
    pico8_trade_t *nil_route;

    start_game();
    nil_want = pico8_jy_new(NULL);
    x_want = pico8_jy_new(&x);
    pico8_ptrs_init(jy);
    pico8_ptrs_add(jy, nil_want);
    pico8_ptrs_add(jy, x_want);
    nil_route = pico8_trade_new(NULL);
    pico8_ents_init(cc);
    pico8_ents_add(cc, &nil_route->header);
    pico8_ents_add(cc, &pico8_trade_kg(nil_route, NULL, NULL)->header);
    pico8_ents_add(cc, &pico8_trade_new(&(pico8_trade_keys_t){.has_qe = 1, .qe = x})->header);
    qh = pico8_qh_new(NULL);
    qh->jo.jy = jy;
    qh->jo.cc = cc;
    pico8_ke_ks(&qh->jo.header, "ly");
    (void)pico8_dh(&qh->jo.header);
    assert(nil_want->has_gc && nil_want->gc == pico8_fixed_from_int(0));
    assert(x_want->has_gc && x_want->gc == pico8_fixed_from_int(1));
}

/* `pico8_qe_equal` is Lua's `==` both ways, `n` unread outside FIXED; and
 * `pico8_trade_qe` reads nil, false (after `kg`) and a number. */
static void test_qe_equal(void) {
    const pico8_qe_t nil = {.kind = PICO8_QE_NIL};
    const pico8_qe_t no = {.kind = PICO8_QE_FALSE};
    const pico8_qe_t n0 = {.kind = PICO8_QE_FIXED, .n = 0};
    const pico8_qe_t n2 = {.kind = PICO8_QE_FIXED, .n = pico8_fixed_from_int(2)};
    const pico8_qe_t n3 = {.kind = PICO8_QE_FIXED, .n = pico8_fixed_from_int(3)};
    pico8_trade_t *nil_route = pico8_trade_new(NULL);
    pico8_trade_t *two = pico8_trade_new(&(pico8_trade_keys_t){.has_qe = 1, .qe = pico8_fixed_from_int(2)});
    pico8_qe_t got;

    assert(pico8_qe_equal(nil, nil) && pico8_qe_equal(no, no) && pico8_qe_equal(n2, n2));
    assert(!pico8_qe_equal(nil, no) && !pico8_qe_equal(no, nil));
    assert(!pico8_qe_equal(n0, nil) && !pico8_qe_equal(nil, n0) && !pico8_qe_equal(n0, no) &&
           !pico8_qe_equal(no, n0));
    assert(!pico8_qe_equal(n2, n3) && !pico8_qe_equal(n3, n2));
    assert(pico8_qe_equal(nil, (pico8_qe_t){.kind = PICO8_QE_NIL, .n = 5}) &&
           pico8_qe_equal((pico8_qe_t){.kind = PICO8_QE_FALSE, .n = 7}, no));

    assert(pico8_trade_qe(NULL).kind == PICO8_QE_NIL);
    assert(pico8_trade_qe(nil_route).kind == PICO8_QE_NIL);
    assert(pico8_trade_qe(pico8_trade_kg(nil_route, NULL, NULL)).kind == PICO8_QE_FALSE);
    got = pico8_trade_qe(two);
    assert(got.kind == PICO8_QE_FIXED && got.n == pico8_fixed_from_int(2));
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
    in_fresh_child(test_cn_wildcard_want);
    in_fresh_child(test_cn_nil_want_after_kg);
    in_fresh_child(test_dh_counts_numbered_routes_only);
    in_fresh_child(test_qe_equal);

    printf("test_pico8_trade: all tests passed\n");
    return 0;
}
