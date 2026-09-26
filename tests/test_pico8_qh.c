/* The laboratory `qh`, read and written only through the dual-form field
 * API and `pico8_class_get_field(pico8_qh_class()/pico8_jo_class(), ...)`,
 * and driven through the real `ie:ldown`/`mv`/`dg`. Its unconverted
 * descendants are read the same way. The frozen values were taken on the
 * code where `qh` was still a class instance and must not be re-taken. */

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
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define EMPTY "tests/fixtures/empty_planet_cleared_events.json"
#define SESSION "tests/fixtures/session_events.json"
#define RECORDED "tests/fixtures/events_recorded_1790198294.json"
#define HUB "tests/fixtures/trading_hub_session_events.json"
#define LONG "tests/fixtures/long_session_events.json"

#define BUILT_HASH 0xA67C1FC6u
#define LIFE_HASH 0x0ECCCC0Au
#define EMPTY_FRAMES 229
#define EMPTY_LABS 2
#define EMPTY_HASH 0xF88BB3C4u
#define SESSION_FRAMES 116045
#define SESSION_LABS 4
#define SESSION_HASH 0xEE585B8Bu
#define SESSION_DESC_HASH 0xEE6EAC3Cu
#define RECORDED_FRAMES 19832
#define RECORDED_DESC_HASH 0xDD7C9833u
#define HUB_FRAMES 4921
#define HUB_DESC_HASH 0xB439DB43u
#define LONG_FRAMES 27273
#define LONG_LABS 2
#define LONG_HASH 0x3798DDA5u
#define LONG_DESC_HASH 0x97A15FB2u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

/* qh's own members, then the 31 of `jo`. */
static const char *const own[] = {"hp", "px", "dq", "qh", "od", "fp", "dw", "s", "ba", "et"};
static const char *const jo_members[] = {"nu", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq",
                                         "ca", "em", "id", "p",  "ra", "oe", "hn", "ep", "jy", "nk", "fx",
                                         "lq", "fo", "jh", "p_", "kr", "cc", "by", "mi", "m_"};
static const char *const methods[] = {"hd", "kj", "ldown", "hb", "gg", "gv", "hk", "kf",
                                      "cn", "ib", "ll", "hq", "ev", "ey", "ly"};
static const char *const absent[] = {"ph", "kz", "rdown", "md", "ol", "pr", "eb"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"qh", "jo", "np", "prober"};
static const char *const qh_tables[] = {"nu", "px", "ba"};
static const char *const jo_tables[] = {"kq", "em", "ka", "dz", "cq", "gq"};

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

static pico8_value_t class_value(pico8_class_t *cls, const char *name) {
    pico8_value_t v;
    assert(pico8_class_get_field(cls, name, &v));
    return v;
}

static void *class_ref(pico8_class_t *cls, const char *name) {
    pico8_value_t v = class_value(cls, name);
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
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

static uint32_t fold_value(uint32_t h, pico8_value_t v) {
    switch (v.kind) {
    case PICO8_VALUE_BOOL:
        return fnv(fnv(h, 0xB0u), (uint32_t)v.as.boolean);
    case PICO8_VALUE_FIXED:
        return fnv(fnv(h, 0xF1u), (uint32_t)v.as.fixed);
    case PICO8_VALUE_STRING:
        return fnv_str(fnv(h, 0x57u), v.as.string);
    case PICO8_VALUE_FUNCTION:
        return fnv(h, 0xFCu);
    case PICO8_VALUE_REF:
        if (v.as.ref == NULL) {
            return fnv(h, 0xDEADu);
        }
        if (listed(pico8_hv(), v.as.ref)) {
            return fnv(fnv(h, 0x4Eu), (uint32_t)rank_in(pico8_hv(), v.as.ref));
        }
        return fnv(fnv(h, 0xEEEEu), (uint32_t)pico8_type_tag_of(v.as.ref));
    default:
        return fnv(h, 0xF0u);
    }
}

static uint32_t fold_field(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v) ? fold_value(fnv(h, 1u), v) : fnv(h, 0u);
}

/* The item count of a list field, -1 when absent. */
static int list_size(const void *e, const char *name) {
    pico8_value_t v;
    if (!lookup(e, name, &v)) {
        return -1;
    }
    assert(v.kind == PICO8_VALUE_REF);
    return ((const pico8_list_t *)v.as.ref)->count;
}

static uint32_t fold_kw(uint32_t h, const void *e) {
    pico8_value_t kw;
    pico8_value_t v;
    if (!lookup(e, "kw", &kw)) {
        return fnv(h, 0xA0u);
    }
    assert(kw.kind == PICO8_VALUE_REF && pico8_type_tag_of(kw.as.ref) == PICO8_TYPE_TAG_OB_TABLE);
    h = pico8_ob_table_get_name(kw.as.ref, "qe", &v) ? fold_value(fnv(h, 1u), v) : fnv(h, 0u);
    h = pico8_ob_table_get_name(kw.as.ref, "mu", &v) ? fold_value(fnv(h, 1u), v) : fnv(h, 0u);
    return fnv(h, (uint32_t)(kw.as.ref == class_ref(pico8_jo_class(), "kw")));
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

static pico8_value_t vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return pico8_value_ref(v);
}

static pico8_vec_t vec_xy(int x, int y) {
    return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
}

static pico8_value_t call(void *e, const char *name, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, argc, arg0, arg1, &out));
    return out;
}

/* The research icon whose `ke` is `gx[0]`, the laboratory. */
static void *qh_icon(void) {
    const pico8_list_t *hb = pico8_cv("hb");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i].as.ref;
        pico8_value_t ke;
        if (lookup(e, "ke", &ke) && ke.kind == PICO8_VALUE_REF && ke.as.ref == pico8_qh_class()) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the laboratory's icon, as the shop does. */
static void *build_lab(void) {
    void *icon = qh_icon();
    pico8_value_t made;

    assert(icon != NULL);
    made = call(icon, "ldown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(made.kind == PICO8_VALUE_REF && made.as.ref != NULL);
    return made.as.ref;
}

/* A laboratory as `ie:ldown` and `jo:pq` left it: its scalars, which of
 * qh's and jo's member names it has, its methods (the very functions of
 * the class chain), which default tables it shares with `qh` and `jo`, and
 * where `lt` registered it. */
static uint32_t fold_lab(uint32_t h, const void *e) {
    static const char *const scalars[] = {"id", "me", "t",  "hp", "dq", "qh", "od", "fp",
                                          "dw", "mi", "s",  "hf", "hs", "fa", "ma", "ca"};
    pico8_value_t gb1;
    size_t i;

    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fold_vec(h, e, "gq");
    for (i = 0; i < sizeof own / sizeof own[0]; i++) {
        h = fnv(h, (uint32_t)has(e, own[i]));
    }
    for (i = 0; i < sizeof jo_members / sizeof jo_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, jo_members[i]));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        pico8_value_t v;
        pico8_value_t want = class_value(pico8_qh_class(), methods[i]);
        assert(pico8_entity_get_field(e, methods[i], &v) && v.kind == PICO8_VALUE_FUNCTION);
        assert(want.kind == PICO8_VALUE_FUNCTION && v.as.function == want.as.function);
    }
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(e, absent[i]));
    }
    for (i = 0; i < sizeof qh_tables / sizeof qh_tables[0]; i++) {
        assert(get_ref(e, qh_tables[i]) == class_ref(pico8_qh_class(), qh_tables[i]));
    }
    for (i = 0; i < sizeof jo_tables / sizeof jo_tables[0]; i++) {
        assert(get_ref(e, jo_tables[i]) == class_ref(pico8_jo_class(), jo_tables[i]));
    }
    assert(get_ref(e, "kw") != NULL && get_ref(e, "kw") != class_ref(pico8_jo_class(), "kw"));
    assert(pico8_ob_table_get_index(pico8_gb(), 1, &gb1) && gb1.kind == PICO8_VALUE_REF);
    assert(get_ref(e, "hn") == gb1.as.ref);
    h = fold_kw(h, e);
    h = fnv(h, (uint32_t)list_size(e, "oe"));
    h = fnv(h, (uint32_t)list_size(e, "jy"));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cv(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cg(tags[i]), e));
    }
    return h;
}

/* Two laboratories built by the shop's `ie:ldown` on `gx[0]`. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game(EMPTY);
    a = build_lab();
    b = build_lab();
    assert(a != b);
    h = fold_lab(h, a);
    h = fold_lab(h, b);
    check("built", h, BUILT_HASH);
}

static pico8_fixed_t credits(void) {
    pico8_value_t nz;
    assert(pico8_entity_get_field(pico8_fm(), "nz", &nz) && nz.kind == PICO8_VALUE_FIXED);
    return nz.as.fixed;
}

static void set_credits(int n) {
    pico8_entity_set_field(pico8_fm(), "nz", pico8_value_fixed(pico8_fixed_from_int(n)));
}

/* What `bv` left on a laboratory: `ra` (the very REF passed), `p`, and
 * what `et` is. */
static uint32_t fold_held(uint32_t h, const void *e, pico8_value_t mp) {
    assert(get_ref(e, "ra") == mp.as.ref);
    h = fold_vec(h, e, "p");
    h = fnv(h, bool_state(e, "et"));
    return fold_field(h, e, "et");
}

static uint32_t fold_ra_y(uint32_t h, const void *e) {
    const pico8_vec_t *ra = get_ref(e, "ra");
    assert(ra != NULL);
    return fnv(h, (uint32_t)ra->y);
}

/* `hq` (`bv`) at an empty point, in the panel and out of it, then next to
 * a planet; `ey` (`d_`) with and without the credits; `dg` on a laboratory
 * in "ly" (`qh.ly` = `na`) and on one still in "kz". */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *out_lab;
    void *in_lab;
    void *poor_lab;
    void *planet;
    pico8_value_t mp;
    pico8_value_t planet_p;
    pico8_value_t ra_before;
    pico8_fixed_t before;
    pico8_fixed_t in_y;
    int i;

    start_game(EMPTY);

    out_lab = build_lab();
    mp = vec_ref(vec_xy(7680, 7680));
    (void)call(out_lab, "hq", 2, mp, vec_ref(vec_xy(0, 0)));
    h = fold_held(h, out_lab, mp);
    assert(bool_state(out_lab, "et") == 3 && !listed(pico8_hv(), get_ref(out_lab, "et")));

    planet = pico8_cg("jo")->items[0].as.ref;
    assert(lookup(planet, "p", &planet_p));
    mp = vec_ref(pico8_vec_add(pico8_vec_mul(*(const pico8_vec_t *)planet_p.as.ref, pico8_fixed_from_int(128)),
                               vec_xy(6, 0)));
    (void)call(out_lab, "hq", 2, mp, vec_ref(vec_xy(64, 64)));
    h = fold_held(h, out_lab, mp);
    assert(get_ref(out_lab, "et") == planet && get_ref(planet, "fo") == out_lab);

    poor_lab = build_lab();
    mp = vec_ref(vec_xy(7680, 7680));
    (void)call(poor_lab, "hq", 2, mp, vec_ref(vec_xy(64, 64)));
    h = fold_held(h, poor_lab, mp);
    assert(bool_state(poor_lab, "et") == 1);
    set_credits(3);
    h = fold_value(h, call(poor_lab, "ey", 1, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fold_field(h, poor_lab, "me");
    h = fold_field(h, poor_lab, "t");
    h = fnv(h, bool_state(poor_lab, "p_"));
    h = fnv(h, (uint32_t)credits());
    assert(bool_state(poor_lab, "p_") == 2 && credits() == pico8_fixed_from_int(3));

    in_lab = build_lab();
    mp = vec_ref(vec_xy(7680, 7552));
    (void)call(in_lab, "hq", 2, mp, vec_ref(vec_xy(64, 64)));
    set_credits(100);
    h = fold_value(h, call(in_lab, "ey", 1, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fold_field(h, in_lab, "me");
    h = fold_field(h, in_lab, "t");
    h = fnv(h, bool_state(in_lab, "p_"));
    h = fnv(h, (uint32_t)credits());
    assert(strcmp(get(in_lab, "me").as.string, "ly") == 0 && credits() == pico8_fixed_from_int(100 - 15));
    in_y = ((const pico8_vec_t *)get_ref(in_lab, "ra"))->y;

    ra_before = get(out_lab, "ra");
    before = ((const pico8_vec_t *)ra_before.as.ref)->y;
    for (i = 0; i < 4; i++) {
        h = fold_ra_y(h, in_lab);
        pico8_dg();
        h = fold_field(h, in_lab, "t");
        assert(get_ref(out_lab, "ra") == ra_before.as.ref && ((const pico8_vec_t *)ra_before.as.ref)->y == before);
    }
    h = fold_ra_y(h, in_lab);
    assert(((const pico8_vec_t *)get_ref(in_lab, "ra"))->y != in_y);
    h = fnv(h, (uint32_t)listed(pico8_hv(), poor_lab));
    check("life", h, LIFE_HASH);
}

/* Per frame, every entry of `cg.qh`: what the shop, `bv`, `d_`, `dg` and
 * `jo`'s functions write on it; plus `#cg.qh` and the screen. */
static uint32_t fold_live(uint32_t h, const void *e) {
    h = fold_field(h, e, "id");
    h = fold_field(h, e, "me");
    h = fold_field(h, e, "t");
    h = fold_vec(h, e, "ra");
    h = fold_vec(h, e, "p");
    h = fnv(h, bool_state(e, "et"));
    h = fold_field(h, e, "et");
    h = fold_field(h, e, "mi");
    h = fnv(h, bool_state(e, "p_"));
    h = fold_kw(h, e);
    h = fnv(h, (uint32_t)list_size(e, "oe"));
    h = fnv(h, (uint32_t)list_size(e, "jy"));
    return fnv(h, (uint32_t)has(e, "m_"));
}

enum { NL, OS, HJ, PM, MX, NR, FR, DESCENDANTS };

static const char *const descendant_names[DESCENDANTS] = {"nl", "os", "hj", "pm", "mx", "nr", "fr"};

static int descendant_of(const void *e) {
    pico8_class_t *const classes[DESCENDANTS] = {pico8_nl_class(), pico8_os_class(), pico8_hj_class(),
                                                 pico8_pm_class(), pico8_mx_class(), pico8_nr_class(),
                                                 pico8_fr_class()};
    int d;

    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_nl_struct_class()) {
        return NL;
    }
    if (e == NULL || pico8_type_tag_of(e) != PICO8_TYPE_TAG_INSTANCE) {
        return -1;
    }
    for (d = 0; d < DESCENDANTS; d++) {
        if (((const pico8_instance_t *)e)->cls == classes[d]) {
            return d;
        }
    }
    return -1;
}

/* What a descendant inherits from `qh`: `dq`, `fp`, `dw`, `hf`, whether
 * its `nu`/`px`/`ba` are qh's own tables, `hq`/`ev`/`ey` (qh's functions),
 * and its `ly` called once (`ra.y` before, after, then put back). */
static uint32_t fold_descendant(uint32_t h, void *d) {
    static const char *const inherited[] = {"dq", "fp", "dw", "hf", "mi"};
    static const char *const fns[] = {"hq", "ev", "ey"};
    size_t i;
    pico8_value_t ra;
    pico8_value_t qw;
    int had_qw;

    for (i = 0; i < sizeof inherited / sizeof inherited[0]; i++) {
        h = fold_field(h, d, inherited[i]);
    }
    for (i = 0; i < sizeof qh_tables / sizeof qh_tables[0]; i++) {
        h = fnv(h, (uint32_t)(get_ref(d, qh_tables[i]) == class_ref(pico8_qh_class(), qh_tables[i])));
    }
    for (i = 0; i < sizeof fns / sizeof fns[0]; i++) {
        pico8_value_t v;
        assert(lookup(d, fns[i], &v) && v.kind == PICO8_VALUE_FUNCTION);
        assert(v.as.function == class_value(pico8_qh_class(), fns[i]).as.function);
    }
    had_qw = lookup(d, "qw", &qw);
    if (lookup(d, "ra", &ra)) {
        pico8_vec_t *v = ra.as.ref;
        pico8_fixed_t y = v->y;
        h = fnv(fnv(h, 1u), (uint32_t)y);
        (void)call(d, "ly", 1, pico8_value_fixed(pico8_fixed_from_int(50)), pico8_value_ref(NULL));
        h = fnv(h, (uint32_t)v->y);
        v->y = y;
    } else {
        (void)call(d, "ly", 1, pico8_value_fixed(pico8_fixed_from_int(50)), pico8_value_ref(NULL));
        h = fnv(h, 0u);
    }
    h = fold_field(h, d, "qw");
    pico8_entity_set_field(d, "qw", had_qw ? qw : pico8_value_ref(NULL));
    return h;
}

#define MAX_SEEN 256

typedef struct {
    const char *label;
    const char *events;
    int want_frames;
    int want_labs;
    uint32_t want_labs_hash;
    uint32_t want_desc_hash;
    const int *required;
    int required_count;
} replay_t;

/* From the title screen, as a raylib recording starts (or from `eu` when
 * `from_title` is 0). Every laboratory of `cg.qh`, every frame; the first
 * instance of each descendant at the frame it first appears in `hv`, then
 * at the end of the replay. */
static void replay(const replay_t *r, int from_title) {
    uint32_t labs_hash = 2166136261u;
    uint32_t desc_hash = 2166136261u;
    void *labs[MAX_SEEN];
    int lab_count = 0;
    void *first[DESCENDANTS] = {NULL};
    void *seen[MAX_SEEN];
    int seen_count = 0;
    int counts[DESCENDANTS] = {0};
    int frames;
    int i;

    if (from_title) {
        start_from_title(r->events);
    } else {
        start_game(r->events);
    }
    frames = input.frame_count;
    for (i = 0; i < frames; i++) {
        const pico8_list_t *qh;
        const pico8_list_t *hv;
        int j;

        play_frame();
        qh = pico8_cg("qh");
        labs_hash = fnv(labs_hash, (uint32_t)(qh == NULL ? 0 : qh->count));
        for (j = 0; qh != NULL && j < qh->count; j++) {
            void *e = qh->items[j].as.ref;
            int k;
            for (k = 0; k < lab_count && labs[k] != e; k++) {
            }
            if (k == lab_count) {
                assert(lab_count < MAX_SEEN);
                labs[lab_count++] = e;
            }
            labs_hash = fold_live(labs_hash, e);
        }
        if (r->want_labs > 0) {
            labs_hash = fnv(labs_hash, screen_hash());
        }

        hv = pico8_hv();
        for (j = 0; j < hv->count; j++) {
            void *e = hv->items[j].as.ref;
            int d = descendant_of(e);
            int k;

            if (d < 0) {
                continue;
            }
            for (k = 0; k < seen_count && seen[k] != e; k++) {
            }
            if (k == seen_count) {
                assert(seen_count < MAX_SEEN);
                seen[seen_count++] = e;
                counts[d]++;
            }
            if (first[d] == NULL) {
                first[d] = e;
                printf("[%s] first %s at frame %d\n", r->label, descendant_names[d], i);
                desc_hash = fnv(fnv(desc_hash, (uint32_t)d), (uint32_t)i);
                desc_hash = fold_descendant(desc_hash, e);
            }
        }
    }
    for (i = 0; i < DESCENDANTS; i++) {
        printf("[%s] %s: %d\n", r->label, descendant_names[i], counts[i]);
        desc_hash = fnv(desc_hash, (uint32_t)counts[i]);
        if (first[i] != NULL) {
            desc_hash = fold_descendant(fnv(desc_hash, (uint32_t)i), first[i]);
        }
    }
    for (i = 0; i < r->required_count; i++) {
        assert(first[r->required[i]] != NULL);
    }
    check_int(r->label, frames, r->want_frames);
    check_int(r->label, lab_count, r->want_labs);
    if (r->want_labs > 0) {
        check(r->label, labs_hash, r->want_labs_hash);
    }
    if (r->required_count > 0) {
        check(r->label, desc_hash, r->want_desc_hash);
    }
}

static void test_empty(void) {
    static const replay_t r = {"empty", EMPTY, EMPTY_FRAMES, EMPTY_LABS, EMPTY_HASH, 0u, NULL, 0};
    replay(&r, 0);
}

static void test_session(void) {
    static const int required[] = {NR, FR, PM};
    static const replay_t r = {"session",    SESSION,           SESSION_FRAMES, SESSION_LABS,
                               SESSION_HASH, SESSION_DESC_HASH, required,       3};
    replay(&r, 1);
}

static void test_recorded(void) {
    static const int required[] = {HJ, MX, PM};
    static const replay_t r = {"recorded", RECORDED, RECORDED_FRAMES, 0, 0u, RECORDED_DESC_HASH,
                               required,   3};
    replay(&r, 1);
}

static void test_hub(void) {
    static const int required[] = {NL, NR};
    static const replay_t r = {"hub", HUB, HUB_FRAMES, 0, 0u, HUB_DESC_HASH, required, 2};
    replay(&r, 1);
}

static void test_long(void) {
    static const int required[] = {OS};
    static const replay_t r = {"long", LONG, LONG_FRAMES, LONG_LABS, LONG_HASH, LONG_DESC_HASH, required, 1};
    replay(&r, 1);
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

static void write_hp(void *e) {
    pico8_entity_set_field(e, "hp", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void write_id(void *e) {
    pico8_entity_set_field(e, "id", pico8_value_string("s0"));
}

static void write_s(void *e) {
    pico8_entity_set_field(e, "s", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void write_od(void *e) {
    pico8_entity_set_field(e, "od", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void write_et(void *e) {
    pico8_entity_set_field(e, "et", pico8_value_bool(0));
}

static void build_with_unknown_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "p", vec_ref(vec_xy(1, 1)));
    (void)pico8_qh_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_qh_new(b);
}

static int is_a_qh_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_qh_struct_class();
}

/* Built by `ie:ldown` or pico8_qh_new, a `qh` is a struct whose descriptor
 * holds jo's very dispatchers and the four functions qh's class installs;
 * it takes exactly the 17 names of `jo` plus `ra`, `p`, `et`, and a
 * construction key other than `id` stops the run. A bare `jo` still
 * refuses `et`; `nl` is a struct under its own descriptor, the other
 * descendants stay instances. */
static void test_qh_is_a_struct(void) {
    static const char *const jo_methods[] = {"hd", "kj", "ldown", "hb", "gg", "gv", "hk", "kf", "cn", "ib", "ll"};
    static const char *const qh_methods[] = {"hq", "ev", "ey", "ly"};
    static const char *const written[] = {"by", "cc", "ep", "fo", "fx", "hn", "jh", "jy", "kr", "kw",
                                          "lq", "m_", "ma", "mi", "nk", "oe", "p_", "ra", "p",  "et"};
    pico8_class_t *const descendants[] = {pico8_nl_class(), pico8_os_class(), pico8_hj_class(), pico8_pm_class(),
                                          pico8_mx_class(), pico8_nr_class(), pico8_fr_class()};
    const pico8_struct_class_t *cls = pico8_qh_struct_class();
    void *a;
    size_t i;
    int j;

    start_game(EMPTY);
    a = build_lab();
    assert(is_a_qh_struct(a) && listed(pico8_cg("qh"), a));
    for (i = 0; i < sizeof jo_methods / sizeof jo_methods[0]; i++) {
        int m = pico8_method_of(jo_methods[i]);
        assert(cls->methods[m] != NULL && cls->methods[m] == pico8_jo_struct_class()->methods[m]);
    }
    for (i = 0; i < sizeof qh_methods / sizeof qh_methods[0]; i++) {
        assert(cls->methods[pico8_method_of(qh_methods[i])] ==
               class_value(pico8_qh_class(), qh_methods[i]).as.function);
    }
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        int k;
        if (cls->methods[j] == NULL) {
            continue;
        }
        for (k = 0; k < (int)(sizeof methods / sizeof methods[0]) && pico8_method_of(methods[k]) != j; k++) {
        }
        assert(k < (int)(sizeof methods / sizeof methods[0]));
    }

    for (i = 0; i < sizeof written / sizeof written[0]; i++) {
        pico8_value_t v;
        pico8_entity_set_field(a, written[i], pico8_value_fixed(pico8_fixed_from_int((int)i + 1)));
        assert(lookup(a, written[i], &v) && v.kind == PICO8_VALUE_FIXED &&
               v.as.fixed == pico8_fixed_from_int((int)i + 1));
    }
    must_abort(write_hp, a);
    must_abort(write_id, a);
    must_abort(write_s, a);
    must_abort(write_od, a);
    must_abort(write_et, pico8_jo_new(NULL));
    must_abort(build_with_unknown_key, NULL);
    must_abort(build_with_positional_key, NULL);
    {
        void *bare = pico8_qh_new(NULL);
        assert(is_a_qh_struct(bare) && !has(bare, "id") && has(bare, "oe") && listed(pico8_cg("qh"), bare));
    }
    for (i = 0; i < sizeof descendants / sizeof descendants[0]; i++) {
        pico8_ob_table_t *b = pico8_ob_table_new();
        void *d;
        pico8_ob_table_set_name(b, "id", pico8_value_string("s99"));
        d = pico8_class_instantiate(descendants[i], b);
        pico8_ob_table_free(b);
        if (descendants[i] == pico8_nl_class()) {
            assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_STRUCT &&
                   ((const pico8_struct_header_t *)d)->cls == pico8_nl_struct_class());
            continue;
        }
        assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_INSTANCE && ((const pico8_instance_t *)d)->cls == descendants[i]);
    }
}

/* A replay depends on the rnd state: run each scenario in a child forked
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
    pid_t recordings[4];
    int i;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    /* The four recordings take minutes each: they run side by side. */
    recordings[0] = spawn(test_session);
    recordings[1] = spawn(test_recorded);
    recordings[2] = spawn(test_hub);
    recordings[3] = spawn(test_long);
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_empty);
    in_fresh_child(test_qh_is_a_struct);
    for (i = 0; i < 4; i++) {
        reap(recordings[i]);
    }

    printf("test_pico8_qh: all tests passed\n");
    return 0;
}
