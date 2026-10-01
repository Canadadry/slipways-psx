/* The probe `g_`, read and written only through the dual-form field API,
 * its defaults read on its archetype, built by
 * its three roads (`bg:ldown`, `pico8_g__new(NULL)`, `pico8_g__new` with the
 * 4 keys of probe_mm_rnd_order) and driven through the real `mv`/`dc`/`dg`.
 * Its methods are compared to the public functions. Its daughter `fv` is
 * read on the title screen. No events file. */

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
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_fm.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_particle.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_vec.h"


/* The tag class instances carried, which the frozen hashes fold: any
 * other value moves every *_HASH. */
#define FROZEN_INSTANCE_TAG 0x9E5C1451u

#define BUILT_HASH 0xD5E726A8u
#define LIFE_HASH 0x26179978u
#define EY_ON_HASH 0x7D81D210u
#define EY_OFF_HASH 0x196FE04Du
#define FV_HASH 0xA5CD3A2Du

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};

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

static int ents_list_count(const pico8_ents_t *list) {
    return list != NULL ? list->count : 0;
}

static int is_me(const pico8_struct_header_t *ke, const char *want) {
    return ke->me != NULL && strcmp(ke->me, want) == 0;
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

/* The throwaway target's `fo`, NULL when absent. */
static const void *bv_et_fo(const void *t) {
    assert(is_bv_target(t));
    return ((const pico8_bv_et_t *)t)->fo;
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

/* The words the frozen hashes fold before a value: 0xF1 a number, 0x57 a
 * string; any other moves every *_HASH. */
static uint32_t fold_fixed(uint32_t h, pico8_fixed_t x) {
    return fnv(fnv(h, 0xF1u), (uint32_t)x);
}

static uint32_t fold_string(uint32_t h, const char *s) {
    return fnv_str(fnv(h, 0x57u), s);
}

/* A member the reader always answers: its presence bit and kind byte are
 * constant by the struct's shape, kept so that the hashes still prove the
 * typed reads. */
static uint32_t fold_fixed_member(uint32_t h, pico8_fixed_t x) {
    return fnv(fnv(fnv(h, 1u), 0xF1u), (uint32_t)x);
}

static uint32_t fold_bool_member(uint32_t h, int b) {
    return fnv(fnv(fnv(h, 1u), 0xB0u), (uint32_t)b);
}

static uint32_t fold_string_member(uint32_t h, const char *s) {
    return s == NULL ? fnv(h, 0u) : fnv_str(fnv(fnv(h, 1u), 0x57u), s);
}

/* A pointer that carried no tag: the frozen hashes fold 0. */
static uint32_t fold_untagged(uint32_t h, const void *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    return fnv(fnv(h, 0xEEEEu), 0u);
}

static uint32_t fold_ref_member(uint32_t h, const void *ref) {
    return ref == NULL ? fnv(h, 0u) : fold_untagged(fnv(h, 1u), ref);
}

static uint32_t fold_p_(uint32_t h, const pico8_g__t *g_) {
    return (g_->p_ != 0) ? fold_bool_member(h, g_->p_ != 0) : fnv(h, 0u);
}

/* `g_`'s 18 spec defaults, in spec order. */
static uint32_t fold_defaults(uint32_t h, const pico8_g__t *g_) {
    const pico8_emitter_t *em = g_->emitter;
    const pico8_probe_t *pr = g_->probe;

    h = fold_ref_member(h, &g_->header.cls->tags);
    h = fold_fixed_member(h, g_->def->fx);
    h = fold_string_member(h, pr->def->dq);
    h = fold_fixed_member(h, pr->od);
    h = fold_bool_member(h, pr->def->fp != 0);
    h = fold_fixed_member(h, g_->def->fa);
    h = fold_fixed_member(h, pr->def->dw);
    h = fold_fixed_member(h, em->py);
    h = fold_fixed_member(h, em->def->ki);
    h = fold_fixed_member(h, em->pn);
    h = fold_fixed_member(h, em->def->pk);
    h = fold_fixed_member(h, em->def->fc);
    h = fold_ref_member(h, em->k_);
    h = fold_fixed_member(h, em->qo);
    h = fold_fixed_member(h, em->ro);
    h = fold_fixed_member(h, em->qg);
    h = fold_fixed_member(h, em->q_);
    return fold_fixed_member(h, em->du);
}

static const pico8_hy_t *fm_hy(void) {
    const pico8_hy_t *hy = pico8_fm_hy();
    assert(hy != NULL);
    return hy;
}

/* A class's tags, in enum order: each position, then its name, then
 * the count. */
static uint32_t fold_tags(uint32_t h, unsigned tags) {
    static const char *const names[PICO8_TAG_COUNT] = {
        [PICO8_TAG_QH] = "qh", [PICO8_TAG_HJ] = "hj", [PICO8_TAG_NR] = "nr",
        [PICO8_TAG_G_] = "g_", [PICO8_TAG_MH] = "mh", [PICO8_TAG_JC] = "jc",
        [PICO8_TAG_MZ] = "mz", [PICO8_TAG_JO] = "jo", [PICO8_TAG_NP] = "np",
        [PICO8_TAG_PROBER] = "prober",
    };
    uint32_t k = 0;
    int t;

    for (t = 0; t < PICO8_TAG_COUNT; t++) {
        if (tags & PICO8_TAG(t)) {
            k++;
            h = fnv(h, k);
            h = fold_string(h, names[t]);
        }
    }
    return fnv(h, k);
}

/* An integer sequence: each position, then its value, then the count. */
static uint32_t fold_seq(uint32_t h, const pico8_int_seq_t *s) {
    int i;

    for (i = 0; i < s->count; i++) {
        h = fnv(h, (uint32_t)(i + 1));
        h = fold_fixed(h, pico8_fixed_from_int(s->items[i]));
    }
    return fnv(h, (uint32_t)s->count);
}

/* The item count of a list member, -1 when NULL. */
static int size_of(const pico8_ptrs_t *list) {
    return list == NULL ? -1 : list->count;
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

/* The background is the `bg` of cv.hd whose `hd` and `fa` are both 1. */
static pico8_bg_t *the_bg(void) {
    const pico8_ents_t *hd = pico8_cv("hd");
    pico8_bg_t *found = NULL;
    int count = 0;
    int i;

    for (i = 0; hd != NULL && i < hd->count; i++) {
        const pico8_struct_header_t *e = hd->items[i];
        pico8_bg_t *bg;

        if (e->cls != pico8_bg_struct_class()) {
            continue;
        }
        bg = (pico8_bg_t *)e;
        if (bg->def->hd == pico8_fixed_from_int(1) && bg->def->fa == pico8_fixed_from_int(1)) {
            found = bg;
            count++;
        }
    }
    assert(count == 1);
    return found;
}

/* A probe, whatever its form: `g_`'s tags and `fa`=2 (an `fv` has 10). */
static int is_probe(pico8_struct_header_t *e) {
    const pico8_g__t *g_ =
        e != NULL && (e->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_G_)) ? (const pico8_g__t *)e : NULL;
    return g_ != NULL && g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)) && g_->def->fa == pico8_fixed_from_int(2);
}

static pico8_g__t *as_g_(void *e) {
    assert(e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_g__struct_class());
    return e;
}

/* Road (a): `bg:ldown` while cg.g_ is empty. */
static pico8_g__t *ldown(void) {
    pico8_bg_t *bg = the_bg();
    pico8_down_t out;

    assert(ents_list_count(pico8_cg("g_")) == 0);
    out = cls_of(bg)->ldown(bg, vec_ref(vec_xy(1, 1)), vec_ref(vec_xy(2, 2)));
    assert(out.stop && out.held != NULL && is_probe(out.held));
    return as_g_(out.held);
}

/* Road (c): the 4 keys of probe_mm_rnd_order. */
static pico8_g__t *keyed(void) {
    pico8_g__keys_t keys = {0};

    keys.has_pn = 1;
    keys.pn = pico8_fixed_from_int(7);
    keys.t = pico8_fixed_from_int(3);
    keys.has_du = 1;
    keys.du = pico8_fixed_from_int(10);
    keys.has_py = 1;
    keys.py = pico8_fixed_from_int(1);
    return pico8_g__new(&keys);
}

/* A probe as its road and `f_`, `lt` left it: its 18 defaults, `me`, `t`,
 * its tables' contents, its empty `qy`; none of the names only `bv`/`d_`
 * write; its 5 methods, the public functions; where `lt` registered it. */
static uint32_t fold_probe(uint32_t h, pico8_g__t *e) {
    const pico8_emitter_t *em = e->emitter;
    const pico8_probe_t *pr = e->probe;
    size_t i;

    assert(is_probe(&e->header));
    assert(e->header.p == &e->p);
    h = fold_fixed_member(h, e->def->fx);
    h = fold_string_member(h, pr->def->dq);
    h = fold_fixed_member(h, pr->od);
    h = fold_bool_member(h, pr->def->fp != 0);
    h = fold_fixed_member(h, e->def->fa);
    h = fold_fixed_member(h, pr->def->dw);
    h = fold_fixed_member(h, em->py);
    h = fold_fixed_member(h, em->def->ki);
    h = fold_fixed_member(h, em->pn);
    h = fold_fixed_member(h, em->def->pk);
    h = fold_fixed_member(h, em->def->fc);
    h = fold_fixed_member(h, em->qo);
    h = fold_fixed_member(h, em->ro);
    h = fold_fixed_member(h, em->qg);
    h = fold_fixed_member(h, em->q_);
    h = fold_fixed_member(h, em->du);
    h = fold_string_member(h, e->header.me);
    h = fold_fixed_member(h, e->header.t);
    assert(pr->def->dq != NULL && em->k_ != NULL);
    h = fold_tags(h, e->header.cls->tags);
    h = fold_seq(h, em->k_);
    assert(e->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)));
    assert(em->k_ == &pico8_g__k__seq);
    h = fnv(h, (uint32_t)size_of(em->qy));
    assert(size_of(em->qy) == 0);
    assert(e->ra == NULL && e->p == NULL && pr->et == NULL && !e->p_);
    assert(cls_of(e)->hd == NULL && cls_of(e)->hk == NULL && cls_of(e)->hb == NULL &&
           cls_of(e)->states[PICO8_STATE_LY] == NULL && cls_of(e)->states[PICO8_STATE_PH] == NULL);
    assert(cls_of(e)->hq == pico8_bv_dispatch);
    assert(cls_of(e)->ey == pico8_d__dispatch);
    assert(cls_of(e)->ev == pico8_dl_dispatch);
    assert(cls_of(e)->kj == pico8_ei_dispatch);
    assert(cls_of(e)->mm == pico8_g__mm);
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(pico8_cv(bm[i]), e));
    }
    assert(ents_listed(pico8_cv("kj"), e) && ents_listed(pico8_cv("ev"), e));
    assert(!ents_listed(pico8_cv("hd"), e) && !ents_listed(pico8_cv("hk"), e) && !ents_listed(pico8_cv("hb"), e));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("g_"), e));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("prober"), e));
    assert(ents_listed(pico8_cg("g_"), e) && ents_listed(pico8_cg("prober"), e));
    return h;
}

/* One probe by each road. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    pico8_g__t *a;
    pico8_g__t *b;
    pico8_g__t *c;

    start_game();
    a = ldown();
    h = fold_probe(h, a);
    b = pico8_g__new(NULL);
    h = fold_probe(h, b);
    c = keyed();
    h = fold_probe(h, c);
    assert(a != b && b != c && a != c);
    assert(a->emitter->qy != b->emitter->qy && b->emitter->qy != c->emitter->qy);
    assert(a->def->fa == pico8_fixed_from_int(2) && a->emitter->def->ki == pico8_fixed_from_int(9));
    assert(a->probe->def->dw == 0x666);
    assert(c->emitter->pn == pico8_fixed_from_int(7) && c->header.t == pico8_fixed_from_int(3) &&
           c->emitter->du == pico8_fixed_from_int(10) && c->emitter->py == pico8_fixed_from_int(1));
    assert(b->emitter->pn == pico8_fixed_from_int(3) && b->header.t == pico8_fixed_from_int(0));
    check("built", h, BUILT_HASH);
}

static uint32_t fold_particle(uint32_t h, const pico8_qy_t *q) {
    assert(q != NULL);
    assert(!q->has_a);
    h = fnv(fnv(h, (uint32_t)q->p.x), (uint32_t)q->p.y);
    h = fnv(fnv(h, (uint32_t)q->v.x), (uint32_t)q->v.y);
    return fnv(h, (uint32_t)q->l);
}

/* The kind of the probe's `et`: `false`, a planet, a slipway, the throwaway
 * struct. */
static uint32_t et_kind(const pico8_g__t *e) {
    const void *ref;

    if (e->probe->et == NULL) {
        return 1;
    }
    ref = e->probe->et;
    if (ents_listed(pico8_cg("np"), ref)) {
        return 2;
    }
    if (ref != NULL &&
        ((const pico8_struct_header_t *)ref)->cls == pico8_jc_struct_class()) {
        return 3;
    }
    assert(is_bv_target(ref));
    return 4;
}

/* The `fo` of the probe's `et`, a planet, a slipway or the throwaway. */
static const void *et_fo(const pico8_g__t *e) {
    pico8_struct_header_t *et = e->probe->et;

    assert(et != NULL);
    if (pico8_jo_is_family(et)) {
        return ((const pico8_jo_t *)et)->fo;
    }
    if (et->cls == pico8_jc_struct_class()) {
        return ((const pico8_jc_t *)et)->fo;
    }
    return bv_et_fo(et);
}

/* `et` in the bytes its value read by name folded to: false, the target. */
static uint32_t fold_et(uint32_t h, const pico8_g__t *e) {
    if (e->probe->et == NULL) {
        return fnv(fnv(h, 0xB0u), 0u);
    }
    return fold_ref(h, e->probe->et);
}

/* The first planet of cg.np. */
static pico8_jo_t *a_planet(void) {
    pico8_struct_header_t *e;

    assert(ents_list_count(pico8_cg("np")) > 0);
    e = pico8_cg("np")->items[0];
    assert(pico8_jo_is_family(e));
    return (pico8_jo_t *)e;
}

static pico8_vec_t planet_p(void) {
    const pico8_jo_t *planet = a_planet();
    assert(planet->p != NULL);
    return *planet->p;
}

/* A slipway between two bare planets far from each other and from the
 * rest, its `re` set directly; its middle is the returned point. */
static pico8_jc_t *a_slipway(pico8_vec_t *middle) {
    pico8_jo_keys_t keys;
    pico8_jc_keys_t jc_keys = {0};
    void *qu;
    void *re;
    pico8_jc_t *jc;
    pico8_vec_t base = planet_p();
    pico8_vec_t qu_p = pico8_vec_add(base, vec_xy(0, 1));
    pico8_vec_t re_p = pico8_vec_add(base, vec_xy(1, 1));

    keys.p = vec_ref(qu_p);
    keys.ra = NULL;
    keys.id = "s91";
    qu = pico8_jo_new(&keys);
    keys.p = vec_ref(re_p);
    keys.id = "s92";
    re = pico8_jo_new(&keys);
    jc_keys.qu = qu;
    jc = pico8_jc_new(&jc_keys);
    jc->re = re;
    *middle = pico8_vec_div(pico8_vec_add(qu_p, re_p), pico8_fixed_from_int(2));
    return jc;
}

/* The mouse one pixel off `at` (world units, as `p`): `ij` skips an entity
 * at the very position. */
static pico8_vec_t *mouse_at(pico8_vec_t at) {
    return vec_ref(pico8_vec_add(pico8_vec_mul(at, pico8_fixed_from_int(128)), vec_xy(1, 1)));
}

/* `hq` at `at`, the raw mouse at `rp`: `et`'s kind, the target's `fo`,
 * `ra` the very REF passed, `p`. */
static uint32_t fold_hq(uint32_t h, pico8_g__t *e, pico8_vec_t at, int rp, uint32_t want_kind) {
    pico8_vec_t *mp = mouse_at(at);
    uint32_t kind;

    cls_of(e)->hq(e, mp, vec_ref(vec_xy(rp, rp)));
    kind = et_kind(e);
    printf("[hq] et kind %u\n", (unsigned)kind);
    fflush(stdout);
    assert(kind == want_kind);
    assert(e->ra == mp);
    assert(kind == 1 || et_fo(e) == e);
    h = fnv(h, kind);
    h = fold_et(h, e);
    h = fold_vec(h, e->p);
    return h;
}

/* `mm`; `hq` on a planet, a slipway, the void; `ev` with a target and
 * without; `dc("kj")` with `qy`'s size; `dg` steps, then `p_` set by `ey`
 * taking it out of `hv` and cg. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    pico8_g__t *a;
    pico8_g__t *b;
    pico8_jc_t *jc;
    pico8_vec_t middle;
    int i;

    start_game();
    a = ldown();
    b = pico8_g__new(NULL);
    jc = a_slipway(&middle);
    h = fold_particle(h, cls_of(a)->mm(&a->header));
    h = fold_particle(h, cls_of(b)->mm(&b->header));

    h = fold_hq(h, a, planet_p(), 64, 2);
    h = fold_hq(h, a, middle, 64, 3);
    assert((void *)jc->fo == a);
    h = fold_hq(h, a, pico8_vec_add(middle, vec_xy(0, 3)), 64, 1);
    h = fold_hq(h, a, pico8_vec_add(middle, vec_xy(0, 3)), 2, 4);

    pico8_ke_ks(&a->header, "kz");
    pico8_ke_ks(&b->header, "kz");
    cls_of(a)->ev(&a->header, NULL);
    h = fnv(h, screen_hash());
    cls_of(b)->ev(&b->header, NULL);
    h = fnv(h, screen_hash());
    cls_of(a)->hq(a, mouse_at(planet_p()), vec_ref(vec_xy(64, 64)));
    cls_of(a)->ev(&a->header, NULL);
    h = fnv(h, screen_hash());

    for (i = 0; i < 3; i++) {
        pico8_dc("kj");
        h = fnv(h, screen_hash());
        h = fnv(h, (uint32_t)size_of(a->emitter->qy));
        h = fnv(h, (uint32_t)size_of(b->emitter->qy));
    }
    for (i = 0; i < 3; i++) {
        pico8_dg();
        h = fold_fixed_member(h, a->header.t);
        h = fold_fixed_member(h, b->header.t);
        h = fold_string_member(h, a->header.me);
        assert(ents_listed(pico8_hv(), a) && ents_listed(pico8_hv(), b));
    }
    assert(cls_of(a)->ey == pico8_d__dispatch);
    assert(pico8_d_(&a->header) == 0);
    assert(a->p_);
    pico8_dg();
    assert(!ents_listed(pico8_hv(), a) && !ents_listed(pico8_cg("g_"), a) && !ents_listed(pico8_cg("prober"), a));
    assert(ents_listed(pico8_hv(), b) && ents_listed(pico8_cg("g_"), b));
    h = fnv(h, (uint32_t)pico8_hv()->count);
    check("life", h, LIFE_HASH);
}

/* `ey` with a target: `p_`, nil returned. */
static void test_ey_on(void) {
    uint32_t h = 2166136261u;
    pico8_g__t *a;
    int out;

    start_game();
    a = ldown();
    cls_of(a)->hq(a, mouse_at(planet_p()), vec_ref(vec_xy(64, 64)));
    assert(cls_of(a)->ey == pico8_d__dispatch);
    out = pico8_d_(&a->header);
    assert(out == 0);
    assert(a->p_);
    h = fold_string_member(h, a->header.me);
    h = fold_fixed_member(h, a->header.t);
    h = fold_p_(h, a);
    h = fold_fixed_member(h, fm_hy()->nz);
    check("ey on", h, EY_ON_HASH);
}

/* `ey` with no target and enough credits: `ks(dq)`, true returned. */
static void test_ey_off(void) {
    uint32_t h = 2166136261u;
    pico8_g__t *a;
    int out;

    start_game();
    a = ldown();
    assert(cls_of(a)->ey == pico8_d__dispatch);
    out = pico8_d_(&a->header);
    assert(out == 1);
    assert(!a->p_);
    assert(is_me(&a->header, "ly"));
    assert(a->header.t == pico8_fixed_from_int(0));
    h = fold_string_member(h, a->header.me);
    h = fold_fixed_member(h, a->header.t);
    h = fold_fixed_member(h, fm_hy()->nz);
    check("ey off", h, EY_OFF_HASH);
}

/* `ey` over the void, off the panel: `et` is the throwaway target, which is
 * truthy, so the drop is refused (`p_`, nil returned) and costs nothing. */
static void test_ey_void(void) {
    pico8_g__t *a;
    pico8_vec_t middle;
    pico8_fixed_t nz;
    const char *me;
    int out;

    start_game();
    a = ldown();
    (void)a_slipway(&middle);
    cls_of(a)->hq(a, mouse_at(pico8_vec_add(middle, vec_xy(0, 3))), vec_ref(vec_xy(2, 2)));
    assert(et_kind(a) == 4);
    assert(et_fo(a) == a);
    assert(!a->p_);
    nz = fm_hy()->nz;
    me = a->header.me;
    assert(cls_of(a)->ey == pico8_d__dispatch);
    out = pico8_d_(&a->header);
    assert(out == 0);
    assert(a->p_);
    assert(fm_hy()->nz == nz);
    assert(a->header.me == me);
    assert(et_kind(a) == 4);
    printf("[ey void] refused\n");
}

/* The help: `g_`'s tags and `fa`=10. */
static int is_help(pico8_struct_header_t *e) {
    const pico8_g__t *g_ =
        e != NULL && (e->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_G_)) ? (const pico8_g__t *)e : NULL;
    return g_ != NULL && g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)) && g_->def->fa == pico8_fixed_from_int(10);
}

/* The title screen's `fv`, of `g_`'s lineage: its
 * tags, `fx`, `dq` inherited, `ev`, `mm` the functions of `g_`. */
static void test_fv(void) {
    uint32_t h = 2166136261u;
    const pico8_ents_t *hv;
    pico8_struct_header_t *found = NULL;
    const pico8_g__t *fv;
    int i;

    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    hv = pico8_hv();
    for (i = 0; i < hv->count; i++) {
        pico8_struct_header_t *e = hv->items[i];
        if (is_help(e)) {
            assert(found == NULL);
            found = e;
        }
    }
    assert(found != NULL && !is_probe(found));
    fv = (const pico8_g__t *)found;
    assert(fv->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)));
    assert(fv->def->fx == pico8_fixed_from_int(1));
    assert(fv->probe->def->dq != NULL && strcmp(fv->probe->def->dq, "ly") == 0);
    assert(fv->def->fa == pico8_fixed_from_int(10));
    assert(fv->emitter->k_ == &pico8_fv_k__seq);
    assert(fv->emitter->k_ != &pico8_g__k__seq);
    assert(cls_of(fv)->ev == pico8_dl_dispatch);
    assert(cls_of(fv)->mm == pico8_g__mm);
    assert(cls_of(fv)->hq == pico8_bv_dispatch);
    assert(cls_of(fv)->ey == pico8_d__dispatch);
    assert(cls_of(fv)->kj != pico8_ei_dispatch);
    assert(cls_of(fv)->kj == pico8_fv_kj_dispatch);
    assert(cls_of(fv)->hd != NULL);
    assert(pico8_g__ready());
    assert(size_of(fv->emitter->qy) >= 0);
    h = fnv(h, (uint32_t)size_of(fv->emitter->qy));
    h = fnv(h, (uint32_t)ents_rank_in(hv, fv));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("g_"), fv));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("prober"), fv));
    assert(ents_listed(pico8_cg("g_"), fv) && ents_listed(pico8_cg("prober"), fv));
    h = fold_defaults(h, fv);
    for (i = 0; i < 3; i++) {
        pico8_dc("kj");
        h = fnv(h, screen_hash());
        h = fnv(h, (uint32_t)size_of(fv->emitter->qy));
    }
    check("fv", h, FV_HASH);
}

static int is_a_g__struct(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_g__struct_class();
}

/* Built by any road, a probe is a struct whose descriptor holds the 5
 * public functions of `g_`, which its class no longer installs. The title's
 * `fv` is a struct of its own. */
static void test_g__is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_g__struct_class();
    const pico8_ents_t *hv;
    pico8_g__t *roads[3];
    size_t i;
    int j;

    start_game();
    roads[0] = ldown();
    roads[1] = pico8_g__new(NULL);
    roads[2] = keyed();
    for (i = 0; i < 3; i++) {
        assert(is_a_g__struct(roads[i]));
    }
    assert(cls->hq == pico8_bv_dispatch && cls->ey == pico8_d__dispatch && cls->ev == pico8_dl_dispatch &&
           cls->kj == pico8_ei_dispatch && cls->mm == pico8_g__mm);
    assert(cls->kf == NULL && cls->cn == NULL && cls->ol == NULL);
    assert(cls->hd == NULL && cls->hk == NULL && cls->hb == NULL && cls->has_hb == NULL && cls->ldown == NULL &&
           cls->rdown == NULL && cls->gv == NULL);
    for (j = 0; j < PICO8_STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(pico8_g__ready());

    {
        pico8_g__def_t fa_zero = pico8_g__def;
        pico8_g__t bare;

        fa_zero.fa = 0;
        pico8_g__struct_init(&bare, cls, &fa_zero);
        assert(is_a_g__struct(&bare) && !is_probe(&bare.header));
    }

    pico8__init();
    hv = pico8_hv();
    for (j = 0; j < hv->count; j++) {
        pico8_struct_header_t *e = hv->items[j];
        if (is_help(e)) {
            break;
        }
    }
    assert(j < hv->count);
    assert(hv->items[j] != NULL &&
           ((const pico8_struct_header_t *)hv->items[j])->cls == pico8_fv_struct_class());
}

/* The throwaway target is a struct that keeps `fo`, the probe aiming at it,
 * and has no `ev`. */
static void test_bv_et_is_a_struct(void) {
    pico8_g__t *a;
    void *et;
    pico8_vec_t middle;

    start_game();
    a = ldown();
    (void)a_slipway(&middle);
    cls_of(a)->hq(a, mouse_at(pico8_vec_add(middle, vec_xy(0, 3))), vec_ref(vec_xy(2, 2)));
    assert(a->probe->et != NULL);
    et = a->probe->et;
    assert(is_bv_target(et) && bv_et_fo(et) == a);
    assert(cls_of(et)->ev == NULL);
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
    in_fresh_child(test_ey_on);
    in_fresh_child(test_ey_off);
    in_fresh_child(test_ey_void);
    in_fresh_child(test_fv);
    in_fresh_child(test_g__is_a_struct);
    in_fresh_child(test_bv_et_is_a_struct);

    printf("test_pico8_g_: all tests passed\n");
    return 0;
}
