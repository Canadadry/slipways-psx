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

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/layout.h"
#include "p8/mem.h"
#include "game/draw_functions.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/research.h"
#include "game/research_display.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "engine/vec.h"
#include "engine/zone.h"
#include "engine/allocator.h"
#include "engine/frame_zone.h"
#include "game/ui_zone.h"
#include "engine/game_zone.h"
#include "heap_allocator.h"

/* `gb[12]`, a colony that needs two resources and exports one; `gb[3]`, a
 * project with an `o` and no `i`, which exports to any planet a slipway
 * joins it to. */
#define COLONY 12
#define EXPORTER 3

#define CONSTRUCTIONS_HASH 0xE8FF55CBu
#define PLANET_RESOURCE_ROW_HASH 0xA122C4CCu
#define RESEARCH_PROJECT_LAYOUT_ROW_HASH 0x9071EEC9u
#define CALLS_HASH 0x26A3DB53u
#define PANEL_HASH 0xDA5A7F3Eu

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static const layout_row_t *as_layout_row(const void *lq) {
    assert(lq != NULL &&
           ((const entity_t *)lq)->class == layout_row_class());
    return lq;
}

/* The only reader of a row's positionals: how many, then the i-th (0-based)
 * in ascending index order. */
static int row_count(const void *lq) {
    return ((const layout_row_t *)lq)->items_count;
}

static layout_row_cell_t row_item(const void *lq, int i) {
    assert(i >= 0 && i < row_count(lq));
    return ((const layout_row_t *)lq)->items[i];
}

/* The kind words the frozen hashes fold before a value: any other value
 * moves them. */
enum { FIXED_KIND = 0, STRING_KIND = 1, BOOL_KIND = 2, REF_KIND = 3 };

static uint32_t fold_fixed(uint32_t h, p8_fixed_t x) {
    return fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)x);
}

static uint32_t fold_bool(uint32_t h, int b) {
    return fnv(fnv(h, (uint32_t)BOOL_KIND), b ? 1u : 0u);
}

/* The string's bytes, then 0xFF. */
static uint32_t fold_string(uint32_t h, const char *s) {
    h = fnv(h, (uint32_t)STRING_KIND);
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFu);
}

/* The frozen hashes were taken when tuples were ob tables: these are the
 * bytes a table folded for its tag and its two kinds of key. */
enum { VIEW_TAG = 0x9E5C0B7Au, VIEW_INDEX_KEY = 0, VIEW_NAME_KEY = 1 };

static uint32_t fold_tuple_name(uint32_t h, const char *name) {
    return fold_string(fnv(h, (uint32_t)VIEW_NAME_KEY), name);
}

static uint32_t fold_tuple_item(uint32_t h, const data_item_t *item) {
    if (item->text != NULL) {
        return fold_string(h, item->text);
    }
    if (item->is_false) {
        return fold_bool(h, 0);
    }
    assert(!item->is_ref && "no test folds a tuple item that points (row, layout row, vector): to fold one, fold REF_KIND then 0xDEAD if NULL, else 0 with no rank, as the frozen hashes did");
    return fold_fixed(h, item->is_fixed ? item->fixed : p8_fixed_from_int(item->number));
}

/* A tuple folded as the frozen hashes folded the table it was: items, then `qe`,
 * then `c`. */
static uint32_t fold_tuple(uint32_t h, const data_row_t *t, int depth) {
    int i;

    h = fnv(fnv(h, (uint32_t)REF_KIND), (uint32_t)VIEW_TAG);
    if (depth <= 0) {
        return h;
    }
    h = fnv(h, (uint32_t)(t->count + (t->has_resource != 0) + (t->has_color != 0)));
    for (i = 0; i < t->count; i++) {
        h = fnv(fnv(h, (uint32_t)VIEW_INDEX_KEY), (uint32_t)(i + 1));
        h = fold_tuple_item(h, &t->items[i]);
    }
    if (t->has_resource) {
        h = fold_fixed(fold_tuple_name(h, "resource"), t->resource);
    }
    if (t->has_color) {
        h = fold_fixed(fold_tuple_name(h, "color"), p8_fixed_from_int(t->color));
    }
    return h;
}

/* The kind of value a cell was sorted from; NOTHING as false. */
static uint32_t cell_value_kind(layout_row_cell_t c) {
    switch (c.kind) {
    case LAYOUT_ROW_CELL_FIXED:
        return FIXED_KIND;
    case LAYOUT_ROW_CELL_STRING:
        return STRING_KIND;
    case LAYOUT_ROW_CELL_ROW:
        return REF_KIND;
    default:
        return BOOL_KIND;
    }
}

/* The same kind and, for its kind, the same number, text or row. A row's
 * text is its own copy, a row borrowed. */
static int same_cell(layout_row_cell_t a, layout_row_cell_t b) {
    if (a.kind != b.kind) {
        return 0;
    }
    switch (a.kind) {
    case LAYOUT_ROW_CELL_FIXED:
        return a.as.fixed == b.as.fixed;
    case LAYOUT_ROW_CELL_STRING:
        return a.as.text != b.as.text && strcmp(a.as.text, b.as.text) == 0;
    case LAYOUT_ROW_CELL_ROW:
        return a.as.row == b.as.row;
    default:
        return 1;
    }
}

/* The frozen hashes folded the value a positional was, nil and false apart:
 * a NOTHING cell folds as nil when `nil` is set (the test's keys said nil
 * there), as false otherwise (no row `src/` builds holds a nil at the top
 * level). */
static uint32_t fold_cell(uint32_t h, layout_row_cell_t c, int nil) {
    switch (c.kind) {
    case LAYOUT_ROW_CELL_ROW:
        return fold_tuple(h, c.as.row, 2);
    case LAYOUT_ROW_CELL_FIXED:
        return fold_fixed(h, c.as.fixed);
    case LAYOUT_ROW_CELL_STRING:
        return fold_string(h, c.as.text);
    case LAYOUT_ROW_CELL_NOTHING:
        /* The bytes the frozen hashes fold for a nil and a false key, which
         * CONSTRUCTIONS_HASH holds apart. */
        return nil ? fnv(fnv(h, 3u), 0xDEADu) : fnv(fnv(h, 2u), 0u);
    default:
        return fold_bool(h, 1);
    }
}

/* `from`: the keys the row was built from, NULL when `src/` built it. Bit
 * `i` of `nil_at`: the key `i` was nil, not false. */
static uint32_t fold_positionals(uint32_t h, const void *lq, const layout_row_keys_t *from, unsigned nil_at) {
    int n = row_count(lq);
    int i;
    assert(from == NULL || from->count == n);
    assert((from != NULL || nil_at == 0) && "nil_at: only keys the test wrote can be absent");
    assert((nil_at >> n) == 0 && "nil_at: a bit past the last key");
    h = fnv(h, (uint32_t)n);
    for (i = 0; i < n; i++) {
        layout_row_cell_t c = row_item(lq, i);
        int nil = ((nil_at >> i) & 1u) != 0;
        if (from != NULL) {
            assert(same_cell(c, from->items[i]));
        }
        assert((!nil || c.kind == LAYOUT_ROW_CELL_NOTHING) && "nil_at: a bit on a cell that is not NOTHING");
        h = fold_cell(h, c, nil);
    }
    return h;
}

static int placed_items_count(const void *lq) {
    assert(as_layout_row(lq)->placed_items != NULL);
    return as_layout_row(lq)->placed_items_count;
}

static p8_fixed_t width_of(const void *lq) {
    return as_layout_row(lq)->width;
}

/* A number's or a boolean's kind word and value; an unset flag folds as
 * absent. */
static uint32_t fold_fixed_member(uint32_t h, int has, p8_fixed_t v) {
    return has ? fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)v) : fnv(h, 0xFFFFFFFFu);
}

static uint32_t fold_background(uint32_t h, const layout_row_t *lq) {
    if (lq->has_background && lq->background_is_bool) {
        return fnv(fnv(h, (uint32_t)BOOL_KIND), (uint32_t)(lq->background != 0));
    }
    return fold_fixed_member(h, lq->has_background, lq->background);
}

/* `c`, `bg`, `score` as values; `ra` as the vector it points at. */
static uint32_t fold_named(uint32_t h, const void *row) {
    const layout_row_t *lq = as_layout_row(row);
    const vec_t *ra;

    h = fold_fixed_member(h, lq->has_color, lq->color);
    h = fold_background(h, lq);
    h = fold_fixed_member(h, lq->has_score, lq->score);
    ra = lq->pos;
    if (ra == NULL) {
        return fnv(h, 0xFFFFFFFFu);
    }
    h = fnv(h, (uint32_t)ra->x);
    return fnv(h, (uint32_t)ra->y);
}

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)p8_peek(&mem, P8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static vec_t vec_xy(int x, int y) {
    return vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y));
}

static vec_t *vec_ref(vec_t at) {
    vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

static layout_row_cell_t num(int n) {
    return layout_row_cell_fixed(p8_fixed_from_int(n));
}

static layout_row_cell_t str(const char *s) {
    return layout_row_cell_string(s);
}

/* A tuple of numbers, never freed. */
static layout_row_cell_t tuple_of(int n, const int *items) {
    data_row_t *t = calloc(1, sizeof *t);
    data_item_t *it = calloc((size_t)n, sizeof *it);
    int i;
    for (i = 0; i < n; i++) {
        it[i].number = items[i];
    }
    t->count = n;
    t->items = it;
    return layout_row_cell_row(t);
}

/* `ol` of `lq` on a cleared screen, at `a` 0, 0.5 and 1. */
static uint32_t fold_drawn(uint32_t h, const void *lq) {
    static const p8_fixed_t as[] = {0, P8_FIXED_LIT(0x00008000, 0.5), P8_FIXED_LIT(0x00010000, 1.0)};
    size_t n;

    for (n = 0; n < sizeof as / sizeof as[0]; n++) {
        p8_gfx_init(&gfx);
        p8_cls(&mem);
        layout_row_draw_at(&gfx, &mem, lq, vec_xy(64, 60), as[n]);
        h = fnv(h, screen_hash());
    }
    return h;
}

static uint32_t fold_row_from(uint32_t h, const void *lq, const layout_row_keys_t *from, unsigned nil_at) {
    h = fold_drawn(h, lq);
    h = fnv(h, (uint32_t)width_of(lq));
    h = fnv(h, (uint32_t)placed_items_count(lq));
    h = fold_positionals(h, lq, from, nil_at);
    return fold_named(h, lq);
}

static uint32_t fold_row(uint32_t h, const void *lq) {
    return fold_row_from(h, lq, NULL, 0u);
}

/* Builds the row from `k` (NULL: no keys), folds it; `nil_at` as in
 * fold_positionals. */
static uint32_t fold_built_from(uint32_t h, const char *label, const layout_row_keys_t *k, unsigned nil_at) {
    const void *lq = layout_row_new(&heap_allocator, k);
    uint32_t one = fold_row_from(2166136261u, lq, k, nil_at);

    assert(((const entity_t *)lq)->pos == &as_layout_row(lq)->pos);

    printf("  %s: w=%d fs=%d n=%d 0x%08X\n", label, p8_fixed_flr_int(width_of(lq)), placed_items_count(lq), row_count(lq),
           (unsigned)one);
    return fnv(h, one);
}

/* The keys of `n` positionals, `items` borrowed, no named key. */
static layout_row_keys_t positionals(int n, const layout_row_cell_t *items) {
    layout_row_keys_t k = {0};

    k.items = items;
    k.count = n;
    return k;
}

static layout_row_keys_t with_color(layout_row_keys_t k, int c) {
    k.has_color = 1;
    k.color = p8_fixed_from_int(c);
    return k;
}

/* `bg` a number, or false for a negative `bg`. */
static layout_row_keys_t with_background(layout_row_keys_t k, int bg) {
    k.has_background = 1;
    k.background_is_bool = bg < 0;
    k.background = bg < 0 ? 0 : p8_fixed_from_int(bg);
    return k;
}

static layout_row_keys_t with_score(layout_row_keys_t k, int score) {
    k.has_score = 1;
    k.score = p8_fixed_from_int(score);
    return k;
}

static layout_row_keys_t with_pos(layout_row_keys_t k, vec_t *ra) {
    k.pos = ra;
    return k;
}

/* Every kind of item `mn` knows, the named keys each constructor passes,
 * no keys at all, the `score` row `pj:ph` passes, its hole already closed
 * (TROU-25), and the stars' row in index order. */
static void test_constructions(void) {
    uint32_t hash = 2166136261u;
    layout_row_keys_t keys;

    {
        layout_row_cell_t a[] = {str("hello")};
        keys = positionals(1, a);
        hash = fold_built_from(hash, "string", &keys, 0u);
    }
    {
        layout_row_cell_t a[] = {str("hello")};
        keys = with_color(positionals(1, a), 13);
        hash = fold_built_from(hash, "string color", &keys, 0u);
    }
    {
        layout_row_cell_t a[] = {num(12)};
        keys = positionals(1, a);
        hash = fold_built_from(hash, "sprite", &keys, 0u);
    }
    {
        static const int f[] = {12, 10, 3, 1};
        layout_row_cell_t a[1];
        a[0] = tuple_of(4, f);
        keys = positionals(1, a);
        hash = fold_built_from(hash, "sprite_id width sprite_palette_block spacing", &keys, 0u);
        assert(layout_row_new(&heap_allocator, &keys)->placed_items[0].sprite_palette_block == p8_fixed_from_int(3));
    }
    {
        data_row_t *qe = calloc(1, sizeof *qe);
        layout_row_cell_t a[1];
        qe->has_resource = 1;
        qe->resource = p8_fixed_from_int(14);
        a[0] = layout_row_cell_row(qe);
        keys = positionals(1, a);
        hash = fold_built_from(hash, "resource", &keys, 0u);
    }
    {
        layout_row_cell_t a[] = {layout_row_cell_nothing()};
        keys = positionals(1, a);
        hash = fold_built_from(hash, "false", &keys, 0u);
    }
    {
        layout_row_cell_t a[] = {layout_row_cell_nothing()};
        keys = positionals(1, a);
        hash = fold_built_from(hash, "nothing", &keys, 1u << 0);
    }
    {
        static const int f[] = {13, 8};
        layout_row_cell_t a[] = {str("a"), num(12), layout_row_cell_nothing(), layout_row_cell_nothing(), str("bc"),
                               layout_row_cell_nothing(), num(40)};
        a[2] = tuple_of(2, f);
        keys = with_color(positionals(7, a), 9);
        hash = fold_built_from(hash, "mixed color", &keys, 1u << 5);
    }
    hash = fold_built_from(hash, "NULL", NULL, 0u);
    {
        layout_row_cell_t a[] = {str("score"), str("")};
        keys = with_color(with_pos(with_score(positionals(2, a), 0), vec_ref(vec_xy(64, 50))), 5);
        hash = fold_built_from(hash, "hole", &keys, 0u);
    }
    {
        layout_row_cell_t a[] = {num(19), str("x"), num(21), num(20), num(22)};
        keys = with_pos(with_score(positionals(5, a), 3), vec_ref(vec_xy(64, 40)));
        hash = fold_built_from(hash, "disorder", &keys, 0u);
    }
    {
        layout_row_cell_t a[] = {str("3"), num(12)};
        keys = with_color(with_background(positionals(2, a), 8), 7);
        hash = fold_built_from(hash, "background", &keys, 0u);
    }
    {
        layout_row_cell_t a[] = {str("4")};
        keys = with_background(positionals(1, a), -1);
        hash = fold_built_from(hash, "background false", &keys, 0u);
    }
    check("constructions", hash, CONSTRUCTIONS_HASH);
}

static void *project(int k) {
    assert(research_projects_at(k) != NULL);
    return research_projects_at(k);
}

/* `jo.ex` on the `i`/`o` of three projects, over a spread of `jq`/`mj`/`d`
 * that includes `jq>5`, the branch no replay reaches; then with no `i`. */
static void test_planet_resource_row(void) {
    static const int ks[] = {1, 7, 20};
    static const int jqs[] = {1, 2, 4, 6, 9};
    uint32_t h = 2166136261u;
    size_t a;
    size_t b;
    p8_fixed_t o;

    for (a = 0; a < sizeof ks / sizeof ks[0]; a++) {
        const research_project_t *p = project(ks[a]);
        const pointer_list_t *is = p->inputs;
        assert(p->has_output);
        o = p->output;
        for (b = 0; b < sizeof jqs / sizeof jqs[0]; b++) {
            const void *lq = research_display_planet_resource_row(frame_zone(), is, &o, p8_fixed_from_int(jqs[b]),
                                         p8_fixed_from_int((int)b % 3), p8_fixed_from_int((int)b % 2));
            assert(lq != NULL);
            assert(as_layout_row(lq)->has_color == (jqs[b] > 5));
            if (jqs[b] > 5) {
                assert(as_layout_row(lq)->color == p8_fixed_from_int(13));
            }
            h = fold_row(h, lq);
        }
    }
    {
        const void *lq =
            research_display_planet_resource_row(frame_zone(), NULL, &o, p8_fixed_from_int(2), 0, p8_fixed_from_int(1));
        assert(lq != NULL);
        h = fold_row(h, lq);
    }
    assert(research_display_planet_resource_row(frame_zone(), NULL, NULL, p8_fixed_from_int(1), 0, 0) == NULL);
    check("planet_resource_row", h, PLANET_RESOURCE_ROW_HASH);
}

static void start_game(void);

/* `hp:lq` appends `fk` and `px` after `pq`: `fs` and `w` do not see them,
 * the copy `js:pq` makes of the row does. Then two buttons with an `h_`,
 * its `bg` a number and false. */
static void test_research_project_layout_row_and_copy(void) {
    static const int project_ranks[] = {1, 5, 12};
    uint32_t hash = 2166136261u;
    size_t allocated;

    start_game();
    for (allocated = 0; allocated < sizeof project_ranks / sizeof project_ranks[0]; allocated++) {
        void *lq = research_project_layout_row(project(project_ranks[allocated]));
        button_keys_t spec = {0};
        button_t *button;
        const void *copy;

        assert(lq != NULL);
        assert(placed_items_count(lq) + 2 == row_count(lq));
        hash = fold_row(hash, lq);

        spec.layout_row_copy = lq;
        button = button_new(&spec);
        copy = button->layout_row;
        assert(copy != NULL && copy != lq);
        assert(placed_items_count(copy) == row_count(copy) && row_count(copy) == row_count(lq));
        hash = fold_row(hash, copy);
        hash = fnv(hash, (uint32_t)button->width);
        p8_gfx_init(&gfx);
        p8_cls(&mem);
        button_draw(&gfx, &mem, button, vec_xy(10, 50));
        hash = fnv(hash, screen_hash());
    }

    for (allocated = 0; allocated < 2; allocated++) {
        layout_row_cell_t layout_row_items[] = {str("ab"), num(12)};
        layout_row_cell_t h_items[] = {str(allocated == 0 ? "3" : "12")};
        layout_row_keys_t row_keys = with_color(positionals(2, layout_row_items), 9);
        layout_row_keys_t h_t = with_background(with_color(positionals(1, h_items), 10), allocated == 0 ? 8 : -1);
        button_keys_t spec = {0};
        button_t *button;
        const void *h_;

        spec.layout_row_keys = &row_keys;
        spec.has_side_tooltip = 1;
        spec.side_tooltip_row = &h_t;
        button = button_new(&spec);
        hash = fold_row(hash, button->layout_row);
        h_ = button->side_tooltip;
        assert(h_ != NULL);
        hash = fold_row(hash, h_);
        hash = fnv(hash, (uint32_t)button->width);
        p8_gfx_init(&gfx);
        p8_cls(&mem);
        button_draw(&gfx, &mem, button, vec_xy(10, 50));
        hash = fnv(hash, screen_hash());
    }
    check("research_project_layout_row", hash, RESEARCH_PROJECT_LAYOUT_ROW_HASH);
}

static uint32_t drawn_by_draw_at(const void *lq, vec_t at, p8_fixed_t a) {
    p8_gfx_init(&gfx);
    p8_cls(&mem);
    layout_row_draw_at(&gfx, &mem, lq, at, a);
    return screen_hash();
}

/* `mv(lq,"ol",p,a)` as `jo:kj` calls it and `{lq,ra,0.5,fn="ld"}` as
 * `pj:ph` hands it to `bl`, each drawing what `ol` draws. */
static void test_calls(void) {
    static const int tuple_values[] = {13, 8};
    layout_row_cell_t cells[] = {str("hi"), num(12), layout_row_cell_nothing()};
    layout_row_keys_t keys;
    void *layout_row;
    data_item_t entry_items[3];
    data_row_t entry = {0};
    const data_row_t *rows[1];
    uint32_t h = 2166136261u;
    uint32_t want;

    cells[2] = tuple_of(2, tuple_values);
    keys = with_color(positionals(3, cells), 11);
    layout_row = layout_row_new(&heap_allocator, &keys);

    want = drawn_by_draw_at(layout_row, vec_xy(64, 30), P8_FIXED_LIT(0x00008000, 0.5));
    h = fnv(h, want);
    p8_gfx_init(&gfx);
    p8_cls(&mem);
    layout_row_class()->draw_at(layout_row, vec_xy(64, 30), P8_FIXED_LIT(0x00008000, 0.5));
    assert(screen_hash() == want);

    entry_items[0] = (data_item_t){.is_ref = 1, .ref = layout_row};
    entry_items[1] = (data_item_t){.is_ref = 1, .ref = vec_ref(vec_xy(64, 30))};
    entry_items[2] = (data_item_t){.is_fixed = 1, .fixed = P8_FIXED_LIT(0x00008000, 0.5)};
    entry.count = 3;
    entry.items = entry_items;
    entry.draw_function = "draw_row";
    rows[0] = &entry;
    p8_gfx_init(&gfx);
    p8_cls(&mem);
    draw_functions_draw_rows_default(&gfx, &mem, rows, 1);
    assert(screen_hash() == want);

    want = drawn_by_draw_at(layout_row, vec_xy(20, 90), 0);
    h = fnv(h, want);
    p8_gfx_init(&gfx);
    p8_cls(&mem);
    layout_row_draw_at_default(&gfx, &mem, layout_row, vec_xy(20, 90));
    assert(screen_hash() == want);

    p8_gfx_init(&gfx);
    p8_cls(&mem);
    want = screen_hash();
    layout_row_draw_at(&gfx, &mem, NULL, vec_xy(20, 90), 0);
    assert(screen_hash() == want);
    check("calls", h, CALLS_HASH);
}

BUMP_ZONE_STORAGE(panel_cells, 4096);

/* Built, grown past its first cells by `lq_add`, then laid out anew by
 * `lq_copy`, all in `zone`. */
static uint32_t drawn_after_adds(const allocator_t *zone) {
    layout_row_cell_t a[] = {str("ab"), num(12)};
    layout_row_keys_t t = with_color(positionals(2, a), 9);
    layout_row_t *lq = layout_row_new(zone, &t);
    layout_row_t *copy;
    int i;

    for (i = 0; i < 6; i++) {
        layout_row_add(lq, i % 2 ? num(16 + i) : str("cd"));
    }
    copy = layout_row_copy(zone, lq);
    assert(row_count(copy) == 8);
    return drawn_by_draw_at(copy, vec_xy(64, 60), P8_FIXED_LIT(0x00008000, 0.5));
}

/* `lq_add` grows `items` in the zone the panel kept: a panel in a bump
 * zone draws what the same panel draws in the transient zone. */
static void test_layout_row_add_in_bump_zone(void) {
    bump_zone_t bump;
    allocator_t handle;
    uint32_t want = drawn_after_adds(&heap_allocator);

    bump_zone_init(&bump, "test-panel", panel_cells, 4096);
    handle = allocator_from_bump_zone(&bump);
    assert(drawn_after_adds(&handle) == want);
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    cart_init();
    cart_start_game(NULL);
}

static planet_t *as_planet(void *e) {
    assert(planet_is_family(e));
    return e;
}

static vec_t pos_of(void *e) {
    const vec_t *p = as_planet(e)->map_pos;
    assert(p != NULL);
    return *p;
}

static p8_fixed_t dist2(vec_t a, vec_t b) {
    vec_t d = vec_sub(a, b);
    return p8_fixed_add(p8_fixed_mul(d.x, d.x), p8_fixed_mul(d.y, d.y));
}

/* A generated planet colonized by COLONY, its nearest one by EXPORTER, the
 * two joined by a slipway paid by `ey` as a drag pays it (`jc:hq` reads the
 * hovered planet off `ok.o_`), left to settle; then the colony's panel,
 * `jo:gh`, up and drawn. */
static void built_planet_panel(void) {
    const entity_list_t *jos;
    slipway_keys_t slipway_keys = {0};
    vec_t *mp = malloc(sizeof *mp);
    void *qu;
    void *re = NULL;
    slipway_t *jc;
    int i;

    start_game();
    jos = entities_by_tag("planet");
    assert(jos != NULL && jos->count > 1);
    qu = jos->items[0];
    for (i = 1; i < jos->count; i++) {
        void *e = jos->items[i];
        if (re == NULL || dist2(pos_of(e), pos_of(qu)) < dist2(pos_of(re), pos_of(qu))) {
            re = e;
        }
    }
    planet_apply_project(qu, project(COLONY));
    planet_apply_project(re, project(EXPORTER));

    slipway_keys.start = qu;
    jc = slipway_new(&slipway_keys);
    *mp = vec_add(vec_mul(pos_of(re), p8_fixed_from_int(128)),
                        vec_make(p8_fixed_from_int(1), p8_fixed_from_int(1)));
    main_ui()->hovered = re;
    slipway_class()->drag(&jc->header, mp, NULL);
    main_ui()->hovered = NULL;
    while (jc->header.state_time < p8_fixed_from_int(15)) {
        update_entities();
    }
    slipway_class()->drop(&jc->header, re);
    for (i = 0; i < 120; i++) {
        update_entities();
    }
    planet_inspection_panel(qu);
    cart_update60();
    cart_draw();
}

/* A button's `lq` or `h_`, NULL when nil or false. */
static uint32_t fold_button_row(uint32_t h, const layout_row_t *lq) {
    int i;

    if (lq == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)width_of(lq));
    h = fold_fixed_member(h, lq->has_color, lq->color);
    h = fold_background(h, lq);
    h = fnv(h, (uint32_t)row_count(lq));
    for (i = 0; i < row_count(lq); i++) {
        h = fnv(h, (uint32_t)cell_value_kind(row_item(lq, i)));
    }
    return h;
}

/* With a built planet's panel up, every button of `ok.le`: its `lq` and
 * `h_`; then the screen. */
static void test_planet_panel(void) {
    uint32_t h = 2166136261u;
    const entity_list_t *le;
    int buttons = 0;
    int i;

    built_planet_panel();
    le = main_ui()->ephemerals;
    assert(le != NULL);
    for (i = 0; i < le->count; i++) {
        const void *button = le->items[i];
        if (button != NULL &&
            ((const entity_t *)button)->class == button_class()) {
            buttons++;
            h = fold_button_row(h, ((const button_t *)button)->layout_row);
            h = fold_button_row(h, ((const button_t *)button)->side_tooltip);
        }
    }
    printf("  panel buttons: %d\n", buttons);
    assert(buttons > 0);
    check("panel", fnv(h, screen_hash()), PANEL_HASH);
}

/* The row of `ok.le` whose first item is `head`, NULL when none. */
static const layout_row_t *panel_row(const char *head) {
    const entity_list_t *le = main_ui()->ephemerals;
    int i;

    for (i = 0; le != NULL && i < le->count; i++) {
        const entity_t *button = le->items[i];
        const layout_row_t *lq;
        layout_row_cell_t v;

        assert(button != NULL && button->class == button_class() && "the main ui's ephemerals hold buttons only");
        lq = ((const button_t *)button)->layout_row;

        if (lq != NULL && lq->items_count > 0) {
            v = lq->items[0];
            if (v.kind == LAYOUT_ROW_CELL_STRING && strcmp(v.as.text, head) == 0) {
                return lq;
            }
        }
    }
    return NULL;
}

/* A planet whose every want arrives lists each under "wants more:", and
 * `gc` times under "receives:", as the `{qe=x}` `lq:mn` reads: a want of a
 * resource draws it 5 wide, a want of none nothing, 6 wide. */
static void test_inspection_panel_wants_more(void) {
    p8_fixed_t ore = p8_fixed_from_int(21);
    need_t *a = need_new(game_zone(), &ore);
    need_t *b = need_new(game_zone(), NULL);
    pointer_list_t *jy = malloc(sizeof *jy);
    planet_t *planet;
    const layout_row_t *wants;
    const layout_row_t *receives;

    start_game();
    a->has_satisfied = 1;
    a->satisfied = p8_fixed_from_int(1);
    b->has_satisfied = 1;
    b->satisfied = p8_fixed_from_int(2);
    pointer_list_init(jy, &heap_allocator);
    pointer_list_add(jy, a);
    pointer_list_add(jy, b);
    planet = planet_new(NULL);
    planet->needs = jy;
    planet->has_development_tier = 1;
    planet->development_tier = p8_fixed_from_int(1);
    planet->pos = malloc(sizeof *planet->pos);
    *planet->pos = vec_xy(64, 64);
    planet_inspection_panel(planet);

    wants = panel_row("wants more:");
    receives = panel_row("receives:");
    assert(wants != NULL && receives != NULL && panel_row("needs:") == NULL);
    assert(wants->items_count == 4 && receives->items_count == 5);
    assert(wants->items[1].as.row == &a->row && wants->items[2].as.row == &b->row);
    assert(receives->items[1].as.row == &a->row && receives->items[2].as.row == &b->row &&
           receives->items[3].as.row == &b->row);
    printf("  wants more: w=%d, receives: w=%d\n", p8_fixed_flr_int(wants->width), p8_fixed_flr_int(receives->width));
    assert(wants->width == p8_fixed_from_int(64) && receives->width == p8_fixed_from_int(64));
}

/* A planet whose `kw.qe` is `r.w` has no "wants more:" row, though every
 * want arrives. */
static void test_inspection_panel_w_maker_wants_no_more(void) {
    p8_fixed_t ore = p8_fixed_from_int(21);
    need_t *a = need_new(game_zone(), &ore);
    pointer_list_t *jy = malloc(sizeof *jy);
    production_t *kw = calloc(1, sizeof *kw);
    planet_t *planet;
    int r_w;

    start_game();
    assert(resource_code_lookup("w", &r_w));
    kw->has_resource = 1;
    kw->resource = p8_fixed_from_int(r_w);
    a->has_satisfied = 1;
    a->satisfied = p8_fixed_from_int(1);
    pointer_list_init(jy, &heap_allocator);
    pointer_list_add(jy, a);
    planet = planet_new(NULL);
    planet->needs = jy;
    planet->production = kw;
    planet->has_development_tier = 1;
    planet->development_tier = p8_fixed_from_int(1);
    planet->pos = malloc(sizeof *planet->pos);
    *planet->pos = vec_xy(64, 64);
    planet_inspection_panel(planet);

    assert(panel_row("wants more:") == NULL && panel_row("needs:") == NULL);
}

/* A laboratory (`qh` true) lists only its last want under "wants more:". */
static void test_inspection_panel_lab_wants_its_last(void) {
    need_t *a = need_new(game_zone(), NULL);
    need_t *b = need_new(game_zone(), NULL);
    pointer_list_t *jy = malloc(sizeof *jy);
    planet_t *lab;
    const layout_row_t *wants;

    start_game();
    a->has_satisfied = 1;
    a->satisfied = p8_fixed_from_int(1);
    b->has_satisfied = 1;
    b->satisfied = p8_fixed_from_int(1);
    pointer_list_init(jy, &heap_allocator);
    pointer_list_add(jy, a);
    pointer_list_add(jy, b);
    lab = structure_new(NULL);
    lab->needs = jy;
    lab->has_development_tier = 1;
    lab->development_tier = p8_fixed_from_int(1);
    lab->pos = malloc(sizeof *lab->pos);
    *lab->pos = vec_xy(64, 64);
    planet_inspection_panel(lab);

    wants = panel_row("wants more:");
    assert(wants != NULL && wants->items_count == 3 && wants->items[1].as.row == &b->row);
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
    layout_row_cell_t items[2];
    (void)layout_row_keys_of_row(e, items, 2);
}

static void draw_at_on_other(void *e) {
    layout_row_draw_at(&gfx, &mem, e, vec_xy(0, 0), 0);
}

/* A row is a struct whose descriptor has `ol`, no other method; the class
 * keeps no `pq` and no `ol` default. A generated row gives its keys but for `fn`, `qe` or more items than the caller
 * holds, and `ol` refuses anything that is not a row. An `hp:lq` row holds the positionals `pq` saw, then the
 * two added after. */
static void test_layout_row_is_a_struct(void) {
    static const data_item_t three[] = {{.text = "a"}, {.is_fixed = 1, .fixed = 0x8000}, {.row = &icon_tooltips_table[0]}};
    static const data_row_t with_fn = {.count = 1, .items = three, .draw_function = "draw_row"};
    static const data_row_t with_resource = {.count = 1, .items = three, .has_resource = 1, .resource = 0x70000};
    static const data_row_t too_long = {.count = 3, .items = three};
    static const data_row_t nested = {.count = 2, .items = three + 1, .has_color = 1, .color = 7};
    const entity_class_t *row_class = layout_row_class();
    layout_row_cell_t cells[] = {str("hi"), num(12)};
    layout_row_cell_t items[2];
    layout_row_keys_t t = with_color(positionals(2, cells), 9);
    layout_row_t *lq;
    layout_row_t *row;
    layout_row_t *copy;
    int k;
    int i;

    lq = layout_row_new(&heap_allocator, &t);
    assert(lq != NULL && lq->header.class == row_class);
    assert(row_class->draw_at != NULL);
    assert(row_class->draw == NULL && row_class->draw_overlay == NULL && row_class->hit_test == NULL && row_class->new_year == NULL && row_class->refresh == NULL &&
           row_class->left_press == NULL && row_class->right_press == NULL && row_class->accept_drop == NULL && row_class->drag == NULL && row_class->drop == NULL &&
           row_class->attach_slipway == NULL && row_class->offer_trade == NULL && row_class->spawn_particle == NULL);
    for (i = 0; i < STATE_COUNT; i++) {
        assert(row_class->states[i] == NULL);
    }
    assert(layout_row_ready());
    assert(lq->has_color && lq->color == p8_fixed_from_int(9));
    assert(!lq->has_background);
    copy = layout_row_copy(&heap_allocator, lq);
    assert(copy != lq && copy->items != lq->items && copy->items_count == 2);
    assert(copy->has_color && copy->color == p8_fixed_from_int(9) && copy->placed_items_count == 2 && copy->width == lq->width);

    t = layout_row_keys_of_row(&nested, items, 2);
    assert(t.items == items && t.count == 2 && t.has_color && t.color == p8_fixed_from_int(7));
    assert(!t.has_background && !t.has_score && t.pos == NULL);
    assert(items[0].kind == LAYOUT_ROW_CELL_FIXED && items[0].as.fixed == 0x8000);
    assert(items[1].kind == LAYOUT_ROW_CELL_ROW && items[1].as.row == &icon_tooltips_table[0]);

    start_game();
    row = research_project_layout_row(project(1));
    k = placed_items_count(row);
    assert(row_count(row) == k + 2);

    must_abort(keys_of_row, (void *)&with_fn);
    must_abort(keys_of_row, (void *)&with_resource);
    must_abort(keys_of_row, (void *)&too_long);
    must_abort(draw_at_on_other, main_ui());
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

/* The largest row `jo:hb` can build: a planet of `r.w`, so two `ml`,
 * with the most wants a project has and `jq` 5, the most drawn one by one.
 * It fits in one row place. */
static void test_planet_resource_row_largest_row_fits_a_slot(void) {
    const pointer_list_t *is = NULL;
    const allocator_t *slot;
    p8_fixed_t o;
    int r_w;
    int k;

    for (k = 1; research_projects_at(k) != NULL; k++) {
        const research_project_t *p = research_projects_at(k);
        if (p->inputs != NULL && (is == NULL || p->inputs->count > is->count)) {
            is = p->inputs;
        }
    }
    assert(is != NULL && is->count == 2 && "the most wants a project has; a longer input list needs a bigger row slot");
    assert(resource_code_lookup("w", &r_w));
    o = p8_fixed_from_int(r_w);
    slot = ui_zone_slot(SLOT_KIND_ROW);
    assert(research_display_planet_resource_row(slot, is, &o, p8_fixed_from_int(5), 0, 0) != NULL);
    arena_release(slot);
}

int main(void) {
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);

    cart_load();
    in_fresh_child(test_constructions);
    in_fresh_child(test_planet_resource_row);
    in_fresh_child(test_planet_resource_row_largest_row_fits_a_slot);
    in_fresh_child(test_research_project_layout_row_and_copy);
    in_fresh_child(test_calls);
    in_fresh_child(test_planet_panel);
    in_fresh_child(test_inspection_panel_wants_more);
    in_fresh_child(test_inspection_panel_w_maker_wants_no_more);
    in_fresh_child(test_inspection_panel_lab_wants_its_last);
    in_fresh_child(test_layout_row_is_a_struct);
    in_fresh_child(test_layout_row_add_in_bump_zone);

    printf("test_layout_row: all tests passed\n");
    return 0;
}
