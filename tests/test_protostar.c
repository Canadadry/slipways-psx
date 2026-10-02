/* The protostar `pm`, read and written on its struct members and the
 * generated defaults (pico8_game_data.h), built as `jo:hu` builds it
 * (`kv["pm"]` on `p`, `ra`, `id`) and driven through the real
 * `mv`/`dg`/`dc`, in games started directly: no events file. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
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
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/particle.h"
#include "game/ui_zone.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/scroll.h"
#include "engine/entity_class.h"
#include "engine/vec.h"


#define BUILT_HASH 0x9F8A0FC3u
#define LIFE_HASH 0x3F990656u

/* The tag `jo:ib`'s list carried as a tagged list, which the frozen hashes
 * fold: any other value moves every *_HASH. */
#define FROZEN_LIST_TAG 0x9E5C1157u

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

/* The item count of a list member, -1 when NULL. */
static int ptrs_size(const pointer_list_t *l) {
    return l == NULL ? -1 : l->count;
}

static int ents_ptrs_size(const entity_list_t *l) {
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

static uint32_t fold_qh(uint32_t h, const planet_t *qh) {
    return qh->archetype->is_structure ? fold_fixed_member(h, p8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

/* `e` as an ascension, a `pm` or an `mx`: an `hj` first. */
static planet_t *as_hj(void *e) {
    const entity_class_t *c = ((const entity_t *)e)->class;
    assert(c == pico8_hj_struct_class() || c == pico8_pm_struct_class() || c == synthesizer_class());
    return e;
}

static uint32_t fold_pk(uint32_t h, const planet_t *hj) {
    return fold_bool_member(h, hj->emitter->archetype->quarter_turn != 0);
}

/* A particle record as `g_.mm` returns it: `p`, `v`, `l`. */
static uint32_t fold_particle(uint32_t h, const particle_t *q) {
    assert(q != NULL);
    assert(!q->has_acceleration);
    h = fnv(fnv(h, (uint32_t)q->pos.x), (uint32_t)q->pos.y);
    h = fnv(fnv(h, (uint32_t)q->velocity.x), (uint32_t)q->velocity.y);
    return fnv(h, (uint32_t)q->life);
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)p8_peek(&mem, P8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
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

/* `kv["pm"]` as `jo:hu` calls it: `p`, `ra`, `id`, each only when given. */
static void *build_protostar(const vec_t *p, const vec_t *ra, const char *id) {
    planet_keys_t keys;
    void *made;

    keys.map_pos = NULL;
    keys.pos = NULL;
    if (p != NULL) {
        keys.map_pos = vec_ref(*p);
    }
    if (ra != NULL) {
        keys.pos = vec_ref(*ra);
    }
    keys.planet_id = id;
    made = pico8_pm_new(&keys);
    assert(made != NULL);
    return made;
}

/* `pm` sets its own `px`: an entity whose `px` is the very table of pm's
 * class is a protostar. */
static int is_pm(const void *e) {
    const planet_t *jo = e != NULL && (cls_of(e)->lineage & LINEAGE(LINEAGE_STRUCTURE)) ? e : NULL;
    return jo != NULL && jo->archetype->name == &protostar_name_row;
}

static int is_ga(const void *e) {
    return e != NULL && cls_of(e) == pico8_ga_struct_class() && ((const planet_t *)e)->archetype->class_name != NULL &&
           strcmp(((const planet_t *)e)->archetype->class_name, "burst") == 0;
}

/* The `ga` that `pm:pq` spawned: registered in `hv` just before the
 * protostar, at `ra + ng`; its `qy` and `mo.ma` as `ga:pq` left them. */
static uint32_t fold_spawned(uint32_t h, void *e) {
    const entity_list_t *hv = live_entities();
    int at = ents_rank_in(hv, e);
    const planet_t *g;
    const planet_t *pm;
    const vec_t *ra;
    const vec_t *ng;
    const vec_t *gra;

    assert(at >= 1 && cls_of(e) == pico8_pm_struct_class());
    pm = e;
    ra = pm->pos;
    ng = &pm->archetype->spawn_offset;
    assert(is_ga(hv->items[at - 1]));
    g = (const planet_t *)hv->items[at - 1];
    gra = g->pos;
    assert(gra != NULL && ng != NULL);
    if (ra != NULL) {
        assert(gra->x == p8_fixed_add(ra->x, ng->x) && gra->y == p8_fixed_add(ra->y, ng->y));
    } else {
        assert(gra->x == ng->x && gra->y == ng->y);
    }
    assert(gra != ra && gra != ng);
    h = fold_vec(h, g->pos);
    h = fnv(h, (uint32_t)ptrs_size(g->emitter->particles));
    assert(main_camera_typed() != NULL);
    return fold_fixed_member(h, main_camera_typed()->shake);
}

/* A protostar as `kv["pm"]`, `pm:pq`, `f_` and `jo:pq` left it: its
 * scalars, which of its own, hj's, qh's and jo's members are set, its
 * methods (hj's very functions), which default tables it shares with `pm`,
 * `qh` and `jo`, where `lt` registered it -- in `cg.jo`/`np`/`prober`,
 * never in `cg.qh` or `cg.hj` -- and the `ga` it spawned. */
static uint32_t fold_protostar(uint32_t h, void *e) {
    const planet_t *hj = as_hj(e);
    const planet_t *pm;
    const emitter_t *em = hj->emitter;
    const planet_t *qh = hj;
    const planet_t *jo = qh;
    size_t i;

    assert(cls_of(e) == pico8_pm_struct_class());
    pm = e;
    h = fold_string_member(h, jo->planet_id);
    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_string_member(h, pm->archetype->class_name);
    h = fold_string_member(h, hj->archetype->required_tech);
    h = fold_qh(h, qh);
    h = fold_fixed_member(h, qh->archetype->project_index);
    h = fold_fixed_member(h, qh->sprite);
    h = fold_fixed_member(h, qh->targeting->cost);
    h = fold_fixed_member(h, em->speed_factor);
    h = fold_fixed_member(h, em->archetype->lifetime);
    h = fold_fixed_member(h, em->angle_slices);
    h = fold_pk(h, hj);
    h = fold_fixed_member(h, em->archetype->spawn_rate);
    h = fold_fixed_member(h, em->base_radius);
    h = fold_fixed_member(h, em->matrix_xx);
    h = fold_fixed_member(h, em->matrix_xy);
    h = fold_fixed_member(h, em->matrix_yx);
    h = fold_fixed_member(h, em->matrix_yy);
    h = fold_string_member(h, qh->targeting->archetype->confirmed_state);
    h = fold_fixed_member(h, qh->targeting->archetype->clock_ticks);
    h = fold_fixed_member(h, qh->targeting->archetype->attach_distance);
    h = jo->has_development_tier ? fold_fixed_member(h, jo->development_tier) : fold_absent(h);
    h = fold_hf(h, jo);
    h = fold_fixed_member(h, jo->archetype->trade_multiplier);
    h = fold_fixed_member(h, jo->archetype->layer);
    h = fold_fixed_member(h, jo->shake);
    h = fold_fixed_member(h, jo->archetype->back_color);
    h = fold_vec(h, &pm->archetype->spawn_offset);
    h = fold_vec(h, &jo->archetype->row_step);
    h = fold_vec(h, jo->pos);
    h = fold_vec(h, jo->map_pos);
    /* The 1s of hj's ten fixed emitter defaults, of `hp`, `qh`, `od`, `fp`,
     * `dw`, `s`, then of `hf`, `hs`, `fa`, `ma` and `ca`: constant by the
     * struct's shape, kept so that BUILT_HASH still proves the typed reads. */
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(pm->archetype->class_name != NULL));
    h = fnv(h, (uint32_t)(hj->archetype->required_tech != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(hj->emitter->trail_colors != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(em->particles != NULL));
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->archetype->name != NULL));
    h = fnv(h, (uint32_t)(qh->targeting->archetype->confirmed_state != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->archetype->accepted_resources != NULL));
    h = fnv(h, qh->targeting->target != NULL ? 1u : 0u);
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
    assert(ptrs_size(em->particles) == 0);
    assert_qh_shape(cls_of(e));
    assert_members_of(cls_of(e), pico8_qh_struct_class(), 0, 1);
    assert(cls_of(e)->attach_slipway == pico8_qh_struct_class()->attach_slipway && cls_of(e)->offer_trade == pico8_qh_struct_class()->offer_trade &&
           planet_is_family(e));
    assert(cls_of(e)->spawn_particle == pico8_hj_struct_class()->spawn_particle && cls_of(e)->spawn_particle != NULL);
    assert(cls_of(e)->draw == pico8_hj_struct_class()->draw);
    assert(pm->archetype->spawn_offset.x == p8_fixed_from_int(1) && pm->archetype->spawn_offset.y == 0);
    assert(jo->header.class->tags == (TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)) && qh->archetype->name == &protostar_name_row && em->trail_colors == &protostar_trail_colors_seq);
    assert(jo->header.class->tags != (TAG(TAG_ASCENSION) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)));
    assert(em->trail_colors != &ascension_trail_colors_seq);
    assert(qh->archetype->accepted_resources == &structure_accepted_resources_seq);
    assert(jo->archetype == &protostar_archetype);
    assert(jo->archetype == &protostar_archetype);
    assert(jo->archetype->spacer == &planet_spacer_row && jo->archetype->ring_colors == &planet_ring_colors_seq && jo->archetype->mood_rows == planet_mood_rows_rows &&
           jo->archetype->made_resources == &planet_made_resources_seq);
    assert(jo->production != NULL && jo->production != planet_shared_production());
    assert(research_projects_at(3) != NULL);
    assert(jo->project == research_projects_at(3));
    h = fold_kw(h, jo);
    h = fnv(h, (uint32_t)ents_ptrs_size(jo->slipways));
    h = fnv(h, (uint32_t)ptrs_size(jo->needs));
    h = fnv(h, (uint32_t)ents_rank_in(live_entities(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(entities_by_message(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(entities_by_tag(tags[i]), e));
    }
    assert(ents_listed(entities_by_tag("planet"), e) && !ents_listed(entities_by_tag("structure"), e) && !ents_listed(entities_by_tag("ascension"), e));
    return fold_spawned(h, e);
}

/* Two protostars built as `jo:hu` builds them, each with a `qy` of its own
 * and a `ga` of its own. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    vec_t pa = vec_xy(40, 50);
    vec_t ra_a = vec_xy(7700, 7600);
    vec_t pb = vec_xy(-30, 12);
    vec_t ra_b = vec_xy(7650, 7720);
    void *a;
    void *b;

    start_game();
    a = build_protostar(&pa, &ra_a, "s77");
    b = build_protostar(&pb, &ra_b, "s78");
    assert(a != b && is_pm(a) && is_pm(b));
    assert(as_hj(a)->emitter->particles != NULL && as_hj(a)->emitter->particles != as_hj(b)->emitter->particles);
    h = fold_protostar(h, a);
    h = fold_protostar(h, b);
    check("built", h, BUILT_HASH);
}

static uint32_t fold_ra_y(uint32_t h, const planet_t *hj) {
    const vec_t *ra = hj->pos;
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the protostar. */
static uint32_t fold_dg(uint32_t h, void *e) {
    const planet_t *hj = as_hj(e);

    h = fold_ra_y(h, hj);
    update_entities();
    h = fold_fixed_member(h, hj->header.state_time);
    return fold_ra_y(h, hj);
}

/* `jo:ib`'s container, folded as the other planet TUs' fold_ref folds the tagged list it was:
 * never in `hv`, so the tag branch. */
static uint32_t fold_ib(uint32_t h, const entity_list_t *ib) {
    if (ib == NULL) {
        return fnv(h, 0xDEADu);
    }
    return fnv(fnv(h, 0xEEEEu), FROZEN_LIST_TAG);
}

/* What `bb`'s `dh`, `ib`, `gg` and `da` (qh false) give on a protostar. */
static uint32_t fold_economy(uint32_t h, void *e) {
    entity_list_t *ib = planet_rebuild_trades(e);
    h = fnv(h, (uint32_t)economy_tiers_compute_tier(e));
    h = fold_ib(h, ib);
    h = fnv(fnv(h, 0xB0u), planet_is_colonized(e) ? 1u : 0u);
    h = fnv(h, (uint32_t)planet_upkeep(e));
    return as_hj(e)->has_development_tier ? fold_fixed_member(h, as_hj(e)->development_tier) : fold_absent(h);
}

/* `dc("kj")` once: the screen it leaves and what `hj:kj` left in `qy`. */
static uint32_t fold_draw(uint32_t h, void *e) {
    draw_layers("draw");
    h = fnv(h, screen_hash());
    return fnv(h, (uint32_t)ptrs_size(as_hj(e)->emitter->particles));
}

/* `dg` in "ly" (qh's `ly`, `na`); `mm` (`g_.mm`, `pk` false: no rotation)
 * and `ez`; `kj` through `dc` with `mi`=1, then `mi`=2 (`ei` moves and
 * draws the particles); `dh`/`ib`/`gg`/`da`; a protostar built on `id`
 * alone, its `ga` at `ng`. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    const vec_t *mo_ra;
    vec_t p = vec_xy(64, 64);
    vec_t ra;
    void *placed;
    void *bare;
    int i;

    start_game();
    assert(main_camera_typed() != NULL);
    mo_ra = main_camera_typed()->pos;
    assert(mo_ra != NULL);
    ra = vec_add(*mo_ra, vec_xy(12, 108));
    placed = build_protostar(&p, &ra, "s90");
    h = fold_spawned(h, placed);
    assert(strcmp(as_hj(placed)->header.state, "settled") == 0);

    h = fold_economy(h, placed);
    for (i = 0; i < 3; i++) {
        h = fold_dg(h, placed);
    }

    h = fold_particle(h, cls_of(placed)->spawn_particle(placed));
    emitter_spawn(placed, P8_FIXED_LIT(0x00028000, 2.5));
    h = fnv(h, (uint32_t)ptrs_size(as_hj(placed)->emitter->particles));
    assert(ptrs_size(as_hj(placed)->emitter->particles) >= 2);

    h = fold_draw(h, placed);
    h = fold_draw(h, placed);
    as_hj(placed)->development_tier = p8_fixed_from_int(2);
    as_hj(placed)->has_development_tier = 1;
    for (i = 0; i < 4; i++) {
        h = fold_draw(h, placed);
    }
    h = fold_economy(h, placed);

    bare = build_protostar(NULL, NULL, "s91");
    assert(as_hj(bare)->pos == NULL && as_hj(bare)->map_pos == NULL && is_pm(bare));
    h = fold_string_member(h, as_hj(bare)->planet_id);
    h = fold_spawned(h, bare);
    h = fold_dg(h, bare);
    check("life", h, LIFE_HASH);
}

static int is_a_pm_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_pm_struct_class();
}

/* Built by `kv["pm"]` or pico8_pm_new, a protostar is a struct whose
 * descriptor holds hj's very entries. pm's class keeps no `pq` of its own. */
static void test_pm_is_a_struct(void) {
    const entity_class_t *cls = pico8_pm_struct_class();
    const entity_class_t *hj = pico8_hj_struct_class();
    vec_t p = vec_xy(40, 50);
    vec_t ra = vec_xy(7700, 7600);
    void *a;

    start_game();
    a = build_protostar(&p, &ra, "s77");
    assert(is_a_pm_struct(a) && cls != hj && ents_listed(entities_by_tag("planet"), a) && !ents_listed(entities_by_tag("ascension"), a));
    assert_qh_shape(cls);
    assert_members_of(cls, hj, 1, 1);
    assert(cls->attach_slipway == hj->attach_slipway && cls->offer_trade == hj->offer_trade && cls->spawn_particle == hj->spawn_particle);
    assert(pico8_pm_ready());
    {
        planet_t *bare = pico8_pm_new(NULL);
        assert(is_a_pm_struct(bare) && bare->planet_id == NULL && bare->pos == NULL &&
               bare->slipways != NULL && ptrs_size(bare->emitter->particles) == 0 &&
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

/* What an emitter holds in the particle zone, by sort. */
typedef struct {
    int particules;
    int qy;
    int qy_fv;
    int rangees;
} held_t;

static held_t held_now(void) {
    held_t h;
    h.particules = particle_zone_live(PARTICLE_SLOT_PARTICLE);
    h.qy = particle_zone_live(PARTICLE_SLOT_LIST);
    h.qy_fv = particle_zone_live(PARTICLE_SLOT_HELP_LIST);
    h.rangees = ui_zone_live(SLOT_KIND_ROW);
    return h;
}

/* A protostar that leaves (`he`, then the exit queue's flush) gives back
 * its particles and its `qy`, not those of the burst it spawned. */
static void test_leaving_gives_back_its_particles(void) {
    held_t built;
    held_t after;
    vec_t at = vec_xy(64, 64);
    planet_t *e;

    start_game();
    pico8_cart_vider_file_sortie();
    e = as_hj(build_protostar(&at, &at, "s90"));
    emitter_spawn(&e->header, p8_fixed_from_int(5));
    cls_of(e)->refresh(&e->header);
    assert(e->emitter->particles->count == 5);
    built = held_now();
    entity_unregister(&e->header);
    pico8_cart_vider_file_sortie();
    after = held_now();
    assert(after.particules == built.particules - 5 && after.qy == built.qy - 1);
    assert(e->emitter->particles == NULL && e->layout_row_slot == NULL);
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
    in_fresh_child(test_pm_is_a_struct);
    in_fresh_child(test_leaving_gives_back_its_particles);

    printf("test_pico8_pm: all tests passed\n");
    return 0;
}
