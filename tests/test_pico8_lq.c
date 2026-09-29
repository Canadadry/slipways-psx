/* The laid-out row `lq`, read on its struct members, its pixels and
 * row_count/row_item: direct constructions, `jo.ex`, `hp:lq` and
 * the copy `js:pq` makes of it, `ol` through mv() and `pi.ld`, and a built
 * planet's panel in a game built directly: no events file. */

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
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_pi.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_display.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
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

#define CONSTRUCTIONS_HASH 0xF3436F4Du
#define JO_EX_HASH 0xA122C4CCu
#define HP_LQ_HASH 0x9071EEC9u
#define CALLS_HASH 0x26A3DB53u
#define PANEL_HASH 0xDA5A7F3Eu

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

static const pico8_lq_t *as_lq(const void *lq) {
    assert(lq != NULL && pico8_type_tag_of(lq) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)lq)->cls == pico8_lq_struct_class());
    return lq;
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
 * folded through (`depth` levels), any other REF by its tag. */
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
        return h;
    default:
        return h;
    }
}

/* The frozen hashes were taken when tuples were ob tables: these are the
 * bytes a table folded for its tag and its two kinds of key. */
enum { VIEW_TAG = 0x9E5C0B7Au, VIEW_INDEX_KEY = 0, VIEW_NAME_KEY = 1 };

static uint32_t fold_tuple_name(uint32_t h, const char *name) {
    return fold_value(fnv(h, (uint32_t)VIEW_NAME_KEY), pico8_value_string(name), 0);
}

static pico8_value_t tuple_item_value(const pico8_jb_item_t *item) {
    if (item->s != NULL) {
        return pico8_value_string(item->s);
    }
    if (item->is_false) {
        return pico8_value_bool(0);
    }
    if (item->is_ref) {
        return pico8_value_ref((void *)item->ref);
    }
    return pico8_value_fixed(item->is_fixed ? item->fixed : pico8_fixed_from_int(item->n));
}

/* A tuple folded as fold_value folded the table it was: items, then `qe`,
 * then `c`. */
static uint32_t fold_tuple(uint32_t h, const pico8_jb_row_t *t, int depth) {
    int i;

    h = fnv(fnv(h, (uint32_t)PICO8_VALUE_REF), (uint32_t)VIEW_TAG);
    if (depth <= 0) {
        return h;
    }
    h = fnv(h, (uint32_t)(t->count + (t->has_qe != 0) + (t->has_c != 0)));
    for (i = 0; i < t->count; i++) {
        h = fnv(fnv(h, (uint32_t)VIEW_INDEX_KEY), (uint32_t)(i + 1));
        h = fold_value(h, tuple_item_value(&t->items[i]), depth - 1);
    }
    if (t->has_qe) {
        h = fold_value(fold_tuple_name(h, "qe"), pico8_value_fixed(t->qe), depth - 1);
    }
    if (t->has_c) {
        h = fold_value(fold_tuple_name(h, "c"), pico8_value_fixed(pico8_fixed_from_int(t->c)), depth - 1);
    }
    return h;
}

static uint32_t fold_positionals(uint32_t h, const void *lq) {
    int n = row_count(lq);
    int i;
    h = fnv(h, (uint32_t)n);
    for (i = 0; i < n; i++) {
        pico8_value_t v = row_item(lq, i);
        h = v.kind == PICO8_VALUE_REF && v.as.ref != NULL ? fold_tuple(h, v.as.ref, 2) : fold_value(h, v, 2);
    }
    return h;
}

static int fs_count(const void *lq) {
    const pico8_list_t *fs = as_lq(lq)->fs;
    assert(fs != NULL && pico8_type_tag_of(fs) == PICO8_TYPE_TAG_LIST);
    return fs->count;
}

static pico8_fixed_t w_of(const void *lq) {
    return as_lq(lq)->w;
}

/* fold_value's FIXED and BOOL cases at depth 0; an unset flag folds as
 * absent. */
static uint32_t fold_fixed_member(uint32_t h, int has, pico8_fixed_t v) {
    return has ? fnv(fnv(h, (uint32_t)PICO8_VALUE_FIXED), (uint32_t)v) : fnv(h, 0xFFFFFFFFu);
}

static uint32_t fold_bg(uint32_t h, const pico8_lq_t *lq) {
    if (lq->has_bg && lq->bg_is_bool) {
        return fnv(fnv(h, (uint32_t)PICO8_VALUE_BOOL), (uint32_t)(lq->bg != 0));
    }
    return fold_fixed_member(h, lq->has_bg, lq->bg);
}

/* `c`, `bg`, `score` as values; `ra` as the vector it points at. */
static uint32_t fold_named(uint32_t h, const void *row) {
    const pico8_lq_t *lq = as_lq(row);
    const pico8_vec_t *ra;

    h = fold_fixed_member(h, lq->has_c, lq->c);
    h = fold_bg(h, lq);
    h = fold_fixed_member(h, lq->has_score, lq->score);
    ra = lq->ra;
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

/* A tuple of numbers, never freed. */
static pico8_value_t tuple_of(int n, const int *items) {
    pico8_jb_row_t *t = calloc(1, sizeof *t);
    pico8_jb_item_t *it = calloc((size_t)n, sizeof *it);
    int i;
    for (i = 0; i < n; i++) {
        it[i].n = items[i];
    }
    t->count = n;
    t->items = it;
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

/* Builds the row from `k` (NULL: no keys), folds it. */
static uint32_t fold_built_from(uint32_t h, const char *label, const pico8_lq_keys_t *k) {
    const void *lq = pico8_lq_new(k);
    uint32_t one = fold_row(2166136261u, lq);

    assert(((const pico8_struct_header_t *)lq)->cls->get_ra(lq) == as_lq(lq)->ra);

    printf("  %s: w=%d fs=%d n=%d 0x%08X\n", label, pico8_fixed_flr_int(w_of(lq)), fs_count(lq), row_count(lq),
           (unsigned)one);
    return fnv(h, one);
}

/* The keys of `n` positionals, `items` borrowed, no named key. */
static pico8_lq_keys_t positionals(int n, const pico8_value_t *items) {
    pico8_lq_keys_t k = {0};

    k.items = items;
    k.count = n;
    return k;
}

static pico8_lq_keys_t with_c(pico8_lq_keys_t k, int c) {
    k.has_c = 1;
    k.c = pico8_fixed_from_int(c);
    return k;
}

/* `bg` a number, or false for a negative `bg`. */
static pico8_lq_keys_t with_bg(pico8_lq_keys_t k, int bg) {
    k.has_bg = 1;
    k.bg_is_bool = bg < 0;
    k.bg = bg < 0 ? 0 : pico8_fixed_from_int(bg);
    return k;
}

static pico8_lq_keys_t with_score(pico8_lq_keys_t k, int score) {
    k.has_score = 1;
    k.score = pico8_fixed_from_int(score);
    return k;
}

static pico8_lq_keys_t with_ra(pico8_lq_keys_t k, pico8_value_t ra) {
    k.ra = ra.as.ref;
    return k;
}

/* Every kind of item `mn` knows, the named keys each constructor passes,
 * no keys at all, the `score` row `pj:ph` passes, its hole already closed
 * (TROU-25), and the stars' row in index order. */
static void test_constructions(void) {
    uint32_t h = 2166136261u;
    pico8_lq_keys_t k;

    {
        pico8_value_t a[] = {str("hello")};
        k = positionals(1, a);
        h = fold_built_from(h, "string", &k);
    }
    {
        pico8_value_t a[] = {str("hello")};
        k = with_c(positionals(1, a), 13);
        h = fold_built_from(h, "string c", &k);
    }
    {
        pico8_value_t a[] = {num(12)};
        k = positionals(1, a);
        h = fold_built_from(h, "sprite", &k);
    }
    {
        static const int f[] = {12, 10, 3, 1};
        pico8_value_t a[1];
        a[0] = tuple_of(4, f);
        k = positionals(1, a);
        h = fold_built_from(h, "spr w qw qv", &k);
    }
    {
        pico8_jb_row_t *qe = calloc(1, sizeof *qe);
        pico8_value_t a[1];
        qe->has_qe = 1;
        qe->qe = pico8_fixed_from_int(14);
        a[0] = pico8_value_ref(qe);
        k = positionals(1, a);
        h = fold_built_from(h, "qe", &k);
    }
    {
        pico8_value_t a[] = {pico8_value_bool(0)};
        k = positionals(1, a);
        h = fold_built_from(h, "false", &k);
    }
    {
        pico8_value_t a[] = {pico8_value_ref(NULL)};
        k = positionals(1, a);
        h = fold_built_from(h, "nil", &k);
    }
    {
        static const int f[] = {13, 8};
        pico8_value_t a[] = {str("a"), num(12), pico8_value_ref(NULL), pico8_value_bool(0), str("bc"),
                             pico8_value_ref(NULL), num(40)};
        a[2] = tuple_of(2, f);
        k = with_c(positionals(7, a), 9);
        h = fold_built_from(h, "mixed c", &k);
    }
    h = fold_built_from(h, "NULL", NULL);
    {
        pico8_value_t a[] = {str("score"), str("")};
        k = with_c(with_ra(with_score(positionals(2, a), 0), vec_ref(vec_xy(64, 50))), 5);
        h = fold_built_from(h, "hole", &k);
    }
    {
        pico8_value_t a[] = {num(19), str("x"), num(21), num(20), num(22)};
        k = with_ra(with_score(positionals(5, a), 3), vec_ref(vec_xy(64, 40)));
        h = fold_built_from(h, "disorder", &k);
    }
    {
        pico8_value_t a[] = {str("3"), num(12)};
        k = with_c(with_bg(positionals(2, a), 8), 7);
        h = fold_built_from(h, "bg", &k);
    }
    {
        pico8_value_t a[] = {str("4")};
        k = with_bg(positionals(1, a), -1);
        h = fold_built_from(h, "bg false", &k);
    }
    check("constructions", h, CONSTRUCTIONS_HASH);
}

static void *project(int k) {
    assert(pico8_gb_at(k) != NULL);
    return pico8_gb_at(k);
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
        const pico8_hp_t *p = project(ks[a]);
        const pico8_list_t *is = p->i;
        assert(p->has_o);
        o = pico8_value_fixed(p->o);
        for (b = 0; b < sizeof jqs / sizeof jqs[0]; b++) {
            const void *lq = pico8_jo_ex(is, o, pico8_fixed_from_int(jqs[b]), pico8_fixed_from_int((int)b % 3),
                                         pico8_fixed_from_int((int)b % 2));
            assert(lq != NULL);
            assert(as_lq(lq)->has_c == (jqs[b] > 5));
            if (jqs[b] > 5) {
                assert(as_lq(lq)->c == pico8_fixed_from_int(13));
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

static void start_game(void);

/* `hp:lq` appends `fk` and `px` after `pq`: `fs` and `w` do not see them,
 * the copy `js:pq` makes of the row does. Then two buttons with an `h_`,
 * its `bg` a number and false. */
static void test_hp_lq_and_copy(void) {
    static const int ks[] = {1, 5, 12};
    uint32_t h = 2166136261u;
    size_t a;

    start_game();
    for (a = 0; a < sizeof ks / sizeof ks[0]; a++) {
        void *lq = pico8_hp_lq(project(ks[a]));
        pico8_js_keys_t spec = {0};
        pico8_js_t *button;
        const void *copy;

        assert(lq != NULL);
        assert(fs_count(lq) + 2 == row_count(lq));
        h = fold_row(h, lq);

        spec.lq = pico8_value_ref(lq);
        button = pico8_js_new(&spec);
        copy = button->lq;
        assert(copy != NULL && copy != lq);
        assert(fs_count(copy) == row_count(copy) && row_count(copy) == row_count(lq));
        h = fold_row(h, copy);
        h = fnv(h, (uint32_t)button->mb);
        pico8_gfx_init(&gfx);
        pico8_cls(&mem);
        pico8_js_kj(&gfx, &mem, button, vec_xy(10, 50));
        h = fnv(h, screen_hash());
    }

    for (a = 0; a < 2; a++) {
        pico8_value_t lq_items[] = {str("ab"), num(12)};
        pico8_value_t h_items[] = {str(a == 0 ? "3" : "12")};
        pico8_lq_keys_t lq_t = with_c(positionals(2, lq_items), 9);
        pico8_lq_keys_t h_t = with_bg(with_c(positionals(1, h_items), 10), a == 0 ? 8 : -1);
        pico8_js_keys_t spec = {0};
        pico8_js_t *button;
        const void *h_;

        spec.lq_row = &lq_t;
        spec.has_h_ = 1;
        spec.h_row = &h_t;
        button = pico8_js_new(&spec);
        h = fold_row(h, button->lq);
        h_ = button->h_;
        assert(h_ != NULL);
        h = fold_row(h, h_);
        h = fnv(h, (uint32_t)button->mb);
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
    static const int f[] = {13, 8};
    pico8_value_t a[] = {str("hi"), num(12), pico8_value_ref(NULL)};
    pico8_lq_keys_t t;
    void *lq;
    pico8_jb_item_t entry_items[3];
    pico8_jb_row_t entry = {0};
    pico8_list_t rb;
    uint32_t h = 2166136261u;
    uint32_t want;

    a[2] = tuple_of(2, f);
    t = with_c(positionals(3, a), 11);
    lq = pico8_lq_new(&t);

    want = drawn_by_ol(lq, vec_xy(64, 30), PICO8_FIXED_LIT(0x00008000, 0.5));
    h = fnv(h, want);
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    pico8_lq_struct_class()->ol(lq, vec_xy(64, 30), PICO8_FIXED_LIT(0x00008000, 0.5));
    assert(screen_hash() == want);

    entry_items[0] = (pico8_jb_item_t){.is_ref = 1, .ref = lq};
    entry_items[1] = (pico8_jb_item_t){.is_ref = 1, .ref = vec_ref(vec_xy(64, 30)).as.ref};
    entry_items[2] = (pico8_jb_item_t){.is_fixed = 1, .fixed = PICO8_FIXED_LIT(0x00008000, 0.5)};
    entry.count = 3;
    entry.items = entry_items;
    entry.fn = "ld";
    pico8_list_init(&rb);
    pico8_list_add(&rb, pico8_value_ref(&entry));
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

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
}

static pico8_jo_t *as_jo(void *e) {
    assert(pico8_jo_is_family(e));
    return e;
}

static pico8_vec_t pos_of(void *e) {
    const pico8_vec_t *p = as_jo(e)->p;
    assert(p != NULL);
    return *p;
}

static pico8_fixed_t dist2(pico8_vec_t a, pico8_vec_t b) {
    pico8_vec_t d = pico8_vec_sub(a, b);
    return pico8_fixed_add(pico8_fixed_mul(d.x, d.x), pico8_fixed_mul(d.y, d.y));
}

/* A generated planet colonized by COLONY, its nearest one by EXPORTER, the
 * two joined by a slipway paid by `ey` as a drag pays it (`jc:hq` reads the
 * hovered planet off `ok.o_`), left to settle; then the colony's panel,
 * `jo:gh`, up and drawn. */
static void built_planet_panel(void) {
    const pico8_list_t *jos;
    pico8_jc_keys_t jc_keys = {0};
    pico8_vec_t *mp = malloc(sizeof *mp);
    void *qu;
    void *re = NULL;
    pico8_jc_t *jc;
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

    jc_keys.qu = qu;
    jc = pico8_jc_new(&jc_keys);
    *mp = pico8_vec_add(pico8_vec_mul(pos_of(re), pico8_fixed_from_int(128)),
                        pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(1)));
    pico8_ok()->o_ = re;
    pico8_jc_struct_class()->hq(jc, mp, NULL);
    pico8_ok()->o_ = NULL;
    while (jc->ke.t < pico8_fixed_from_int(15)) {
        pico8_dg();
    }
    pico8_jc_struct_class()->ey(jc, re);
    for (i = 0; i < 120; i++) {
        pico8_dg();
    }
    pico8_jo_gh(qu);
    pico8__update60();
    pico8__draw();
}

/* A button's `lq` or `h_`, NULL when nil or false. */
static uint32_t fold_button_row(uint32_t h, const pico8_lq_t *lq) {
    int i;

    if (lq == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)w_of(lq));
    h = fold_fixed_member(h, lq->has_c, lq->c);
    h = fold_bg(h, lq);
    h = fnv(h, (uint32_t)row_count(lq));
    for (i = 0; i < row_count(lq); i++) {
        h = fnv(h, (uint32_t)row_item(lq, i).kind);
    }
    return h;
}

/* With a built planet's panel up, every button of `ok.le`: its `lq` and
 * `h_`; then the screen. */
static void test_planet_panel(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *le;
    int buttons = 0;
    int i;

    built_planet_panel();
    le = pico8_ok()->le;
    assert(le != NULL);
    for (i = 0; i < le->count; i++) {
        const void *button = le->items[i].as.ref;
        if (button != NULL && pico8_type_tag_of(button) == PICO8_TYPE_TAG_STRUCT &&
            ((const pico8_struct_header_t *)button)->cls == pico8_js_struct_class()) {
            buttons++;
            h = fold_button_row(h, ((const pico8_js_t *)button)->lq);
            h = fold_button_row(h, ((const pico8_js_t *)button)->h_);
        }
    }
    printf("  panel buttons: %d\n", buttons);
    assert(buttons > 0);
    check("panel", fnv(h, screen_hash()), PANEL_HASH);
}

/* The row of `ok.le` whose first item is `head`, NULL when none. */
static const pico8_lq_t *panel_row(const char *head) {
    const pico8_list_t *le = pico8_ok()->le;
    int i;

    for (i = 0; le != NULL && i < le->count; i++) {
        const pico8_struct_header_t *button = le->items[i].as.ref;
        const pico8_lq_t *lq;
        pico8_value_t v;

        assert(button != NULL && button->cls == pico8_js_struct_class() && "ok.le holds js buttons only");
        lq = ((const pico8_js_t *)button)->lq;

        if (lq != NULL && lq->items.count > 0) {
            v = lq->items.items[0];
            if (v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, head) == 0) {
                return lq;
            }
        }
    }
    return NULL;
}

/* A planet whose every want arrives lists each under "wants more:", and
 * `gc` times under "receives:", as the `{qe=x}` `lq:mn` reads: a want of a
 * resource draws it 5 wide, a want of none nothing, 6 wide. */
static void test_gh_wants_more(void) {
    pico8_fixed_t ore = pico8_fixed_from_int(21);
    pico8_jy_t *a = pico8_jy_new(&ore);
    pico8_jy_t *b = pico8_jy_new(NULL);
    pico8_list_t *jy = malloc(sizeof *jy);
    pico8_jo_t *planet;
    const pico8_lq_t *wants;
    const pico8_lq_t *receives;
    pico8_vec_t *gq;

    start_game();
    a->has_gc = 1;
    a->gc = pico8_fixed_from_int(1);
    b->has_gc = 1;
    b->gc = pico8_fixed_from_int(2);
    pico8_list_init(jy);
    pico8_list_add(jy, pico8_value_ref(a));
    pico8_list_add(jy, pico8_value_ref(b));
    planet = pico8_jo_new(NULL);
    planet->jy = jy;
    planet->has_mi = 1;
    planet->mi = pico8_fixed_from_int(1);
    planet->ra = malloc(sizeof *planet->ra);
    *planet->ra = vec_xy(64, 64);
    gq = malloc(sizeof *gq);
    *gq = vec_xy(0, 9);
    planet->gq = gq;
    pico8_jo_gh(planet);

    wants = panel_row("wants more:");
    receives = panel_row("receives:");
    assert(wants != NULL && receives != NULL && panel_row("needs:") == NULL);
    assert(wants->items.count == 4 && receives->items.count == 5);
    assert(wants->items.items[1].as.ref == &a->row && wants->items.items[2].as.ref == &b->row);
    assert(receives->items.items[1].as.ref == &a->row && receives->items.items[2].as.ref == &b->row &&
           receives->items.items[3].as.ref == &b->row);
    printf("  wants more: w=%d, receives: w=%d\n", pico8_fixed_flr_int(wants->w), pico8_fixed_flr_int(receives->w));
    assert(wants->w == pico8_fixed_from_int(64) && receives->w == pico8_fixed_from_int(64));
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

/* `e` is a generated row with a key no member takes, or too many items. */
static void keys_of_row(void *e) {
    pico8_value_t items[2];
    (void)pico8_lq_keys_of_row(e, items, 2);
}

static void ol_on_other(void *e) {
    pico8_lq_ol(&gfx, &mem, e, vec_xy(0, 0), 0);
}

static void button_on_other(void *e) {
    pico8_js_keys_t spec = {0};
    spec.lq = pico8_value_ref(e);
    (void)pico8_js_new(&spec);
}

/* A row is a struct whose descriptor has `ol`, no other method; the class
 * keeps no `pq` and no `ol` default. A generated row gives its keys but for `fn`, `qe` or more items than the caller
 * holds, and `ol` refuses anything that is not a row, as `js:pq` refuses
 * another struct. An `hp:lq` row holds the positionals `pq` saw, then the
 * two added after. */
static void test_lq_is_a_struct(void) {
    static const pico8_jb_item_t three[] = {{.s = "a"}, {.is_fixed = 1, .fixed = 0x8000}, {.row = &pico8_dr_table[0]}};
    static const pico8_jb_row_t with_fn = {.count = 1, .items = three, .fn = "ld"};
    static const pico8_jb_row_t with_qe = {.count = 1, .items = three, .has_qe = 1, .qe = 0x70000};
    static const pico8_jb_row_t too_long = {.count = 3, .items = three};
    static const pico8_jb_row_t nested = {.count = 2, .items = three + 1, .has_c = 1, .c = 7};
    const pico8_struct_class_t *cls = pico8_lq_struct_class();
    pico8_value_t a[] = {str("hi"), num(12)};
    pico8_value_t items[2];
    pico8_lq_keys_t t = with_c(positionals(2, a), 9);
    pico8_lq_t *lq;
    pico8_lq_t *row;
    int k;
    int i;

    lq = pico8_lq_new(&t);
    assert(pico8_type_tag_of(lq) == PICO8_TYPE_TAG_STRUCT && lq->header.cls == cls);
    assert(cls->ol != NULL);
    assert(cls->kj == NULL && cls->ev == NULL && cls->hd == NULL && cls->hk == NULL && cls->hb == NULL &&
           cls->ldown == NULL && cls->rdown == NULL && cls->gv == NULL && cls->hq == NULL && cls->ey == NULL &&
           cls->kf == NULL && cls->cn == NULL && cls->mm == NULL);
    for (i = 0; i < PICO8_STATE_COUNT; i++) {
        assert(cls->states[i] == NULL);
    }
    assert(pico8_lq_ready());
    assert(lq->has_c && lq->c == pico8_fixed_from_int(9));
    assert(!lq->has_bg);

    t = pico8_lq_keys_of_row(&nested, items, 2);
    assert(t.items == items && t.count == 2 && t.has_c && t.c == pico8_fixed_from_int(7));
    assert(!t.has_bg && !t.has_score && t.ra == NULL);
    assert(items[0].kind == PICO8_VALUE_FIXED && items[0].as.fixed == 0x8000);
    assert(items[1].kind == PICO8_VALUE_REF && items[1].as.ref == &pico8_dr_table[0]);

    start_game();
    row = pico8_hp_lq(project(1));
    k = fs_count(row);
    assert(row_count(row) == k + 2);

    must_abort(keys_of_row, (void *)&with_fn);
    must_abort(keys_of_row, (void *)&with_qe);
    must_abort(keys_of_row, (void *)&too_long);
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
    in_fresh_child(test_planet_panel);
    in_fresh_child(test_gh_wants_more);
    in_fresh_child(test_lq_is_a_struct);

    printf("test_pico8_lq: all tests passed\n");
    return 0;
}
