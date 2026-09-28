/* The protostar `pm`, read and written only through the dual-form field API
 * and `pico8_pm_default()`, `pico8_hj_default()`, `pico8_qh_default()`,
 * `pico8_jo_default()` and `pico8_ga_default()`, built as `jo:hu` builds it
 * (`kv["pm"]` on `p`, `ra`, `id`) and driven through the real
 * `mv`/`dg`/`dc`. The frozen values were taken on the code where `pm` was
 * still a class instance and must not be re-taken; only test_pm_is_a_struct
 * looks at the form. */

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
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy_score.h"
#include "../src/pico8_economy_tiers.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_particle.h"
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

#define BUILT_HASH 0x73FAD653u
#define LIFE_HASH 0x3F990656u
#define SESSION_FRAMES 116045
#define SESSION_PROTOSTARS 3
#define SESSION_HASH 0xC0C2E262u
#define RECORDED_FRAMES 19832
#define RECORDED_PROTOSTARS 1
#define RECORDED_HASH 0xFCA97785u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

/* pm's own `ng` and `fz`, then hj's 12 emitter defaults and `qy`, then the
 * 10 members of `qh`, then the 31 of `jo`. */
static const char *const pm_own[] = {"ng", "fz"};
static const char *const hj_members[] = {"rr", "py", "ki", "pn", "pk", "fc", "k_",
                                         "du", "qo", "ro", "qg", "q_", "qy"};
static const char *const qh_members[] = {"hp", "px", "dq", "qh", "od", "fp", "dw", "s", "ba", "et"};
static const char *const jo_members[] = {"nu", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq",
                                         "ca", "em", "id", "p",  "ra", "oe", "hn", "ep", "jy", "nk", "fx",
                                         "lq", "fo", "jh", "p_", "kr", "cc", "by", "mi", "m_"};
static const char *const methods[] = {"hd", "kj", "ldown", "hb", "gg", "gv", "hk", "kf",
                                      "cn", "ib", "ll", "hq", "ev", "ey", "ly", "mm"};
static const char *const absent[] = {"ph", "kz", "rdown", "md", "ol", "pr", "eb"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"jo", "np", "prober"};
static const char *const pm_tables[] = {"nu", "px", "k_", "ng"};
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

/* The instance reader keeps a field written nil (`ep`, `fo`: TROU-9), the
 * struct reader reads a nil member as absent: nil and absent are one state
 * here. */
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

static pico8_value_t class_value(int (*default_of)(const char *, pico8_value_t *), const char *name) {
    pico8_value_t v;
    assert(default_of(name, &v));
    return v;
}

static void *class_ref(int (*default_of)(const char *, pico8_value_t *), const char *name) {
    pico8_value_t v = class_value(default_of, name);
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

static pico8_value_function_t method(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_FUNCTION);
    return v.as.function;
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
    return fnv(h, (uint32_t)(kw.as.ref == class_ref(pico8_jo_default, "kw")));
}

/* A particle record as `g_.mm` returns it: `p`, `v`, `l`. */
static uint32_t fold_particle(uint32_t h, pico8_value_t pt) {
    static const char *const vecs[] = {"p", "v"};
    pico8_value_t v;
    size_t i;

    assert(pt.kind == PICO8_VALUE_REF && pt.as.ref != NULL && pico8_type_tag_of(pt.as.ref) == PICO8_TYPE_TAG_OB_TABLE);
    for (i = 0; i < sizeof vecs / sizeof vecs[0]; i++) {
        const pico8_vec_t *p;
        assert(pico8_ob_table_get_name(pt.as.ref, vecs[i], &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
        p = v.as.ref;
        h = fnv(fnv(h, (uint32_t)p->x), (uint32_t)p->y);
    }
    assert(pico8_ob_table_get_name(pt.as.ref, "l", &v) && v.kind == PICO8_VALUE_FIXED);
    return fnv(h, (uint32_t)v.as.fixed);
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

/* `kv["pm"]` as `jo:hu` calls it: `p`, `ra`, `id`, each only when given. */
static void *build_protostar(const pico8_vec_t *p, const pico8_vec_t *ra, const char *id) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    void *made;

    if (p != NULL) {
        pico8_ob_table_set_name(b, "p", vec_ref(*p));
    }
    if (ra != NULL) {
        pico8_ob_table_set_name(b, "ra", vec_ref(*ra));
    }
    if (id != NULL) {
        pico8_ob_table_set_name(b, "id", pico8_value_string(id));
    }
    made = pico8_pm_new(b);
    pico8_ob_table_free(b);
    assert(made != NULL);
    return made;
}

/* `pm` sets its own `px`: an entity whose `px` is the very table of pm's
 * class is a protostar, recognized by what it reads, not by its form. */
static int is_pm(const void *e) {
    return e != NULL && get_ref(e, "px") == class_ref(pico8_pm_default, "px");
}

static int is_ga(const void *e) {
    pico8_value_t fz;
    return e != NULL && lookup(e, "fz", &fz) && fz.kind == PICO8_VALUE_STRING && strcmp(fz.as.string, "ga") == 0;
}

/* The `ga` that `pm:pq` spawned: registered in `hv` just before the
 * protostar, at `ra + ng`; its `qy` and `mo.ma` as `ga:pq` left them. */
static uint32_t fold_spawned(uint32_t h, const void *e) {
    const pico8_list_t *hv = pico8_hv();
    int at = rank_in(hv, e);
    const void *g;
    const pico8_vec_t *ra = get_ref(e, "ra");
    const pico8_vec_t *ng = get_ref(e, "ng");
    const pico8_vec_t *gra;

    assert(at >= 1);
    g = hv->items[at - 1].as.ref;
    assert(is_ga(g));
    gra = get_ref(g, "ra");
    assert(gra != NULL && ng != NULL);
    if (ra != NULL) {
        assert(gra->x == pico8_fixed_add(ra->x, ng->x) && gra->y == pico8_fixed_add(ra->y, ng->y));
    } else {
        assert(gra->x == ng->x && gra->y == ng->y);
    }
    assert(gra != ra && gra != ng);
    h = fold_vec(h, g, "ra");
    h = fnv(h, (uint32_t)list_size(g, "qy"));
    return fold_field(h, pico8_mo(), "ma");
}

/* A protostar as `kv["pm"]`, `pm:pq`, `f_` and `jo:pq` left it: its
 * scalars, which of its own, hj's, qh's and jo's names it has, its methods
 * (hj's very functions), which default tables it shares with `pm`, `qh`
 * and `jo`, where `lt` registered it -- in `cg.jo`/`np`/`prober`, never in
 * `cg.qh` or `cg.hj` -- and the `ga` it spawned. */
static uint32_t fold_protostar(uint32_t h, const void *e) {
    static const char *const scalars[] = {"id", "me", "t",  "fz", "rr", "qh", "hp", "s",  "od", "py",
                                          "ki", "pn", "pk", "fc", "du", "qo", "ro", "qg", "q_", "dq",
                                          "fp", "dw", "mi", "hf", "hs", "fa", "ma", "ca"};
    static const char *const not_pm[] = {"nl", "qw", "ht", "et"};
    pico8_value_t gb3;
    size_t i;

    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fold_vec(h, e, "ng");
    h = fold_vec(h, e, "gq");
    h = fold_vec(h, e, "ra");
    h = fold_vec(h, e, "p");
    for (i = 0; i < sizeof pm_own / sizeof pm_own[0]; i++) {
        h = fnv(h, (uint32_t)has(e, pm_own[i]));
    }
    for (i = 0; i < sizeof hj_members / sizeof hj_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, hj_members[i]));
    }
    for (i = 0; i < sizeof qh_members / sizeof qh_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, qh_members[i]));
    }
    for (i = 0; i < sizeof jo_members / sizeof jo_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, jo_members[i]));
    }
    for (i = 0; i < sizeof not_pm / sizeof not_pm[0]; i++) {
        assert(!has(e, not_pm[i]));
    }
    assert(list_size(e, "qy") == 0);
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        pico8_value_function_t got = method(e, methods[i]);
        if (strcmp(methods[i], "kj") == 0 || strcmp(methods[i], "mm") == 0) {
            assert(got == pico8_hj_struct_class()->methods[pico8_method_of(methods[i])]);
        } else {
            assert(got == pico8_qh_struct_class()->methods[pico8_method_of(methods[i])]);
        }
    }
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(e, absent[i]));
    }
    for (i = 0; i < sizeof pm_tables / sizeof pm_tables[0]; i++) {
        assert(get_ref(e, pm_tables[i]) == class_ref(pico8_pm_default, pm_tables[i]));
    }
    assert(get_ref(e, "nu") != class_ref(pico8_hj_default, "nu"));
    assert(get_ref(e, "k_") != class_ref(pico8_hj_default, "k_"));
    assert(get_ref(e, "ba") == class_ref(pico8_qh_default, "ba"));
    for (i = 0; i < sizeof jo_tables / sizeof jo_tables[0]; i++) {
        assert(get_ref(e, jo_tables[i]) == class_ref(pico8_jo_default, jo_tables[i]));
    }
    assert(get_ref(e, "kw") != NULL && get_ref(e, "kw") != class_ref(pico8_jo_default, "kw"));
    assert(pico8_ob_table_get_index(pico8_gb(), 3, &gb3) && gb3.kind == PICO8_VALUE_REF);
    assert(get_ref(e, "hn") == gb3.as.ref);
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
    assert(listed(pico8_cg("jo"), e) && !listed(pico8_cg("qh"), e) && !listed(pico8_cg("hj"), e));
    return fold_spawned(h, e);
}

/* Two protostars built as `jo:hu` builds them, each with a `qy` of its own
 * and a `ga` of its own. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    pico8_vec_t pa = vec_xy(40, 50);
    pico8_vec_t ra_a = vec_xy(7700, 7600);
    pico8_vec_t pb = vec_xy(-30, 12);
    pico8_vec_t ra_b = vec_xy(7650, 7720);
    void *a;
    void *b;

    start_game(EMPTY);
    a = build_protostar(&pa, &ra_a, "s77");
    b = build_protostar(&pb, &ra_b, "s78");
    assert(a != b && is_pm(a) && is_pm(b));
    assert(get_ref(a, "qy") != NULL && get_ref(a, "qy") != get_ref(b, "qy"));
    h = fold_protostar(h, a);
    h = fold_protostar(h, b);
    check("built", h, BUILT_HASH);
}

static uint32_t fold_ra_y(uint32_t h, const void *e) {
    const pico8_vec_t *ra = get_ref(e, "ra");
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the protostar. */
static uint32_t fold_dg(uint32_t h, const void *e) {
    h = fold_ra_y(h, e);
    pico8_dg();
    h = fold_field(h, e, "t");
    return fold_ra_y(h, e);
}

/* What `bb`'s `dh`, `ib`, `gg` and `da` (qh false) give on a protostar. */
static uint32_t fold_economy(uint32_t h, void *e) {
    pico8_value_t ib = call(e, "ib", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_value(h, ib);
    h = fold_value(h, call(e, "gg", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fnv(h, (uint32_t)pico8_da(e));
    return fold_field(h, e, "mi");
}

/* `dc("kj")` once: the screen it leaves and what `hj:kj` left in `qy`. */
static uint32_t fold_draw(uint32_t h, const void *e) {
    pico8_dc("kj");
    h = fnv(h, screen_hash());
    return fnv(h, (uint32_t)list_size(e, "qy"));
}

/* `dg` in "ly" (qh's `ly`, `na`); `mm` (`g_.mm`, `pk` false: no rotation)
 * and `ez`; `kj` through `dc` with `mi`=1, then `mi`=2 through the generic
 * writer (`ei` moves and draws the particles); `dh`/`ib`/`gg`/`da`; a
 * protostar built on `id` alone, its `ga` at `ng`. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    const pico8_vec_t *mo_ra;
    pico8_vec_t p = vec_xy(64, 64);
    pico8_vec_t ra;
    void *placed;
    void *bare;
    int i;

    start_game(EMPTY);
    mo_ra = get_ref(pico8_mo(), "ra");
    assert(mo_ra != NULL);
    ra = pico8_vec_add(*mo_ra, vec_xy(12, 108));
    placed = build_protostar(&p, &ra, "s90");
    h = fold_spawned(h, placed);
    assert(strcmp(get(placed, "me").as.string, "ly") == 0);

    h = fold_economy(h, placed);
    for (i = 0; i < 3; i++) {
        h = fold_dg(h, placed);
    }

    h = fold_particle(h, call(placed, "mm", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    pico8_ez(placed, PICO8_FIXED_LIT(0x00028000, 2.5));
    h = fnv(h, (uint32_t)list_size(placed, "qy"));
    assert(list_size(placed, "qy") >= 2);

    h = fold_draw(h, placed);
    h = fold_draw(h, placed);
    pico8_entity_set_field(placed, "mi", pico8_value_fixed(pico8_fixed_from_int(2)));
    for (i = 0; i < 4; i++) {
        h = fold_draw(h, placed);
    }
    h = fold_economy(h, placed);

    bare = build_protostar(NULL, NULL, "s91");
    assert(!has(bare, "ra") && !has(bare, "p") && is_pm(bare));
    h = fold_field(h, bare, "id");
    h = fold_spawned(h, bare);
    h = fold_dg(h, bare);
    check("life", h, LIFE_HASH);
}

/* Per frame, every protostar in `hv`: its rank and what `jo:hu`, `jo:pq`,
 * `dg`, `hj:kj` and the economy write on it; plus the screen. */
static uint32_t fold_live(uint32_t h, const void *e) {
    h = fold_field(h, e, "id");
    h = fold_field(h, e, "me");
    h = fold_field(h, e, "t");
    h = fold_vec(h, e, "ra");
    h = fold_vec(h, e, "p");
    h = fold_field(h, e, "mi");
    h = fnv(h, bool_state(e, "p_"));
    h = fold_kw(h, e);
    h = fnv(h, (uint32_t)list_size(e, "oe"));
    h = fnv(h, (uint32_t)list_size(e, "jy"));
    h = fnv(h, (uint32_t)list_size(e, "cc"));
    h = fnv(h, (uint32_t)list_size(e, "by"));
    h = fnv(h, (uint32_t)list_size(e, "qy"));
    return fnv(h, (uint32_t)has(e, "m_"));
}

#define MAX_SEEN 16

/* From the title screen, as a raylib recording starts. */
static void replay(const char *label, const char *events, int want_frames, int want_protostars, uint32_t want) {
    uint32_t h = 2166136261u;
    void *seen[MAX_SEEN];
    int count = 0;
    int frames;
    int i;

    start_from_title(events);
    frames = input.frame_count;
    for (i = 0; i < frames; i++) {
        const pico8_list_t *hv;
        int j;

        play_frame();
        hv = pico8_hv();
        for (j = 0; j < hv->count; j++) {
            void *e = hv->items[j].as.ref;
            int k;

            if (!is_pm(e)) {
                continue;
            }
            for (k = 0; k < count && seen[k] != e; k++) {
            }
            if (k == count) {
                assert(count < MAX_SEEN);
                seen[count++] = e;
            }
            h = fold_live(fnv(fnv(h, (uint32_t)j), (uint32_t)k), e);
        }
        h = fnv(h, screen_hash());
    }
    check_int(label, frames, want_frames);
    check_int(label, count, want_protostars);
    check(label, h, want);
}

static void test_session(void) {
    replay("session", SESSION, SESSION_FRAMES, SESSION_PROTOSTARS, SESSION_HASH);
}

static void test_recorded(void) {
    replay("recorded", RECORDED, RECORDED_FRAMES, RECORDED_PROTOSTARS, RECORDED_HASH);
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

static void write_fixed(void *e, const char *name) {
    pico8_entity_set_field(e, name, pico8_value_fixed(pico8_fixed_from_int(9)));
}

static void write_ng(void *e) {
    write_fixed(e, "ng");
}

static void write_fz(void *e) {
    write_fixed(e, "fz");
}

static void write_rr(void *e) {
    write_fixed(e, "rr");
}

static void write_py(void *e) {
    write_fixed(e, "py");
}

static void write_hp(void *e) {
    write_fixed(e, "hp");
}

static void write_id(void *e) {
    pico8_entity_set_field(e, "id", pico8_value_string("s0"));
}

static void write_qw(void *e) {
    write_fixed(e, "qw");
}

static void write_ht(void *e) {
    write_fixed(e, "ht");
}

static void build_with_unknown_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "hp", pico8_value_fixed(pico8_fixed_from_int(1)));
    (void)pico8_pm_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_pm_new(b);
}

static int is_a_pm_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_pm_struct_class();
}

/* Built by `kv["pm"]` or pico8_pm_new, a protostar is a struct whose
 * descriptor holds hj's very entries and hj's writer, its reader its own;
 * hj's reader sees the 16 defaults of pm that fall on an inherited member;
 * it takes exactly hj's 21 names, and a construction key other than `p`,
 * `ra`, `id` stops the run. pm's class keeps no `pq` of its own. */
static void test_pm_is_a_struct(void) {
    static const char *const written[] = {"qy"};
    static const char *const inherited[] = {"me", "nu", "hp", "px", "s",  "py", "ki", "pn",
                                            "pk", "du", "fc", "k_", "qo", "ro", "qg", "q_"};
    const pico8_struct_class_t *cls = pico8_pm_struct_class();
    const pico8_struct_class_t *hj = pico8_hj_struct_class();
    pico8_vec_t p = vec_xy(40, 50);
    pico8_vec_t ra = vec_xy(7700, 7600);
    void *a;
    size_t i;
    int j;

    start_game(EMPTY);
    a = build_protostar(&p, &ra, "s77");
    assert(is_a_pm_struct(a) && cls != hj && listed(pico8_cg("jo"), a) && !listed(pico8_cg("hj"), a));
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        int named = 0;
        for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
            named |= pico8_method_of(methods[i]) == j;
        }
        assert((cls->methods[j] != NULL) == named);
        assert(cls->methods[j] == hj->methods[j]);
    }
    assert(cls->set_field == hj->set_field && cls->field != hj->field);
    for (i = 0; i < sizeof inherited / sizeof inherited[0]; i++) {
        pico8_value_t own;
        pico8_value_t through_hj;
        assert(cls->field(a, inherited[i], &own) && hj->field(a, inherited[i], &through_hj));
        assert(pico8_value_equal(own, through_hj));
    }
    assert(pico8_pm_ready());

    must_abort(write_ng, a);
    must_abort(write_fz, a);
    must_abort(write_rr, a);
    must_abort(write_py, a);
    must_abort(write_hp, a);
    must_abort(write_id, a);
    must_abort(write_qw, a);
    must_abort(write_ht, a);
    must_abort(build_with_unknown_key, NULL);
    must_abort(build_with_positional_key, NULL);
    for (i = 0; i < sizeof written / sizeof written[0]; i++) {
        pico8_value_t v;
        pico8_entity_set_field(a, written[i], pico8_value_fixed(pico8_fixed_from_int((int)i + 1)));
        assert(lookup(a, written[i], &v) && v.kind == PICO8_VALUE_FIXED &&
               v.as.fixed == pico8_fixed_from_int((int)i + 1));
    }
    {
        pico8_pm_t *bare = pico8_pm_new(NULL);
        assert(is_a_pm_struct(bare) && !has(bare, "id") && !has(bare, "ra") && has(bare, "oe") &&
               list_size(bare, "qy") == 0 && listed(pico8_cg("jo"), bare));
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
    pid_t session;
    pid_t recorded;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    session = spawn(test_session);
    recorded = spawn(test_recorded);
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_pm_is_a_struct);
    reap(recorded);
    reap(session);

    printf("test_pico8_pm: all tests passed\n");
    return 0;
}
