/* The export route `trade`, built by `jo:ib` and `kg` in a game started as
 * the net starts it, or by hand on that game's planets and slipways, read
 * only through pico8_entity_get_field and `to`/`kg`/`eq`. The frozen values
 * were taken on the code where `trade` was still a class instance and must
 * not be re-taken. */

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
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_menu.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_trade.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"

#define BUILT "tests/fixtures/built_planet_panel_events.json"
#define HUB "tests/fixtures/trading_hub_session_events.json"

#define GAME_ROUTE_HASH 0x5C07E005u
#define BUILT_REPLAY_HASH 0x3D21D018u
#define HUB_REPLAY_HASH 0x763C36DBu
#define BUILT_ROUTES 1
#define HUB_ROUTES 5

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

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

static void check_count(const char *label, int got, int want) {
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

static void start_from_title(const char *events) {
    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_cheat_init(&cheat);
}

/* A game started as the net starts it: `eu()` on `bo[2]` before frame 0. */
static void start_game(const char *events) {
    pico8_value_t difficulty;

    start_from_title(events);
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty) && difficulty.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty.as.ref));
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

/* NULL `rl` or nil `qe` leaves that key out. */
static void *route(pico8_list_t *lp, pico8_list_t *rl, pico8_value_t qe) {
    pico8_ob_table_t *o = pico8_ob_table_new();
    void *t;
    if (lp != NULL) {
        pico8_ob_table_set_name(o, "lp", ref(lp));
    }
    if (rl != NULL) {
        pico8_ob_table_set_name(o, "rl", ref(rl));
    }
    if (!(qe.kind == PICO8_VALUE_REF && qe.as.ref == NULL)) {
        pico8_ob_table_set_name(o, "qe", qe);
    }
    t = pico8_trade_new(o);
    pico8_ob_table_free(o);
    return t;
}

static void *planet(int i) {
    const pico8_list_t *jo = pico8_cg("jo");
    assert(jo != NULL && i < jo->count && jo->items[i].kind == PICO8_VALUE_REF);
    return jo->items[i].as.ref;
}

/* The built_planet_panel game at its last frame, and its first route: the
 * first element of `by` on the first planet of `cg.jo` that exports. */
static void *game_route(void) {
    const pico8_list_t *jo;
    int i;

    start_game(BUILT);
    for (i = 0; i < input.frame_count; i++) {
        play_frame(i);
    }
    jo = pico8_cg("jo");
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
    pico8_list_t *lp = get_ref(g, "lp");
    pico8_list_t *rl = get_ref(g, "rl");
    pico8_value_t qe;
    pico8_value_t v;
    void *t;
    size_t i;

    assert(lp != NULL && rl != NULL && lookup(g, "qe", &qe));
    h = fold_route(h, g);
    t = route(lp, rl, qe);
    assert(t != NULL && t != g);
    assert(get_ref(t, "lp") == lp && get_ref(t, "rl") == rl);
    assert(lookup(t, "qe", &v) && v.kind == qe.kind && same_value(v, qe));
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(t, absent[i]) && !has(g, absent[i]));
    }
    h = fold_route(h, t);

    t = pico8_trade_new(NULL);
    assert(t != NULL && !has(t, "lp") && !has(t, "rl") && !has(t, "qe"));
    t = route(lp, NULL, nil());
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
    t = route(list_of(3, items), NULL, nil());
    assert(pico8_trade_to(t, &out) && out.kind == PICO8_VALUE_REF && out.as.ref == planet(2));
    t = route(list_of(1, items), NULL, nil());
    assert(pico8_trade_to(t, &out) && out.as.ref == planet(0));

    out = sentinel;
    assert(!pico8_trade_to(route(list_of(0, items), NULL, nil()), &out) && same_value(out, sentinel));
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

    k = pico8_trade_kg(route((pico8_list_t *)lp, NULL, nil()), ref(np), nil());
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
    ta = route(list_of(2, a), NULL, nil());
    tb = route(list_of(3, b), NULL, nil());
    assert(pico8_trade_eq(ta, tb) && pico8_trade_eq(tb, ta) && pico8_trade_eq(ta, ta));
    assert(pico8_trade_eq(ta, route(list_of(2, a), list_of(0, a), pico8_value_bool(1))));
    assert(!pico8_trade_eq(ta, route(list_of(2, c), NULL, nil())));
    assert(!pico8_trade_eq(ta, route(list_of(2, d), NULL, nil())));
    assert(!pico8_trade_eq(ta, route(list_of(0, a), NULL, nil())));
    assert(!pico8_trade_eq(route(list_of(0, a), NULL, nil()), ta));
    assert(!pico8_trade_eq(ta, pico8_trade_new(NULL)) && !pico8_trade_eq(pico8_trade_new(NULL), ta));
    assert(!pico8_trade_eq(ta, NULL) && !pico8_trade_eq(NULL, ta));
}

static const void *g_seen[4096];
static int g_seen_count;

static void see(const void *t) {
    int i;
    for (i = 0; i < g_seen_count; i++) {
        if (g_seen[i] == t) {
            return;
        }
    }
    assert(g_seen_count < (int)(sizeof g_seen / sizeof g_seen[0]));
    g_seen[g_seen_count++] = t;
}

/* Each frame, each planet of `cg.jo` in order: the counts of `by` and `cc`,
 * then each route of `by`. Then the screen. `*routes` is the number of
 * distinct routes seen in a `by`. */
static uint32_t replay(int from_title, const char *events, int *routes) {
    uint32_t h = 2166136261u;
    int i;

    g_seen_count = 0;
    if (from_title) {
        start_from_title(events);
    } else {
        start_game(events);
    }
    for (i = 0; i < input.frame_count; i++) {
        const pico8_list_t *jo;
        int j;
        play_frame(i);
        jo = pico8_cg("jo");
        for (j = 0; jo != NULL && j < jo->count; j++) {
            const void *p = jo->items[j].as.ref;
            const pico8_list_t *by = get_list(p, "by");
            const pico8_list_t *cc = get_list(p, "cc");
            int k;
            h = fnv(h, (uint32_t)(by != NULL ? by->count : -1));
            h = fnv(h, (uint32_t)(cc != NULL ? cc->count : -1));
            for (k = 0; by != NULL && k < by->count; k++) {
                see(by->items[k].as.ref);
                h = fold_route(h, by->items[k].as.ref);
            }
        }
    }
    *routes = g_seen_count;
    return fnv(h, screen_hash());
}

static void test_built_replay(void) {
    int routes;
    check("built replay", replay(0, BUILT, &routes), BUILT_REPLAY_HASH);
    check_count("built routes", routes, BUILT_ROUTES);
}

static void test_hub_replay(void) {
    int routes;
    check("hub replay", replay(1, HUB, &routes), HUB_REPLAY_HASH);
    check_count("hub routes", routes, HUB_ROUTES);
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

static void build_with_named_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "x", pico8_value_bool(1));
    (void)pico8_trade_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_bool(1));
    (void)pico8_trade_new(b);
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
 * it takes no key but lp, rl, qe, and `to`/`kg`/`eq` refuse anything that
 * is not a route. */
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
    must_abort(build_with_named_key, NULL);
    must_abort(build_with_positional_key, NULL);
    must_abort(to_on_other, pico8_ok());
    must_abort(kg_on_other, pico8_ok());
    must_abort(eq_left_other, pico8_ok());
    must_abort(eq_right_other, pico8_ok());
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
    in_fresh_child(test_built_replay);
    in_fresh_child(test_hub_replay);
    in_fresh_child(test_trade_is_a_struct);

    printf("test_pico8_trade: all tests passed\n");
    return 0;
}
