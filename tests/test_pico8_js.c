/* The button `js`, read and written only through the dual-form field API
 * and driven through the real `pz`/`eu`/`cr`/`ix`/`dg`/`dc`/`mv`. The
 * frozen values were taken on the code where `js` was still a class
 * instance and must not be re-taken. */

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
#include "../src/pico8_cheat.h"
#include "../src/pico8_class.h"
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
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
#include "../src/pico8_ob.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_tech_gate.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define TITLE_CLICK "tests/fixtures/title_click_reasonable.json"
#define PAUSE "tests/fixtures/pause_menu_music_events.json"
#define TECH "tests/fixtures/tech_screen_invent_events.json"
#define BUILT "tests/fixtures/built_planet_panel_events.json"
#define GAME_END "tests/fixtures/game_end_events.json"

/* built_planet_panel_events.json: the panel of the known "p1", then of the
 * built "p27" (see test_pico8_built_planet_panel_nil_row.c). */
#define KNOWN_PANEL_FRAME 245
#define BUILT_PANEL_FRAME 277

#define TITLE_HASH 0xB044B5F2u
#define EU_HASH 0x08EB17A6u
#define PAUSE_HASH 0xF715AAC0u
#define RESEARCH_HASH 0x48094C1Eu
#define FE_HASH 0xF7292196u
#define GH_HASH 0x8B1AE656u
#define PJ_HASH 0xC28C95AFu
#define HARNESS_HASH 0xD8AB9548u
#define DK_HASH 0x5178903Bu
#define TITLE_CLICK_FRAMES 340
#define TITLE_CLICK_HASH 0xDFB69B55u
#define PAUSE_FRAMES 32
#define PAUSE_REPLAY_HASH 0xEA3F2516u
#define TECH_FRAMES 58
#define TECH_REPLAY_HASH 0x3A948B77u
#define BUILT_FRAMES 278
#define BUILT_REPLAY_HASH 0xEBA9C7C6u
#define GAME_END_FRAMES 74
#define GAME_END_REPLAY_HASH 0x23547372u
#define PH_HASH 0xD2A611E7u
#define LDOWN_HASH 0x4621266Fu
#define HB_HASH 0x0FA6ACC7u
#define FE_MY_HASH 0x77F1CF92u
#define CR_MY_HASH 0x7ED641D9u
#define DRAW_HASH 0x92E72EEBu

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

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
            return fnv(fnv(h, 0xEEEEu), (uint32_t)PICO8_TYPE_TAG_INSTANCE);
        }
        return fnv(fnv(h, 0xEEEEu), (uint32_t)pico8_type_tag_of(v.as.ref));
    default:
        return fnv(h, 0xF0u);
    }
}

/* An `lq` instance by its width and its positional items. */
static uint32_t fold_lq(uint32_t h, const void *lq) {
    const pico8_ob_table_t *fields;
    int i;

    assert(pico8_type_tag_of(lq) == PICO8_TYPE_TAG_INSTANCE && ((const pico8_instance_t *)lq)->cls == pico8_lq_class());
    h = fold_value(h, get(lq, "w"));
    fields = ((const pico8_instance_t *)lq)->fields;
    for (i = 0; i < fields->count; i++) {
        if (fields->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
            pico8_value_t item = fields->entries[i].value;
            h = fnv(h, (uint32_t)fields->entries[i].key.index);
            h = item.kind == PICO8_VALUE_REF ? fnv(h, 0x12u) : fold_value(h, item);
        }
    }
    return h;
}

/* Absent, `false`, or what it holds: an `lq` instance, a `jb` struct. */
static uint32_t fold_built(uint32_t h, const void *b, const char *name) {
    pico8_value_t v;
    if (!lookup(b, name, &v)) {
        return fnv(h, 0xA0u);
    }
    if (v.kind != PICO8_VALUE_REF) {
        return fold_value(fnv(h, 0xA1u), v);
    }
    h = fnv(fnv(h, 0xA2u), (uint32_t)pico8_type_tag_of(v.as.ref));
    if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_INSTANCE) {
        h = fold_lq(h, v.as.ref);
    }
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

static const pico8_ob_table_t *bo_row(int i) {
    pico8_value_t row;
    assert(pico8_ob_table_get_index(pico8_bo(), i, &row) && row.kind == PICO8_VALUE_REF);
    return row.as.ref;
}

static void start_from_title(const char *events) {
    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_cheat_init(&cheat);
}

static void start_game(const char *events) {
    start_from_title(events);
    pico8_eu(pico8_js_new(bo_row(2)));
}

static void play_frame(void) {
    pico8_dispatch_set_input(&input);
    pico8_cheat_step(&cheat, &input);
    pico8__update60();
    pico8__draw();
    if (input.current_frame + 1 < input.frame_count) {
        pico8_input_step(&input);
    }
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

    start_from_title(TITLE_CLICK);
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

    start_game(PAUSE);
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

    start_game(PAUSE);
    assert(ix_buttons(ix) == 4);
    (void)call(ix[3], "my", nil(), nil());
    check_shared_ra();
    h = fold_le(h, "pause");
    check("pause", h, PAUSE_HASH);
}

static void invent(const char *rr) {
    pico8_value_t v;
    assert(pico8_fm() != NULL && pico8_instance_get_field(pico8_fm(), "invented", &v) &&
           v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    pico8_list_add((pico8_list_t *)v.as.ref, pico8_value_string(rr));
}

static void test_research(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *le;
    int unaffordable = 0;
    int i;

    start_game(TECH);
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

static void run_to(const char *events, int frame) {
    int i;
    start_game(events);
    for (i = 0; i <= frame; i++) {
        play_frame();
    }
}

static void test_fe(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *le;
    int with_pt = 0;
    int i;

    run_to(BUILT, KNOWN_PANEL_FRAME);
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
    run_to(BUILT, BUILT_PANEL_FRAME);
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

    run_to(GAME_END, GAME_END_FRAMES - 1);
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

/* The harness's `pico8_js_new(bo[2])`: a copy of a row `pz` already wrote
 * `ra`/`qf`/`fa` into. */
static void test_harness(void) {
    uint32_t h = 2166136261u;
    void *nj;

    start_from_title(PAUSE);
    nj = pico8_js_new(bo_row(2));
    assert(listed(pico8_hv(), nj) && !listed(ok_le(), nj));
    h = fold_button(h, nj);
    check("harness", h, HARNESS_HASH);
}

/* `dk` is the button `eu` got: its `en` sizes the galaxy. */
static void test_dk(void) {
    uint32_t h = 2166136261u;
    int i;

    start_from_title(PAUSE);
    for (i = 1; i <= 4; i++) {
        void *nj;
        const pico8_list_t *jo;
        int j;

        pico8__init();
        nj = pico8_js_new(bo_row(i));
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

static int is_js_like(const void *e) {
    pico8_value_t fa;
    return lookup(e, "fa", &fa) && fa.kind == PICO8_VALUE_FIXED && fa.as.fixed == pico8_fixed_from_int(11) &&
           has(e, "lq") && has(e, "mb");
}

static uint32_t fold_live(uint32_t h, const void *b) {
    h = fold_vec(h, b, "ra");
    h = fnv(h, bool_state(b, "m_"));
    h = fold_value(h, get(b, "t"));
    return fold_value(h, get(get_ref(b, "lq"), "w"));
}

/* Per frame: every button of `ok.le` and `cv.hb`, the hovered one, the
 * screen. */
static void replay(const char *label, const char *events, int from_title, int want_frames, uint32_t want_hash) {
    uint32_t h = 2166136261u;
    const void *last_le = NULL;
    int le_changes = 0;
    int frames;
    int i;

    if (from_title) {
        start_from_title(events);
    } else {
        start_game(events);
    }
    frames = input.frame_count;
    for (i = 0; i < frames; i++) {
        const pico8_list_t *le;
        const pico8_list_t *hb;
        int j;

        play_frame();
        le = ok_le();
        if (le != last_le) {
            le_changes++;
            last_le = le;
        }
        h = fnv(h, (uint32_t)(le == NULL ? 0 : le->count));
        for (j = 0; le != NULL && j < le->count; j++) {
            h = fold_live(h, le->items[j].as.ref);
        }
        hb = pico8_cv("hb");
        for (j = 0; hb != NULL && j < hb->count; j++) {
            if (is_js_like(hb->items[j].as.ref)) {
                h = fold_live(fnv(h, (uint32_t)j), hb->items[j].as.ref);
            }
        }
        h = fnv(h, (uint32_t)rank_in(le, get_ref(pico8_ok(), "o_")));
        h = fnv(h, screen_hash());
    }
    printf("[%s] ok.le replaced %d time(s)\n", label, le_changes);
    check_int(label, frames, want_frames);
    check(label, h, want_hash);
}

static void test_replay_title_click(void) {
    replay("title click", TITLE_CLICK, 1, TITLE_CLICK_FRAMES, TITLE_CLICK_HASH);
}

static void test_replay_pause(void) {
    replay("pause replay", PAUSE, 0, PAUSE_FRAMES, PAUSE_REPLAY_HASH);
}

static void test_replay_tech(void) {
    replay("tech replay", TECH, 0, TECH_FRAMES, TECH_REPLAY_HASH);
}

static void test_replay_built(void) {
    replay("built replay", BUILT, 0, BUILT_FRAMES, BUILT_REPLAY_HASH);
}

static void test_replay_game_end(void) {
    replay("game end replay", GAME_END, 0, GAME_END_FRAMES, GAME_END_REPLAY_HASH);
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

    start_game(PAUSE);
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

    start_game(PAUSE);
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
    const pico8_instance_t *lq = get_ref(b, "lq");
    pico8_value_t v;
    assert(pico8_ob_table_get_index(lq->fields, 1, &v) && v.kind == PICO8_VALUE_FIXED);
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

    start_game(TECH);
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

    run_to(BUILT, KNOWN_PANEL_FRAME);
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

    run_to(TECH, 45);
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

    start_game(PAUSE);
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

    start_game(PAUSE);
    pico8_ob_table_set_name(b, "lq", pico8_value_ref(NULL));
    no_keys = pico8_js_new(b);
    pico8_ob_table_set_name(b, "h_", pico8_value_ref(NULL));
    pico8_ob_table_set_name(b, "jb", pico8_value_ref(NULL));
    nil_keys = pico8_js_new(b);
    pico8_ob_table_free(b);
    assert(!has(no_keys, "h_") && !has(no_keys, "jb"));
    assert(pico8_type_tag_of(get_ref(nil_keys, "h_")) == PICO8_TYPE_TAG_INSTANCE);
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

    start_from_title(TECH);
    check_int("struct title", all_buttons_are_structs(), 4);
    nj = pico8_js_new(bo_row(2));
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

static void test_panels_are_structs(void) {
    run_to(BUILT, KNOWN_PANEL_FRAME);
    assert(all_buttons_are_structs() >= 2);
}

static void test_pj_are_structs(void) {
    run_to(GAME_END, GAME_END_FRAMES - 1);
    assert(all_buttons_are_structs() >= 2);
}

/* A replay depends on the rnd state: run each scenario in a child forked
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
    in_fresh_child(test_harness);
    in_fresh_child(test_dk);
    in_fresh_child(test_replay_title_click);
    in_fresh_child(test_replay_pause);
    in_fresh_child(test_replay_tech);
    in_fresh_child(test_replay_built);
    in_fresh_child(test_replay_game_end);
    in_fresh_child(test_ph);
    in_fresh_child(test_ldown);
    in_fresh_child(test_hb);
    in_fresh_child(test_fe_my);
    in_fresh_child(test_cr_my);
    in_fresh_child(test_draw);
    in_fresh_child(test_nil_keys);
    in_fresh_child(test_js_is_a_struct);
    in_fresh_child(test_panels_are_structs);
    in_fresh_child(test_pj_are_structs);

    printf("test_pico8_js: all tests passed\n");
    return 0;
}
