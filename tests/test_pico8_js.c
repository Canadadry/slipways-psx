/* The button `js`, read and written only through the dual-form field API
 * and driven through the real `pz`/`eu`/`cr`/`ix`/`dg`/`dc`/`mv`, on title
 * screens and games built directly: no events file. */

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
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy.h"
#include "../src/pico8_economy_score.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_geom.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_menu.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_tech_gate.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

/* `gb[12]`, a colony that needs two resources and exports one; `gb[3]`, a
 * project with an `o` and no `i`, which exports to any planet a slipway
 * joins it to. */
#define COLONY 12
#define EXPORTER 3

/* The tag class instances carried, which the frozen hashes fold: any
 * other value moves every *_HASH. */
#define FROZEN_INSTANCE_TAG 0x9E5C1451u

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
#define JS_MEMBERS_HASH 0x31CE3B93u

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

static int rank_in(const pico8_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i].kind == PICO8_VALUE_REF && list->items[i].as.ref == e) {
            return i;
        }
    }
    return -1;
}

static int listed(const pico8_list_t *list, const void *e) {
    return rank_in(list, e) >= 0;
}

/* The instance reader keeps a field written nil, the struct reader reads a
 * nil member as absent: nil and absent are one state here. */
static int lookup(const void *e, const char *name, pico8_value_t *out) {
    return pico8_entity_get_field(e, name, out) && !(out->kind == PICO8_VALUE_REF && out->as.ref == NULL);
}

static int has(const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v);
}

static pico8_value_t get(const void *e, const char *name) {
    pico8_value_t v = pico8_value_ref(NULL);
    (void)pico8_entity_get_field(e, name, &v);
    return v;
}

static void *get_ref(const void *e, const char *name) {
    pico8_value_t v = get(e, name);
    return v.kind == PICO8_VALUE_REF ? v.as.ref : NULL;
}

static pico8_fixed_t must_fixed(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static int is_string(const void *e, const char *name, const char *want) {
    pico8_value_t v;
    return pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, want) == 0;
}

/* 0 absent, 1 false, 2 true, 3 other. */
static uint32_t bool_state(const void *e, const char *name) {
    pico8_value_t v;
    if (!lookup(e, name, &v)) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_BOOL) {
        return v.as.boolean ? 2 : 1;
    }
    return 3;
}

static pico8_vec_t vec_of(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return *(const pico8_vec_t *)v.as.ref;
}

static uint32_t fold_vec(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    const pico8_vec_t *p;
    if (!lookup(e, name, &v)) {
        return fnv(h, 0xDEADu);
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    p = v.as.ref;
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

/* A function by the order it was first seen in the scenario: its address
 * is not stable from one build to the next. */
static pico8_value_function_t seen_fns[64];
static int seen_count;

static uint32_t fn_index(pico8_value_function_t fn) {
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

/* A research project folds by the tag of the class-instance form these
 * values were frozen on, whatever its form now. */
static int in_gb(const void *e) {
    const pico8_ob_table_t *gb = pico8_gb();
    int i;
    for (i = 0; gb != NULL && i < gb->count; i++) {
        if (gb->entries[i].value.kind == PICO8_VALUE_REF && gb->entries[i].value.as.ref == e) {
            return 1;
        }
    }
    return 0;
}

static int is_lq(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_lq_struct_class();
}

static int row_count(const void *lq) {
    return ((const pico8_lq_t *)lq)->items.count;
}

static pico8_value_t row_item(const void *lq, int i) {
    assert(i >= 0 && i < row_count(lq));
    return ((const pico8_lq_t *)lq)->items.items[i];
}

static uint32_t fold_value(uint32_t h, pico8_value_t v) {
    switch (v.kind) {
    case PICO8_VALUE_BOOL:
        return fnv(fnv(h, 0xB0u), (uint32_t)v.as.boolean);
    case PICO8_VALUE_FIXED:
        return fnv(fnv(h, 0xF1u), (uint32_t)v.as.fixed);
    case PICO8_VALUE_STRING:
        return fnv_str(fnv(h, 0x57u), v.as.string);
    case PICO8_VALUE_FUNCTION:
        return fnv(fnv(h, 0xFCu), fn_index(v.as.function));
    case PICO8_VALUE_REF:
        if (v.as.ref == NULL) {
            return fnv(h, 0xDEADu);
        }
        if (listed(pico8_hv(), v.as.ref)) {
            return fnv(fnv(h, 0x4Eu), (uint32_t)rank_in(pico8_hv(), v.as.ref));
        }
        if (in_gb(v.as.ref)) {
            return fnv(fnv(h, 0xEEEEu), FROZEN_INSTANCE_TAG);
        }
        return fnv(fnv(h, 0xEEEEu), (uint32_t)pico8_type_tag_of(v.as.ref));
    default:
        return fnv(h, 0xF0u);
    }
}

/* A row by its width and its positional items. */
static uint32_t fold_lq(uint32_t h, const void *lq) {
    int i;

    assert(is_lq(lq));
    h = fold_value(h, get(lq, "w"));
    for (i = 0; i < row_count(lq); i++) {
        pico8_value_t item = row_item(lq, i);
        h = fnv(h, (uint32_t)(i + 1));
        h = item.kind == PICO8_VALUE_REF ? fnv(h, 0x12u) : fold_value(h, item);
    }
    return h;
}

/* Absent, `false`, or what it holds: a row, a `jb` struct. A row folds by
 * the tag of the class-instance form these values were frozen on. */
static uint32_t fold_built(uint32_t h, const void *b, const char *name) {
    pico8_value_t v;
    if (!lookup(b, name, &v)) {
        return fnv(h, 0xA0u);
    }
    if (v.kind != PICO8_VALUE_REF) {
        return fold_value(fnv(h, 0xA1u), v);
    }
    if (is_lq(v.as.ref)) {
        return fold_lq(fnv(fnv(h, 0xA2u), FROZEN_INSTANCE_TAG), v.as.ref);
    }
    h = fnv(fnv(h, 0xA2u), (uint32_t)pico8_type_tag_of(v.as.ref));
    return h;
}

static uint32_t fold_kq(uint32_t h, const void *b) {
    static const int dy[] = {-2, -1, 0, 5, 9, 10, 11};
    const pico8_qz_box_t *kq = get_ref(b, "kq");
    int mb = pico8_fixed_flr_int(must_fixed(b, "mb"));
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

static const pico8_list_t *ok_le(void) {
    pico8_value_t le;
    if (pico8_ok() == NULL || !lookup(pico8_ok(), "le", &le) || le.kind != PICO8_VALUE_REF) {
        return NULL;
    }
    return le.as.ref;
}

/* A button as `js(b)` and `js:pq` left it: every construction key, what
 * `pq` built, the methods and where `lt` registered it. */
static uint32_t fold_button(uint32_t h, const void *b) {
    static const char *const copied[] = {"hb", "fg", "px", "trade", "en", "_jo_fe_pt", "_jo_fe_b", "_jo_fe_od"};
    static const char *const methods[] = {"ph", "kj", "ev", "ldown", "hd"};
    static const char *const absent[] = {"p_", "nu", "ql", "rdown", "gv", "hq", "ey", "md", "hk"};
    static const char *const bm[] = {"kj", "ev", "hd", "hb", "hk"};
    pico8_value_t v;
    size_t i;

    assert(is_string(b, "me", "ph"));
    h = fold_value(h, get(b, "t"));
    h = fold_value(h, get(b, "fa"));
    h = fold_value(h, get(b, "mb"));
    h = fold_kq(h, b);
    h = fold_vec(h, b, "ra");
    h = fold_vec(h, b, "qf");
    h = fold_built(h, b, "lq");
    h = fold_built(h, b, "h_");
    h = fold_built(h, b, "jb");
    if (lookup(b, "my", &v)) {
        h = fold_value(fnv(h, 0x33u), v);
    } else {
        h = fnv(h, 0x34u);
    }
    for (i = 0; i < sizeof copied / sizeof copied[0]; i++) {
        h = lookup(b, copied[i], &v) ? fold_value(fnv(h, 1u), v) : fnv(h, 0u);
    }
    if (lookup(b, "_cr_t", &v)) {
        pico8_value_t nk;
        assert(v.kind == PICO8_VALUE_REF && pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE);
        assert(pico8_ob_table_get_name(v.as.ref, "nk", &nk));
        h = fold_value(fnv(h, 1u), nk);
    } else {
        h = fnv(h, 0u);
    }
    h = fnv(h, bool_state(b, "m_"));
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        assert(pico8_entity_get_field(b, methods[i], &v) && v.kind == PICO8_VALUE_FUNCTION);
    }
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(b, absent[i]));
    }
    h = fnv(h, (uint32_t)listed(pico8_hv(), b));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)listed(pico8_cv(bm[i]), b));
    }
    return fnv(h, (uint32_t)rank_in(ok_le(), b));
}

static uint32_t fold_le(uint32_t h, const char *label) {
    const pico8_list_t *le = ok_le();
    int i;

    assert(le != NULL);
    printf("[%s] %d button(s) in ok.le\n", label, le->count);
    h = fnv(h, (uint32_t)le->count);
    for (i = 0; i < le->count; i++) {
        void *b = le->items[i].as.ref;
        printf("[%s] button %d: my %u, h_ %u, jb %u\n", label, i, bool_state(b, "my"), bool_state(b, "h_"),
               bool_state(b, "jb"));
        h = fold_button(h, b);
    }
    return h;
}

/* The `ra` every row of one `pz` call shares. */
static void check_shared_ra(void) {
    const pico8_list_t *le = ok_le();
    int i;
    for (i = 1; le != NULL && i < le->count; i++) {
        assert(get_ref(le->items[i].as.ref, "ra") == get_ref(le->items[0].as.ref, "ra"));
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
    const pico8_list_t *hd = pico8_cv("hd");
    int k;

    for (k = 0; hd != NULL && k < hd->count; k++) {
        void *e = hd->items[k].as.ref;
        pico8_value_t mb;
        if (pico8_entity_get_field(e, "mb", &mb) && mb.kind == PICO8_VALUE_FIXED && --i == 0) {
            return e;
        }
    }
    assert(0 && "no such title button");
    return NULL;
}

static pico8_vec_t pos_of(const void *e) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, "p", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return *(const pico8_vec_t *)v.as.ref;
}

static pico8_fixed_t dist2(pico8_vec_t a, pico8_vec_t b) {
    pico8_vec_t d = pico8_vec_sub(a, b);
    return pico8_fixed_add(pico8_fixed_mul(d.x, d.x), pico8_fixed_mul(d.y, d.y));
}

static pico8_fixed_t world_fixed(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static pico8_hp_t *project(int k) {
    pico8_value_t v;
    assert(pico8_ob_table_get_index(pico8_gb(), k, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

/* A generated planet colonized by COLONY, its nearest one by EXPORTER, the
 * two joined by a slipway paid by `ey` as a drag pays it (`jc:hq` reads the
 * hovered planet off `ok.o_`), then left to settle; returns the colony. */
static void *built_world(void) {
    const pico8_list_t *jos;
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    pico8_vec_t *mp = malloc(sizeof *mp);
    pico8_value_t out;
    void *qu;
    void *re = NULL;
    void *jc;
    int i;

    start_game();
    jos = pico8_cg("jo");
    assert(jos != NULL && jos->count > 1);
    qu = jos->items[0].as.ref;
    for (i = 1; i < jos->count; i++) {
        void *e = jos->items[i].as.ref;
        if (re == NULL || dist2(pos_of(e), pos_of(qu)) < dist2(pos_of(re), pos_of(qu))) {
            re = e;
        }
    }
    pico8_jo_hu(qu, project(COLONY));
    pico8_jo_hu(re, project(EXPORTER));

    pico8_ob_table_set_name(overrides, "qu", pico8_value_ref(qu));
    jc = pico8_jc_new(overrides);
    pico8_ob_table_free(overrides);
    *mp = pico8_vec_add(pico8_vec_mul(pos_of(re), pico8_fixed_from_int(128)),
                        pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(1)));
    pico8_ok()->o_ = re;
    assert(pico8_mv(jc, "hq", 1, pico8_value_ref(mp), pico8_value_ref(NULL), &out));
    pico8_ok()->o_ = NULL;
    while (world_fixed(jc, "t") < pico8_fixed_from_int(15)) {
        pico8_dg();
    }
    assert(pico8_mv(jc, "ey", 1, pico8_value_ref(re), pico8_value_ref(NULL), &out));
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
    p = pico8_cg("jo")->items[0].as.ref;
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
    pico8_value_t out;

    start_game();
    pico8_fm_hy()->hl = "final score";
    pico8_fm_hy()->has_hl = 1;
    assert(pico8_mv((pico8_struct_header_t *)pico8_ju(), "ph", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    one_frame();
}

static pico8_value_t call(void *e, const char *name, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, 2, arg0, arg1, &out));
    return out;
}

static pico8_value_t nil(void) {
    return pico8_value_ref(NULL);
}

/* The buttons `eu`'s `nv(ix,js)` built: the only `fa`=11 entities with an
 * `lq` in `cv.hd` right after `eu`. */
static int ix_buttons(void *out[4]) {
    const pico8_list_t *hd = pico8_cv("hd");
    int n = 0;
    int i;
    for (i = 0; hd != NULL && i < hd->count; i++) {
        void *e = hd->items[i].as.ref;
        pico8_value_t fa;
        if (lookup(e, "fa", &fa) && fa.kind == PICO8_VALUE_FIXED && fa.as.fixed == pico8_fixed_from_int(11) &&
            has(e, "lq")) {
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
    const pico8_list_t *hd;
    void *first = NULL;
    int n = 0;
    int i;

    start_title();
    hd = pico8_cv("hd");
    for (i = 0; i < hd->count; i++) {
        void *e = hd->items[i].as.ref;
        if (has(e, "mb")) {
            if (first == NULL) {
                first = e;
            }
            assert(get_ref(e, "ra") == get_ref(first, "ra"));
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
        assert(!listed(ok_le(), ix[i]));
        assert(has(ix[i], "hb") == (i == 1));
        h = fold_button(h, ix[i]);
    }
    check("eu", h, EU_HASH);
}

static void test_pause(void) {
    uint32_t h = 2166136261u;
    void *ix[4];

    start_game();
    assert(ix_buttons(ix) == 4);
    (void)call(ix[3], "my", nil(), nil());
    check_shared_ra();
    h = fold_le(h, "pause");
    check("pause", h, PAUSE_HASH);
}

static void invent(const char *rr) {
    pico8_value_t v;
    assert(pico8_fm() != NULL && pico8_entity_get_field(pico8_fm(), "invented", &v) &&
           v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    pico8_list_add((pico8_list_t *)v.as.ref, pico8_value_string(rr));
}

static void test_research(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *le;
    int unaffordable = 0;
    int i;

    start_game();
    invent("slipgates");
    pico8_cr();
    check_shared_ra();
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        if (bool_state(le->items[i].as.ref, "my") == 1) {
            unaffordable++;
        }
    }
    printf("[research] %d unaffordable tech(s)\n", unaffordable);
    assert(unaffordable >= 1);
    assert(!has(le->items[le->count - 1].as.ref, "my") && has(le->items[le->count - 1].as.ref, "jb"));
    h = fold_le(h, "research");
    check("research", h, RESEARCH_HASH);
}



static void test_fe(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *le;
    int with_pt = 0;
    int i;

    known_panel();
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        with_pt += has(le->items[i].as.ref, "_jo_fe_pt");
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
    const pico8_list_t *hd;
    int n = 0;
    int i;

    game_end();
    hd = pico8_cv("hd");
    for (i = 0; i < hd->count; i++) {
        void *e = hd->items[i].as.ref;
        pico8_value_t fa;
        if (lookup(e, "fa", &fa) && fa.kind == PICO8_VALUE_FIXED && fa.as.fixed == pico8_fixed_from_int(15) &&
            has(e, "lq")) {
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
        const pico8_list_t *jo;
        int j;

        start_title();
        nj = title_button(i);
        pico8_eu(nj);
        assert(pico8_dk() == nj);
        h = fold_value(h, get(nj, "en"));
        h = fold_value(h, get(nj, "trade"));
        h = fold_value(h, get(nj, "px"));
        jo = pico8_cg("jo");
        printf("[dk] bo[%d]: %d planet(s)\n", i, jo->count);
        h = fnv(h, (uint32_t)jo->count);
        for (j = 0; j < jo->count; j++) {
            h = fold_vec(h, jo->items[j].as.ref, "ra");
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

static void *build(const char *spec, const pico8_ob_table_t *extra) {
    pico8_ob_table_t *b = pico8_ob(spec, extra);
    void *made = pico8_js_new(b);
    pico8_ob_table_free(b);
    return made;
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
    b = build("\nra=v(3,4),\nqf=v(40,-20),\nlq=o(\"go\"),\n", NULL);
    first = get_ref(b, "ra");
    first_at = *first;
    while (steps < 40) {
        pico8_vec_t before = vec_of(b, "ra");
        (void)call(b, "ph", nil(), nil());
        steps++;
        h = fold_vec(h, b, "ra");
        if (vec_of(b, "ra").x == before.x && vec_of(b, "ra").y == before.y) {
            break;
        }
    }
    assert(first->x == first_at.x && first->y == first_at.y);
    assert(vec_of(b, "ra").x == pico8_fixed_from_int(40) && vec_of(b, "ra").y == pico8_fixed_from_int(-20));
    printf("[ph] %d step(s)\n", steps);
    h = fnv(h, (uint32_t)steps);
    check("ph", h, PH_HASH);
}

static int my_calls;
static void *my_self;

static pico8_value_t counting_my(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    my_calls++;
    my_self = self;
    return pico8_value_fixed(pico8_fixed_from_int(7));
}

static void test_ldown(void) {
    uint32_t h = 2166136261u;
    pico8_ob_table_t *extra = pico8_ob_table_new();
    pico8_value_t r;
    void *with_my;
    void *with_false;

    start_game();
    pico8_ob_table_set_name(extra, "my", pico8_value_function(counting_my));
    with_my = build("\nlq=o(\"a\"),\n", extra);
    pico8_ob_table_set_name(extra, "my", pico8_value_bool(0));
    with_false = build("\nlq=o(\"b\"),\n", extra);
    pico8_ob_table_free(extra);

    r = call(with_my, "ldown", nil(), nil());
    assert(r.kind == PICO8_VALUE_BOOL && r.as.boolean);
    assert(my_calls == 1 && my_self == with_my);
    r = call(with_false, "ldown", nil(), nil());
    assert(r.kind == PICO8_VALUE_BOOL && r.as.boolean);
    assert(my_calls == 1);
    assert(bool_state(with_false, "my") == 1);
    h = fnv(h, (uint32_t)my_calls);
    h = fold_button(h, with_my);
    h = fold_button(h, with_false);
    check("ldown", h, LDOWN_HASH);
}

static uint32_t lq_first(const void *b) {
    const void *lq = get_ref(b, "lq");
    pico8_value_t v;
    assert(row_count(lq) > 0);
    v = row_item(lq, 0);
    assert(v.kind == PICO8_VALUE_FIXED);
    return (uint32_t)pico8_fixed_flr_int(v.as.fixed);
}

/* `ix[2].hb` rebuilds its button's `lq`: 24 when a tech is affordable. */
static void test_hb(void) {
    uint32_t h = 2166136261u;
    void *ix[4];
    pico8_list_t cb;
    void *before;
    int affordable;
    int i;

    start_game();
    assert(ix_buttons(ix) == 4);
    for (i = 0; i < 4; i++) {
        pico8_value_t out;
        if (i != 1) {
            assert(!pico8_mv(ix[i], "hb", 0, nil(), nil(), &out));
        }
    }
    before = get_ref(ix[1], "lq");
    (void)call(ix[1], "hb", nil(), nil());
    cb = pico8_cb(nil());
    affordable = cb.count > 0;
    pico8_list_free(&cb);
    assert(get_ref(ix[1], "lq") != before);
    assert(lq_first(ix[1]) == (affordable ? 24u : 37u));
    h = fnv(h, lq_first(ix[1]));
    h = fold_value(h, get(get_ref(ix[1], "lq"), "w"));
    h = fold_value(h, get(ix[1], "mb"));
    check("hb", h, HB_HASH);
}

/* The `my` of a `jo:fe` row builds on the planet it captured. */
static void test_fe_my(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *le;
    int i;

    known_panel();
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        void *b = le->items[i].as.ref;
        void *pt;
        if (!has(b, "_jo_fe_pt")) {
            continue;
        }
        pt = get_ref(b, "_jo_fe_pt");
        h = fold_value(h, get(pt, "me"));
        h = fnv(h, (uint32_t)pico8_hv()->count);
        (void)call(b, "my", nil(), nil());
        printf("[fe my] row %d: planet me=%s, %d entities\n", i, is_string(pt, "me", "ly") ? "ly" : "not ly", pico8_hv()->count);
        h = fold_value(h, get(pt, "me"));
        h = fnv(h, (uint32_t)pico8_hv()->count);
        break;
    }
    assert(i < le->count);
    check("fe my", h, FE_MY_HASH);
}

/* The `my` of a `cr` row invents the tech it captured. */
static void test_cr_my(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *le;
    const pico8_list_t *invented;
    int i;

    tech_screen();
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        if (bool_state(le->items[i].as.ref, "my") == 3) {
            break;
        }
    }
    assert(i < le->count);
    invented = get_ref(pico8_fm(), "invented");
    h = fnv(h, (uint32_t)invented->count);
    (void)call(le->items[i].as.ref, "my", nil(), nil());
    invented = get_ref(pico8_fm(), "invented");
    h = fnv(h, (uint32_t)invented->count);
    assert(invented->count >= 1);
    h = fold_value(h, invented->items[invented->count - 1]);
    printf("[cr my] row %d invented %s\n", i, invented->items[invented->count - 1].as.string);
    check("cr my", h, CR_MY_HASH);
}

static uint32_t draw_alone(void *b) {
    pico8_camera(&gfx, 0, 0);
    pico8_cls(&mem);
    (void)call(b, "kj", get(b, "ra"), nil());
    (void)call(b, "ev", nil(), nil());
    return screen_hash();
}

static const pico8_ob_table_t *dr_row(int i) {
    pico8_value_t row;
    assert(pico8_ob_table_get_index(pico8_dr(), i + 1, &row) && row.kind == PICO8_VALUE_REF);
    return row.as.ref;
}

/* One button at a time on a cleared screen: `my` absent, present, hovered,
 * with a price `h_`, a tooltip `jb` shown only when hovered, and an `h_`
 * left `false` (a free building of `jo:fe`). */
static void test_draw(void) {
    uint32_t h = 2166136261u;
    pico8_ob_table_t *extra = pico8_ob_table_new();
    void *plain;
    void *with_my;
    void *priced;
    void *tooltip;
    void *free_;
    uint32_t px[7];
    size_t i;

    start_game();
    plain = build("\nra=v(10,20),\nlq=o(\"plain\"),\n", NULL);
    pico8_ob_table_set_name(extra, "my", pico8_value_function(counting_my));
    with_my = build("\nra=v(10,20),\nlq=o(\"plain\"),\n", extra);
    priced = build("\nra=v(10,20),\nlq=o(\"plain\"),\nh_=o(\"12$\",c=13),\n", extra);
    pico8_ob_table_set_name(extra, "h_", pico8_value_bool(0));
    free_ = build("\nra=v(10,20),\nlq=o(\"plain\"),\n", extra);
    pico8_ob_table_free(extra);
    extra = pico8_ob_table_new();
    pico8_ob_table_set_name(extra, "jb", pico8_value_ref((void *)dr_row(0)));
    tooltip = build("\nra=v(10,20),\nlq=o(\"plain\"),\n", extra);
    pico8_ob_table_free(extra);

    px[0] = draw_alone(plain);
    px[1] = draw_alone(with_my);
    pico8_entity_set_field(with_my, "m_", pico8_value_bool(1));
    px[2] = draw_alone(with_my);
    px[3] = draw_alone(priced);
    px[4] = draw_alone(tooltip);
    pico8_entity_set_field(tooltip, "m_", pico8_value_bool(1));
    px[5] = draw_alone(tooltip);
    px[6] = draw_alone(free_);
    assert(bool_state(free_, "h_") == 1 && px[6] == px[1]);
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
    pico8_ob_table_t *b = pico8_ob_table_new();
    void *nil_keys;
    void *no_keys;

    start_game();
    pico8_ob_table_set_name(b, "lq", pico8_value_ref(NULL));
    no_keys = pico8_js_new(b);
    pico8_ob_table_set_name(b, "h_", pico8_value_ref(NULL));
    pico8_ob_table_set_name(b, "jb", pico8_value_ref(NULL));
    nil_keys = pico8_js_new(b);
    pico8_ob_table_free(b);
    assert(!has(no_keys, "h_") && !has(no_keys, "jb"));
    assert(is_lq(get_ref(nil_keys, "h_")));
    assert(must_fixed(get_ref(nil_keys, "h_"), "w") == 0);
    assert(pico8_type_tag_of(get_ref(nil_keys, "jb")) == PICO8_TYPE_TAG_STRUCT);
}

static int is_a_js_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_js_struct_class();
}

/* Every entity with an `mb`, which only a button has, is a `js` struct;
 * returns how many. */
static int all_buttons_are_structs(void) {
    const pico8_list_t *hv = pico8_hv();
    int n = 0;
    int i;
    for (i = 0; i < hv->count; i++) {
        if (has(hv->items[i].as.ref, "mb")) {
            assert(is_a_js_struct(hv->items[i].as.ref));
            n++;
        }
    }
    return n;
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

static void write_ra(void *e) {
    pico8_entity_set_field(e, "ra", pico8_value_ref(NULL));
}

static void write_my(void *e) {
    pico8_entity_set_field(e, "my", pico8_value_bool(0));
}

static void write_mb(void *e) {
    pico8_entity_set_field(e, "mb", pico8_value_fixed(0));
}

static void build_with_unknown_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "zz", pico8_value_fixed(pico8_fixed_from_int(1)));
    (void)pico8_js_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_js_new(b);
}

static void build_with_mb(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "mb", pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_js_new(b);
}

/* Built by any producer, a `js` is a struct with the five methods of its
 * descriptor; a generic write of a name other than `m_`/`lq`, or a
 * construction key outside the measured ones, stops the run. */
static void test_js_is_a_struct(void) {
    void *ix[4];
    void *nj;
    int i;

    start_title();
    check_int("struct title", all_buttons_are_structs(), 4);
    nj = title_button(2);
    pico8_eu(nj);
    assert(is_a_js_struct(nj) && pico8_dk() == nj);
    assert(ix_buttons(ix) == 4);
    check_int("struct eu", all_buttons_are_structs(), 4);
    for (i = 0; i < 4; i++) {
        pico8_value_t out;
        assert(pico8_mv(ix[i], "hb", 0, nil(), nil(), &out) == (i == 1));
    }
    (void)call(ix[3], "my", nil(), nil());
    check_int("struct pause", all_buttons_are_structs(), 8);
    pico8_cr();
    check_int("struct research", all_buttons_are_structs(), 9);

    pico8_entity_set_field(ix[0], "m_", pico8_value_bool(1));
    assert(bool_state(ix[0], "m_") == 2);
    pico8_entity_set_field(ix[0], "lq", get(ix[1], "lq"));
    assert(get_ref(ix[0], "lq") == get_ref(ix[1], "lq"));
    must_abort(write_ra, ix[0]);
    must_abort(write_my, ix[0]);
    must_abort(write_mb, ix[0]);
    must_abort(build_with_unknown_key, NULL);
    must_abort(build_with_positional_key, NULL);
    must_abort(build_with_mb, NULL);
    {
        pico8_ob_table_t *empty = pico8_ob_table_new();
        void *bare = pico8_js_new(empty);
        pico8_ob_table_free(empty);
        assert(is_a_js_struct(bare) && must_fixed(bare, "fa") == pico8_fixed_from_int(11) && !has(bare, "my"));
    }
}

static const char *const js_names[] = {"fa", "lq", "mb", "kq", "h_", "jb", "ra", "qf", "my", "m_",
                                       "hb", "fg", "px", "trade", "en", "_cr_t", "_jo_fe_pt", "_jo_fe_b", "_jo_fe_od"};

static pico8_value_t zero_union(pico8_value_kind_t kind) {
    pico8_value_t v;

    memset(&v, 0, sizeof v);
    v.kind = kind;
    return v;
}

static pico8_value_t vec_ref(int x, int y) {
    pico8_vec_t *p = malloc(sizeof *p);

    *p = pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
    return pico8_value_ref(p);
}

/* The raw reader and the name reader agree; absent leaves both witnesses
 * untouched. */
static int read_both(const void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(0x1234);
    pico8_value_t named = pico8_value_fixed(0x1234);
    int got_raw = pico8_js_struct_class()->field(e, name, &raw);
    int got_named = pico8_entity_get_field(e, name, &named);

    assert(got_raw == got_named);
    if (!got_raw) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        assert(named.kind == PICO8_VALUE_FIXED && named.as.fixed == 0x1234);
        return 0;
    }
    assert(pico8_value_equal(raw, named));
    *out = raw;
    return 1;
}

static void assert_same(const void *e, const char *name, pico8_value_t want) {
    pico8_value_t v;

    assert(read_both(e, name, &v) && pico8_value_equal(v, want));
}

static void assert_absent(const void *e, const char *name) {
    pico8_value_t v;

    assert(!read_both(e, name, &v));
}

static void *read_ref(const void *e, const char *name) {
    pico8_value_t v;

    assert(read_both(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

static int is_of_class(const void *ref, const pico8_struct_class_t *cls) {
    return pico8_type_tag_of(ref) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)ref)->cls == cls;
}

/* The 19 names by both readers: presence, kind, value; rows, vectors and
 * the box by content, structs by class, a function by first sight. */
static uint32_t fold_js_members(uint32_t h, const void *e) {
    size_t i;

    for (i = 0; i < sizeof js_names / sizeof js_names[0]; i++) {
        const char *name = js_names[i];
        pico8_value_t v;

        if (!read_both(e, name, &v)) {
            h = fnv(h, 0u);
            continue;
        }
        h = fnv(fnv(h, 1u), (uint32_t)v.kind);
        if (v.kind == PICO8_VALUE_FUNCTION) {
            assert(v.as.function != NULL);
        }
        if (v.kind == PICO8_VALUE_STRING) {
            assert(v.as.string != NULL);
        }
        if (v.kind != PICO8_VALUE_REF) {
            h = fold_value(h, v);
            continue;
        }
        assert(v.as.ref != NULL);
        if (is_lq(v.as.ref)) {
            h = fold_lq(fnv(h, 0xA2u), v.as.ref);
        } else if (strcmp(name, "kq") == 0) {
            h = fold_kq(h, e);
        } else if (strcmp(name, "ra") == 0 || strcmp(name, "qf") == 0) {
            assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN);
            h = fold_vec(h, e, name);
        } else if (is_of_class(v.as.ref, pico8_jb_struct_class())) {
            h = fnv(h, 0xB1u);
        } else if (is_of_class(v.as.ref, pico8_jo_struct_class())) {
            h = fnv(h, 0xB2u);
        } else if (is_of_class(v.as.ref, pico8_hp_struct_class())) {
            h = fnv(h, 0xB3u);
        } else if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE) {
            pico8_value_t nk;
            assert(pico8_ob_table_get_name(v.as.ref, "nk", &nk));
            h = fold_value(fnv(h, 0xB4u), nk);
        } else {
            h = fnv(fnv(h, 0xB5u), (uint32_t)pico8_type_tag_of(v.as.ref));
        }
    }
    return h;
}

static uint32_t fold_le_members(uint32_t h) {
    const pico8_list_t *le = ok_le();
    int i;

    assert(le != NULL);
    h = fnv(h, (uint32_t)le->count);
    for (i = 0; i < le->count; i++) {
        h = fold_js_members(h, le->items[i].as.ref);
    }
    return h;
}

/* One `pz` call: one `ra` for all, a `qf` each. */
static void check_one_pz(void *const *b, int n) {
    int i;
    int j;

    for (i = 0; i < n; i++) {
        assert(read_ref(b[i], "ra") == read_ref(b[0], "ra"));
        for (j = 0; j < i; j++) {
            assert(read_ref(b[i], "qf") != read_ref(b[j], "qf"));
        }
    }
}

static void check_le_one_pz(void) {
    const pico8_list_t *le = ok_le();
    void *b[32];
    int i;

    assert(le != NULL && le->count <= 32);
    for (i = 0; i < le->count; i++) {
        b[i] = le->items[i].as.ref;
    }
    check_one_pz(b, le->count);
}

static void write_named(void *e, const char *name, pico8_value_t v) {
    assert(pico8_js_struct_class()->set_field(e, name, v) == 1);
    pico8_entity_set_field(e, name, v);
}

/* `m_` takes true, false (a zeroed union too) and nil; `lq` a row or nil.
 * The 17 other names and any stranger are no writer's: the raw writer
 * returns 0 and touches nothing. */
static uint32_t fold_written_by_name(uint32_t h, void *e, void *other) {
    static const char *const unwritten[] = {"", "M_", "m_ ", "lq ", "has_m_", "has_h_"};
    void *kq_before = read_ref(e, "kq");
    pico8_value_t mb;
    void *row;
    size_t i;

    assert(read_both(e, "mb", &mb));
    write_named(e, "m_", pico8_value_bool(1));
    assert_same(e, "m_", pico8_value_bool(1));
    h = fold_js_members(h, e);
    write_named(e, "m_", pico8_value_bool(0));
    assert_same(e, "m_", pico8_value_bool(0));
    h = fold_js_members(h, e);
    write_named(e, "m_", pico8_value_bool(1));
    write_named(e, "m_", zero_union(PICO8_VALUE_BOOL));
    assert_same(e, "m_", pico8_value_bool(0));
    h = fold_js_members(h, e);
    write_named(e, "m_", pico8_value_bool(1));
    write_named(e, "m_", pico8_value_ref(NULL));
    assert_absent(e, "m_");
    write_named(e, "m_", pico8_value_bool(0));
    write_named(e, "m_", pico8_value_ref(NULL));
    assert_absent(e, "m_");
    write_named(e, "m_", pico8_value_ref(NULL));
    assert_absent(e, "m_");
    h = fold_js_members(h, e);

    row = read_ref(other, "lq");
    write_named(e, "lq", pico8_value_ref(row));
    assert(read_ref(e, "lq") == row);
    h = fold_js_members(h, e);
    row = pico8_lq_new(NULL);
    write_named(e, "lq", pico8_value_ref(row));
    assert(read_ref(e, "lq") == row);
    h = fold_js_members(h, e);
    write_named(e, "lq", pico8_value_ref(NULL));
    assert_absent(e, "lq");
    assert(read_ref(e, "kq") == kq_before);
    assert_same(e, "mb", mb);
    h = fold_js_members(h, e);
    write_named(e, "lq", pico8_value_ref(row));

    write_named(e, "m_", pico8_value_bool(1));
    for (i = 0; i < sizeof js_names / sizeof js_names[0]; i++) {
        uint32_t before = fold_js_members(2166136261u, e);
        int got;

        if (strcmp(js_names[i], "m_") == 0 || strcmp(js_names[i], "lq") == 0) {
            continue;
        }
        got = pico8_js_struct_class()->set_field(e, js_names[i], pico8_value_fixed(pico8_fixed_from_int(9)));
        assert(got == 0);
        assert(fold_js_members(2166136261u, e) == before);
        h = fnv(h, (uint32_t)got);
    }
    for (i = 0; i < sizeof unwritten / sizeof unwritten[0]; i++) {
        uint32_t before = fold_js_members(2166136261u, e);
        int got = pico8_js_struct_class()->set_field(e, unwritten[i], pico8_value_fixed(pico8_fixed_from_int(9)));
        assert(got == 0);
        assert(fold_js_members(2166136261u, e) == before);
        h = fnv(h, (uint32_t)got);
    }
    assert_same(e, "m_", pico8_value_bool(1));
    return fold_js_members(h, e);
}

static void *built_with(const char *name, pico8_value_t v) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    void *made;

    pico8_ob_table_set_name(b, name, v);
    made = pico8_js_new(b);
    pico8_ob_table_free(b);
    return made;
}

/* Every construction key at nil, the fixed ones and `my` at a zeroed
 * union, `h_` false, TROU-23, a row and a list as `lq(x)`'s argument. */
static uint32_t fold_keys(uint32_t h, void *row) {
    static const char *const nil_absent[] = {"fg", "px", "trade", "en", "_jo_fe_od", "ra",
                                             "qf", "hb", "my", "_cr_t", "_jo_fe_pt", "_jo_fe_b"};
    static const char *const fixed[] = {"fg", "trade", "en", "_jo_fe_od"};
    pico8_list_t *list = malloc(sizeof *list);
    void *b;
    size_t i;

    for (i = 0; i < sizeof nil_absent / sizeof nil_absent[0]; i++) {
        b = built_with(nil_absent[i], pico8_value_ref(NULL));
        assert_absent(b, nil_absent[i]);
        h = fold_js_members(h, b);
    }
    for (i = 0; i < sizeof fixed / sizeof fixed[0]; i++) {
        b = built_with(fixed[i], zero_union(PICO8_VALUE_FIXED));
        assert_same(b, fixed[i], pico8_value_fixed(0));
        h = fold_js_members(h, b);
    }
    b = built_with("my", pico8_value_bool(0));
    assert_same(b, "my", pico8_value_bool(0));
    h = fold_js_members(h, b);
    b = built_with("my", zero_union(PICO8_VALUE_BOOL));
    assert_same(b, "my", pico8_value_bool(0));
    h = fold_js_members(h, b);
    b = built_with("h_", pico8_value_bool(0));
    assert_same(b, "h_", pico8_value_bool(0));
    h = fold_js_members(h, b);

    b = built_with("h_", pico8_value_ref(NULL));
    assert(is_lq(read_ref(b, "h_")) && must_fixed(read_ref(b, "h_"), "w") == 0);
    h = fold_js_members(h, b);
    b = built_with("jb", pico8_value_ref(NULL));
    assert(is_of_class(read_ref(b, "jb"), pico8_jb_struct_class()));
    h = fold_js_members(h, b);
    b = built_with("lq", pico8_value_ref(NULL));
    assert(is_lq(read_ref(b, "lq")));
    h = fold_js_members(h, b);

    b = built_with("h_", pico8_value_ref(row));
    assert(is_lq(read_ref(b, "h_")) && read_ref(b, "h_") != row);
    h = fold_js_members(h, b);

    pico8_list_init(list);
    pico8_list_add(list, pico8_value_string("from a list"));
    pico8_list_add(list, pico8_value_fixed(pico8_fixed_from_int(3)));
    b = built_with("lq", pico8_value_ref(list));
    assert(is_lq(read_ref(b, "lq")) && row_count(read_ref(b, "lq")) == 2);
    return fold_js_members(h, b);
}

static uint32_t members_title_and_game(void) {
    static const char *const strangers[] = {"",       "l",      "lq ",    "LQ",     "h__",
                                            "has_h_", "has_my", "_jo_fe", "tradex", "en_"};
    pico8_ob_table_t *extra;
    uint32_t h = 2166136261u;
    const pico8_list_t *hd;
    void *title[4];
    void *ix[4];
    void *fresh[3];
    void *nj;
    void *plain;
    void *my_false;
    pico8_value_t ca;
    pico8_value_t px;
    int n = 0;
    int i;
    size_t k;

    start_title();
    hd = pico8_cv("hd");
    for (i = 0; i < hd->count; i++) {
        void *e = hd->items[i].as.ref;
        if (has(e, "mb")) {
            assert(n < 4);
            title[n++] = e;
            assert(read_both(e, "px", &px) && px.kind == PICO8_VALUE_STRING);
            assert(pico8_fv_default("ca", &ca));
            assert_same(e, "fa", ca);
            h = fold_js_members(h, e);
        }
    }
    check_int("members title", n, 4);
    check_one_pz(title, 4);

    nj = title[1];
    assert_same(nj, "my", pico8_value_function(pico8_eu_field_value().as.function));
    printf("[members] dk: trade %d, en %d/65536\n", pico8_fixed_flr_int(must_fixed(nj, "trade")),
           (int)must_fixed(nj, "en"));
    h = fold_js_members(h, nj);
    pico8_eu(nj);
    assert(pico8_dk() == nj);
    assert(ix_buttons(ix) == 4);
    for (i = 0; i < 4; i++) {
        assert(has(ix[i], "hb") == (i == 1));
        h = fold_js_members(h, ix[i]);
    }

    fresh[0] = pico8_js_new(NULL);
    extra = pico8_ob_table_new();
    fresh[1] = pico8_js_new(extra);
    pico8_ob_table_free(extra);
    for (k = 0; k < 2; k++) {
        const void *lq = read_ref(fresh[k], "lq");
        assert(is_lq(lq) && row_count(lq) == 0);
        assert_same(fresh[k], "fa", pico8_value_fixed(pico8_fixed_from_int(11)));
        assert_same(fresh[k], "mb", pico8_value_fixed(pico8_fixed_add(must_fixed(lq, "w"), pico8_fixed_from_int(2))));
        (void)read_ref(fresh[k], "kq");
        for (i = 0; i < (int)(sizeof js_names / sizeof js_names[0]); i++) {
            if (strcmp(js_names[i], "fa") != 0 && strcmp(js_names[i], "lq") != 0 && strcmp(js_names[i], "mb") != 0 &&
                strcmp(js_names[i], "kq") != 0) {
                assert_absent(fresh[k], js_names[i]);
            }
        }
        h = fold_js_members(h, fresh[k]);
    }

    (void)call(ix[3], "my", nil(), nil());
    check_le_one_pz();
    printf("[members] ha: %d row(s), fg on the 4th: %d\n", ok_le()->count, has(ok_le()->items[3].as.ref, "fg"));
    h = fold_le_members(h);

    fresh[2] = pico8_js_new(NULL);
    h = fold_written_by_name(h, ix[0], ix[1]);
    h = fold_written_by_name(h, fresh[2], ix[2]);
    h = fold_keys(h, read_ref(ix[2], "lq"));

    plain = build("\nra=v(10,20),\nlq=o(\"plain\"),\n", NULL);
    extra = pico8_ob_table_new();
    pico8_ob_table_set_name(extra, "my", pico8_value_bool(0));
    my_false = build("\nra=v(10,20),\nlq=o(\"plain\"),\n", extra);
    pico8_ob_table_free(extra);
    assert(draw_alone(my_false) == draw_alone(plain));
    h = fnv(h, draw_alone(my_false));

    for (k = 0; k < sizeof strangers / sizeof strangers[0]; k++) {
        assert(!read_both(ix[0], strangers[k], &(pico8_value_t){0}));
        assert(!read_both(fresh[2], strangers[k], &(pico8_value_t){0}));
        h = fnv(h, 0u);
    }
    return h;
}

static uint32_t members_research(void) {
    const pico8_ob_table_t *nb = pico8_nb();
    const pico8_list_t *le;
    uint32_t h = 2166136261u;
    int dear = 0;
    int affordable = 0;
    int i;

    for (i = 0; i < nb->count; i++) {
        pico8_value_t oy;
        assert(nb->entries[i].value.kind == PICO8_VALUE_REF);
        if (pico8_ob_table_get_name(nb->entries[i].value.as.ref, "oy", &oy)) {
            assert(oy.kind == PICO8_VALUE_REF && pico8_type_tag_of(oy.as.ref) == PICO8_TYPE_TAG_OB_TABLE);
        }
    }
    h = fnv(h, (uint32_t)nb->count);

    tech_screen();
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        void *b = le->items[i].as.ref;
        pico8_value_t my;

        if (!read_both(b, "my", &my)) {
            continue;
        }
        if (my.kind == PICO8_VALUE_BOOL) {
            assert(my.as.boolean == 0);
            assert_absent(b, "_cr_t");
            dear++;
        } else {
            const void *t = read_ref(b, "_cr_t");
            int j;
            assert(my.kind == PICO8_VALUE_FUNCTION);
            for (j = 0; j < nb->count && nb->entries[j].value.as.ref != t; j++) {
            }
            assert(j < nb->count);
            affordable++;
        }
        assert(is_lq(read_ref(b, "h_")));
    }
    printf("[members] cr: %d row(s), %d too dear, %d affordable\n", le->count, dear, affordable);
    assert(dear >= 1 && affordable >= 1);
    return fold_le_members(h);
}

static uint32_t members_fe(void) {
    const pico8_list_t *le;
    const void *pt = NULL;
    uint32_t h = 2166136261u;
    int rows = 0;
    int free_rows = 0;
    int i;

    known_panel();
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        void *b = le->items[i].as.ref;
        pico8_value_t od;

        if (!has(b, "_jo_fe_pt")) {
            continue;
        }
        if (pt == NULL) {
            pt = read_ref(b, "_jo_fe_pt");
        }
        assert(read_ref(b, "_jo_fe_pt") == pt);
        assert(is_of_class(pt, pico8_jo_struct_class()) && listed(pico8_cg("jo"), pt));
        assert(is_of_class(read_ref(b, "_jo_fe_b"), pico8_hp_struct_class()));
        assert(read_both(b, "_jo_fe_od", &od) && od.kind == PICO8_VALUE_FIXED);
        assert(read_both(b, "my", &(pico8_value_t){0}));
        assert(is_of_class(read_ref(b, "jb"), pico8_jb_struct_class()));
        if (od.as.fixed > 0) {
            assert(is_lq(read_ref(b, "h_")));
        } else {
            assert_same(b, "h_", pico8_value_bool(0));
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
    const pico8_list_t *hd;
    int n = 0;
    int i;

    game_end();
    hd = pico8_cv("hd");
    for (i = 0; i < hd->count; i++) {
        void *e = hd->items[i].as.ref;
        pico8_value_t fa;
        if (lookup(e, "fa", &fa) && fa.kind == PICO8_VALUE_FIXED && fa.as.fixed == pico8_fixed_from_int(15) &&
            has(e, "lq")) {
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
        PICO8_MEMBER_CENSUS_CHILD_EXIT(0);
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

static pico8_value_t g_off_row;
static const char *g_off_name;

static void write_off_row(void *e) {
    pico8_entity_set_field(e, g_off_name, g_off_row);
}

static void build_off_row(void *e) {
    (void)e;
    (void)built_with(g_off_name, g_off_row);
}

static void stops_on(void (*write)(void *), void *e, const char *name, const pico8_value_t *off, size_t n) {
    size_t i;

    g_off_name = name;
    for (i = 0; i < n; i++) {
        g_off_row = off[i];
        must_abort(write, e);
    }
}

/* A value off a name's census row stops the run, a zeroed union included:
 * the writer's `m_` and `lq`, every construction key but `lq` and `h_`,
 * and `jb` false. */
static void test_js_writer_stops_off_row(void) {
    static const char *const fixed_keys[] = {"fg", "trade", "en", "_jo_fe_od"};
    const pico8_ob_table_t *gb;
    pico8_value_t table;
    pico8_value_t vec;
    pico8_value_t list;
    pico8_value_t js;
    pico8_value_t hp = pico8_value_ref(NULL);
    void *ix[4];
    size_t i;
    int j;

    start_game();
    assert(ix_buttons(ix) == 4);
    table = pico8_value_ref(pico8_ob_table_new());
    vec = vec_ref(1, 1);
    list = pico8_value_ref((void *)pico8_hv());
    js = pico8_value_ref(ix[1]);
    gb = pico8_gb();
    for (j = 0; j < gb->count && hp.as.ref == NULL; j++) {
        if (gb->entries[j].value.kind == PICO8_VALUE_REF &&
            is_of_class(gb->entries[j].value.as.ref, pico8_hp_struct_class())) {
            hp = gb->entries[j].value;
        }
    }
    assert(hp.as.ref != NULL);
    {
        const pico8_value_t off[] = {pico8_value_fixed(pico8_fixed_from_int(1)), zero_union(PICO8_VALUE_FIXED),
                                     pico8_value_string("x"), zero_union(PICO8_VALUE_STRING),
                                     zero_union(PICO8_VALUE_FUNCTION), table, vec, list, js};
        stops_on(write_off_row, ix[0], "m_", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_fixed(pico8_fixed_from_int(1)), zero_union(PICO8_VALUE_FIXED),
                                     pico8_value_bool(1), zero_union(PICO8_VALUE_BOOL), pico8_value_string("x"),
                                     zero_union(PICO8_VALUE_STRING), zero_union(PICO8_VALUE_FUNCTION), table, list,
                                     vec, pico8_value_ref(pico8_jb_new(NULL)), js};
        stops_on(write_off_row, ix[0], "lq", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_ref(NULL), pico8_value_bool(1), zero_union(PICO8_VALUE_BOOL),
                                     pico8_value_string("x"), zero_union(PICO8_VALUE_STRING),
                                     zero_union(PICO8_VALUE_FUNCTION), table};
        stops_on(build_off_row, NULL, "fa", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_fixed(pico8_fixed_from_int(1)), zero_union(PICO8_VALUE_FIXED),
                                     pico8_value_bool(1), zero_union(PICO8_VALUE_BOOL), pico8_value_string("x"),
                                     zero_union(PICO8_VALUE_STRING), zero_union(PICO8_VALUE_FUNCTION), table, list,
                                     js};
        stops_on(build_off_row, NULL, "ra", off, sizeof off / sizeof off[0]);
        stops_on(build_off_row, NULL, "qf", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_bool(1), pico8_value_fixed(pico8_fixed_from_int(1)),
                                     zero_union(PICO8_VALUE_FIXED), pico8_value_string("x"),
                                     zero_union(PICO8_VALUE_STRING), zero_union(PICO8_VALUE_FUNCTION), table};
        stops_on(build_off_row, NULL, "my", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_bool(1), pico8_value_bool(0), zero_union(PICO8_VALUE_FIXED),
                                     pico8_value_string("x"), zero_union(PICO8_VALUE_FUNCTION), table};
        stops_on(build_off_row, NULL, "hb", off, sizeof off / sizeof off[0]);
    }
    for (i = 0; i < sizeof fixed_keys / sizeof fixed_keys[0]; i++) {
        const pico8_value_t off[] = {pico8_value_bool(1), zero_union(PICO8_VALUE_BOOL), pico8_value_string("x"),
                                     zero_union(PICO8_VALUE_STRING), zero_union(PICO8_VALUE_FUNCTION), table};
        stops_on(build_off_row, NULL, fixed_keys[i], off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_fixed(pico8_fixed_from_int(1)), zero_union(PICO8_VALUE_FIXED),
                                     pico8_value_bool(1), zero_union(PICO8_VALUE_BOOL), zero_union(PICO8_VALUE_STRING),
                                     zero_union(PICO8_VALUE_FUNCTION), table};
        stops_on(build_off_row, NULL, "px", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_fixed(pico8_fixed_from_int(1)), zero_union(PICO8_VALUE_FIXED),
                                     pico8_value_bool(1), zero_union(PICO8_VALUE_BOOL), pico8_value_string("x"),
                                     zero_union(PICO8_VALUE_STRING), zero_union(PICO8_VALUE_FUNCTION), list, vec,
                                     js};
        stops_on(build_off_row, NULL, "_cr_t", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_fixed(pico8_fixed_from_int(1)), zero_union(PICO8_VALUE_BOOL),
                                     pico8_value_string("x"), table, vec, list,
                                     pico8_value_ref(pico8_qh_new(NULL)), hp, js};
        stops_on(build_off_row, NULL, "_jo_fe_pt", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_fixed(pico8_fixed_from_int(1)), zero_union(PICO8_VALUE_BOOL),
                                     pico8_value_string("x"), table, vec, pico8_value_ref(pico8_jo_new(NULL)), js};
        stops_on(build_off_row, NULL, "_jo_fe_b", off, sizeof off / sizeof off[0]);
    }
    {
        const pico8_value_t off[] = {pico8_value_bool(0), zero_union(PICO8_VALUE_BOOL)};
        stops_on(build_off_row, NULL, "jb", off, sizeof off / sizeof off[0]);
    }
}

static void write_hi(void *e) {
    pico8_entity_set_field(e, "hi", pico8_value_ref(NULL));
}

static void write_ca(void *e) {
    pico8_entity_set_field(e, "ca", pico8_value_fixed(0));
}

static void write_ep(void *e) {
    pico8_entity_set_field(e, "ep", pico8_value_string("x"));
}

static void same_as_ha(const void *o, const char *name) {
    pico8_value_t in_table;
    pico8_value_t in_struct;
    assert(pico8_ob_table_get_name(pico8_ha(), name, &in_table));
    assert(lookup(o, name, &in_struct));
    assert(in_struct.kind == in_table.kind);
    if (in_table.kind == PICO8_VALUE_REF) {
        assert(in_table.as.ref != NULL && in_struct.as.ref == in_table.as.ref);
    } else {
        assert(in_table.kind == PICO8_VALUE_FIXED && in_struct.as.fixed == in_table.as.fixed);
    }
}

/* What `ix[4].my` hands `pz` as `p`: a struct without method or writer
 * whose `hi`/`gq`/`ca` are `ha`'s own, for as long as `ha` lives. */
static void test_ha_pz_is_a_struct(void) {
    static const char *const absent[] = {"ep", "eo", "ra", "ff", "fo", "nu", "my"};
    const pico8_struct_header_t *o;
    pico8_value_t v;
    size_t i;

    start_game();
    o = pico8_ha_pz();
    assert(o != NULL && o->tag == PICO8_TYPE_TAG_STRUCT && o->cls == pico8_ha_pz_struct_class());
    for (i = 0; i < PICO8_METHOD_COUNT; i++) {
        assert(o->cls->methods[i] == NULL);
    }
    assert(o->cls->set_field == NULL);
    assert(!pico8_mv((void *)o, "ph", 0, nil(), nil(), &v));
    assert(!pico8_mv((void *)o, "my", 0, nil(), nil(), &v));
    same_as_ha(o, "hi");
    same_as_ha(o, "gq");
    same_as_ha(o, "ca");
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(o, absent[i]));
    }
    must_abort(write_hi, (void *)o);
    must_abort(write_ca, (void *)o);
    must_abort(write_ep, (void *)o);

    pico8_menu_shutdown();
    assert(pico8_ha_pz() == NULL);
    assert(!has(o, "hi") && !has(o, "gq") && !has(o, "ca"));
    pico8_menu_init();
    assert(pico8_ha_pz() == o);
    same_as_ha(o, "hi");
    same_as_ha(o, "gq");
    same_as_ha(o, "ca");
}

static void ha_pz_vec_as_ha(const void *o, const char *name, int x, int y) {
    pico8_value_t in_table;
    pico8_value_t out = pico8_value_fixed(0x1234);
    const pico8_vec_t *vec;
    assert(pico8_ob_table_get_name(pico8_ha(), name, &in_table));
    assert(pico8_entity_get_field((void *)o, name, &out) == 1);
    assert(out.kind == PICO8_VALUE_REF && out.as.ref != NULL && out.as.ref == in_table.as.ref);
    vec = out.as.ref;
    assert(vec->x == pico8_fixed_from_int(x) && vec->y == pico8_fixed_from_int(y));
}

static void ha_pz_ca_is_14(const void *o) {
    pico8_value_t out = pico8_value_fixed(0x1234);
    assert(pico8_entity_get_field((void *)o, "ca", &out) == 1);
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == pico8_fixed_from_int(14));
}

static void ha_pz_misses(const pico8_struct_header_t *o, const char *name) {
    pico8_value_t out = pico8_value_fixed(0x1234);
    assert(o->cls->field(o, name, &out) == 0);
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == 0x1234);
}

static void test_ha_pz_members_by_name(void) {
    const pico8_struct_header_t *o;

    start_game();
    o = pico8_ha_pz();
    ha_pz_vec_as_ha(o, "hi", 0, 107);
    ha_pz_vec_as_ha(o, "gq", 0, -11);
    ha_pz_ca_is_14(o);
    ha_pz_misses(o, "ra");
    ha_pz_misses(o, "ep");
    ha_pz_misses(o, "");

    pico8_menu_shutdown();
    ha_pz_misses(o, "hi");
    ha_pz_misses(o, "gq");
    ha_pz_misses(o, "ca");

    pico8_menu_init();
    ha_pz_vec_as_ha(o, "hi", 0, 107);
    ha_pz_vec_as_ha(o, "gq", 0, -11);
    ha_pz_ca_is_14(o);
}

static void pz_on_a_table(void *e) {
    pico8_list_t none;
    pico8_list_init(&none);
    pico8_pz(e, &none);
}

/* `pz` takes a struct or NULL: an ob table stops it. */
static void test_pz_closed(void) {
    pico8_ob_table_t *table = pico8_ob_table_new();
    pico8_list_t none;

    start_game();
    must_abort(pz_on_a_table, table);
    pico8_ob_table_free(table);
    pico8_list_init(&none);
    pico8_pz(NULL, &none);
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
        PICO8_MEMBER_CENSUS_CHILD_EXIT(0);
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
    in_fresh_child(test_js_is_a_struct);
    in_fresh_child(test_js_members_by_name);
    in_fresh_child(test_js_writer_stops_off_row);
    in_fresh_child(test_ha_pz_is_a_struct);
    in_fresh_child(test_ha_pz_members_by_name);
    in_fresh_child(test_pz_closed);
    in_fresh_child(test_panels_are_structs);
    in_fresh_child(test_pj_are_structs);

    printf("test_pico8_js: all tests passed\n");
    return 0;
}
