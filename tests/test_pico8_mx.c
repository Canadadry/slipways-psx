/* The synthesizer `mx`, read and written only through the dual-form field
 * API and `pico8_mx_default()`, `pico8_hj_default()`, `pico8_qh_default()`
 * and `pico8_jo_default()`, built as the shop builds it (`ie:ldown` on
 * `gx[5]`, `id`) and driven through the real `mv`/`dg`/`dc` and a slipway's
 * `ey`. The frozen values were taken on the code where `mx` was still a
 * class instance and must not be re-taken; only test_mx_is_a_struct looks at
 * the form. */

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
#include "../src/pico8_slipway.h"
#include "../src/pico8_research.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define EMPTY "tests/fixtures/empty_planet_cleared_events.json"
#define RECORDED "tests/fixtures/events_recorded_1790198294.json"

#define BUILT_HASH 0x2A676693u
#define LIFE_HASH 0x902C2D62u
#define SYNTHESIS_HASH 0x2DBF1611u
#define RECORDED_FRAMES 19832
#define RECORDED_SYNTHESIZERS 3
#define RECORDED_ACCEPTED 2
#define RECORDED_HASH 0x5900CBD8u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

/* mx's own `ht`, then hj's 12 emitter defaults and `qy`, then the 10
 * members of `qh`, then the 31 of `jo`. */
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
static const char *const mx_tables[] = {"nu", "px", "ht", "k_"};
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

/* The research icon whose `ke` is `kind`. */
static void *icon_of(const pico8_entity_kind_t *kind) {
    const pico8_list_t *hb = pico8_cv("hb");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i].as.ref;
        pico8_value_t ke;
        if (lookup(e, "ke", &ke) && ke.kind == PICO8_VALUE_REF && ke.as.ref == kind) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the icon of `kind`, as the shop does. */
static void *build(const pico8_entity_kind_t *kind) {
    void *icon = icon_of(kind);
    pico8_value_t made;

    assert(icon != NULL);
    made = call(icon, "ldown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(made.kind == PICO8_VALUE_REF && made.as.ref != NULL);
    return made.as.ref;
}

static void *build_synthesizer(void) {
    return build(pico8_mx_kind());
}

/* `mx` sets its own `px`: an entity whose `px` is the very table of mx's
 * class is a synthesizer, recognized by what it reads, not by its form. */
static int is_mx(const void *e) {
    return e != NULL && get_ref(e, "px") == class_ref(pico8_mx_default, "px");
}

/* The entry count of the `ht` table, -1 when absent. */
static int ht_size(const void *e) {
    const pico8_ob_table_t *ht = get_ref(e, "ht");
    return ht == NULL ? -1 : ht->count;
}

static int ht_is_the_class_one(const void *e) {
    return get_ref(e, "ht") == class_ref(pico8_mx_default, "ht");
}

/* A synthesizer as `ie:ldown`, `f_` and `jo:pq` left it: its scalars (the
 * 12 emitter defaults among them), which of its own, hj's, qh's and jo's
 * names it has, its methods (hj's very functions but its own `kf`), which
 * default tables it shares with `mx`, `qh` and `jo`, and where `lt`
 * registered it -- in `cg.jo`/`np`/`prober`, never in `cg.qh` or `cg.hj`. */
static uint32_t fold_synthesizer(uint32_t h, const void *e) {
    static const char *const scalars[] = {"id", "me", "t",  "rr", "qh", "hp", "s",  "od", "py",
                                          "ki", "pn", "pk", "fc", "du", "qo", "ro", "qg", "q_",
                                          "dq", "fp", "dw", "mi", "hf", "hs", "fa", "ma", "ca"};
    static const char *const not_mx[] = {"nl", "qw", "ng", "fz", "et"};
    const pico8_struct_class_t *hj = pico8_hj_struct_class();
    pico8_value_t gb5;
    size_t i;

    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fold_vec(h, e, "gq");
    h = fnv(h, (uint32_t)has(e, "ht"));
    h = fnv(h, (uint32_t)ht_size(e));
    for (i = 0; i < sizeof hj_members / sizeof hj_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, hj_members[i]));
    }
    for (i = 0; i < sizeof qh_members / sizeof qh_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, qh_members[i]));
    }
    for (i = 0; i < sizeof jo_members / sizeof jo_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, jo_members[i]));
    }
    for (i = 0; i < sizeof not_mx / sizeof not_mx[0]; i++) {
        assert(!has(e, not_mx[i]));
    }
    assert(list_size(e, "qy") == 0);
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        pico8_value_function_t got = method(e, methods[i]);
        if (strcmp(methods[i], "kf") == 0) {
            assert(got != hj->methods[PICO8_METHOD_KF]);
            assert(got != pico8_jo_struct_class()->methods[PICO8_METHOD_KF]);
        } else {
            assert(got == hj->methods[pico8_method_of(methods[i])]);
        }
    }
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(e, absent[i]));
    }
    for (i = 0; i < sizeof mx_tables / sizeof mx_tables[0]; i++) {
        assert(get_ref(e, mx_tables[i]) == class_ref(pico8_mx_default, mx_tables[i]));
    }
    assert(get_ref(e, "nu") != class_ref(pico8_hj_default, "nu"));
    assert(get_ref(e, "k_") != class_ref(pico8_hj_default, "k_"));
    assert(get_ref(e, "ba") == class_ref(pico8_qh_default, "ba"));
    for (i = 0; i < sizeof jo_tables / sizeof jo_tables[0]; i++) {
        assert(get_ref(e, jo_tables[i]) == class_ref(pico8_jo_default, jo_tables[i]));
    }
    assert(get_ref(e, "kw") != NULL && get_ref(e, "kw") != class_ref(pico8_jo_default, "kw"));
    assert(pico8_ob_table_get_index(pico8_gb(), 5, &gb5) && gb5.kind == PICO8_VALUE_REF);
    assert(get_ref(e, "hn") == gb5.as.ref);
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
    return h;
}

/* Two synthesizers built by the shop's `ie:ldown` on `gx[5]`, each with a
 * `qy` of its own. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game(EMPTY);
    a = build_synthesizer();
    b = build_synthesizer();
    assert(a != b && is_mx(a) && is_mx(b));
    assert(get_ref(a, "qy") != NULL && get_ref(a, "qy") != get_ref(b, "qy"));
    h = fold_synthesizer(h, a);
    h = fold_synthesizer(h, b);
    check("built", h, BUILT_HASH);
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = pico8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const void *e) {
    const pico8_vec_t *ra = get_ref(e, "ra");
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the synthesizer. */
static uint32_t fold_dg(uint32_t h, const void *e) {
    h = fold_ra_y(h, e);
    pico8_dg();
    h = fold_field(h, e, "t");
    return fold_ra_y(h, e);
}

/* What `bb`'s `dh`, `ib`, `gg` and `da` (qh false) give on a synthesizer. */
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

/* Placed on screen by `hq` (`bv`) and `ey` (`d_`); `dg` on one still in
 * "kz" (no `kz` method: nothing moves), then in "ly" (qh's `ly`, `na`);
 * `mm` (`g_.mm`, `pk` true) and `ez`; `kj` through `dc` with `mi`=1, then
 * `mi`=2 through the generic writer (`ei` moves and draws the particles);
 * `dh`/`ib`/`gg`/`da`. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *placed;
    void *idle;
    pico8_value_t mp;
    const pico8_vec_t *mo_ra;
    int i;

    start_game(EMPTY);
    idle = build_synthesizer();
    h = fold_field(h, idle, "me");
    h = fold_dg(h, idle);

    placed = build_synthesizer();
    mo_ra = get_ref(pico8_mo(), "ra");
    assert(mo_ra != NULL);
    mp = vec_ref(pico8_vec_add(*mo_ra, vec_xy(12, 108)));
    (void)call(placed, "hq", 2, mp, vec_ref(vec_xy(64, 64)));
    assert(get_ref(placed, "ra") == mp.as.ref);
    set_credits(200);
    h = fold_value(h, call(placed, "ey", 1, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fold_field(h, placed, "me");
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
    check("life", h, LIFE_HASH);
}

/* A slipway from `qu` to the synthesizer, joined by its real `ey`: `jc:ey`
 * charges the player, then runs `kf` on both ends. */
static void *join(void *qu, void *synthesizer) {
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    void *jc;

    pico8_ob_table_set_name(overrides, "qu", pico8_value_ref(qu));
    jc = pico8_jc_new(overrides);
    pico8_ob_table_free(overrides);
    assert(jc != NULL);
    set_credits(999);
    (void)call(jc, "ey", 1, pico8_value_ref(synthesizer), pico8_value_ref(NULL));
    assert(listed(get_ref(synthesizer, "oe"), jc));
    return jc;
}

/* `mx:kf` reads the other end's `jy`: one row per resource, `gc` its
 * count of suppliers. */
static void give_row(void *e, int qe) {
    pico8_ob_table_t *row = pico8_ob_table_new();
    pico8_list_t *jy = malloc(sizeof *jy);

    pico8_ob_table_set_name(row, "gc", pico8_value_fixed(pico8_fixed_from_int(0)));
    pico8_ob_table_set_name(row, "qe", pico8_value_fixed(pico8_fixed_from_int(qe)));
    pico8_list_init(jy);
    pico8_list_add(jy, pico8_value_ref(row));
    pico8_entity_set_field(e, "jy", pico8_value_ref(jy));
}

static uint32_t fold_kf(uint32_t h, const void *e) {
    h = fnv(h, (uint32_t)ht_is_the_class_one(e));
    h = fnv(h, (uint32_t)ht_size(e));
    h = fold_kw(h, e);
    h = fold_field(h, e, "ma");
    h = fold_field(h, e, "fx");
    return fnv(h, (uint32_t)list_size(e, "oe"));
}

/* `mx:kf` on a slipway from an ascension whose `jy` lacks a resource of `ht`: nothing
 * but `jo:kf`; then on one whose unsupplied row is `qe`=20, one of `ht`:
 * `kw.qe` takes it and `ht` becomes a fresh empty table, the class one
 * untouched; a later slipway finds `ht` empty and leaves it as it is. A
 * second synthesizer still starts from the class `ht`. */
static void test_synthesis(void) {
    uint32_t h = 2166136261u;
    void *home;
    void *s;
    void *other;
    const void *fresh;
    pico8_value_t qe;

    start_game(EMPTY);
    home = build(pico8_hj_kind());
    s = build_synthesizer();
    assert(ht_is_the_class_one(s) && ht_size(s) == 4);

    give_row(home, 7);
    (void)join(home, s);
    assert(ht_is_the_class_one(s));
    h = fold_kf(h, s);
    h = fold_kf(h, home);

    give_row(home, 20);
    (void)join(home, s);
    assert(!ht_is_the_class_one(s) && ht_size(s) == 0);
    assert(pico8_ob_table_get_name(get_ref(s, "kw"), "qe", &qe) && qe.kind == PICO8_VALUE_FIXED &&
           qe.as.fixed == pico8_fixed_from_int(20));
    assert(((const pico8_ob_table_t *)class_ref(pico8_mx_default, "ht"))->count == 4);
    assert(!pico8_ob_table_get_name(class_ref(pico8_jo_default, "kw"), "qe", &qe) ||
           !(qe.kind == PICO8_VALUE_FIXED && qe.as.fixed == pico8_fixed_from_int(20)));
    h = fold_kf(h, s);
    fresh = get_ref(s, "ht");

    give_row(home, 21);
    (void)join(home, s);
    assert(get_ref(s, "ht") == fresh && ht_size(s) == 0);
    assert(pico8_ob_table_get_name(get_ref(s, "kw"), "qe", &qe) && qe.as.fixed == pico8_fixed_from_int(20));
    h = fold_kf(h, s);

    other = build_synthesizer();
    assert(ht_is_the_class_one(other) && ht_size(other) == 4);
    h = fold_kf(h, other);
    check("synthesis", h, SYNTHESIS_HASH);
}

/* Per frame, every synthesizer in `hv`: its rank and what the shop, `f_`,
 * `bv`, `d_`, `dg`, `hj:kj`, `mx:kf` and the economy write on it; plus the
 * screen. */
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
    h = fnv(h, (uint32_t)has(e, "m_"));
    h = fnv(h, (uint32_t)ht_is_the_class_one(e));
    return fnv(h, (uint32_t)ht_size(e));
}

#define MAX_SEEN 16

/* From the title screen, as a raylib recording starts. */
static void test_recorded(void) {
    uint32_t h = 2166136261u;
    void *seen[MAX_SEEN];
    int accepted[MAX_SEEN] = {0};
    int count = 0;
    int total_accepted = 0;
    int frames;
    int i;

    start_from_title(RECORDED);
    frames = input.frame_count;
    for (i = 0; i < frames; i++) {
        const pico8_list_t *hv;
        int j;

        play_frame();
        hv = pico8_hv();
        for (j = 0; j < hv->count; j++) {
            void *e = hv->items[j].as.ref;
            int k;

            if (!is_mx(e)) {
                continue;
            }
            for (k = 0; k < count && seen[k] != e; k++) {
            }
            if (k == count) {
                assert(count < MAX_SEEN);
                seen[count++] = e;
            }
            if (!accepted[k] && !ht_is_the_class_one(e)) {
                accepted[k] = 1;
                total_accepted++;
            }
            h = fold_live(fnv(fnv(h, (uint32_t)j), (uint32_t)k), e);
        }
        h = fnv(h, screen_hash());
    }
    check_int("recorded frames", frames, RECORDED_FRAMES);
    check_int("recorded synthesizers", count, RECORDED_SYNTHESIZERS);
    check_int("recorded accepted", total_accepted, RECORDED_ACCEPTED);
    check("recorded", h, RECORDED_HASH);
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

static void write_qw(void *e) {
    write_fixed(e, "qw");
}

static void write_nl(void *e) {
    write_fixed(e, "nl");
}

static void build_with_unknown_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "hp", pico8_value_fixed(pico8_fixed_from_int(1)));
    (void)pico8_mx_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_mx_new(b);
}

static int is_an_mx_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_mx_struct_class();
}

/* Built by `ie:ldown` or pico8_mx_new, a synthesizer is a struct whose
 * descriptor holds hj's very entries but its own `kf`; hj's reader sees
 * the 17 defaults of mx that fall on an inherited member; it takes exactly
 * hj's 21 names plus `ht`, and a construction key other than `id` stops
 * the run. No class of mx's chain installs a function any more. */
static void test_mx_is_a_struct(void) {
    static const char *const inherited[] = {"nu", "rr", "hp", "s",  "px", "od", "py", "ki", "pn",
                                            "pk", "fc", "k_", "du", "qo", "ro", "qg", "q_"};
    const pico8_struct_class_t *cls = pico8_mx_struct_class();
    const pico8_struct_class_t *hj = pico8_hj_struct_class();
    pico8_value_t v;
    void *a;
    size_t i;
    int j;

    start_game(EMPTY);
    a = build_synthesizer();
    assert(is_an_mx_struct(a) && cls != hj && listed(pico8_cg("jo"), a) && !listed(pico8_cg("hj"), a));
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        int named = 0;
        for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
            named |= pico8_method_of(methods[i]) == j;
        }
        assert((cls->methods[j] != NULL) == named);
        if (j != PICO8_METHOD_KF) {
            assert(cls->methods[j] == hj->methods[j]);
        }
    }
    assert(cls->methods[PICO8_METHOD_KF] != hj->methods[PICO8_METHOD_KF] &&
           cls->methods[PICO8_METHOD_KF] != pico8_jo_struct_class()->methods[PICO8_METHOD_KF]);
    assert(cls->field != hj->field && cls->set_field != hj->set_field);
    for (i = 0; i < sizeof inherited / sizeof inherited[0]; i++) {
        pico8_value_t own;
        pico8_value_t through_hj;
        assert(cls->field(a, inherited[i], &own) && hj->field(a, inherited[i], &through_hj));
        assert(pico8_value_equal(own, through_hj));
    }
    assert(!hj->field(a, "ht", &v) && cls->field(a, "ht", &v));
    assert(pico8_mx_ready());
    assert(!pico8_mx_default("kf", &v));
    assert(!pico8_hj_default("kj", &v));
    assert(!pico8_hj_default("mm", &v));
    assert(pico8_hj_ready());

    must_abort(write_ng, a);
    must_abort(write_fz, a);
    must_abort(write_qw, a);
    must_abort(write_nl, a);
    must_abort(build_with_unknown_key, NULL);
    must_abort(build_with_positional_key, NULL);
    {
        pico8_mx_t *bare = pico8_mx_new(NULL);
        assert(is_an_mx_struct(bare) && !has(bare, "id") && has(bare, "oe") && has(bare, "kw") &&
               list_size(bare, "qy") == 0 && listed(pico8_cg("jo"), bare));
        pico8_entity_set_field(bare, "ht", pico8_value_ref(NULL));
        assert(!cls->field(bare, "ht", &v));
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
    recorded = spawn(test_recorded);
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_synthesis);
    in_fresh_child(test_mx_is_a_struct);
    reap(recorded);

    printf("test_pico8_mx: all tests passed\n");
    return 0;
}
