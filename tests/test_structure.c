/* The laboratory `qh`, read on its struct members and the generated
 * defaults (pico8_game_data.h), and driven through the real
 * `ie:ldown`/`mv`/`dg`. Its descendants are read the same way. Games are
 * started directly: no events file. */

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
#include "game/background.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "game/economy_score.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "game/help.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/probe.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "game/wormhole.h"

/* The cart keeps the box of `mp`, the port a copy of it (nil stays nil). */
static int ra_copies(const pico8_vec_t *ra, const pico8_vec_t *mp) {
    return mp == NULL ? ra == NULL : ra != NULL && ra != mp && ra->x == mp->x && ra->y == mp->y;
}


/* The tag class instances carried, which the frozen hashes fold: any
 * other value moves every *_HASH. */
#define FROZEN_INSTANCE_TAG 0x9E5C1451u

#define BUILT_HASH 0xA67C1FC6u
#define LIFE_HASH 0x0ECCCC0Au
#define DESCENDANTS_HASH 0x19708F66u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"qh", "jo", "np", "prober"};

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

/* `e` as a planet of qh's lineage. */
static pico8_jo_t *as_qh(void *e) {
    const entity_t *h = e;

    assert(h != NULL && (h->class->lineage & LINEAGE(LINEAGE_STRUCTURE)));
    return e;
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

static uint32_t fold_qh(uint32_t h, const pico8_jo_t *qh) {
    return qh->def->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

/* 0 absent, 1 false, 2 true. */
static uint32_t p__state(const pico8_jo_t *jo) {
    return jo->p_ ? 2u : 0u;
}

/* 1 no target, 3 a target: the bytes the frozen hashes fold. */
static uint32_t et_state(const pico8_jo_t *qh) {
    return qh->probe->et == NULL ? 1u : 3u;
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
           ((const entity_t *)e)->class == pico8_bv_et_struct_class();
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (ents_listed(pico8_hv(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)ents_rank_in(pico8_hv(), ref));
    }
    /* The frozen hashes folded the INSTANCE tag the throwaway had. */
    assert(is_bv_target(ref) && "no test folds an entity outside hv that is not the throwaway target: to fold one, fold 0xEEEE then the frozen struct tag 0x9E5C57C7, as test_pico8_fr.c does");
    return fnv(fnv(h, 0xEEEEu), FROZEN_INSTANCE_TAG);
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

/* The item count of a list member, -1 when NULL. */
static int list_size(const pico8_ptrs_t *l) {
    return l == NULL ? -1 : l->count;
}

static int ents_list_size(const pico8_ents_t *l) {
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

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
    pico8_cart_vider_file_sortie();
}

static pico8_vec_t *vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

static pico8_vec_t vec_xy(int x, int y) {
    return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
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

/* The research icon whose `ke` is `gx[0]`, the laboratory. */
static void *qh_icon(void) {
    const pico8_ents_t *hb = pico8_cv("hb");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i];
        if (cls_of(e) == pico8_ie_struct_class() && ((const pico8_ie_t *)e)->ke == pico8_qh_kind()) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the laboratory's icon, as the shop does. */
static void *build_lab(void) {
    void *icon = qh_icon();
    down_t made;

    assert(icon != NULL);
    made = cls_of(icon)->left_press(icon, NULL, NULL);
    assert(made.stop && made.held != NULL);
    return made.held;
}

/* A laboratory as `ie:ldown` and `jo:pq` left it: its scalars, which of
 * qh's and jo's members are set, its methods (the very functions of the
 * class chain), which default tables it shares with `qh` and `jo`, and
 * where `lt` registered it. */
static uint32_t fold_lab(uint32_t h, void *e) {
    const pico8_jo_t *qh = as_qh(e);
    const pico8_jo_t *jo = qh;
    size_t i;

    h = fold_string_member(h, jo->id);
    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_fixed_member(h, qh->def->hp);
    h = fold_string_member(h, qh->probe->def->dq);
    h = fold_qh(h, qh);
    h = fold_fixed_member(h, qh->probe->od);
    h = fold_fixed_member(h, qh->probe->def->fp);
    h = fold_fixed_member(h, qh->probe->def->dw);
    h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
    h = fold_fixed_member(h, qh->s);
    h = fold_hf(h, jo);
    h = fold_fixed_member(h, jo->def->hs);
    h = fold_fixed_member(h, jo->def->fa);
    h = fold_fixed_member(h, jo->ma);
    h = fold_fixed_member(h, jo->def->ca);
    h = fold_vec(h, &jo->def->gq);
    /* The 1s of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`, `hs`, `fa`,
     * `ma` and `ca`: constant by the struct's shape, kept so that BUILT_HASH
     * still proves the typed reads. */
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
    assert_qh_shape(cls_of(e));
    assert_members_of(cls_of(e), pico8_qh_struct_class(), 1, 1);
    assert(cls_of(e)->attach_slipway == pico8_qh_struct_class()->attach_slipway && cls_of(e)->offer_trade == pico8_qh_struct_class()->offer_trade &&
           cls_of(e)->spawn_particle == NULL && pico8_jo_is_family(e));
    assert(jo->header.class->tags == (TAG(TAG_STRUCTURE) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)) && qh->def->px == &pico8_qh_px_row && qh->def->ba == &pico8_qh_ba_seq);
    assert(jo->def == &pico8_qh_def);
    assert(jo->def == &pico8_qh_def);
    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
           jo->def->cq == &pico8_jo_cq_seq);
    assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
    assert(pico8_gb_at(1) != NULL);
    assert(jo->hn == pico8_gb_at(1));
    h = fold_kw(h, jo);
    h = fnv(h, (uint32_t)ents_list_size(jo->oe));
    h = fnv(h, (uint32_t)list_size(jo->jy));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(pico8_cv(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(pico8_cg(tags[i]), e));
    }
    return h;
}

/* Two laboratories built by the shop's `ie:ldown` on `gx[0]`. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game();
    a = build_lab();
    b = build_lab();
    assert(a != b);
    h = fold_lab(h, a);
    h = fold_lab(h, b);
    check("built", h, BUILT_HASH);
}

static pico8_fixed_t credits(void) {
    assert(pico8_fm_hy() != NULL);
    return pico8_fm_hy()->nz;
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = pico8_fixed_from_int(n);
}

/* What `bv` left on a laboratory: `ra` (a copy of what was passed), `p`, and
 * what `et` is. */
static uint32_t fold_held(uint32_t h, void *e, const pico8_vec_t *mp) {
    const pico8_jo_t *qh = as_qh(e);

    assert(ra_copies(qh->ra, mp));
    h = fold_vec(h, qh->p);
    h = fnv(h, et_state(qh));
    return fold_et(h, qh);
}

static uint32_t fold_ra_y(uint32_t h, void *e) {
    const pico8_vec_t *ra = as_qh(e)->ra;
    assert(ra != NULL);
    return fnv(h, (uint32_t)ra->y);
}

/* `hq` (`bv`) at an empty point, in the panel and out of it, then next to
 * a planet; `ey` (`d_`) with and without the credits; `dg` on a laboratory
 * in "ly" (`qh.ly` = `na`) and on one still in "kz". */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *out_lab;
    void *in_lab;
    void *poor_lab;
    void *planet;
    pico8_vec_t *mp;
    const pico8_vec_t *planet_p;
    const pico8_vec_t *ra_before;
    pico8_fixed_t before;
    pico8_fixed_t in_y;
    int i;

    start_game();

    out_lab = build_lab();
    mp = vec_ref(vec_xy(7680, 7680));
    cls_of(out_lab)->drag(out_lab, mp, vec_ref(vec_xy(0, 0)));
    h = fold_held(h, out_lab, mp);
    assert(et_state(as_qh(out_lab)) == 3 && !ents_listed(pico8_hv(), as_qh(out_lab)->probe->et));

    planet = pico8_cg("jo")->items[0];
    assert(pico8_jo_is_family(planet));
    planet_p = ((const pico8_jo_t *)planet)->p;
    assert(planet_p != NULL);
    mp = vec_ref(pico8_vec_add(pico8_vec_mul(*planet_p, pico8_fixed_from_int(128)), vec_xy(6, 0)));
    cls_of(out_lab)->drag(out_lab, mp, vec_ref(vec_xy(64, 64)));
    h = fold_held(h, out_lab, mp);
    assert(as_qh(out_lab)->probe->et == planet &&
           ((const pico8_jo_t *)planet)->fo == out_lab);

    poor_lab = build_lab();
    mp = vec_ref(vec_xy(7680, 7680));
    cls_of(poor_lab)->drag(poor_lab, mp, vec_ref(vec_xy(64, 64)));
    h = fold_held(h, poor_lab, mp);
    assert(et_state(as_qh(poor_lab)) == 1);
    set_credits(3);
    assert(cls_of(poor_lab)->drop == pico8_d__dispatch);
    h = pico8_d_(poor_lab) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);
    h = fold_string_member(h, as_qh(poor_lab)->header.state);
    h = fold_fixed_member(h, as_qh(poor_lab)->header.state_time);
    h = fnv(h, p__state(as_qh(poor_lab)));
    h = fnv(h, (uint32_t)credits());
    assert(p__state(as_qh(poor_lab)) == 2 && credits() == pico8_fixed_from_int(3));

    in_lab = build_lab();
    mp = vec_ref(vec_xy(7680, 7552));
    cls_of(in_lab)->drag(in_lab, mp, vec_ref(vec_xy(64, 64)));
    set_credits(100);
    assert(cls_of(in_lab)->drop == pico8_d__dispatch);
    h = pico8_d_(in_lab) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);
    h = fold_string_member(h, as_qh(in_lab)->header.state);
    h = fold_fixed_member(h, as_qh(in_lab)->header.state_time);
    h = fnv(h, p__state(as_qh(in_lab)));
    h = fnv(h, (uint32_t)credits());
    assert(strcmp(as_qh(in_lab)->header.state, "ly") == 0 && credits() == pico8_fixed_from_int(100 - 15));
    in_y = as_qh(in_lab)->ra->y;

    ra_before = as_qh(out_lab)->ra;
    before = ra_before->y;
    for (i = 0; i < 4; i++) {
        h = fold_ra_y(h, in_lab);
        pico8_dg();
        h = fold_fixed_member(h, as_qh(in_lab)->header.state_time);
        assert(as_qh(out_lab)->ra == ra_before && ra_before->y == before);
    }
    h = fold_ra_y(h, in_lab);
    assert(as_qh(in_lab)->ra->y != in_y);
    h = fnv(h, (uint32_t)ents_listed(pico8_hv(), poor_lab));
    check("life", h, LIFE_HASH);
}

enum { NL, OS, HJ, PM, MX, NR, FR, DESCENDANTS };

static int descendant_of(const void *e) {
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_nl_struct_class()) {
        return NL;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_os_struct_class()) {
        return OS;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_hj_struct_class()) {
        return HJ;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_pm_struct_class()) {
        return PM;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_mx_struct_class()) {
        return MX;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_nr_struct_class()) {
        return NR;
    }
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_fr_struct_class()) {
        return FR;
    }
    return -1;
}

/* What a descendant inherits from `qh`: `dq`, `fp`, `dw`, `hf`, whether
 * its tags/`px`/`ba` are qh's own, `hq`/`ev`/`ey` (qh's functions),
 * and its `ly` called once (`ra.y` before, after, then put back). */
static uint32_t fold_descendant(uint32_t h, void *d) {
    pico8_jo_t *qh = as_qh(d);
    pico8_jo_t *nl = cls_of(d) == pico8_nl_struct_class() ? (pico8_jo_t *)d : NULL;
    pico8_fixed_t qw = nl != NULL ? nl->qw : 0;

    h = fold_string_member(h, qh->probe->def->dq);
    h = fold_fixed_member(h, qh->probe->def->fp);
    h = fold_fixed_member(h, qh->probe->def->dw);
    h = fold_hf(h, qh);
    h = qh->has_mi ? fold_fixed_member(h, qh->mi) : fold_absent(h);
    h = fnv(h, (uint32_t)(qh->header.class->tags == (TAG(TAG_STRUCTURE) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING))));
    h = fnv(h, (uint32_t)(qh->def->px == &pico8_qh_px_row));
    h = fnv(h, (uint32_t)(qh->def->ba == &pico8_qh_ba_seq));
    assert(cls_of(d)->drag == pico8_qh_struct_class()->drag && cls_of(d)->drop == pico8_qh_struct_class()->drop);
    assert(cls_of(d)->draw_overlay == pico8_qh_struct_class()->draw_overlay);
    if (qh->ra != NULL) {
        pico8_vec_t *v = qh->ra;
        pico8_fixed_t y = v->y;
        h = fnv(fnv(h, 1u), (uint32_t)y);
        cls_of(d)->states[ENTITY_STATE_SETTLED](d, pico8_fixed_from_int(50));
        h = fnv(h, (uint32_t)v->y);
        v->y = y;
    } else {
        cls_of(d)->states[ENTITY_STATE_SETTLED](d, pico8_fixed_from_int(50));
        h = fnv(h, 0u);
    }
    h = nl != NULL ? fold_fixed_member(h, nl->qw) : fold_absent(h);
    if (nl != NULL) {
        nl->qw = qw;
    }
    return h;
}

/* The shop's `ie:ldown` on the icon of `kind`, as the player buys it, then
 * dragged by `hq` to the middle of the screen. */
static void *bought(const entity_kind_t *kind) {
    const pico8_ents_t *hb = pico8_cv("hb");
    const pico8_qb_t *mo = pico8_mo_qb();
    const pico8_vec_t *mo_ra;
    int i;

    assert(mo != NULL && mo->ra != NULL);
    mo_ra = mo->ra;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *icon = hb->items[i];
        if (cls_of(icon) == pico8_ie_struct_class() && ((const pico8_ie_t *)icon)->ke == kind) {
            down_t made = cls_of(icon)->left_press(icon, NULL, NULL);
            assert(made.stop && made.held != NULL);
            cls_of(made.held)->drag(made.held, vec_ref(pico8_vec_add(*mo_ra, vec_xy(64, 64))),
                                  vec_ref(vec_xy(64, 64)));
            return made.held;
        }
    }
    assert(0 && "no icon for that kind");
    return NULL;
}

/* The project of `gb` whose `cw` names `cw`. */
static pico8_hp_t *cw_project(const char *cw) {
    int k;

    for (k = 1;; k++) {
        pico8_hp_t *hp = pico8_gb_at(k);
        assert(hp != NULL);
        if (hp->cw != NULL && strcmp(hp->cw, cw) == 0) {
            return hp;
        }
    }
}

/* Each descendant by the road a game takes to it: the six of the shop
 * bought from their icons, `pm` turned from a generated planet by `jo:hu`
 * on the collapse project (the newest entity of `hv`). */
static void test_descendants(void) {
    static const struct {
        int d;
        const entity_kind_t *(*kind)(void);
    } shop[] = {{OS, pico8_os_kind}, {NR, pico8_nr_kind}, {FR, pico8_fr_kind},
                {NL, pico8_nl_kind}, {MX, pico8_mx_kind}, {HJ, pico8_hj_kind}};
    void *made[DESCENDANTS] = {NULL};
    uint32_t h = 2166136261u;
    void *p;
    size_t i;

    start_game();
    for (i = 0; i < sizeof shop / sizeof shop[0]; i++) {
        made[shop[i].d] = bought(shop[i].kind());
    }
    p = pico8_cg("jo")->items[0];
    pico8_jo_hh(p);
    pico8_jo_hu(p, cw_project("pm"));
    made[PM] = pico8_hv()->items[pico8_hv()->count - 1];
    for (i = 0; i < DESCENDANTS; i++) {
        assert(made[i] != NULL && descendant_of(made[i]) == (int)i && ents_listed(pico8_hv(), made[i]));
        h = fold_descendant(fnv(h, (uint32_t)i), made[i]);
    }
    check("descendants", h, DESCENDANTS_HASH);
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
    (void)pico8_qh_new(&keys);
}

static int is_a_qh_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_qh_struct_class();
}

/* Built by `ie:ldown` or pico8_qh_new, a `qh` is a struct whose descriptor
 * holds jo's very dispatchers and qh's own `hq`, `ev`, `ey`, `ly`; a
 * construction key other than `id` stops the run. Every descendant is a
 * struct under its own descriptor. */
static void test_qh_is_a_struct(void) {
    const entity_kind_t *const descendants[] = {pico8_nl_kind(), pico8_os_kind(), pico8_hj_kind(), pico8_pm_kind(),
                                                      pico8_mx_kind(), pico8_nr_kind(), pico8_fr_kind()};
    const entity_class_t *cls = pico8_qh_struct_class();
    void *a;
    size_t i;

    start_game();
    a = build_lab();
    assert(is_a_qh_struct(a) && ents_listed(pico8_cg("qh"), a));
    assert(pico8_da(a) == pico8_fixed_from_int(pico8_cg("qh")->count));
    assert(cls->attach_slipway == pico8_jo_struct_class()->attach_slipway && cls->offer_trade == pico8_jo_struct_class()->offer_trade);
    assert_qh_shape(cls);
    assert(cls->hit_test == pico8_jo_struct_class()->hit_test && cls->draw == pico8_jo_struct_class()->draw &&
           cls->left_press == pico8_jo_struct_class()->left_press && cls->refresh == pico8_jo_struct_class()->refresh &&
           cls->accept_drop == pico8_jo_struct_class()->accept_drop && cls->new_year == pico8_jo_struct_class()->new_year &&
           cls->states[ENTITY_STATE_VANISHING] == pico8_jo_struct_class()->states[ENTITY_STATE_VANISHING]);
    assert(cls->draw_overlay != pico8_jo_struct_class()->draw_overlay && cls->states[ENTITY_STATE_SETTLED] != NULL);
    assert(cls->drag != pico8_jo_struct_class()->drag && cls->drop != pico8_jo_struct_class()->drop && cls->spawn_particle == NULL);

    must_abort(build_with_unknown_key, NULL);
    {
        pico8_jo_t *bare = pico8_qh_new(NULL);
        assert(is_a_qh_struct(bare) && bare->id == NULL && bare->oe != NULL && ents_listed(pico8_cg("qh"), bare));
    }
    for (i = 0; i < sizeof descendants / sizeof descendants[0]; i++) {
        planet_keys_t keys;
        void *d;
        keys.planet_id = "s99";
        keys.map_pos = NULL;
        keys.pos = NULL;
        d = descendants[i]->new_entity(&keys);
        if (descendants[i] == pico8_nl_kind()) {
            assert(d != NULL &&
                   ((const entity_t *)d)->class == pico8_nl_struct_class());
            continue;
        }
        if (descendants[i] == pico8_os_kind()) {
            assert(d != NULL &&
                   ((const entity_t *)d)->class == pico8_os_struct_class());
            continue;
        }
        if (descendants[i] == pico8_hj_kind()) {
            assert(d != NULL &&
                   ((const entity_t *)d)->class == pico8_hj_struct_class());
            continue;
        }
        if (descendants[i] == pico8_pm_kind()) {
            assert(d != NULL &&
                   ((const entity_t *)d)->class == pico8_pm_struct_class());
            continue;
        }
        if (descendants[i] == pico8_mx_kind()) {
            assert(d != NULL &&
                   ((const entity_t *)d)->class == pico8_mx_struct_class());
            continue;
        }
        if (descendants[i] == pico8_nr_kind()) {
            assert(d != NULL &&
                   ((const entity_t *)d)->class == pico8_nr_struct_class());
            continue;
        }
        assert(d != NULL &&
               ((const entity_t *)d)->class == pico8_fr_struct_class());
    }
}

static unsigned lineage_of(const entity_t *e) {
    return e->class->lineage;
}

/* The eight kinds of qh's lineage are of jo's and qh's, nl alone of nl's
 * too; ga and jo of jo's alone, g_ and fv of g_'s alone, the other classes
 * of none. */
static void test_lineage_of_each_class(void) {
    const unsigned jo = LINEAGE(LINEAGE_PLANET);
    const unsigned qh = jo | LINEAGE(LINEAGE_STRUCTURE);
    const entity_kind_t *const lineage[] = {pico8_qh_kind(), pico8_nl_kind(), pico8_os_kind(), pico8_hj_kind(),
                                                  pico8_pm_kind(), pico8_mx_kind(), pico8_nr_kind(), pico8_fr_kind()};
    planet_keys_t keys = {"s99", NULL, NULL};
    const entity_t *e;
    size_t i;

    start_game();
    for (i = 0; i < sizeof lineage / sizeof lineage[0]; i++) {
        e = lineage[i]->new_entity(&keys);
        assert(lineage_of(e) == (lineage[i] == pico8_nl_kind() ? (qh | LINEAGE(LINEAGE_TRADING_HUB)) : qh));
    }
    assert(lineage_of(pico8_ga_kind()->new_entity(&keys)) == jo);
    assert(lineage_of(&pico8_jo_new(&keys)->header) == jo);
    assert(lineage_of(&pico8_g__new(NULL)->header) == LINEAGE(LINEAGE_PROBE));
    assert(lineage_of(&pico8_fv_new()->header) == LINEAGE(LINEAGE_PROBE));
    assert(lineage_of(&pico8_qb_new()->header) == 0 && lineage_of(&pico8_bg_new()->header) == 0);
    assert(lineage_of(&pico8_mz_new()->header) == 0 && lineage_of(&pico8_jc_new(NULL)->header) == 0);
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
    in_fresh_child(test_descendants);
    in_fresh_child(test_qh_is_a_struct);
    in_fresh_child(test_lineage_of_each_class);

    printf("test_pico8_qh: all tests passed\n");
    return 0;
}
