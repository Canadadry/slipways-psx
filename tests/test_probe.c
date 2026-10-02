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

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "game/help.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "p8/mem.h"
#include "game/particle.h"
#include "game/ui_zone.h"
#include "game/planet.h"
#include "game/probe.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "engine/vec.h"

/* The cart keeps the box of `mp`, the port a copy of it (nil stays nil). */
static int ra_copies(const pico8_vec_t *ra, const pico8_vec_t *mp) {
    return mp == NULL ? ra == NULL : ra != NULL && ra != mp && ra->x == mp->x && ra->y == mp->y;
}


/* The tag class instances carried, which the frozen hashes fold: any
 * other value moves every *_HASH. */
#define FROZEN_INSTANCE_TAG 0x9E5C1451u

#define BUILT_HASH 0xD2F8504Au
#define LIFE_HASH 0x74EB6007u
#define EY_ON_HASH 0x49056D9Bu
#define EY_OFF_HASH 0x3EA86A53u
#define FV_HASH 0x938EC51Bu

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const char *const bm[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};

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

static int is_me(const entity_t *ke, const char *want) {
    return ke->state != NULL && strcmp(ke->state, want) == 0;
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
static uint32_t fold_fixed(uint32_t h, p8_fixed_t x) {
    return fnv(fnv(h, 0xF1u), (uint32_t)x);
}

static uint32_t fold_string(uint32_t h, const char *s) {
    return fnv_str(fnv(h, 0x57u), s);
}

/* A member the reader always answers: its presence bit and kind byte are
 * constant by the struct's shape, kept so that the hashes still prove the
 * typed reads. */
static uint32_t fold_fixed_member(uint32_t h, p8_fixed_t x) {
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

    h = fold_ref_member(h, &g_->header.class->tags);
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
    static const char *const names[TAG_COUNT] = {
        [TAG_STRUCTURE] = "structure", [TAG_ASCENSION] = "ascension", [TAG_SLIPGATE] = "slipgate",
        [TAG_PROBE] = "probe", [TAG_MESSAGE_BOX] = "message_box", [TAG_SLIPWAY] = "slipway",
        [TAG_WORMHOLE] = "wormhole", [TAG_PLANET] = "planet", [TAG_NODE] = "node",
        [TAG_TARGETING] = "targeting",
    };
    uint32_t k = 0;
    int t;

    for (t = 0; t < TAG_COUNT; t++) {
        if (tags & TAG(t)) {
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
        h = fold_fixed(h, p8_fixed_from_int(s->items[i]));
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
    pico8_cart_vider_file_sortie();
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

/* The background is the `bg` of cv.hd whose `hd` and `fa` are both 1. */
static pico8_bg_t *the_bg(void) {
    const pico8_ents_t *hd = pico8_cv("hit_test");
    pico8_bg_t *found = NULL;
    int count = 0;
    int i;

    for (i = 0; hd != NULL && i < hd->count; i++) {
        const entity_t *e = hd->items[i];
        pico8_bg_t *bg;

        if (e->class != pico8_bg_struct_class()) {
            continue;
        }
        bg = (pico8_bg_t *)e;
        if (bg->def->hd == p8_fixed_from_int(1) && bg->def->fa == p8_fixed_from_int(1)) {
            found = bg;
            count++;
        }
    }
    assert(count == 1);
    return found;
}

/* A probe, whatever its form: `g_`'s tags and `fa`=2 (an `fv` has 10). */
static int is_probe(entity_t *e) {
    const pico8_g__t *g_ =
        e != NULL && (e->class->lineage & LINEAGE(LINEAGE_PROBE)) ? (const pico8_g__t *)e : NULL;
    return g_ != NULL && g_->header.class->tags == (TAG(TAG_PROBE) | TAG(TAG_TARGETING)) && g_->def->fa == p8_fixed_from_int(2);
}

static pico8_g__t *as_g_(void *e) {
    assert(e != NULL &&
           ((const entity_t *)e)->class == pico8_g__struct_class());
    return e;
}

/* Road (a): `bg:ldown` while cg.g_ is empty. */
static pico8_g__t *ldown(void) {
    pico8_bg_t *bg = the_bg();
    down_t out;

    assert(ents_list_count(pico8_cg("probe")) == 0);
    out = cls_of(bg)->left_press(&bg->header, vec_ref(vec_xy(1, 1)), vec_ref(vec_xy(2, 2)));
    assert(out.stop && out.held != NULL && is_probe(out.held));
    return as_g_(out.held);
}

/* Road (c): the 4 keys of probe_mm_rnd_order. */
static pico8_g__t *keyed(void) {
    pico8_g__keys_t keys = {0};

    keys.has_pn = 1;
    keys.pn = p8_fixed_from_int(7);
    keys.t = p8_fixed_from_int(3);
    keys.has_du = 1;
    keys.du = p8_fixed_from_int(10);
    keys.has_py = 1;
    keys.py = p8_fixed_from_int(1);
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
    assert(e->header.map_pos == &e->p);
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
    h = fold_string_member(h, e->header.state);
    h = fold_fixed_member(h, e->header.state_time);
    assert(pr->def->dq != NULL && em->k_ != NULL);
    h = fold_tags(h, e->header.class->tags);
    h = fold_seq(h, em->k_);
    assert(e->header.class->tags == (TAG(TAG_PROBE) | TAG(TAG_TARGETING)));
    assert(em->k_ == &pico8_g__k__seq);
    h = fnv(h, (uint32_t)size_of(em->qy));
    assert(size_of(em->qy) == 0);
    assert(e->ra == NULL && e->p == NULL && pr->et == NULL && !e->p_);
    assert(cls_of(e)->hit_test == NULL && cls_of(e)->new_year == NULL && cls_of(e)->refresh == NULL &&
           cls_of(e)->states[ENTITY_STATE_SETTLED] == NULL && cls_of(e)->states[ENTITY_STATE_NORMAL] == NULL);
    assert(cls_of(e)->drag == pico8_bv_dispatch);
    assert(cls_of(e)->drop == pico8_d__dispatch);
    assert(cls_of(e)->draw_overlay == pico8_dl_dispatch);
    assert(cls_of(e)->draw == pico8_ei_dispatch);
    assert(cls_of(e)->spawn_particle == pico8_g__mm);
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(pico8_cv(bm[i]), e));
    }
    assert(ents_listed(pico8_cv("draw"), e) && ents_listed(pico8_cv("draw_overlay"), e));
    assert(!ents_listed(pico8_cv("hit_test"), e) && !ents_listed(pico8_cv("new_year"), e) && !ents_listed(pico8_cv("refresh"), e));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("probe"), e));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("targeting"), e));
    assert(ents_listed(pico8_cg("probe"), e) && ents_listed(pico8_cg("targeting"), e));
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
    assert(a->def->fa == p8_fixed_from_int(2) && a->emitter->def->ki == p8_fixed_from_int(9));
    assert(a->probe->def->dw == 0x666);
    assert(c->emitter->pn == p8_fixed_from_int(7) && c->header.state_time == p8_fixed_from_int(3) &&
           c->emitter->du == p8_fixed_from_int(10) && c->emitter->py == p8_fixed_from_int(1));
    assert(b->emitter->pn == p8_fixed_from_int(3) && b->header.state_time == p8_fixed_from_int(0));
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
    if (ents_listed(pico8_cg("node"), ref)) {
        return 2;
    }
    if (ref != NULL &&
        ((const entity_t *)ref)->class == pico8_jc_struct_class()) {
        return 3;
    }
    assert(is_bv_target(ref));
    return 4;
}

/* The `fo` of the probe's `et`, a planet, a slipway or the throwaway. */
static const void *et_fo(const pico8_g__t *e) {
    entity_t *et = e->probe->et;

    assert(et != NULL);
    if (pico8_jo_is_family(et)) {
        return ((const pico8_jo_t *)et)->fo;
    }
    if (et->class == pico8_jc_struct_class()) {
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
    entity_t *e;

    assert(ents_list_count(pico8_cg("node")) > 0);
    e = pico8_cg("node")->items[0];
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
    planet_keys_t keys;
    pico8_jc_keys_t jc_keys = {0};
    void *qu;
    void *re;
    pico8_jc_t *jc;
    pico8_vec_t base = planet_p();
    pico8_vec_t qu_p = pico8_vec_add(base, vec_xy(0, 1));
    pico8_vec_t re_p = pico8_vec_add(base, vec_xy(1, 1));

    keys.map_pos = vec_ref(qu_p);
    keys.pos = NULL;
    keys.planet_id = "s91";
    qu = pico8_jo_new(&keys);
    keys.map_pos = vec_ref(re_p);
    keys.planet_id = "s92";
    re = pico8_jo_new(&keys);
    jc_keys.qu = qu;
    jc = pico8_jc_new(&jc_keys);
    jc->re = re;
    *middle = pico8_vec_div(pico8_vec_add(qu_p, re_p), p8_fixed_from_int(2));
    return jc;
}

/* The mouse one pixel off `at` (world units, as `p`): `ij` skips an entity
 * at the very position. */
static pico8_vec_t *mouse_at(pico8_vec_t at) {
    return vec_ref(pico8_vec_add(pico8_vec_mul(at, p8_fixed_from_int(128)), vec_xy(1, 1)));
}

/* `hq` at `at`, the raw mouse at `rp`: `et`'s kind, the target's `fo`,
 * `ra` a copy of what was passed, `p`. */
static uint32_t fold_hq(uint32_t h, pico8_g__t *e, pico8_vec_t at, int rp, uint32_t want_kind) {
    pico8_vec_t *mp = mouse_at(at);
    uint32_t kind;

    cls_of(e)->drag(&e->header, mp, vec_ref(vec_xy(rp, rp)));
    kind = et_kind(e);
    printf("[hq] et kind %u\n", (unsigned)kind);
    fflush(stdout);
    assert(kind == want_kind);
    assert(ra_copies(e->ra, mp));
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
    h = fold_particle(h, cls_of(a)->spawn_particle(&a->header));
    h = fold_particle(h, cls_of(b)->spawn_particle(&b->header));

    h = fold_hq(h, a, planet_p(), 64, 2);
    h = fold_hq(h, a, middle, 64, 3);
    assert((void *)jc->fo == a);
    h = fold_hq(h, a, pico8_vec_add(middle, vec_xy(0, 3)), 64, 1);
    h = fold_hq(h, a, pico8_vec_add(middle, vec_xy(0, 3)), 2, 4);

    pico8_ke_ks(&a->header, "placing");
    pico8_ke_ks(&b->header, "placing");
    cls_of(a)->draw_overlay(&a->header, NULL);
    h = fnv(h, screen_hash());
    cls_of(b)->draw_overlay(&b->header, NULL);
    h = fnv(h, screen_hash());
    cls_of(a)->drag(&a->header, mouse_at(planet_p()), vec_ref(vec_xy(64, 64)));
    cls_of(a)->draw_overlay(&a->header, NULL);
    h = fnv(h, screen_hash());

    for (i = 0; i < 3; i++) {
        pico8_dc("draw");
        h = fnv(h, screen_hash());
        h = fnv(h, (uint32_t)size_of(a->emitter->qy));
        h = fnv(h, (uint32_t)size_of(b->emitter->qy));
    }
    for (i = 0; i < 3; i++) {
        pico8_dg();
        h = fold_fixed_member(h, a->header.state_time);
        h = fold_fixed_member(h, b->header.state_time);
        h = fold_string_member(h, a->header.state);
        assert(ents_listed(pico8_hv(), a) && ents_listed(pico8_hv(), b));
    }
    assert(cls_of(a)->drop == pico8_d__dispatch);
    assert(pico8_d_(&a->header) == 0);
    assert(a->p_);
    pico8_dg();
    assert(!ents_listed(pico8_hv(), a) && !ents_listed(pico8_cg("probe"), a) && !ents_listed(pico8_cg("targeting"), a));
    assert(ents_listed(pico8_hv(), b) && ents_listed(pico8_cg("probe"), b));
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
    cls_of(a)->drag(&a->header, mouse_at(planet_p()), vec_ref(vec_xy(64, 64)));
    assert(cls_of(a)->drop == pico8_d__dispatch);
    out = pico8_d_(&a->header);
    assert(out == 0);
    assert(a->p_);
    h = fold_string_member(h, a->header.state);
    h = fold_fixed_member(h, a->header.state_time);
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
    assert(cls_of(a)->drop == pico8_d__dispatch);
    out = pico8_d_(&a->header);
    assert(out == 1);
    assert(!a->p_);
    assert(is_me(&a->header, "settled"));
    assert(a->header.state_time == p8_fixed_from_int(0));
    h = fold_string_member(h, a->header.state);
    h = fold_fixed_member(h, a->header.state_time);
    h = fold_fixed_member(h, fm_hy()->nz);
    check("ey off", h, EY_OFF_HASH);
}

/* `ey` over the void, off the panel: `et` is the throwaway target, which is
 * truthy, so the drop is refused (`p_`, nil returned) and costs nothing. */
static void test_ey_void(void) {
    pico8_g__t *a;
    pico8_vec_t middle;
    p8_fixed_t nz;
    const char *me;
    int out;

    start_game();
    a = ldown();
    (void)a_slipway(&middle);
    cls_of(a)->drag(&a->header, mouse_at(pico8_vec_add(middle, vec_xy(0, 3))), vec_ref(vec_xy(2, 2)));
    assert(et_kind(a) == 4);
    assert(et_fo(a) == a);
    assert(!a->p_);
    nz = fm_hy()->nz;
    me = a->header.state;
    assert(cls_of(a)->drop == pico8_d__dispatch);
    out = pico8_d_(&a->header);
    assert(out == 0);
    assert(a->p_);
    assert(fm_hy()->nz == nz);
    assert(a->header.state == me);
    assert(et_kind(a) == 4);
    printf("[ey void] refused\n");
}

/* The help: `g_`'s tags and `fa`=10. */
static int is_help(entity_t *e) {
    const pico8_g__t *g_ =
        e != NULL && (e->class->lineage & LINEAGE(LINEAGE_PROBE)) ? (const pico8_g__t *)e : NULL;
    return g_ != NULL && g_->header.class->tags == (TAG(TAG_PROBE) | TAG(TAG_TARGETING)) && g_->def->fa == p8_fixed_from_int(10);
}

/* The title screen's `fv`, of `g_`'s lineage: its
 * tags, `fx`, `dq` inherited, `ev`, `mm` the functions of `g_`. */
static void test_fv(void) {
    uint32_t h = 2166136261u;
    const pico8_ents_t *hv;
    entity_t *found = NULL;
    const pico8_g__t *fv;
    int i;

    p8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    hv = pico8_hv();
    for (i = 0; i < hv->count; i++) {
        entity_t *e = hv->items[i];
        if (is_help(e)) {
            assert(found == NULL);
            found = e;
        }
    }
    assert(found != NULL && !is_probe(found));
    fv = (const pico8_g__t *)found;
    assert(fv->header.class->tags == (TAG(TAG_PROBE) | TAG(TAG_TARGETING)));
    assert(fv->def->fx == p8_fixed_from_int(1));
    assert(fv->probe->def->dq != NULL && strcmp(fv->probe->def->dq, "settled") == 0);
    assert(fv->def->fa == p8_fixed_from_int(10));
    assert(fv->emitter->k_ == &pico8_fv_k__seq);
    assert(fv->emitter->k_ != &pico8_g__k__seq);
    assert(cls_of(fv)->draw_overlay == pico8_dl_dispatch);
    assert(cls_of(fv)->spawn_particle == pico8_g__mm);
    assert(cls_of(fv)->drag == pico8_bv_dispatch);
    assert(cls_of(fv)->drop == pico8_d__dispatch);
    assert(cls_of(fv)->draw != pico8_ei_dispatch);
    assert(cls_of(fv)->draw == pico8_fv_kj_dispatch);
    assert(cls_of(fv)->hit_test != NULL);
    assert(pico8_g__ready());
    assert(size_of(fv->emitter->qy) >= 0);
    h = fnv(h, (uint32_t)size_of(fv->emitter->qy));
    h = fnv(h, (uint32_t)ents_rank_in(hv, fv));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("probe"), fv));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("targeting"), fv));
    assert(ents_listed(pico8_cg("probe"), fv) && ents_listed(pico8_cg("targeting"), fv));
    h = fold_defaults(h, fv);
    for (i = 0; i < 3; i++) {
        pico8_dc("draw");
        h = fnv(h, screen_hash());
        h = fnv(h, (uint32_t)size_of(fv->emitter->qy));
    }
    check("help_screen", h, FV_HASH);
}

static int is_a_g__struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_g__struct_class();
}

/* Built by any road, a probe is a struct whose descriptor holds the 5
 * public functions of `g_`, which its class no longer installs. The title's
 * `fv` is a struct of its own. */
static void test_g__is_a_struct(void) {
    const entity_class_t *cls = pico8_g__struct_class();
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
    assert(cls->drag == pico8_bv_dispatch && cls->drop == pico8_d__dispatch && cls->draw_overlay == pico8_dl_dispatch &&
           cls->draw == pico8_ei_dispatch && cls->spawn_particle == pico8_g__mm);
    assert(cls->attach_slipway == NULL && cls->offer_trade == NULL && cls->draw_at == NULL);
    assert(cls->hit_test == NULL && cls->new_year == NULL && cls->refresh == NULL && cls->has_refresh == NULL && cls->left_press == NULL &&
           cls->right_press == NULL && cls->accept_drop == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
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
        entity_t *e = hv->items[j];
        if (is_help(e)) {
            break;
        }
    }
    assert(j < hv->count);
    assert(hv->items[j] != NULL &&
           ((const entity_t *)hv->items[j])->class == pico8_fv_struct_class());
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
    cls_of(a)->drag(&a->header, mouse_at(pico8_vec_add(middle, vec_xy(0, 3))), vec_ref(vec_xy(2, 2)));
    assert(a->probe->et != NULL);
    et = a->probe->et;
    assert(is_bv_target(et) && bv_et_fo(et) == a);
    assert(cls_of(et)->draw_overlay == NULL);
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

/* A probe that leaves (`he`, then the exit queue's flush) gives back its
 * particles and its `qy`, and keeps its struct. */
static void test_leaving_gives_back_its_particles(void) {
    held_t before;
    held_t after;
    pico8_g__t *a;

    start_game();
    before = held_now();
    a = ldown();
    pico8_ez(&a->header, p8_fixed_from_int(5));
    assert(a->emitter->qy->count == 5);
    assert(held_now().particules == before.particules + 5 && held_now().qy == before.qy + 1);
    pico8_he(&a->header);
    pico8_cart_vider_file_sortie();
    after = held_now();
    assert(after.particules == before.particules && after.qy == before.qy);
    assert(a->emitter->qy == NULL);
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
    in_fresh_child(test_ey_on);
    in_fresh_child(test_ey_off);
    in_fresh_child(test_ey_void);
    in_fresh_child(test_fv);
    in_fresh_child(test_g__is_a_struct);
    in_fresh_child(test_bv_et_is_a_struct);
    in_fresh_child(test_leaving_gives_back_its_particles);

    printf("test_pico8_g_: all tests passed\n");
    return 0;
}
