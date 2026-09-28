/* The slipgate `nr`, read and written only through the dual-form field API
 * and `pico8_nr_default()`, `pico8_qh_default()` and `pico8_jo_default()`,
 * built as the shop builds it (`ie:ldown` on `gx[2]`, `id`) and driven
 * through the real `mv`/`dg`/`dc`. Its daughter `fr` runs nr's four methods,
 * so the replays follow both. The frozen values were taken on the code where
 * `nr` was still a class instance and must not be re-taken; only
 * test_nr_is_a_struct looks at the form. */

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
#define HUB "tests/fixtures/trading_hub_session_events.json"

#define BUILT_HASH 0x4194DF7Bu
#define LIFE_HASH 0x6D8FAD96u
#define SESSION_FRAMES 116045
#define SESSION_NR 5
#define SESSION_FR 5
#define SESSION_HASH 0x1FB92F93u
#define HUB_FRAMES 4921
#define HUB_NR 1
#define HUB_FR 0
#define HUB_HASH 0x65DBE61Eu

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

/* nr's own `hz` and `rr`, then the 10 members of `qh`, then the 31 of
 * `jo`. */
static const char *const nr_members[] = {"hz", "rr"};
static const char *const qh_members[] = {"hp", "px", "dq", "qh", "od", "fp", "dw", "s", "ba", "et"};
static const char *const jo_members[] = {"nu", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq",
                                         "ca", "em", "id", "p",  "ra", "oe", "hn", "ep", "jy", "nk", "fx",
                                         "lq", "fo", "jh", "p_", "kr", "cc", "by", "mi", "m_"};
static const char *const methods[] = {"hd", "kj", "ldown", "hb", "gg", "gv", "hk", "kf",
                                      "cn", "ib", "ll", "hq", "ev", "ey", "ly"};
static const char *const own_methods[] = {"gv", "ldown", "hb", "cn"};
static const char *const absent[] = {"ph", "kz", "rdown", "md", "ol", "pr", "eb", "mm"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"nr", "np", "prober"};
static const char *const nr_tables[] = {"nu", "kq", "hz"};
static const char *const qh_tables[] = {"px", "ba"};
static const char *const jo_tables[] = {"em", "ka", "dz", "cq", "gq"};

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

/* The instance reader keeps a field written nil (`ep`: TROU-9), the struct
 * reader reads a nil member as absent: nil and absent are one state here. */
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

static void *build_slipgate(void) {
    return build(pico8_nr_kind());
}

enum { NR, FR };

/* `hz` is never written on an entity: one whose `hz` is the very table of
 * nr's class is a slipgate, of fr's class an infraspace gate, whatever its
 * form. -1 for anything else. */
static int gate_kind(const void *e) {
    const void *hz = e == NULL ? NULL : get_ref(e, "hz");
    if (hz == NULL) {
        return -1;
    }
    if (hz == class_ref(pico8_nr_default, "hz")) {
        return NR;
    }
    if (hz == class_ref(pico8_fr_default, "hz")) {
        return FR;
    }
    return -1;
}

/* A slipgate as `ie:ldown` and `jo:pq` left it: its scalars, which of its
 * own, qh's and jo's names it has, its methods (qh's very functions but its
 * own `gv`, `ldown`, `hb`, `cn`), which default tables it shares with `nr`,
 * `qh` and `jo`, and where `lt` registered it -- in `cg.nr`/`np`/`prober`,
 * never in `cg.jo` or `cg.qh`. */
static uint32_t fold_slipgate(uint32_t h, const void *e) {
    static const char *const scalars[] = {"id", "me", "t",  "rr", "hp", "s",  "od", "fp", "dw",
                                          "dq", "qh", "mi", "hf", "hs", "fa", "ma", "ca"};
    static const char *const not_nr[] = {"mt", "qu", "qw", "ht", "qy", "et"};
    const pico8_struct_class_t *qh = pico8_qh_struct_class();
    pico8_value_t gb1;
    size_t i;

    assert(gate_kind(e) == NR);
    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fold_vec(h, e, "gq");
    h = fnv(h, (uint32_t)((const pico8_ob_table_t *)get_ref(e, "hz"))->count);
    for (i = 0; i < sizeof nr_members / sizeof nr_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, nr_members[i]));
    }
    for (i = 0; i < sizeof qh_members / sizeof qh_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, qh_members[i]));
    }
    for (i = 0; i < sizeof jo_members / sizeof jo_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, jo_members[i]));
    }
    for (i = 0; i < sizeof not_nr / sizeof not_nr[0]; i++) {
        assert(!has(e, not_nr[i]));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        pico8_value_function_t got = method(e, methods[i]);
        size_t k;
        int own = 0;
        for (k = 0; k < sizeof own_methods / sizeof own_methods[0]; k++) {
            own |= strcmp(methods[i], own_methods[k]) == 0;
        }
        if (own) {
            assert(got == pico8_nr_struct_class()->methods[pico8_method_of(methods[i])]);
            assert(got != qh->methods[pico8_method_of(methods[i])]);
            assert(got != pico8_jo_struct_class()->methods[pico8_method_of(methods[i])]);
        } else {
            assert(got == qh->methods[pico8_method_of(methods[i])]);
        }
    }
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(e, absent[i]));
    }
    for (i = 0; i < sizeof nr_tables / sizeof nr_tables[0]; i++) {
        assert(get_ref(e, nr_tables[i]) == class_ref(pico8_nr_default, nr_tables[i]));
    }
    assert(get_ref(e, "nu") != class_ref(pico8_qh_default, "nu"));
    assert(get_ref(e, "kq") != class_ref(pico8_jo_default, "kq"));
    for (i = 0; i < sizeof qh_tables / sizeof qh_tables[0]; i++) {
        assert(get_ref(e, qh_tables[i]) == class_ref(pico8_qh_default, qh_tables[i]));
    }
    for (i = 0; i < sizeof jo_tables / sizeof jo_tables[0]; i++) {
        assert(get_ref(e, jo_tables[i]) == class_ref(pico8_jo_default, jo_tables[i]));
    }
    assert(get_ref(e, "kw") != NULL && get_ref(e, "kw") != class_ref(pico8_jo_default, "kw"));
    assert(pico8_ob_table_get_index(pico8_gb(), 1, &gb1) && gb1.kind == PICO8_VALUE_REF);
    assert(get_ref(e, "hn") == gb1.as.ref);
    h = fnv(h, (uint32_t)list_size(e, "oe"));
    h = fnv(h, (uint32_t)list_size(e, "jy"));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cv(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cg(tags[i]), e));
    }
    assert(listed(pico8_cg("nr"), e) && !listed(pico8_cg("jo"), e) && !listed(pico8_cg("qh"), e));
    return h;
}

/* Two slipgates built by the shop's `ie:ldown` on `gx[2]`. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game(EMPTY);
    a = build_slipgate();
    b = build_slipgate();
    assert(a != b && get_ref(a, "oe") != get_ref(b, "oe") && get_ref(a, "kw") != get_ref(b, "kw"));
    h = fold_slipgate(h, a);
    h = fold_slipgate(h, b);
    check("built", h, BUILT_HASH);
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = pico8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const void *e) {
    const pico8_vec_t *ra = get_ref(e, "ra");
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the slipgate. */
static uint32_t fold_dg(uint32_t h, const void *e) {
    h = fold_ra_y(h, e);
    pico8_dg();
    h = fold_field(h, e, "t");
    return fold_ra_y(h, e);
}

/* What `bb`'s `dh`, `ib`, `gg` and `da` give on a slipgate. */
static uint32_t fold_economy(uint32_t h, void *e) {
    pico8_value_t ib = call(e, "ib", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_value(h, ib);
    h = fold_value(h, call(e, "gg", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fnv(h, (uint32_t)pico8_da(e));
    return fold_field(h, e, "mi");
}

/* A list of `n` copies of `item`, for `oe`: `gv`, `ldown` and `hb` only
 * count it. */
static pico8_value_t list_of(int n, void *item) {
    pico8_list_t *l = malloc(sizeof *l);
    int i;
    pico8_list_init(l);
    for (i = 0; i < n; i++) {
        pico8_list_add(l, pico8_value_ref(item));
    }
    return pico8_value_ref(l);
}

/* Placed on screen by `hq` (`bv`) and `ey` (`d_`); `dg` on one still in
 * "kz", then in "ly"; `kj` through `dc`; `gv` against no argument, another
 * slipgate and a slipway drawn from itself; `ldown` with no link (a `jc`
 * whose `qu` is the slipgate) and with 3 (refused); `hb` with 0 to 3
 * links; an `fr`'s `hb`; `pico8_nr_new` with no key, then `p` and `ra`. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *placed;
    void *idle;
    void *fr;
    void *bare;
    pico8_value_t mp;
    pico8_value_t jc;
    pico8_value_t oe;
    const pico8_vec_t *mo_ra;
    int i;

    start_game(EMPTY);
    idle = build_slipgate();
    h = fold_field(h, idle, "me");
    h = fold_dg(h, idle);

    placed = build_slipgate();
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
    for (i = 0; i < 2; i++) {
        pico8_dc("kj");
        h = fnv(h, screen_hash());
    }

    h = fold_value(h, call(placed, "gv", 1, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fold_value(h, call(placed, "gv", 1, pico8_value_ref(idle), pico8_value_ref(NULL)));
    jc = call(placed, "ldown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(jc.kind == PICO8_VALUE_REF && jc.as.ref != NULL);
    assert(get_ref(jc.as.ref, "qu") == placed);
    h = fnv(h, (uint32_t)pico8_type_tag_of(jc.as.ref));
    h = fold_value(h, call(placed, "gv", 1, jc, pico8_value_ref(NULL)));
    h = fold_value(h, call(idle, "gv", 1, jc, pico8_value_ref(NULL)));

    oe = get(placed, "oe");
    for (i = 0; i <= 3; i++) {
        pico8_entity_set_field(placed, "oe", list_of(i, idle));
        (void)call(placed, "hb", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
        h = fold_field(h, placed, "s");
        h = fold_value(h, call(placed, "gv", 1, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    }
    assert(get(placed, "s").as.fixed == pico8_fixed_from_int(168));
    h = fold_value(h, call(placed, "ldown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    pico8_entity_set_field(placed, "oe", oe);
    (void)call(placed, "hb", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    h = fold_field(h, placed, "s");
    h = fold_economy(h, placed);

    fr = build(pico8_fr_kind());
    assert(gate_kind(fr) == FR);
    h = fold_field(h, fr, "s");
    (void)call(fr, "hb", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    h = fold_field(h, fr, "s");
    assert(get(fr, "s").as.fixed == pico8_fixed_from_int(78));

    bare = pico8_nr_new(NULL);
    assert(gate_kind(bare) == NR && !has(bare, "id") && has(bare, "oe") && has(bare, "kw"));
    h = fnv(h, (uint32_t)rank_in(pico8_cg("nr"), bare));
    {
        pico8_ob_table_t *b = pico8_ob_table_new();
        pico8_value_t p = vec_ref(vec_xy(3, 4));
        pico8_value_t ra = vec_ref(vec_xy(5, 6));
        void *with;
        pico8_ob_table_set_name(b, "p", p);
        pico8_ob_table_set_name(b, "ra", ra);
        with = pico8_nr_new(b);
        pico8_ob_table_free(b);
        assert(get_ref(with, "p") == p.as.ref && get_ref(with, "ra") == ra.as.ref && !has(with, "id"));
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), with));
    }
    check("life", h, LIFE_HASH);
}

/* Per frame, every slipgate and infraspace gate in `hv`: its rank, its
 * kind, and what the shop, `bv`, `d_`, `dg`, `hb`, the slipways and the
 * probes write on it; plus the screen. */
static uint32_t fold_live(uint32_t h, const void *e) {
    h = fold_field(h, e, "id");
    h = fold_field(h, e, "me");
    h = fold_field(h, e, "t");
    h = fold_vec(h, e, "ra");
    h = fold_field(h, e, "s");
    h = fold_field(h, e, "mi");
    h = fnv(h, (uint32_t)list_size(e, "oe"));
    h = fnv(h, (uint32_t)list_size(e, "jy"));
    h = fnv(h, (uint32_t)has(e, "m_"));
    return fnv(h, (uint32_t)has(e, "et"));
}

#define MAX_SEEN 32

typedef struct {
    const char *label;
    const char *events;
    int want_frames;
    int want_nr;
    int want_fr;
    uint32_t want_hash;
} replay_t;

/* From the title screen, as a raylib recording starts. */
static void replay(const replay_t *r) {
    uint32_t h = 2166136261u;
    void *seen[MAX_SEEN];
    int counts[2] = {0, 0};
    int count = 0;
    int frames;
    int i;
    char label[64];

    start_from_title(r->events);
    frames = input.frame_count;
    for (i = 0; i < frames; i++) {
        const pico8_list_t *hv;
        int j;

        play_frame();
        hv = pico8_hv();
        for (j = 0; j < hv->count; j++) {
            void *e = hv->items[j].as.ref;
            int kind = gate_kind(e);
            int k;

            if (kind < 0) {
                continue;
            }
            for (k = 0; k < count && seen[k] != e; k++) {
            }
            if (k == count) {
                assert(count < MAX_SEEN);
                seen[count++] = e;
                counts[kind]++;
            }
            h = fold_live(fnv(fnv(fnv(h, (uint32_t)j), (uint32_t)k), (uint32_t)kind), e);
        }
        h = fnv(h, screen_hash());
    }
    snprintf(label, sizeof label, "%s frames", r->label);
    check_int(label, frames, r->want_frames);
    snprintf(label, sizeof label, "%s nr", r->label);
    check_int(label, counts[NR], r->want_nr);
    snprintf(label, sizeof label, "%s fr", r->label);
    check_int(label, counts[FR], r->want_fr);
    check(r->label, h, r->want_hash);
}

static void test_session(void) {
    static const replay_t r = {"session", SESSION, SESSION_FRAMES, SESSION_NR, SESSION_FR, SESSION_HASH};
    replay(&r);
}

static void test_hub(void) {
    static const replay_t r = {"hub", HUB, HUB_FRAMES, HUB_NR, HUB_FR, HUB_HASH};
    replay(&r);
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

static void write_hz(void *e) {
    write_fixed(e, "hz");
}

static void write_rr(void *e) {
    write_fixed(e, "rr");
}

static void write_mt(void *e) {
    write_fixed(e, "mt");
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
    (void)pico8_nr_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_nr_new(b);
}

static int is_an_nr_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_nr_struct_class();
}

/* Built by `ie:ldown` or pico8_nr_new, a slipgate is a struct whose
 * descriptor holds qh's very entries but its own `gv`, `ldown`, `hb`, `cn`,
 * which no class of its chain installs any more; qh's reader sees the 6 defaults of nr that fall on an
 * inherited member; it takes exactly qh's 20 names plus `s`, and a
 * construction key other than `id`, `p`, `ra` stops the run. */
static void test_nr_is_a_struct(void) {
    static const char *const written[] = {"by", "cc", "fo", "kr", "m_", "ma", "mi", "et", "s"};
    static const char *const inherited[] = {"nu", "kq", "od", "fp", "s", "dw"};
    const pico8_struct_class_t *cls = pico8_nr_struct_class();
    const pico8_struct_class_t *qh = pico8_qh_struct_class();
    pico8_value_t v;
    void *a;
    void *fr;
    size_t i;
    int j;

    start_game(EMPTY);
    a = build_slipgate();
    assert(is_an_nr_struct(a) && cls != qh && listed(pico8_cg("nr"), a));
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        int named = 0;
        int own = 0;
        for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
            named |= pico8_method_of(methods[i]) == j;
        }
        for (i = 0; i < sizeof own_methods / sizeof own_methods[0]; i++) {
            if (pico8_method_of(own_methods[i]) == j) {
                own = 1;
                assert(!pico8_nr_default(own_methods[i], &v));
            }
        }
        assert((cls->methods[j] != NULL) == named);
        if (own) {
            assert(cls->methods[j] != qh->methods[j]);
        } else {
            assert(cls->methods[j] == qh->methods[j]);
        }
    }
    assert(cls->field != qh->field && cls->set_field != qh->set_field);
    for (i = 0; i < sizeof inherited / sizeof inherited[0]; i++) {
        pico8_value_t own;
        pico8_value_t through_qh;
        assert(cls->field(a, inherited[i], &own) && qh->field(a, inherited[i], &through_qh));
        assert(pico8_value_equal(own, through_qh) &&
               pico8_value_equal(own, class_value(pico8_nr_default, inherited[i])));
    }
    assert(!qh->field(a, "hz", &v) && cls->field(a, "hz", &v) && !qh->field(a, "rr", &v) && cls->field(a, "rr", &v));
    assert(pico8_nr_ready() && pico8_fr_ready());

    must_abort(write_hz, a);
    must_abort(write_rr, a);
    must_abort(write_mt, a);
    must_abort(write_qw, a);
    must_abort(write_ht, a);
    must_abort(build_with_unknown_key, NULL);
    must_abort(build_with_positional_key, NULL);
    for (i = 0; i < sizeof written / sizeof written[0]; i++) {
        pico8_entity_set_field(a, written[i], pico8_value_fixed(pico8_fixed_from_int((int)i + 1)));
        assert(lookup(a, written[i], &v) && v.kind == PICO8_VALUE_FIXED &&
               v.as.fixed == pico8_fixed_from_int((int)i + 1));
    }
    {
        pico8_nr_t *bare = pico8_nr_new(NULL);
        assert(is_an_nr_struct(bare) && !has(bare, "id") && has(bare, "oe") && has(bare, "kw"));
        bare->hz = pico8_value_ref(NULL);
        bare->rr = pico8_value_ref(NULL);
        assert(!cls->field(bare, "hz", &v) && !cls->field(bare, "rr", &v));
    }
    fr = build(pico8_fr_kind());
    assert(pico8_type_tag_of(fr) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)fr)->cls == pico8_fr_struct_class());
    (void)call(fr, "hb", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(get(fr, "s").as.fixed == pico8_fixed_from_int(78));
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
    pid_t hub;

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
    hub = spawn(test_hub);
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_nr_is_a_struct);
    reap(hub);
    reap(session);

    printf("test_pico8_nr: all tests passed\n");
    return 0;
}
