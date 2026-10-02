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
#include "game/scroll.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "game/trade.h"
#include "engine/game_zone.h"
#include "engine/vec.h"
#include "heap_allocator.h"

/* The cart keeps the box of `mp`, the port a copy of it (nil stays nil). */
static int ra_copies(const pico8_vec_t *ra, const pico8_vec_t *mp) {
    return mp == NULL ? ra == NULL : ra != NULL && ra != mp && ra->x == mp->x && ra->y == mp->y;
}


#define BUILT_HASH 0xBAF7572Fu
#define LIFE_HASH 0xC22C89F7u

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
static const char *const tags[] = {"slipgate", "node", "targeting"};

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
static pico8_jo_t *as_qh(void *e) {
    const entity_t *h = e;

    assert(h != NULL && (h->class->lineage & LINEAGE(LINEAGE_STRUCTURE)));
    return e;
}

static uint32_t fold_qh(uint32_t h, const pico8_jo_t *qh) {
    return qh->def->qh ? fold_fixed_member(h, p8_fixed_from_int(1)) : fold_bool_member(h, 0);
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
    return pico8_vec_v(p8_fixed_from_int(x), p8_fixed_from_int(y));
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

/* The research icon whose `ke` is `kind`. */
static void *icon_of(const entity_kind_t *kind) {
    const pico8_ents_t *hb = pico8_cv("refresh");
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
        hz = ((const pico8_jo_t *)e)->def->hz;
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
    const entity_class_t *qh_cls = pico8_qh_struct_class();
    const pico8_jo_t *nr;
    const pico8_jo_t *qh;
    const pico8_jo_t *jo;
    size_t i;

    assert(gate_kind(e) == NR && cls_of(e) == pico8_nr_struct_class());
    nr = e;
    qh = nr;
    jo = qh;
    h = fold_string_member(h, jo->id);
    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_string_member(h, nr->def->rr);
    h = fold_fixed_member(h, qh->def->hp);
    h = fold_fixed_member(h, qh->s);
    h = fold_fixed_member(h, qh->probe->od);
    h = fold_fixed_member(h, qh->probe->def->fp);
    h = fold_fixed_member(h, qh->probe->def->dw);
    h = fold_string_member(h, qh->probe->def->dq);
    h = fold_qh(h, qh);
    h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
    h = fold_hf(h, jo);
    h = fold_fixed_member(h, jo->def->hs);
    h = fold_fixed_member(h, jo->def->fa);
    h = fold_fixed_member(h, jo->ma);
    h = fold_fixed_member(h, jo->def->ca);
    h = fold_vec(h, &jo->def->gq);
    h = fnv(h, (uint32_t)nr->def->hz->count);
    /* The 1s of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`, `hs`, `fa`,
     * `ma` and `ca`: constant by the struct's shape, kept so that BUILT_HASH
     * still proves the typed reads. */
    h = fnv(h, (uint32_t)(nr->def->hz != NULL));
    h = fnv(h, (uint32_t)(nr->def->rr != NULL));
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
    assert(cls_of(e)->offer_trade == pico8_nr_struct_class()->offer_trade && cls_of(e)->offer_trade != qh_cls->offer_trade &&
           cls_of(e)->offer_trade != pico8_jo_struct_class()->offer_trade);
    assert(cls_of(e)->attach_slipway == qh_cls->attach_slipway && cls_of(e)->drag == qh_cls->drag && cls_of(e)->drop == qh_cls->drop &&
           cls_of(e)->spawn_particle == NULL && pico8_jo_is_family(e));
    assert_qh_shape(cls_of(e));
    assert(cls_of(e)->draw == qh_cls->draw && cls_of(e)->draw_overlay == qh_cls->draw_overlay && cls_of(e)->hit_test == qh_cls->hit_test &&
           cls_of(e)->new_year == qh_cls->new_year);
    assert(cls_of(e)->states[ENTITY_STATE_VANISHING] == qh_cls->states[ENTITY_STATE_VANISHING] &&
           cls_of(e)->states[ENTITY_STATE_SETTLED] == qh_cls->states[ENTITY_STATE_SETTLED]);
    assert(cls_of(e)->accept_drop != qh_cls->accept_drop && cls_of(e)->left_press != qh_cls->left_press && cls_of(e)->refresh != qh_cls->refresh);
    assert(cls_of(e)->accept_drop != pico8_jo_struct_class()->accept_drop && cls_of(e)->left_press != pico8_jo_struct_class()->left_press &&
           cls_of(e)->refresh != pico8_jo_struct_class()->refresh);
    assert(jo->def == &pico8_nr_def);
    assert(jo->header.class->tags == (TAG(TAG_SLIPGATE) | TAG(TAG_NODE) | TAG(TAG_TARGETING)) && nr->def->hz == &pico8_nr_hz_seq);
    assert(jo->header.class->tags != (TAG(TAG_STRUCTURE) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)));
    assert(jo->def->kq.xl != pico8_jo_def.kq.xl);
    assert(qh->def->px == &pico8_qh_px_row && qh->def->ba == &pico8_qh_ba_seq);
    assert(jo->def == &pico8_nr_def);
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
    assert(ents_listed(pico8_cg("slipgate"), e) && !ents_listed(pico8_cg("planet"), e) && !ents_listed(pico8_cg("structure"), e));
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
    assert(a != b && as_qh(a)->oe != as_qh(b)->oe && as_qh(a)->kw != as_qh(b)->kw);
    h = fold_slipgate(h, a);
    h = fold_slipgate(h, b);
    check("built", h, BUILT_HASH);
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = p8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const pico8_jo_t *qh) {
    const pico8_vec_t *ra = qh->ra;
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the slipgate. */
static uint32_t fold_dg(uint32_t h, void *e) {
    const pico8_jo_t *qh = as_qh(e);

    h = fold_ra_y(h, qh);
    pico8_dg();
    h = fold_fixed_member(h, qh->header.state_time);
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

/* What `bb`'s `dh`, `ib`, `gg` and `da` give on a slipgate. */
static uint32_t fold_economy(uint32_t h, void *e) {
    pico8_ents_t *ib = pico8_jo_ib(e);
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_ib(h, ib);
    h = fnv(fnv(h, 0xB0u), pico8_jo_gg(e) ? 1u : 0u);
    h = fnv(h, (uint32_t)pico8_da(e));
    return as_qh(e)->has_mi ? fold_fixed_member(h, as_qh(e)->mi) : fold_absent(h);
}

/* A list of `n` copies of `item`, for `oe`: `gv`, `ldown` and `hb` only
 * count it. */
static pico8_ents_t *list_of(int n, entity_t *item) {
    pico8_ents_t *l = malloc(sizeof *l);
    int i;

    pico8_ents_init(l, &zone_tas);
    for (i = 0; i < n; i++) {
        pico8_ents_add(l, item);
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
    pico8_vec_t *mp;
    down_t jc;
    pico8_ents_t *oe;
    const pico8_vec_t *mo_ra;
    int i;

    start_game();
    idle = build_slipgate();
    h = fold_string_member(h, as_qh(idle)->header.state);
    h = fold_dg(h, idle);

    placed = build_slipgate();
    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
    assert(mo_ra != NULL);
    mp = vec_ref(pico8_vec_add(*mo_ra, vec_xy(12, 108)));
    cls_of(placed)->drag(placed, mp, vec_ref(vec_xy(64, 64)));
    assert(ra_copies(as_qh(placed)->ra, mp));
    set_credits(200);
    assert(cls_of(placed)->drop == pico8_d__dispatch);
    h = pico8_d_(placed) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);
    h = fold_string_member(h, as_qh(placed)->header.state);
    assert(strcmp(as_qh(placed)->header.state, "settled") == 0);
    h = fold_economy(h, placed);
    for (i = 0; i < 3; i++) {
        h = fold_dg(h, placed);
    }
    for (i = 0; i < 2; i++) {
        pico8_dc("draw");
        h = fnv(h, screen_hash());
    }

    h = fold_bool(h, cls_of(placed)->accept_drop(placed, NULL).stop);
    h = fold_bool(h, cls_of(placed)->accept_drop(placed, idle).stop);
    jc = cls_of(placed)->left_press(placed, NULL, NULL);
    assert(jc.stop && jc.held != NULL);
    assert(cls_of(jc.held) == pico8_jc_struct_class() && ((const pico8_jc_t *)jc.held)->qu == placed);
    h = fnv(h, FROZEN_STRUCT_TAG);
    h = fold_bool(h, cls_of(placed)->accept_drop(placed, jc.held).stop);
    h = fold_bool(h, cls_of(idle)->accept_drop(idle, jc.held).stop);

    oe = as_qh(placed)->oe;
    for (i = 0; i <= 3; i++) {
        as_qh(placed)->oe = list_of(i, idle);
        cls_of(placed)->refresh(placed);
        h = fold_fixed_member(h, as_qh(placed)->s);
        h = fold_bool(h, cls_of(placed)->accept_drop(placed, NULL).stop);
    }
    assert(as_qh(placed)->s == p8_fixed_from_int(168));
    jc = cls_of(placed)->left_press(placed, NULL, NULL);
    assert(jc.held == NULL);
    h = fold_bool(h, jc.stop);
    as_qh(placed)->oe = oe;
    cls_of(placed)->refresh(placed);
    h = fold_fixed_member(h, as_qh(placed)->s);
    h = fold_economy(h, placed);

    fr = build(pico8_fr_kind());
    assert(gate_kind(fr) == FR);
    h = fold_fixed_member(h, as_qh(fr)->s);
    cls_of(fr)->refresh(fr);
    h = fold_fixed_member(h, as_qh(fr)->s);
    assert(as_qh(fr)->s == p8_fixed_from_int(78));

    bare = pico8_nr_new(NULL);
    assert(gate_kind(bare) == NR && as_qh(bare)->id == NULL && as_qh(bare)->oe != NULL &&
           as_qh(bare)->kw != NULL);
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("slipgate"), bare));
    {
        planet_keys_t keys;
        pico8_vec_t *p = vec_ref(vec_xy(3, 4));
        pico8_vec_t *ra = vec_ref(vec_xy(5, 6));
        void *with;
        keys.planet_id = NULL;
        keys.map_pos = p;
        keys.pos = ra;
        with = pico8_nr_new(&keys);
        assert(as_qh(with)->p == p && as_qh(with)->ra == ra && as_qh(with)->id == NULL);
        h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), with));
    }
    check("life", h, LIFE_HASH);
}

static int is_an_nr_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_nr_struct_class();
}

/* Built by `ie:ldown` or pico8_nr_new, a slipgate is a struct whose
 * descriptor holds qh's very entries but its own `gv`, `ldown`, `hb`, `cn`,
 * which no class of its chain installs any more. */
static void test_nr_is_a_struct(void) {
    const entity_class_t *cls = pico8_nr_struct_class();
    const entity_class_t *qh = pico8_qh_struct_class();
    void *a;
    void *fr;

    start_game();
    a = build_slipgate();
    assert(is_an_nr_struct(a) && cls != qh && ents_listed(pico8_cg("slipgate"), a));
    assert(cls->offer_trade != qh->offer_trade && cls->attach_slipway == qh->attach_slipway && cls->drag == qh->drag && cls->drop == qh->drop && cls->spawn_particle == qh->spawn_particle);
    assert_qh_shape(cls);
    assert(cls->draw == qh->draw && cls->draw_overlay == qh->draw_overlay && cls->hit_test == qh->hit_test && cls->new_year == qh->new_year);
    assert(cls->states[ENTITY_STATE_VANISHING] == qh->states[ENTITY_STATE_VANISHING] &&
           cls->states[ENTITY_STATE_SETTLED] == qh->states[ENTITY_STATE_SETTLED]);
    assert(cls->accept_drop != qh->accept_drop && cls->left_press != qh->left_press && cls->refresh != qh->refresh);
    assert(cls->accept_drop != pico8_jo_struct_class()->accept_drop && cls->left_press != pico8_jo_struct_class()->left_press &&
           cls->refresh != pico8_jo_struct_class()->refresh);
    assert(pico8_nr_ready() && pico8_fr_ready());
    {
        pico8_jo_t *bare = pico8_nr_new(NULL);
        assert(is_an_nr_struct(bare) && bare->id == NULL && bare->oe != NULL && bare->kw != NULL);
    }
    fr = build(pico8_fr_kind());
    assert(fr != NULL &&
           ((const entity_t *)fr)->class == pico8_fr_struct_class());
    cls_of(fr)->refresh(fr);
    assert(as_qh(fr)->s == p8_fixed_from_int(78));
}

/* Counts the offers `cn` makes, taking each as `jo:ib`'s closure does. */
static int g_offers = 0;

static void take_offer(pico8_jo_trade_proposal_t *self, pico8_trade_t *a) {
    pico8_ents_add(self->ib, &a->header);
    self->mu -= p8_fixed_from_int(1);
    g_offers++;
}

/* `nr:cn` forwards a trade over each of its slipways but one the trade's
 * `rl` already runs through: over `w` to a planet wanting the trade's
 * resource, a trade whose `rl` holds `w` is not offered, one whose `rl`
 * does not is, once. */
static void test_cn_skips_a_slipway_already_in_rl(void) {
    pico8_jo_t *self;
    pico8_jo_t *other;
    pico8_jc_keys_t jc_keys = {0};
    pico8_jc_t *w;
    pico8_ptrs_t *jy = malloc(sizeof *jy);
    pico8_ents_t *with_w = malloc(sizeof *with_w);
    pico8_ents_t *without_w = malloc(sizeof *without_w);
    pico8_ents_t ib;
    pico8_trade_keys_t keys = {0};
    pico8_jo_trade_proposal_t lw = {0};
    p8_fixed_t qe;
    int r_p;

    start_game();
    assert(pico8_r_lookup("p", &r_p));
    qe = p8_fixed_from_int(r_p);
    self = pico8_nr_new(NULL);
    other = pico8_qh_new(NULL);
    pico8_ptrs_init(jy, &zone_tas);
    pico8_ptrs_add(jy, pico8_jy_new(pico8_zone_partie(), &qe));
    other->jy = jy;
    jc_keys.qu = &self->header;
    w = pico8_jc_new(&jc_keys);
    w->re = &other->header;
    assert(self->oe != NULL && self->oe->count == 0);
    pico8_ents_add(self->oe, &w->header);

    pico8_ents_init(with_w, &zone_tas);
    pico8_ents_add(with_w, &w->header);
    pico8_ents_init(&ib, &zone_tas);
    keys.has_qe = 1;
    keys.qe = qe;
    keys.rl = with_w;
    lw.t = pico8_trade_new(pico8_zone_partie(), &keys);
    lw.mu = p8_fixed_from_int(2);
    lw.kf = take_offer;
    lw.ib = &ib;
    pico8_nr_cn(self, &lw);
    assert(g_offers == 0 && ib.count == 0 && lw.mu == p8_fixed_from_int(2));

    pico8_ents_init(without_w, &zone_tas);
    keys.rl = without_w;
    lw.t = pico8_trade_new(pico8_zone_partie(), &keys);
    pico8_nr_cn(self, &lw);
    assert(g_offers == 1 && ib.count == 1 && lw.mu == p8_fixed_from_int(1));
    pico8_ents_free(&ib);
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
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_nr_is_a_struct);
    in_fresh_child(test_cn_skips_a_slipway_already_in_rl);

    printf("test_pico8_nr: all tests passed\n");
    return 0;
}
