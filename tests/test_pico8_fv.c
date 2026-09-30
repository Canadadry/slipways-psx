/* The help screen `fv`, built by its only road (`pico8__init()` on the
 * title screen, `pico8_fv_new()`), read only through the dual-form field
 * API, its defaults read on its archetype, and driven through the
 * real `mv`/`dc`/`dg`/`lj`. Its
 * methods are compared to the public functions. No events file. */

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
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_entity_geom.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_geom.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_particle.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_vec.h"


#define BUILT_HASH 0x0B80F384u
#define LIFE_HASH 0x3BFADEC0u
#define HOVER_HASH 0xEE114A89u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

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

/* 0 absent or false, 2 true. */
static uint32_t m__state(const pico8_fv_t *fv) {
    return fv->m_ ? 2 : 0;
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

/* A generated row: items (a nested row folded the same way), then `c`,
 * then `fn`, then the entry count. */
static uint32_t fold_row(uint32_t h, const pico8_jb_row_t *row) {
    int i;

    for (i = 0; i < row->count; i++) {
        const pico8_jb_item_t *item = &row->items[i];

        h = fnv(h, (uint32_t)(i + 1));
        if (item->row != NULL) {
            h = fold_row(fnv(h, 0x7Au), item->row);
        } else if (item->s != NULL) {
            h = fold_string(h, item->s);
        } else if (item->is_false) {
            h = fold_bool(h, 0);
        } else {
            h = fold_fixed(h, item->is_fixed ? item->fixed : pico8_fixed_from_int(item->n));
        }
    }
    if (row->has_c) {
        h = fold_fixed(fnv_str(h, "c"), pico8_fixed_from_int(row->c));
    }
    if (row->fn != NULL) {
        h = fold_string(fnv_str(h, "fn"), row->fn);
    }
    return fnv(h, (uint32_t)(row->count + (row->has_c != 0) + (row->fn != NULL)));
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (ents_listed(pico8_hv(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)ents_rank_in(pico8_hv(), ref));
    }
    if (ref == (const void *)&pico8_fv_k__seq) {
        return fold_seq(fnv(h, 0x7Au), ref);
    }
    if (ref == (const void *)&pico8_fv_rb_row) {
        return fold_row(fnv(h, 0x7Au), ref);
    }
    return fnv(fnv(h, 0xEEEEu), 0u);
}

/* A member the reader always answers: its presence bit and kind byte are
 * constant by the struct's shape, kept so that BUILT_HASH and LIFE_HASH
 * still prove the typed reads. */
static uint32_t fold_fixed_member(uint32_t h, pico8_fixed_t x) {
    return fnv(fnv(fnv(h, 1u), 0xF1u), (uint32_t)x);
}

static uint32_t fold_bool_member(uint32_t h, int b) {
    return fnv(fnv(fnv(h, 1u), 0xB0u), (uint32_t)b);
}

static uint32_t fold_string_member(uint32_t h, const char *s) {
    return s == NULL ? fnv(h, 0u) : fnv_str(fnv(fnv(h, 1u), 0x57u), s);
}

static uint32_t fold_ref_member(uint32_t h, const void *ref) {
    return ref == NULL ? fnv(h, 0u) : fold_ref(fnv(h, 1u), ref);
}

static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    return fnv(fnv(h, (uint32_t)p->x), (uint32_t)p->y);
}

static uint32_t fold_box(uint32_t h, const pico8_qz_box_t *b) {
    assert(b != NULL);
    h = fnv(fnv(h, (uint32_t)b->xl), (uint32_t)b->yt);
    return fnv(fnv(h, (uint32_t)b->xr), (uint32_t)b->yb);
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

static pico8_vec_t *vec_ref(int x, int y) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
    return v;
}

static const pico8_struct_class_t *cls_of(const void *e) {
    return ((const pico8_struct_header_t *)e)->cls;
}

static void start_title(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
}

/* The help, whatever its form: `g_`'s tags and `fa`=10 (a probe has 2). */
static int is_help(pico8_struct_header_t *e) {
    const pico8_g__t *g_ = e != NULL ? pico8_g__of(e) : NULL;
    return g_ != NULL && g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)) && g_->def->fa == pico8_fixed_from_int(10);
}

static pico8_fv_t *the_help(void) {
    const pico8_ents_t *hv = pico8_hv();
    pico8_struct_header_t *found = NULL;
    int i;

    for (i = 0; i < hv->count; i++) {
        if (is_help(hv->items[i])) {
            assert(found == NULL);
            found = hv->items[i];
        }
    }
    assert(found != NULL && found->cls == pico8_fv_struct_class());
    return (pico8_fv_t *)found;
}

static int in_layer(int layer, const void *e) {
    const pico8_ui_ic_t *ic = pico8_ok()->ic;

    if (ic == NULL) {
        return 0;
    }
    return ents_listed(&ic->layers[layer], e);
}

static const pico8_js_t *as_js(const void *e) {
    assert(e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_js_struct_class());
    return e;
}

/* The title's help as `fv:pq`, `f_`, `lt` left it: its 22 defaults and the
 * 6 of g_, their tables' contents and identity, `me`, `t`, its empty `qy`;
 * no name only read or written elsewhere; its methods; the 4 buttons `pz`
 * built before it, each carrying the `my` stamped on its `bo` row; where
 * `lt` registered it; `ok.le` emptied. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    const pico8_ents_t *hv;
    pico8_fv_t *fv;
    const pico8_g__t *g_;
    const pico8_emitter_t *em;
    const pico8_probe_t *pr;
    int rank;
    size_t i;
    int k;

    start_title();
    fv = the_help();
    g_ = &fv->g_;
    em = g_->emitter;
    pr = g_->probe;
    hv = pico8_hv();

    assert(g_->def->fa == pico8_fixed_from_int(10));
    assert(em->def->ki == pico8_fixed_from_int(17));
    assert(g_->def->ca == pico8_fixed_from_int(11));
    assert(g_->header.cls->tags == (PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER)));
    assert(g_->ra != NULL && em->k_ != NULL);
    assert(g_->def->rb != NULL && pr->def->dq != NULL);
    h = fold_fixed_member(h, g_->def->fa);
    h = fold_ref_member(h, g_->ra);
    h = fold_ref_member(h, &g_->def->hi);
    h = fold_fixed_member(h, g_->def->ca);
    h = fold_ref_member(h, &g_->def->gq);
    h = fold_fixed_member(h, g_->def->eo);
    h = fold_fixed_member(h, g_->def->ff);
    h = fold_fixed_member(h, em->def->fc);
    h = fold_ref_member(h, em->k_);
    h = fold_fixed_member(h, em->qo);
    h = fold_fixed_member(h, em->ro);
    h = fold_fixed_member(h, em->qg);
    h = fold_fixed_member(h, em->q_);
    h = fold_fixed_member(h, em->pn);
    h = fold_fixed_member(h, em->du);
    h = fold_fixed_member(h, em->py);
    h = fold_fixed_member(h, em->def->pk);
    h = fold_fixed_member(h, em->def->ki);
    h = fold_ref_member(h, &g_->def->kq);
    h = fold_fixed_member(h, g_->def->ql);
    h = fold_fixed_member(h, g_->def->fg);
    h = fold_ref_member(h, g_->def->rb);
    h = fold_tags(fnv(fnv(h, 1u), 0x7Au), g_->header.cls->tags);
    h = fold_fixed_member(h, g_->def->fx);
    h = fold_string_member(h, pr->def->dq);
    h = fold_fixed_member(h, pr->od);
    h = fold_bool_member(h, pr->def->fp != 0);
    h = fold_fixed_member(h, pr->def->dw);
    h = fold_vec(h, g_->ra);
    h = fold_vec(h, &g_->def->hi);
    h = fold_vec(h, &g_->def->gq);
    h = fold_box(h, &g_->def->kq);
    assert(g_->def == &pico8_fv_def && g_->ra == &pico8_fv_def.ra);
    assert(em->k_ == &pico8_fv_k__seq);
    assert(em->k_ != &pico8_g__k__seq);
    assert(g_->def->rb == &pico8_fv_rb_row);
    h = fold_string_member(h, g_->header.me);
    h = fold_fixed_member(h, g_->header.t);
    h = fnv(h, (uint32_t)size_of(em->qy));
    assert(size_of(em->qy) == 0);
    assert(!fv->m_ && g_->p == NULL && pr->et == NULL && !g_->p_);
    assert(cls_of(fv)->hk == NULL && cls_of(fv)->hb == NULL && cls_of(fv)->states[PICO8_STATE_PH] == NULL &&
           cls_of(fv)->ldown == NULL);

    assert(cls_of(fv)->kj == pico8_fv_kj_dispatch);
    assert(cls_of(fv)->ev == pico8_dl_dispatch);
    assert(cls_of(fv)->mm == pico8_g__mm);
    assert(cls_of(fv)->hq == pico8_bv_dispatch);
    assert(cls_of(fv)->ey == pico8_d__dispatch);
    assert(cls_of(fv)->hd != NULL);

    rank = ents_rank_in(hv, fv);
    h = fnv(h, (uint32_t)rank);
    h = fnv(h, (uint32_t)hv->count);
    assert(rank >= 4);
    for (k = 0; k < 4; k++) {
        const pico8_js_t *b = as_js(hv->items[rank - 4 + k]);
        pico8_vec_t qf;
        assert(b->fa == pico8_fixed_from_int(11));
        assert(b->qf != NULL);
        qf = *b->qf;
        assert(qf.x == pico8_fixed_from_int(34) && qf.y == pico8_fixed_from_int(56 + 11 * k));
        h = fold_vec(h, b->qf);
        h = fold_vec(h, b->ra);
        h = fold_fixed_member(h, b->fa);
    }
    for (i = 0; i < 5; i++) {
        static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
        h = fnv(h, (uint32_t)ents_rank_in(pico8_cv(bm[i]), fv));
    }
    assert(ents_listed(pico8_cv("kj"), fv) && ents_listed(pico8_cv("ev"), fv) && ents_listed(pico8_cv("hd"), fv));
    assert(!ents_listed(pico8_cv("hk"), fv) && !ents_listed(pico8_cv("hb"), fv));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("g_"), fv));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cg("prober"), fv));
    assert(ents_listed(pico8_cg("g_"), fv) && ents_listed(pico8_cg("prober"), fv));

    for (k = 0; k < 4; k++) {
        const pico8_js_t *row = (const pico8_js_t *)hv->items[rank - 4 + k];
        assert(row->has_my);
        assert(row->my == pico8_eu_field_value());
    }
    assert(pico8_ok()->le != NULL && pico8_ok()->le->count == 0);
    check("built", h, BUILT_HASH);
}

/* `dc("kj")` x3 with the screen and `qy`; `dc("ev")`; `dg` x3; `hd` in and
 * out of `kq`, `rp` retained since `fa` >= 10. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    pico8_fv_t *fv;
    int i;

    start_title();
    fv = the_help();
    for (i = 0; i < 3; i++) {
        pico8_dc("kj");
        h = fnv(h, screen_hash());
        h = fnv(h, (uint32_t)size_of(fv->g_.emitter->qy));
    }
    pico8_dc("ev");
    h = fnv(h, screen_hash());
    for (i = 0; i < 3; i++) {
        pico8_dg();
        h = fold_fixed_member(h, fv->g_.header.t);
        h = fold_string_member(h, fv->g_.header.me);
        assert(ents_listed(pico8_hv(), fv));
    }
    assert(cls_of(fv)->hd(fv, vec_ref(64, 10), vec_ref(64, 112)) == 1);
    assert(cls_of(fv)->hd(fv, vec_ref(64, 112), vec_ref(64, 10)) == 0);
    assert(cls_of(fv)->hd(fv, vec_ref(24, 106), vec_ref(24, 106)) == 1);
    assert(cls_of(fv)->hd(fv, vec_ref(23, 106), vec_ref(23, 106)) == 0);
    h = fnv(h, (uint32_t)size_of(fv->g_.emitter->qy));
    check("life", h, LIFE_HASH);
}

static pico8_input_frame_t g_mouse;

static void mouse_at(int x, int y) {
    memset(&g_mouse, 0, sizeof g_mouse);
    g_mouse.mouse_x = x;
    g_mouse.mouse_y = y;
    input.frames = &g_mouse;
    input.frame_count = 1;
    input.current_frame = 0;
    pico8_dispatch_set_input(&input);
    pico8_dg();
}

/* The mouse over "need help?": `ok.o_`, `m_`, `fg` poked at 0x5f80, the
 * help in layer `fa` of `ic`, the cursor sprite `ql`; then out, `m_` false. */
static void test_hover(void) {
    uint32_t h = 2166136261u;
    pico8_fv_t *fv;
    uint32_t drawn;

    start_title();
    fv = the_help();
    mouse_at(64, 112);
    pico8_ui_lj(&mem, pico8_ok());
    assert((void *)pico8_ok()->o_ == fv);
    assert(m__state(fv) == 2);
    check_int("hover 0x5f80", pico8_peek(&mem, 0x5f80), 5);
    assert(in_layer(10, fv));
    h = fnv(h, m__state(fv));
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));

    pico8_cls(&mem);
    pico8_ui_ev(&gfx, &mem, pico8_ok());
    drawn = screen_hash();
    pico8_cls(&mem);
    pico8_spr_default(&gfx, &mem, 11, 64, 112);
    assert(drawn == screen_hash());
    h = fnv(h, drawn);

    mouse_at(64, 10);
    pico8_ui_lj(&mem, pico8_ok());
    assert((void *)pico8_ok()->o_ != fv);
    assert(m__state(fv) == 0);
    assert(!in_layer(10, fv));
    h = fnv(h, m__state(fv));
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    check("hover", h, HOVER_HASH);
}

/* `eu`, as a difficulty button starts it, takes the help out of `hv`,
 * `cg.g_` and `cv.kj`/`cv.hd`. */
/* A particle of the help is drawn in the help's own `k_`: at `l`=6.5, less
 * the frame's 0.14, index 6 is colour 9, where `g_`'s five have none. */
static void test_particle_takes_the_help_palette(void) {
    static pico8_qy_t particle;
    pico8_ptrs_t *qy;
    pico8_vec_t ra;
    pico8_fv_t *fv;
    int at;

    start_title();
    fv = the_help();
    assert(fv->g_.ra != NULL);
    ra = *fv->g_.ra;
    particle.p = pico8_vec_v(0, 0);
    particle.v = pico8_vec_v(0, 0);
    particle.has_a = 0;
    particle.l = PICO8_FIXED_LIT(0x00068000, 6.5);
    qy = fv->g_.emitter->qy;
    qy->count = 0;
    pico8_ptrs_add(qy, &particle);
    pico8_camera_reset(&gfx);
    at = PICO8_ADDR_SCREEN + pico8_fixed_flr_int(ra.y) * PICO8_SCREEN_WIDTH + pico8_fixed_flr_int(ra.x);
    pico8_poke(&mem, at, 3);
    pico8_ei(&gfx, &mem, &fv->g_.header);
    assert(pico8_peek(&mem, at) == pico8_fv_k__seq.items[5] && pico8_fv_k__seq.items[5] == 9);
}

static void test_eu_retires_the_help(void) {
    void *fv;

    start_title();
    fv = the_help();
    pico8_eu(NULL);
    assert(!ents_listed(pico8_hv(), fv));
    assert(!ents_listed(pico8_cg("g_"), fv) && !ents_listed(pico8_cv("kj"), fv) && !ents_listed(pico8_cv("hd"), fv));
}

/* The title's help is a struct whose descriptor shares `g_`'s 4 inherited
 * entries and holds its own `kj`, `hd`. Ready from its init to its
 * shutdown. */
static void test_fv_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_fv_struct_class();
    const pico8_struct_class_t *g_ = pico8_g__struct_class();
    void *fv;
    int j;

    start_title();
    fv = the_help();
    assert(fv != NULL && ((const pico8_struct_header_t *)fv)->cls == cls);
    assert(cls->hq != NULL && cls->hq == g_->hq && cls->ey != NULL && cls->ey == g_->ey && cls->mm != NULL &&
           cls->mm == g_->mm);
    assert(cls->kf == NULL && cls->cn == NULL && cls->ol == NULL);
    assert(cls->ev != NULL && cls->ev == g_->ev);
    assert(cls->kj == pico8_fv_kj_dispatch && cls->hd != NULL);
    assert(cls->hk == NULL && cls->hb == NULL && cls->has_hb == NULL && cls->ldown == NULL && cls->rdown == NULL &&
           cls->gv == NULL);
    for (j = 0; j < PICO8_STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(pico8_fv_ready());

    pico8_help_shutdown();
    assert(!pico8_fv_ready());
    pico8_help_init();
    assert(pico8_fv_ready());
}

static void call_f_(void *e) {
    pico8_f_(e);
}

static void call_ez(void *e) {
    pico8_ez(e, pico8_fixed_from_int(1));
}

static void call_ei(void *e) {
    pico8_ei(&gfx, &mem, e);
}

static void call_g__mm(void *e) {
    (void)pico8_g__mm(e);
}

static void call_bv(void *e) {
    pico8_bv(e, vec_ref(1, 1), vec_ref(2, 2));
}

static void call_dl(void *e) {
    pico8_dl(&gfx, &mem, e);
}

static void call_d_(void *e) {
    (void)pico8_d_(e);
}

static void call_gk(void *e) {
    (void)pico8_gk(e, vec_ref(1, 1), vec_ref(2, 2));
}

static void call_na(void *e) {
    pico8_na(e, pico8_fixed_from_int(1));
}

/* On NULL `self`: `d_`, `gk` return 0; `g__mm` draws three `rnd`, divides by
 * `pn` = 0 and returns a new particle; the six others do nothing. */
static void test_g__closed(void) {
    static void (*const fns[])(void *) = {call_f_, call_ez, call_ei, call_g__mm, call_bv,
                                          call_dl, call_d_, call_gk, call_na};
    size_t i;

    start_title();
    assert(pico8_g__ready());
    for (i = 0; i < sizeof fns / sizeof fns[0]; i++) {
        fns[i](NULL);
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
    in_fresh_child(test_hover);
    in_fresh_child(test_particle_takes_the_help_palette);
    in_fresh_child(test_eu_retires_the_help);
    in_fresh_child(test_fv_is_a_struct);
    in_fresh_child(test_g__closed);

    printf("test_pico8_fv: all tests passed\n");
    return 0;
}
