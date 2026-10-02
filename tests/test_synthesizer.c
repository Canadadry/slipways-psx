/* The synthesizer `mx`, read and written on its struct members and the
 * generated defaults (pico8_game_data.h), built as the shop builds it (`ie:ldown` on
 * `gx[5]`, `id`) and driven through the real `mv`/`dg`/`dc` and a slipway's
 * `ey`, in games started directly: no events file. */

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
#include "game/particle.h"
#include "game/ui_zone.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/slipway.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/scroll.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "engine/game_zone.h"
#include "heap_allocator.h"

/* The cart keeps the box of `mp`, the port a copy of it (nil stays nil). */
static int ra_copies(const vec_t *ra, const vec_t *mp) {
    return mp == NULL ? ra == NULL : ra != NULL && ra != mp && ra->x == mp->x && ra->y == mp->y;
}


#define BUILT_HASH 0xB935BCCBu
#define LIFE_HASH 0xF71C0563u
#define SYNTHESIS_HASH 0x2DBF1611u

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
    return fold_fixed_member(h, hj->emitter->def->pk);
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

/* The research icon whose `ke` is `kind`. */
static void *icon_of(const entity_kind_t *kind) {
    const entity_list_t *hb = entities_by_message("refresh");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i];
        if (cls_of(e) == pico8_ie_struct_class() && ((const pico8_ie_t *)e)->ke == kind) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the icon of `kind`, as the shop does. */
static void *build(const entity_kind_t *kind) {
    void *icon = icon_of(kind);
    down_t made;

    assert(icon != NULL);
    made = cls_of(icon)->left_press(icon, NULL, NULL);
    assert(made.stop && made.held != NULL);
    return made.held;
}

static void *build_synthesizer(void) {
    return build(pico8_mx_kind());
}

/* `mx` sets its own `px`: an entity whose `px` is the very table of mx's
 * class is a synthesizer. */
static int is_mx(const void *e) {
    const pico8_jo_t *jo = e != NULL && (cls_of(e)->lineage & LINEAGE(LINEAGE_STRUCTURE)) ? e : NULL;
    return jo != NULL && jo->def->px == &pico8_mx_px_row;
}

/* `ht` of an `mx`; NULL on any other struct, which has no `ht`. */
static const pico8_int_seq_t *ht_of(const void *e) {
    return cls_of(e) == pico8_mx_struct_class() ? ((const pico8_jo_t *)e)->ht : NULL;
}

/* The entry count of the `ht` table, -1 when absent. */
static int ht_size(const void *e) {
    const pico8_int_seq_t *ht = ht_of(e);
    return ht == NULL ? -1 : ht->count;
}

static int ht_is_the_class_one(const void *e) {
    return ht_of(e) == &pico8_mx_ht_seq;
}

/* A synthesizer as `ie:ldown`, `f_` and `jo:pq` left it: its scalars (the
 * 12 emitter defaults among them), which of its own, hj's, qh's and jo's
 * members are set, its methods (hj's very functions but its own `kf`), which
 * default tables it shares with `mx`, `qh` and `jo`, and where `lt`
 * registered it -- in `cg.jo`/`np`/`prober`, never in `cg.qh` or `cg.hj`. */
static uint32_t fold_synthesizer(uint32_t h, void *e) {
    const entity_class_t *hj_cls = pico8_hj_struct_class();
    const pico8_jo_t *hj = as_hj(e);
    const pico8_emitter_t *em = hj->emitter;
    const pico8_jo_t *qh = hj;
    const pico8_jo_t *jo = qh;
    size_t i;

    h = fold_string_member(h, jo->id);
    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
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
    h = fold_vec(h, &jo->def->gq);
    h = fnv(h, (uint32_t)(ht_of(e) != NULL));
    h = fnv(h, (uint32_t)ht_size(e));
    /* The 1s of hj's ten fixed emitter defaults, of `hp`, `qh`, `od`, `fp`,
     * `dw`, `s`, then of `hf`, `hs`, `fa`, `ma` and `ca`: constant by the
     * struct's shape, kept so that BUILT_HASH still proves the typed reads. */
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
    assert_members_of(cls_of(e), hj_cls, 1, 1);
    assert(cls_of(e)->attach_slipway != hj_cls->attach_slipway && cls_of(e)->attach_slipway != pico8_jo_struct_class()->attach_slipway);
    assert(cls_of(e)->offer_trade == hj_cls->offer_trade && cls_of(e)->spawn_particle == hj_cls->spawn_particle && cls_of(e)->spawn_particle != NULL &&
           pico8_jo_is_family(e));
    assert(jo->header.class->tags == (TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)) && qh->def->px == &pico8_mx_px_row && ht_of(e) == &pico8_mx_ht_seq &&
           em->k_ == &pico8_mx_k__seq);
    assert(jo->header.class->tags != (TAG(TAG_ASCENSION) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)));
    assert(em->k_ != &pico8_hj_k__seq);
    assert(qh->def->ba == &pico8_qh_ba_seq);
    assert(jo->def == &pico8_mx_def);
    assert(jo->def == &pico8_mx_def);
    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
           jo->def->cq == &pico8_jo_cq_seq);
    assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
    assert(pico8_gb_at(5) != NULL);
    assert(jo->hn == pico8_gb_at(5));
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
    return h;
}

/* Two synthesizers built by the shop's `ie:ldown` on `gx[5]`, each with a
 * `qy` of its own. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game();
    a = build_synthesizer();
    b = build_synthesizer();
    assert(a != b && is_mx(a) && is_mx(b));
    assert(as_hj(a)->emitter->qy != NULL && as_hj(a)->emitter->qy != as_hj(b)->emitter->qy);
    h = fold_synthesizer(h, a);
    h = fold_synthesizer(h, b);
    check("built", h, BUILT_HASH);
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = p8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const pico8_jo_t *hj) {
    const vec_t *ra = hj->ra;
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the synthesizer. */
static uint32_t fold_dg(uint32_t h, void *e) {
    const pico8_jo_t *hj = as_hj(e);

    h = fold_ra_y(h, hj);
    update_entities();
    h = fold_fixed_member(h, hj->header.state_time);
    return fold_ra_y(h, hj);
}

/* `jo:ib`'s container, folded as fold_ref folded the tagged list it was:
 * never in `hv`, so the tag branch. */
static uint32_t fold_ib(uint32_t h, const entity_list_t *ib) {
    if (ib == NULL) {
        return fnv(h, 0xDEADu);
    }
    return fnv(fnv(h, 0xEEEEu), FROZEN_LIST_TAG);
}

/* What `bb`'s `dh`, `ib`, `gg` and `da` (qh false) give on a synthesizer. */
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

/* Placed on screen by `hq` (`bv`) and `ey` (`d_`); `dg` on one still in
 * "kz" (no `kz` method: nothing moves), then in "ly" (qh's `ly`, `na`);
 * `mm` (`g_.mm`, `pk` true) and `ez`; `kj` through `dc` with `mi`=1, then
 * `mi`=2 (`ei` moves and draws the particles); `dh`/`ib`/`gg`/`da`. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *placed;
    void *idle;
    vec_t *mp;
    const vec_t *mo_ra;
    int i;

    start_game();
    idle = build_synthesizer();
    h = fold_string_member(h, as_hj(idle)->header.state);
    h = fold_dg(h, idle);

    placed = build_synthesizer();
    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
    assert(mo_ra != NULL);
    mp = vec_ref(vec_add(*mo_ra, vec_xy(12, 108)));
    cls_of(placed)->drag(placed, mp, vec_ref(vec_xy(64, 64)));
    assert(ra_copies(as_hj(placed)->ra, mp));
    set_credits(200);
    assert(cls_of(placed)->drop == pico8_d__dispatch);
    h = pico8_d_(placed) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);
    h = fold_string_member(h, as_hj(placed)->header.state);
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
    check("life", h, LIFE_HASH);
}

/* A slipway from `qu` to the synthesizer, joined by its real `ey`: `jc:ey`
 * charges the player, then runs `kf` on both ends. */
static void *join(void *qu, void *synthesizer) {
    pico8_jc_keys_t jc_keys = {0};
    void *jc;

    jc_keys.qu = qu;
    jc = pico8_jc_new(&jc_keys);
    assert(jc != NULL);
    set_credits(999);
    cls_of(jc)->drop(jc, synthesizer);
    assert(ents_listed(as_hj(synthesizer)->oe, jc));
    return jc;
}

/* `mx:kf` reads the other end's `jy`: one row per resource, `gc` its
 * count of suppliers. */
static void give_row(void *e, int qe) {
    p8_fixed_t want_qe = p8_fixed_from_int(qe);
    pico8_jy_t *row = pico8_jy_new(game_zone(), &want_qe);
    pointer_list_t *jy = malloc(sizeof *jy);

    row->has_gc = 1;
    row->gc = p8_fixed_from_int(0);
    pointer_list_init(jy, &heap_allocator);
    pointer_list_add(jy, row);
    as_hj(e)->jy = jy;
}

static uint32_t fold_kf(uint32_t h, void *e) {
    const pico8_jo_t *jo = as_hj(e);

    h = fnv(h, (uint32_t)ht_is_the_class_one(e));
    h = fnv(h, (uint32_t)ht_size(e));
    h = fold_kw(h, jo);
    h = fold_fixed_member(h, jo->ma);
    h = (jo->fx != 0) ? fold_bool_member(h, jo->fx) : fold_absent(h);
    return fnv(h, (uint32_t)ents_ptrs_size(jo->oe));
}

/* `mx:kf` on a slipway from an ascension whose `jy` lacks a resource of `ht`: nothing
 * but `jo:kf`; then on one whose unsupplied row is `qe`=20, one of `ht`:
 * `kw.qe` takes it and `ht` becomes a fresh empty table, the class one
 * untouched; a later slipway finds `ht` empty and leaves it as it is. A
 * second synthesizer still starts from the class `ht`. */
static void test_synthesis(void) {
    uint32_t h = 2166136261u;
    void *home;
    void *s;
    void *other;
    const void *fresh;
    const pico8_kw_t *kw;

    start_game();
    home = build(pico8_hj_kind());
    s = build_synthesizer();
    assert(ht_is_the_class_one(s) && ht_size(s) == 4);

    give_row(home, 7);
    (void)join(home, s);
    assert(ht_is_the_class_one(s));
    h = fold_kf(h, s);
    h = fold_kf(h, home);

    give_row(home, 20);
    (void)join(home, s);
    assert(!ht_is_the_class_one(s) && ht_size(s) == 0);
    kw = as_hj(s)->kw;
    assert(kw->has_qe && kw->qe == p8_fixed_from_int(20));
    assert(pico8_mx_ht_seq.count == 4);
    assert(!pico8_jo_shared_kw()->has_qe || pico8_jo_shared_kw()->qe != p8_fixed_from_int(20));
    h = fold_kf(h, s);
    fresh = ht_of(s);

    give_row(home, 21);
    (void)join(home, s);
    assert(ht_of(s) == fresh && ht_size(s) == 0);
    kw = as_hj(s)->kw;
    assert(kw->has_qe && kw->qe == p8_fixed_from_int(20));
    h = fold_kf(h, s);

    other = build_synthesizer();
    assert(ht_is_the_class_one(other) && ht_size(other) == 4);
    h = fold_kf(h, other);
    check("synthesis", h, SYNTHESIS_HASH);
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

static void build_with_unknown_key(void *e) {
    planet_keys_t keys;
    (void)e;
    keys.planet_id = NULL;
    keys.map_pos = vec_ref(vec_xy(1, 1));
    keys.pos = NULL;
    (void)pico8_mx_new(&keys);
}

static int is_an_mx_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_mx_struct_class();
}

/* Built by `ie:ldown` or pico8_mx_new, a synthesizer is a struct whose
 * descriptor holds hj's very entries but its own `kf`; a construction key
 * other than `id` stops the run. No class of mx's chain installs a function
 * any more. */
static void test_mx_is_a_struct(void) {
    const entity_class_t *cls = pico8_mx_struct_class();
    const entity_class_t *hj = pico8_hj_struct_class();
    void *a;

    start_game();
    a = build_synthesizer();
    assert(is_an_mx_struct(a) && cls != hj && ents_listed(entities_by_tag("planet"), a) && !ents_listed(entities_by_tag("ascension"), a));
    assert(cls->attach_slipway != hj->attach_slipway && cls->attach_slipway != pico8_jo_struct_class()->attach_slipway);
    assert_qh_shape(cls);
    assert_members_of(cls, hj, 1, 1);
    assert(cls->offer_trade == hj->offer_trade && cls->spawn_particle == hj->spawn_particle);
    assert(pico8_mx_ready());
    assert(pico8_hj_ready());

    must_abort(build_with_unknown_key, NULL);
    {
        pico8_jo_t *bare = pico8_mx_new(NULL);
        assert(is_an_mx_struct(bare) && bare->id == NULL && bare->oe != NULL &&
               bare->kw != NULL && ptrs_size(bare->emitter->qy) == 0 &&
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

/* A synthesizer that leaves (`he`, then the exit queue's flush) gives
 * back its particles, its `qy` and its row, and keeps its struct. */
static void test_leaving_gives_back_its_particles(void) {
    held_t before;
    held_t after;
    pico8_jo_t *e;

    start_game();
    pico8_cart_vider_file_sortie();
    before = held_now();
    e = as_hj(build_synthesizer());
    pico8_ez(&e->header, p8_fixed_from_int(5));
    cls_of(e)->refresh(&e->header);
    assert(e->emitter->qy->count == 5);
    assert(held_now().particules == before.particules + 5 && held_now().qy == before.qy + 1);
    entity_unregister(&e->header);
    pico8_cart_vider_file_sortie();
    after = held_now();
    assert(after.particules == before.particules && after.qy == before.qy && after.rangees == before.rangees);
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
    in_fresh_child(test_synthesis);
    in_fresh_child(test_mx_is_a_struct);
    in_fresh_child(test_leaving_gives_back_its_particles);

    printf("test_pico8_mx: all tests passed\n");
    return 0;
}
