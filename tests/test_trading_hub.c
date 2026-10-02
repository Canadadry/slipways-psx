/* The trading hub `nl`, read and written on its struct members and the
 * generated defaults (pico8_game_data.h), and driven through the real `ie:ldown`/`mv`/`dg`, in
 * games built directly: no events file. */

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
#include "game/economy_score.h"
#include "game/economy_tiers.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* The cart keeps the box of `mp`, the port a copy of it (nil stays nil). */
static int ra_copies(const vec_t *ra, const vec_t *mp) {
    return mp == NULL ? ra == NULL : ra != NULL && ra != mp && ra->x == mp->x && ra->y == mp->y;
}


#define BUILT_HASH 0x067C3AC7u
#define LIFE_HASH 0x31386BDAu

/* The tag `jo:ib`'s list carried as a tagged list, which the frozen hashes
 * fold: any other value moves every *_HASH. */
#define FROZEN_LIST_TAG 0x9E5C1157u

/* The tag entity structs carried, which the frozen hashes fold: any other
 * value moves every *_HASH. */
#define FROZEN_STRUCT_TAG 0x9E5C57C7u

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const char *const bm[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};
static const char *const tags[] = {"planet", "node", "targeting"};

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

static int ents_rank_in(const entity_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int ents_listed(const entity_list_t *list, const void *e) {
    return ents_rank_in(list, e) >= 0;
}

static uint32_t fold_absent(uint32_t h) {
    return fnv(h, 0u);
}

static uint32_t fold_fixed_member(uint32_t h, p8_fixed_t v) {
    return fnv(fnv(fnv(h, 1u), 0xF1u), (uint32_t)v);
}

static uint32_t fold_bool_member(uint32_t h, int b) {
    return fnv(fnv(fnv(h, 1u), 0xB0u), (uint32_t)(b != 0));
}

/* A NULL string folds as absent. */
static uint32_t fold_string_member(uint32_t h, const char *s) {
    return s == NULL ? fold_absent(h) : fnv_str(fnv(fnv(h, 1u), 0x57u), s);
}

static uint32_t fold_hf(uint32_t h, const planet_t *jo) {
    return jo->archetype->is_planet ? fold_fixed_member(h, p8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

static uint32_t fold_vec(uint32_t h, const vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (ents_listed(live_entities(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)ents_rank_in(live_entities(), ref));
    }
    return fnv(fnv(h, 0xEEEEu), FROZEN_STRUCT_TAG);
}

/* The item count of a list member, -1 when NULL. */
static int list_size(const pointer_list_t *l) {
    return l == NULL ? -1 : l->count;
}

static int ents_list_size(const entity_list_t *l) {
    return l == NULL ? -1 : l->count;
}

static uint32_t fold_kw(uint32_t h, const planet_t *jo) {
    const production_t *k = jo->production;

    if (k == NULL) {
        return fnv(h, 0xA0u);
    }
    h = k->has_resource ? fold_fixed_member(h, k->resource) : fold_absent(h);
    h = fold_fixed_member(h, k->amount);
    return fnv(h, (uint32_t)(k == planet_shared_production()));
}

/* `e` as a planet of qh's lineage. */
static planet_t *as_qh(void *e) {
    const entity_t *h = e;

    assert(h != NULL && (h->class->lineage & LINEAGE(LINEAGE_STRUCTURE)));
    return e;
}

static uint32_t fold_qh(uint32_t h, const planet_t *qh) {
    return qh->archetype->is_structure ? fold_fixed_member(h, p8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

/* 1 no target, 3 a target: the bytes the frozen hashes fold. */
static uint32_t et_state(const planet_t *qh) {
    return qh->targeting->et == NULL ? 1u : 3u;
}

/* `et` as qh's reader gave it: false or the target. */
static uint32_t fold_et(uint32_t h, const planet_t *qh) {
    switch (et_state(qh)) {
    case 1u:
        return fold_bool_member(h, 0);
    default:
        return fold_ref(fnv(h, 1u), qh->targeting->et);
    }
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
}

static vec_t *vec_ref(vec_t at) {
    vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

static vec_t vec_xy(int x, int y) {
    return vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y));
}

static const entity_class_t *cls_of(const void *e) {
    return ((const entity_t *)e)->class;
}

/* `e` as a planet of nl's lineage. */
static const planet_t *as_nl(const void *e) {
    assert(e != NULL && (cls_of(e)->lineage & LINEAGE(LINEAGE_TRADING_HUB)));
    return e;
}

/* The members a planet of qh's tree holds: `kj`, `ev`, `hd`, `hk`, `hb`,
 * `ldown`, `gv`, `hq`, `ey`, `kf`, `cn` and the states `ll`, `ly`; `mm`
 * aside, no other. */
static void assert_qh_shape(const entity_class_t *c) {
    assert(c->draw != NULL && c->draw_overlay != NULL && c->hit_test != NULL && c->new_year != NULL && c->refresh != NULL && c->left_press != NULL &&
           c->accept_drop != NULL && c->states[ENTITY_STATE_VANISHING] != NULL && c->states[ENTITY_STATE_SETTLED] != NULL);
    assert(c->has_refresh == NULL && c->right_press == NULL && c->states[ENTITY_STATE_NORMAL] == NULL &&
           c->states[ENTITY_STATE_PLACING] == NULL && c->states[ENTITY_STATE_UNFOLDING] == NULL);
    assert(c->drag != NULL && c->drop != NULL && c->attach_slipway != NULL && c->offer_trade != NULL && c->draw_at == NULL);
}

/* `c` holds `ref`'s very members but `kf`, `cn` and `mm`, `kj` and the
 * state `ly` included when `with_kj`/`with_ly`. */
static void assert_members_of(const entity_class_t *c, const entity_class_t *ref, int with_kj,
                              int with_ly) {
    assert(c->draw_overlay == ref->draw_overlay && c->hit_test == ref->hit_test && c->new_year == ref->new_year && c->refresh == ref->refresh && c->has_refresh == ref->has_refresh &&
           c->left_press == ref->left_press && c->right_press == ref->right_press && c->accept_drop == ref->accept_drop);
    assert(c->states[ENTITY_STATE_VANISHING] == ref->states[ENTITY_STATE_VANISHING] && c->drag == ref->drag && c->drop == ref->drop);
    assert(!with_kj || c->draw == ref->draw);
    assert(!with_ly || c->states[ENTITY_STATE_SETTLED] == ref->states[ENTITY_STATE_SETTLED]);
}

/* The research icon whose `ke` is `gx[4]`, the trading hub. */
static void *nl_icon(void) {
    const entity_list_t *hb = entities_by_message("refresh");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i];
        if (cls_of(e) == pico8_ie_struct_class() && ((const pico8_ie_t *)e)->ke == pico8_nl_kind()) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the trading hub's icon, as the shop does. */
static void *build_plate(void) {
    void *icon = nl_icon();
    down_t made;

    assert(icon != NULL);
    made = cls_of(icon)->left_press(icon, NULL, NULL);
    assert(made.stop && made.held != NULL);
    return made.held;
}

/* A plate is the only entity whose chain holds `nl`, and its `nl` is 1. */
static int is_plate(const void *e) {
    const planet_t *jo = e != NULL && (cls_of(e)->lineage & LINEAGE(LINEAGE_TRADING_HUB)) ? e : NULL;
    return jo != NULL && jo->archetype->is_trading_hub == p8_fixed_from_int(1);
}

/* A plate as `ie:ldown` and `jo:pq` left it: its scalars, which of nl's,
 * qh's and jo's members are set, its methods (qh's chain's very
 * functions, but its own `ly`), which default tables it shares with `nl`
 * and `jo`, and where `lt` registered it -- never in `cg.qh`. */
static uint32_t fold_plate(uint32_t h, const void *e) {
    const planet_t *nl = as_nl(e);
    const planet_t *qh;
    const planet_t *jo;
    size_t i;

    qh = nl;
    jo = qh;
    h = fold_string_member(h, jo->planet_id);
    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_string_member(h, nl->archetype->required_tech);
    h = fold_fixed_member(h, nl->archetype->is_trading_hub);
    h = fold_qh(h, qh);
    h = fold_fixed_member(h, qh->archetype->project_index);
    h = fold_fixed_member(h, qh->sprite);
    h = fold_fixed_member(h, qh->targeting->od);
    h = fold_fixed_member(h, nl->palette_block);
    h = fold_string_member(h, qh->targeting->def->confirmed_state);
    h = fold_fixed_member(h, qh->targeting->def->clock_ticks);
    h = fold_fixed_member(h, qh->targeting->def->attach_distance);
    h = jo->has_development_tier ? fold_fixed_member(h, jo->development_tier) : fold_absent(h);
    h = fold_hf(h, jo);
    h = fold_fixed_member(h, jo->archetype->trade_multiplier);
    h = fold_fixed_member(h, jo->archetype->layer);
    h = fold_fixed_member(h, jo->shake);
    h = fold_fixed_member(h, jo->archetype->back_color);
    h = fold_vec(h, &jo->archetype->row_step);
    /* The 1s of `nl` and `qw`, of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of
     * `hf`, `hs`, `fa`, `ma` and `ca`: constant by the struct's shape, kept
     * so that BUILT_HASH still proves the typed reads. */
    h = fnv(h, (uint32_t)(nl->archetype->required_tech != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->archetype->name != NULL));
    h = fnv(h, (uint32_t)(qh->targeting->def->confirmed_state != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->archetype->accepted_resources != NULL));
    h = fnv(h, qh->targeting->et != NULL ? 1u : 0u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(jo->archetype->ring_colors != NULL));
    h = fnv(h, (uint32_t)(jo->archetype->mood_rows != NULL));
    h = fnv(h, (uint32_t)(jo->archetype->made_resources != NULL));
    h = fnv(h, (uint32_t)(jo->production != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(jo->archetype->spacer != NULL));
    h = fnv(h, (uint32_t)(jo->planet_id != NULL));
    h = fnv(h, (uint32_t)(jo->map_pos != NULL));
    h = fnv(h, (uint32_t)(jo->pos != NULL));
    h = fnv(h, (uint32_t)(jo->slipways != NULL));
    h = fnv(h, (uint32_t)(jo->project != NULL));
    h = fnv(h, (uint32_t)(jo->description != NULL));
    h = fnv(h, (uint32_t)(jo->needs != NULL));
    h = fnv(h, (uint32_t)(jo->planet_type != NULL));
    h = fnv(h, (uint32_t)(jo->fold_anchor != 0));
    h = fnv(h, (uint32_t)(jo->layout_row != NULL));
    h = fnv(h, (uint32_t)(jo->targeted_by != NULL));
    h = fnv(h, (uint32_t)jo->has_production_bonus);
    h = fnv(h, (uint32_t)(jo->is_dead != 0));
    h = fnv(h, (uint32_t)jo->has_net_income);
    h = fnv(h, (uint32_t)(jo->incoming_routes != NULL));
    h = fnv(h, (uint32_t)(jo->outgoing_routes != NULL));
    h = fnv(h, (uint32_t)jo->has_development_tier);
    h = fnv(h, (uint32_t)(jo->is_hovered != 0));
    assert(planet_is_family(e));
    assert_qh_shape(cls_of(e));
    assert_members_of(cls_of(e), pico8_qh_struct_class(), 1, 0);
    assert(cls_of(e)->attach_slipway == pico8_qh_struct_class()->attach_slipway && cls_of(e)->offer_trade == pico8_qh_struct_class()->offer_trade &&
           cls_of(e)->spawn_particle == NULL);
    assert(cls_of(e)->states[ENTITY_STATE_SETTLED] != pico8_qh_struct_class()->states[ENTITY_STATE_SETTLED]);
    assert(jo->header.class->tags == (TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)));
    assert(qh->archetype->name == &trading_hub_name_row && qh->archetype->name != &structure_name_row);
    assert(qh->archetype->accepted_resources == &trading_hub_accepted_resources_seq && qh->archetype->accepted_resources != &structure_accepted_resources_seq);
    assert(jo->archetype == &trading_hub_archetype);
    assert(jo->archetype == &trading_hub_archetype);
    assert(jo->archetype->spacer == &planet_spacer_row && jo->archetype->ring_colors == &planet_ring_colors_seq && jo->archetype->mood_rows == planet_mood_rows_rows &&
           jo->archetype->made_resources == &planet_made_resources_seq);
    assert(jo->production != NULL && jo->production != planet_shared_production());
    assert(pico8_gb_at(6) != NULL);
    assert(jo->project == pico8_gb_at(6));
    h = fold_kw(h, jo);
    h = fnv(h, (uint32_t)ents_list_size(jo->slipways));
    h = fnv(h, (uint32_t)list_size(jo->needs));
    h = fnv(h, (uint32_t)ents_rank_in(live_entities(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(entities_by_message(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(entities_by_tag(tags[i]), e));
    }
    assert(!ents_listed(entities_by_tag("structure"), e));
    return h;
}

/* Two plates built by the shop's `ie:ldown` on `gx[4]`. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game();
    a = build_plate();
    b = build_plate();
    assert(a != b && is_plate(a) && is_plate(b));
    h = fold_plate(h, a);
    h = fold_plate(h, b);
    check("built", h, BUILT_HASH);
}

static p8_fixed_t credits(void) {
    assert(pico8_fm_hy() != NULL);
    return pico8_fm_hy()->nz;
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = p8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const planet_t *nl) {
    const vec_t *ra = nl->pos;
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what `nl:ly` left on the plate. */
static uint32_t fold_dg(uint32_t h, const void *e) {
    const planet_t *nl = as_nl(e);

    h = fold_ra_y(h, nl);
    update_entities();
    h = fold_fixed_member(h, nl->header.state_time);
    h = fold_ra_y(h, nl);
    return fold_fixed_member(h, nl->palette_block);
}

/* `jo:ib`'s container, folded as fold_ref folded the tagged list it was:
 * never in `hv`, so the tag branch. */
static uint32_t fold_ib(uint32_t h, const entity_list_t *ib) {
    if (ib == NULL) {
        return fnv(h, 0xDEADu);
    }
    return fnv(fnv(h, 0xEEEEu), FROZEN_LIST_TAG);
}

/* What `bb`'s `dh` (the `pt.nl` branch), `ib`, `gg` and `da` (qh false)
 * give on a plate. */
static uint32_t fold_economy(uint32_t h, void *e) {
    entity_list_t *ib = planet_rebuild_trades(e);
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_ib(h, ib);
    h = fnv(fnv(h, 0xB0u), planet_is_colonized(e) ? 1u : 0u);
    h = fnv(h, (uint32_t)pico8_da(e));
    return as_qh(e)->has_development_tier ? fold_fixed_member(h, as_qh(e)->development_tier) : fold_absent(h);
}

/* `hq` (`bv`) in the panel then `ey` (`d_`) with the credits; `dg` in "ly"
 * with `mi`=1 (`qw` stays), then `mi` at 2 and 3 (`qw` = 8+t%42/6); `dg`
 * on a plate still in "kz". */
static void test_life(void) {
    uint32_t h = 2166136261u;
    planet_t *placed;
    planet_t *idle;
    vec_t *mp;
    int i;

    start_game();

    placed = build_plate();
    mp = vec_ref(vec_xy(7680, 7552));
    cls_of(placed)->drag(&placed->header, mp, vec_ref(vec_xy(64, 64)));
    assert(ra_copies(placed->pos, mp));
    h = fold_vec(h, placed->map_pos);
    h = fnv(h, et_state(placed));
    h = fold_et(h, placed);
    set_credits(100);
    assert(cls_of(placed)->drop == pico8_d__dispatch);
    h = pico8_d_(&placed->header) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);
    h = fold_string_member(h, placed->header.state);
    h = fold_fixed_member(h, placed->header.state_time);
    h = fnv(h, (uint32_t)credits());
    assert(strcmp(placed->header.state, "settled") == 0 && credits() == p8_fixed_from_int(100 - 20));

    h = fold_economy(h, placed);

    for (i = 0; i < 3; i++) {
        h = fold_dg(h, placed);
    }
    assert(placed->palette_block == p8_fixed_from_int(15));
    placed->development_tier = p8_fixed_from_int(2);
    placed->has_development_tier = 1;
    h = fold_dg(h, placed);
    h = fold_dg(h, placed);
    placed->development_tier = p8_fixed_from_int(3);
    placed->has_development_tier = 1;
    h = fold_dg(h, placed);
    assert(placed->palette_block != p8_fixed_from_int(15));
    h = fold_economy(h, placed);

    idle = build_plate();
    h = fold_string_member(h, idle->header.state);
    h = fold_dg(h, idle);
    assert(idle->palette_block == p8_fixed_from_int(15));
    check("life", h, LIFE_HASH);
}

/* Per frame, every plate in `hv`: what the shop, `bv`, `d_`, `dg`, `nl:ly`
 * and `jo`'s functions write on it; plus the screen. */
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

static void build_with_unknown_key(void *e) {
    planet_keys_t keys;
    (void)e;
    keys.planet_id = NULL;
    keys.map_pos = vec_ref(vec_xy(1, 1));
    keys.pos = NULL;
    (void)pico8_nl_new(&keys);
}

static int is_an_nl_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_nl_struct_class();
}

/* Built by `ie:ldown` or pico8_nl_new, a plate is a struct whose descriptor
 * holds qh's very entries but its own `ly`, which nl's class no longer
 * carries; a construction key other than `id` stops the run. */
static void test_nl_is_a_struct(void) {
    const entity_class_t *cls = pico8_nl_struct_class();
    const entity_class_t *qh = pico8_qh_struct_class();
    void *a;

    start_game();
    a = build_plate();
    assert(is_an_nl_struct(a) && ents_listed(entities_by_tag("planet"), a) && !ents_listed(entities_by_tag("structure"), a));
    assert_qh_shape(cls);
    assert_members_of(cls, qh, 1, 0);
    assert(cls->attach_slipway == qh->attach_slipway && cls->offer_trade == qh->offer_trade && cls->spawn_particle == NULL);
    assert(cls->states[ENTITY_STATE_SETTLED] != qh->states[ENTITY_STATE_SETTLED]);

    must_abort(build_with_unknown_key, NULL);
    {
        planet_t *bare = pico8_nl_new(NULL);
        assert(is_an_nl_struct(bare) && bare->planet_id == NULL && bare->slipways != NULL &&
               ents_listed(entities_by_tag("planet"), bare));
    }
}

/* The galaxy depends on the rnd state: run each scenario in a child forked
 * right after pico8_cart_load(), as a fresh process would start it. */
static pid_t spawn(void (*test)(void)) {
    pid_t child;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        test();
        fflush(stdout);
        _exit(0);
    }
    return child;
}

static void reap(pid_t child) {
    int status;
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static void in_fresh_child(void (*test)(void)) {
    reap(spawn(test));
}

int main(void) {
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_nl_is_a_struct);

    printf("test_pico8_nl: all tests passed\n");
    return 0;
}
