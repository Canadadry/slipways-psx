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

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy_score.h"
#include "../src/pico8_economy_tiers.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_particle.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_vec.h"


#define BUILT_HASH 0x2A676693u
#define LIFE_HASH 0x902C2D62u
#define SYNTHESIS_HASH 0x2DBF1611u

/* The tag `jo:ib`'s list carried as a tagged list, which the frozen hashes
 * fold: any other value moves every *_HASH. */
#define FROZEN_LIST_TAG 0x9E5C1157u

/* The tag entity structs carried, which the frozen hashes fold: any other
 * value moves every *_HASH. */
#define FROZEN_STRUCT_TAG 0x9E5C57C7u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"jo", "np", "prober"};

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

static int ents_rank_in(const pico8_ents_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int ents_listed(const pico8_ents_t *list, const void *e) {
    return ents_rank_in(list, e) >= 0;
}

static uint32_t fold_absent(uint32_t h) {
    return fnv(h, 0u);
}

static uint32_t fold_fixed_member(uint32_t h, pico8_fixed_t v) {
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
    return jo->def->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
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
    if (ents_listed(pico8_hv(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)ents_rank_in(pico8_hv(), ref));
    }
    return fnv(fnv(h, 0xEEEEu), FROZEN_STRUCT_TAG);
}

/* The item count of a list member, -1 when NULL. */
static int ptrs_size(const pico8_ptrs_t *l) {
    return l == NULL ? -1 : l->count;
}

static int ents_ptrs_size(const pico8_ents_t *l) {
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

static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
    return qh->jo.def->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

/* `e` as an ascension, a `pm` or an `mx`: an `hj` first. */
static pico8_hj_t *as_hj(void *e) {
    const pico8_struct_class_t *c = ((const pico8_struct_header_t *)e)->cls;
    assert(c == pico8_hj_struct_class() || c == pico8_pm_struct_class() || c == pico8_mx_struct_class());
    return e;
}

static uint32_t fold_pk(uint32_t h, const pico8_hj_t *hj) {
    return fold_fixed_member(h, hj->emitter->pk);
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

static pico8_vec_t *vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

static pico8_vec_t vec_xy(int x, int y) {
    return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
}

static const pico8_struct_class_t *cls_of(const void *e) {
    return ((const pico8_struct_header_t *)e)->cls;
}

/* The members a planet of qh's tree holds: `kj`, `ev`, `hd`, `hk`, `hb`,
 * `ldown`, `gv`, `hq`, `ey`, `kf`, `cn` and the states `ll`, `ly`; `mm`
 * aside, no other. */
static void assert_qh_shape(const pico8_struct_class_t *c) {
    assert(c->kj != NULL && c->ev != NULL && c->hd != NULL && c->hk != NULL && c->hb != NULL && c->ldown != NULL &&
           c->gv != NULL && c->states[PICO8_STATE_LL] != NULL && c->states[PICO8_STATE_LY] != NULL);
    assert(c->has_hb == NULL && c->rdown == NULL && c->states[PICO8_STATE_PH] == NULL &&
           c->states[PICO8_STATE_KZ] == NULL && c->states[PICO8_STATE_PR] == NULL);
    assert(c->hq != NULL && c->ey != NULL && c->kf != NULL && c->cn != NULL && c->ol == NULL);
}

/* `c` holds `ref`'s very members but `kf`, `cn` and `mm`, `kj` and the
 * state `ly` included when `with_kj`/`with_ly`. */
static void assert_members_of(const pico8_struct_class_t *c, const pico8_struct_class_t *ref, int with_kj,
                              int with_ly) {
    assert(c->ev == ref->ev && c->hd == ref->hd && c->hk == ref->hk && c->hb == ref->hb && c->has_hb == ref->has_hb &&
           c->ldown == ref->ldown && c->rdown == ref->rdown && c->gv == ref->gv);
    assert(c->states[PICO8_STATE_LL] == ref->states[PICO8_STATE_LL] && c->hq == ref->hq && c->ey == ref->ey);
    assert(!with_kj || c->kj == ref->kj);
    assert(!with_ly || c->states[PICO8_STATE_LY] == ref->states[PICO8_STATE_LY]);
}

/* The research icon whose `ke` is `kind`. */
static void *icon_of(const pico8_entity_kind_t *kind) {
    const pico8_ents_t *hb = pico8_cv("hb");
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
static void *build(const pico8_entity_kind_t *kind) {
    void *icon = icon_of(kind);
    pico8_down_t made;

    assert(icon != NULL);
    made = cls_of(icon)->ldown(icon, NULL, NULL);
    assert(made.stop && made.held != NULL);
    return made.held;
}

static void *build_synthesizer(void) {
    return build(pico8_mx_kind());
}

/* `mx` sets its own `px`: an entity whose `px` is the very table of mx's
 * class is a synthesizer. */
static int is_mx(const void *e) {
    const pico8_qh_t *qh = pico8_qh_of(e);
    return qh != NULL && qh->jo.def->px == &pico8_mx_px_row;
}

/* `ht` of an `mx`; NULL on any other struct, which has no `ht`. */
static const pico8_int_seq_t *ht_of(const void *e) {
    return cls_of(e) == pico8_mx_struct_class() ? ((const pico8_mx_t *)e)->ht : NULL;
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
    const pico8_struct_class_t *hj_cls = pico8_hj_struct_class();
    const pico8_hj_t *hj = as_hj(e);
    const pico8_emitter_t *em = hj->emitter;
    const pico8_qh_t *qh = &hj->qh;
    const pico8_jo_t *jo = &qh->jo;
    size_t i;

    h = fold_string_member(h, jo->id);
    h = fold_string_member(h, jo->header.me);
    h = fold_fixed_member(h, jo->header.t);
    h = fold_string_member(h, hj->qh.jo.def->rr);
    h = fold_qh(h, qh);
    h = fold_fixed_member(h, qh->jo.def->hp);
    h = fold_fixed_member(h, qh->s);
    h = fold_fixed_member(h, qh->probe->od);
    h = fold_fixed_member(h, em->py);
    h = fold_fixed_member(h, em->ki);
    h = fold_fixed_member(h, em->pn);
    h = fold_pk(h, hj);
    h = fold_fixed_member(h, em->fc);
    h = fold_fixed_member(h, em->du);
    h = fold_fixed_member(h, em->qo);
    h = fold_fixed_member(h, em->ro);
    h = fold_fixed_member(h, em->qg);
    h = fold_fixed_member(h, em->q_);
    h = fold_string_member(h, qh->probe->dq);
    h = fold_fixed_member(h, qh->probe->fp);
    h = fold_fixed_member(h, qh->probe->dw);
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
    h = fnv(h, (uint32_t)(hj->qh.jo.def->rr != NULL));
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
    h = fnv(h, (uint32_t)(qh->jo.def->px != NULL));
    h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->jo.def->ba != NULL));
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
    h = fnv(h, (uint32_t)jo->has_fx);
    h = fnv(h, (uint32_t)(jo->lq != NULL));
    h = fnv(h, (uint32_t)(jo->fo != NULL));
    h = fnv(h, (uint32_t)jo->has_jh);
    h = fnv(h, (uint32_t)jo->has_p_);
    h = fnv(h, (uint32_t)jo->has_kr);
    h = fnv(h, (uint32_t)(jo->cc != NULL));
    h = fnv(h, (uint32_t)(jo->by != NULL));
    h = fnv(h, (uint32_t)jo->has_mi);
    h = fnv(h, (uint32_t)jo->has_m_);
    assert(ptrs_size(em->qy) == 0);
    assert_qh_shape(cls_of(e));
    assert_members_of(cls_of(e), hj_cls, 1, 1);
    assert(cls_of(e)->kf != hj_cls->kf && cls_of(e)->kf != pico8_jo_struct_class()->kf);
    assert(cls_of(e)->cn == hj_cls->cn && cls_of(e)->mm == hj_cls->mm && cls_of(e)->mm != NULL &&
           pico8_jo_is_family(e));
    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->jo.def->px == &pico8_mx_px_row && ht_of(e) == &pico8_mx_ht_seq &&
           em->k_ == &pico8_mx_k__seq);
    assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
    assert(em->k_ != &pico8_hj_k__seq);
    assert(qh->jo.def->ba == &pico8_qh_ba_seq);
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
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(pico8_cv(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(pico8_cg(tags[i]), e));
    }
    assert(ents_listed(pico8_cg("jo"), e) && !ents_listed(pico8_cg("qh"), e) && !ents_listed(pico8_cg("hj"), e));
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
    pico8_fm_hy()->nz = pico8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const pico8_hj_t *hj) {
    const pico8_vec_t *ra = hj->qh.jo.ra;
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the synthesizer. */
static uint32_t fold_dg(uint32_t h, void *e) {
    const pico8_hj_t *hj = as_hj(e);

    h = fold_ra_y(h, hj);
    pico8_dg();
    h = fold_fixed_member(h, hj->qh.jo.header.t);
    return fold_ra_y(h, hj);
}

/* `jo:ib`'s container, folded as fold_ref folded the tagged list it was:
 * never in `hv`, so the tag branch. */
static uint32_t fold_ib(uint32_t h, const pico8_ents_t *ib) {
    if (ib == NULL) {
        return fnv(h, 0xDEADu);
    }
    return fnv(fnv(h, 0xEEEEu), FROZEN_LIST_TAG);
}

/* What `bb`'s `dh`, `ib`, `gg` and `da` (qh false) give on a synthesizer. */
static uint32_t fold_economy(uint32_t h, void *e) {
    pico8_ents_t *ib = pico8_jo_ib(e);
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_ib(h, ib);
    h = fnv(fnv(h, 0xB0u), pico8_jo_gg(e) ? 1u : 0u);
    h = fnv(h, (uint32_t)pico8_da(e));
    return as_hj(e)->qh.jo.has_mi ? fold_fixed_member(h, as_hj(e)->qh.jo.mi) : fold_absent(h);
}

/* `dc("kj")` once: the screen it leaves and what `hj:kj` left in `qy`. */
static uint32_t fold_draw(uint32_t h, void *e) {
    pico8_dc("kj");
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
    pico8_vec_t *mp;
    const pico8_vec_t *mo_ra;
    int i;

    start_game();
    idle = build_synthesizer();
    h = fold_string_member(h, as_hj(idle)->qh.jo.header.me);
    h = fold_dg(h, idle);

    placed = build_synthesizer();
    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
    assert(mo_ra != NULL);
    mp = vec_ref(pico8_vec_add(*mo_ra, vec_xy(12, 108)));
    cls_of(placed)->hq(placed, mp, vec_ref(vec_xy(64, 64)));
    assert(as_hj(placed)->qh.jo.ra == mp);
    set_credits(200);
    assert(cls_of(placed)->ey == pico8_d__dispatch);
    h = pico8_d_(placed) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);
    h = fold_string_member(h, as_hj(placed)->qh.jo.header.me);
    assert(strcmp(as_hj(placed)->qh.jo.header.me, "ly") == 0);

    h = fold_economy(h, placed);
    for (i = 0; i < 3; i++) {
        h = fold_dg(h, placed);
    }

    h = fold_particle(h, cls_of(placed)->mm(placed));
    pico8_ez(placed, PICO8_FIXED_LIT(0x00028000, 2.5));
    h = fnv(h, (uint32_t)ptrs_size(as_hj(placed)->emitter->qy));
    assert(ptrs_size(as_hj(placed)->emitter->qy) >= 2);

    h = fold_draw(h, placed);
    h = fold_draw(h, placed);
    as_hj(placed)->qh.jo.mi = pico8_fixed_from_int(2);
    as_hj(placed)->qh.jo.has_mi = 1;
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
    cls_of(jc)->ey(jc, synthesizer);
    assert(ents_listed(as_hj(synthesizer)->qh.jo.oe, jc));
    return jc;
}

/* `mx:kf` reads the other end's `jy`: one row per resource, `gc` its
 * count of suppliers. */
static void give_row(void *e, int qe) {
    pico8_fixed_t want_qe = pico8_fixed_from_int(qe);
    pico8_jy_t *row = pico8_jy_new(&want_qe);
    pico8_ptrs_t *jy = malloc(sizeof *jy);

    row->has_gc = 1;
    row->gc = pico8_fixed_from_int(0);
    pico8_ptrs_init(jy);
    pico8_ptrs_add(jy, row);
    as_hj(e)->qh.jo.jy = jy;
}

static uint32_t fold_kf(uint32_t h, void *e) {
    const pico8_jo_t *jo = &as_hj(e)->qh.jo;

    h = fnv(h, (uint32_t)ht_is_the_class_one(e));
    h = fnv(h, (uint32_t)ht_size(e));
    h = fold_kw(h, jo);
    h = fold_fixed_member(h, jo->ma);
    h = jo->has_fx ? fold_bool_member(h, jo->fx) : fold_absent(h);
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
    kw = as_hj(s)->qh.jo.kw;
    assert(kw->has_qe && kw->qe == pico8_fixed_from_int(20));
    assert(pico8_mx_ht_seq.count == 4);
    assert(!pico8_jo_shared_kw()->has_qe || pico8_jo_shared_kw()->qe != pico8_fixed_from_int(20));
    h = fold_kf(h, s);
    fresh = ht_of(s);

    give_row(home, 21);
    (void)join(home, s);
    assert(ht_of(s) == fresh && ht_size(s) == 0);
    kw = as_hj(s)->qh.jo.kw;
    assert(kw->has_qe && kw->qe == pico8_fixed_from_int(20));
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
    pico8_jo_keys_t keys;
    (void)e;
    keys.id = NULL;
    keys.p = vec_ref(vec_xy(1, 1));
    keys.ra = NULL;
    (void)pico8_mx_new(&keys);
}

static int is_an_mx_struct(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_mx_struct_class();
}

/* Built by `ie:ldown` or pico8_mx_new, a synthesizer is a struct whose
 * descriptor holds hj's very entries but its own `kf`; a construction key
 * other than `id` stops the run. No class of mx's chain installs a function
 * any more. */
static void test_mx_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_mx_struct_class();
    const pico8_struct_class_t *hj = pico8_hj_struct_class();
    void *a;

    start_game();
    a = build_synthesizer();
    assert(is_an_mx_struct(a) && cls != hj && ents_listed(pico8_cg("jo"), a) && !ents_listed(pico8_cg("hj"), a));
    assert(cls->kf != hj->kf && cls->kf != pico8_jo_struct_class()->kf);
    assert_qh_shape(cls);
    assert_members_of(cls, hj, 1, 1);
    assert(cls->cn == hj->cn && cls->mm == hj->mm);
    assert(pico8_mx_ready());
    assert(pico8_hj_ready());

    must_abort(build_with_unknown_key, NULL);
    {
        pico8_mx_t *bare = pico8_mx_new(NULL);
        assert(is_an_mx_struct(bare) && bare->hj.qh.jo.id == NULL && bare->hj.qh.jo.oe != NULL &&
               bare->hj.qh.jo.kw != NULL && ptrs_size(bare->hj.emitter->qy) == 0 && ents_listed(pico8_cg("jo"), bare));
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
    in_fresh_child(test_synthesis);
    in_fresh_child(test_mx_is_a_struct);

    printf("test_pico8_mx: all tests passed\n");
    return 0;
}
