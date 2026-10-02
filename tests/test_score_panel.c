/* The score panel `pj`, built by its only road (`eu()`, `pico8_pj_new()`,
 * on a game started directly: no events file), read on its members and on
 * the class's shared `ou`/`ea`/`cr`, its defaults read on its archetype, and
 * driven through the real `mv`/`cr()`/`de.my`. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "p8/assets.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "game/economy_score.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/layout.h"
#include "p8/mem.h"
#include "game/menu.h"
#include "game/message_box.h"
#include "game/score_panel.h"
#include "game/score_panel_display.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "engine/vec.h"

#define BUILT_HASH 0x9BE763EBu
#define SHARED_HASH 0xAC16627Bu
#define LIFE_HASH 0xDDBCC765u
#define CR_HASH 0x7D1C5B1Fu

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const char *const tags[] = {"planet", "probe", "structure", "slipway", "wormhole", "message_box", "node", "ascension", "targeting", "slipgate", "background"};

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

static int ents_rank_in(const pico8_ents_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int ents_listed(const pico8_ents_t *list, const void *e) {
    return ents_rank_in(list, e) >= 0;
}

static int ents_count_of(const pico8_ents_t *list) {
    return list == NULL ? 0 : list->count;
}

/* 0 absent, 1 false, 2 true. */
static uint32_t c__state(const pico8_pj_t *pj) {
    if (!pj->has_c_) {
        return 0;
    }
    return pj->c_ ? 2 : 1;
}

/* `ed` NULL reads as the class's `ed`, the number 1. */
static p8_fixed_t ed_fixed(const pico8_pj_t *pj) {
    assert(pj->ed == NULL);
    return p8_fixed_from_int(1);
}

static int is_lq(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_lq_struct_class();
}

static int row_count(const void *lq) {
    return ((const pico8_lq_t *)lq)->items_count;
}

static pico8_lq_cell_t row_item(const void *lq, int i) {
    assert(i >= 0 && i < row_count(lq));
    return ((const pico8_lq_t *)lq)->items[i];
}

/* The words the frozen hashes fold before a value: 0xF1 a number, 0xB0 a
 * boolean (then 0 or 1), 0x57 a string; any other moves every *_HASH. */
static uint32_t fold_fixed(uint32_t h, p8_fixed_t x) {
    return fnv(fnv(h, 0xF1u), (uint32_t)x);
}

static uint32_t fold_bool(uint32_t h, int b) {
    return fnv(fnv(h, 0xB0u), b ? 1u : 0u);
}

static uint32_t fold_string(uint32_t h, const char *s) {
    return fnv_str(fnv(h, 0x57u), s);
}

static uint32_t fold_ref(uint32_t h, const void *ref);
static uint32_t fold_row(uint32_t h, const pico8_jb_row_t *row);

static uint32_t fold_item(uint32_t h, const pico8_jb_item_t *item) {
    if (item->s != NULL) {
        return fold_string(h, item->s);
    }
    if (item->is_false) {
        return fold_bool(h, 0);
    }
    if (item->is_ref) {
        return fold_ref(h, item->ref);
    }
    return fold_fixed(h, item->is_fixed ? item->fixed : p8_fixed_from_int(item->n));
}

/* A row's items (a nested row folded the same way), then `c`, then `fn`;
 * `extra` more entries follow, the caller's. */
static uint32_t fold_row_entries(uint32_t h, const pico8_jb_row_t *row) {
    int i;

    for (i = 0; i < row->count; i++) {
        h = fnv(h, (uint32_t)(i + 1));
        h = row->items[i].row != NULL ? fold_row(fnv(h, 0x7Au), row->items[i].row)
                                      : fold_item(h, &row->items[i]);
    }
    if (row->has_c) {
        h = fold_fixed(fnv_str(h, "c"), p8_fixed_from_int(row->c));
    }
    if (row->fn != NULL) {
        h = fold_string(fnv_str(h, "fn"), row->fn);
    }
    return h;
}

static int row_entries(const pico8_jb_row_t *row) {
    return row->count + (row->has_c != 0) + (row->fn != NULL);
}

static uint32_t fold_row(uint32_t h, const pico8_jb_row_t *row) {
    return fnv(fold_row_entries(h, row), (uint32_t)row_entries(row));
}

/* `ea`: its positionals, then `lh`. */
static uint32_t fold_ea(uint32_t h, const pico8_pj_ea_t *ea) {
    h = fold_row_entries(h, &ea->row);
    if (ea->lh != NULL) {
        h = fnv(fnv_str(h, "lh"), 0xFCu);
    }
    return fnv(h, (uint32_t)(row_entries(&ea->row) + (ea->lh != NULL)));
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (ents_listed(pico8_hv(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)ents_rank_in(pico8_hv(), ref));
    }
    if (ref == (const void *)&pico8_pj_gs_row || ref == (const void *)&pico8_pj_i__row) {
        return fold_row(fnv(h, 0x7Au), ref);
    }
    if (ref == (const void *)pico8_pj_ou()) {
        return fold_row(fnv(h, 0x7Au), &pico8_pj_ou()->row);
    }
    if (ref == (const void *)pico8_pj_ea()) {
        return fold_ea(fnv(h, 0x7Au), ref);
    }
    return fnv(fnv(h, 0xEEEEu), 0u);
}

/* A present member: bit 1, then the value as the frozen hashes fold it.
 * The 1 of a member the struct always holds, and the 0xF1 of a fixed, are
 * constant by the struct's shape, kept so that the hashes still prove the
 * typed reads. */
static uint32_t fold_fixed_member(uint32_t h, p8_fixed_t f) {
    return fnv(fnv(fnv(h, 1u), 0xF1u), (uint32_t)f);
}

static uint32_t fold_opt_fixed(uint32_t h, int has, p8_fixed_t f) {
    return has ? fold_fixed_member(h, f) : fnv(h, 0u);
}

/* A NULL reference folds as an absent member. */
static uint32_t fold_ref_member(uint32_t h, const void *ref) {
    return ref != NULL ? fold_ref(fnv(h, 1u), ref) : fnv(h, 0u);
}

static uint32_t fold_ed(uint32_t h, const pico8_pj_t *pj) {
    assert(pj->ed == NULL && "no test folds a pj holding an ed: to fold one, fold 1 then its rank in hv (0x4E, rank) as fold_ref does, or outside hv 0xEEEE then the frozen struct tag 0x9E5C57C7");
    return fold_fixed_member(h, ed_fixed(pj));
}

/* `cr` as the spec table it was: its entries in spec order, then their
 * count; the 0xF1 of `ca` and the 0x57 of `ep` are constant by the
 * struct's shape. */
static uint32_t fold_cr(uint32_t h, const pico8_pj_cr_t *cr) {
    int count = 0;

    assert(cr != NULL);
    h = fnv(fnv(h, 1u), 0x7Au);
    if (cr->hi != NULL) {
        h = fold_ref(fnv_str(h, "hi"), cr->hi);
        count++;
    }
    if (cr->gq != NULL) {
        h = fold_ref(fnv_str(h, "gq"), cr->gq);
        count++;
    }
    h = fnv(fnv(fnv_str(h, "ca"), 0xF1u), (uint32_t)cr->ca);
    count++;
    if (cr->ep != NULL) {
        h = fnv_str(fnv(fnv_str(h, "ep"), 0x57u), cr->ep);
        count++;
    }
    return fnv(h, (uint32_t)count);
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
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

static const entity_class_t *cls_of(const void *e) {
    return ((const entity_t *)e)->class;
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static pico8_pj_t *start_game(void) {
    p8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_cart_vider_file_sortie();
    pico8_eu(NULL);
    pico8_cart_vider_file_sortie();
    assert(pico8_ju() != NULL);
    return pico8_ju();
}

static const pico8_jb_row_t *row_of(const pico8_jb_row_t *t, int i) {
    assert(i <= t->count && t->items[i - 1].row != NULL);
    return t->items[i - 1].row;
}

static const char *string_at(const pico8_jb_row_t *t, int i) {
    assert(i <= t->count && t->items[i - 1].s != NULL);
    return t->items[i - 1].s;
}

/* The 7 `ou[i][1]` as `hb` wrote them. */
static uint32_t fold_ou_heads(uint32_t h) {
    const pico8_pj_ou_t *ou = pico8_pj_ou();
    int i;

    assert(ou != NULL);
    for (i = 1; i <= 7; i++) {
        h = fold_item(h, &row_of(&ou->row, i)->items[0]);
    }
    return h;
}

static int is_of(const void *e, const entity_class_t *cls) {
    return e != NULL && ((const entity_t *)e)->class == cls;
}

static int fq_count(void) {
    const pico8_ents_t *hv = pico8_hv();
    int n = 0;
    int i;
    for (i = 0; i < hv->count; i++) {
        n += is_of(hv->items[i], pico8_fq_struct_class());
    }
    return n;
}

static int js15_count(void) {
    const pico8_ents_t *hv = pico8_hv();
    int n = 0;
    int i;
    for (i = 0; i < hv->count; i++) {
        const void *e = hv->items[i];
        n += is_of(e, pico8_js_struct_class()) && ((const pico8_js_t *)e)->fa == p8_fixed_from_int(15);
    }
    return n;
}

static int de_my(void) {
    return pico8_de_my()(NULL);
}

/* `pj:ph`'s second button, the one whose `my` is `de`'s. */
static pico8_js_t *de_button(void) {
    const pico8_ents_t *hv = pico8_hv();
    int i;
    for (i = hv->count - 1; i >= 0; i--) {
        pico8_js_t *e = (pico8_js_t *)hv->items[i];
        if (is_of(e, pico8_js_struct_class()) && e->fa == p8_fixed_from_int(15) &&
            e->my != pico8_init_field_value()) {
            return e;
        }
    }
    return NULL;
}

/* The game's panel as `eu()` and `lt` left it: where it registered, its
 * defaults, the names only read elsewhere, its 4 methods, and the 5 tables
 * of the class, by identity and content. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    pico8_pj_t *ju = start_game();
    size_t i;

    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), ju));
    h = fnv(h, (uint32_t)pico8_hv()->count);
    assert(ents_listed(pico8_cv("draw"), ju) && ents_listed(pico8_cv("new_year"), ju) && ents_listed(pico8_cv("refresh"), ju));
    assert(!ents_listed(pico8_cv("draw_overlay"), ju) && !ents_listed(pico8_cv("hit_test"), ju));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cv("draw"), ju));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cv("new_year"), ju));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cv("refresh"), ju));
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        assert(!ents_listed(pico8_cg(tags[i]), ju));
    }

    assert(ju->fa == p8_fixed_from_int(10));
    assert(ju->def == &pico8_pj_def && ju->fa == ju->def->fa);
    assert(ju->ed == NULL);
    assert(ju->header.state != NULL && strcmp(ju->header.state, "normal") == 0);
    h = fold_fixed_member(h, ju->header.state_time);
    assert(!ju->has_c_ && !ju->has_gz);
    assert(cls_of(ju)->draw != NULL && cls_of(ju)->states[ENTITY_STATE_NORMAL] != NULL && cls_of(ju)->refresh != NULL &&
           cls_of(ju)->new_year != NULL);
    assert(cls_of(ju)->hit_test == NULL && cls_of(ju)->draw_overlay == NULL);
    h = fold_ref_member(h, pico8_pj_ea());
    assert(ju->def->gs == &pico8_pj_gs_row);
    h = fold_ref_member(h, ju->def->gs);
    h = fold_cr(h, pico8_pj_cr());
    assert(ju->def->i_ == &pico8_pj_i__row);
    h = fold_ref_member(h, ju->def->i_);
    h = fold_ref_member(h, pico8_pj_ou());
    check("built", h, BUILT_HASH);
}

/* A second panel shares the class's `ou`/`ea`/`cr`/`gs`/`i_` and sees what
 * `hb`/`hk` wrote into them through the first; `fa`/`ed`/`c_`/`gz` stay
 * each panel's own. */
static void test_shared(void) {
    uint32_t h = 2166136261u;
    pico8_pj_t *ju = start_game();
    pico8_pj_t *other = pico8_pj_new();
    const pico8_pj_ea_t *ea;

    assert(other != NULL && other != ju);
    assert(other->def == ju->def);
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), other));

    cls_of(ju)->refresh(&ju->header);
    h = fold_ou_heads(h);

    cls_of(ju)->new_year(&ju->header, p8_fixed_from_int(3425));
    ea = pico8_pj_ea();
    assert(strcmp(string_at(row_of(&ea->row, 2), 1), "year 3425") == 0);
    assert(ea->row.count == 3 && ea->lh != NULL);
    h = fold_ref_member(h, pico8_pj_ea());

    pico8_fm_hy()->hl = "final score";
    pico8_fm_hy()->has_hl = 1;
    cls_of(ju)->states[ENTITY_STATE_NORMAL](&ju->header, p8_fixed_from_int(0));
    assert(ju->fa == p8_fixed_from_int(13));
    assert(c__state(ju) == 2);
    assert(other->fa == p8_fixed_from_int(10));
    assert(ed_fixed(other) == p8_fixed_from_int(1));
    assert(!other->has_c_ && !other->has_gz);
    h = fold_fixed_member(h, other->fa);
    h = fold_ed(h, other);
    h = fold_opt_fixed(h, ju->has_gz, ju->gz);
    check("shared", h, SHARED_HASH);
}

/* `hb`; `hk` on a year with a message and two without; `cr()`; `kj` closed;
 * the game's end through `ph`, twice; `kj` open; `de.my` twice. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    pico8_pj_t *ju = start_game();
    const pico8_pj_ea_t *ea = pico8_pj_ea();
    const pico8_pj_cr_t *cr = pico8_pj_cr();
    static const int years[] = {3425, 3400, 3401};
    int fq_before;
    int js_before;
    int boxes;
    size_t i;

    cls_of(ju)->refresh(&ju->header);
    h = fold_ou_heads(h);

    for (i = 0; i < sizeof years / sizeof years[0]; i++) {
        cls_of(ju)->new_year(&ju->header, p8_fixed_from_int(years[i]));
        h = fnv(h, (uint32_t)(ea->row.count >= 3));
        h = fnv(h, (uint32_t)(ea->lh != NULL));
        h = fnv_str(h, string_at(row_of(&ea->row, 2), 1));
        h = fnv(h, (uint32_t)ents_count_of(pico8_cg("message_box")));
        h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), pico8_kc()));
    }
    assert(ea->row.count == 2 && ea->lh != NULL);

    pico8_cr();
    assert(cr == pico8_pj_cr());
    assert(cr->ep != NULL);
    h = fnv_str(h, cr->ep);

    for (i = 0; i < 2; i++) {
        cls_of(ju)->draw(&ju->header, NULL);
        h = fnv(h, screen_hash());
    }
    assert(!ju->has_gz);

    fq_before = fq_count();
    js_before = js15_count();
    pico8_fm_hy()->hl = "final score";
    pico8_fm_hy()->has_hl = 1;
    cls_of(ju)->states[ENTITY_STATE_NORMAL](&ju->header, p8_fixed_from_int(0));
    assert(c__state(ju) == 2);
    assert(ju->has_gz && ju->gz == 0);
    assert(ju->fa == p8_fixed_from_int(13));
    assert(fq_count() == fq_before + 1 && js15_count() == js_before + 2);
    assert(ju->ed == pico8_kc() && pico8_kc() != NULL);
    boxes = ents_count_of(pico8_cg("message_box"));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), pico8_kc()));
    h = fnv(h, (uint32_t)pico8_hv()->count);

    cls_of(ju)->states[ENTITY_STATE_NORMAL](&ju->header, p8_fixed_from_int(0));
    assert(ents_count_of(pico8_cg("message_box")) == boxes);
    assert(ju->ed == pico8_kc());

    for (i = 0; i < 3; i++) {
        cls_of(ju)->draw(&ju->header, NULL);
        h = fold_opt_fixed(h, ju->has_gz, ju->gz);
        h = fnv(h, screen_hash());
    }
    assert(ju->has_gz && ju->gz == P8_FIXED_LIT(0x00009999, 0.6));

    assert(de_my() == 1);
    assert(c__state(ju) == 1);
    assert(de_my() == 1);
    assert(c__state(ju) == 2);
    {
        pico8_js_t *de = de_button();
        assert(de != NULL && de->has_my && de->my != NULL);
        de->my(de);
        assert(c__state(ju) == 1);
        de->my(de);
        assert(c__state(ju) == 2);
    }
    h = fold_fixed_member(h, ju->fa);
    check("life", h, LIFE_HASH);
}

static const pico8_js_t *as_js(const void *e) {
    assert(is_of(e, pico8_js_struct_class()));
    return e;
}

static pico8_vec_t vec_of(const pico8_vec_t *v) {
    assert(v != NULL);
    return *v;
}

/* A positional as the value it was sorted from folded: a row as any REF,
 * NOTHING as false (no row folded here holds a nil at the top level). */
static uint32_t fold_cell(uint32_t h, pico8_lq_cell_t c) {
    switch (c.kind) {
    case PICO8_LQ_CELL_ROW:
        return fnv(h, 0x12u);
    case PICO8_LQ_CELL_FIXED:
        return fold_fixed(h, c.as.fixed);
    case PICO8_LQ_CELL_STRING:
        return fold_string(h, c.as.s);
    default:
        return fold_bool(h, c.kind == PICO8_LQ_CELL_TRUE);
    }
}

/* A row's `lq` as `js:pq` built it: its `w`, its `c`, its positional items. */
static uint32_t fold_lq(uint32_t h, const pico8_lq_t *lq) {
    int i;

    assert(is_lq(lq));
    h = fold_fixed_member(h, lq->w);
    h = fold_opt_fixed(h, lq->has_c, lq->c);
    for (i = 0; i < row_count(lq); i++) {
        h = fnv(h, (uint32_t)(i + 1));
        h = fold_cell(h, row_item(lq, i));
    }
    return h;
}

/* What `pz(pj.cr, bs)` left in `ok.le`: the `ep` head row, then one row per
 * tech, all at `fa` = `ca`, each with the `ra` = `hi`, `qf` stepped by `gq`. */
static uint32_t fold_cr_le(uint32_t h, const char **head) {
    const pico8_ents_t *le;
    const pico8_lq_t *lq;
    const pico8_vec_t *ra;
    int i;

    assert(pico8_ok() != NULL && pico8_ok()->le != NULL);
    le = pico8_ok()->le;
    assert(le->count >= 2);
    h = fnv(h, (uint32_t)le->count);
    lq = as_js(le->items[0])->lq;
    assert(lq != NULL && lq->has_c && lq->c == p8_fixed_from_int(13));
    assert(row_count(lq) > 0 && row_item(lq, 0).kind == PICO8_LQ_CELL_STRING);
    *head = row_item(lq, 0).as.s;
    h = fnv_str(h, *head);
    ra = as_js(le->items[0])->ra;
    for (i = 0; i < le->count; i++) {
        const pico8_js_t *b = as_js(le->items[i]);
        pico8_vec_t qf = vec_of(b->qf);
        assert(b->ra != NULL && b->ra->x == ra->x && b->ra->y == ra->y);
        assert(b->fa == p8_fixed_from_int(10));
        assert(qf.x == 0 && qf.y == p8_fixed_from_int(11 * (i + 1)));
        h = fold_fixed_member(h, b->fa);
        h = fnv(fnv(h, (uint32_t)qf.x), (uint32_t)qf.y);
        assert(b->lq != NULL);
        h = fold_lq(h, b->lq);
    }
    assert(vec_of(ra).x == 0 && vec_of(ra).y == p8_fixed_from_int(11));
    return h;
}

/* `cr()` twice on one game, then on the next game started in the same
 * process: one shared `pj.cr`, `ep` absent until the first `cr()` and kept
 * across games, the same head row each time. */
static void test_cr(void) {
    uint32_t h = 2166136261u;
    const pico8_pj_cr_t *cr;
    const char *head1;
    const char *head2;

    (void)start_game();
    cr = pico8_pj_cr();
    assert(cr != NULL && cr->ep == NULL);
    pico8_cr();
    h = fold_cr_le(h, &head1);
    assert(cr->ep != NULL && strcmp(cr->ep, head1) == 0);
    pico8_cr();
    h = fold_cr_le(h, &head2);
    assert(strcmp(head1, head2) == 0);
    assert(pico8_pj_new() != NULL && pico8_pj_cr() == cr);

    (void)start_game();
    assert(pico8_pj_cr() == cr);
    assert(cr->ep != NULL && strcmp(cr->ep, head2) == 0);
    pico8_cr();
    h = fold_cr_le(h, &head1);
    check("cr", h, CR_HASH);
}

static int pico8_pj_ea_lh_witness(pico8_mh_t *self) {
    (void)self;
    return 1;
}

/* Right after the init, before any `hb`/`hk`, `ou` and `ea` hold what
 * their generated rows hold, and are copies of them; each init resets what
 * `hb`/`hk` wrote. */
static void test_ou_ea_start_as_their_rows(void) {
    pico8_pj_ou_t *ou;

    pico8_score_panel_init();
    ou = pico8_pj_ou();
    assert(ou != NULL && pico8_pj_ea() != NULL);
    assert(&ou->row != &pico8_pj_ou_row && &pico8_pj_ea()->row != &pico8_pj_ea_row);
    assert(fold_row(0u, &ou->row) == fold_row(0u, &pico8_pj_ou_row));
    assert(fold_row(0u, &pico8_pj_ea()->row) == fold_row(0u, &pico8_pj_ea_row) && pico8_pj_ea()->lh == NULL);
    ou->sub_items[0][0] = (pico8_jb_item_t){.s = "written"};
    pico8_pj_ea()->second_items[0] = (pico8_jb_item_t){.s = "year 3425"};
    pico8_pj_ea()->items[2] = (pico8_jb_item_t){.row = &pico8_pj_gs_row};
    pico8_pj_ea()->row.count = 3;
    pico8_pj_ea()->lh = pico8_pj_ea_lh_witness;
    assert(fold_row(0u, &ou->row) != fold_row(0u, &pico8_pj_ou_row));
    assert(fold_row(0u, &pico8_pj_ea()->row) != fold_row(0u, &pico8_pj_ea_row));
    pico8_score_panel_init();
    assert(pico8_pj_ou() == ou);
    assert(fold_row(0u, &pico8_pj_ou()->row) == fold_row(0u, &pico8_pj_ou_row));
    assert(fold_row(0u, &pico8_pj_ea()->row) == fold_row(0u, &pico8_pj_ea_row) && pico8_pj_ea()->lh == NULL);
    pico8_score_panel_shutdown();
    assert(pico8_pj_ou() == NULL && pico8_pj_ea() == NULL);
}

/* The game's panel is a struct whose descriptor holds exactly `hb`, `kj`,
 * `hk`, `ph`; it takes no construction key. `ea`, `cr`, `ou`
 * are the class's shared state; `gs`, `i_` are the generated rows. */
static void test_pj_is_a_struct(void) {
    const entity_class_t *cls = pico8_pj_struct_class();
    pico8_pj_t *ju = start_game();
    int j;

    assert(is_of(ju, cls));
    assert(cls->drag == NULL && cls->drop == NULL && cls->attach_slipway == NULL && cls->offer_trade == NULL && cls->draw_at == NULL &&
           cls->spawn_particle == NULL);
    assert(cls->refresh != NULL && cls->draw != NULL && cls->new_year != NULL && cls->states[ENTITY_STATE_NORMAL] != NULL);
    assert(cls->draw_overlay == NULL && cls->hit_test == NULL && cls->has_refresh == NULL && cls->left_press == NULL && cls->right_press == NULL &&
           cls->accept_drop == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
        assert((cls->states[j] != NULL) == (j == ENTITY_STATE_NORMAL));
    }
    assert(pico8_pj_ready());
    assert(pico8_pj_ou() != NULL && pico8_pj_ea() != NULL && pico8_pj_cr() != NULL);
    assert(pico8_pj_i_() == &pico8_pj_i__row && pico8_pj_gs() == &pico8_pj_gs_row);
    assert(ju->def->gs == &pico8_pj_gs_row && ju->def->i_ == &pico8_pj_i__row);
}

/* `pj.cr` is one static struct without method, holding `hi`,
 * `gq`, `ca` and, once `cr()` ran, `ep`; the init resets `ep`, the shutdown
 * hides it. */
static void test_cr_is_a_struct(void) {
    const entity_class_t *cls = pico8_pj_cr_struct_class();
    pico8_pj_cr_t *cr;
    int j;

    (void)start_game();
    cr = pico8_pj_cr();
    assert(cr != NULL && is_of(cr, cls));
    assert(cls->draw == NULL && cls->draw_overlay == NULL && cls->hit_test == NULL && cls->new_year == NULL && cls->refresh == NULL &&
           cls->left_press == NULL && cls->right_press == NULL && cls->accept_drop == NULL);
    assert(cls->drag == NULL && cls->drop == NULL && cls->attach_slipway == NULL && cls->offer_trade == NULL && cls->draw_at == NULL &&
           cls->spawn_particle == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(cr->hi != NULL);
    assert(cr->gq != NULL && cr->gq != cr->hi);
    assert(cr->ca == p8_fixed_from_int(10));
    assert(cr->ep == NULL);

    pico8_cr();
    assert(cr->ep != NULL);
    assert(pico8_pj_new() != NULL && pico8_pj_cr() == cr);

    pico8_score_panel_init();
    assert(pico8_pj_cr() == cr && cr->ep == NULL);
    pico8_score_panel_shutdown();
    assert(pico8_pj_cr() == NULL);
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

/* `ky()` on a started game with ten techs invented and a happiness of 1:
 * no planet scores yet, so `s[5]` is the techs' part and `s[6]` its tier. */
static void test_ky_reads_invented_and_happiness(void) {
    pico8_hy_t *fm;
    pico8_ky_t s;
    int i;

    (void)start_game();
    fm = pico8_fm_hy();
    for (i = 0; i < 10; i++) {
        pico8_ptrs_add(fm->invented, "a tech");
    }
    fm->happiness = p8_fixed_from_int(1);
    fm->has_happiness = 1;
    s = pico8_ky();
    assert(s.di == 0 && s.gl == 0 && s.el == p8_fixed_from_int(1430));
    assert(strcmp(s.happiness, "1%") == 0);
    assert(s.total == p8_fixed_from_int(14) && s.tier == 0);
}

/* Item `i` (1-based) of the line that `ph` put in the box's row `k`
 * (0-based), an article `ld`. */
static pico8_lq_cell_t ph_row_item(const pico8_mh_t *mh, int k, int i) {
    const pico8_jb_row_t *ld;
    const pico8_lq_t *lq;

    assert(k < mh->items_count);
    ld = mh->items[k];
    assert(ld->fn != NULL && strcmp(ld->fn, "ld") == 0 && ld->items[0].is_ref);
    lq = ld->items[0].ref;
    assert(i <= lq->items_count);
    return lq->items[i - 1];
}

/* `ph` shows `ky()`: the score-5 row (box row 10) carries `s[5]`, the total,
 * and the tier row (11) lights `s[6]` of its five icons. */
static void test_ph_rows_show_ky(void) {
    void *ju = start_game();
    pico8_hy_t *fm = pico8_fm_hy();
    const pico8_mh_t *mh;
    pico8_lq_cell_t total;
    pico8_lq_cell_t icon;
    int i;

    for (i = 0; i < 10; i++) {
        pico8_ptrs_add(fm->invented, "a tech");
    }
    fm->happiness = p8_fixed_from_int(1);
    fm->has_happiness = 1;
    fm->hl = "final score";
    fm->has_hl = 1;
    cls_of(ju)->states[ENTITY_STATE_NORMAL](ju, p8_fixed_from_int(0));
    mh = pico8_kc();
    assert(mh != NULL && mh->items_count == PICO8_BZ_COUNT);
    total = ph_row_item(mh, 10, 3);
    assert(total.kind == PICO8_LQ_CELL_STRING && strcmp(total.as.s, "14") == 0);
    for (i = 1; i <= 5; i++) {
        icon = ph_row_item(mh, 11, i);
        assert(icon.kind == PICO8_LQ_CELL_FIXED && icon.as.fixed == p8_fixed_from_int(57));
    }
}

/* `eg` on a started game: no planet is `ly` yet, so half their count is 0
 * and every pattern of `fb` gets its flag. */
static void test_eg_flags_every_fb_pattern_without_ly(void) {
    int i;

    (void)start_game();
    assert(pico8_cg("planet")->count >= 2);
    pico8_eg();
    for (i = 0; i < PICO8_FB_COUNT; i++) {
        assert(p8_peek(&mem, 0x3101 + 4 * pico8_fb_table[i]) >= 128);
    }
}

int main(void) {
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_built);
    in_fresh_child(test_shared);
    in_fresh_child(test_life);
    in_fresh_child(test_cr);
    in_fresh_child(test_ou_ea_start_as_their_rows);
    in_fresh_child(test_pj_is_a_struct);
    in_fresh_child(test_cr_is_a_struct);
    in_fresh_child(test_ky_reads_invented_and_happiness);
    in_fresh_child(test_ph_rows_show_ky);
    in_fresh_child(test_eg_flags_every_fb_pattern_without_ly);

    printf("test_pico8_pj: all tests passed\n");
    return 0;
}
