/* The button `js`, read and written on its members and driven through the
 * real `pz`/`eu`/`cr`/`ix`/`dg`/`dc`/`mv`, on title screens and games built
 * directly: no events file. */

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
#include "../src/pico8_economy_score.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_fm.h"
#include "../src/pico8_geom.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_menu.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_research.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_tech_gate.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_vec.h"

/* `gb[12]`, a colony that needs two resources and exports one; `gb[3]`, a
 * project with an `o` and no `i`, which exports to any planet a slipway
 * joins it to. */
#define COLONY 12
#define EXPORTER 3

/* The tag class instances carried, which the frozen hashes fold: any
 * other value moves every *_HASH. */
#define FROZEN_INSTANCE_TAG 0x9E5C1451u

/* The tag entity structs carried, which the frozen hashes fold: any other
 * value moves every *_HASH. */
#define FROZEN_STRUCT_TAG 0x9E5C57C7u

#define TITLE_HASH 0xB044B5F2u
#define EU_HASH 0x08EB17A6u
#define PAUSE_HASH 0xF715AAC0u
#define RESEARCH_HASH 0x48094C1Eu
#define FE_HASH 0xF6B832D5u
#define GH_HASH 0x67CA51E5u
#define PJ_HASH 0x0C7D2BEDu
#define DK_HASH 0x5178903Bu
#define PH_HASH 0xD2A611E7u
#define LDOWN_HASH 0x4621266Fu
#define HB_HASH 0x0FA6ACC7u
#define FE_MY_HASH 0x459139D2u
#define CR_MY_HASH 0x7ED641D9u
#define DRAW_HASH 0x92E72EEBu
#define JS_MEMBERS_HASH 0x264E8964u

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

static const pico8_struct_class_t *cls_of(const void *e) {
    return ((const pico8_struct_header_t *)e)->cls;
}

static int is_js(const void *e) {
    return e != NULL && cls_of(e) == pico8_js_struct_class();
}

static pico8_vec_t vec_of(const pico8_vec_t *v) {
    assert(v != NULL);
    return *v;
}

static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

/* A function by the order it was first seen in the scenario: its address
 * is not stable from one build to the next. */
static pico8_js_fn seen_fns[64];
static int seen_count;

static uint32_t fn_index(pico8_js_fn fn) {
    int i;
    for (i = 0; i < seen_count; i++) {
        if (seen_fns[i] == fn) {
            return (uint32_t)i;
        }
    }
    assert(seen_count < 64);
    seen_fns[seen_count] = fn;
    return (uint32_t)seen_count++;
}

static pico8_js_t *js_of(const void *e) {
    assert(cls_of(e) == pico8_js_struct_class());
    return (pico8_js_t *)e;
}

/* 0 absent or false, 2 true. */
static uint32_t m__state(const void *e) {
    const pico8_js_t *b = js_of(e);
    return b->m_ ? 2u : 0u;
}

/* 0 absent, 1 false, 3 a row. */
static uint32_t h__state(const void *e) {
    const pico8_js_t *b = js_of(e);
    if (!b->has_h_) {
        return 0;
    }
    return b->h_ == NULL ? 1u : 3u;
}

/* 0 absent, 1 false, 3 a function. */
static uint32_t my_state(const void *e) {
    const pico8_js_t *b = js_of(e);
    if (!b->has_my) {
        return 0;
    }
    return b->my == NULL ? 1u : 3u;
}

/* A research project folds by the tag of the class-instance form these
 * values were frozen on, whatever its form now. */
static int in_gb(const void *e) {
    int k;
    for (k = 1; k <= pico8_gb_count(); k++) {
        if (pico8_gb_at(k) == e) {
            return 1;
        }
    }
    return 0;
}

static int is_lq(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_lq_struct_class();
}

static int row_count(const void *lq) {
    return ((const pico8_lq_t *)lq)->items_count;
}

static pico8_lq_cell_t row_item(const void *lq, int i) {
    assert(i >= 0 && i < row_count(lq));
    return ((const pico8_lq_t *)lq)->items[i];
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (ents_listed(pico8_hv(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)ents_rank_in(pico8_hv(), ref));
    }
    if (in_gb(ref)) {
        return fnv(fnv(h, 0xEEEEu), FROZEN_INSTANCE_TAG);
    }
    return fnv(fnv(h, 0xEEEEu), FROZEN_STRUCT_TAG);
}

/* The words the frozen hashes fold before a fixed, a boolean and a string: the 0xF1, 0xB0
 * and 0x57 of a member of one kind are constant by the struct's shape,
 * kept so that the hashes still prove the typed reads. */
static uint32_t fold_fixed(uint32_t h, pico8_fixed_t f) {
    return fnv(fnv(h, 0xF1u), (uint32_t)f);
}

static uint32_t fold_bool(uint32_t h, int b) {
    return fnv(fnv(h, 0xB0u), (uint32_t)b);
}

static uint32_t fold_string(uint32_t h, const char *s) {
    return fnv_str(fnv(h, 0x57u), s);
}

/* An absent member folds as the nil the name reader gave. */
static uint32_t fold_fixed_or_nil(uint32_t h, int has, pico8_fixed_t f) {
    return has ? fold_fixed(h, f) : fnv(h, 0xDEADu);
}

static uint32_t fold_string_or_nil(uint32_t h, const char *s) {
    return s != NULL ? fold_string(h, s) : fnv(h, 0xDEADu);
}

/* A member read as present (bit 1) or absent (bit 0), then its value. */
static uint32_t fold_opt_fixed(uint32_t h, int has, pico8_fixed_t f) {
    return has ? fold_fixed(fnv(h, 1u), f) : fnv(h, 0u);
}

static uint32_t fold_opt_string(uint32_t h, const char *s) {
    return s != NULL ? fold_string(fnv(h, 1u), s) : fnv(h, 0u);
}

static uint32_t fold_opt_ref(uint32_t h, const void *ref) {
    return ref != NULL ? fold_ref(fnv(h, 1u), ref) : fnv(h, 0u);
}

/* A positional as the value it was sorted from folded: a row as any REF,
 * NOTHING as false (no row folded here holds a nil at the top level). */
static uint32_t fold_cell(uint32_t h, pico8_lq_cell_t c) {
    switch (c.kind) {
    case PICO8_LQ_CELL_ROW:
        return fnv(h, 0x12u);
    case PICO8_LQ_CELL_FIXED:
        return fold_fixed(h, c.as.fixed);
    case PICO8_LQ_CELL_STRING:
        return fold_string(h, c.as.s);
    default:
        return fold_bool(h, c.kind == PICO8_LQ_CELL_TRUE);
    }
}

/* A row by its width and its positional items. */
static uint32_t fold_lq(uint32_t h, const pico8_lq_t *lq) {
    int i;

    assert(is_lq(lq));
    h = fold_fixed(h, lq->w);
    for (i = 0; i < row_count(lq); i++) {
        h = fnv(h, (uint32_t)(i + 1));
        h = fold_cell(h, row_item(lq, i));
    }
    return h;
}

/* Absent, `false`, or what it holds: a row, a `jb` struct. A row folds by
 * the tag of the class-instance form these values were frozen on. */
static uint32_t fold_built(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xA0u);
    }
    if (is_lq(ref)) {
        return fold_lq(fnv(fnv(h, 0xA2u), FROZEN_INSTANCE_TAG), ref);
    }
    h = fnv(fnv(h, 0xA2u), FROZEN_STRUCT_TAG);
    return h;
}

static uint32_t fold_built_h_(uint32_t h, const pico8_js_t *b) {
    if (b->has_h_ && b->h_ == NULL) {
        return fold_bool(fnv(h, 0xA1u), 0);
    }
    return fold_built(h, b->has_h_ ? b->h_ : NULL);
}

static uint32_t fold_kq(uint32_t h, const pico8_js_t *b) {
    static const int dy[] = {-2, -1, 0, 5, 9, 10, 11};
    const pico8_qz_box_t *kq = b->header.kq;
    int mb = pico8_fixed_flr_int(b->mb);
    int dx[6];
    size_t i;
    size_t j;

    assert(kq != NULL);
    dx[0] = -1;
    dx[1] = 0;
    dx[2] = 1;
    dx[3] = mb - 1;
    dx[4] = mb;
    dx[5] = mb + 1;
    for (i = 0; i < 6; i++) {
        for (j = 0; j < sizeof dy / sizeof dy[0]; j++) {
            pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(dx[i]), pico8_fixed_from_int(dy[j]));
            h = fnv(h, (uint32_t)pico8_qz_call(kq, p));
        }
    }
    return h;
}

static const pico8_ents_t *ok_le(void) {
    return pico8_ok() == NULL ? NULL : pico8_ok()->le;
}

/* A button as `js(b)` and `js:pq` left it: every construction key, what
 * `pq` built, the methods and where `lt` registered it. */
static uint32_t fold_button(uint32_t h, const void *e) {
    static const char *const bm[] = {"kj", "ev", "hd", "hb", "hk"};
    const pico8_js_t *b = js_of(e);
    size_t i;

    assert(b->header.me != NULL && strcmp(b->header.me, "ph") == 0);
    h = fold_fixed(h, b->header.t);
    h = fold_fixed(h, b->fa);
    h = fold_fixed(h, b->mb);
    h = fold_kq(h, b);
    h = fold_vec(h, b->ra);
    h = fold_vec(h, b->qf);
    h = fold_built(h, b->lq);
    h = fold_built_h_(h, b);
    h = fold_built(h, b->jb);
    if (!js_of(b)->has_my) {
        h = fnv(h, 0x34u);
    } else if (js_of(b)->my == NULL) {
        h = fnv(fnv(fnv(h, 0x33u), 0xB0u), 0u);
    } else {
        h = fnv(fnv(fnv(h, 0x33u), 0xFCu), fn_index(js_of(b)->my));
    }
    if (js_of(b)->hb != NULL) {
        h = fnv(fnv(fnv(h, 1u), 0xFCu), fn_index(js_of(b)->hb));
    } else {
        h = fnv(h, 0u);
    }
    h = fold_opt_fixed(h, b->has_fg, b->fg);
    h = fold_opt_string(h, b->px);
    h = fold_opt_fixed(h, b->has_trade, b->trade);
    h = fold_opt_fixed(h, b->has_en, b->en);
    h = fold_opt_ref(h, b->_jo_fe_pt);
    h = fold_opt_ref(h, b->_jo_fe_b);
    h = fold_opt_fixed(h, b->has__jo_fe_od, b->_jo_fe_od);
    if (b->_cr_t != NULL) {
        const pico8_nb_entry_t *t = b->_cr_t;
        assert(t == pico8_nb_lookup(t->px));
        h = fold_fixed(fnv(h, 1u), pico8_fixed_from_int(t->nk));
    } else {
        h = fnv(h, 0u);
    }
    h = fnv(h, m__state(b));
    assert(cls_of(b)->states[PICO8_STATE_PH] != NULL && cls_of(b)->kj != NULL && cls_of(b)->ev != NULL &&
           cls_of(b)->ldown != NULL && cls_of(b)->hd != NULL);
    assert(cls_of(b)->rdown == NULL && cls_of(b)->gv == NULL && cls_of(b)->hk == NULL);
    h = fnv(h, (uint32_t)ents_listed(pico8_hv(), b));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)ents_listed(pico8_cv(bm[i]), b));
    }
    return fnv(h, (uint32_t)ents_rank_in(ok_le(), b));
}

static uint32_t fold_le(uint32_t h, const char *label) {
    const pico8_ents_t *le = ok_le();
    int i;

    assert(le != NULL);
    printf("[%s] %d button(s) in ok.le\n", label, le->count);
    h = fnv(h, (uint32_t)le->count);
    for (i = 0; i < le->count; i++) {
        void *b = le->items[i];
        printf("[%s] button %d: my %u, h_ %u, jb %u\n", label, i, my_state(b), h__state(b),
               js_of(b)->jb != NULL ? 3u : 0u);
        h = fold_button(h, b);
    }
    return h;
}

/* The `ra` every row of one `pz` call shares. */
static void check_shared_ra(void) {
    const pico8_ents_t *le = ok_le();
    int i;
    for (i = 1; le != NULL && i < le->count; i++) {
        assert(js_of(le->items[i])->ra == js_of(le->items[0])->ra);
    }
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

/* The `i`th (from 1) of the title's four buttons, which `pz` built from
 * `bo`'s rows in order: the entities of `cv.hd` with an `mb`. */
static void *title_button(int i) {
    const pico8_ents_t *hd = pico8_cv("hd");
    int k;

    for (k = 0; hd != NULL && k < hd->count; k++) {
        void *e = hd->items[k];
        if (is_js(e) && --i == 0) {
            return e;
        }
    }
    assert(0 && "no such title button");
    return NULL;
}

static pico8_jo_t *as_jo(void *e) {
    assert(pico8_jo_is_family(e));
    return e;
}

static pico8_vec_t pos_of(void *e) {
    return vec_of(as_jo(e)->p);
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
 * hovered planet off `ok.o_`), then left to settle; returns the colony. */
static void *built_world(void) {
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
    pico8_jc_struct_class()->hq(jc, mp, NULL);
    pico8_ok()->o_ = NULL;
    while (jc->header.t < pico8_fixed_from_int(15)) {
        pico8_dg();
    }
    pico8_jc_struct_class()->ey(jc, re);
    for (i = 0; i < 120; i++) {
        pico8_dg();
    }
    return qu;
}

static void one_frame(void) {
    pico8__update60();
    pico8__draw();
}

/* A generated planet discovered by `jo:hh` and clicked: `jo:fe`'s panel. */
static void known_panel(void) {
    void *p;

    start_game();
    p = pico8_cg("jo")->items[0];
    pico8_jo_hh(p);
    (void)pico8_jo_ldown(p);
    one_frame();
}

/* built_world's colony with its panel, `jo:gh`, up. */
static void built_panel(void) {
    pico8_jo_gh(built_world());
    one_frame();
}

/* The research screen, `cr`, opened on a started game holding 5 `jk`:
 * some techs affordable, some too dear. */
static void tech_screen(void) {
    start_game();
    pico8_fm_hy()->jk = pico8_fixed_from_int(5);
    pico8_cr();
    one_frame();
}

/* The game's end: `pj:ph` once `hy.hl` is "final score". */
static void game_end(void) {
    pico8_struct_header_t *ju;

    start_game();
    pico8_fm_hy()->hl = "final score";
    pico8_fm_hy()->has_hl = 1;
    ju = (pico8_struct_header_t *)pico8_ju();
    assert(ju->cls->states[PICO8_STATE_PH] != NULL);
    ju->cls->states[PICO8_STATE_PH](ju, pico8_fixed_from_int(0));
    one_frame();
}

/* The button's `my` callback, called as a click calls it. */
static void call_my(void *e) {
    pico8_js_t *b = e;

    assert(b->my != NULL);
    b->my(b);
}

/* The buttons `eu`'s `nv(ix,js)` built: the only `fa`=11 entities with an
 * `lq` in `cv.hd` right after `eu`. */
static int ix_buttons(void *out[4]) {
    const pico8_ents_t *hd = pico8_cv("hd");
    int n = 0;
    int i;
    for (i = 0; hd != NULL && i < hd->count; i++) {
        void *e = hd->items[i];
        if (is_js(e) && js_of(e)->fa == pico8_fixed_from_int(11) && js_of(e)->lq != NULL) {
            assert(n < 4);
            out[n++] = e;
        }
    }
    return n;
}

/* `fv`'s `ff` empties `ok.le` right after `pz`: its buttons are the
 * entities of `cv.hd` with an `mb`, which only a `js` has. */
static void test_title(void) {
    uint32_t h = 2166136261u;
    const pico8_ents_t *hd;
    void *first = NULL;
    int n = 0;
    int i;

    start_title();
    hd = pico8_cv("hd");
    for (i = 0; i < hd->count; i++) {
        void *e = hd->items[i];
        if (is_js(e)) {
            if (first == NULL) {
                first = e;
            }
            assert(js_of(e)->ra == js_of(first)->ra);
            h = fold_button(h, e);
            n++;
        }
    }
    check_int("title buttons", n, 4);
    check("title", h, TITLE_HASH);
}

static void test_eu(void) {
    uint32_t h = 2166136261u;
    void *ix[4];
    int i;

    start_game();
    check_int("eu buttons", ix_buttons(ix), 4);
    for (i = 0; i < 4; i++) {
        assert(!ents_listed(ok_le(), ix[i]));
        assert((js_of(ix[i])->hb != NULL) == (i == 1));
        h = fold_button(h, ix[i]);
    }
    check("eu", h, EU_HASH);
}

static void test_pause(void) {
    uint32_t h = 2166136261u;
    void *ix[4];

    start_game();
    assert(ix_buttons(ix) == 4);
    call_my(ix[3]);
    check_shared_ra();
    h = fold_le(h, "pause");
    check("pause", h, PAUSE_HASH);
}

static void invent(char *rr) {
    assert(pico8_fm_hy() != NULL && pico8_fm_hy()->invented != NULL);
    pico8_ptrs_add(pico8_fm_hy()->invented, rr);
}

static void test_research(void) {
    uint32_t h = 2166136261u;
    const pico8_ents_t *le;
    const pico8_jb_t *jb;
    const pico8_lq_t *line;
    int unaffordable = 0;
    int i;

    start_game();
    invent("slipgates");
    pico8_cr();
    check_shared_ra();
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        if (my_state(le->items[i]) == 1) {
            unaffordable++;
        }
    }
    printf("[research] %d unaffordable tech(s)\n", unaffordable);
    assert(unaffordable >= 1);
    assert(!js_of(le->items[le->count - 1])->has_my && js_of(le->items[le->count - 1])->jb != NULL);
    jb = js_of(le->items[le->count - 1])->jb;
    assert(jb->ls_count == 1);
    line = jb->ls[0];
    assert(line->items_count == 1 && line->items[0].kind == PICO8_LQ_CELL_STRING);
    assert(strcmp(line->items[0].as.s, "slipgates") == 0);
    h = fold_le(h, "research");
    check("research", h, RESEARCH_HASH);
}



static void test_fe(void) {
    uint32_t h = 2166136261u;
    const pico8_ents_t *le;
    int with_pt = 0;
    int i;

    known_panel();
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        with_pt += js_of(le->items[i])->_jo_fe_pt != NULL;
    }
    printf("[fe] %d row(s) carry _jo_fe_pt\n", with_pt);
    assert(with_pt >= 1);
    h = fold_le(h, "fe");
    check("fe", h, FE_HASH);
}

static void test_gh(void) {
    uint32_t h = 2166136261u;
    built_panel();
    h = fold_le(h, "gh");
    check("gh", h, GH_HASH);
}

/* `pj:ph`'s two buttons, whose results nothing keeps: the `fa`=15 entities
 * with an `lq`. */
static void test_pj(void) {
    uint32_t h = 2166136261u;
    const pico8_ents_t *hd;
    int n = 0;
    int i;

    game_end();
    hd = pico8_cv("hd");
    for (i = 0; i < hd->count; i++) {
        void *e = hd->items[i];
        if (is_js(e) && js_of(e)->fa == pico8_fixed_from_int(15) && js_of(e)->lq != NULL) {
            n++;
            h = fold_button(h, e);
        }
    }
    check_int("pj buttons", n, 2);
    check("pj", h, PJ_HASH);
}

/* `dk` is the button `eu` got: its `en` sizes the galaxy. */
static void test_dk(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 1; i <= 4; i++) {
        void *nj;
        const pico8_ents_t *jo;
        int j;

        start_title();
        nj = title_button(i);
        pico8_eu(nj);
        assert(pico8_dk() == nj);
        h = fold_fixed_or_nil(h, js_of(nj)->has_en, js_of(nj)->en);
        h = fold_fixed_or_nil(h, js_of(nj)->has_trade, js_of(nj)->trade);
        h = fold_string_or_nil(h, js_of(nj)->px);
        jo = pico8_cg("jo");
        printf("[dk] bo[%d]: %d planet(s)\n", i, jo->count);
        h = fnv(h, (uint32_t)jo->count);
        for (j = 0; j < jo->count; j++) {
            h = fold_vec(h, as_jo(jo->items[j])->ra);
        }
    }
    check("dk", h, DK_HASH);
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

static pico8_vec_t *vec_at(int x, int y) {
    pico8_vec_t *p = malloc(sizeof *p);

    *p = pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
    return p;
}

/* A button of `keys` whose `lq` is `{lq_text}`, and whose `h_`, when
 * `h_text` is not NULL, is `{h_text,c=13}`. */
static void *build(pico8_js_keys_t keys, const char *lq_text, const char *h_text) {
    pico8_lq_cell_t lq_item = pico8_lq_cell_string(lq_text);
    pico8_lq_cell_t h_item;
    pico8_lq_keys_t lq = {0};
    pico8_lq_keys_t h_ = {0};

    lq.items = &lq_item;
    lq.count = 1;
    keys.lq_row = &lq;
    if (h_text != NULL) {
        h_item = pico8_lq_cell_string(h_text);
        h_.items = &h_item;
        h_.count = 1;
        h_.has_c = 1;
        h_.c = pico8_fixed_from_int(13);
        keys.has_h_ = 1;
        keys.h_row = &h_;
    }
    return pico8_js_new(&keys);
}

static pico8_js_keys_t at(int x, int y) {
    pico8_js_keys_t keys = {0};

    keys.ra = vec_at(x, y);
    return keys;
}

static pico8_js_keys_t keyed_my(pico8_js_keys_t keys, pico8_js_fn my) {
    keys.has_my = 1;
    keys.my = my;
    return keys;
}

/* `ph` moves `ra` 0.4 of the way to `qf`, then jumps the last unit, always
 * onto a fresh vector. */
static void test_ph(void) {
    uint32_t h = 2166136261u;
    void *b;
    const pico8_vec_t *first;
    pico8_vec_t first_at;
    int steps = 0;

    start_game();
    {
        pico8_js_keys_t keys = at(3, 4);

        keys.qf = vec_at(40, -20);
        b = build(keys, "go", NULL);
    }
    first = js_of(b)->ra;
    first_at = vec_of(first);
    while (steps < 40) {
        pico8_vec_t before = vec_of(js_of(b)->ra);
        cls_of(b)->states[PICO8_STATE_PH](b, pico8_fixed_from_int(0));
        steps++;
        h = fold_vec(h, js_of(b)->ra);
        if (vec_of(js_of(b)->ra).x == before.x && vec_of(js_of(b)->ra).y == before.y) {
            break;
        }
    }
    assert(first->x == first_at.x && first->y == first_at.y);
    assert(vec_of(js_of(b)->ra).x == pico8_fixed_from_int(40) && vec_of(js_of(b)->ra).y == pico8_fixed_from_int(-20));
    printf("[ph] %d step(s)\n", steps);
    h = fnv(h, (uint32_t)steps);
    check("ph", h, PH_HASH);
}

static int my_calls;
static void *my_self;

static void counting_my(pico8_js_t *self) {
    my_calls++;
    my_self = self;
}

static void test_ldown(void) {
    uint32_t h = 2166136261u;
    const pico8_js_keys_t none = {0};
    pico8_down_t r;
    void *with_my;
    void *with_false;

    start_game();
    with_my = build(keyed_my(none, counting_my), "a", NULL);
    with_false = build(keyed_my(none, NULL), "b", NULL);

    r = cls_of(with_my)->ldown(with_my, NULL, NULL);
    assert(r.stop && r.held == NULL);
    assert(my_calls == 1 && my_self == with_my);
    r = cls_of(with_false)->ldown(with_false, NULL, NULL);
    assert(r.stop && r.held == NULL);
    assert(my_calls == 1);
    assert(my_state(with_false) == 1);
    h = fnv(h, (uint32_t)my_calls);
    h = fold_button(h, with_my);
    h = fold_button(h, with_false);
    check("ldown", h, LDOWN_HASH);
}

static uint32_t lq_first(const void *b) {
    const pico8_lq_t *lq = js_of(b)->lq;
    pico8_lq_cell_t v;
    assert(lq != NULL && row_count(lq) > 0);
    v = row_item(lq, 0);
    assert(v.kind == PICO8_LQ_CELL_FIXED);
    return (uint32_t)pico8_fixed_flr_int(v.as.fixed);
}

/* `ix[2].hb` rebuilds its button's `lq`: 24 when a tech is affordable. */
static void test_hb(void) {
    uint32_t h = 2166136261u;
    void *ix[4];
    pico8_cb_kept_t cb;
    void *before;
    int affordable;
    int i;

    start_game();
    assert(ix_buttons(ix) == 4);
    for (i = 0; i < 4; i++) {
        assert(cls_of(ix[i])->has_hb(ix[i]) == (i == 1));
    }
    before = js_of(ix[1])->lq;
    cls_of(ix[1])->hb(ix[1]);
    cb = pico8_cb(0);
    affordable = cb.count > 0;
    assert(js_of(ix[1])->lq != before);
    assert(lq_first(ix[1]) == (affordable ? 24u : 37u));
    h = fnv(h, lq_first(ix[1]));
    h = fold_fixed(h, js_of(ix[1])->lq->w);
    h = fold_fixed(h, js_of(ix[1])->mb);
    check("hb", h, HB_HASH);
}

/* The `my` of a `jo:fe` row builds on the planet it captured. */
static void test_fe_my(void) {
    uint32_t h = 2166136261u;
    const pico8_ents_t *le;
    int i;

    known_panel();
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        void *b = le->items[i];
        const pico8_jo_t *pt;
        if (js_of(b)->_jo_fe_pt == NULL) {
            continue;
        }
        pt = js_of(b)->_jo_fe_pt;
        h = fold_string_or_nil(h, pt->header.me);
        h = fnv(h, (uint32_t)pico8_hv()->count);
        call_my(b);
        printf("[fe my] row %d: planet me=%s, %d entities\n", i,
               pt->header.me != NULL && strcmp(pt->header.me, "ly") == 0 ? "ly" : "not ly", pico8_hv()->count);
        h = fold_string_or_nil(h, pt->header.me);
        h = fnv(h, (uint32_t)pico8_hv()->count);
        break;
    }
    assert(i < le->count);
    check("fe my", h, FE_MY_HASH);
}

/* The `my` of a `cr` row invents the tech it captured. */
static void test_cr_my(void) {
    uint32_t h = 2166136261u;
    const pico8_ents_t *le;
    const pico8_ptrs_t *invented;
    const char *last;
    int i;

    tech_screen();
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        if (my_state(le->items[i]) == 3) {
            break;
        }
    }
    assert(i < le->count);
    assert(pico8_fm_hy() != NULL);
    invented = pico8_fm_hy()->invented;
    h = fnv(h, (uint32_t)invented->count);
    call_my(le->items[i]);
    invented = pico8_fm_hy()->invented;
    h = fnv(h, (uint32_t)invented->count);
    assert(invented->count >= 1);
    last = invented->items[invented->count - 1];
    h = fold_string(h, last);
    printf("[cr my] row %d invented %s\n", i, last);
    check("cr my", h, CR_MY_HASH);
}

static uint32_t draw_alone(void *b) {
    pico8_camera(&gfx, 0, 0);
    pico8_cls(&mem);
    cls_of(b)->kj(b, js_of(b)->ra);
    cls_of(b)->ev(b, NULL);
    return screen_hash();
}

/* One button at a time on a cleared screen: `my` absent, present, hovered,
 * with a price `h_`, a tooltip `jb` shown only when hovered, and an `h_`
 * left `false` (a free building of `jo:fe`). */
static void test_draw(void) {
    uint32_t h = 2166136261u;
    pico8_js_keys_t free_keys = keyed_my(at(10, 20), counting_my);
    void *plain;
    void *with_my;
    void *priced;
    void *tooltip;
    void *free_;
    uint32_t px[7];
    size_t i;

    start_game();
    plain = build(at(10, 20), "plain", NULL);
    with_my = build(keyed_my(at(10, 20), counting_my), "plain", NULL);
    priced = build(keyed_my(at(10, 20), counting_my), "plain", "12$");
    free_keys.has_h_ = 1;
    free_keys.h_is_false = 1;
    free_ = build(free_keys, "plain", NULL);
    tooltip = build(at(10, 20), "plain", NULL);
    ((pico8_js_t *)tooltip)->jb = pico8_jb_new(&pico8_dr_table[0]);

    px[0] = draw_alone(plain);
    px[1] = draw_alone(with_my);
    js_of(with_my)->m_ = 1;
    px[2] = draw_alone(with_my);
    px[3] = draw_alone(priced);
    px[4] = draw_alone(tooltip);
    js_of(tooltip)->m_ = 1;
    px[5] = draw_alone(tooltip);
    px[6] = draw_alone(free_);
    assert(h__state(free_) == 1 && px[6] == px[1]);
    assert(px[0] != px[1] && px[1] != px[2] && px[1] != px[3] && px[4] == px[0] && px[5] != px[4]);
    h = fold_button(h, free_);
    for (i = 0; i < 7; i++) {
        h = fnv(h, px[i]);
    }
    check("draw", h, DRAW_HASH);
}

/* TROU-23: a row that carries `h_` or `jb` at nil still gets an empty
 * `lq` or `jb`; only `false`, or no key at all, gets none. */
static void test_nil_keys(void) {
    pico8_js_keys_t b = {0};
    void *nil_keys;
    void *no_keys;

    start_game();
    no_keys = pico8_js_new(&b);
    b.has_h_ = 1;
    b.has_jb = 1;
    nil_keys = pico8_js_new(&b);
    assert(!js_of(no_keys)->has_h_ && js_of(no_keys)->jb == NULL);
    assert(js_of(nil_keys)->has_h_ && is_lq(js_of(nil_keys)->h_));
    assert(js_of(nil_keys)->h_->w == 0);
    assert(js_of(nil_keys)->jb != NULL);
}

/* How many `js` structs hv holds. */
static int all_buttons_are_structs(void) {
    const pico8_ents_t *hv = pico8_hv();
    int n = 0;
    int i;
    for (i = 0; i < hv->count; i++) {
        n += is_js(hv->items[i]);
    }
    return n;
}

/* Built by any producer, a `js` is a struct with the five methods of its
 * descriptor. */
static void test_js_is_a_struct(void) {
    void *ix[4];
    void *nj;
    int i;

    start_title();
    check_int("struct title", all_buttons_are_structs(), 4);
    nj = title_button(2);
    pico8_eu(nj);
    assert(is_js(nj) && pico8_dk() == nj);
    assert(ix_buttons(ix) == 4);
    check_int("struct eu", all_buttons_are_structs(), 4);
    for (i = 0; i < 4; i++) {
        assert(cls_of(ix[i])->has_hb(ix[i]) == (i == 1));
        if (i == 1) {
            cls_of(ix[i])->hb(ix[i]);
        }
    }
    call_my(ix[3]);
    check_int("struct pause", all_buttons_are_structs(), 8);
    pico8_cr();
    check_int("struct research", all_buttons_are_structs(), 9);
    {
        const pico8_js_keys_t empty = {0};
        void *bare = pico8_js_new(&empty);
        assert(is_js(bare) && js_of(bare)->fa == pico8_fixed_from_int(11) && !js_of(bare)->has_my);
    }
}

/* The kind words the frozen hashes fold before a value, FUNCTION_KIND the one
 * the name reader gave a callback (the constants that fold `my` and `hb`); the
 * enum entries are gone. Any other value moves them. */
enum { FIXED_KIND = 0, STRING_KIND = 1, BOOL_KIND = 2, REF_KIND = 3, FUNCTION_KIND = 4 };

static int is_of_class(const void *ref, const pico8_struct_class_t *cls) {
    return ref != NULL && ((const pico8_struct_header_t *)ref)->cls == cls;
}

/* `my` or `hb` from its member, in the bytes the name reader's value folded
 * to: absent, `false`, or a function by first sight. */
static uint32_t fold_js_fn(uint32_t h, int present, pico8_js_fn fn) {
    if (!present) {
        return fnv(h, 0u);
    }
    if (fn == NULL) {
        return fold_bool(fnv(fnv(h, 1u), (uint32_t)BOOL_KIND), 0);
    }
    return fnv(fnv(fnv(fnv(h, 1u), (uint32_t)FUNCTION_KIND), 0xFCu), fn_index(fn));
}

/* A present member: bit 1, then the word of the kind the name reader gave
 * it. Both are constant by the struct's shape for a member of one kind,
 * kept so that JS_MEMBERS_HASH still proves the typed reads. */
static uint32_t fold_kind(uint32_t h, uint32_t kind) {
    return fnv(fnv(h, 1u), (uint32_t)kind);
}

/* A present reference: a row by content, a `jb`/`jo`/`hp` struct by class,
 * anything else by its tag. */
static uint32_t fold_member_ref(uint32_t h, const void *ref) {
    h = fold_kind(h, REF_KIND);
    if (is_lq(ref)) {
        return fold_lq(fnv(h, 0xA2u), ref);
    }
    if (is_of_class(ref, pico8_jb_struct_class())) {
        return fnv(h, 0xB1u);
    }
    if (is_of_class(ref, pico8_jo_struct_class())) {
        return fnv(h, 0xB2u);
    }
    if (is_of_class(ref, pico8_hp_struct_class())) {
        return fnv(h, 0xB3u);
    }
    return fnv(fnv(h, 0xB5u), FROZEN_STRUCT_TAG);
}

static uint32_t fold_member_opt_ref(uint32_t h, const void *ref) {
    return ref != NULL ? fold_member_ref(h, ref) : fnv(h, 0u);
}

static uint32_t fold_member_vec(uint32_t h, const pico8_vec_t *v) {
    if (v == NULL) {
        return fnv(h, 0u);
    }
    return fold_vec(fold_kind(h, REF_KIND), v);
}

static uint32_t fold_member_fixed(uint32_t h, int has, pico8_fixed_t f) {
    return has ? fold_fixed(fold_kind(h, FIXED_KIND), f) : fnv(h, 0u);
}

/* The 17 members and the two callbacks, in the order the frozen constants
 * folded them: `my` after `qf`, `hb` after `m_`. */
static uint32_t fold_js_members(uint32_t h, const void *e) {
    const pico8_js_t *b = js_of(e);

    h = fold_member_fixed(h, 1, b->fa);
    h = fold_member_opt_ref(h, b->lq);
    h = fold_member_fixed(h, 1, b->mb);
    h = b->header.kq != NULL ? fold_kq(fold_kind(h, REF_KIND), b) : fnv(h, 0u);
    if (b->has_h_ && b->h_ == NULL) {
        h = fold_bool(fold_kind(h, BOOL_KIND), 0);
    } else {
        h = fold_member_opt_ref(h, b->has_h_ ? b->h_ : NULL);
    }
    h = fold_member_opt_ref(h, b->jb);
    h = fold_member_vec(h, b->ra);
    h = fold_member_vec(h, b->qf);
    h = fold_js_fn(h, b->has_my, b->my);
    h = (b->m_ != 0) ? fold_bool(fold_kind(h, BOOL_KIND), b->m_) : fnv(h, 0u);
    h = fold_js_fn(h, b->hb != NULL, b->hb);
    h = fold_member_fixed(h, b->has_fg, b->fg);
    h = b->px != NULL ? fold_string(fold_kind(h, STRING_KIND), b->px) : fnv(h, 0u);
    h = fold_member_fixed(h, b->has_trade, b->trade);
    h = fold_member_fixed(h, b->has_en, b->en);
    if (b->_cr_t != NULL) {
        const pico8_nb_entry_t *t = b->_cr_t;
        assert(t == pico8_nb_lookup(t->px));
        h = fold_fixed(fnv(fold_kind(h, REF_KIND), 0xB4u), pico8_fixed_from_int(t->nk));
    } else {
        h = fnv(h, 0u);
    }
    h = fold_member_opt_ref(h, b->_jo_fe_pt);
    h = fold_member_opt_ref(h, b->_jo_fe_b);
    return fold_member_fixed(h, b->has__jo_fe_od, b->_jo_fe_od);
}

static uint32_t fold_le_members(uint32_t h) {
    const pico8_ents_t *le = ok_le();
    int i;

    assert(le != NULL);
    h = fnv(h, (uint32_t)le->count);
    for (i = 0; i < le->count; i++) {
        h = fold_js_members(h, le->items[i]);
    }
    return h;
}

/* One `pz` call: one `ra` for all, a `qf` each. */
static void check_one_pz(void *const *b, int n) {
    int i;
    int j;

    for (i = 0; i < n; i++) {
        assert(js_of(b[i])->ra != NULL && js_of(b[i])->ra == js_of(b[0])->ra);
        for (j = 0; j < i; j++) {
            assert(js_of(b[i])->qf != NULL && js_of(b[i])->qf != js_of(b[j])->qf);
        }
    }
}

static void check_le_one_pz(void) {
    const pico8_ents_t *le = ok_le();
    void *b[32];
    int i;

    assert(le != NULL && le->count <= 32);
    for (i = 0; i < le->count; i++) {
        b[i] = le->items[i];
    }
    check_one_pz(b, le->count);
}

/* `m_` true, false, or nil (`present` 0), which reads as false. */
static void write_m_(pico8_js_t *b, int present, int value) {
    b->m_ = present ? value : 0;
}

/* `m_` takes true, false and nil; `lq` a row or nil. The raw writer
 * answered 0 to the 15 other names and to 8 strangers: those 23 words are
 * constant by the writer's shape, kept so that JS_MEMBERS_HASH still proves
 * the typed reads. */
static uint32_t fold_written(uint32_t h, void *e, void *other) {
    pico8_js_t *b = js_of(e);
    const pico8_qz_box_t *kq_before = b->header.kq;
    pico8_fixed_t mb = b->mb;
    pico8_lq_t *row;
    int i;

    assert(kq_before != NULL);
    write_m_(b, 1, 1);
    assert(b->m_ == 1);
    h = fold_js_members(h, e);
    write_m_(b, 1, 0);
    assert(b->m_ == 0);
    h = fold_js_members(h, e);
    write_m_(b, 1, 1);
    write_m_(b, 1, 0);
    assert(b->m_ == 0);
    h = fold_js_members(h, e);
    write_m_(b, 1, 1);
    write_m_(b, 0, 0);
    assert(!b->m_);
    write_m_(b, 1, 0);
    write_m_(b, 0, 0);
    assert(!b->m_);
    write_m_(b, 0, 0);
    assert(!b->m_);
    h = fold_js_members(h, e);

    row = js_of(other)->lq;
    assert(row != NULL);
    b->lq = row;
    h = fold_js_members(h, e);
    row = pico8_lq_new(NULL);
    b->lq = row;
    h = fold_js_members(h, e);
    b->lq = NULL;
    assert(b->header.kq == kq_before);
    assert(b->mb == mb);
    h = fold_js_members(h, e);
    b->lq = row;

    write_m_(b, 1, 1);
    for (i = 0; i < 15 + 8; i++) {
        h = fnv(h, 0u);
    }
    assert(b->m_ == 1);
    return fold_js_members(h, e);
}

static void *built(pico8_js_keys_t keys) {
    return pico8_js_new(&keys);
}

/* Each optional key unset, the fixed ones and `my` given at zero, `h_`
 * false, TROU-23, an `h_` copied from an `lq` (written on the member: no
 * key builds it), a row's keys as `lq(x)`'s argument. */
static uint32_t fold_keys(uint32_t h, pico8_lq_t *row) {
    const pico8_js_keys_t none = {0};
    pico8_js_keys_t k;
    pico8_lq_cell_t from_keys[2];
    pico8_lq_keys_t from = {0};
    pico8_js_t *b;
    int i;

    for (i = 0; i < 12; i++) {
        b = built(none);
        assert(!b->has_fg && b->px == NULL && !b->has_trade && !b->has_en && !b->has__jo_fe_od);
        assert(b->ra == NULL && b->qf == NULL && b->hb == NULL && !b->has_my && b->_cr_t == NULL);
        assert(b->_jo_fe_pt == NULL && b->_jo_fe_b == NULL);
        h = fold_js_members(h, b);
    }
    k = none;
    k.has_fg = 1;
    b = built(k);
    assert(b->has_fg && b->fg == 0);
    h = fold_js_members(h, b);
    k = none;
    k.has_trade = 1;
    b = built(k);
    assert(b->has_trade && b->trade == 0);
    h = fold_js_members(h, b);
    k = none;
    k.has_en = 1;
    b = built(k);
    assert(b->has_en && b->en == 0);
    h = fold_js_members(h, b);
    k = none;
    k.has__jo_fe_od = 1;
    b = built(k);
    assert(b->has__jo_fe_od && b->_jo_fe_od == 0);
    h = fold_js_members(h, b);
    for (i = 0; i < 2; i++) {
        b = built(keyed_my(none, NULL));
        assert(my_state(b) == 1);
        h = fold_js_members(h, b);
    }
    k = none;
    k.has_h_ = 1;
    k.h_is_false = 1;
    b = built(k);
    assert(b->has_h_ && b->h_ == NULL);
    h = fold_js_members(h, b);

    k = none;
    k.has_h_ = 1;
    b = built(k);
    assert(b->has_h_ && is_lq(b->h_) && b->h_->w == 0);
    h = fold_js_members(h, b);
    k = none;
    k.has_jb = 1;
    b = built(k);
    assert(b->jb != NULL && is_of_class(b->jb, pico8_jb_struct_class()));
    h = fold_js_members(h, b);
    k = none;
    b = built(k);
    assert(b->lq != NULL && is_lq(b->lq));
    h = fold_js_members(h, b);

    k = none;
    k.has_h_ = 1;
    b = built(k);
    b->h_ = pico8_lq_copy(row);
    assert(b->has_h_ && is_lq(b->h_) && b->h_ != row);
    h = fold_js_members(h, b);

    from_keys[0] = pico8_lq_cell_string("from a list");
    from_keys[1] = pico8_lq_cell_fixed(pico8_fixed_from_int(3));
    from.items = from_keys;
    from.count = 2;
    k = none;
    k.lq_row = &from;
    b = built(k);
    assert(b->lq != NULL && is_lq(b->lq) && row_count(b->lq) == 2);
    return fold_js_members(h, b);
}

static uint32_t members_title_and_game(void) {
    const pico8_js_keys_t empty = {0};
    uint32_t h = 2166136261u;
    const pico8_ents_t *hd;
    void *title[4];
    void *ix[4];
    void *fresh[3];
    void *nj;
    void *plain;
    void *my_false;
    int n = 0;
    int i;
    size_t k;

    start_title();
    hd = pico8_cv("hd");
    for (i = 0; i < hd->count; i++) {
        void *e = hd->items[i];
        if (is_js(e)) {
            assert(n < 4);
            title[n++] = e;
            assert(js_of(e)->px != NULL);
            assert(js_of(e)->fa == pico8_fv_def.ca);
            h = fold_js_members(h, e);
        }
    }
    check_int("members title", n, 4);
    check_one_pz(title, 4);

    nj = title[1];
    assert(js_of(nj)->my == pico8_eu_field_value());
    assert(js_of(nj)->has_trade && js_of(nj)->has_en);
    printf("[members] dk: trade %d, en %d/65536\n", pico8_fixed_flr_int(js_of(nj)->trade), (int)js_of(nj)->en);
    h = fold_js_members(h, nj);
    pico8_eu(nj);
    assert(pico8_dk() == nj);
    assert(ix_buttons(ix) == 4);
    for (i = 0; i < 4; i++) {
        assert((js_of(ix[i])->hb != NULL) == (i == 1));
        h = fold_js_members(h, ix[i]);
    }

    fresh[0] = pico8_js_new(NULL);
    fresh[1] = pico8_js_new(&empty);
    for (k = 0; k < 2; k++) {
        const pico8_js_t *b = js_of(fresh[k]);
        const pico8_lq_t *lq = b->lq;
        assert(lq != NULL && is_lq(lq) && row_count(lq) == 0);
        assert(b->fa == pico8_fixed_from_int(11));
        assert(b->mb == pico8_fixed_add(lq->w, pico8_fixed_from_int(2)));
        assert(b->header.kq != NULL);
        assert(!b->has_h_ && b->jb == NULL && b->ra == NULL && b->qf == NULL && !b->m_);
        assert(!b->has_fg && b->px == NULL && !b->has_trade && !b->has_en && b->_cr_t == NULL);
        assert(b->_jo_fe_pt == NULL && b->_jo_fe_b == NULL && !b->has__jo_fe_od);
        assert(my_state(fresh[k]) == 0 && js_of(fresh[k])->hb == NULL);
        h = fold_js_members(h, fresh[k]);
    }

    call_my(ix[3]);
    check_le_one_pz();
    printf("[members] ha: %d row(s), fg on the 4th: %d\n", ok_le()->count, js_of(ok_le()->items[3])->has_fg);
    h = fold_le_members(h);

    fresh[2] = pico8_js_new(NULL);
    h = fold_written(h, ix[0], ix[1]);
    h = fold_written(h, fresh[2], ix[2]);
    assert(js_of(ix[2])->lq != NULL);
    h = fold_keys(h, js_of(ix[2])->lq);

    plain = build(at(10, 20), "plain", NULL);
    my_false = build(keyed_my(at(10, 20), NULL), "plain", NULL);
    assert(draw_alone(my_false) == draw_alone(plain));
    h = fnv(h, draw_alone(my_false));

    /* 10 strangers the name reader refused: constant by the struct's shape. */
    for (k = 0; k < 10; k++) {
        h = fnv(h, 0u);
    }
    return h;
}

static uint32_t members_research(void) {
    const pico8_ents_t *le;
    uint32_t h = 2166136261u;
    int dear = 0;
    int affordable = 0;
    int i;

    for (i = 0; i < PICO8_NB_COUNT; i++) {
        assert(pico8_nb_table[i].oy != NULL);
    }
    /* The frozen hash folded the count of the old nb table: each record
     * under its rank and under its px. */
    h = fnv(h, (uint32_t)(2 * PICO8_NB_COUNT));

    tech_screen();
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        void *b = le->items[i];

        if (my_state(b) == 0) {
            continue;
        }
        if (my_state(b) == 1) {
            assert(js_of(b)->_cr_t == NULL);
            dear++;
        } else {
            const pico8_nb_entry_t *t = js_of(b)->_cr_t;
            const pico8_lq_t *lq = js_of(b)->lq;
            assert(my_state(b) == 3 && lq != NULL);
            assert(lq->items_count >= 2 && lq->items[1].kind == PICO8_LQ_CELL_STRING);
            assert(t != NULL && t == pico8_nb_lookup(lq->items[1].as.s));
            affordable++;
        }
        assert(js_of(b)->has_h_ && is_lq(js_of(b)->h_));
    }
    printf("[members] cr: %d row(s), %d too dear, %d affordable\n", le->count, dear, affordable);
    assert(dear >= 1 && affordable >= 1);
    return fold_le_members(h);
}

static uint32_t members_fe(void) {
    const pico8_ents_t *le;
    const void *pt = NULL;
    uint32_t h = 2166136261u;
    int rows = 0;
    int free_rows = 0;
    int i;

    known_panel();
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        const pico8_js_t *b = js_of(le->items[i]);

        if (b->_jo_fe_pt == NULL) {
            continue;
        }
        if (pt == NULL) {
            pt = b->_jo_fe_pt;
        }
        assert(b->_jo_fe_pt == pt);
        assert(is_of_class(pt, pico8_jo_struct_class()) && ents_listed(pico8_cg("jo"), pt));
        assert(b->_jo_fe_b != NULL && is_of_class(b->_jo_fe_b, pico8_hp_struct_class()));
        assert(b->has__jo_fe_od);
        assert(b->my != NULL);
        assert(b->jb != NULL && is_of_class(b->jb, pico8_jb_struct_class()));
        if (b->_jo_fe_od > 0) {
            assert(b->has_h_ && is_lq(b->h_));
        } else {
            assert(b->has_h_ && b->h_ == NULL);
            free_rows++;
        }
        rows++;
    }
    printf("[members] fe: %d row(s), %d free\n", rows, free_rows);
    assert(rows >= 1);
    return fold_le_members(h);
}

static uint32_t members_gh(void) {
    built_panel();
    return fold_le_members(2166136261u);
}

static uint32_t members_pj(void) {
    uint32_t h = 2166136261u;
    const pico8_ents_t *hd;
    int n = 0;
    int i;

    game_end();
    hd = pico8_cv("hd");
    for (i = 0; i < hd->count; i++) {
        void *e = hd->items[i];
        if (is_js(e) && js_of(e)->fa == pico8_fixed_from_int(15) && js_of(e)->lq != NULL) {
            n++;
            h = fold_js_members(h, e);
        }
    }
    assert(n == 2);
    return h;
}

/* A part runs in its own child forked from the fresh state, as
 * in_fresh_child does, and hands its hash back through a pipe. */
static uint32_t in_fresh_part(uint32_t (*part)(void)) {
    int fd[2];
    pid_t child;
    int status;
    uint32_t got = 0;
    ssize_t n;

    assert(pipe(fd) == 0);
    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        close(fd[0]);
        seen_count = 0;
        got = part();
        n = write(fd[1], &got, sizeof got);
        assert(n == (ssize_t)sizeof got);
        fflush(stdout);
        _exit(0);
    }
    close(fd[1]);
    n = read(fd[0], &got, sizeof got);
    close(fd[0]);
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    assert(n == (ssize_t)sizeof got);
    return got;
}

static void test_js_members_by_name(void) {
    uint32_t h = 2166136261u;

    h = fnv(h, in_fresh_part(members_title_and_game));
    h = fnv(h, in_fresh_part(members_research));
    h = fnv(h, in_fresh_part(members_fe));
    h = fnv(h, in_fresh_part(members_gh));
    h = fnv(h, in_fresh_part(members_pj));
    check("js members", h, JS_MEMBERS_HASH);
}

/* `h_=od>0 and {…}` of `jo:fe`: a free building's `h_` is false, not the
 * empty row of a present nil. */
static void test_fe_free_row(void) {
    const pico8_ents_t *le;
    pico8_hp_t *b = NULL;
    pico8_jo_t *pt = NULL;
    int found = 0;
    int i;

    known_panel();
    le = ok_le();
    for (i = 0; i < le->count && b == NULL; i++) {
        b = js_of(le->items[i])->_jo_fe_b;
        pt = js_of(le->items[i])->_jo_fe_pt;
    }
    assert(b != NULL && pt != NULL);
    b->has_fj = 1;
    b->fj = 0;
    pico8_jo_fe(pt);
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        const pico8_js_t *row = js_of(le->items[i]);

        if (row->_jo_fe_b == b) {
            assert(row->_jo_fe_od == 0 && row->has_h_ && row->h_ == NULL);
            found = 1;
        }
    }
    assert(found);
}

/* What `ix[4].my` hands `pz` as `p`: a struct without method, for as long
 * as `ha` lives. */
static void test_ha_pz_is_a_struct(void) {
    const pico8_struct_header_t *o;
    size_t i;

    start_game();
    o = pico8_ha_pz();
    assert(o != NULL && o->cls == pico8_ha_pz_struct_class());
    assert(o->cls->kj == NULL && o->cls->ev == NULL && o->cls->hd == NULL && o->cls->hk == NULL &&
           o->cls->hb == NULL && o->cls->ldown == NULL && o->cls->rdown == NULL && o->cls->gv == NULL);
    assert(o->cls->hq == NULL && o->cls->ey == NULL && o->cls->kf == NULL && o->cls->cn == NULL && o->cls->ol == NULL &&
           o->cls->mm == NULL);
    for (i = 0; i < PICO8_STATE_COUNT; i++) {
        assert(o->cls->states[i] == NULL);
    }

    pico8_menu_shutdown();
    assert(pico8_ha_pz() == NULL);
    pico8_menu_init();
    assert(pico8_ha_pz() == o);
}

/* TROU-9: a button's `fa` is `p.ca` when `p` has one, else `js`'s own. */
static void test_pz_fa_only_from_ca(void) {
    pico8_pz_frame_t frame = {0};
    pico8_js_keys_t row = {0};

    start_game();
    pico8_pz(&frame, &row, 1);
    assert(ok_le()->count == 1 && js_of(ok_le()->items[0])->fa == pico8_fixed_from_int(11));
    frame.has_ca = 1;
    frame.ca = pico8_fixed_from_int(14);
    pico8_pz(&frame, &row, 1);
    assert(ok_le()->count == 1 && js_of(ok_le()->items[0])->fa == pico8_fixed_from_int(14));
}

/* `cb` writes `e_` for every entry, kept or not; `pico8_data_init` forgets
 * them all. */
static void test_cb_writes_every_e_(void) {
    pico8_cb_kept_t kept;
    pico8_fixed_t jk;
    int dropped_affordable = 0;
    int i;

    start_game();
    pico8_fm_hy()->jk = pico8_fixed_from_int(12);
    jk = pico8_fm_hy()->jk;
    kept = pico8_cb(0);
    for (i = 0; i < PICO8_NB_COUNT; i++) {
        const pico8_nb_entry_t *t = &pico8_nb_table[i];
        int affordable = pico8_gi_nb(t) <= jk;
        int k;
        int is_kept = 0;

        assert(pico8_nb_e_(t) == affordable);
        for (k = 0; k < kept.count; k++) {
            is_kept |= kept.items[k] == t;
        }
        dropped_affordable += affordable && !is_kept;
    }
    assert(dropped_affordable > 0);
    pico8_fm_hy()->jk = pico8_fixed_from_int(999);
    (void)pico8_cb(0);
    for (i = 0; i < PICO8_NB_COUNT; i++) {
        assert(pico8_nb_e_(&pico8_nb_table[i]) == 1);
    }
    pico8_data_init();
    for (i = 0; i < PICO8_NB_COUNT; i++) {
        assert(pico8_nb_e_(&pico8_nb_table[i]) == 0);
    }
}

/* A NULL frame makes `pz` a no-op. */
static void test_pz_closed(void) {
    start_game();
    pico8_pz(NULL, NULL, 0);
}

static void test_panels_are_structs(void) {
    known_panel();
    assert(all_buttons_are_structs() >= 2);
}

static void test_pj_are_structs(void) {
    game_end();
    assert(all_buttons_are_structs() >= 2);
}

/* The galaxy depends on the rnd state: run each scenario in a child forked
 * right after pico8_cart_load(), as a fresh process would start it. */
static void in_fresh_child(void (*test)(void)) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        seen_count = 0;
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
    in_fresh_child(test_title);
    in_fresh_child(test_eu);
    in_fresh_child(test_pause);
    in_fresh_child(test_research);
    in_fresh_child(test_fe);
    in_fresh_child(test_gh);
    in_fresh_child(test_pj);
    in_fresh_child(test_dk);
    in_fresh_child(test_ph);
    in_fresh_child(test_ldown);
    in_fresh_child(test_hb);
    in_fresh_child(test_fe_my);
    in_fresh_child(test_cr_my);
    in_fresh_child(test_draw);
    in_fresh_child(test_nil_keys);
    in_fresh_child(test_fe_free_row);
    in_fresh_child(test_js_is_a_struct);
    in_fresh_child(test_js_members_by_name);
    in_fresh_child(test_ha_pz_is_a_struct);
    in_fresh_child(test_pz_closed);
    in_fresh_child(test_pz_fa_only_from_ca);
    in_fresh_child(test_cb_writes_every_e_);
    in_fresh_child(test_panels_are_structs);
    in_fresh_child(test_pj_are_structs);

    printf("test_pico8_js: all tests passed\n");
    return 0;
}
