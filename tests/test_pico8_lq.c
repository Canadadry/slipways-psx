/* The laid-out row `lq`, read only through pico8_entity_get_field, its
 * pixels and row_count/row_item: direct constructions, `jo.ex`, `hp:lq` and
 * the copy `js:pq` makes of it, `ol` through mv() and `pi.ld`, and a short
 * replay. The frozen values were taken on the code where `lq` was still a
 * class instance and must not be re-taken. */

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
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_pi.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_display.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define BUILT "tests/fixtures/built_planet_panel_events.json"

#define CONSTRUCTIONS_HASH 0xF3436F4Du
#define JO_EX_HASH 0xA122C4CCu
#define HP_LQ_HASH 0x9071EEC9u
#define CALLS_HASH 0x26A3DB53u
#define REPLAY_HASH 0xB2458D77u

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

/* The instance reader keeps a field written nil (TROU-9), the struct reader
 * reads a nil member as absent: nil and absent are one state here. */
static int lookup(const void *e, const char *name, pico8_value_t *out) {
    return pico8_entity_get_field(e, name, out) && !(out->kind == PICO8_VALUE_REF && out->as.ref == NULL);
}

static void *get_ref(const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v) && v.kind == PICO8_VALUE_REF ? v.as.ref : NULL;
}

/* The only reader of a row's positionals: how many, then the i-th (0-based)
 * in ascending index order. */
static int row_count(const void *lq) {
    return ((const pico8_lq_t *)lq)->items.count;
}

static pico8_value_t row_item(const void *lq, int i) {
    assert(i >= 0 && i < row_count(lq));
    return ((const pico8_lq_t *)lq)->items.items[i];
}

/* Kind, then the string's bytes, the number, the boolean; a list or an
 * ob_table folded through (`depth` levels), any other REF by its tag. */
static uint32_t fold_value(uint32_t h, pico8_value_t v, int depth) {
    const char *s;
    h = fnv(h, (uint32_t)v.kind);
    switch (v.kind) {
    case PICO8_VALUE_FIXED:
        return fnv(h, (uint32_t)v.as.fixed);
    case PICO8_VALUE_BOOL:
        return fnv(h, (uint32_t)v.as.boolean);
    case PICO8_VALUE_STRING:
        for (s = v.as.string; *s != '\0'; s++) {
            h = fnv(h, (uint32_t)(unsigned char)*s);
        }
        return fnv(h, 0xFFu);
    case PICO8_VALUE_FUNCTION:
        return fnv(h, 0xFCu);
    case PICO8_VALUE_REF:
        if (v.as.ref == NULL) {
            return fnv(h, 0xDEADu);
        }
        h = fnv(h, (uint32_t)pico8_type_tag_of(v.as.ref));
        if (depth <= 0) {
            return h;
        }
        if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST) {
            const pico8_list_t *l = v.as.ref;
            int i;
            h = fnv(h, (uint32_t)l->count);
            for (i = 0; i < l->count; i++) {
                h = fold_value(h, l->items[i], depth - 1);
            }
            return h;
        }
        if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE) {
            const pico8_ob_table_t *t = v.as.ref;
            int i;
            h = fnv(h, (uint32_t)t->count);
            for (i = 0; i < t->count; i++) {
                h = fnv(h, (uint32_t)t->entries[i].key.kind);
                if (t->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
                    h = fnv(h, (uint32_t)t->entries[i].key.index);
                } else {
                    h = fold_value(h, pico8_value_string(t->entries[i].key.name), 0);
                }
                h = fold_value(h, t->entries[i].value, depth - 1);
            }
            return h;
        }
        return h;
    default:
        return h;
    }
}

static uint32_t fold_positionals(uint32_t h, const void *lq) {
    int n = row_count(lq);
    int i;
    h = fnv(h, (uint32_t)n);
    for (i = 0; i < n; i++) {
        h = fold_value(h, row_item(lq, i), 2);
    }
    return h;
}

static int fs_count(const void *lq) {
    const pico8_list_t *fs = get_ref(lq, "fs");
    assert(fs != NULL && pico8_type_tag_of(fs) == PICO8_TYPE_TAG_LIST);
    return fs->count;
}

static pico8_fixed_t w_of(const void *lq) {
    pico8_value_t w;
    assert(lookup(lq, "w", &w) && w.kind == PICO8_VALUE_FIXED);
    return w.as.fixed;
}

/* `c`, `bg`, `score` as values; `ra` as the vector it points at. */
static uint32_t fold_named(uint32_t h, const void *lq) {
    static const char *const names[] = {"c", "bg", "score"};
    const pico8_vec_t *ra;
    pico8_value_t v;
    size_t n;

    for (n = 0; n < sizeof names / sizeof names[0]; n++) {
        h = lookup(lq, names[n], &v) ? fold_value(h, v, 0) : fnv(h, 0xFFFFFFFFu);
    }
    ra = get_ref(lq, "ra");
    if (ra == NULL) {
        return fnv(h, 0xFFFFFFFFu);
    }
    h = fnv(h, (uint32_t)ra->x);
    return fnv(h, (uint32_t)ra->y);
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

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static pico8_vec_t vec_xy(int x, int y) {
    return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
}

static pico8_value_t vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return pico8_value_ref(v);
}

static pico8_value_t num(int n) {
    return pico8_value_fixed(pico8_fixed_from_int(n));
}

static pico8_value_t str(const char *s) {
    return pico8_value_string(s);
}

static pico8_value_t table_of(int n, const pico8_value_t *items) {
    pico8_ob_table_t *t = pico8_ob_table_new();
    int i;
    for (i = 0; i < n; i++) {
        pico8_ob_table_set_index(t, i + 1, items[i]);
    }
    return pico8_value_ref(t);
}

/* `ol` of `lq` on a cleared screen, at `a` 0, 0.5 and 1. */
static uint32_t fold_drawn(uint32_t h, const void *lq) {
    static const pico8_fixed_t as[] = {0, PICO8_FIXED_LIT(0x00008000, 0.5), PICO8_FIXED_LIT(0x00010000, 1.0)};
    size_t n;

    for (n = 0; n < sizeof as / sizeof as[0]; n++) {
        pico8_gfx_init(&gfx);
        pico8_cls(&mem);
        pico8_lq_ol(&gfx, &mem, lq, vec_xy(64, 60), as[n]);
        h = fnv(h, screen_hash());
    }
    return h;
}

static uint32_t fold_row(uint32_t h, const void *lq) {
    h = fold_drawn(h, lq);
    h = fnv(h, (uint32_t)w_of(lq));
    h = fnv(h, (uint32_t)fs_count(lq));
    h = fold_positionals(h, lq);
    return fold_named(h, lq);
}

/* Builds the row, folds it, frees the overrides it was built from. */
static uint32_t fold_built_from(uint32_t h, const char *label, pico8_ob_table_t *t) {
    const void *lq = pico8_lq_new(t);
    uint32_t one = fold_row(2166136261u, lq);

    printf("  %s: w=%d fs=%d n=%d 0x%08X\n", label, pico8_fixed_flr_int(w_of(lq)), fs_count(lq), row_count(lq),
           (unsigned)one);
    if (t != NULL) {
        pico8_ob_table_free(t);
    }
    return fnv(h, one);
}

static pico8_ob_table_t *positionals(int n, const pico8_value_t *items) {
    pico8_ob_table_t *t = pico8_ob_table_new();
    int i;
    for (i = 0; i < n; i++) {
        pico8_ob_table_set_index(t, i + 1, items[i]);
    }
    return t;
}

/* Every kind of item `mn` knows, the named keys each constructor passes,
 * no overrides at all, a row with a hole as `pj:ph`'s "score 0" row, and a
 * row written out of index order as the stars. */
static void test_constructions(void) {
    uint32_t h = 2166136261u;
    pico8_ob_table_t *t;

    {
        pico8_value_t a[] = {str("hello")};
        h = fold_built_from(h, "string", positionals(1, a));
    }
    {
        pico8_value_t a[] = {str("hello")};
        t = positionals(1, a);
        pico8_ob_table_set_name(t, "c", num(13));
        h = fold_built_from(h, "string c", t);
    }
    {
        pico8_value_t a[] = {num(12)};
        h = fold_built_from(h, "sprite", positionals(1, a));
    }
    {
        pico8_value_t f[] = {num(12), num(10), num(3), num(1)};
        pico8_value_t a[] = {table_of(4, f)};
        h = fold_built_from(h, "spr w qw qv", positionals(1, a));
    }
    {
        pico8_ob_table_t *qe = pico8_ob_table_new();
        pico8_value_t a[1];
        pico8_ob_table_set_name(qe, "qe", num(14));
        a[0] = pico8_value_ref(qe);
        h = fold_built_from(h, "qe", positionals(1, a));
    }
    {
        pico8_value_t a[] = {pico8_value_bool(0)};
        h = fold_built_from(h, "false", positionals(1, a));
    }
    {
        pico8_value_t a[] = {pico8_value_ref(NULL)};
        h = fold_built_from(h, "nil", positionals(1, a));
    }
    {
        pico8_value_t f[] = {num(13), num(8)};
        pico8_value_t a[] = {str("a"), num(12), table_of(2, f), pico8_value_bool(0), str("bc"), pico8_value_ref(NULL),
                             num(40)};
        t = positionals(7, a);
        pico8_ob_table_set_name(t, "c", num(9));
        h = fold_built_from(h, "mixed c", t);
    }
    h = fold_built_from(h, "NULL", NULL);
    {
        t = pico8_ob_table_new();
        pico8_ob_table_set_index(t, 1, str("score"));
        pico8_ob_table_set_name(t, "score", num(0));
        pico8_ob_table_set_name(t, "ra", vec_ref(vec_xy(64, 50)));
        pico8_ob_table_set_name(t, "c", num(5));
        pico8_ob_table_set_index(t, 3, str(""));
        h = fold_built_from(h, "hole", t);
    }
    {
        t = pico8_ob_table_new();
        pico8_ob_table_set_name(t, "score", num(3));
        pico8_ob_table_set_name(t, "ra", vec_ref(vec_xy(64, 40)));
        pico8_ob_table_set_index(t, 3, num(21));
        pico8_ob_table_set_index(t, 1, num(19));
        pico8_ob_table_set_index(t, 2, str("x"));
        pico8_ob_table_set_index(t, 4, num(20));
        pico8_ob_table_set_index(t, 5, num(22));
        h = fold_built_from(h, "disorder", t);
    }
    {
        pico8_value_t a[] = {str("3"), num(12)};
        t = positionals(2, a);
        pico8_ob_table_set_name(t, "bg", num(8));
        pico8_ob_table_set_name(t, "c", num(7));
        h = fold_built_from(h, "bg", t);
    }
    {
        pico8_value_t a[] = {str("4")};
        t = positionals(1, a);
        pico8_ob_table_set_name(t, "bg", pico8_value_bool(0));
        h = fold_built_from(h, "bg false", t);
    }
    check("constructions", h, CONSTRUCTIONS_HASH);
}

static void *project(int k) {
    pico8_value_t v;
    assert(pico8_ob_table_get_index(pico8_gb(), k, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

/* `jo.ex` on the `i`/`o` of three projects, over a spread of `jq`/`mj`/`d`
 * that includes `jq>5`, the branch no replay reaches; then with no `i`. */
static void test_jo_ex(void) {
    static const int ks[] = {1, 7, 20};
    static const int jqs[] = {1, 2, 4, 6, 9};
    uint32_t h = 2166136261u;
    size_t a;
    size_t b;
    pico8_value_t o;

    for (a = 0; a < sizeof ks / sizeof ks[0]; a++) {
        void *p = project(ks[a]);
        const pico8_list_t *is = get_ref(p, "i");
        assert(lookup(p, "o", &o));
        for (b = 0; b < sizeof jqs / sizeof jqs[0]; b++) {
            const void *lq = pico8_jo_ex(is, o, pico8_fixed_from_int(jqs[b]), pico8_fixed_from_int((int)b % 3),
                                         pico8_fixed_from_int((int)b % 2));
            pico8_value_t c;
            assert(lq != NULL);
            assert(lookup(lq, "c", &c) == (jqs[b] > 5));
            if (jqs[b] > 5) {
                assert(c.kind == PICO8_VALUE_FIXED && c.as.fixed == pico8_fixed_from_int(13));
            }
            h = fold_row(h, lq);
        }
    }
    {
        const void *lq = pico8_jo_ex(NULL, o, pico8_fixed_from_int(2), 0, pico8_fixed_from_int(1));
        assert(lq != NULL);
        h = fold_row(h, lq);
    }
    assert(pico8_jo_ex(NULL, pico8_value_bool(0), pico8_fixed_from_int(1), 0, 0) == NULL);
    check("jo.ex", h, JO_EX_HASH);
}

static void start_game(const char *events);

/* `hp:lq` appends `fk` and `px` after `pq`: `fs` and `w` do not see them,
 * the copy `js:pq` makes of the row does. Then two buttons with an `h_`,
 * its `bg` a number and false. */
static void test_hp_lq_and_copy(void) {
    static const int ks[] = {1, 5, 12};
    uint32_t h = 2166136261u;
    size_t a;

    start_game(BUILT);
    for (a = 0; a < sizeof ks / sizeof ks[0]; a++) {
        void *lq = pico8_hp_lq(project(ks[a]));
        pico8_ob_table_t *spec = pico8_ob_table_new();
        pico8_js_t *button;
        const void *copy;
        pico8_value_t mb;

        assert(lq != NULL);
        assert(fs_count(lq) + 2 == row_count(lq));
        h = fold_row(h, lq);

        pico8_ob_table_set_name(spec, "lq", pico8_value_ref(lq));
        button = pico8_js_new(spec);
        pico8_ob_table_free(spec);
        copy = get_ref(button, "lq");
        assert(copy != NULL && copy != lq);
        assert(fs_count(copy) == row_count(copy) && row_count(copy) == row_count(lq));
        h = fold_row(h, copy);
        assert(lookup(button, "mb", &mb) && mb.kind == PICO8_VALUE_FIXED);
        h = fnv(h, (uint32_t)mb.as.fixed);
        pico8_gfx_init(&gfx);
        pico8_cls(&mem);
        pico8_js_kj(&gfx, &mem, button, vec_xy(10, 50));
        h = fnv(h, screen_hash());
    }

    for (a = 0; a < 2; a++) {
        pico8_value_t lq_items[] = {str("ab"), num(12)};
        pico8_value_t h_items[] = {str(a == 0 ? "3" : "12")};
        pico8_ob_table_t *lq_t = positionals(2, lq_items);
        pico8_ob_table_t *h_t = positionals(1, h_items);
        pico8_ob_table_t *spec = pico8_ob_table_new();
        pico8_js_t *button;
        const void *h_;
        pico8_value_t mb;

        pico8_ob_table_set_name(lq_t, "c", num(9));
        pico8_ob_table_set_name(h_t, "c", num(10));
        pico8_ob_table_set_name(h_t, "bg", a == 0 ? num(8) : pico8_value_bool(0));
        pico8_ob_table_set_name(spec, "lq", pico8_value_ref(lq_t));
        pico8_ob_table_set_name(spec, "h_", pico8_value_ref(h_t));
        button = pico8_js_new(spec);
        h = fold_row(h, get_ref(button, "lq"));
        h_ = get_ref(button, "h_");
        assert(h_ != NULL);
        h = fold_row(h, h_);
        assert(lookup(button, "mb", &mb) && mb.kind == PICO8_VALUE_FIXED);
        h = fnv(h, (uint32_t)mb.as.fixed);
        pico8_gfx_init(&gfx);
        pico8_cls(&mem);
        pico8_js_kj(&gfx, &mem, button, vec_xy(10, 50));
        h = fnv(h, screen_hash());
    }
    check("hp:lq and copy", h, HP_LQ_HASH);
}

static uint32_t drawn_by_ol(const void *lq, pico8_vec_t at, pico8_fixed_t a) {
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    pico8_lq_ol(&gfx, &mem, lq, at, a);
    return screen_hash();
}

/* `mv(lq,"ol",p,a)` as `jo:kj` calls it and `{lq,ra,0.5,fn="ld"}` as
 * `pj:ph` hands it to `bl`, each drawing what `ol` draws. */
static void test_calls(void) {
    pico8_value_t f[] = {num(13), num(8)};
    pico8_value_t a[] = {str("hi"), num(12), table_of(2, f)};
    pico8_ob_table_t *t = positionals(3, a);
    void *lq;
    pico8_value_t discard;
    pico8_ob_table_t *entry;
    pico8_list_t rb;
    uint32_t h = 2166136261u;
    uint32_t want;

    pico8_ob_table_set_name(t, "c", num(11));
    lq = pico8_lq_new(t);

    want = drawn_by_ol(lq, vec_xy(64, 30), PICO8_FIXED_LIT(0x00008000, 0.5));
    h = fnv(h, want);
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    assert(pico8_mv(lq, "ol", 2, vec_ref(vec_xy(64, 30)), pico8_value_fixed(PICO8_FIXED_LIT(0x00008000, 0.5)),
                    &discard));
    assert(screen_hash() == want);

    entry = pico8_ob_table_new();
    pico8_ob_table_set_index(entry, 1, pico8_value_ref(lq));
    pico8_ob_table_set_index(entry, 2, vec_ref(vec_xy(64, 30)));
    pico8_ob_table_set_index(entry, 3, pico8_value_fixed(PICO8_FIXED_LIT(0x00008000, 0.5)));
    pico8_ob_table_set_name(entry, "fn", str("ld"));
    pico8_list_init(&rb);
    pico8_list_add(&rb, pico8_value_ref(entry));
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    pico8_bl_default(&gfx, &mem, &rb);
    assert(screen_hash() == want);

    want = drawn_by_ol(lq, vec_xy(20, 90), 0);
    h = fnv(h, want);
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    pico8_lq_ol_default(&gfx, &mem, lq, vec_xy(20, 90));
    assert(screen_hash() == want);

    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    want = screen_hash();
    pico8_lq_ol(&gfx, &mem, NULL, vec_xy(20, 90), 0);
    assert(screen_hash() == want);
    check("calls", h, CALLS_HASH);
}

/* A game started as the net starts it: `eu()` on `bo[2]` before frame 0. */
static void start_game(const char *events) {
    pico8_value_t difficulty;

    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_cheat_init(&cheat);
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty) && difficulty.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty.as.ref));
}

static uint32_t fold_button_row(uint32_t h, const void *button, const char *name) {
    const void *lq = get_ref(button, name);
    pico8_value_t v;
    int i;

    if (lq == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)w_of(lq));
    h = lookup(lq, "c", &v) ? fold_value(h, v, 0) : fnv(h, 0xFFFFFFFFu);
    h = lookup(lq, "bg", &v) ? fold_value(h, v, 0) : fnv(h, 0xFFFFFFFFu);
    h = fnv(h, (uint32_t)row_count(lq));
    for (i = 0; i < row_count(lq); i++) {
        h = fnv(h, (uint32_t)row_item(lq, i).kind);
    }
    return h;
}

/* At the last frame of the built-planet panel replay, every button of
 * `ok.le`: its `lq` and `h_`; then the screen. */
static void test_replay(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *le;
    int buttons = 0;
    int i;

    start_game(BUILT);
    for (i = 0; i < input.frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();
        if (i < input.frame_count - 1) {
            pico8_input_step(&input);
        }
    }
    le = get_ref(pico8_ok(), "le");
    assert(le != NULL);
    for (i = 0; i < le->count; i++) {
        const void *button = le->items[i].as.ref;
        if (button != NULL && pico8_type_tag_of(button) == PICO8_TYPE_TAG_STRUCT &&
            ((const pico8_struct_header_t *)button)->cls == pico8_js_struct_class()) {
            buttons++;
            h = fold_button_row(h, button, "lq");
            h = fold_button_row(h, button, "h_");
        }
    }
    printf("  replay buttons: %d\n", buttons);
    assert(buttons > 0);
    check("replay", fnv(h, screen_hash()), REPLAY_HASH);
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

static void write_w(void *e) {
    pico8_entity_set_field(e, "w", num(1));
}

static void build_with_named_key(void *e) {
    pico8_ob_table_t *t = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(t, "x", pico8_value_bool(1));
    (void)pico8_lq_new(t);
}

static void ol_on_other(void *e) {
    pico8_lq_ol(&gfx, &mem, e, vec_xy(0, 0), 0);
}

static void button_on_other(void *e) {
    pico8_ob_table_t *spec = pico8_ob_table_new();
    pico8_ob_table_set_name(spec, "lq", pico8_value_ref(e));
    (void)pico8_js_new(spec);
}

/* A row is a struct whose descriptor has `ol` and a reader, no other method
 * and no writer; the class keeps no `pq` and no `ol` default. It takes no
 * named key but c, bg, score, ra, fs, w, and `ol` refuses anything that is
 * not a row, as `js:pq` refuses another struct. The table view of an
 * `hp:lq` row lists the positionals `pq` saw, `fs`, `w`, then the two added
 * after; the named keys come between the positionals and `fs`. */
static void test_lq_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_lq_struct_class();
    pico8_value_t a[] = {str("hi"), num(12)};
    pico8_ob_table_t *t = positionals(2, a);
    pico8_lq_t *lq;
    pico8_lq_t *row;
    pico8_ob_table_t *view;
    pico8_value_t v;
    int k;
    int i;

    pico8_ob_table_set_name(t, "c", num(9));
    lq = pico8_lq_new(t);
    pico8_ob_table_free(t);
    assert(pico8_type_tag_of(lq) == PICO8_TYPE_TAG_STRUCT && lq->header.cls == cls);
    for (i = 0; i < PICO8_METHOD_COUNT; i++) {
        assert((cls->methods[i] != NULL) == (i == PICO8_METHOD_OL));
    }
    assert(cls->field != NULL && cls->set_field == NULL);
    assert(pico8_lq_ready());
    assert(!pico8_lq_default("ol", &v));
    assert(lookup(lq, "c", &v) && v.as.fixed == pico8_fixed_from_int(9));
    assert(!lookup(lq, "bg", &v) && !lookup(lq, "items", &v));
    assert(lookup(lq, "ol", &v) && v.kind == PICO8_VALUE_FUNCTION);

    start_game(BUILT);
    row = pico8_hp_lq(project(1));
    k = fs_count(row);
    assert(row_count(row) == k + 2);
    view = pico8_lq_table_view(row);
    assert(view->count == row_count(row) + 2);
    for (i = 0; i < k; i++) {
        assert(view->entries[i].key.kind == PICO8_OB_KEY_INDEX && view->entries[i].key.index == i + 1);
        assert(pico8_value_equal(view->entries[i].value, row_item(row, i)));
    }
    assert(view->entries[k].key.kind == PICO8_OB_KEY_NAME && strcmp(view->entries[k].key.name, "fs") == 0);
    assert(view->entries[k + 1].key.kind == PICO8_OB_KEY_NAME && strcmp(view->entries[k + 1].key.name, "w") == 0);
    for (i = k; i < k + 2; i++) {
        assert(view->entries[i + 2].key.kind == PICO8_OB_KEY_INDEX && view->entries[i + 2].key.index == i + 1);
        assert(pico8_value_equal(view->entries[i + 2].value, row_item(row, i)));
    }
    pico8_ob_table_free(view);

    t = positionals(2, a);
    pico8_ob_table_set_name(t, "ra", vec_ref(vec_xy(1, 2)));
    pico8_ob_table_set_name(t, "score", num(4));
    pico8_ob_table_set_name(t, "bg", pico8_value_bool(0));
    view = pico8_lq_table_view(pico8_lq_new(t));
    pico8_ob_table_free(t);
    assert(view->count == 7);
    assert(view->entries[0].key.kind == PICO8_OB_KEY_INDEX && view->entries[0].key.index == 1);
    assert(view->entries[1].key.kind == PICO8_OB_KEY_INDEX && view->entries[1].key.index == 2);
    assert(strcmp(view->entries[2].key.name, "bg") == 0 && strcmp(view->entries[3].key.name, "score") == 0);
    assert(strcmp(view->entries[4].key.name, "ra") == 0 && strcmp(view->entries[5].key.name, "fs") == 0);
    assert(strcmp(view->entries[6].key.name, "w") == 0);
    pico8_ob_table_free(view);

    must_abort(write_w, lq);
    must_abort(build_with_named_key, NULL);
    must_abort(ol_on_other, pico8_ok());
    must_abort(button_on_other, pico8_ok());
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
    in_fresh_child(test_constructions);
    in_fresh_child(test_jo_ex);
    in_fresh_child(test_hp_lq_and_copy);
    in_fresh_child(test_calls);
    in_fresh_child(test_replay);
    in_fresh_child(test_lq_is_a_struct);

    printf("test_pico8_lq: all tests passed\n");
    return 0;
}
