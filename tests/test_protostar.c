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

static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
    return jo->def->hf ? fold_fixed_member(h, p8_fixed_from_int(1)) : fold_bool_member(h, 0);
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

static uint32_t fold_kw(uint32_t h, const pico8_jo_t *jo) {
    const pico8_kw_t *k = jo->kw;

    if (k == NULL) {
        return fnv(h, 0xA0u);
    }
    h = k->has_qe ? fold_fixed_member(h, k->qe) : fold_absent(h);
    h = fold_fixed_member(h, k->mu);
    return fnv(h, (uint32_t)(k == pico8_jo_shared_kw()));
}

static uint32_t fold_qh(uint32_t h, const pico8_jo_t *qh) {
    return qh->def->qh ? fold_fixed_member(h, p8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

/* `e` as an ascension, a `pm` or an `mx`: an `hj` first. */
static pico8_jo_t *as_hj(void *e) {
    const entity_class_t *c = ((const entity_t *)e)->class;
    assert(c == pico8_hj_struct_class() || c == pico8_pm_struct_class() || c == pico8_mx_struct_class());
    return e;
}

static uint32_t fold_pk(uint32_t h, const pico8_jo_t *hj) {
    return fold_bool_member(h, hj->emitter->def->pk != 0);
}

/* A particle record as `g_.mm` returns it: `p`, `v`, `l`. */
static uint32_t fold_particle(uint32_t h, const pico8_qy_t *q) {
    assert(q != NULL);
    assert(!q->has_a);
    h = fnv(fnv(h, (uint32_t)q->p.x), (uint32_t)q->p.y);
    h = fnv(fnv(h, (uint32_t)q->v.x), (uint32_t)q->v.y);
    return fnv(h, (uint32_t)q->l);
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
    const pico8_jo_t *jo = e != NULL && (cls_of(e)->lineage & LINEAGE(LINEAGE_STRUCTURE)) ? e : NULL;
    return jo != NULL && jo->def->px == &pico8_pm_px_row;
}

static int is_ga(const void *e) {
    return e != NULL && cls_of(e) == pico8_ga_struct_class() && ((const pico8_jo_t *)e)->def->fz != NULL &&
           strcmp(((const pico8_jo_t *)e)->def->fz, "burst") == 0;
}

/* The `ga` that `pm:pq` spawned: registered in `hv` just before the
 * protostar, at `ra + ng`; its `qy` and `mo.ma` as `ga:pq` left them. */
static uint32_t fold_spawned(uint32_t h, void *e) {
    const entity_list_t *hv = live_entities();
    int at = ents_rank_in(hv, e);
    const pico8_jo_t *g;
    const pico8_jo_t *pm;
    const vec_t *ra;
    const vec_t *ng;
    const vec_t *gra;

    assert(at >= 1 && cls_of(e) == pico8_pm_struct_class());
    pm = e;
    ra = pm->ra;
    ng = &pm->def->ng;
    assert(is_ga(hv->items[at - 1]));
    g = (const pico8_jo_t *)hv->items[at - 1];
    gra = g->ra;
    assert(gra != NULL && ng != NULL);
    if (ra != NULL) {
        assert(gra->x == p8_fixed_add(ra->x, ng->x) && gra->y == p8_fixed_add(ra->y, ng->y));
    } else {
        assert(gra->x == ng->x && gra->y == ng->y);
    }
    assert(gra != ra && gra != ng);
    h = fold_vec(h, g->ra);
    h = fnv(h, (uint32_t)ptrs_size(g->emitter->qy));
    assert(pico8_mo_qb() != NULL);
    return fold_fixed_member(h, pico8_mo_qb()->ma);
}

/* A protostar as `kv["pm"]`, `pm:pq`, `f_` and `jo:pq` left it: its
 * scalars, which of its own, hj's, qh's and jo's members are set, its
 * methods (hj's very functions), which default tables it shares with `pm`,
 * `qh` and `jo`, where `lt` registered it -- in `cg.jo`/`np`/`prober`,
 * never in `cg.qh` or `cg.hj` -- and the `ga` it spawned. */
static uint32_t fold_protostar(uint32_t h, void *e) {
    const pico8_jo_t *hj = as_hj(e);
    const pico8_jo_t *pm;
    const pico8_emitter_t *em = hj->emitter;
    const pico8_jo_t *qh = hj;
    const pico8_jo_t *jo = qh;
    size_t i;

    assert(cls_of(e) == pico8_pm_struct_class());
    pm = e;
    h = fold_string_member(h, jo->id);
    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_string_member(h, pm->def->fz);
    h = fold_string_member(h, hj->def->rr);
    h = fold_qh(h, qh);
    h = fold_fixed_member(h, qh->def->hp);
    h = fold_fixed_member(h, qh->s);
    h = fold_fixed_member(h, qh->probe->od);
    h = fold_fixed_member(h, em->py);
    h = fold_fixed_member(h, em->def->ki);
    h = fold_fixed_member(h, em->pn);
    h = fold_pk(h, hj);
    h = fold_fixed_member(h, em->def->fc);
    h = fold_fixed_member(h, em->du);
    h = fold_fixed_member(h, em->qo);
    h = fold_fixed_member(h, em->ro);
    h = fold_fixed_member(h, em->qg);
    h = fold_fixed_member(h, em->q_);
    h = fold_string_member(h, qh->probe->def->dq);
    h = fold_fixed_member(h, qh->probe->def->fp);
    h = fold_fixed_member(h, qh->probe->def->dw);
    h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
    h = fold_hf(h, jo);
    h = fold_fixed_member(h, jo->def->hs);
    h = fold_fixed_member(h, jo->def->fa);
    h = fold_fixed_member(h, jo->ma);
    h = fold_fixed_member(h, jo->def->ca);
    h = fold_vec(h, &pm->def->ng);
    h = fold_vec(h, &jo->def->gq);
    h = fold_vec(h, jo->ra);
    h = fold_vec(h, jo->p);
    /* The 1s of hj's ten fixed emitter defaults, of `hp`, `qh`, `od`, `fp`,
     * `dw`, `s`, then of `hf`, `hs`, `fa`, `ma` and `ca`: constant by the
     * struct's shape, kept so that BUILT_HASH still proves the typed reads. */
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(pm->def->fz != NULL));
    h = fnv(h, (uint32_t)(hj->def->rr != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(hj->emitter->k_ != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(em->qy != NULL));
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->def->px != NULL));
    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->def->ba != NULL));
    h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(jo->def->ka != NULL));
    h = fnv(h, (uint32_t)(jo->def->dz != NULL));
    h = fnv(h, (uint32_t)(jo->def->cq != NULL));
    h = fnv(h, (uint32_t)(jo->kw != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(jo->def->em != NULL));
    h = fnv(h, (uint32_t)(jo->id != NULL));
    h = fnv(h, (uint32_t)(jo->p != NULL));
    h = fnv(h, (uint32_t)(jo->ra != NULL));
    h = fnv(h, (uint32_t)(jo->oe != NULL));
    h = fnv(h, (uint32_t)(jo->hn != NULL));
    h = fnv(h, (uint32_t)(jo->ep != NULL));
    h = fnv(h, (uint32_t)(jo->jy != NULL));
    h = fnv(h, (uint32_t)(jo->nk != NULL));
    h = fnv(h, (uint32_t)(jo->fx != 0));
    h = fnv(h, (uint32_t)(jo->lq != NULL));
    h = fnv(h, (uint32_t)(jo->fo != NULL));
    h = fnv(h, (uint32_t)jo->has_jh);
    h = fnv(h, (uint32_t)(jo->p_ != 0));
    h = fnv(h, (uint32_t)jo->has_kr);
    h = fnv(h, (uint32_t)(jo->cc != NULL));
    h = fnv(h, (uint32_t)(jo->by != NULL));
    h = fnv(h, (uint32_t)jo->has_mi);
    h = fnv(h, (uint32_t)(jo->m_ != 0));
    assert(ptrs_size(em->qy) == 0);
    assert_qh_shape(cls_of(e));
    assert_members_of(cls_of(e), pico8_qh_struct_class(), 0, 1);
    assert(cls_of(e)->attach_slipway == pico8_qh_struct_class()->attach_slipway && cls_of(e)->offer_trade == pico8_qh_struct_class()->offer_trade &&
           pico8_jo_is_family(e));
    assert(cls_of(e)->spawn_particle == pico8_hj_struct_class()->spawn_particle && cls_of(e)->spawn_particle != NULL);
    assert(cls_of(e)->draw == pico8_hj_struct_class()->draw);
    assert(pm->def->ng.x == p8_fixed_from_int(1) && pm->def->ng.y == 0);
    assert(jo->header.class->tags == (TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)) && qh->def->px == &pico8_pm_px_row && em->k_ == &pico8_pm_k__seq);
    assert(jo->header.class->tags != (TAG(TAG_ASCENSION) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)));
    assert(em->k_ != &pico8_hj_k__seq);
    assert(qh->def->ba == &pico8_qh_ba_seq);
    assert(jo->def == &pico8_pm_def);
    assert(jo->def == &pico8_pm_def);
    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
           jo->def->cq == &pico8_jo_cq_seq);
    assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
    assert(pico8_gb_at(3) != NULL);
    assert(jo->hn == pico8_gb_at(3));
    h = fold_kw(h, jo);
    h = fnv(h, (uint32_t)ents_ptrs_size(jo->oe));
    h = fnv(h, (uint32_t)ptrs_size(jo->jy));
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
    assert(as_hj(a)->emitter->qy != NULL && as_hj(a)->emitter->qy != as_hj(b)->emitter->qy);
    h = fold_protostar(h, a);
    h = fold_protostar(h, b);
    check("built", h, BUILT_HASH);
}

static uint32_t fold_ra_y(uint32_t h, const pico8_jo_t *hj) {
    const vec_t *ra = hj->ra;
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the protostar. */
static uint32_t fold_dg(uint32_t h, void *e) {
    const pico8_jo_t *hj = as_hj(e);

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
    entity_list_t *ib = pico8_jo_ib(e);
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_ib(h, ib);
    h = fnv(fnv(h, 0xB0u), pico8_jo_gg(e) ? 1u : 0u);
    h = fnv(h, (uint32_t)pico8_da(e));
    return as_hj(e)->has_mi ? fold_fixed_member(h, as_hj(e)->mi) : fold_absent(h);
}

/* `dc("kj")` once: the screen it leaves and what `hj:kj` left in `qy`. */
static uint32_t fold_draw(uint32_t h, void *e) {
    draw_layers("draw");
    h = fnv(h, screen_hash());
    return fnv(h, (uint32_t)ptrs_size(as_hj(e)->emitter->qy));
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
    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
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
    pico8_ez(placed, P8_FIXED_LIT(0x00028000, 2.5));
    h = fnv(h, (uint32_t)ptrs_size(as_hj(placed)->emitter->qy));
    assert(ptrs_size(as_hj(placed)->emitter->qy) >= 2);

    h = fold_draw(h, placed);
    h = fold_draw(h, placed);
    as_hj(placed)->mi = p8_fixed_from_int(2);
    as_hj(placed)->has_mi = 1;
    for (i = 0; i < 4; i++) {
        h = fold_draw(h, placed);
    }
    h = fold_economy(h, placed);

    bare = build_protostar(NULL, NULL, "s91");
    assert(as_hj(bare)->ra == NULL && as_hj(bare)->p == NULL && is_pm(bare));
    h = fold_string_member(h, as_hj(bare)->id);
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
        pico8_jo_t *bare = pico8_pm_new(NULL);
        assert(is_a_pm_struct(bare) && bare->id == NULL && bare->ra == NULL &&
               bare->oe != NULL && ptrs_size(bare->emitter->qy) == 0 &&
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
    h.particules = pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE);
    h.qy = pico8_zone_particules_vivantes(PICO8_PARTICULES_QY);
    h.qy_fv = pico8_zone_particules_vivantes(PICO8_PARTICULES_QY_FV);
    h.rangees = pico8_zone_interface_vivantes(PICO8_SORTE_RANGEE);
    return h;
}

/* A protostar that leaves (`he`, then the exit queue's flush) gives back
 * its particles and its `qy`, not those of the burst it spawned. */
static void test_leaving_gives_back_its_particles(void) {
    held_t built;
    held_t after;
    vec_t at = vec_xy(64, 64);
    pico8_jo_t *e;

    start_game();
    pico8_cart_vider_file_sortie();
    e = as_hj(build_protostar(&at, &at, "s90"));
    pico8_ez(&e->header, p8_fixed_from_int(5));
    cls_of(e)->refresh(&e->header);
    assert(e->emitter->qy->count == 5);
    built = held_now();
    entity_unregister(&e->header);
    pico8_cart_vider_file_sortie();
    after = held_now();
    assert(after.particules == built.particules - 5 && after.qy == built.qy - 1);
    assert(e->emitter->qy == NULL && e->lq_place == NULL);
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
