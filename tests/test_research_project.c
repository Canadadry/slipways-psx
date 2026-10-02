/* The research project `hp`, the 34 entries of `gb` built at cart load, read
 * on its struct members and through `po`/`od`/`jb`/`lq`, then
 * installed on fresh planets by `jo:lz`/`jo:hu` and in games built directly:
 * no events file. */

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
#include "game/economy.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/layout.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/research.h"
#include "game/research_display.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "game/tech_gate.h"
#include "game/ui.h"
#include "engine/vec.h"
#include "engine/zone.h"
#include "engine/game_zone.h"

/* `gb[12]`, a colony that needs two resources and exports one; `gb[3]`, a
 * project with an `o` and no `i`, which exports to any planet a slipway
 * joins it to. */
#define COLONY 12
#define EXPORTER 3

#define GB_COUNT 34
#define GB_NAMED 26
#define GB_HASH 0x537BCD0Fu
#define DEFAULTS_HASH 0xC841FB8Au
#define FUNCTIONS_HASH 0x538CF904u
#define FUNCTIONS_INVENTED_HASH 0xA64C8C0Fu
#define INSTALL_HASH 0x81BCB6CDu
#define BUILT_WORLD_HASH 0xDF3A48A1u
#define HARVESTED_WORLD_HASH 0xE7F2768Cu

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

static int ents_rank_in(const entity_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

/* The projects of `gb` by rank, in order. */
static int gb_count(void) {
    return pico8_gb_count();
}

static void *project(int k) {
    assert(pico8_gb_at(k) != NULL);
    return pico8_gb_at(k);
}

/* 1-based rank of `e` among the positional entries of `gb`, -1 when none. */
static int gb_rank(const void *e) {
    int k;
    for (k = 1; e != NULL && k <= gb_count(); k++) {
        if (project(k) == e) {
            return k;
        }
    }
    return -1;
}

/* The kind words the frozen hashes fold before a value, FUNCTION_KIND the one
 * the name reader gave `lq` when it fell back to the method (GB_HASH); the
 * enum entries are gone. Any other value moves them. */
enum { FIXED_KIND = 0, STRING_KIND = 1, BOOL_KIND = 2, REF_KIND = 3, FUNCTION_KIND = 4 };

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

/* 0xDEAD for NULL, else 0 (no pointer it sees carried a tag) and, above
 * `depth` 0, the rank in `hv` and in `gb`. */
static uint32_t fold_ref(uint32_t h, const void *ref, int depth) {
    h = fnv(h, (uint32_t)REF_KIND);
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, 0u);
    if (depth <= 0) {
        return h;
    }
    h = fnv(h, (uint32_t)ents_rank_in(live_entities(), ref));
    return fnv(h, (uint32_t)gb_rank(ref));
}

/* A want's `qe` with the bytes the frozen hashes fold for it, which the frozen
 * hashes hold: kind word 0 then the number; kind word 3 then 0xDEAD for nil. */
static uint32_t fold_qe(uint32_t h, resource_t qe) {
    assert(qe.kind != RESOURCE_FALSE && "a want's qe is never false");
    return qe.kind == RESOURCE_FIXED ? fnv(fnv(h, 0u), qe.number) : fnv(fnv(h, 3u), 0xDEADu);
}

/* The frozen hashes were taken when rows were ob views and name lists ob
 * tables: these are the bytes they folded for their tag and their two kinds
 * of key, frozen values of a tag and an enum since deleted. Changing one
 * reddens every constant that folds it. */
enum { VIEW_TAG = 0x9E5C0B7Au, VIEW_INDEX_KEY = 0, VIEW_NAME_KEY = 1, VIEW_LIST_TAG = 0x9E5C1157u };

/* `REF`, the count, then per name the index-key kind, its position and
 * the string: the format DEFAULTS_HASH froze. */
static uint32_t fold_names(uint32_t h, const str_seq_t *names) {
    int i;

    h = fnv(h, (uint32_t)REF_KIND);
    h = fnv(h, (uint32_t)names->count);
    for (i = 0; i < names->count; i++) {
        h = fnv(h, (uint32_t)VIEW_INDEX_KEY);
        h = fnv(h, (uint32_t)(i + 1));
        h = fold_string(h, names->items[i]);
    }
    return h;
}

/* Copies of the cart's `mr` and `gp` name lists of `hp`: the port has no
 * such lists, and DEFAULTS_HASH folds these. */
static const char *const cart_mr_items[] = {"px", "lr", "fj", "ja", "i", "o", "il", "dm"};
static const str_seq_t CART_MR = {8, cart_mr_items};
static const char *const cart_gp_items[] = {"o", "dm"};
static const str_seq_t CART_GP = {2, cart_gp_items};

static uint32_t fold_absent(uint32_t h) {
    return fnv(h, 0xFFFFFFFFu);
}

/* A member of one kind: the kind word is constant by
 * the struct's shape, kept so that GB_HASH and INSTALL_HASH still prove the
 * typed reads. A NULL string, or an unset flag, folds as absent. */
static uint32_t fold_string_member(uint32_t h, const char *s) {
    if (s == NULL) {
        return fold_absent(h);
    }
    h = fnv(h, (uint32_t)STRING_KIND);
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFu);
}

static uint32_t fold_fixed_member(uint32_t h, int has, p8_fixed_t v) {
    return has ? fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)v) : fold_absent(h);
}

static uint32_t fold_bool_member(uint32_t h, int has, int b) {
    return has ? fnv(fnv(h, (uint32_t)BOOL_KIND), (uint32_t)(b != 0)) : fold_absent(h);
}

/* `il` false is `has_il` with a NULL `il`. */
static uint32_t fold_il(uint32_t h, const pico8_hp_t *b) {
    if (!b->has_il) {
        return fold_absent(h);
    }
    return b->il == NULL ? fold_bool_member(h, 1, 0) : fold_string_member(h, b->il);
}

static uint32_t fold_view_head(uint32_t h, int count) {
    h = fnv(h, (uint32_t)REF_KIND);
    h = fnv(h, (uint32_t)VIEW_TAG);
    return fnv(h, (uint32_t)count);
}

static uint32_t fold_view_index(uint32_t h, int i) {
    return fnv(fnv(h, (uint32_t)VIEW_INDEX_KEY), (uint32_t)i);
}

static uint32_t fold_view_name(uint32_t h, const char *name) {
    return fold_string(fnv(h, (uint32_t)VIEW_NAME_KEY), name);
}

static uint32_t fold_row(uint32_t h, const data_row_t *row, int depth);

static uint32_t fold_item(uint32_t h, const data_item_t *item, int depth) {
    if (item->row != NULL) {
        return fold_row(h, item->row, depth);
    }
    if (item->text != NULL) {
        return fold_string(h, item->text);
    }
    if (item->is_false) {
        return fold_bool(h, 0);
    }
    if (item->is_ref) {
        return fold_ref(h, item->ref, depth);
    }
    return fold_fixed(h, item->is_fixed ? item->fixed : p8_fixed_from_int(item->number));
}

/* A generated or built row, folded as the frozen hashes folded its view: items,
 * then `c`, then `fn`. */
static uint32_t fold_row(uint32_t h, const data_row_t *row, int depth) {
    int i;

    if (depth <= 0) {
        return fnv(fnv(h, (uint32_t)REF_KIND), (uint32_t)VIEW_TAG);
    }
    h = fold_view_head(h, row->count + (row->has_color != 0) + (row->draw_function != NULL));
    for (i = 0; i < row->count; i++) {
        h = fold_item(fold_view_index(h, i + 1), &row->items[i], depth - 1);
    }
    if (row->has_color) {
        h = fold_fixed(fold_view_name(h, "c"), p8_fixed_from_int(row->color));
    }
    if (row->draw_function != NULL) {
        h = fold_string(fold_view_name(h, "fn"), row->draw_function);
    }
    return h;
}

/* A row's positional as the value it was sorted from folded; NOTHING as
 * false (no row folded here holds a nil at the top level). */
static uint32_t fold_lq_item(uint32_t h, layout_row_cell_t c, int depth) {
    switch (c.kind) {
    case LAYOUT_ROW_CELL_ROW:
        return fold_row(h, c.as.row, depth);
    case LAYOUT_ROW_CELL_FIXED:
        return fold_fixed(h, c.as.fixed);
    case LAYOUT_ROW_CELL_STRING:
        return fold_string(h, c.as.text);
    default:
        return fold_bool(h, c.kind == LAYOUT_ROW_CELL_TRUE);
    }
}

/* A row folded as the frozen hashes folded its table view: its first `#fs`
 * positionals, then `c`, `bg`, `score`, `ra`, `fs`, `w`, then the rest. */
static uint32_t fold_lq_instance(uint32_t h, const void *l) {
    const layout_row_t *lq = l;
    int k;
    int i;

    if (l == NULL) {
        return fnv(h, 0xDEADu);
    }
    assert(lq != NULL && lq->header.class == layout_row_class());
    assert(!lq->has_background);
    assert(lq->placed_items != NULL);
    k = lq->placed_items_count;
    h = fold_view_head(h, lq->items_count + (lq->has_color != 0) +
                              (lq->has_score != 0) + (lq->pos != NULL) + 2);
    for (i = 0; i < k; i++) {
        h = fold_lq_item(fold_view_index(h, i + 1), lq->items[i], 3);
    }
    if (lq->has_color) {
        h = fold_fixed(fold_view_name(h, "c"), lq->color);
    }
    if (lq->has_score) {
        h = fold_fixed(fold_view_name(h, "score"), lq->score);
    }
    if (lq->pos != NULL) {
        h = fold_ref(fold_view_name(h, "ra"), lq->pos, 3);
    }
    /* `fs` as the frozen hashes folded the tagged list it was: REF, its tag, its count, each entry a REF. */
    h = fnv(fnv(fold_view_name(h, "fs"), (uint32_t)REF_KIND), (uint32_t)VIEW_LIST_TAG);
    h = fnv(h, (uint32_t)lq->placed_items_count);
    for (i = 0; i < lq->placed_items_count; i++) {
        h = fold_ref(h, &lq->placed_items[i], 2);
    }
    h = fold_fixed(fold_view_name(h, "w"), lq->width);
    for (i = k; i < lq->items_count; i++) {
        h = fold_lq_item(fold_view_index(h, i + 1), lq->items[i], 3);
    }
    return h;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static void check_count(const char *label, int got, int want) {
    printf("[%s] %d\n", label, got);
    fflush(stdout);
    assert(got == want);
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

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
}

static planet_t *as_jo(void *e) {
    assert(planet_is_family(e));
    return e;
}

static vec_t pos_of(void *e) {
    const vec_t *p = as_jo(e)->map_pos;
    assert(p != NULL);
    return *p;
}

static p8_fixed_t dist2(vec_t a, vec_t b) {
    vec_t d = vec_sub(a, b);
    return p8_fixed_add(p8_fixed_mul(d.x, d.x), p8_fixed_mul(d.y, d.y));
}

/* A generated planet colonized by COLONY, its nearest one by EXPORTER, the
 * two joined by a slipway paid by `ey` as a drag pays it (`jc:hq` reads the
 * hovered planet off `ok.o_`), then left to settle. */
static void built_world(void) {
    const entity_list_t *jos;
    slipway_keys_t jc_keys = {0};
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

    jc_keys.start = qu;
    jc = slipway_new(&jc_keys);
    *mp = vec_add(vec_mul(pos_of(re), p8_fixed_from_int(128)),
                        vec_make(p8_fixed_from_int(1), p8_fixed_from_int(1)));
    pico8_ok()->o_ = re;
    slipway_class()->drag(&jc->header, mp, NULL);
    pico8_ok()->o_ = NULL;
    while (jc->header.state_time < p8_fixed_from_int(15)) {
        update_entities();
    }
    slipway_class()->drop(&jc->header, re);
    for (i = 0; i < 120; i++) {
        update_entities();
    }
}

/* Several projects share a `px`: `gb[px]` is the last one `hp:pq` saw. */
static void *last_with_px(const char *px) {
    void *found = NULL;
    int k;
    for (k = 1; k <= gb_count(); k++) {
        const pico8_hp_t *b = project(k);
        if (b->px != NULL && strcmp(b->px, px) == 0) {
            found = project(k);
        }
    }
    return found;
}

/* Every project of `gb` as `hp:pq` left it: its own fields, `i`, `lq`, the
 * class's `fk` shared by all, and `gb[px]` aliasing the last
 * project of that `px`. */
static void test_gb(void) {
    uint32_t h = 2166136261u;
    uint32_t d = 2166136261u;
    void *fk = NULL;
    int named = 0;
    int k;

    check_count("gb", gb_count(), GB_COUNT);
    for (k = 1; k <= gb_count(); k++) {
        named += pico8_gb_find(((const pico8_hp_t *)project(k))->px) == project(k);
    }
    check_count("gb named", named, GB_NAMED);

    for (k = 1; k <= GB_COUNT; k++) {
        pico8_hp_t *b = project(k);
        const pointer_list_t *i_list;
        void *found;
        int j;

        h = fnv(h, (uint32_t)k);
        h = fold_string_member(h, b->px);
        h = fold_string_member(h, b->lr);
        h = fold_fixed_member(h, b->has_fj, b->fj);
        h = fold_string_member(h, b->ja);
        h = fold_fixed_member(h, b->has_o, b->o);
        h = fold_il(h, b);
        h = fold_fixed_member(h, b->has_dm, b->dm);
        h = fold_string_member(h, b->rr);
        h = fold_fixed_member(h, b->has_ii, b->ii);
        h = fold_string_member(h, b->cw);
        h = fold_string_member(h, b->go);
        h = fold_fixed_member(h, b->has_rn, b->rn);
        i_list = b->i;
        assert(i_list != NULL);
        h = fnv(h, (uint32_t)i_list->count);
        for (j = 0; j < i_list->count; j++) {
            resource_t qe;
            qe = pico8_jy_qe(i_list->items[j]);
            h = fold_qe(h, qe);
        }
        if (b->lq != NULL) {
            const data_row_t *lq = b->lq;
            /* Constant by the struct's shape, kept so that GB_HASH still
             * proves the typed reads. */
            h = fnv(h, (uint32_t)REF_KIND);
            assert(lq == &b->lq_own && b->has_ii);
            for (j = 1; j <= 3; j++) {
                h = j <= lq->count ? fold_item(h, &lq->items[j - 1], 2) : fnv(h, 0xFFFFFFFFu);
            }
        } else {
            h = fnv(h, (uint32_t)FUNCTION_KIND);
            assert(b->lq == NULL && !b->has_ii);
        }

        if (k == 1) {
            fk = (void *)b->def->row_tail;
            assert(fk != NULL);
        }
        assert((void *)b->def->row_tail == fk);
        if (b->px != NULL) {
            found = pico8_gb_find(b->px);
            assert(found != NULL);
            assert(found == last_with_px(b->px));
            h = fnv(h, (uint32_t)gb_rank(found));
        }
    }
    assert(fk == &research_project_row_tail_row);
    d = fold_names(d, &CART_MR);
    d = fold_names(d, &CART_GP);
    d = fold_row(d, fk, 2);
    check("gb", h, GB_HASH);
    check("mr gp fk", d, DEFAULTS_HASH);
}

/* Per project: `po` as its entry in `hc`, `od`, `jb`, `lq`, and what `b.lq`
 * names: the member for a project with `ii`, the method `lq` for the
 * others. */
static uint32_t fold_functions(uint32_t h) {
    int k;

    for (k = 1; k <= GB_COUNT; k++) {
        pico8_hp_t *b = project(k);
        const production_tables_entry_t *po = pico8_hp_po(b);
        const data_row_t *jb = pico8_hp_jb(b);
        uint32_t lq_h;

        h = fnv(h, (uint32_t)k);
        h = fnv(h, (uint32_t)(po == NULL ? -2 : (int)(po - production_tables_table)));
        h = fnv(h, (uint32_t)pico8_hp_od(b));
        h = jb == NULL ? fnv(h, 0xDEADu) : fold_row(h, jb, 4);
        assert((jb == NULL) == (b->has_ii != 0));
        lq_h = fold_lq_instance(2166136261u, pico8_hp_lq(b));
        h = fnv(h, lq_h);
        if (b->lq == NULL) {
            assert(fold_lq_instance(2166136261u, pico8_hp_lq(b)) == lq_h);
        }
    }
    return h;
}

/* `px` invented at no cost: an nb entry whose `nk` (0) no resource of `r`
 * is. Never freed. */
static void invent(const char *px) {
    technologies_entry_t *t = malloc(sizeof *t);

    *t = (technologies_entry_t){px, 0, NULL, 0, 0, NULL};
    pico8_hy_kt(pico8_fm_hy(), t);
    assert(pico8_oj(px));
}

static void test_functions(void) {
    int k;

    assert(pico8_fm_hy() == NULL && !pico8_oj("nanomaterials"));
    start_game();
    assert(!pico8_oj("nanomaterials"));
    check("functions", fold_functions(2166136261u), FUNCTIONS_HASH);

    invent("nanomaterials");
    for (k = 1; k <= GB_COUNT; k++) {
        const pico8_hp_t *b = project(k);
        if (b->has_il && b->il != NULL && !pico8_oj(b->il)) {
            invent(b->il);
        }
    }
    check("functions invented", fold_functions(2166136261u), FUNCTIONS_INVENTED_HASH);

    assert(pico8_hp_po(NULL) == NULL && pico8_hp_od(NULL) == 0);
    assert(pico8_hp_jb(NULL) == NULL && pico8_hp_lq(NULL) == NULL);
}

static vec_t *vec_ref(vec_t at) {
    vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

static vec_t vec_xy(int x, int y) {
    return vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y));
}

/* A bare planet on `id`/`p`/`ra`, `ra` offset from `mo`'s. */
static void *new_planet(const char *id, int x, int y) {
    planet_keys_t keys;
    const vec_t *mo_ra;

    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
    assert(mo_ra != NULL);
    keys.planet_id = id;
    keys.map_pos = vec_ref(vec_xy(x / 2, y / 2));
    keys.pos = vec_ref(vec_add(*mo_ra, vec_xy(x, y)));
    return planet_new(&keys);
}

static int with_go(const pico8_hp_t *b) {
    return b->go != NULL;
}

static int with_cw(const pico8_hp_t *b) {
    return b->cw != NULL;
}

static int ordinary(const pico8_hp_t *b) {
    return b->cw == NULL && b->go == NULL && !b->has_ii;
}

/* `nth` counts from 0. */
static void *project_with(int (*match)(const pico8_hp_t *), int nth) {
    int k;
    for (k = 1; k <= GB_COUNT; k++) {
        void *b = project(k);
        if (match(b) && nth-- == 0) {
            return b;
        }
    }
    assert(0 && "no such project");
    return NULL;
}

/* `kw` by content: a pico8_kw_t has no tag to fold. */
static uint32_t fold_kw(uint32_t h, const planet_t *p) {
    const production_t *k = p->production;

    if (k == NULL) {
        return fnv(h, 0xFFFFFFFFu);
    }
    h = fnv(h, 1u);
    h = k->has_resource ? fnv(fnv(h, 1u), (uint32_t)k->resource) : fnv(h, 0u);
    return fnv(h, (uint32_t)k->amount);
}

/* A planet's `jy`, folded as the frozen hashes folded the list of tables it was:
 * each want, `qe` (nil included), then `gc` when written. */
static uint32_t fold_jy(uint32_t h, const planet_t *p) {
    const pointer_list_t *jy = p->needs;
    int i;

    if (jy == NULL) {
        return fnv(h, 0xFFFFFFFFu);
    }
    h = fnv(fnv(h, (uint32_t)REF_KIND), (uint32_t)VIEW_LIST_TAG);
    h = fnv(h, (uint32_t)jy->count);
    for (i = 0; i < jy->count; i++) {
        const pico8_jy_t *want = jy->items[i];

        h = fold_view_head(h, 1 + (want->has_gc != 0));
        h = fold_qe(fold_view_name(h, "resource"), pico8_jy_qe(want));
        if (want->has_gc) {
            h = fold_fixed(fold_view_name(h, "gc"), want->gc);
        }
    }
    return h;
}

static uint32_t fold_planet(uint32_t h, void *e) {
    const planet_t *p = as_jo(e);

    h = fnv(h, (uint32_t)gb_rank(p->project));
    h = fnv(h, (uint32_t)(p->project != NULL));
    h = fold_string_member(h, p->description);
    h = fold_jy(h, p);
    h = fold_kw(h, p);
    h = fold_string_member(h, p->header.state);
    h = fold_bool_member(h, (p->fold_anchor != 0), p->fold_anchor);
    h = fold_bool_member(h, (p->is_dead != 0), p->is_dead);
    h = fold_fixed_member(h, 1, p->shake);
    h = p->planet_type != NULL ? fnv(h, (uint32_t)(p->planet_type - planet_types_table)) : fnv(h, 0xFFFFFFFFu);
    return fnv(h, (uint32_t)ents_rank_in(live_entities(), p));
}

/* `hp:pq` makes a want per character of `i`, one of a character outside
 * `r` wanting no resource; `jo:lz` copies each want, never shares it. */
static void test_wants(void) {
    project_records_entry_t ri = {0};
    pico8_hp_keys_t keys = {0};
    const pointer_list_t *jy;
    pico8_hp_t *b;
    void *a;
    int p;
    int k;

    start_game();
    ri.npos = 5;
    ri.name = "x";
    ri.planet_codes = "y";
    ri.base_cost = 1;
    ri.production_key = "z";
    ri.inputs = "pz";
    keys.ri = &ri;
    b = pico8_hp_new(&keys);
    assert(b->i->count == 2 && resource_code_lookup("p", &p));
    assert(((const pico8_jy_t *)b->i->items[0])->row.has_resource);
    assert(((const pico8_jy_t *)b->i->items[0])->row.resource == p8_fixed_from_int(p));
    assert(!((const pico8_jy_t *)b->i->items[1])->row.has_resource);

    a = new_planet("p950", 12, 16);
    planet_install_project(a, b);
    jy = as_jo(a)->needs;
    assert(jy != NULL && jy->count == 2);
    for (k = 0; k < 2; k++) {
        assert(jy->items[k] != b->i->items[k]);
        assert(memcmp(jy->items[k], b->i->items[k], sizeof(pico8_jy_t)) == 0);
    }
}

static void test_install(void) {
    uint32_t h = 2166136261u;
    void *plain;
    void *a;
    void *go_project;
    int nth;

    start_game();
    plain = project_with(ordinary, 0);

    a = new_planet("p901", 10, 12);
    planet_install_project(a, plain);
    assert((void *)as_jo(a)->project == plain && as_jo(a)->description == NULL);
    h = fold_planet(h, a);

    a = new_planet("p902", 30, 14);
    planet_apply_project(a, plain);
    assert((void *)as_jo(a)->project == plain);
    h = fold_planet(h, a);

    go_project = project_with(with_go, 0);
    a = new_planet("p903", 50, 16);
    planet_apply_project(a, go_project);
    assert(as_jo(a)->planet_type != NULL);
    h = fold_planet(h, a);

    for (nth = 0; nth < 2; nth++) {
        int before;
        pico8_hp_t *cw = project_with(with_cw, nth);
        a = new_planet(nth == 0 ? "p904" : "p905", 70 + 20 * nth, 18);
        before = live_entities()->count;
        planet_apply_project(a, cw);
        h = fold_string_member(h, cw->cw);
        h = fnv(h, (uint32_t)(live_entities()->count - before));
        h = fold_planet(h, a);
        h = fold_planet(h, live_entities()->items[live_entities()->count - 1]);
    }

    a = new_planet("p906", 110, 20);
    planet_apply_project(a, NULL);
    assert(as_jo(a)->project == NULL);
    h = fold_planet(h, a);
    check("install", h, INSTALL_HASH);
}

/* Each planet of `cg.jo`: the rank in `gb` of its `hn`; then the screen,
 * once a frame is played and drawn. */
static uint32_t fold_world(void) {
    uint32_t h = 2166136261u;
    const entity_list_t *jo;
    int i;

    pico8__update60();
    pico8__draw();
    jo = entities_by_tag("planet");
    for (i = 0; jo != NULL && i < jo->count; i++) {
        h = fnv(h, (uint32_t)gb_rank(as_jo(jo->items[i])->project));
    }
    return fnv(h, screen_hash());
}

static void test_built_world(void) {
    built_world();
    check("built world", fold_world(), BUILT_WORLD_HASH);
}

/* A discovered planet harvested (`cw` "ga"): `jo:hu` swaps it for a `ga`. */
static void test_harvested_world(void) {
    const entity_t *last;
    void *p;
    int i;

    start_game();
    p = entities_by_tag("planet")->items[0];
    planet_roll_type(p);
    planet_apply_project(p, project_with(with_cw, 1));
    last = live_entities()->items[live_entities()->count - 1];
    assert(last != NULL && last->class == pico8_ga_struct_class());
    for (i = 0; i < 60; i++) {
        update_entities();
    }
    check("harvested world", fold_world(), HARVESTED_WORLD_HASH);
}

static size_t octets_d_un_bloc(size_t taille) {
    return ZONE_CELLS(taille) * sizeof(zone_cell_t);
}

/* Each project of `gb`, its list `i`, its items unless empty, its wants. */
static size_t octets_de_gb(void) {
    size_t octets = 0;
    int k;

    for (k = 1; k <= gb_count(); k++) {
        const pico8_hp_t *b = project(k);
        const int n = b->i->count;

        octets += octets_d_un_bloc(sizeof *b) + octets_d_un_bloc(sizeof *b->i) +
                  (size_t)n * octets_d_un_bloc(sizeof(pico8_jy_t));
        if (n > 0) {
            octets += octets_d_un_bloc((size_t)n * sizeof(void *));
        }
    }
    return octets;
}

/* `gb` is all the load sub-zone holds after pico8_cart_load(), each `i`
 * reserved to its exact length. */
static void test_gb_in_chargement(void) {
    int k;

    for (k = 1; k <= gb_count(); k++) {
        const pico8_hp_t *b = project(k);
        assert(b->i->capacity == b->i->count && b->i->zone == game_zone_load_zone());
    }
    printf("  chargement apres pico8_cart_load : %lu\n", (unsigned long)load_zone_used());
    assert(load_zone_used() == octets_de_gb());
}

/* A want is taken from the zone its caller passes. */
static void test_jy_new_in_the_zone_passed(void) {
    const p8_fixed_t qe = p8_fixed_from_int(3);
    const size_t chargement = load_zone_used();
    const size_t partie = game_zone_used();
    pico8_jy_t *a = pico8_jy_new(game_zone_load_zone(), &qe);
    pico8_jy_t *b = pico8_jy_new(game_zone(), NULL);

    assert(load_zone_used() == chargement + octets_d_un_bloc(sizeof *a));
    assert(game_zone_used() == partie + octets_d_un_bloc(sizeof *b));
    assert(a->row.has_resource && a->row.resource == qe && !a->has_gc);
    assert(!b->row.has_resource && !b->has_gc);
}

/* `gb` rebuilt from `fd` forgets the projects it held, whose memory goes
 * back at the next load, and gives the same projects; shutdown leaves no
 * class and no `gb`, `fk` still the generated row; init again, then a
 * load, gives them once more, and a second load takes the same bytes. */
static void test_rebuild_and_shutdown(void) {
    size_t k;

    pico8_research_build_gb();
    test_gb();

    pico8_research_shutdown();
    assert(pico8_gb_count() == 0 && pico8_gb_at(1) == NULL && pico8_gb_find("colony") == NULL);
    assert(!pico8_hp_ready());
    assert(pico8_hp_fk() == &research_project_row_tail_row);
    pico8_research_shutdown();

    pico8_research_init();
    assert(pico8_hp_ready() && pico8_hp_fk() == &research_project_row_tail_row);
    pico8_cart_load();
    test_gb();
    k = load_zone_used();
    pico8_cart_load();
    assert(load_zone_used() == k);
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

/* As must_abort, and the abort is the assert whose text holds `needle`:
 * past it, the construction reads out of bounds and may abort elsewhere. */
static void must_abort_saying(void (*write)(void *), void *e, const char *needle) {
    char said[512];
    ssize_t n;
    size_t got = 0;
    int fds[2];
    pid_t child;
    int status;

    fflush(stdout);
    assert(pipe(fds) == 0);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        close(fds[0]);
        assert(dup2(fds[1], 2) == 2);
        write(e);
        _exit(0);
    }
    close(fds[1]);
    while (got < sizeof said - 1 && (n = read(fds[0], said + got, sizeof said - 1 - got)) > 0) {
        got += (size_t)n;
    }
    said[got] = '\0';
    close(fds[0]);
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
    assert(strstr(said, needle) != NULL);
}

static void build_with_header(void *e) {
    project_records_entry_t header = {0};
    pico8_hp_keys_t keys = {0};

    (void)e;
    header.header = "x";
    keys.ri = &header;
    (void)pico8_hp_new(&keys);
}

static void build_with_o_missing_from_r(void *e) {
    project_records_entry_t ri = {0};
    pico8_hp_keys_t keys = {0};

    (void)e;
    ri.npos = 6;
    ri.name = "x";
    ri.planet_codes = "y";
    ri.base_cost = 1;
    ri.production_key = "z";
    ri.inputs = "";
    ri.output = "not a key of r";
    keys.ri = &ri;
    (void)pico8_hp_new(&keys);
}

static void build_with_ii_and_no_px(void *e) {
    project_records_entry_t ri = {0};
    pico8_hp_keys_t keys = {0};

    (void)e;
    ri.has_icon_sprite = 1;
    ri.icon_sprite = 3;
    keys.ri = &ri;
    (void)pico8_hp_new(&keys);
}

static void build_with_nine_positions(void *e) {
    project_records_entry_t ri = {0};
    pico8_hp_keys_t keys = {0};

    (void)e;
    ri.npos = 9;
    ri.name = "x";
    keys.ri = &ri;
    (void)pico8_hp_new(&keys);
}

static void build_with_no_px_string(void *e) {
    project_records_entry_t ri = {0};
    pico8_hp_keys_t keys = {0};

    (void)e;
    ri.npos = 1;
    keys.ri = &ri;
    (void)pico8_hp_new(&keys);
}

static void build_with_no_o_number(void *e) {
    project_records_entry_t ri = {0};
    pico8_hp_keys_t keys = {0};

    (void)e;
    ri.npos = 6;
    ri.name = "x";
    ri.planet_codes = "y";
    ri.base_cost = 1;
    ri.production_key = "z";
    ri.inputs = "";
    keys.ri = &ri;
    (void)pico8_hp_new(&keys);
}

static void od_on_other(void *e) {
    (void)pico8_hp_od(e);
}

static void po_on_other(void *e) {
    (void)pico8_hp_po(e);
}

static void jb_on_other(void *e) {
    (void)pico8_hp_jb(e);
}

static void lq_on_other(void *e) {
    (void)pico8_hp_lq(e);
}

static void lz_on_other(void *e) {
    planet_install_project((planet_t *)entities_by_tag("planet")->items[0], e);
}

static void hu_on_other(void *e) {
    planet_apply_project((planet_t *)entities_by_tag("planet")->items[0], e);
}

/* A project is a struct whose descriptor has no method and no writer, its
 * `fk` the class's; a header entry, a value `gp` maps that `r`
 * lacks, `ii` without `px`, more than eight positions, or a position of the
 * wrong type stops its construction, and its functions
 * refuse anything that is not a project. With no key, only `fk`, an empty `i` and the `lq`
 * method. */
static void test_hp_is_a_struct(void) {
    const entity_class_t *cls = pico8_hp_struct_class();
    pico8_hp_t *b = project(1);
    pico8_hp_t *bare;
    int j;

    assert(b != NULL && b->header.class == cls);
    assert(cls->draw == NULL && cls->draw_overlay == NULL && cls->hit_test == NULL && cls->new_year == NULL && cls->refresh == NULL &&
           cls->has_refresh == NULL && cls->left_press == NULL && cls->right_press == NULL && cls->accept_drop == NULL);
    assert(cls->drag == NULL && cls->drop == NULL && cls->attach_slipway == NULL && cls->offer_trade == NULL && cls->draw_at == NULL &&
           cls->spawn_particle == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(pico8_hp_ready());
    assert(b->def->row_tail != NULL && b->def->row_tail == pico8_hp_fk());

    bare = pico8_hp_new(NULL);
    assert(bare != NULL && bare->header.class == cls);
    assert(bare->def == &research_project_archetype && bare->def == b->def);
    assert(bare->i != NULL && bare->i->count == 0);
    assert(bare->lq == NULL);
    assert(bare->px == NULL && bare->lr == NULL && !bare->has_fj && bare->ja == NULL && !bare->has_o);
    assert(!bare->has_il && !bare->has_dm && bare->rr == NULL && !bare->has_ii && bare->cw == NULL);
    assert(bare->go == NULL && !bare->has_rn);

    start_game();
    must_abort(build_with_header, NULL);
    must_abort(build_with_o_missing_from_r, NULL);
    must_abort(build_with_ii_and_no_px, NULL);
    must_abort_saying(build_with_nine_positions, NULL, "hp: fd has eight positions");
    must_abort(build_with_no_px_string, NULL);
    must_abort(build_with_no_o_number, NULL);
    must_abort(od_on_other, pico8_ok());
    must_abort(po_on_other, pico8_ok());
    must_abort(jb_on_other, pico8_ok());
    must_abort(lq_on_other, pico8_ok());
    must_abort(lz_on_other, pico8_ok());
    must_abort(hu_on_other, pico8_ok());
}

/* `gb[px]` is the last project `hp:pq` registered under that `px`, even one
 * `build_gb` did not make; `gb[k]` keeps its own. */
static void test_gb_px_is_the_last_registered(void) {
    pico8_hp_t *first = project(1);
    pico8_hp_keys_t keys = {0};
    pico8_hp_t *again;
    int i;

    for (i = 0; keys.ri == NULL; i++) {
        assert(i < PROJECT_RECORDS_COUNT);
        if (project_records_table[i].header == NULL && strcmp(project_records_table[i].name, first->px) == 0) {
            keys.ri = &project_records_table[i];
        }
    }
    again = pico8_hp_new(&keys);
    assert(again != first && pico8_gb_find(first->px) == again);
    assert(pico8_gb_at(1) == first && pico8_gb_count() == GB_COUNT);
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
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_gb);
    in_fresh_child(test_functions);
    in_fresh_child(test_install);
    in_fresh_child(test_wants);
    in_fresh_child(test_built_world);
    in_fresh_child(test_harvested_world);
    in_fresh_child(test_hp_is_a_struct);
    in_fresh_child(test_gb_px_is_the_last_registered);
    in_fresh_child(test_rebuild_and_shutdown);
    in_fresh_child(test_gb_in_chargement);
    in_fresh_child(test_jy_new_in_the_zone_passed);

    printf("test_pico8_hp: all tests passed\n");
    return 0;
}
