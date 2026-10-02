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

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "game/economy.h"
#include "game/economy_formulas.h"
#include "engine/entity_registry.h"
#include "game/help.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/scroll.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "game/ui_zone.h"

#define GALAXY_PLANETS 90
#define GALAXY_HASH 0xE70E2EE3u
#define DAUGHTERS_HASH 0x668D1EBEu
#define NEW_PLANET_HASH 0x48C32418u
#define LL_DG_CALLS 62

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

static const pico8_jo_t *as_jo(const void *e) {
    assert(pico8_jo_is_family(e));
    return e;
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

/* 0 absent, 1 false, 2 true. */
static uint32_t p__state(const pico8_jo_t *jo) {
    return jo->p_ ? 2u : 0u;
}

static uint32_t fold_vec(uint32_t h, const vec_t *p) {
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
static int list_size(const pointer_list_t *l) {
    return l == NULL ? -1 : l->count;
}

static int ents_list_size(const entity_list_t *l) {
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

/* Un _init sur l'ecran titre, sans frame : l'aide d'avant rend sa place
 * qy_fv, unique, avant que la nouvelle la prenne. */
static void sortir_l_aide(void) {
    const entity_list_t *hv = live_entities();
    for (int i = 0; i < hv->count; i++) {
        if (hv->items[i]->class == pico8_fv_struct_class()) {
            entity_unregister(hv->items[i]);
            pico8_cart_vider_file_sortie();
            return;
        }
    }
}

static void start_title(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    sortir_l_aide();
    pico8__init();
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    start_title();
    pico8_eu(NULL);
    pico8_cart_vider_file_sortie();
}

static vec_t *vec_ref(vec_t at) {
    vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

static const entity_class_t *cls_of(const void *e) {
    return ((const entity_t *)e)->class;
}

/* jo's own members: `hd`, `kj`, `ldown`, `hb`, `gv`, `hk`, `kf`, `cn` and
 * the state `ll`; no other. */
static void assert_jo_members(const entity_class_t *c) {
    const entity_class_t *jo = pico8_jo_struct_class();

    assert(c->hit_test == jo->hit_test && c->draw == jo->draw && c->left_press == jo->left_press && c->refresh == jo->refresh && c->accept_drop == jo->accept_drop &&
           c->new_year == jo->new_year && c->states[ENTITY_STATE_VANISHING] == jo->states[ENTITY_STATE_VANISHING]);
    assert(c->hit_test != NULL && c->draw != NULL && c->left_press != NULL && c->refresh != NULL && c->accept_drop != NULL && c->new_year != NULL &&
           c->states[ENTITY_STATE_VANISHING] != NULL);
    assert(c->attach_slipway == jo->attach_slipway && c->offer_trade == jo->offer_trade && c->attach_slipway != NULL && c->offer_trade != NULL);
    assert(c->drag == NULL && c->drop == NULL && c->draw_at == NULL && c->spawn_particle == NULL);
    assert(c->draw_overlay == NULL && c->has_refresh == NULL && c->right_press == NULL && c->states[ENTITY_STATE_NORMAL] == NULL &&
           c->states[ENTITY_STATE_SETTLED] == NULL && c->states[ENTITY_STATE_PLACING] == NULL &&
           c->states[ENTITY_STATE_UNFOLDING] == NULL);
}

/* A bare planet as `jo(...)` and `jo:pq` left it: the scalars, which of
 * its 31 members are set, its methods, which default tables it shares
 * with the class, and where `lt` registered it. */
static uint32_t fold_planet(uint32_t h, const void *e) {
    const pico8_jo_t *jo = as_jo(e);
    size_t i;

    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_string_member(h, jo->id);
    h = fold_hf(h, jo);
    h = fold_fixed_member(h, jo->def->hs);
    h = fold_fixed_member(h, jo->def->fa);
    h = fold_fixed_member(h, jo->ma);
    h = fold_fixed_member(h, jo->def->ca);
    h = fold_vec(h, jo->p);
    h = fold_vec(h, jo->ra);
    h = fold_vec(h, &jo->def->gq);
    /* The 1s of `hf`, `hs`, `fa`, `ma` and `ca`: constant by the struct's
     * shape, kept so that GALAXY_HASH and NEW_PLANET_HASH still prove the
     * typed reads. */
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
    assert_jo_members(cls_of(e));
    assert(jo->def == &pico8_jo_def);
    assert(jo->def == &pico8_jo_def);
    assert(jo->header.class->tags == (TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING)) && jo->kw == pico8_jo_shared_kw() && jo->def->em == &pico8_jo_em_row);
    h = fnv(h, (uint32_t)ents_rank_in(live_entities(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(entities_by_message(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(entities_by_tag(tags[i]), e));
    }
    return h;
}

/* After `eu`: the 90 planets `bw` built. */
static void test_galaxy(void) {
    uint32_t h = 2166136261u;
    const entity_list_t *jo;
    int i;

    start_game();
    jo = entities_by_tag("planet");
    for (i = 0; i < jo->count; i++) {
        h = fold_planet(h, jo->items[i]);
    }
    check_int("galaxy planets", jo->count, GALAXY_PLANETS);
    check("galaxy", h, GALAXY_HASH);
}

static uint32_t fold_live(uint32_t h, const void *e) {
    const pico8_jo_t *jo = as_jo(e);

    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_fixed_member(h, jo->ma);
    h = (jo->fx != 0) ? fold_bool_member(h, jo->fx) : fold_absent(h);
    h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
    h = jo->has_kr ? fold_fixed_member(h, jo->kr) : fold_absent(h);
    h = fold_string_member(h, jo->ep);
    h = fnv(h, p__state(jo));
    h = fold_kw(h, jo);
    h = fnv(h, (uint32_t)ents_list_size(jo->oe));
    h = fnv(h, (uint32_t)list_size(jo->jy));
    h = fnv(h, (uint32_t)ents_list_size(jo->by));
    h = fnv(h, (uint32_t)ents_list_size(jo->cc));
    h = fnv(h, (uint32_t)(jo->fo != NULL));
    h = fnv(h, (uint32_t)(jo->nk != NULL));
    h = fnv(h, (uint32_t)(jo->lq != NULL));
    h = fnv(h, (uint32_t)(jo->m_ != 0));
    return h;
}

enum { QH, NL, OS, HJ, PM, MX, NR, FR, GA, DAUGHTERS };

static int daughter_of(const void *e) {
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_qh_struct_class()) {
        return QH;
    }
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
    if (e != NULL &&
        ((const entity_t *)e)->class == pico8_ga_struct_class()) {
        return GA;
    }
    return -1;
}

/* What a daughter inherits from `jo`: `ca`, `gq`, the `em` table itself,
 * and `jo`'s `gg` and `hd` called on it. */
static uint32_t fold_daughter(uint32_t h, void *d) {
    const pico8_jo_t *jo = as_jo(d);
    vec_t ra;

    h = fold_fixed_member(h, jo->def->ca);
    h = fold_vec(h, &jo->def->gq);
    assert(jo->def->em == &pico8_jo_em_row);
    h = fnv(fnv(h, 0xB0u), pico8_jo_gg(jo) ? 1u : 0u);
    assert(jo->ra != NULL);
    ra = *jo->ra;
    h = fold_bool(h, cls_of(d)->hit_test(d, vec_ref(ra), vec_ref(ra)));
    ra = vec_add(ra, vec_make(p8_fixed_from_int(8), p8_fixed_from_int(8)));
    return fold_bool(h, cls_of(d)->hit_test(d, vec_ref(ra), vec_ref(ra)));
}

/* The shop's `ie:ldown` on the icon of `kind`, as the player buys it, then
 * dragged by `hq` to the middle of the screen. */
static void *bought(const entity_kind_t *kind) {
    const entity_list_t *hb = entities_by_message("refresh");
    const pico8_qb_t *mo = pico8_mo_qb();
    const vec_t *mo_ra;
    int i;

    assert(mo != NULL && mo->ra != NULL);
    mo_ra = mo->ra;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *icon = hb->items[i];
        if (cls_of(icon) == pico8_ie_struct_class() && ((const pico8_ie_t *)icon)->ke == kind) {
            down_t made = cls_of(icon)->left_press(icon, NULL, NULL);
            vec_t middle = vec_make(p8_fixed_from_int(64), p8_fixed_from_int(64));
            assert(made.stop && made.held != NULL);
            cls_of(made.held)->drag(made.held, vec_ref(vec_add(*mo_ra, middle)), vec_ref(middle));
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
    void *p = entities_by_tag("planet")->items[planet];

    pico8_jo_hh(p);
    pico8_jo_hu(p, cw_project(cw));
    return live_entities()->items[live_entities()->count - 1];
}

/* Each daughter by the road a game takes to it: the seven of the shop
 * bought from their icons, `pm` and `ga` turned from planets by the
 * collapse and harvest projects. */
static void test_daughters(void) {
    static const struct {
        int d;
        const entity_kind_t *(*kind)(void);
    } shop[] = {{QH, pico8_qh_kind}, {OS, pico8_os_kind}, {NR, pico8_nr_kind}, {FR, pico8_fr_kind},
                {NL, pico8_nl_kind}, {MX, pico8_mx_kind}, {HJ, pico8_hj_kind}};
    void *made[DAUGHTERS] = {NULL};
    uint32_t h = 2166136261u;
    size_t i;

    start_game();
    for (i = 0; i < sizeof shop / sizeof shop[0]; i++) {
        made[shop[i].d] = bought(shop[i].kind());
    }
    made[PM] = turned(0, "protostar");
    made[GA] = turned(1, "burst");
    for (i = 0; i < DAUGHTERS; i++) {
        assert(made[i] != NULL && daughter_of(made[i]) == (int)i && ents_listed(live_entities(), made[i]));
        h = fold_daughter(fnv(h, (uint32_t)i), made[i]);
    }
    check("daughters", h, DAUGHTERS_HASH);
}

static void *new_planet(const char *id, int x, int y) {
    planet_keys_t keys;

    keys.planet_id = id;
    keys.map_pos = vec_ref(vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y)));
    keys.pos = vec_ref(vec_make(p8_fixed_from_int(x * 128), p8_fixed_from_int(y * 128)));
    return pico8_jo_new(&keys);
}

static void test_new_planet(void) {
    uint32_t h = 2166136261u;
    void *a;

    start_title();
    a = new_planet("p900", 1, 2);
    assert(ents_listed(live_entities(), a) && ents_listed(entities_by_tag("planet"), a));
    h = fold_planet(h, a);
    h = fold_live(h, a);
    check("new planet", h, NEW_PLANET_HASH);
}

/* cg answers NULL for a tag that no entity has carried since
 * pico8_entity_init(), known or not, and keeps a list once emptied. */
static void test_cg_null_until_seen(void) {
    void *a;

    start_title();
    entity_registry_init();
    assert(entities_by_tag("planet") == NULL && entities_by_tag("node") == NULL && entities_by_tag("background") == NULL);
    a = new_planet("p902", 5, 6);
    assert(entities_by_tag("planet") != NULL && entities_by_tag("node") != NULL && entities_by_tag("targeting") != NULL);
    assert(entities_by_tag("structure") == NULL && entities_by_tag("background") == NULL);
    entity_unregister(a);
    assert(entities_by_tag("planet") != NULL && entities_by_tag("planet")->count == 0);
    entity_registry_init();
    assert(entities_by_tag("planet") == NULL);
}

/* `dg` calls `qc[qc.me]`: a planet whose `me` is "ll" runs `jo:ll` until
 * its `t` passes 60, then leaves `hv`. */
static void test_ll(void) {
    pico8_jo_t *a;
    int calls = 0;

    start_title();
    a = new_planet("p901", 3, 4);
    update_entities();
    assert(p__state(a) == 0);
    entity_set_state(&a->header, "vanishing");
    while (ents_listed(live_entities(), a) && calls < 200) {
        update_entities();
        calls++;
    }
    assert(p__state(a) == 2);
    assert(!ents_listed(live_entities(), a) && !ents_listed(entities_by_tag("planet"), a));
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
    p8_fixed_t before = p8_fixed_from_int(0);
    p8_fixed_t after = p8_fixed_from_int(0);

    start_title();
    a = new_planet("p902", 5, 6);
    b = new_planet("p903", 7, 8);
    kw = a->kw;
    assert(kw == pico8_jo_shared_kw() && b->kw == kw);
    kw->mu = p8_fixed_from_int(7);
    assert(b->kw->mu == p8_fixed_from_int(7));
    assert(pico8_jo_shared_kw()->mu == p8_fixed_from_int(7));

    assert(pico8_gb_count() > 0);
    hp = pico8_gb_find("");
    assert(hp != NULL);
    pico8_jo_lz(a, hp);
    assert(a->kw != NULL && a->kw != pico8_jo_shared_kw());
    assert(a->kw->mu == p8_fixed_from_int(0));
    assert(b->kw == pico8_jo_shared_kw());
    assert(a->hn == hp && list_size(a->jy) >= 0);

    pico8_bx(&po_before);
    kw->has_qe = 1;
    kw->qe = p8_fixed_from_int(28);
    pico8_bx(&po_after);
    pico8_qe_map_get(&po_before, kw->qe, &before);
    pico8_qe_map_get(&po_after, kw->qe, &after);
    assert(after == before);
    pico8_jo_init();
    assert(b->kw == pico8_jo_shared_kw());
    assert(!kw->has_qe && kw->mu == p8_fixed_from_int(0));
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
        took_ll = p->nk == pico8_df_lookup("vanishing");
        assert(strcmp(p->header.state, took_ll ? "vanishing" : "known") == 0);
    }
    assert(took_ll);
}

static int is_a_jo_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_jo_struct_class();
}

/* Built by `bw` or pico8_jo_new, a `jo` is a struct whose descriptor holds
 * jo's 11 dispatchers, and its class installs none of them, nor a `pq`. */
static void test_jo_is_a_struct(void) {
    const entity_list_t *jo;
    void *a;
    int j;

    start_game();
    jo = entities_by_tag("planet");
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
    jo = entities_by_tag("structure");
    assert(jo != NULL && jo->count == 1);
    assert(jo->items[0] != NULL &&
           ((const entity_t *)jo->items[0])->class == pico8_qh_struct_class());
}

static int rangee_places(void) {
    return pico8_zone_interface_vivantes(PICO8_SORTE_RANGEE);
}

/* A galaxy planet given a `kw` of its own with a `qe`, as `lz` and `bb`
 * would: `jo:hb` then gives it a row. */
static pico8_jo_t *planet_with_a_row(void) {
    pico8_jo_t *a = (pico8_jo_t *)entities_by_tag("planet")->items[0];

    a->kw = &a->kw_own;
    a->kw_own.has_qe = 1;
    a->kw_own.qe = p8_fixed_from_int(28);
    a->kw_own.mu = p8_fixed_from_int(2);
    return a;
}

/* `jo:hb` rebuilds the planet's row in a place of its own and gives the
 * replaced one back on the spot; once the planet leaves `hv`, emptying the
 * exit queue gives its row's place back. */
static void test_hb_row_place(void) {
    pico8_jo_t *a;
    int once;
    int i;

    start_game();
    pico8_cart_vider_file_sortie();
    a = planet_with_a_row();
    cls_of(a)->refresh(&a->header);
    once = rangee_places();
    assert(a->lq != NULL && once > 0);
    for (i = 0; i < 5; i++) {
        cls_of(a)->refresh(&a->header);
        assert(rangee_places() == once && "the replaced row gives its place back on the spot");
    }
    entity_unregister(&a->header);
    assert(rangee_places() == once && "out of hv, the planet is still read this frame");
    pico8_cart_vider_file_sortie();
    assert(rangee_places() == once - 1 && a->lq == NULL);
}

/* A planet whose `kw` has no `qe` gets no row from `jo.ex`, and no place. */
static void test_hb_without_qe_takes_no_place(void) {
    pico8_jo_t *a;
    int before;

    start_title();
    a = new_planet("p903", 7, 8);
    assert(a->kw != NULL && !a->kw->has_qe);
    before = rangee_places();
    cls_of(a)->refresh(&a->header);
    assert(a->lq == NULL && rangee_places() == before);
}

/* `jl()` hands every planet to the exit queue: a title after a game whose
 * planets all built a row holds as many row places as before. */
static void test_rows_given_back_after_restart(void) {
    const entity_list_t *jo;
    int title;
    int i;

    start_title();
    pico8_cart_vider_file_sortie();
    title = rangee_places();
    start_game();
    jo = entities_by_tag("planet");
    (void)planet_with_a_row();
    for (i = 0; i < jo->count; i++) {
        cls_of(jo->items[i])->refresh(jo->items[i]);
    }
    assert(rangee_places() > title);
    start_title();
    pico8_cart_vider_file_sortie();
    assert(rangee_places() == title);
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
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_galaxy);
    in_fresh_child(test_daughters);
    in_fresh_child(test_new_planet);
    in_fresh_child(test_cg_null_until_seen);
    in_fresh_child(test_ll);
    in_fresh_child(test_kw_shared);
    in_fresh_child(test_hh_takes_ll);
    in_fresh_child(test_jo_is_a_struct);
    in_fresh_child(test_hb_row_place);
    in_fresh_child(test_hb_without_qe_takes_no_place);
    in_fresh_child(test_rows_given_back_after_restart);

    printf("test_pico8_jo: all tests passed\n");
    return 0;
}
