/* The ascension `hj`, read and written on its struct members and the
 * generated defaults (pico8_game_data.h), and driven through the real
 * `ie:ldown`/`mv`/`dg`/`dc`, in games started directly: no events file. */

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
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy_score.h"
#include "../src/pico8_economy_tiers.h"
#include "../src/pico8_entity.h"
#include "p8/fixed.h"
#include "../src/pico8_fm.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "../src/pico8_particle.h"
#include "../src/pico8_zone_interface.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_vec.h"

/* The cart keeps the box of `mp`, the port a copy of it (nil stays nil). */
static int ra_copies(const pico8_vec_t *ra, const pico8_vec_t *mp) {
    return mp == NULL ? ra == NULL : ra != NULL && ra != mp && ra->x == mp->x && ra->y == mp->y;
}


/* The tag class instances carried, which the frozen hashes fold: any
 * other value moves every *_HASH. */
#define FROZEN_INSTANCE_TAG 0x9E5C1451u

/* The tag entity structs carried, which the frozen hashes fold: any other
 * value moves every *_HASH. */
#define FROZEN_STRUCT_TAG 0x9E5C57C7u

#define BUILT_HASH 0x3F7BD05Au
#define LIFE_HASH 0xD3BC6FC2u
#define HEIRS_HASH 0xB8D3FBCCu

/* The tag `jo:ib`'s list carried as a tagged list, which the frozen hashes
 * fold: any other value moves every *_HASH. */
#define FROZEN_LIST_TAG 0x9E5C1157u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"hj", "jo", "np", "prober"};

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

static int is_bv_target(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_bv_et_struct_class();
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (ents_listed(pico8_hv(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)ents_rank_in(pico8_hv(), ref));
    }
    /* The frozen hashes folded the INSTANCE tag the throwaway had. */
    if (is_bv_target(ref)) {
        return fnv(fnv(h, 0xEEEEu), FROZEN_INSTANCE_TAG);
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

static uint32_t fold_qh(uint32_t h, const pico8_jo_t *qh) {
    return qh->def->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

/* 1 no target, 3 a target: the bytes the frozen hashes fold. */
static uint32_t et_state(const pico8_jo_t *qh) {
    return qh->probe->et == NULL ? 1u : 3u;
}

/* `et` as qh's reader gave it: false or the target. */
static uint32_t fold_et(uint32_t h, const pico8_jo_t *qh) {
    switch (et_state(qh)) {
    case 1u:
        return fold_bool_member(h, 0);
    default:
        return fold_ref(fnv(h, 1u), qh->probe->et);
    }
}

/* `e` as an ascension, a `pm` or an `mx`: an `hj` first. */
static pico8_jo_t *as_hj(void *e) {
    const pico8_struct_class_t *c = ((const pico8_struct_header_t *)e)->cls;
    assert(c == pico8_hj_struct_class() || c == pico8_pm_struct_class() || c == pico8_mx_struct_class());
    return e;
}

static uint32_t fold_pk(uint32_t h, const pico8_jo_t *hj) {
    return hj->header.cls == pico8_pm_struct_class() ? fold_bool_member(h, hj->emitter->def->pk != 0)
                                                           : fold_fixed_member(h, hj->emitter->def->pk);
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

/* `c` holds qh's very members, but its own `kj` and `mm` (`mm` checked
 * apart). */
static void assert_qh_members_but_kj(const pico8_struct_class_t *c) {
    const pico8_struct_class_t *qh = pico8_qh_struct_class();
    int j;

    assert(c->kj != NULL && c->kj != qh->kj);
    assert(c->ev == qh->ev && c->hd == qh->hd && c->hk == qh->hk && c->hb == qh->hb && c->has_hb == qh->has_hb &&
           c->ldown == qh->ldown && c->rdown == qh->rdown && c->gv == qh->gv);
    assert(c->ev != NULL && c->hd != NULL && c->hk != NULL && c->hb != NULL && c->ldown != NULL && c->gv != NULL);
    assert(c->hq == qh->hq && c->ey == qh->ey && c->kf == qh->kf && c->cn == qh->cn && c->ol == NULL);
    assert(c->hq != NULL && c->ey != NULL && c->kf != NULL && c->cn != NULL);
    for (j = 0; j < PICO8_STATE_COUNT; j++) {
        assert(c->states[j] == qh->states[j]);
    }
    assert(c->states[PICO8_STATE_LL] != NULL && c->states[PICO8_STATE_LY] != NULL);
    assert(c->rdown == NULL && c->states[PICO8_STATE_PH] == NULL && c->states[PICO8_STATE_KZ] == NULL &&
           c->states[PICO8_STATE_PR] == NULL);
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

static void *build_ascension(void) {
    return build(pico8_hj_kind());
}

/* `pm` and `mx` each set their own `px`: an entity whose `px` is the very
 * row `px` is one of that kind. */
static int is_of(const void *e, const pico8_jb_row_t *px) {
    const pico8_jo_t *jo = e != NULL && (cls_of(e)->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) ? e : NULL;
    return jo != NULL && jo->def->px == px;
}

/* An ascension as `ie:ldown`, `f_` and `jo:pq` left it: its scalars (the 12
 * emitter defaults among them), which of its own, qh's and jo's members
 * are set, its methods (qh's chain's very functions but `kj`, its own, and
 * `mm`, `g_`'s), which default tables it shares with `hj`, `qh` and `jo`,
 * and where `lt` registered it -- in `cg.hj`, never in `cg.qh`. */
static uint32_t fold_ascension(uint32_t h, void *e) {
    const pico8_jo_t *hj = as_hj(e);
    const pico8_emitter_t *em = hj->emitter;
    const pico8_jo_t *qh = hj;
    const pico8_jo_t *jo = qh;
    size_t i;

    h = fold_string_member(h, jo->id);
    h = fold_string_member(h, jo->header.me);
    h = fold_fixed_member(h, jo->header.t);
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
    /* The 1s of the ten fixed emitter defaults, of `hp`, `qh`, `od`, `fp`,
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
    assert_qh_members_but_kj(cls_of(e));
    assert(cls_of(e)->kj == pico8_hj_struct_class()->kj && pico8_jo_is_family(e));
    assert(cls_of(e)->mm == pico8_g__mm);
    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)) && qh->def->px == &pico8_hj_px_row && em->k_ == &pico8_hj_k__seq);
    assert(jo->header.cls->tags != (PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
    assert(qh->def->px != &pico8_qh_px_row);
    assert(qh->def->ba == &pico8_qh_ba_seq);
    assert(jo->def == &pico8_hj_def);
    assert(jo->def == &pico8_hj_def);
    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
           jo->def->cq == &pico8_jo_cq_seq);
    assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
    assert(pico8_gb_at(2) != NULL);
    assert(jo->hn == pico8_gb_at(2));
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
    assert(ents_listed(pico8_cg("hj"), e) && !ents_listed(pico8_cg("qh"), e));
    return h;
}

/* Two ascensions built by the shop's `ie:ldown` on `gx[6]`, each with a
 * `qy` of its own. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game();
    a = build_ascension();
    b = build_ascension();
    assert(a != b && is_of(a, &pico8_hj_px_row) && is_of(b, &pico8_hj_px_row));
    assert(as_hj(a)->emitter->qy != NULL && as_hj(a)->emitter->qy != as_hj(b)->emitter->qy);
    h = fold_ascension(h, a);
    h = fold_ascension(h, b);
    check("built", h, BUILT_HASH);
}

static pico8_fixed_t credits(void) {
    assert(pico8_fm_hy() != NULL);
    return pico8_fm_hy()->nz;
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = pico8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const pico8_jo_t *hj) {
    const pico8_vec_t *ra = hj->ra;
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the ascension. */
static uint32_t fold_dg(uint32_t h, void *e) {
    const pico8_jo_t *hj = as_hj(e);

    h = fold_ra_y(h, hj);
    pico8_dg();
    h = fold_fixed_member(h, hj->header.t);
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

/* What `bb`'s `dh` (no `pt.nl`, `pt.hf` false), `ib`, `gg` and `da` (qh
 * false) give on an ascension. */
static uint32_t fold_economy(uint32_t h, void *e) {
    pico8_ents_t *ib = pico8_jo_ib(e);
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_ib(h, ib);
    h = fnv(fnv(h, 0xB0u), pico8_jo_gg(e) ? 1u : 0u);
    h = fnv(h, (uint32_t)pico8_da(e));
    return as_hj(e)->has_mi ? fold_fixed_member(h, as_hj(e)->mi) : fold_absent(h);
}

/* `dc("kj")` once: the screen it leaves and what `hj:kj` left in `qy`. */
static uint32_t fold_draw(uint32_t h, void *e) {
    pico8_dc("kj");
    h = fnv(h, screen_hash());
    return fnv(h, (uint32_t)ptrs_size(as_hj(e)->emitter->qy));
}

/* `hq` (`bv`) on screen then `ey` (`d_`) with the credits; `bb`/`ib`/`gg`/
 * `da`; `dg` in "ly" (qh's `ly`, `na`); `mm` (`g_.mm`, `pk` true) and `ez`;
 * `kj` through `dc` with `mi`=1, then `mi`=2 (`ei` moves and draws the
 * particles); `dg` on an ascension still in "kz". */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *placed;
    void *idle;
    pico8_vec_t *mp;
    const pico8_vec_t *mo_ra;
    int i;

    start_game();

    placed = build_ascension();
    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
    assert(mo_ra != NULL);
    mp = vec_ref(pico8_vec_add(*mo_ra, vec_xy(12, 108)));
    cls_of(placed)->hq(placed, mp, vec_ref(vec_xy(64, 64)));
    assert(ra_copies(as_hj(placed)->ra, mp));
    h = fold_vec(h, as_hj(placed)->p);
    h = fnv(h, et_state(as_hj(placed)));
    h = fold_et(h, as_hj(placed));
    set_credits(200);
    assert(cls_of(placed)->ey == pico8_d__dispatch);
    h = pico8_d_(placed) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);
    h = fold_string_member(h, as_hj(placed)->header.me);
    h = fold_fixed_member(h, as_hj(placed)->header.t);
    h = fnv(h, (uint32_t)credits());
    assert(strcmp(as_hj(placed)->header.me, "ly") == 0);

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
    as_hj(placed)->mi = pico8_fixed_from_int(2);
    as_hj(placed)->has_mi = 1;
    for (i = 0; i < 4; i++) {
        h = fold_draw(h, placed);
    }
    h = fold_economy(h, placed);

    idle = build_ascension();
    h = fold_string_member(h, as_hj(idle)->header.me);
    h = fold_dg(h, idle);
    check("life", h, LIFE_HASH);
}

/* A `pm` as `jo:hu` builds it (`p`, `ra`, `id`) or an `mx` by `ie:ldown`
 * on `gx[5]`, each a struct under its own descriptor holding hj's `kj` and
 * `mm`. */
static uint32_t fold_heir(uint32_t h, void *e, const pico8_struct_class_t *cls) {
    const pico8_jo_t *hj;

    assert(e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == cls);
    hj = as_hj(e);
    assert(is_of(e, cls == pico8_pm_struct_class() ? &pico8_pm_px_row : &pico8_mx_px_row));
    assert(cls_of(e)->kj == pico8_hj_struct_class()->kj);
    assert(cls_of(e)->mm == pico8_hj_struct_class()->mm);
    assert(ptrs_size(hj->emitter->qy) == 0);
    assert(hj->def->qh == 0);
    h = fold_string_member(h, hj->id);
    h = fold_string_member(h, hj->header.me);
    h = fold_qh(h, hj);
    h = fold_string_member(h, hj->def->rr);
    h = fold_fixed_member(h, hj->probe->od);
    h = fold_pk(h, hj);
    h = fold_fixed_member(h, hj->def->hp);
    h = fold_fixed_member(h, hj->s);
    h = fold_fixed_member(h, hj->emitter->py);
    h = fold_fixed_member(h, hj->emitter->pn);
    h = fold_fixed_member(h, hj->emitter->du);
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("hj"), e));
    return fold_particle(h, cls_of(e)->mm(e));
}

static void test_heirs(void) {
    uint32_t h = 2166136261u;
    pico8_jo_keys_t keys;
    void *pm;
    void *mx;

    start_game();
    mx = build(pico8_mx_kind());
    keys.p = vec_ref(vec_xy(40, 50));
    keys.ra = vec_ref(vec_xy(7700, 7600));
    keys.id = "s77";
    pm = pico8_pm_new(&keys);

    assert(strcmp(as_hj(pm)->def->rr, "ascension") == 0 && as_hj(pm)->emitter->def->pk == 0);
    assert(as_hj(pm)->probe->od == pico8_fixed_from_int(60));
    assert(strcmp(as_hj(mx)->def->rr, "void synthesis") == 0);
    h = fold_heir(h, pm, pico8_pm_struct_class());
    h = fold_heir(h, mx, pico8_mx_struct_class());
    check("heirs", h, HEIRS_HASH);
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
    (void)pico8_hj_new(&keys);
}

static int is_an_hj_struct(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_hj_struct_class();
}

/* Built by `ie:ldown` or pico8_hj_new, an ascension is a struct whose
 * descriptor holds qh's very entries but its own `kj` and `g_`'s `mm`; a
 * construction key other than `id` stops the run. No other descriptor but
 * pm's and mx's has an `mm`. */
static void test_hj_is_a_struct(void) {
    const pico8_struct_class_t *const no_mm[] = {pico8_jo_struct_class(), pico8_qh_struct_class(),
                                                 pico8_nl_struct_class(), pico8_os_struct_class()};
    const pico8_struct_class_t *cls = pico8_hj_struct_class();
    const pico8_struct_class_t *qh = pico8_qh_struct_class();
    void *a;
    size_t i;

    start_game();
    a = build_ascension();
    assert(is_an_hj_struct(a) && cls != qh && ents_listed(pico8_cg("hj"), a) && !ents_listed(pico8_cg("qh"), a));
    assert_qh_members_but_kj(cls);
    assert(cls->mm == pico8_g__mm && qh->mm == NULL);
    for (i = 0; i < sizeof no_mm / sizeof no_mm[0]; i++) {
        assert(no_mm[i]->mm == NULL);
    }

    must_abort(build_with_unknown_key, NULL);
    {
        pico8_jo_t *bare = pico8_hj_new(NULL);
        assert(is_an_hj_struct(bare) && bare->id == NULL && bare->oe != NULL &&
               ptrs_size(bare->emitter->qy) == 0 && ents_listed(pico8_cg("hj"), bare));
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

/* An ascension that leaves (`he`, then the exit queue's flush) gives back
 * its particles, its `qy` and its row, and keeps its struct. */
static void test_leaving_gives_back_its_particles(void) {
    held_t before;
    held_t after;
    pico8_jo_t *e;

    start_game();
    pico8_cart_vider_file_sortie();
    before = held_now();
    e = as_hj(build_ascension());
    pico8_ez(&e->header, pico8_fixed_from_int(5));
    cls_of(e)->hb(&e->header);
    assert(e->emitter->qy->count == 5);
    assert(held_now().particules == before.particules + 5 && held_now().qy == before.qy + 1);
    pico8_he(&e->header);
    pico8_cart_vider_file_sortie();
    after = held_now();
    assert(after.particules == before.particules && after.qy == before.qy && after.rangees == before.rangees);
    assert(e->emitter->qy == NULL && e->lq_place == NULL);
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
    in_fresh_child(test_heirs);
    in_fresh_child(test_hj_is_a_struct);
    in_fresh_child(test_leaving_gives_back_its_particles);

    printf("test_pico8_hj: all tests passed\n");
    return 0;
}
