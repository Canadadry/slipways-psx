/* The export route, built by planet_rebuild_trades and trade_extend in a
 * game built directly (no events file), or by hand on that game's planets
 * and slipways, read on its struct members and through trade_last_planet,
 * trade_extend and trade_equal. */

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
#include "game/economy_tiers.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/planet_trade.h"
#include "game/research.h"
#include "game/slipway.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "engine/entity_class.h"
#include "game/trade.h"
#include "engine/game_zone.h"
#include "game/ui.h"
#include "engine/vec.h"
#include "engine/frame_zone.h"
#include "heap_allocator.h"

/* Research project 3: an output and no inputs, so its planet exports to
 * any planet a slipway joins it to. */
#define EXPORTER 3

#define GAME_ROUTE_HASH 0x93A4DA45u

static p8_memory_t memory;
static p8_graphics_t graphics;
static p8_input_t input;

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static int entity_list_rank_in(const entity_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static trade_route_t *as_trade(void *e) {
    assert(e != NULL && ((const entity_t *)e)->class == trade_class());
    return e;
}

static planet_t *as_planet(void *e) {
    assert(planet_is_family(e));
    return e;
}

/* Both resources set, of one kind and one value. */
static int same_resource(const trade_route_t *a, const trade_route_t *b) {
    return a->has_resource && b->has_resource && a->resource_is_bool == b->resource_is_bool && a->resource == b->resource;
}

/* The kind words the frozen hashes fold before a value: any other value
 * moves them. */
enum { FIXED_KIND = 0, BOOL_KIND = 2, REF_KIND = 3 };

/* The kind word of a reference, then presence and the rank in the live
 * entities. */
static uint32_t fold_ref(uint32_t h, const void *p) {
    h = fnv(h, (uint32_t)REF_KIND);
    h = fnv(h, (uint32_t)(p != NULL));
    return fnv(h, (uint32_t)entity_list_rank_in(live_entities(), p));
}

/* A boolean or fixed value's kind word and payload; an unset resource
 * folds as absent. */
static uint32_t fold_resource(uint32_t h, const trade_route_t *t) {
    if (!t->has_resource) {
        return fnv(h, 0xFFFFFFFFu);
    }
    if (t->resource_is_bool) {
        return fnv(fnv(h, (uint32_t)BOOL_KIND), (uint32_t)(t->resource != 0));
    }
    return fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)t->resource);
}

static uint32_t fold_ranks(uint32_t h, const entity_list_t *l) {
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

/* A route: the live entity ranks of its planets and slipways, its
 * resource, and the rank of its last planet. */
static uint32_t fold_route(uint32_t h, const trade_route_t *t) {
    entity_t *to;
    h = fold_ranks(h, t->planets);
    h = fold_ranks(h, t->slipways);
    h = fold_resource(h, t);
    if (trade_last_planet(t, &to)) {
        return fold_ref(fnv(h, 1u), to);
    }
    return fnv(h, 0u);
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

/* A game started by cart_start_game with no difficulty picked: a NULL
 * difficulty reads as difficulty 0. */
static void start_game(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    cart_init();
    cart_start_game(NULL);
}

static vec_t pos_of(void *e) {
    const vec_t *p = as_planet(e)->map_pos;
    assert(p != NULL);
    return *p;
}

static p8_fixed_t dist2(vec_t a, vec_t b) {
    vec_t d = vec_sub(a, b);
    return p8_fixed_add(p8_fixed_mul(d.x, d.x), p8_fixed_mul(d.y, d.y));
}

static research_project_t *project(int k) {
    assert(research_projects_at(k) != NULL);
    return research_projects_at(k);
}

/* slipway_drag reads the hovered planet off the main user interface; the
 * slipway is paid by slipway_drop as a drag onto the end planet pays it. */
static void join(void *qu, void *re) {
    slipway_keys_t slipway_keys = {0};
    vec_t *mp = malloc(sizeof *mp);
    slipway_t *jc;

    slipway_keys.start = qu;
    jc = slipway_new(&slipway_keys);
    *mp = vec_add(vec_mul(pos_of(re), p8_fixed_from_int(128)),
                        vec_make(p8_fixed_from_int(1), p8_fixed_from_int(1)));
    main_ui()->hovered = re;
    slipway_class()->drag(&jc->header, mp, NULL);
    main_ui()->hovered = NULL;
    while (jc->header.state_time < p8_fixed_from_int(15)) {
        update_entities();
    }
    slipway_class()->drop(&jc->header, re);
}

static entity_list_t *list_of(int n, void *const *items) {
    entity_list_t *l = malloc(sizeof *l);
    int i;
    entity_list_init(l, &heap_allocator);
    for (i = 0; i < n; i++) {
        entity_list_add(l, items[i]);
    }
    return l;
}

/* NULL planets or slipways stay NULL; has_resource 0 is no resource. */
static trade_route_t *route(entity_list_t *lp, entity_list_t *rl, int has_resource, int resource_is_bool, p8_fixed_t qe) {
    trade_keys_t keys = {0};

    keys.planets = lp;
    keys.slipways = rl;
    keys.has_resource = has_resource;
    keys.resource_is_bool = resource_is_bool;
    keys.resource = qe;
    return trade_new(game_zone(), &keys);
}

static void *planet(int i) {
    const entity_list_t *jo = entities_by_tag("planet");
    assert(jo != NULL && i < jo->count);
    return jo->items[i];
}

/* A generated planet colonized by EXPORTER, its nearest one by a bare
 * project, the two joined by a paid slipway, then left to settle; and the
 * first route: the first outgoing route of the first planet (planet tag)
 * that exports. */
static trade_route_t *game_route(void) {
    const entity_list_t *jo;
    void *qu;
    void *re = NULL;
    int i;

    start_game();
    jo = entities_by_tag("planet");
    assert(jo != NULL && jo->count > 3);
    qu = jo->items[0];
    for (i = 1; i < jo->count; i++) {
        void *e = jo->items[i];
        if (re == NULL || dist2(pos_of(e), pos_of(qu)) < dist2(pos_of(re), pos_of(qu))) {
            re = e;
        }
    }
    planet_apply_project(qu, project(EXPORTER));
    planet_apply_project(re, research_project_new(NULL));
    join(qu, re);
    for (i = 0; i < 120; i++) {
        update_entities();
    }
    for (i = 0; jo != NULL && i < jo->count; i++) {
        const entity_list_t *by = as_planet(jo->items[i])->outgoing_routes;
        if (by != NULL && by->count > 0) {
            return as_trade(by->items[0]);
        }
    }
    assert(0 && "no route in the game");
    return NULL;
}

/* A route built from the game's route's own planets, slipways and
 * resource reads them back by identity; with no key, or the planets alone,
 * the others are absent. */
static void test_construction(void) {
    uint32_t h = 2166136261u;
    trade_route_t *g = game_route();
    entity_list_t *lp = g->planets;
    entity_list_t *rl = g->slipways;
    trade_route_t *t;

    assert(lp != NULL && rl != NULL && g->has_resource);
    h = fold_route(h, g);
    t = route(lp, rl, g->has_resource, g->resource_is_bool, g->resource);
    assert(t != NULL && t != g);
    assert(t->planets == lp && t->slipways == rl);
    assert(same_resource(t, g));
    h = fold_route(h, t);

    t = trade_new(game_zone(), NULL);
    assert(t != NULL && t->planets == NULL && t->slipways == NULL && !t->has_resource);
    t = route(lp, NULL, 0, 0, 0);
    assert(t->planets == lp && t->slipways == NULL && !t->has_resource);
    check("game_route", h, GAME_ROUTE_HASH);
}

/* trade_last_planet is the last of the planets; an empty or missing list
 * gives nothing and leaves the output alone. */
static void test_last_planet(void) {
    static entity_t sentinel_target;
    entity_t *const sentinel = &sentinel_target;
    void *items[3];
    entity_t *out;
    trade_route_t *t;

    (void)game_route();
    items[0] = planet(0);
    items[1] = planet(1);
    items[2] = planet(2);
    t = route(list_of(3, items), NULL, 0, 0, 0);
    assert(trade_last_planet(t, &out) && out == planet(2));
    t = route(list_of(1, items), NULL, 0, 0, 0);
    assert(trade_last_planet(t, &out) && out == planet(0));

    out = sentinel;
    assert(!trade_last_planet(route(list_of(0, items), NULL, 0, 0, 0), &out) && out == sentinel);
    assert(!trade_last_planet(trade_new(game_zone(), NULL), &out) && out == sentinel);
    assert(!trade_last_planet(NULL, &out) && out == sentinel);
}

static int list_is(const entity_list_t *l, int n, entity_t *const *items) {
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

/* trade_extend builds a new route and leaves `self` alone: the node
 * appended to the planets, the slipway to the slipways unless NULL, the
 * same resource; `self` with no resource gives the new route false. */
static void test_extend(void) {
    trade_route_t *built_route = game_route();
    const entity_list_t *planets = built_route->planets;
    const entity_list_t *slipways = built_route->slipways;
    const int planets_n = planets->count;
    const int slipways_n = slipways->count;
    void *node = planet(3);
    void *slipway = slipways->items[0];
    entity_t *want[16];
    trade_route_t *extended;
    int i;

    assert(built_route->has_resource && planets_n + 1 < 16 && slipways_n + 1 < 16);
    extended = trade_extend(game_zone(), built_route, node, slipway);
    assert(extended != NULL && extended != built_route);
    assert(built_route->planets == planets && built_route->slipways == slipways && planets->count == planets_n && slipways->count == slipways_n);
    assert(extended->planets != planets && extended->slipways != slipways);
    for (i = 0; i < planets_n; i++) {
        want[i] = planets->items[i];
    }
    want[planets_n] = node;
    assert(list_is(extended->planets, planets_n + 1, want));
    for (i = 0; i < slipways_n; i++) {
        want[i] = slipways->items[i];
    }
    want[slipways_n] = slipway;
    assert(list_is(extended->slipways, slipways_n + 1, want));
    assert(same_resource(extended, built_route));

    extended = trade_extend(game_zone(), built_route, node, NULL);
    assert(extended->planets->count == planets_n + 1 && list_is(extended->slipways, slipways_n, slipways->items));
    assert(extended->slipways != slipways);
    assert(same_resource(extended, built_route));

    extended = trade_extend(game_zone(), built_route, NULL, NULL);
    assert(list_is(extended->planets, planets_n, planets->items) && list_is(extended->slipways, slipways_n, slipways->items));

    extended = trade_extend(game_zone(), route((entity_list_t *)planets, NULL, 0, 0, 0), node, NULL);
    assert(extended->planets->count == planets_n + 1 && extended->slipways->count == 0);
    assert(extended->has_resource && extended->resource_is_bool && extended->resource == 0);

    extended = trade_extend(game_zone(), trade_new(game_zone(), NULL), node, slipway);
    want[0] = node;
    assert(list_is(extended->planets, 1, want));
    want[0] = slipway;
    assert(list_is(extended->slipways, 1, want));
    assert(extended->has_resource && extended->resource_is_bool && extended->resource == 0);

    assert(trade_extend(game_zone(), NULL, node, slipway) == NULL);
}

/* trade_new and trade_extend build in the zone they are given: an arena
 * taken from the game zone grows, the game zone itself does not. */
static void test_built_in_the_given_zone(void) {
    trade_route_t *g = game_route();
    const allocator_t *arena = arena_take(game_zone(), "routes", 4096);
    const size_t game_zone_used_before = game_zone_used();
    size_t taken = arena_used(arena);
    trade_route_t *k;

    assert(trade_new(arena, NULL) != NULL);
    assert(arena_used(arena) > taken);
    taken = arena_used(arena);
    k = trade_extend(arena, g, planet(3), g->slipways->items[0]);
    assert(k != NULL && arena_used(arena) > taken);
    assert(game_zone_used() == game_zone_used_before);
}

/* A route made in the frame zone, copied into the game zone: the copy
 * shares neither struct nor list with it, and reads back whole once the
 * frame zone is reset, which ASan poisons. */
static void test_copy_outlives_the_frame(void) {
    trade_route_t *g = game_route();
    trade_route_t *k = trade_extend(frame_zone(), g, planet(3), g->slipways->items[0]);
    trade_route_t *c = trade_copy(game_zone(), k);
    entity_t *planets_want[16];
    entity_t *slipways_want[16];
    const int planets_n = k->planets->count;
    const int slipways_n = k->slipways->count;
    const int resource_is_bool = k->resource_is_bool;
    const p8_fixed_t qe = k->resource;
    trade_route_t *nil_copy;
    int i;

    assert(planets_n < 16 && slipways_n < 16 && k->has_resource);
    for (i = 0; i < planets_n; i++) {
        planets_want[i] = k->planets->items[i];
    }
    for (i = 0; i < slipways_n; i++) {
        slipways_want[i] = k->slipways->items[i];
    }
    assert(c != NULL && c != k && c->header.class == trade_class());
    assert(c->planets != k->planets && c->slipways != k->slipways && c->planets->items != k->planets->items && c->slipways->items != k->slipways->items);
    assert(trade_equal(c, k));

    frame_zone_reset();
    assert(list_is(c->planets, planets_n, planets_want) && list_is(c->slipways, slipways_n, slipways_want));
    assert(c->has_resource && c->resource_is_bool == resource_is_bool && c->resource == qe);

    nil_copy = trade_copy(game_zone(), trade_new(game_zone(), NULL));
    assert(nil_copy != NULL && nil_copy->planets == NULL && nil_copy->slipways == NULL && !nil_copy->has_resource);
    assert(trade_copy(game_zone(), NULL) == NULL);
}

/* Equal when they start and end at the same planet, whatever the length. */
static void test_equal(void) {
    void *a[2];
    void *b[3];
    void *c[2];
    void *d[2];
    trade_route_t *ta;
    trade_route_t *tb;

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
    assert(trade_equal(ta, tb) && trade_equal(tb, ta) && trade_equal(ta, ta));
    assert(trade_equal(ta, route(list_of(2, a), list_of(0, a), 1, 1, 1)));
    assert(!trade_equal(ta, route(list_of(2, c), NULL, 0, 0, 0)));
    assert(!trade_equal(ta, route(list_of(2, d), NULL, 0, 0, 0)));
    assert(!trade_equal(ta, route(list_of(0, a), NULL, 0, 0, 0)));
    assert(!trade_equal(route(list_of(0, a), NULL, 0, 0, 0), ta));
    assert(!trade_equal(ta, trade_new(game_zone(), NULL)) && !trade_equal(trade_new(game_zone(), NULL), ta));
    assert(!trade_equal(ta, NULL) && !trade_equal(NULL, ta));
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

static void build_with_bool_resource_2(void *e) {
    (void)e;
    (void)route(NULL, NULL, 1, 1, 2);
}

static void build_with_nil_resource_holding_1(void *e) {
    (void)e;
    (void)route(NULL, NULL, 0, 0, 1);
}

static void build_with_nil_resource_as_bool(void *e) {
    (void)e;
    (void)route(NULL, NULL, 0, 1, 0);
}

static void resource_of_true_route(void *e) {
    (void)e;
    (void)trade_resource(route(NULL, NULL, 1, 1, 1));
}

static void last_planet_on_other(void *e) {
    entity_t *out;
    (void)trade_last_planet(e, &out);
}

static void extend_on_other(void *e) {
    (void)trade_extend(game_zone(), e, NULL, NULL);
}

static void copy_of_other(void *e) {
    (void)trade_copy(game_zone(), e);
}

static void equal_left_other(void *e) {
    (void)trade_equal(e, trade_new(game_zone(), NULL));
}

static void equal_right_other(void *e) {
    (void)trade_equal(trade_new(game_zone(), NULL), e);
}

/* A game's route is a struct whose class has no method; a resource key its
 * member could not hold stops the run, as does reading a true resource, and
 * trade_last_planet, trade_extend, trade_copy and trade_equal refuse
 * anything that is not a route. */
static void test_trade_is_a_struct(void) {
    const entity_class_t *cls = trade_class();
    trade_route_t *g = game_route();
    trade_route_t *t = trade_new(game_zone(), NULL);
    int j;

    assert(g != NULL && g->header.class == cls);
    assert(t != NULL && t->header.class == cls);
    assert(cls->draw == NULL && cls->draw_overlay == NULL && cls->hit_test == NULL && cls->new_year == NULL && cls->refresh == NULL &&
           cls->left_press == NULL && cls->right_press == NULL && cls->accept_drop == NULL && cls->drag == NULL && cls->drop == NULL &&
           cls->attach_slipway == NULL && cls->offer_trade == NULL && cls->draw_at == NULL && cls->spawn_particle == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(trade_ready());
    assert(trade_new(game_zone(), NULL) != NULL);

    must_abort(build_with_bool_resource_2, NULL);
    must_abort(build_with_nil_resource_holding_1, NULL);
    must_abort(build_with_nil_resource_as_bool, NULL);
    must_abort(resource_of_true_route, NULL);
    must_abort(last_planet_on_other, main_ui());
    must_abort(extend_on_other, main_ui());
    must_abort(copy_of_other, main_ui());
    must_abort(equal_left_other, main_ui());
    must_abort(equal_right_other, main_ui());
}

static int g_accept_calls = 0;

static void count_accept(trade_offer_t *self, trade_route_t *a) {
    (void)self;
    (void)a;
    g_accept_calls++;
}

/* A need of any resource (code "?") takes the resource of a trade its
 * planet accepts (accepted_resources), then answers that trade once. */
static void test_offer_trade_wildcard_want(void) {
    trade_keys_t keys = {0};
    trade_offer_t lw = {0};
    pointer_list_t *jy = malloc(sizeof *jy);
    planet_t *qh;
    need_t *want;
    p8_fixed_t any;
    int any_resource;

    start_game();
    assert(resource_code_lookup("?", &any_resource));
    any = p8_fixed_from_int(any_resource);
    want = need_new(game_zone(), &any);
    pointer_list_init(jy, &heap_allocator);
    pointer_list_add(jy, want);
    qh = structure_new(NULL);
    qh->needs = jy;
    keys.has_resource = 1;
    keys.resource = p8_fixed_from_int(qh->archetype->accepted_resources->items[0]);
    lw.route = trade_new(game_zone(), &keys);
    lw.accept = count_accept;
    planet_offer_trade(qh, &lw);
    assert(want->row.has_resource && want->row.resource == keys.resource && g_accept_calls == 1);
}

/* A route through trade_extend holds false, not absent: it no longer
 * answers a need with no resource, as a route with no resource does --
 * GAP-24. */
static void test_offer_trade_nil_want_after_extend(void) {
    trade_offer_t lw = {0};
    pointer_list_t *jy = malloc(sizeof *jy);
    planet_t *qh;
    trade_route_t *nil_route;

    start_game();
    pointer_list_init(jy, &heap_allocator);
    pointer_list_add(jy, need_new(game_zone(), NULL));
    qh = structure_new(NULL);
    qh->needs = jy;
    nil_route = trade_new(game_zone(), &(trade_keys_t){.planets = list_of(0, NULL)});
    lw.accept = count_accept;
    lw.route = nil_route;
    planet_offer_trade(qh, &lw);
    assert(g_accept_calls == 1);
    lw.route = trade_extend(game_zone(), nil_route, NULL, NULL);
    assert(lw.route->has_resource && lw.route->resource_is_bool && lw.route->resource == 0);
    planet_offer_trade(qh, &lw);
    assert(g_accept_calls == 1);
}

/* economy_tiers_compute_tier counts for a need only the routes whose
 * resource equals the need's: a route with no resource is skipped by the
 * has_resource guard, and one through trade_extend holds false, which is
 * not the absent resource of a need with none -- GAP-24. */
static void test_compute_tier_counts_numbered_routes_only(void) {
    pointer_list_t *jy = malloc(sizeof *jy);
    entity_list_t *cc = malloc(sizeof *cc);
    planet_t *qh;
    need_t *nil_want;
    need_t *x_want;
    p8_fixed_t x = p8_fixed_from_int(2);
    trade_route_t *nil_route;

    start_game();
    nil_want = need_new(game_zone(), NULL);
    x_want = need_new(game_zone(), &x);
    pointer_list_init(jy, &heap_allocator);
    pointer_list_add(jy, nil_want);
    pointer_list_add(jy, x_want);
    nil_route = trade_new(game_zone(), NULL);
    entity_list_init(cc, &heap_allocator);
    entity_list_add(cc, &nil_route->header);
    entity_list_add(cc, &trade_extend(game_zone(), nil_route, NULL, NULL)->header);
    entity_list_add(cc, &trade_new(game_zone(), &(trade_keys_t){.has_resource = 1, .resource = x})->header);
    qh = structure_new(NULL);
    qh->needs = jy;
    qh->incoming_routes = cc;
    entity_set_state(&qh->header, "settled");
    (void)economy_tiers_compute_tier(&qh->header);
    assert(nil_want->has_satisfied && nil_want->satisfied == p8_fixed_from_int(0));
    assert(x_want->has_satisfied && x_want->satisfied == p8_fixed_from_int(1));
}

/* resource_equal is symmetric, `number` unread outside RESOURCE_FIXED; and
 * trade_resource reads absent, false (after trade_extend) and a number. */
static void test_resource_equal(void) {
    const resource_t nil = {.kind = RESOURCE_NIL};
    const resource_t no = {.kind = RESOURCE_FALSE};
    const resource_t n0 = {.kind = RESOURCE_FIXED, .number = 0};
    const resource_t n2 = {.kind = RESOURCE_FIXED, .number = p8_fixed_from_int(2)};
    const resource_t n3 = {.kind = RESOURCE_FIXED, .number = p8_fixed_from_int(3)};
    trade_route_t *nil_route = trade_new(game_zone(), NULL);
    trade_route_t *two = trade_new(game_zone(), &(trade_keys_t){.has_resource = 1, .resource = p8_fixed_from_int(2)});
    resource_t got;

    assert(resource_equal(nil, nil) && resource_equal(no, no) && resource_equal(n2, n2));
    assert(!resource_equal(nil, no) && !resource_equal(no, nil));
    assert(!resource_equal(n0, nil) && !resource_equal(nil, n0) && !resource_equal(n0, no) &&
           !resource_equal(no, n0));
    assert(!resource_equal(n2, n3) && !resource_equal(n3, n2));
    assert(resource_equal(nil, (resource_t){.kind = RESOURCE_NIL, .number = 5}) &&
           resource_equal((resource_t){.kind = RESOURCE_FALSE, .number = 7}, no));

    assert(trade_resource(NULL).kind == RESOURCE_NIL);
    assert(trade_resource(nil_route).kind == RESOURCE_NIL);
    assert(trade_resource(trade_extend(game_zone(), nil_route, NULL, NULL)).kind == RESOURCE_FALSE);
    got = trade_resource(two);
    assert(got.kind == RESOURCE_FIXED && got.number == p8_fixed_from_int(2));
}

/* Each scenario runs in a child forked right after cart_load(), as a
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
    p8_mem_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    assert(p8_load_map_json(&memory, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);

    cart_load();
    in_fresh_child(test_construction);
    in_fresh_child(test_last_planet);
    in_fresh_child(test_extend);
    in_fresh_child(test_built_in_the_given_zone);
    in_fresh_child(test_copy_outlives_the_frame);
    in_fresh_child(test_equal);
    in_fresh_child(test_trade_is_a_struct);
    in_fresh_child(test_offer_trade_wildcard_want);
    in_fresh_child(test_offer_trade_nil_want_after_extend);
    in_fresh_child(test_compute_tier_counts_numbered_routes_only);
    in_fresh_child(test_resource_equal);

    printf("test_trade: all tests passed\n");
    return 0;
}
