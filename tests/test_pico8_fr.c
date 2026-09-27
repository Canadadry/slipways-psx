/* The infraspace gate `fr`, read and written only through the dual-form
 * field API and `pico8_fr_default()`, `pico8_nr_default()`,
 * `pico8_qh_default()` and `pico8_jo_default()` for spec defaults, built as
 * the shop builds it (`ie:ldown` on `gx[3]`, `id`) and driven through the
 * real `mv`/`dg`/`dc`. Its methods are compared to those of a slipgate built
 * in the same game. The frozen values were taken on the code where `fr` was
 * still a class instance and must not be re-taken. */

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

#define BUILT_HASH 0xA5047E01u
#define LIFE_HASH 0x8B3BFBB2u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

/* fr's own `mt`, then nr's `hz` and `rr`, the 10 members of `qh`, the 31
 * of `jo`. */
static const char *const gate_members[] = {"mt", "hz", "rr"};
static const char *const qh_members[] = {"hp", "px", "dq", "qh", "od", "fp", "dw", "s", "ba", "et"};
static const char *const jo_members[] = {"nu", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq",
                                         "ca", "em", "id", "p",  "ra", "oe", "hn", "ep", "jy", "nk", "fx",
                                         "lq", "fo", "jh", "p_", "kr", "cc", "by", "mi", "m_"};
static const char *const methods[] = {"hd", "kj", "ldown", "hb", "gg", "gv", "hk", "kf",
                                      "cn", "ib", "ll", "hq", "ev", "ey", "ly"};
static const char *const absent[] = {"ph", "kz", "rdown", "md", "ol", "pr", "eb", "mm"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"nr", "np", "prober"};
static const char *const nr_tables[] = {"nu", "kq"};
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

static void start_game(const char *events) {
    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_cheat_init(&cheat);
    pico8_eu(pico8_js_new(bo_row(2)));
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

/* `hz` is never written on an entity: one whose `hz` is the very table of
 * fr's class is an infraspace gate, whatever its form. */
static int is_a_gate(const void *e) {
    return get_ref(e, "hz") == class_ref(pico8_fr_default, "hz");
}

static void *build_gate(void) {
    void *e = build(pico8_fr_kind());
    assert(is_a_gate(e));
    return e;
}

/* A gate as `ie:ldown` and `jo:pq` left it: its scalars, which of its own,
 * nr's, qh's and jo's names it has, its methods (a slipgate's very ones),
 * which default tables it shares with `fr`, `nr`, `qh` and `jo`, and where
 * `lt` registered it -- in `cg.nr`/`np`/`prober`, never in `cg.jo` or
 * `cg.qh`. */
static uint32_t fold_gate(uint32_t h, const void *e, const void *slipgate) {
    static const char *const scalars[] = {"id", "me", "t",  "rr", "mt", "hp", "s",  "od", "fp", "dw",
                                          "dq", "qh", "mi", "hf", "hs", "fa", "ma", "ca"};
    static const char *const not_fr[] = {"qu", "qw", "ht", "qy", "et"};
    pico8_value_t gb1;
    size_t i;

    assert(is_a_gate(e) && !is_a_gate(slipgate));
    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fold_vec(h, e, "gq");
    h = fnv(h, (uint32_t)((const pico8_ob_table_t *)get_ref(e, "hz"))->count);
    for (i = 0; i < sizeof gate_members / sizeof gate_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, gate_members[i]));
    }
    for (i = 0; i < sizeof qh_members / sizeof qh_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, qh_members[i]));
    }
    for (i = 0; i < sizeof jo_members / sizeof jo_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, jo_members[i]));
    }
    for (i = 0; i < sizeof not_fr / sizeof not_fr[0]; i++) {
        assert(!has(e, not_fr[i]));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        assert(method(e, methods[i]) == method(slipgate, methods[i]));
    }
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(e, absent[i]));
    }
    assert(get_ref(e, "hz") != class_ref(pico8_nr_default, "hz"));
    for (i = 0; i < sizeof nr_tables / sizeof nr_tables[0]; i++) {
        assert(get_ref(e, nr_tables[i]) == class_ref(pico8_nr_default, nr_tables[i]));
    }
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

/* Two infraspace gates built by the shop's `ie:ldown` on `gx[3]`, around a
 * slipgate. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *slipgate;
    void *b;

    start_game(EMPTY);
    a = build_gate();
    slipgate = build(pico8_nr_kind());
    b = build_gate();
    assert(a != b && get_ref(a, "oe") != get_ref(b, "oe") && get_ref(a, "kw") != get_ref(b, "kw"));
    h = fold_gate(h, a, slipgate);
    h = fold_gate(h, b, slipgate);
    check("built", h, BUILT_HASH);
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = pico8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const void *e) {
    const pico8_vec_t *ra = get_ref(e, "ra");
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the gate. */
static uint32_t fold_dg(uint32_t h, const void *e) {
    h = fold_ra_y(h, e);
    pico8_dg();
    h = fold_field(h, e, "t");
    return fold_ra_y(h, e);
}

/* What `bb`'s `dh`, `ib`, `gg` and `da` give on a gate. */
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

/* `hq` then `ey`, as the player drops a planet at `offset` from `mo`. */
static void place(void *e, int x, int y) {
    const pico8_vec_t *mo_ra = get_ref(pico8_mo(), "ra");
    pico8_value_t mp;

    assert(mo_ra != NULL);
    mp = vec_ref(pico8_vec_add(*mo_ra, vec_xy(x, y)));
    (void)call(e, "hq", 2, mp, vec_ref(vec_xy(64, 64)));
    assert(get_ref(e, "ra") == mp.as.ref);
}

/* A slipway drawn from `from` by its `ldown`, aimed by `jc:hq` at a point
 * with no planet: its `mt` (then `fa`) comes from `from` alone. */
static uint32_t fold_route(uint32_t h, void *from) {
    pico8_value_t jc = call(from, "ldown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    const pico8_vec_t *ra = get_ref(from, "ra");

    assert(jc.kind == PICO8_VALUE_REF && jc.as.ref != NULL && get_ref(jc.as.ref, "qu") == from);
    assert(ra != NULL);
    (void)call(jc.as.ref, "hq", 1, vec_ref(pico8_vec_add(*ra, vec_xy(20, 0))), pico8_value_ref(NULL));
    assert(has(jc.as.ref, "mt") == has(from, "mt"));
    assert(get(jc.as.ref, "fa").as.fixed == pico8_fixed_from_int(has(from, "mt") ? 2 : 4));
    h = fold_field(h, jc.as.ref, "mt");
    return fold_field(h, jc.as.ref, "fa");
}

/* Placed on screen by `hq` (`bv`) and `ey` (`d_`); `dg` on one still in
 * "kz", then in "ly"; `kj` through `dc`; `gv` against no argument, another
 * gate and a slipway drawn from itself; `ldown` with no link (a `jc` whose
 * `qu` is the gate) and with 3 (refused); `hb` with 0 to 3 links; the `mt`
 * of a slipway drawn from a gate and from a slipgate. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *placed;
    void *idle;
    void *slipgate;
    pico8_value_t jc;
    pico8_value_t oe;
    int i;

    start_game(EMPTY);
    idle = build_gate();
    h = fold_field(h, idle, "me");
    h = fold_dg(h, idle);

    placed = build_gate();
    place(placed, 12, 108);
    set_credits(999);
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
    assert(get(placed, "s").as.fixed == pico8_fixed_from_int(76));
    h = fold_value(h, call(placed, "ldown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    pico8_entity_set_field(placed, "oe", oe);
    (void)call(placed, "hb", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    h = fold_field(h, placed, "s");
    assert(get(placed, "s").as.fixed == pico8_fixed_from_int(78));
    h = fold_economy(h, placed);

    h = fold_route(h, placed);
    assert(get(placed, "mt").as.fixed == pico8_fixed_from_int(1));
    slipgate = build(pico8_nr_kind());
    place(slipgate, 40, 108);
    h = fold_route(h, slipgate);
    assert(!has(slipgate, "mt"));
    check("life", h, LIFE_HASH);
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

static void write_mt(void *e) {
    write_fixed(e, "mt");
}

static void write_hz(void *e) {
    write_fixed(e, "hz");
}

static void write_rr(void *e) {
    write_fixed(e, "rr");
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
    pico8_ob_table_set_name(b, "p", pico8_value_ref(NULL));
    (void)pico8_fr_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_fr_new(b);
}

static int is_an_fr_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_fr_struct_class();
}

/* Built by `ie:ldown` or pico8_fr_new, a gate is a struct whose descriptor
 * holds nr's very entries and writer; nr's reader sees the 4 defaults of fr
 * that fall on an inherited member, not `mt`; it takes exactly nr's 21
 * names, and a construction key other than `id` stops the run. Neither
 * nr's nor qh's class installs a function any more. */
static void test_fr_is_a_struct(void) {
    static const char *const written[] = {"by", "cc", "ep", "fo", "fx", "hn", "jh", "jy", "kr", "kw", "lq",
                                          "m_", "ma", "mi", "nk", "oe", "p_", "ra", "p",  "et", "s"};
    static const char *const inherited[] = {"rr", "hz", "od", "s"};
    const pico8_struct_class_t *cls = pico8_fr_struct_class();
    const pico8_struct_class_t *nr = pico8_nr_struct_class();
    pico8_value_t v;
    void *a;
    size_t i;
    int j;
    int named = 0;

    start_game(EMPTY);
    a = build_gate();
    assert(is_an_fr_struct(a) && cls != nr && listed(pico8_cg("nr"), a));
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        assert(cls->methods[j] == nr->methods[j]);
        named += cls->methods[j] != NULL;
    }
    assert(named == (int)(sizeof methods / sizeof methods[0]));
    assert(cls->set_field == nr->set_field && cls->field != nr->field);
    for (i = 0; i < sizeof inherited / sizeof inherited[0]; i++) {
        pico8_value_t own;
        pico8_value_t through_nr;
        assert(cls->field(a, inherited[i], &own) && nr->field(a, inherited[i], &through_nr));
        assert(pico8_value_equal(own, through_nr) &&
               pico8_value_equal(own, class_value(pico8_fr_default, inherited[i])));
    }
    assert(!nr->field(a, "mt", &v) && cls->field(a, "mt", &v));
    assert(pico8_fr_ready());
    for (i = 0; i < 4; i++) {
        static const char *const nr_installed[] = {"gv", "ldown", "hb", "cn"};
        static const char *const qh_installed[] = {"hq", "ly", "ev", "ey"};
        assert(!pico8_nr_default(nr_installed[i], &v));
        assert(!pico8_qh_default(qh_installed[i], &v));
    }

    must_abort(write_mt, a);
    must_abort(write_hz, a);
    must_abort(write_rr, a);
    must_abort(write_qw, a);
    must_abort(write_ht, a);
    must_abort(build_with_unknown_key, NULL);
    must_abort(build_with_positional_key, NULL);
    PICO8_MEMBER_CENSUS_STOP() for (i = 0; i < sizeof written / sizeof written[0]; i++) {
        pico8_entity_set_field(a, written[i], pico8_value_fixed(pico8_fixed_from_int((int)i + 1)));
        assert(lookup(a, written[i], &v) && v.kind == PICO8_VALUE_FIXED &&
               v.as.fixed == pico8_fixed_from_int((int)i + 1));
    }
    {
        pico8_fr_t *bare = pico8_fr_new(NULL);
        assert(is_an_fr_struct(bare) && is_a_gate(bare) && !has(bare, "id") && has(bare, "oe") && has(bare, "kw"));
        assert(listed(pico8_cg("nr"), bare));
        bare->mt = pico8_value_ref(NULL);
        assert(!cls->field(bare, "mt", &v));
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
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_fr_is_a_struct);

    printf("test_pico8_fr: all tests passed\n");
    return 0;
}
