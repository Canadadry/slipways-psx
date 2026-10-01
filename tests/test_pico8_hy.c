/* The economy `hy`, the one `fm` per game, read on its struct members: its
 * construction, its five functions `nx`, `hk`,
 * `lu`, `lj`, `kt`, and the cheat code on the provisional `fm` of the title
 * screen, in games built directly: no events file. */

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
#include "game/cheat.h"
#include "engine/dispatch_context.h"
#include "game/economy.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/message_box.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_slipway.h"
#include "engine/entity_class.h"
#include "../src/pico8_ui.h"
#include "engine/vec.h"
#include "engine/game_zone.h"
#include "zone_tas.h"

/* `gb[12]`, a colony that needs two resources and exports one; `gb[3]`, a
 * project with an `o` and no `i`, which exports to any planet a slipway
 * joins it to. */
#define COLONY 12
#define EXPORTER 3

/* The tag entity structs carried, which the frozen hashes fold: any other
 * value moves every *_HASH. */
#define FROZEN_STRUCT_TAG 0x9E5C57C7u

#define CONSTRUCTION_HASH 0x8343CAA6u
#define NX_HASH 0xC143EEC1u
#define HK_HASH 0x6CD37E64u
#define LU_HASH 0xB07FE872u
#define LJ_HASH 0x77BE9594u
#define KT_HASH 0x5EE5D720u

/* The tag `invented` carried as a tagged list, which the frozen hashes fold:
 * any other value moves every *_HASH. */
#define FROZEN_LIST_TAG 0x9E5C1157u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

/* NULL on the provisional `fm`, as the functions of `hy` take it. */
static pico8_hy_t *fm_self(void) {
    return pico8_fm_hy();
}

static pico8_hy_t *fm_hy(void) {
    assert(pico8_fm_hy() != NULL);
    return pico8_fm_hy();
}

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static const pico8_ptrs_t *invented_of(const pico8_hy_t *hy) {
    const pico8_ptrs_t *l = hy->invented;
    assert(l != NULL);
    return l;
}

static uint32_t fold_string(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFu);
}

/* The kind words the frozen hashes fold before a value: any other value
 * moves them. */
enum { FIXED_KIND = 0, STRING_KIND = 1, BOOL_KIND = 2, REF_KIND = 3 };

static uint32_t fold_fixed(uint32_t h, pico8_fixed_t x) {
    return fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)x);
}

/* The byte an ob table folded before a positional key, frozen in the
 * constants that fold `po`/`fu`; the enum that gave it is deleted.
 * Changing it reddens those constants. */
enum { TABLE_INDEX_KEY = 0 };

/* `REF`, the count, then per entry the index-key kind, `qe` and the
 * value: the format the constants froze. */
static uint32_t fold_qe_map(uint32_t h, const pico8_qe_map_t *map) {
    int i;

    h = fnv(h, (uint32_t)REF_KIND);
    h = fnv(h, (uint32_t)map->count);
    for (i = 0; i < map->count; i++) {
        h = fnv(h, (uint32_t)TABLE_INDEX_KEY);
        h = fnv(h, (uint32_t)map->entries[i].qe);
        h = fold_fixed(h, map->entries[i].v);
    }
    return h;
}

static uint32_t fold_absent(uint32_t h) {
    return fnv(h, 0xFFFFFFFFu);
}

/* A member of one kind: the kind word and the list's tag are constant by
 * the struct's shape, kept so that the frozen constants still prove the typed
 * reads. An unset flag or a NULL member folds as absent. */
static uint32_t fold_fixed_member(uint32_t h, int has, pico8_fixed_t v) {
    return has ? fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)v) : fold_absent(h);
}

static uint32_t fold_list_member(uint32_t h, const pico8_ptrs_t *l) {
    int i;

    if (l == NULL) {
        return fold_absent(h);
    }
    h = fnv(h, (uint32_t)REF_KIND);
    h = fnv(h, FROZEN_LIST_TAG);
    h = fnv(h, (uint32_t)l->count);
    for (i = 0; i < l->count; i++) {
        h = fold_string(fnv(h, (uint32_t)STRING_KIND), l->items[i]);
    }
    return h;
}

static uint32_t fold_qe_member(uint32_t h, const pico8_qe_map_t *map) {
    return map == NULL ? fold_absent(h) : fold_qe_map(h, map);
}

/* `hl` false is `has_hl` with a NULL `hl`. */
static uint32_t fold_hl(uint32_t h, const pico8_hy_t *hy) {
    if (!hy->has_hl) {
        return fold_absent(h);
    }
    if (hy->hl == NULL) {
        return fnv(fnv(h, (uint32_t)BOOL_KIND), 0u);
    }
    return fold_string(fnv(h, (uint32_t)STRING_KIND), hy->hl);
}

/* The 11 members of a `hy`, each with its kind and value. */
static uint32_t fold_hy(uint32_t h, const pico8_hy_t *hy) {
    h = fold_fixed_member(h, 1, hy->nz);
    h = fold_fixed_member(h, 1, hy->ge);
    h = fold_fixed_member(h, 1, hy->jk);
    h = fold_fixed_member(h, 1, hy->ew);
    h = fold_list_member(h, hy->invented);
    h = fold_fixed_member(h, 1, hy->lo);
    h = fold_fixed_member(h, 1, hy->yr);
    h = fold_qe_member(h, hy->po);
    h = fold_qe_member(h, hy->fu);
    h = fold_fixed_member(h, hy->has_happiness, hy->happiness);
    return fold_hl(h, hy);
}

static uint32_t fold_fm(uint32_t h) {
    return fold_hy(h, fm_hy());
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static void new_game(void) {
    pico8_eu(NULL);
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
}

static pico8_jo_t *as_jo(void *e) {
    assert(pico8_jo_is_family(e));
    return e;
}

static pico8_vec_t pos_of(void *e) {
    const pico8_vec_t *p = as_jo(e)->p;
    assert(p != NULL);
    return *p;
}

static pico8_fixed_t dist2(pico8_vec_t a, pico8_vec_t b) {
    pico8_vec_t d = pico8_vec_sub(a, b);
    return pico8_fixed_add(pico8_fixed_mul(d.x, d.x), pico8_fixed_mul(d.y, d.y));
}

static pico8_hp_t *project(int k) {
    assert(pico8_gb_at(k) != NULL);
    return pico8_gb_at(k);
}

/* A generated planet colonized by COLONY, its nearest one by EXPORTER, the
 * two joined by a slipway paid by `ey` as a drag pays it (`jc:hq` reads the
 * hovered planet off `ok.o_`), then left to settle. */
static void built_world(void) {
    const pico8_ents_t *jos;
    pico8_jc_keys_t jc_keys = {0};
    pico8_vec_t *mp = malloc(sizeof *mp);
    void *qu;
    void *re = NULL;
    pico8_jc_t *jc;
    int i;

    start_game();
    jos = pico8_cg("jo");
    assert(jos != NULL && jos->count > 1);
    qu = jos->items[0];
    for (i = 1; i < jos->count; i++) {
        void *e = jos->items[i];
        if (re == NULL || dist2(pos_of(e), pos_of(qu)) < dist2(pos_of(re), pos_of(qu))) {
            re = e;
        }
    }
    pico8_jo_hu(qu, project(COLONY));
    pico8_jo_hu(re, project(EXPORTER));

    jc_keys.qu = qu;
    jc = pico8_jc_new(&jc_keys);
    *mp = pico8_vec_add(pico8_vec_mul(pos_of(re), pico8_fixed_from_int(128)),
                        pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(1)));
    pico8_ok()->o_ = re;
    pico8_jc_struct_class()->hq(&jc->header, mp, NULL);
    pico8_ok()->o_ = NULL;
    while (jc->header.t < pico8_fixed_from_int(15)) {
        pico8_dg();
    }
    pico8_jc_struct_class()->ey(&jc->header, re);
    for (i = 0; i < 120; i++) {
        pico8_dg();
    }
}

/* The descriptor every `iv` shares, taken off one built for that purpose;
 * it stays in `hv`. */
static const pico8_struct_class_t *iv_descriptor(void) {
    pico8_iv_keys_t keys = {0};
    const pico8_iv_t *ref;

    keys.at1 = "ref";
    ref = pico8_iv_new(&keys);
    return ref->header.cls;
}

/* Every entry `hv` gained since it held `from` entries: its tag, and for an
 * `iv` the text and color it shows. */
static uint32_t fold_new_entities(uint32_t h, int from, const pico8_struct_class_t *iv_cls) {
    const pico8_ents_t *hv = pico8_hv();
    int i;

    h = fnv(h, (uint32_t)(hv->count - from));
    for (i = from; i < hv->count; i++) {
        const void *e = hv->items[i];
        h = fnv(h, FROZEN_STRUCT_TAG);
        if (((const pico8_struct_header_t *)e)->cls == iv_cls) {
            const pico8_iv_t *iv = e;
            h = iv->at1 != NULL ? fold_string(fnv(h, (uint32_t)STRING_KIND), iv->at1)
                                : fnv(fnv(h, (uint32_t)REF_KIND), 0xDEADu);
            h = fold_fixed(h, iv->at2);
        }
    }
    return h;
}

/* A `hy` built on its own holds the 7 names of its spec, `invented` an empty
 * list, and none of the 4 names `lj` writes; `fm` after `eu()` holds all 11.
 * Two games have two distinct `invented` lists. */
static void test_construction(void) {
    uint32_t h = 2166136261u;
    pico8_hy_t *bare;
    const pico8_ptrs_t *first;

    pico8__init();
    bare = pico8_hy_new();
    assert(bare->invented != NULL && bare->po == NULL && bare->fu == NULL && !bare->has_happiness);
    assert(!bare->has_hl);
    h = fold_hy(h, bare);
    assert(invented_of(bare)->count == 0);
    assert(bare->nz == pico8_fixed_from_int(100) && bare->yr == pico8_fixed_from_int(3401));

    new_game();
    assert(fm_hy()->invented != NULL && fm_hy()->po != NULL && fm_hy()->fu != NULL);
    assert(fm_hy()->has_happiness && fm_hy()->has_hl);
    h = fold_fm(h);
    first = invented_of(fm_hy());
    assert(first != invented_of(bare));

    pico8_cart_vider_file_sortie();
    new_game();
    assert(invented_of(fm_hy()) != first && invented_of(fm_hy())->count == 0);
    h = fold_fm(h);
    check("construction", h, CONSTRUCTION_HASH);
}

static uint32_t fold_lo_yr(uint32_t h) {
    h = fold_fixed_member(h, 1, fm_hy()->lo);
    return fold_fixed_member(h, 1, fm_hy()->yr);
}

/* `lo` accumulates; past 12 without "time compression", past 15 with it,
 * the year turns once and `lo` drops by that bound. */
static void test_nx(void) {
    uint32_t h = 2166136261u;
    pico8_fixed_t yr;

    start_game();
    yr = fm_hy()->yr;
    assert(fm_hy()->lo == pico8_fixed_from_int(1));

    pico8_hy_nx(fm_self(), pico8_fixed_from_int(3));
    assert(fm_hy()->lo == pico8_fixed_from_int(4) && fm_hy()->yr == yr);
    h = fold_lo_yr(h);
    pico8_hy_nx(fm_self(), pico8_fixed_from_int(8));
    assert(fm_hy()->lo == pico8_fixed_from_int(12) && fm_hy()->yr == yr);
    h = fold_lo_yr(h);
    pico8_hy_nx(fm_self(), pico8_fixed_from_int(1));
    assert(fm_hy()->lo == pico8_fixed_from_int(1));
    assert(fm_hy()->yr == pico8_fixed_add(yr, pico8_fixed_from_int(1)));
    h = fold_fm(h);

    pico8_ptrs_add(fm_hy()->invented, "time compression");
    pico8_hy_nx(fm_self(), pico8_fixed_from_int(14));
    assert(fm_hy()->lo == pico8_fixed_from_int(15));
    assert(fm_hy()->yr == pico8_fixed_add(yr, pico8_fixed_from_int(1)));
    h = fold_lo_yr(h);
    pico8_hy_nx(fm_self(), PICO8_FIXED_LIT(0x00008000, 0.5));
    assert(fm_hy()->lo == PICO8_FIXED_LIT(0x00008000, 0.5));
    assert(fm_hy()->yr == pico8_fixed_add(yr, pico8_fixed_from_int(2)));
    h = fold_fm(h);

    pico8_hy_nx(NULL, pico8_fixed_from_int(20));
    h = fold_fm(h);
    check("nx", h, NX_HASH);
}

/* After a game with colonies: `nz += ge`, `jk += po[r.k] + 1`, `yr + 1`,
 * and each planet of `cv.hk` shows its income. */
static void test_hk(void) {
    uint32_t h = 2166136261u;
    const pico8_struct_class_t *iv_cls;
    pico8_fixed_t nz;
    pico8_fixed_t ge;
    pico8_fixed_t jk;
    pico8_fixed_t yr;
    int from;

    built_world();
    iv_cls = iv_descriptor();
    nz = fm_hy()->nz;
    ge = fm_hy()->ge;
    jk = fm_hy()->jk;
    yr = fm_hy()->yr;
    h = fold_fm(h);

    from = pico8_hv()->count;
    pico8_hy_hk(fm_self());
    assert(fm_hy()->nz == pico8_fixed_add(nz, ge));
    assert(fm_hy()->jk > jk);
    assert(fm_hy()->yr == pico8_fixed_add(yr, pico8_fixed_from_int(1)));
    h = fold_fm(h);
    h = fold_new_entities(h, from, iv_cls);
    assert(pico8_hv()->count > from);

    pico8_hy_hk(NULL);
    h = fold_fm(h);
    check("hk", h, HK_HASH);
}

static uint32_t lu_step(uint32_t h, pico8_hy_t *self, int jt, int want, const pico8_struct_class_t *iv_cls) {
    int from = pico8_hv()->count;
    int got = pico8_hy_lu(self, pico8_fixed_from_int(jt));
    assert(got == want);
    h = fnv(h, (uint32_t)got);
    h = fold_fixed_member(h, 1, fm_hy()->nz);
    return fold_new_entities(h, from, iv_cls);
}

/* Short of credits: 0 and "-no money-"; 0: paid, nothing shown; a cost and
 * a payment: `nz` and the amount shown; a NULL `hy` holds no credits. */
static void test_lu(void) {
    uint32_t h = 2166136261u;
    const pico8_struct_class_t *iv_cls;
    int from;

    start_game();
    iv_cls = iv_descriptor();
    assert(fm_hy()->nz == pico8_fixed_from_int(100));

    from = pico8_hv()->count;
    h = lu_step(h, fm_self(), 101, 0, iv_cls);
    assert(fm_hy()->nz == pico8_fixed_from_int(100) && pico8_hv()->count == from + 1);
    assert(strcmp(((const pico8_iv_t *)pico8_hv()->items[from])->at1, "-no money-") == 0);

    from = pico8_hv()->count;
    h = lu_step(h, fm_self(), 0, 1, iv_cls);
    assert(fm_hy()->nz == pico8_fixed_from_int(100) && pico8_hv()->count == from);

    h = lu_step(h, fm_self(), 30, 1, iv_cls);
    assert(fm_hy()->nz == pico8_fixed_from_int(70));
    h = lu_step(h, fm_self(), -20, 1, iv_cls);
    assert(fm_hy()->nz == pico8_fixed_from_int(90));
    h = lu_step(h, fm_self(), 90, 1, iv_cls);
    assert(fm_hy()->nz == pico8_fixed_from_int(0));

    h = lu_step(h, NULL, 5, 0, iv_cls);
    h = lu_step(h, NULL, 0, 1, iv_cls);
    h = lu_step(h, NULL, -3, 1, iv_cls);
    check("lu", h, LU_HASH);
}

static uint32_t fold_planets_kr(uint32_t h) {
    const pico8_ents_t *jo = pico8_cg("jo");
    int i;
    for (i = 0; jo != NULL && i < jo->count; i++) {
        const pico8_jo_t *p = as_jo(jo->items[i]);
        h = fold_fixed_member(h, p->has_kr, p->kr);
    }
    return h;
}

/* NULL: `hl` false. */
static const char *hl_string(void) {
    assert(fm_hy()->has_hl);
    return fm_hy()->hl;
}

/* The first producing planet of `cg.jo`: `jy` set and its own `kw`. */
static pico8_jo_t *a_maker(void) {
    const pico8_ents_t *jos = pico8_cg("jo");
    int i;

    for (i = 0; i < jos->count; i++) {
        pico8_jo_t *p = as_jo(jos->items[i]);
        if (p->jy != NULL && p->kw != NULL && p->kw->has_qe) {
            return p;
        }
    }
    assert(0 && "built_world() makes a producing planet");
    return NULL;
}

/* `po`/`fu` recomputed each call -- a planet's `qe` moved shows in `po`, a
 * resource no planet makes any more is gone from it --, `ge`, `happiness`,
 * each planet's `kr`, and `hl`: false, "final score" at 3426, "bankrupt"
 * with no income and under 3 credits; then a `qh` needing a resource
 * outside `r` gets it in `fu`, from 1, gone once the need is. */
static void test_lj(void) {
    uint32_t h = 2166136261u;
    const pico8_qe_map_t *po;
    const pico8_qe_map_t *fu;
    pico8_jo_t *maker;
    pico8_jo_t *needer;
    pico8_ptrs_t needs;
    pico8_fixed_t need_qe = pico8_fixed_from_int(99);
    pico8_jy_t *need;
    pico8_fixed_t made;
    pico8_fixed_t now;
    pico8_fixed_t qe;

    built_world();
    po = fm_hy()->po;
    fu = fm_hy()->fu;
    assert(po != NULL && fu != NULL);
    maker = a_maker();
    qe = maker->kw->qe;
    assert(pico8_qe_map_get(po, qe, &made) && maker->kw->mu > 0);
    maker->kw->qe = pico8_fixed_from_int(99);
    pico8_hy_lj(fm_self());
    assert(pico8_qe_map_get(po, pico8_fixed_from_int(99), &now) && now == maker->kw->mu);
    assert(pico8_qe_map_get(po, qe, &now) && now == pico8_fixed_sub(made, maker->kw->mu));
    maker->kw->qe = qe;
    pico8_hy_lj(fm_self());
    assert(!pico8_qe_map_get(po, pico8_fixed_from_int(99), &now));
    assert(pico8_qe_map_get(po, qe, &now) && now == made);
    assert(fm_hy()->po == po && fm_hy()->fu == fu);
    assert(hl_string() == NULL);
    h = fold_fm(h);
    h = fold_planets_kr(h);

    fm_hy()->yr = pico8_fixed_from_int(3426);
    pico8_hy_lj(fm_self());
    assert(hl_string() != NULL && strcmp(hl_string(), "final score") == 0);
    h = fold_fm(h);

    fm_hy()->yr = pico8_fixed_from_int(3425);
    fm_hy()->nz = pico8_fixed_from_int(2);
    pico8_hy_lj(fm_self());
    h = fold_fm(h);
    h = fnv(h, (uint32_t)(hl_string() == NULL));

    pico8_cart_vider_file_sortie();
    new_game();
    fm_hy()->nz = pico8_fixed_from_int(2);
    pico8_hy_lj(fm_self());
    assert(fm_hy()->ge <= pico8_fixed_from_int(0));
    assert(hl_string() != NULL && strcmp(hl_string(), "bankrupt") == 0);
    h = fold_fm(h);
    fm_hy()->nz = pico8_fixed_from_int(3);
    pico8_hy_lj(fm_self());
    assert(hl_string() == NULL);
    h = fold_fm(h);

    pico8_hy_lj(NULL);
    h = fold_fm(h);
    check("lj", h, LJ_HASH);

    fu = fm_hy()->fu;
    need = pico8_jy_new(pico8_zone_partie(), &need_qe);
    needer = pico8_qh_new(NULL);
    pico8_ptrs_init(&needs, &zone_tas);
    pico8_ptrs_add(&needs, need);
    needer->jy = &needs;
    pico8_hy_lj(fm_self());
    assert(pico8_qe_map_get(fu, pico8_fixed_from_int(99), &now) && now == pico8_fixed_from_int(1) - 0x2666);
    needer->jy = NULL;
    pico8_hy_lj(fm_self());
    assert(!pico8_qe_map_get(fu, pico8_fixed_from_int(99), &now));
}

/* An nb entry of cost `od` whose `nk` (0) no resource of `r` is: gi reads
 * no `fu` for it. Never freed. */
static const pico8_nb_entry_t *tech(const char *px, int od) {
    pico8_nb_entry_t *t = malloc(sizeof *t);

    *t = (pico8_nb_entry_t){px, 0, NULL, 0, od, NULL};
    return t;
}

/* `jk -= gi(t)`, `px` appended (nothing without `px`), `ew` up one at each
 * threshold of `ek` while under 4, never past it. */
static void test_kt(void) {
    static const char *const pxs[] = {"t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9", "t10", "t11", "t12", "t13"};
    uint32_t h = 2166136261u;
    size_t k;

    start_game();
    fm_hy()->jk = pico8_fixed_from_int(500);

    pico8_hy_kt(fm_self(), tech(NULL, 10));
    assert(fm_hy()->jk == pico8_fixed_from_int(495));
    assert(invented_of(fm_hy())->count == 0 && fm_hy()->ew == pico8_fixed_from_int(1));
    h = fold_fm(h);

    for (k = 0; k < sizeof pxs / sizeof pxs[0]; k++) {
        pico8_hy_kt(fm_self(), tech(pxs[k], (int)k));
        assert(invented_of(fm_hy())->count == (int)k + 1);
        h = fnv(h, (uint32_t)fm_hy()->ew);
        h = fnv(h, (uint32_t)fm_hy()->jk);
    }
    assert(fm_hy()->ew == pico8_fixed_from_int(4));
    h = fold_fm(h);

    pico8_hy_kt(fm_self(), NULL);
    pico8_hy_kt(NULL, tech("t14", 3));
    assert(invented_of(fm_hy())->count == (int)(sizeof pxs / sizeof pxs[0]));
    h = fold_fm(h);
    check("kt", h, KT_HASH);
}

/* The full A A B B A A B B on the title screen writes the provisional `fm`;
 * the `hy` of the game then starts from its own spec. */
static void test_cheat_on_provisional(void) {
    static const int presses[] = {4, 4, 5, 5, 4, 4, 5, 5};
    pico8_input_frame_t frames[16];
    pico8_input_t pad;
    const pico8_fm_placeholder_t *provisional;
    int fired = 0;
    int i;

    pico8__init();
    provisional = pico8_fm_placeholder();
    assert(provisional != NULL && !provisional->has_jk && !provisional->has_nz);

    memset(frames, 0, sizeof frames);
    for (i = 0; i < 8; i++) {
        frames[2 * i].dpad[presses[i]] = 1;
    }
    pad.frames = frames;
    pad.frame_count = 16;
    pico8_cheat_init(&cheat);
    for (pad.current_frame = 0; pad.current_frame < 16; pad.current_frame++) {
        fired += pico8_cheat_step(&cheat, &pad);
    }
    assert(fired == 1 && pico8_fm_placeholder() == provisional);
    assert(provisional->has_jk && provisional->jk == pico8_fixed_from_int(99));
    assert(provisional->has_nz && provisional->nz == pico8_fixed_from_int(200));

    new_game();
    assert(fm_hy()->jk == pico8_fixed_from_int(0));
    assert(fm_hy()->nz == pico8_fixed_from_int(100));
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

static void lu_on_other(void *e) {
    (void)pico8_hy_lu(e, pico8_fixed_from_int(1));
}

/* `fm` is a struct whose descriptor has no method;
 * `hy` takes no key, its functions refuse anything but a `hy`, and the
 * provisional `fm` is never handed to them. */
static void test_hy_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_hy_struct_class();
    pico8_hy_t *bare;
    int j;

    assert(pico8_hy_ready());
    pico8__init();
    assert(pico8_fm() != NULL && ((const pico8_struct_header_t *)pico8_fm())->cls == pico8_fm_placeholder_struct_class());
    assert(pico8_fm_hy() == NULL);

    new_game();
    assert(pico8_fm() != NULL);
    assert(((const pico8_struct_header_t *)pico8_fm())->cls == cls && pico8_fm_hy() == (void *)pico8_fm());
    assert(cls->kj == NULL && cls->ev == NULL && cls->hd == NULL && cls->hk == NULL && cls->hb == NULL &&
           cls->has_hb == NULL && cls->ldown == NULL && cls->rdown == NULL && cls->gv == NULL);
    assert(cls->hq == NULL && cls->ey == NULL && cls->kf == NULL && cls->cn == NULL && cls->ol == NULL &&
           cls->mm == NULL);
    for (j = 0; j < PICO8_STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }

    bare = pico8_hy_new();
    assert(bare != NULL && bare->header.cls == cls);

    must_abort(lu_on_other, pico8_ok());

    pico8_probe_shutdown();
    assert(pico8_fm() == NULL && pico8_fm_hy() == NULL);
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
    in_fresh_child(test_construction);
    in_fresh_child(test_nx);
    in_fresh_child(test_hk);
    in_fresh_child(test_lu);
    in_fresh_child(test_lj);
    in_fresh_child(test_kt);
    in_fresh_child(test_cheat_on_provisional);
    in_fresh_child(test_hy_is_a_struct);

    printf("test_pico8_hy: all tests passed\n");
    return 0;
}
