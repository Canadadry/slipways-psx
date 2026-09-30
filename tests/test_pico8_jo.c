/* The planet `jo`, read on its struct members and its generated defaults
 * (pico8_game_data.h), and driven through the real `bw`/`dg`/`dc`/`mv`. Its
 * daughters are read the same way. Games are built directly: no events
 * file. */

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
#include "../src/pico8_economy_formulas.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_vec.h"

#define GALAXY_PLANETS 90
#define GALAXY_HASH 0xE70E2EE3u
#define DAUGHTERS_HASH 0x668D1EBEu
#define NEW_PLANET_HASH 0x48C32418u
#define LL_DG_CALLS 62

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

static int rank_in(const pico8_ptrs_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int listed(const pico8_ptrs_t *list, const void *e) {
    return rank_in(list, e) >= 0;
}

static const pico8_jo_t *as_jo(const void *e) {
    assert(pico8_jo_is_family(e));
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
    return jo->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

/* 0 absent, 1 false, 2 true. */
static uint32_t p__state(const pico8_jo_t *jo) {
    return !jo->has_p_ ? 0u : jo->p_ ? 2u : 1u;
}

static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
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

static void check_int(const char *label, int got, int want) {
    printf("[%s] %d\n", label, got);
    fflush(stdout);
    assert(got == want);
}

static void start_title(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    start_title();
    pico8_eu(NULL);
}

static pico8_vec_t *vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

static const pico8_struct_class_t *cls_of(const void *e) {
    return ((const pico8_struct_header_t *)e)->cls;
}

/* jo's own members: `hd`, `kj`, `ldown`, `hb`, `gv`, `hk`, `kf`, `cn` and
 * the state `ll`; no other. */
static void assert_jo_members(const pico8_struct_class_t *c) {
    const pico8_struct_class_t *jo = pico8_jo_struct_class();

    assert(c->hd == jo->hd && c->kj == jo->kj && c->ldown == jo->ldown && c->hb == jo->hb && c->gv == jo->gv &&
           c->hk == jo->hk && c->states[PICO8_STATE_LL] == jo->states[PICO8_STATE_LL]);
    assert(c->hd != NULL && c->kj != NULL && c->ldown != NULL && c->hb != NULL && c->gv != NULL && c->hk != NULL &&
           c->states[PICO8_STATE_LL] != NULL);
    assert(c->kf == jo->kf && c->cn == jo->cn && c->kf != NULL && c->cn != NULL);
    assert(c->hq == NULL && c->ey == NULL && c->ol == NULL && c->mm == NULL);
    assert(c->ev == NULL && c->has_hb == NULL && c->rdown == NULL && c->states[PICO8_STATE_PH] == NULL &&
           c->states[PICO8_STATE_LY] == NULL && c->states[PICO8_STATE_KZ] == NULL &&
           c->states[PICO8_STATE_PR] == NULL);
}

/* A bare planet as `jo(...)` and `jo:pq` left it: the scalars, which of
 * its 31 members are set, its methods, which default tables it shares
 * with the class, and where `lt` registered it. */
static uint32_t fold_planet(uint32_t h, const void *e) {
    const pico8_jo_t *jo = as_jo(e);
    size_t i;

    h = fold_string_member(h, jo->ke.header.me);
    h = fold_fixed_member(h, jo->ke.header.t);
    h = fold_string_member(h, jo->id);
    h = fold_hf(h, jo);
    h = fold_fixed_member(h, jo->hs);
    h = fold_fixed_member(h, jo->fa);
    h = fold_fixed_member(h, jo->ma);
    h = fold_fixed_member(h, jo->ca);
    h = fold_vec(h, jo->p);
    h = fold_vec(h, jo->ra);
    h = fold_vec(h, jo->gq);
    /* The 1s of `hf`, `hs`, `fa`, `ma` and `ca`: constant by the struct's
     * shape, kept so that GALAXY_HASH and NEW_PLANET_HASH still prove the
     * typed reads. */
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
    assert_jo_members(cls_of(e));
    assert(jo->kq == &pico8_jo_start.kq);
    assert(jo->gq == &pico8_jo_start.gq);
    assert(jo->nu == &pico8_jo_nu_tags && jo->kw == pico8_jo_shared_kw() && jo->em == &pico8_jo_em_row);
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cv(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cg(tags[i]), e));
    }
    return h;
}

/* After `eu`: the 90 planets `bw` built. */
static void test_galaxy(void) {
    uint32_t h = 2166136261u;
    const pico8_ptrs_t *jo;
    int i;

    start_game();
    jo = pico8_cg("jo");
    for (i = 0; i < jo->count; i++) {
        h = fold_planet(h, jo->items[i]);
    }
    check_int("galaxy planets", jo->count, GALAXY_PLANETS);
    check("galaxy", h, GALAXY_HASH);
}

static uint32_t fold_live(uint32_t h, const void *e) {
    const pico8_jo_t *jo = as_jo(e);

    h = fold_string_member(h, jo->ke.header.me);
    h = fold_fixed_member(h, jo->ke.header.t);
    h = fold_fixed_member(h, jo->ma);
    h = jo->has_fx ? fold_bool_member(h, jo->fx) : fold_absent(h);
    h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
    h = jo->has_kr ? fold_fixed_member(h, jo->kr) : fold_absent(h);
    h = fold_string_member(h, jo->ep);
    h = fnv(h, p__state(jo));
    h = fold_kw(h, jo);
    h = fnv(h, (uint32_t)list_size(jo->oe));
    h = fnv(h, (uint32_t)list_size(jo->jy));
    h = fnv(h, (uint32_t)list_size(jo->by));
    h = fnv(h, (uint32_t)list_size(jo->cc));
    h = fnv(h, (uint32_t)(jo->fo != NULL));
    h = fnv(h, (uint32_t)(jo->nk != NULL));
    h = fnv(h, (uint32_t)(jo->lq != NULL));
    h = fnv(h, (uint32_t)jo->has_m_);
    return h;
}

enum { QH, NL, OS, HJ, PM, MX, NR, FR, GA, DAUGHTERS };

static int daughter_of(const void *e) {
    if (e != NULL &&
        ((const pico8_struct_header_t *)e)->cls == pico8_qh_struct_class()) {
        return QH;
    }
    if (e != NULL &&
        ((const pico8_struct_header_t *)e)->cls == pico8_nl_struct_class()) {
        return NL;
    }
    if (e != NULL &&
        ((const pico8_struct_header_t *)e)->cls == pico8_os_struct_class()) {
        return OS;
    }
    if (e != NULL &&
        ((const pico8_struct_header_t *)e)->cls == pico8_hj_struct_class()) {
        return HJ;
    }
    if (e != NULL &&
        ((const pico8_struct_header_t *)e)->cls == pico8_pm_struct_class()) {
        return PM;
    }
    if (e != NULL &&
        ((const pico8_struct_header_t *)e)->cls == pico8_mx_struct_class()) {
        return MX;
    }
    if (e != NULL &&
        ((const pico8_struct_header_t *)e)->cls == pico8_nr_struct_class()) {
        return NR;
    }
    if (e != NULL &&
        ((const pico8_struct_header_t *)e)->cls == pico8_fr_struct_class()) {
        return FR;
    }
    if (e != NULL &&
        ((const pico8_struct_header_t *)e)->cls == pico8_ga_struct_class()) {
        return GA;
    }
    return -1;
}

/* What a daughter inherits from `jo`: `ca`, `gq`, the `em` table itself,
 * and `jo`'s `gg` and `hd` called on it. */
static uint32_t fold_daughter(uint32_t h, void *d) {
    const pico8_jo_t *jo = as_jo(d);
    pico8_vec_t ra;

    h = fold_fixed_member(h, jo->ca);
    h = fold_vec(h, jo->gq);
    assert(jo->em == &pico8_jo_em_row);
    h = fnv(fnv(h, 0xB0u), pico8_jo_gg(jo) ? 1u : 0u);
    assert(jo->ra != NULL);
    ra = *jo->ra;
    h = fold_bool(h, cls_of(d)->hd(d, vec_ref(ra), vec_ref(ra)));
    ra = pico8_vec_add(ra, pico8_vec_v(pico8_fixed_from_int(8), pico8_fixed_from_int(8)));
    return fold_bool(h, cls_of(d)->hd(d, vec_ref(ra), vec_ref(ra)));
}

/* The shop's `ie:ldown` on the icon of `kind`, as the player buys it, then
 * dragged by `hq` to the middle of the screen. */
static void *bought(const pico8_entity_kind_t *kind) {
    const pico8_ptrs_t *hb = pico8_cv("hb");
    const pico8_qb_t *mo = pico8_mo_qb();
    const pico8_vec_t *mo_ra;
    int i;

    assert(mo != NULL && mo->ra != NULL);
    mo_ra = mo->ra;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *icon = hb->items[i];
        if (cls_of(icon) == pico8_ie_struct_class() && ((const pico8_ie_t *)icon)->ke == kind) {
            pico8_down_t made = cls_of(icon)->ldown(icon, NULL, NULL);
            pico8_vec_t middle = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
            assert(made.stop && made.held != NULL);
            cls_of(made.held)->hq(made.held, vec_ref(pico8_vec_add(*mo_ra, middle)), vec_ref(middle));
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

/* A generated planet turned by `jo:hu` into the daughter `cw` names: the
 * newest entity of `hv`. */
static void *turned(int planet, const char *cw) {
    void *p = pico8_cg("jo")->items[planet];

    pico8_jo_hh(p);
    pico8_jo_hu(p, cw_project(cw));
    return pico8_hv()->items[pico8_hv()->count - 1];
}

/* Each daughter by the road a game takes to it: the seven of the shop
 * bought from their icons, `pm` and `ga` turned from planets by the
 * collapse and harvest projects. */
static void test_daughters(void) {
    static const struct {
        int d;
        const pico8_entity_kind_t *(*kind)(void);
    } shop[] = {{QH, pico8_qh_kind}, {OS, pico8_os_kind}, {NR, pico8_nr_kind}, {FR, pico8_fr_kind},
                {NL, pico8_nl_kind}, {MX, pico8_mx_kind}, {HJ, pico8_hj_kind}};
    void *made[DAUGHTERS] = {NULL};
    uint32_t h = 2166136261u;
    size_t i;

    start_game();
    for (i = 0; i < sizeof shop / sizeof shop[0]; i++) {
        made[shop[i].d] = bought(shop[i].kind());
    }
    made[PM] = turned(0, "pm");
    made[GA] = turned(1, "ga");
    for (i = 0; i < DAUGHTERS; i++) {
        assert(made[i] != NULL && daughter_of(made[i]) == (int)i && listed(pico8_hv(), made[i]));
        h = fold_daughter(fnv(h, (uint32_t)i), made[i]);
    }
    check("daughters", h, DAUGHTERS_HASH);
}

static void *new_planet(const char *id, int x, int y) {
    pico8_jo_keys_t keys;

    keys.id = id;
    keys.p = vec_ref(pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y)));
    keys.ra = vec_ref(pico8_vec_v(pico8_fixed_from_int(x * 128), pico8_fixed_from_int(y * 128)));
    return pico8_jo_new(&keys);
}

static void test_new_planet(void) {
    uint32_t h = 2166136261u;
    void *a;

    start_title();
    a = new_planet("p900", 1, 2);
    assert(listed(pico8_hv(), a) && listed(pico8_cg("jo"), a));
    h = fold_planet(h, a);
    h = fold_live(h, a);
    check("new planet", h, NEW_PLANET_HASH);
}

/* `dg` calls `qc[qc.me]`: a planet whose `me` is "ll" runs `jo:ll` until
 * its `t` passes 60, then leaves `hv`. */
static void test_ll(void) {
    pico8_jo_t *a;
    int calls = 0;

    start_title();
    a = new_planet("p901", 3, 4);
    pico8_dg();
    assert(p__state(a) == 0);
    pico8_ke_ks(&a->ke.header, "ll");
    while (listed(pico8_hv(), a) && calls < 200) {
        pico8_dg();
        calls++;
    }
    assert(p__state(a) == 2);
    assert(!listed(pico8_hv(), a) && !listed(pico8_cg("jo"), a));
    check_int("ll dg calls", calls, LL_DG_CALLS);
}

/* `kw` of a planet that never ran `lz` is the class's shared one: an
 * in-place write to it is seen by every such planet and by the class, until
 * the next pico8_jo_init() puts it back to `{mu=0}`. */
static void test_kw_shared(void) {
    pico8_jo_t *a;
    pico8_jo_t *b;
    pico8_kw_t *kw;
    pico8_hp_t *hp;
    pico8_qe_map_t po_before;
    pico8_qe_map_t po_after;
    pico8_fixed_t before = pico8_fixed_from_int(0);
    pico8_fixed_t after = pico8_fixed_from_int(0);

    start_title();
    a = new_planet("p902", 5, 6);
    b = new_planet("p903", 7, 8);
    kw = a->kw;
    assert(kw == pico8_jo_shared_kw() && b->kw == kw);
    kw->mu = pico8_fixed_from_int(7);
    assert(b->kw->mu == pico8_fixed_from_int(7));
    assert(pico8_jo_shared_kw()->mu == pico8_fixed_from_int(7));

    assert(pico8_gb_count() > 0);
    hp = pico8_gb_find("");
    assert(hp != NULL);
    pico8_jo_lz(a, hp);
    assert(a->kw != NULL && a->kw != pico8_jo_shared_kw());
    assert(a->kw->mu == pico8_fixed_from_int(0));
    assert(b->kw == pico8_jo_shared_kw());
    assert(a->hn == hp && list_size(a->jy) >= 0);

    pico8_bx(&po_before);
    kw->has_qe = 1;
    kw->qe = pico8_fixed_from_int(28);
    pico8_bx(&po_after);
    pico8_qe_map_get(&po_before, kw->qe, &before);
    pico8_qe_map_get(&po_after, kw->qe, &after);
    assert(after == before);
    pico8_jo_init();
    assert(b->kw == pico8_jo_shared_kw());
    assert(!kw->has_qe && kw->mu == pico8_fixed_from_int(0));
}

/* `jo:hh` takes `df.ll` itself on a roll of `jo.ir()` under 0.45, and its
 * `me` for the planet's state; any other draw is a `jo.oc()` entry, and
 * `known`. */
static void test_hh_takes_ll(void) {
    char id[8];
    int took_ll = 0;
    int i;

    start_game();
    for (i = 0; i < 20 && !took_ll; i++) {
        pico8_jo_t *p;

        snprintf(id, sizeof id, "p%d", 950 + i);
        p = new_planet(id, i, i);
        pico8_jo_hh(p);
        assert(p->nk != NULL && p->ep == p->nk->d);
        took_ll = p->nk == pico8_df_lookup("ll");
        assert(strcmp(p->ke.header.me, took_ll ? "ll" : "known") == 0);
    }
    assert(took_ll);
}

static int is_a_jo_struct(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_jo_struct_class();
}

/* Built by `bw` or pico8_jo_new, a `jo` is a struct whose descriptor holds
 * jo's 11 dispatchers, and its class installs none of them, nor a `pq`. */
static void test_jo_is_a_struct(void) {
    const pico8_ptrs_t *jo;
    void *a;
    int j;

    start_game();
    jo = pico8_cg("jo");
    for (j = 0; j < jo->count; j++) {
        assert(is_a_jo_struct(jo->items[j]));
    }
    assert_jo_members(pico8_jo_struct_class());
    assert(pico8_jo_ready());

    assert(pico8_jo_dz() == pico8_jo_dz_rows && pico8_jo_cq() == &pico8_jo_cq_seq &&
           pico8_jo_ka() == &pico8_jo_ka_seq && pico8_jo_em() == &pico8_jo_em_row);

    a = new_planet("p904", 9, 10);
    assert(is_a_jo_struct(a));
    {
        pico8_jo_t *bare = pico8_jo_new(NULL);
        assert(is_a_jo_struct(bare) && bare->id == NULL && bare->ra == NULL && bare->oe != NULL);
    }

    (void)bought(pico8_qh_kind());
    jo = pico8_cg("qh");
    assert(jo != NULL && jo->count == 1);
    assert(jo->items[0] != NULL &&
           ((const pico8_struct_header_t *)jo->items[0])->cls == pico8_qh_struct_class());
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
    in_fresh_child(test_galaxy);
    in_fresh_child(test_daughters);
    in_fresh_child(test_new_planet);
    in_fresh_child(test_ll);
    in_fresh_child(test_kw_shared);
    in_fresh_child(test_hh_takes_ll);
    in_fresh_child(test_jo_is_a_struct);

    printf("test_pico8_jo: all tests passed\n");
    return 0;
}
