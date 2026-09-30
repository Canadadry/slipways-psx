/* The infraspace gate `fr`, read and written on its struct members and the
 * generated defaults (pico8_game_data.h), built as
 * the shop builds it (`ie:ldown` on `gx[3]`, `id`) and driven through the
 * real `mv`/`dg`/`dc`. Its methods are compared to those of a slipgate built
 * in the same game, started directly: no events file. */

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
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_vec.h"


#define BUILT_HASH 0xA5047E01u
#define LIFE_HASH 0x8B3BFBB2u

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
static const char *const tags[] = {"nr", "np", "prober"};

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
    return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
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

/* The word the frozen hashes fold before a boolean, 0xB0, then the boolean
 * as 0 or 1; any other moves every *_HASH. */
static uint32_t fold_bool(uint32_t h, int b) {
    return fnv(fnv(h, 0xB0u), b ? 1u : 0u);
}

/* The item count of a list member, -1 when NULL. */
static int list_size(const pico8_ptrs_t *l) {
    return l == NULL ? -1 : l->count;
}

static int ents_list_size(const pico8_ents_t *l) {
    return l == NULL ? -1 : l->count;
}

/* `e` as a planet of qh's lineage. */
static pico8_qh_t *as_qh(void *e) {
    assert(pico8_qh_of(e) == e);
    return e;
}

static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
    return qh->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
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

/* `hz` of a slipgate or a gate; NULL on any other struct, which has none. */
static const pico8_int_seq_t *hz_of(const void *e) {
    if (cls_of(e) == pico8_nr_struct_class() || cls_of(e) == pico8_fr_struct_class()) {
        return ((const pico8_nr_t *)e)->hz;
    }
    return NULL;
}

/* `hz` is never written on an entity: one whose `hz` is the very table of
 * fr's class is an infraspace gate. */
static int is_a_gate(const void *e) {
    return hz_of(e) == &pico8_fr_hz_seq;
}

static void *build_gate(void) {
    void *e = build(pico8_fr_kind());
    assert(is_a_gate(e));
    return e;
}

/* A gate as `ie:ldown` and `jo:pq` left it: its scalars, which of its own,
 * nr's, qh's and jo's members are set, its methods (a slipgate's very
 * ones), which default tables it shares with `fr`, `nr`, `qh` and `jo`, and
 * where `lt` registered it -- in `cg.nr`/`np`/`prober`, never in `cg.jo` or
 * `cg.qh`. */
static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
    const pico8_fr_t *fr;
    const pico8_nr_t *nr;
    const pico8_qh_t *qh;
    const pico8_jo_t *jo;
    size_t i;

    assert(is_a_gate(e) && !is_a_gate(slipgate) && cls_of(e) == pico8_fr_struct_class());
    fr = e;
    nr = &fr->nr;
    qh = &nr->qh;
    jo = &qh->jo;
    h = fold_string_member(h, jo->id);
    h = fold_string_member(h, jo->header.me);
    h = fold_fixed_member(h, jo->header.t);
    h = fold_string_member(h, nr->rr);
    h = fold_fixed_member(h, fr->mt);
    h = fold_fixed_member(h, qh->hp);
    h = fold_fixed_member(h, qh->s);
    h = fold_fixed_member(h, qh->probe->od);
    h = fold_fixed_member(h, qh->probe->fp);
    h = fold_fixed_member(h, qh->probe->dw);
    h = fold_string_member(h, qh->probe->dq);
    h = fold_qh(h, qh);
    h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
    h = fold_hf(h, jo);
    h = fold_fixed_member(h, jo->hs);
    h = fold_fixed_member(h, jo->fa);
    h = fold_fixed_member(h, jo->ma);
    h = fold_fixed_member(h, jo->def->ca);
    h = fold_vec(h, &jo->def->gq);
    h = fnv(h, (uint32_t)nr->hz->count);
    /* The 1s of `mt`, of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`,
     * `hs`, `fa`, `ma` and `ca`: constant by the struct's shape, kept so that
     * BUILT_HASH still proves the typed reads. */
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(nr->hz != NULL));
    h = fnv(h, (uint32_t)(nr->rr != NULL));
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->px != NULL));
    h = fnv(h, (uint32_t)(qh->probe->dq != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->ba != NULL));
    h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(jo->kq != NULL));
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
    assert(cls_of(e)->kf == cls_of(slipgate)->kf && cls_of(e)->cn == cls_of(slipgate)->cn &&
           cls_of(e)->hq == cls_of(slipgate)->hq && cls_of(e)->ey == cls_of(slipgate)->ey);
    assert(cls_of(e)->kf != NULL && cls_of(e)->cn != NULL && cls_of(e)->hq != NULL && cls_of(e)->ey != NULL &&
           pico8_jo_is_family(e));
    assert(cls_of(e)->hd == cls_of(slipgate)->hd && cls_of(e)->kj == cls_of(slipgate)->kj &&
           cls_of(e)->ldown == cls_of(slipgate)->ldown && cls_of(e)->hb == cls_of(slipgate)->hb &&
           cls_of(e)->gv == cls_of(slipgate)->gv && cls_of(e)->hk == cls_of(slipgate)->hk &&
           cls_of(e)->ev == cls_of(slipgate)->ev);
    assert(cls_of(e)->states[PICO8_STATE_LL] == cls_of(slipgate)->states[PICO8_STATE_LL] &&
           cls_of(e)->states[PICO8_STATE_LY] == cls_of(slipgate)->states[PICO8_STATE_LY]);
    assert(cls_of(e)->hd != NULL && cls_of(e)->kj != NULL && cls_of(e)->ldown != NULL && cls_of(e)->hb != NULL &&
           cls_of(e)->gv != NULL && cls_of(e)->hk != NULL && cls_of(e)->ev != NULL &&
           cls_of(e)->states[PICO8_STATE_LL] != NULL && cls_of(e)->states[PICO8_STATE_LY] != NULL);
    assert(cls_of(e)->ol == NULL && cls_of(e)->mm == NULL);
    assert(cls_of(e)->states[PICO8_STATE_PH] == NULL && cls_of(e)->states[PICO8_STATE_KZ] == NULL &&
           cls_of(e)->states[PICO8_STATE_PR] == NULL && cls_of(e)->rdown == NULL);
    assert(nr->hz != &pico8_nr_hz_seq);
    assert(jo->kq == &pico8_nr_start.kq);
    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
    assert(qh->px == &pico8_qh_px_row && qh->ba == &pico8_qh_ba_seq);
    assert(jo->def == &pico8_fr_def);
    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
           jo->def->cq == &pico8_jo_cq_seq);
    assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
    assert(pico8_gb_at(1) != NULL);
    assert(jo->hn == pico8_gb_at(1));
    h = fnv(h, (uint32_t)ents_list_size(jo->oe));
    h = fnv(h, (uint32_t)list_size(jo->jy));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(pico8_cv(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(pico8_cg(tags[i]), e));
    }
    assert(ents_listed(pico8_cg("nr"), e) && !ents_listed(pico8_cg("jo"), e) && !ents_listed(pico8_cg("qh"), e));
    return h;
}

/* Two infraspace gates built by the shop's `ie:ldown` on `gx[3]`, around a
 * slipgate. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *slipgate;
    void *b;

    start_game();
    a = build_gate();
    slipgate = build(pico8_nr_kind());
    b = build_gate();
    assert(a != b && as_qh(a)->jo.oe != as_qh(b)->jo.oe && as_qh(a)->jo.kw != as_qh(b)->jo.kw);
    h = fold_gate(h, a, slipgate);
    h = fold_gate(h, b, slipgate);
    check("built", h, BUILT_HASH);
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = pico8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const pico8_qh_t *qh) {
    const pico8_vec_t *ra = qh->jo.ra;
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the gate. */
static uint32_t fold_dg(uint32_t h, void *e) {
    const pico8_qh_t *qh = as_qh(e);

    h = fold_ra_y(h, qh);
    pico8_dg();
    h = fold_fixed_member(h, qh->jo.header.t);
    return fold_ra_y(h, qh);
}

/* `jo:ib`'s container, folded as fold_ref folded the tagged list it was:
 * never in `hv`, so the tag branch. */
static uint32_t fold_ib(uint32_t h, const pico8_ents_t *ib) {
    if (ib == NULL) {
        return fnv(h, 0xDEADu);
    }
    return fnv(fnv(h, 0xEEEEu), FROZEN_LIST_TAG);
}

/* What `bb`'s `dh`, `ib`, `gg` and `da` give on a gate. */
static uint32_t fold_economy(uint32_t h, void *e) {
    pico8_ents_t *ib = pico8_jo_ib(e);
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_ib(h, ib);
    h = fnv(fnv(h, 0xB0u), pico8_jo_gg(e) ? 1u : 0u);
    h = fnv(h, (uint32_t)pico8_da(e));
    return as_qh(e)->jo.has_mi ? fold_fixed_member(h, as_qh(e)->jo.mi) : fold_absent(h);
}

/* A list of `n` copies of `item`, for `oe`: `gv`, `ldown` and `hb` only
 * count it. */
static pico8_ents_t *list_of(int n, pico8_struct_header_t *item) {
    pico8_ents_t *l = malloc(sizeof *l);
    int i;
    pico8_ents_init(l);
    for (i = 0; i < n; i++) {
        pico8_ents_add(l, item);
    }
    return l;
}

/* `hq` then `ey`, as the player drops a planet at `offset` from `mo`. */
static void place(void *e, int x, int y) {
    const pico8_vec_t *mo_ra;
    pico8_vec_t *mp;

    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
    assert(mo_ra != NULL);
    mp = vec_ref(pico8_vec_add(*mo_ra, vec_xy(x, y)));
    cls_of(e)->hq(e, mp, vec_ref(vec_xy(64, 64)));
    assert(as_qh(e)->jo.ra == mp);
}

/* A slipway drawn from `from` by its `ldown`, aimed by `jc:hq` at a point
 * with no planet: its `mt` (then `fa`) comes from `from` alone. */
static uint32_t fold_route(uint32_t h, void *from) {
    pico8_down_t jc = cls_of(from)->ldown(from, NULL, NULL);
    const pico8_vec_t *ra = as_qh(from)->jo.ra;
    int from_has_mt = cls_of(from) == pico8_fr_struct_class();
    const pico8_jc_t *made;

    assert(jc.stop && jc.held != NULL && cls_of(jc.held) == pico8_jc_struct_class());
    made = (const pico8_jc_t *)jc.held;
    assert(made->qu == from);
    assert(ra != NULL);
    cls_of(jc.held)->hq(jc.held, vec_ref(pico8_vec_add(*ra, vec_xy(20, 0))), NULL);
    assert(made->has_mt == from_has_mt);
    assert(made->fa == pico8_fixed_from_int(from_has_mt ? 2 : 4));
    h = made->has_mt ? fold_fixed_member(h, made->mt) : fold_absent(h);
    return fold_fixed_member(h, made->fa);
}

/* Placed on screen by `hq` (`bv`) and `ey` (`d_`); `dg` on one still in
 * "kz", then in "ly"; `kj` through `dc`; `gv` against no argument, another
 * gate and a slipway drawn from itself; `ldown` with no link (a `jc` whose
 * `qu` is the gate) and with 3 (refused); `hb` with 0 to 3 links; the `mt`
 * of a slipway drawn from a gate and from a slipgate. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *placed;
    void *idle;
    void *slipgate;
    pico8_down_t jc;
    pico8_ents_t *oe;
    int i;

    start_game();
    idle = build_gate();
    h = fold_string_member(h, as_qh(idle)->jo.header.me);
    h = fold_dg(h, idle);

    placed = build_gate();
    place(placed, 12, 108);
    set_credits(999);
    assert(cls_of(placed)->ey == pico8_d__dispatch);
    h = pico8_d_(placed) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);
    h = fold_string_member(h, as_qh(placed)->jo.header.me);
    assert(strcmp(as_qh(placed)->jo.header.me, "ly") == 0);
    h = fold_economy(h, placed);
    for (i = 0; i < 3; i++) {
        h = fold_dg(h, placed);
    }
    for (i = 0; i < 2; i++) {
        pico8_dc("kj");
        h = fnv(h, screen_hash());
    }

    h = fold_bool(h, cls_of(placed)->gv(placed, NULL).stop);
    h = fold_bool(h, cls_of(placed)->gv(placed, idle).stop);
    jc = cls_of(placed)->ldown(placed, NULL, NULL);
    assert(jc.stop && jc.held != NULL);
    assert(cls_of(jc.held) == pico8_jc_struct_class() && ((const pico8_jc_t *)jc.held)->qu == placed);
    h = fnv(h, FROZEN_STRUCT_TAG);
    h = fold_bool(h, cls_of(placed)->gv(placed, jc.held).stop);
    h = fold_bool(h, cls_of(idle)->gv(idle, jc.held).stop);

    oe = as_qh(placed)->jo.oe;
    for (i = 0; i <= 3; i++) {
        as_qh(placed)->jo.oe = list_of(i, idle);
        cls_of(placed)->hb(placed);
        h = fold_fixed_member(h, as_qh(placed)->s);
        h = fold_bool(h, cls_of(placed)->gv(placed, NULL).stop);
    }
    assert(as_qh(placed)->s == pico8_fixed_from_int(76));
    jc = cls_of(placed)->ldown(placed, NULL, NULL);
    assert(jc.held == NULL);
    h = fold_bool(h, jc.stop);
    as_qh(placed)->jo.oe = oe;
    cls_of(placed)->hb(placed);
    h = fold_fixed_member(h, as_qh(placed)->s);
    assert(as_qh(placed)->s == pico8_fixed_from_int(78));
    h = fold_economy(h, placed);

    h = fold_route(h, placed);
    assert(is_a_gate(placed) && ((const pico8_fr_t *)placed)->mt == pico8_fixed_from_int(1));
    slipgate = build(pico8_nr_kind());
    place(slipgate, 40, 108);
    h = fold_route(h, slipgate);
    assert(cls_of(slipgate) != pico8_fr_struct_class());
    check("life", h, LIFE_HASH);
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
    (void)pico8_fr_new(&keys);
}

static int is_an_fr_struct(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_fr_struct_class();
}

/* Built by `ie:ldown` or pico8_fr_new, a gate is a struct whose descriptor
 * holds nr's very entries; a construction key other than `id` stops the
 * run. Neither nr's nor qh's class installs a function any more. */
static void test_fr_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_fr_struct_class();
    const pico8_struct_class_t *nr = pico8_nr_struct_class();
    void *a;
    int j;

    start_game();
    a = build_gate();
    assert(is_an_fr_struct(a) && cls != nr && ents_listed(pico8_cg("nr"), a));
    assert(cls->kj == nr->kj && cls->ev == nr->ev && cls->hd == nr->hd && cls->hk == nr->hk && cls->hb == nr->hb &&
           cls->has_hb == nr->has_hb && cls->ldown == nr->ldown && cls->rdown == nr->rdown && cls->gv == nr->gv &&
           cls->hq == nr->hq && cls->ey == nr->ey && cls->kf == nr->kf && cls->cn == nr->cn && cls->ol == nr->ol &&
           cls->mm == nr->mm);
    for (j = 0; j < PICO8_STATE_COUNT; j++) {
        assert(cls->states[j] == nr->states[j]);
    }
    assert(pico8_fr_ready());

    must_abort(build_with_unknown_key, NULL);
    {
        pico8_fr_t *bare = pico8_fr_new(NULL);
        assert(is_an_fr_struct(bare) && is_a_gate(bare) && bare->nr.qh.jo.id == NULL && bare->nr.qh.jo.oe != NULL &&
               bare->nr.qh.jo.kw != NULL);
        assert(ents_listed(pico8_cg("nr"), bare));
    }
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
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_fr_is_a_struct);

    printf("test_pico8_fr: all tests passed\n");
    return 0;
}
