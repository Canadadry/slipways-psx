/* The particle burst `ga`, read and written on its struct members and the
 * generated defaults (pico8_game_data.h), built by its three roads (`jo:hu`
 * on the harvest recipe, `pm:pq`, `pico8_ga_new(NULL)`) and driven through
 * the real `mv`/`dc`. jo's methods are named by `pico8_jo_struct_class()`.
 * No events file. */

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
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/layout.h"
#include "p8/mem.h"
#include "game/particle.h"
#include "game/ui_zone.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "engine/vec.h"
#include "engine/game_zone.h"


#define BUILT_HASH 0xA539DD6Du
#define LIFE_HASH 0x196B693Au
#define BURST_FRAMES 86

/* The tag `lq.fs` carried as a tagged list, which the frozen hashes fold:
 * any other value moves every *_HASH. */
#define FROZEN_LIST_TAG 0x9E5C1157u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static const char *const tags[] = {"planet", "node", "targeting", "ascension", "structure"};
static const char *const bm[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};

static int tag_count(unsigned tags) {
    int t, n = 0;

    for (t = 0; t < TAG_COUNT; t++) {
        n += (tags & TAG(t)) != 0;
    }
    return n;
}

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

static uint32_t fold_p_(uint32_t h, const pico8_jo_t *jo) {
    return (jo->p_ != 0) ? fold_bool_member(h, jo->p_) : fold_absent(h);
}

static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

static uint32_t fold_ref_member(uint32_t h, const void *ref) {
    assert(ref == NULL && "no test folds a set ref: to fold one, fold 1 then its rank in hv (0x4E, rank), or outside hv 0xEEEE then the frozen struct tag 0x9E5C57C7, as the frozen hashes did");
    return fold_absent(h);
}

/* The item count of a list member, -1 when NULL. */
static int ptrs_size(const pico8_ptrs_t *l) {
    return l == NULL ? -1 : l->count;
}

static int ents_ptrs_size(const pico8_ents_t *l) {
    return l == NULL ? -1 : l->count;
}

/* The words the frozen hashes fold before a value: 0xF1 a number, 0xB0 a
 * boolean (then 0 or 1), 0x57 a string; any other moves every *_HASH. */
static uint32_t fold_fixed(uint32_t h, pico8_fixed_t x) {
    return fnv(fnv(h, 0xF1u), (uint32_t)x);
}

static uint32_t fold_bool(uint32_t h, int b) {
    return fnv(fnv(h, 0xB0u), b ? 1u : 0u);
}

static uint32_t fold_string(uint32_t h, const char *s) {
    return fnv_str(fnv(h, 0x57u), s);
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

static pico8_vec_t near_mo(int x, int y) {
    const pico8_vec_t *mo_ra;

    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
    assert(mo_ra != NULL);
    return pico8_vec_add(*mo_ra, vec_xy(x, y));
}

static const entity_class_t *cls_of(const void *e) {
    return ((const entity_t *)e)->class;
}

/* `fz` is "ga" on a burst and on nothing else. */
static int is_ga(const void *e) {
    return e != NULL && cls_of(e) == pico8_ga_struct_class() && ((const pico8_jo_t *)e)->def->fz != NULL &&
           strcmp(((const pico8_jo_t *)e)->def->fz, "burst") == 0;
}

static pico8_jo_t *as_ga(void *e) {
    assert(e != NULL && cls_of(e) == pico8_ga_struct_class());
    return e;
}

/* The first burst of `hv` at rank `from` or later, NULL when none. */
static void *ga_from(int from) {
    const pico8_ents_t *hv = pico8_hv();
    int i;
    for (i = from; i < hv->count; i++) {
        if (is_ga(hv->items[i])) {
            return hv->items[i];
        }
    }
    return NULL;
}

/* The recipe of `gb` whose `cw` names "ga": the harvest. */
static void *harvest_recipe(void) {
    void *found = NULL;
    int i;

    for (i = 1; i <= pico8_gb_count(); i++) {
        pico8_hp_t *hp = pico8_gb_at(i);
        if (hp->cw != NULL && strcmp(hp->cw, "burst") == 0) {
            assert(found == NULL || found == hp);
            found = hp;
        }
    }
    assert(found != NULL);
    return found;
}

/* Road (a): a bare planet on `id`/`p`/`ra`, harvested by `jo:hu`; the burst
 * is the one `hv` gained. */
static void *harvest(int x, int y, const char *id) {
    planet_keys_t keys;
    pico8_jo_t *planet;
    void *made;
    int before = pico8_hv()->count;

    keys.planet_id = id;
    keys.map_pos = vec_ref(vec_xy(x / 2, y / 2));
    keys.pos = vec_ref(near_mo(x, y));
    planet = pico8_jo_new(&keys);
    assert(!is_ga(planet) && pico8_hv()->count == before + 1);
    pico8_jo_hu(planet, harvest_recipe());
    made = ga_from(before + 1);
    assert(made != NULL && pico8_hv()->count == before + 2 && ga_from(0) == made);
    assert(as_ga(made)->ra == planet->ra && as_ga(made)->p == planet->p);
    return made;
}

/* Road (b): `kv["pm"]` on `p`/`ra`/`id`; its burst is registered just
 * before the protostar. */
static void *spawn(int x, int y, const char *id, void **protostar) {
    planet_keys_t keys;
    void *pm;
    int at;

    keys.planet_id = id;
    keys.map_pos = vec_ref(vec_xy(x / 2, y / 2));
    keys.pos = vec_ref(near_mo(x, y));
    pm = pico8_pm_new(&keys);
    at = ents_rank_in(pico8_hv(), pm);
    assert(at >= 1 && is_ga(pico8_hv()->items[at - 1]));
    {
        const pico8_vec_t *ra = as_ga(pico8_hv()->items[at - 1])->ra;
        pico8_vec_t want = pico8_vec_add(*keys.pos, ((pico8_jo_t *)pm)->def->ng);
        assert(ra != keys.pos && ra->x == want.x && ra->y == want.y);
    }
    if (protostar != NULL) {
        *protostar = pm;
    }
    return pico8_hv()->items[at - 1];
}

/* A burst as its road and `ga:pq`, `jo:pq`, `ke:pq` left it: its scalars,
 * which of its own and jo's names it has, its 12 methods (jo's very
 * functions but `kj` and `mm`), which default tables it shares with `ga`
 * and `jo`, where `lt` registered it -- in `hv` alone -- and `mo.ma`. */
static uint32_t fold_burst(uint32_t h, void *e, int from_pm) {
    const pico8_jo_t *g = as_ga(e);
    const pico8_emitter_t *em = g->emitter;
    const pico8_jo_t *jo = g;
    int j;

    assert(is_ga(e));
    h = fold_string_member(h, g->def->fz);
    h = fold_fixed_member(h, jo->def->fa);
    h = fold_fixed_member(h, em->def->fc);
    h = fold_fixed_member(h, em->qo);
    h = fold_fixed_member(h, em->ro);
    h = fold_fixed_member(h, em->qg);
    h = fold_fixed_member(h, em->q_);
    h = fold_fixed_member(h, g->iu);
    h = fold_fixed_member(h, em->py);
    h = fold_fixed_member(h, g->ms);
    h = fold_hf(h, jo);
    h = fold_string_member(h, jo->header.state);
    h = fold_fixed_member(h, jo->header.state_time);
    h = fold_string_member(h, jo->id);
    h = fold_fixed_member(h, jo->ma);
    h = fold_p_(h, jo);
    h = fold_vec(h, jo->ra);
    h = fold_vec(h, jo->p);
    h = fnv(h, (uint32_t)(em->k_ == NULL ? -1 : em->k_->count));
    h = fnv(h, (uint32_t)tag_count(jo->header.class->tags));
    h = fnv(h, (uint32_t)ents_ptrs_size(jo->by));
    h = fnv(h, (uint32_t)ptrs_size(em->qy));
    h = fnv(h, (uint32_t)ents_ptrs_size(jo->oe));
    assert(pico8_mo_qb() != NULL);
    h = fold_fixed_member(h, pico8_mo_qb()->ma);
    /* The 1s of `fc`, `qo`, `ro`, `qg`, `q_`, `iu`, `py`, `ms` (ga's own)
     * and of `hf`, `hs`, `fa`, `ma`, `ca` (jo's) are constant by the
     * struct's shape, kept so that BUILT_HASH still proves the typed reads. */
    h = fnv(h, (uint32_t)(g->def->fz != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(em->k_ != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(em->qy != NULL));
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
    h = fnv(h, (uint32_t)(jo->has_jh != 0));
    h = fnv(h, (uint32_t)(jo->p_ != 0));
    h = fnv(h, (uint32_t)(jo->has_kr != 0));
    h = fnv(h, (uint32_t)(jo->cc != NULL));
    h = fnv(h, (uint32_t)(jo->by != NULL));
    h = fnv(h, (uint32_t)(jo->has_mi != 0));
    h = fnv(h, (uint32_t)(jo->m_ != 0));
    assert(jo->jy == NULL && !jo->has_kr);
    assert(cls_of(e)->attach_slipway == pico8_jo_struct_class()->attach_slipway && cls_of(e)->offer_trade == pico8_jo_struct_class()->offer_trade &&
           cls_of(e)->attach_slipway != NULL && cls_of(e)->offer_trade != NULL && pico8_jo_is_family(e));
    assert(cls_of(e)->spawn_particle != pico8_jo_struct_class()->spawn_particle && cls_of(e)->spawn_particle != NULL);
    assert(cls_of(e)->draw != NULL && cls_of(e)->draw != pico8_jo_struct_class()->draw);
    assert(cls_of(e)->hit_test == pico8_jo_struct_class()->hit_test && cls_of(e)->left_press == pico8_jo_struct_class()->left_press &&
           cls_of(e)->refresh == pico8_jo_struct_class()->refresh && cls_of(e)->accept_drop == pico8_jo_struct_class()->accept_drop &&
           cls_of(e)->new_year == pico8_jo_struct_class()->new_year &&
           cls_of(e)->states[ENTITY_STATE_VANISHING] == pico8_jo_struct_class()->states[ENTITY_STATE_VANISHING]);
    assert(cls_of(e)->hit_test != NULL && cls_of(e)->left_press != NULL && cls_of(e)->refresh != NULL && cls_of(e)->accept_drop != NULL &&
           cls_of(e)->new_year != NULL && cls_of(e)->states[ENTITY_STATE_VANISHING] != NULL);
    assert(cls_of(e)->states[ENTITY_STATE_NORMAL] == NULL && cls_of(e)->draw_overlay == NULL &&
           cls_of(e)->states[ENTITY_STATE_PLACING] == NULL && cls_of(e)->states[ENTITY_STATE_UNFOLDING] == NULL &&
           cls_of(e)->right_press == NULL && cls_of(e)->states[ENTITY_STATE_SETTLED] == NULL);
    assert(cls_of(e)->draw_at == NULL && cls_of(e)->drag == NULL && cls_of(e)->drop == NULL);
    assert(jo->header.class->tags == 0);
    assert(jo->def == &pico8_ga_def);
    assert(jo->by != NULL);
    if (from_pm) {
        assert(em->k_ != &pico8_ga_k__seq);
        assert(em->k_ != &pico8_pm_k__seq);
    } else {
        assert(em->k_ == &pico8_ga_k__seq);
    }
    assert(jo->def == &pico8_ga_def);
    assert(jo->kw == pico8_jo_shared_kw());
    assert(jo->def->em == &pico8_jo_em_row);
    assert(jo->def->ka == &pico8_jo_ka_seq);
    assert(jo->def->dz == pico8_jo_dz_rows);
    assert(jo->def->cq == &pico8_jo_cq_seq);
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), e));
    for (j = 0; j < (int)(sizeof bm / sizeof bm[0]); j++) {
        h = fnv(h, (uint32_t)ents_rank_in(pico8_cv(bm[j]), e));
    }
    for (j = 0; j < (int)(sizeof tags / sizeof tags[0]); j++) {
        assert(!ents_listed(pico8_cg(tags[j]), e));
    }
    return h;
}

/* One burst by each road, the first two on screen. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;
    void *c;

    start_game();
    assert(ga_from(0) == NULL);
    a = harvest(40, 40, "s61");
    h = fold_burst(h, a, 0);
    b = spawn(80, 70, "s62", NULL);
    h = fold_burst(h, b, 1);
    c = pico8_ga_new(NULL);
    h = fold_burst(h, c, 0);
    assert(a != b && b != c && a != c);
    assert(as_ga(a)->emitter->qy != as_ga(b)->emitter->qy &&
           as_ga(b)->emitter->qy != as_ga(c)->emitter->qy);
    assert(as_ga(a)->oe != as_ga(b)->oe);
    assert(as_ga(a)->by == as_ga(b)->by && as_ga(b)->by == as_ga(c)->by);
    assert(as_ga(c)->ra == NULL && as_ga(c)->p == NULL && as_ga(c)->id == NULL);
    assert(as_ga(a)->def->fa == pico8_fixed_from_int(3) && as_ga(a)->ms == pico8_fixed_from_int(3));
    assert(as_ga(a)->def->hf == 0);
    assert(as_ga(b)->ms == PICO8_FIXED_LIT(0x00018000, 1.5));
    check("built", h, BUILT_HASH);
}

static uint32_t fold_particle(uint32_t h, const pico8_qy_t *q) {
    assert(q != NULL);
    assert(q->has_a);
    h = fnv(fnv(h, (uint32_t)q->p.x), (uint32_t)q->p.y);
    h = fnv(fnv(h, (uint32_t)q->v.x), (uint32_t)q->v.y);
    h = fnv(fnv(h, (uint32_t)q->a.x), (uint32_t)q->a.y);
    return fnv(h, (uint32_t)q->l);
}

/* A tuple of `lq:mn`: items, `qe`, `c`, then the entry count. */
static uint32_t fold_tuple(uint32_t h, const pico8_jb_row_t *t) {
    int i;

    for (i = 0; i < t->count; i++) {
        const pico8_jb_item_t *item = &t->items[i];

        h = fnv(h, (uint32_t)(i + 1));
        if (item->s != NULL) {
            h = fold_string(h, item->s);
        } else if (item->is_false) {
            h = fold_bool(h, 0);
        } else {
            assert(!item->is_ref && "no test folds a tuple item that points: to fold one, fold 0xDEAD if NULL, else 0xEEEE then 0, as the frozen hashes did");
            h = fold_fixed(h, item->is_fixed ? item->fixed : pico8_fixed_from_int(item->n));
        }
    }
    if (t->has_qe) {
        h = fold_fixed(fnv_str(h, "qe"), t->qe);
    }
    if (t->has_c) {
        h = fold_fixed(fnv_str(h, "c"), pico8_fixed_from_int(t->c));
    }
    return fnv(h, (uint32_t)(t->count + (t->has_qe != 0) + (t->has_c != 0)));
}

/* A positional as the value it was sorted from folded; NOTHING as false
 * (no row folded here holds a nil at the top level). */
static uint32_t fold_lq_positional(uint32_t h, const pico8_lq_t *lq, int i) {
    pico8_lq_cell_t c = lq->items[i];

    h = fnv(h, (uint32_t)(i + 1));
    switch (c.kind) {
    case PICO8_LQ_CELL_ROW:
        return fold_tuple(h, c.as.row);
    case PICO8_LQ_CELL_FIXED:
        return fold_fixed(h, c.as.fixed);
    case PICO8_LQ_CELL_STRING:
        return fold_string(h, c.as.s);
    default:
        return fold_bool(h, c.kind == PICO8_LQ_CELL_TRUE);
    }
}

/* The row `jo:ex` built into `lq`: its first `#fs` positionals, `c`, `bg`,
 * `score`, `ra`, `fs`, `w`, the rest. */
static uint32_t fold_lq(uint32_t h, void *e) {
    const pico8_lq_t *lq = as_ga(e)->lq;
    int n = 0;
    int k;
    int i;

    assert(lq != NULL &&
           lq->header.class == pico8_lq_struct_class());
    assert(!lq->has_bg);
    assert(lq->fs != NULL);
    k = lq->fs_count;
    for (i = 0; i < k; i++) {
        h = fold_lq_positional(h, lq, i);
    }
    if (lq->has_c) {
        h = fold_fixed(fnv_str(h, "c"), lq->c);
        n++;
    }
    if (lq->has_score) {
        h = fold_fixed(fnv_str(h, "score"), lq->score);
        n++;
    }
    assert(lq->ra == NULL && "no test folds an lq whose ra is set: to fold one, fold the key \"ra\", then 0xEEEE then 0, count it in n, as the frozen hashes did");
    /* As fold_ref folded the tagged list `fs` was: never in `hv`, so the tag branch. */
    h = fnv(fnv(fnv_str(h, "fs"), 0xEEEEu), FROZEN_LIST_TAG);
    h = fold_fixed(fnv_str(h, "w"), lq->w);
    for (i = k; i < lq->items_count; i++) {
        h = fold_lq_positional(h, lq, i);
    }
    return fnv(h, (uint32_t)(lq->items_count + n + 2));
}

/* Cuts `qy` to its first `n` particles, giving back the others. */
static void cut_qy(pico8_ptrs_t *qy, int n) {
    while (qy->count > n) {
        pico8_zone_release(pico8_zone_particules(PICO8_PARTICULES_PARTICULE), qy->items[qy->count - 1]);
        qy->count--;
    }
}

/* `kj` on `e` with its `qy` cut in place to `n` particles: `p_` after. */
static uint32_t fold_cut(uint32_t h, void *e, int n) {
    pico8_jo_t *g = as_ga(e);
    pico8_ptrs_t *qy = g->emitter->qy;

    assert(qy != NULL && qy->count >= n);
    cut_qy(qy, n);
    cls_of(e)->draw(e, g->ra);
    h = fold_p_(h, g);
    return fnv(h, (uint32_t)qy->count);
}

/* `mm` on each burst; `dc("kj")` three times, then three more with the
 * protostar burst cut to 3 particles, so that its circles, drawn in the
 * class `k_` (TROU-21), are not all covered by `ei`'s dots in its own; `p_` once `qy` holds 3, 2, 0 particles; `hd` on and off the burst,
 * `gg`; `hb` on jo's `kw` (no row), then on one with a `qe`, before and
 * after two routes land in the shared `by`; `hk` with no `kr`. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;
    void *c;
    void *pm;
    pico8_ents_t *by;
    static pico8_kw_t kw;
    uint32_t row;
    int hv_count;
    int i;

    start_game();
    a = harvest(40, 40, "s71");
    b = spawn(84, 60, "s72", &pm);
    c = pico8_ga_new(NULL);
    h = fold_particle(h, cls_of(a)->spawn_particle(a));
    h = fold_particle(h, cls_of(b)->spawn_particle(b));
    h = fold_particle(h, cls_of(c)->spawn_particle(c));
    for (i = 0; i < 3; i++) {
        pico8_dc("draw");
        h = fnv(h, screen_hash());
        h = fold_p_(h, as_ga(a));
        h = fold_p_(h, as_ga(b));
        h = fold_p_(h, as_ga(c));
        h = fnv(h, (uint32_t)ptrs_size(as_ga(b)->emitter->qy));
    }

    cut_qy(as_ga(b)->emitter->qy, 3);
    for (i = 0; i < 3; i++) {
        pico8_dc("draw");
        h = fnv(h, screen_hash());
    }

    h = fold_cut(h, a, 3);
    assert(!as_ga(a)->p_);
    h = fold_cut(h, a, 2);
    assert(as_ga(a)->p_);
    h = fold_cut(h, a, 0);
    assert(as_ga(a)->p_);

    h = fold_bool(h, cls_of(b)->hit_test(b, as_ga(b)->ra, as_ga(b)->ra));
    h = fold_bool(h, cls_of(b)->hit_test(b, vec_ref(near_mo(0, 0)), vec_ref(near_mo(0, 0))));
    h = fnv(fnv(h, 0xB0u), pico8_jo_gg(b) ? 1u : 0u);
    assert(pico8_jo_gg(a) == 0);

    cls_of(a)->refresh(a);
    h = fold_ref_member(h, as_ga(a)->lq);
    assert(as_ga(a)->lq == NULL);
    kw.has_qe = 1;
    kw.qe = pico8_fixed_from_int(12);
    kw.mu = pico8_fixed_from_int(2);
    as_ga(a)->kw = &kw;
    cls_of(a)->refresh(a);
    row = fold_lq(2166136261u, a);
    h = fnv(h, row);
    by = as_ga(c)->by;
    assert(by == as_ga(a)->by && by->count == 0);
    pico8_ents_add(by, &pico8_trade_new(pico8_zone_partie(), NULL)->header);
    pico8_ents_add(by, &pico8_trade_new(pico8_zone_partie(), NULL)->header);
    cls_of(a)->refresh(a);
    assert(fold_lq(2166136261u, a) != row);
    h = fold_lq(h, a);
    assert(ents_ptrs_size(as_ga(b)->by) == 2);
    /* `jo:ib` is `self.by or {}`: the `by` held, routes kept, never a fresh one. */
    assert(pico8_jo_ib(as_ga(b)) == by && by->count == 2);

    hv_count = pico8_hv()->count;
    cls_of(b)->new_year(b, pico8_fixed_from_int(0));
    assert(pico8_hv()->count == hv_count && !as_ga(b)->has_kr);
    h = fnv(h, screen_hash());
    check("life", h, LIFE_HASH);
}

/* A harvest played frame by frame: one live burst at a time, gone after
 * BURST_FRAMES frames. */
static void test_burst(void) {
    int frames = 0;
    void *e;

    start_game();
    e = harvest(40, 40, "s64");
    assert(ga_from(0) == e);
    while (ga_from(0) != NULL) {
        e = ga_from(0);
        assert(ga_from(ents_rank_in(pico8_hv(), e) + 1) == NULL);
        pico8__update60();
        pico8__draw();
        frames++;
        assert(frames < 600);
    }
    check_int("burst frames", frames, BURST_FRAMES);
    pico8_cart_vider_file_sortie();
    check_int("ga places after the burst", pico8_zone_particules_vivantes(PICO8_PARTICULES_GA), 0);
}

/* `ei` moves a particle of `qy` in place: `p` by the old `v`, `v` turned
 * (its second row reads the x just written), then accelerated by `a`, and
 * `l` less 0.14. A protostar's burst turns (`qg` is not 0). */
static void test_ei_moves_a_particle_in_place(void) {
    pico8_qy_t *particle = pico8_particule_prendre();
    pico8_vec_t p0 = vec_xy(3, -2);
    pico8_vec_t v0 = pico8_vec_v(PICO8_FIXED_LIT(0x00018000, 1.5), PICO8_FIXED_LIT(0x00004000, 0.25));
    pico8_vec_t a0 = pico8_vec_v(PICO8_FIXED_LIT(0x00002000, 0.125), PICO8_FIXED_LIT(0xFFFF8000, -0.5));
    pico8_ptrs_t *qy;
    pico8_fixed_t vx;
    pico8_fixed_t vy;
    void *e;
    const pico8_emitter_t *em;

    start_game();
    e = spawn(84, 60, "s73", NULL);
    em = as_ga(e)->emitter;
    assert(em->qg != 0);
    particle->p = p0;
    particle->v = v0;
    particle->has_a = 1;
    particle->a = a0;
    particle->l = pico8_fixed_from_int(5);
    qy = em->qy;
    cut_qy(qy, 0);
    pico8_ptrs_add(qy, particle);

    pico8_ei(&gfx, &mem, e);

    vx = pico8_fixed_add(pico8_fixed_mul(em->qo, v0.x), pico8_fixed_mul(em->ro, v0.y));
    vy = pico8_fixed_add(pico8_fixed_mul(em->qg, vx), pico8_fixed_mul(em->q_, v0.y));
    assert(qy->count >= 1 && qy->items[0] == particle);
    assert(particle->p.x == pico8_fixed_add(p0.x, v0.x) && particle->p.y == pico8_fixed_add(p0.y, v0.y));
    assert(particle->v.x == pico8_fixed_add(vx, a0.x) && particle->v.y == pico8_fixed_add(vy, a0.y));
    assert(particle->a.x == a0.x && particle->a.y == a0.y);
    assert(particle->l == pico8_fixed_sub(pico8_fixed_from_int(5), PICO8_FIXED_LIT(0x000023d7, 0.14)));
}

static int is_a_ga_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_ga_struct_class();
}

/* Built by any road, a burst is a struct whose descriptor holds jo's very
 * entries but its own `kj`, plus its own `mm`. Neither ga's class nor jo's
 * installs a function or a `pq`. */
static void test_ga_is_a_struct(void) {
    const entity_class_t *cls = pico8_ga_struct_class();
    const entity_class_t *jo = pico8_jo_struct_class();
    void *roads[3];
    size_t i;
    int j;

    start_game();
    roads[0] = harvest(40, 40, "s81");
    roads[1] = spawn(80, 70, "s82", NULL);
    roads[2] = pico8_ga_new(NULL);
    for (i = 0; i < 3; i++) {
        assert(is_a_ga_struct(roads[i]));
    }
    assert(cls->draw != NULL && cls->draw != jo->draw);
    assert(cls->draw_overlay == jo->draw_overlay && cls->hit_test == jo->hit_test && cls->new_year == jo->new_year && cls->refresh == jo->refresh &&
           cls->has_refresh == jo->has_refresh && cls->left_press == jo->left_press && cls->right_press == jo->right_press && cls->accept_drop == jo->accept_drop);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == jo->states[j]);
    }
    assert(cls->spawn_particle == pico8_ga_mm && jo->spawn_particle == NULL);
    assert(cls->drag == jo->drag && cls->drop == jo->drop && cls->attach_slipway == jo->attach_slipway && cls->offer_trade == jo->offer_trade && cls->draw_at == jo->draw_at);
    assert(pico8_ga_ready());
    assert(pico8_jo_ready());
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

/* What an emitter holds in the particle zone, by sort. */
typedef struct {
    int particules;
    int qy;
    int qy_fv;
    int ga;
    int rangees;
} held_t;

static held_t held_now(void) {
    held_t h;
    h.particules = pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE);
    h.qy = pico8_zone_particules_vivantes(PICO8_PARTICULES_QY);
    h.qy_fv = pico8_zone_particules_vivantes(PICO8_PARTICULES_QY_FV);
    h.ga = pico8_zone_particules_vivantes(PICO8_PARTICULES_GA);
    h.rangees = pico8_zone_interface_vivantes(PICO8_SORTE_RANGEE);
    return h;
}

/* A burst that leaves (`he`, then the exit queue's flush) gives back the
 * particles it still holds, its `qy`, then its own place. */
static void test_leaving_gives_back_its_particles(void) {
    held_t built;
    held_t after;
    pico8_jo_t *e;
    int count;

    start_game();
    pico8_cart_vider_file_sortie();
    e = as_ga(harvest(40, 40, "s74"));
    count = e->emitter->qy->count;
    assert(count == 140);
    built = held_now();
    pico8_he(&e->header);
    pico8_cart_vider_file_sortie();
    after = held_now();
    assert(after.particules == built.particules - count && after.qy == built.qy - 1);
    assert(after.ga == built.ga - 1);
}

/* The largest bursts: a protostar's, which copies its `ra` into its place,
 * and a harvest's, which borrows the planet's; each with its `oe`. Neither
 * stops on « zone place ga pleine ». */
static void test_the_largest_bursts_fit_their_place(void) {
    held_t before;

    start_game();
    pico8_cart_vider_file_sortie();
    before = held_now();
    (void)harvest(40, 40, "s84");
    (void)spawn(80, 70, "s83", NULL);
    assert(held_now().ga == before.ga + 2);
    printf("ga place: %lu bytes at most\n", (unsigned long)pico8_zone_particules_max_octets(PICO8_PARTICULES_GA));
}

/* A burst can be hovered, so a dragged slipway can end on it; the flush
 * that gives the burst back lets go of that end too. */
static void test_leaving_lets_go_of_a_dragged_slipway(void) {
    planet_keys_t keys;
    pico8_jc_keys_t jc_keys;
    pico8_jc_t *jc;
    pico8_vec_t mp = vec_xy(40, 40);
    void *e;

    start_game();
    pico8_cart_vider_file_sortie();
    keys.planet_id = "s85";
    keys.map_pos = vec_ref(vec_xy(30, 30));
    keys.pos = vec_ref(near_mo(60, 60));
    jc_keys.qu = &pico8_jo_new(&keys)->header;
    jc = pico8_jc_new(&jc_keys);
    e = harvest(40, 40, "s86");
    pico8_ok()->lheld = &jc->header;
    pico8_ok()->o_ = e;
    pico8_jc_hq(jc, &mp);
    assert(jc->re == e);
    pico8_he(e);
    pico8_cart_vider_file_sortie();
    assert(jc->re == NULL);
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
    in_fresh_child(test_burst);
    in_fresh_child(test_ei_moves_a_particle_in_place);
    in_fresh_child(test_ga_is_a_struct);
    in_fresh_child(test_leaving_gives_back_its_particles);
    in_fresh_child(test_the_largest_bursts_fit_their_place);
    in_fresh_child(test_leaving_lets_go_of_a_dragged_slipway);

    printf("test_pico8_ga: all tests passed\n");
    return 0;
}
