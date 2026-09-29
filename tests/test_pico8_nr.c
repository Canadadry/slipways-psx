/* The slipgate `nr`, read and written on its struct members and the
 * generated defaults (pico8_game_data.h), built as the shop builds it (`ie:ldown` on `gx[2]`, `id`) and driven
 * through the real `mv`/`dg`/`dc`, in games started directly: no events file. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
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
#include "../src/pico8_list.h"
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
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"


#define BUILT_HASH 0x4194DF7Bu
#define LIFE_HASH 0x6D8FAD96u

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

static int rank_in(const pico8_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i].kind == PICO8_VALUE_REF && list->items[i].as.ref == e) {
            return i;
        }
    }
    return -1;
}

static int listed(const pico8_list_t *list, const void *e) {
    return rank_in(list, e) >= 0;
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
    return jo->hf_is_bool ? fold_bool_member(h, jo->hf != 0) : fold_fixed_member(h, jo->hf);
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
    if (listed(pico8_hv(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)rank_in(pico8_hv(), ref));
    }
    return fnv(fnv(h, 0xEEEEu), (uint32_t)pico8_type_tag_of(ref));
}

static uint32_t fold_value(uint32_t h, pico8_value_t v) {
    switch (v.kind) {
    case PICO8_VALUE_BOOL:
        return fnv(fnv(h, 0xB0u), (uint32_t)v.as.boolean);
    case PICO8_VALUE_FIXED:
        return fnv(fnv(h, 0xF1u), (uint32_t)v.as.fixed);
    case PICO8_VALUE_STRING:
        return fnv_str(fnv(h, 0x57u), v.as.string);
    case PICO8_VALUE_REF:
        return fold_ref(h, v.as.ref);
    default:
        return fnv(h, 0xF0u);
    }
}

/* The item count of a list member, -1 when NULL. */
static int list_size(const pico8_list_t *l) {
    return l == NULL ? -1 : l->count;
}

/* `e` as a planet of qh's lineage. */
static pico8_qh_t *as_qh(void *e) {
    assert(pico8_qh_of(e) == e);
    return e;
}

static uint32_t fold_qh(uint32_t h, const pico8_qh_t *qh) {
    return qh->qh_is_bool ? fold_bool_member(h, qh->qh != 0) : fold_fixed_member(h, qh->qh);
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

static pico8_value_t vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return pico8_value_ref(v);
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

/* The research icon whose `ke` is `kind`. */
static void *icon_of(const pico8_entity_kind_t *kind) {
    const pico8_list_t *hb = pico8_cv("hb");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i].as.ref;
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

static void *build_slipgate(void) {
    return build(pico8_nr_kind());
}

enum { NR, FR };

/* `hz` is never written on an entity: one whose `hz` is the very table of
 * nr's class is a slipgate, of fr's class an infraspace gate. -1 for
 * anything else. */
static int gate_kind(const void *e) {
    const void *hz = NULL;

    if (e != NULL && (cls_of(e) == pico8_nr_struct_class() || cls_of(e) == pico8_fr_struct_class())) {
        hz = ((const pico8_nr_t *)e)->hz;
    }
    if (hz == NULL) {
        return -1;
    }
    if (hz == &pico8_nr_hz_seq) {
        return NR;
    }
    if (hz == &pico8_fr_hz_seq) {
        return FR;
    }
    return -1;
}

/* A slipgate as `ie:ldown` and `jo:pq` left it: its scalars, which of its
 * own, qh's and jo's members are set, its methods (qh's very functions but
 * its own `gv`, `ldown`, `hb`, `cn`), which default tables it shares with
 * `nr`, `qh` and `jo`, and where `lt` registered it -- in
 * `cg.nr`/`np`/`prober`, never in `cg.jo` or `cg.qh`. */
static uint32_t fold_slipgate(uint32_t h, const void *e) {
    const pico8_struct_class_t *qh_cls = pico8_qh_struct_class();
    const pico8_nr_t *nr;
    const pico8_qh_t *qh;
    const pico8_jo_t *jo;
    size_t i;

    assert(gate_kind(e) == NR && cls_of(e) == pico8_nr_struct_class());
    nr = e;
    qh = &nr->qh;
    jo = &qh->jo;
    h = fold_string_member(h, jo->id);
    h = fold_string_member(h, jo->ke.me);
    h = fold_fixed_member(h, jo->ke.t);
    h = fold_string_member(h, nr->rr);
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
    h = fold_fixed_member(h, jo->ca);
    h = fold_vec(h, jo->gq);
    h = fnv(h, (uint32_t)nr->hz->count);
    /* The 1s of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`, `hs`, `fa`,
     * `ma` and `ca`: constant by the struct's shape, kept so that BUILT_HASH
     * still proves the typed reads. */
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
    h = fnv(h, (uint32_t)qh->probe->has_et);
    h = fnv(h, (uint32_t)(jo->nu != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(jo->kq != NULL));
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(jo->ka != NULL));
    h = fnv(h, (uint32_t)(jo->dz != NULL));
    h = fnv(h, (uint32_t)(jo->cq != NULL));
    h = fnv(h, (uint32_t)(jo->kw != NULL));
    h = fnv(h, (uint32_t)(jo->gq != NULL));
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(jo->em != NULL));
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
    assert(cls_of(e)->cn == pico8_nr_struct_class()->cn && cls_of(e)->cn != qh_cls->cn &&
           cls_of(e)->cn != pico8_jo_struct_class()->cn);
    assert(cls_of(e)->kf == qh_cls->kf && cls_of(e)->hq == qh_cls->hq && cls_of(e)->ey == qh_cls->ey &&
           cls_of(e)->mm == NULL && pico8_jo_is_family(e));
    assert_qh_shape(cls_of(e));
    assert(cls_of(e)->kj == qh_cls->kj && cls_of(e)->ev == qh_cls->ev && cls_of(e)->hd == qh_cls->hd &&
           cls_of(e)->hk == qh_cls->hk);
    assert(cls_of(e)->states[PICO8_STATE_LL] == qh_cls->states[PICO8_STATE_LL] &&
           cls_of(e)->states[PICO8_STATE_LY] == qh_cls->states[PICO8_STATE_LY]);
    assert(cls_of(e)->gv != qh_cls->gv && cls_of(e)->ldown != qh_cls->ldown && cls_of(e)->hb != qh_cls->hb);
    assert(cls_of(e)->gv != pico8_jo_struct_class()->gv && cls_of(e)->ldown != pico8_jo_struct_class()->ldown &&
           cls_of(e)->hb != pico8_jo_struct_class()->hb);
    assert(jo->kq == &pico8_nr_start.kq);
    assert(jo->nu == &pico8_nr_nu_tags && nr->hz == &pico8_nr_hz_seq);
    assert(jo->nu != &pico8_qh_nu_tags);
    assert(jo->kq != &pico8_jo_start.kq);
    assert(qh->px == &pico8_qh_px_row && qh->ba == &pico8_qh_ba_seq);
    assert(jo->gq == &pico8_jo_start.gq);
    assert(jo->em == &pico8_jo_em_row && jo->ka == &pico8_jo_ka_seq && jo->dz == pico8_jo_dz_rows &&
           jo->cq == &pico8_jo_cq_seq);
    assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
    assert(pico8_gb_at(1) != NULL);
    assert(jo->hn == pico8_gb_at(1));
    h = fnv(h, (uint32_t)list_size(jo->oe));
    h = fnv(h, (uint32_t)list_size(jo->jy));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cv(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cg(tags[i]), e));
    }
    assert(listed(pico8_cg("nr"), e) && !listed(pico8_cg("jo"), e) && !listed(pico8_cg("qh"), e));
    return h;
}

/* Two slipgates built by the shop's `ie:ldown` on `gx[2]`. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game();
    a = build_slipgate();
    b = build_slipgate();
    assert(a != b && as_qh(a)->jo.oe != as_qh(b)->jo.oe && as_qh(a)->jo.kw != as_qh(b)->jo.kw);
    h = fold_slipgate(h, a);
    h = fold_slipgate(h, b);
    check("built", h, BUILT_HASH);
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = pico8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const pico8_qh_t *qh) {
    const pico8_vec_t *ra = qh->jo.ra;
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the slipgate. */
static uint32_t fold_dg(uint32_t h, void *e) {
    const pico8_qh_t *qh = as_qh(e);

    h = fold_ra_y(h, qh);
    pico8_dg();
    h = fold_fixed_member(h, qh->jo.ke.t);
    return fold_ra_y(h, qh);
}

/* What `bb`'s `dh`, `ib`, `gg` and `da` give on a slipgate. */
static uint32_t fold_economy(uint32_t h, void *e) {
    pico8_value_t ib = pico8_value_ref(pico8_jo_ib(e));
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_value(h, ib);
    h = fold_value(h, pico8_jo_gg(e));
    h = fnv(h, (uint32_t)pico8_da(e));
    return as_qh(e)->jo.has_mi ? fold_fixed_member(h, as_qh(e)->jo.mi) : fold_absent(h);
}

/* A list of `n` copies of `item`, for `oe`: `gv`, `ldown` and `hb` only
 * count it. */
static pico8_list_t *list_of(int n, void *item) {
    pico8_list_t *l = malloc(sizeof *l);
    int i;

    pico8_list_init(l);
    for (i = 0; i < n; i++) {
        pico8_list_add(l, pico8_value_ref(item));
    }
    return l;
}

/* Placed on screen by `hq` (`bv`) and `ey` (`d_`); `dg` on one still in
 * "kz", then in "ly"; `kj` through `dc`; `gv` against no argument, another
 * slipgate and a slipway drawn from itself; `ldown` with no link (a `jc`
 * whose `qu` is the slipgate) and with 3 (refused); `hb` with 0 to 3
 * links; an `fr`'s `hb`; `pico8_nr_new` with no key, then `p` and `ra`. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *placed;
    void *idle;
    void *fr;
    void *bare;
    pico8_value_t mp;
    pico8_down_t jc;
    pico8_list_t *oe;
    const pico8_vec_t *mo_ra;
    int i;

    start_game();
    idle = build_slipgate();
    h = fold_string_member(h, as_qh(idle)->jo.ke.me);
    h = fold_dg(h, idle);

    placed = build_slipgate();
    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
    assert(mo_ra != NULL);
    mp = vec_ref(pico8_vec_add(*mo_ra, vec_xy(12, 108)));
    cls_of(placed)->hq(placed, mp.as.ref, vec_ref(vec_xy(64, 64)).as.ref);
    assert(as_qh(placed)->jo.ra == mp.as.ref);
    set_credits(200);
    assert(cls_of(placed)->ey == pico8_d__dispatch);
    h = fold_value(h, pico8_d_(placed));
    h = fold_string_member(h, as_qh(placed)->jo.ke.me);
    assert(strcmp(as_qh(placed)->jo.ke.me, "ly") == 0);
    h = fold_economy(h, placed);
    for (i = 0; i < 3; i++) {
        h = fold_dg(h, placed);
    }
    for (i = 0; i < 2; i++) {
        pico8_dc("kj");
        h = fnv(h, screen_hash());
    }

    h = fold_value(h, pico8_value_bool(cls_of(placed)->gv(placed, NULL).stop));
    h = fold_value(h, pico8_value_bool(cls_of(placed)->gv(placed, idle).stop));
    jc = cls_of(placed)->ldown(placed, NULL, NULL);
    assert(jc.stop && jc.held != NULL);
    assert(cls_of(jc.held) == pico8_jc_struct_class() && ((const pico8_jc_t *)jc.held)->qu == placed);
    h = fnv(h, (uint32_t)pico8_type_tag_of(jc.held));
    h = fold_value(h, pico8_value_bool(cls_of(placed)->gv(placed, jc.held).stop));
    h = fold_value(h, pico8_value_bool(cls_of(idle)->gv(idle, jc.held).stop));

    oe = as_qh(placed)->jo.oe;
    for (i = 0; i <= 3; i++) {
        as_qh(placed)->jo.oe = list_of(i, idle);
        cls_of(placed)->hb(placed);
        h = fold_fixed_member(h, as_qh(placed)->s);
        h = fold_value(h, pico8_value_bool(cls_of(placed)->gv(placed, NULL).stop));
    }
    assert(as_qh(placed)->s == pico8_fixed_from_int(168));
    jc = cls_of(placed)->ldown(placed, NULL, NULL);
    assert(jc.held == NULL);
    h = fold_value(h, pico8_value_bool(jc.stop));
    as_qh(placed)->jo.oe = oe;
    cls_of(placed)->hb(placed);
    h = fold_fixed_member(h, as_qh(placed)->s);
    h = fold_economy(h, placed);

    fr = build(pico8_fr_kind());
    assert(gate_kind(fr) == FR);
    h = fold_fixed_member(h, as_qh(fr)->s);
    cls_of(fr)->hb(fr);
    h = fold_fixed_member(h, as_qh(fr)->s);
    assert(as_qh(fr)->s == pico8_fixed_from_int(78));

    bare = pico8_nr_new(NULL);
    assert(gate_kind(bare) == NR && as_qh(bare)->jo.id == NULL && as_qh(bare)->jo.oe != NULL &&
           as_qh(bare)->jo.kw != NULL);
    h = fnv(h, (uint32_t)rank_in(pico8_cg("nr"), bare));
    {
        pico8_jo_keys_t keys;
        pico8_value_t p = vec_ref(vec_xy(3, 4));
        pico8_value_t ra = vec_ref(vec_xy(5, 6));
        void *with;
        keys.id = NULL;
        keys.p = p.as.ref;
        keys.ra = ra.as.ref;
        with = pico8_nr_new(&keys);
        assert(as_qh(with)->jo.p == p.as.ref && as_qh(with)->jo.ra == ra.as.ref && as_qh(with)->jo.id == NULL);
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), with));
    }
    check("life", h, LIFE_HASH);
}

static int is_an_nr_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_nr_struct_class();
}

/* Built by `ie:ldown` or pico8_nr_new, a slipgate is a struct whose
 * descriptor holds qh's very entries but its own `gv`, `ldown`, `hb`, `cn`,
 * which no class of its chain installs any more. */
static void test_nr_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_nr_struct_class();
    const pico8_struct_class_t *qh = pico8_qh_struct_class();
    void *a;
    void *fr;

    start_game();
    a = build_slipgate();
    assert(is_an_nr_struct(a) && cls != qh && listed(pico8_cg("nr"), a));
    assert(cls->cn != qh->cn && cls->kf == qh->kf && cls->hq == qh->hq && cls->ey == qh->ey && cls->mm == qh->mm);
    assert_qh_shape(cls);
    assert(cls->kj == qh->kj && cls->ev == qh->ev && cls->hd == qh->hd && cls->hk == qh->hk);
    assert(cls->states[PICO8_STATE_LL] == qh->states[PICO8_STATE_LL] &&
           cls->states[PICO8_STATE_LY] == qh->states[PICO8_STATE_LY]);
    assert(cls->gv != qh->gv && cls->ldown != qh->ldown && cls->hb != qh->hb);
    assert(cls->gv != pico8_jo_struct_class()->gv && cls->ldown != pico8_jo_struct_class()->ldown &&
           cls->hb != pico8_jo_struct_class()->hb);
    assert(pico8_nr_ready() && pico8_fr_ready());
    {
        pico8_nr_t *bare = pico8_nr_new(NULL);
        assert(is_an_nr_struct(bare) && bare->qh.jo.id == NULL && bare->qh.jo.oe != NULL && bare->qh.jo.kw != NULL);
    }
    fr = build(pico8_fr_kind());
    assert(pico8_type_tag_of(fr) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)fr)->cls == pico8_fr_struct_class());
    cls_of(fr)->hb(fr);
    assert(as_qh(fr)->s == pico8_fixed_from_int(78));
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
    in_fresh_child(test_nr_is_a_struct);

    printf("test_pico8_nr: all tests passed\n");
    return 0;
}
