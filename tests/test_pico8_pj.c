/* The score panel `pj`, built by its only road (`eu()`, `pico8_pj_new()`,
 * on a game started directly: no events file), read only through the
 * dual-form field API, its defaults compared to `pico8_pj_start`, and
 * driven through the real `mv`/`cr()`/`de.my`. */

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
#include "../src/pico8_economy.h"
#include "../src/pico8_economy_score.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_game_data.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_menu.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_score_panel_display.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"


#define BUILT_HASH 0x9BE763EBu
#define SHARED_HASH 0xAC16627Bu
#define LIFE_HASH 0xDDBCC765u
#define CR_HASH 0x7D1C5B1Fu
#define PJ_MEMBERS_HASH 0xD786BEE2u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

/* 4 of the 5 tables of pj's spec, read on the class by the cart; the 5th,
 * `cr`, is its own struct. */
static const char *const tables[] = {"ea", "gs", "i_", "ou"};
static const char *const absent[] = {"c_", "gz", "ra", "p_", "nu", "fx", "hd"};
static const char *const tags[] = {"jo", "g_", "qh", "jc", "mz", "mh", "np", "hj", "prober", "nr", "bg"};

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

static int count_of(const pico8_list_t *list) {
    return list == NULL ? 0 : list->count;
}

/* The instance reader keeps a field written nil (TROU-9), the struct reader
 * reads a nil member as absent: nil and absent are one state here. */
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

static pico8_fixed_t get_fixed(const void *e, const char *name) {
    pico8_value_t v;
    assert(lookup(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

/* 0 absent, 1 false, 2 true, 3 anything else. */
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

static int is_lq(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_lq_struct_class();
}

static int row_count(const void *lq) {
    return ((const pico8_lq_t *)lq)->items.count;
}

static pico8_value_t row_item(const void *lq, int i) {
    assert(i >= 0 && i < row_count(lq));
    return ((const pico8_lq_t *)lq)->items.items[i];
}

static uint32_t fold_value(uint32_t h, pico8_value_t v);
static uint32_t fold_row(uint32_t h, const pico8_jb_row_t *row);

static pico8_value_t item_value(const pico8_jb_item_t *item) {
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

/* A row's items (a nested row folded the same way), then `c`, then `fn`;
 * `extra` more entries follow, the caller's. */
static uint32_t fold_row_entries(uint32_t h, const pico8_jb_row_t *row) {
    int i;

    for (i = 0; i < row->count; i++) {
        h = fnv(h, (uint32_t)(i + 1));
        h = row->items[i].row != NULL ? fold_row(fnv(h, 0x7Au), row->items[i].row)
                                      : fold_value(h, item_value(&row->items[i]));
    }
    if (row->has_c) {
        h = fold_value(fnv_str(h, "c"), pico8_value_fixed(pico8_fixed_from_int(row->c)));
    }
    if (row->fn != NULL) {
        h = fold_value(fnv_str(h, "fn"), pico8_value_string(row->fn));
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
        h = fold_value(fnv_str(h, "lh"), pico8_value_function(ea->lh));
    }
    return fnv(h, (uint32_t)(row_entries(&ea->row) + (ea->lh != NULL)));
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
        if (v.as.ref == (const void *)&pico8_pj_gs_row || v.as.ref == (const void *)&pico8_pj_i__row) {
            return fold_row(fnv(h, 0x7Au), v.as.ref);
        }
        if (v.as.ref == (const void *)pico8_pj_ou()) {
            return fold_row(fnv(h, 0x7Au), &pico8_pj_ou()->row);
        }
        if (v.as.ref == (const void *)pico8_pj_ea()) {
            return fold_ea(fnv(h, 0x7Au), v.as.ref);
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

/* `cr` read off a panel, folded as `fold_field` folded the spec table it
 * was: its entries in spec order, then their count. */
static uint32_t fold_cr_field(uint32_t h, const void *e) {
    static const char *const names[] = {"hi", "gq", "ca", "ep"};
    const void *cr = get_ref(e, "cr");
    int count = 0;
    size_t i;

    assert(cr != NULL);
    h = fnv(fnv(h, 1u), 0x7Au);
    for (i = 0; i < sizeof names / sizeof names[0]; i++) {
        pico8_value_t v;
        if (lookup(cr, names[i], &v)) {
            h = fold_value(fnv_str(h, names[i]), v);
            count++;
        }
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

    for (i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)pico8_peek(&mem, PICO8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
}

static const pico8_struct_class_t *cls_of(const void *e) {
    return ((const pico8_struct_header_t *)e)->cls;
}

static pico8_value_t nil(void) {
    return pico8_value_ref(NULL);
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void *start_game(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
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
static uint32_t fold_ou_heads(uint32_t h, const void *e) {
    const pico8_pj_ou_t *ou = get_ref(e, "ou");
    int i;

    for (i = 1; i <= 7; i++) {
        h = fold_value(h, item_value(&row_of(&ou->row, i)->items[0]));
    }
    return h;
}

static int is_of(const void *e, const pico8_struct_class_t *cls) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)e)->cls == cls;
}

static int fq_count(void) {
    const pico8_list_t *hv = pico8_hv();
    int n = 0;
    int i;
    for (i = 0; i < hv->count; i++) {
        n += is_of(hv->items[i].as.ref, pico8_fq_struct_class());
    }
    return n;
}

static int js15_count(void) {
    const pico8_list_t *hv = pico8_hv();
    int n = 0;
    int i;
    for (i = 0; i < hv->count; i++) {
        const void *e = hv->items[i].as.ref;
        n += is_of(e, pico8_js_struct_class()) && get_fixed(e, "fa") == pico8_fixed_from_int(15);
    }
    return n;
}

static pico8_value_t de_my(void) {
    return pico8_de_my()(NULL, 0, nil(), nil());
}

/* The game's panel as `eu()` and `lt` left it: where it registered, its
 * defaults, the names only read elsewhere, its 4 methods, and the 5 tables
 * of the class, by identity and content. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *ju = start_game();
    pico8_value_t me;
    size_t i;

    h = fnv(h, (uint32_t)rank_in(pico8_hv(), ju));
    h = fnv(h, (uint32_t)pico8_hv()->count);
    assert(listed(pico8_cv("kj"), ju) && listed(pico8_cv("hk"), ju) && listed(pico8_cv("hb"), ju));
    assert(!listed(pico8_cv("ev"), ju) && !listed(pico8_cv("hd"), ju));
    h = fnv(h, (uint32_t)rank_in(pico8_cv("kj"), ju));
    h = fnv(h, (uint32_t)rank_in(pico8_cv("hk"), ju));
    h = fnv(h, (uint32_t)rank_in(pico8_cv("hb"), ju));
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        assert(!listed(pico8_cg(tags[i]), ju));
    }

    assert(get_fixed(ju, "fa") == pico8_fixed_from_int(10));
    assert(get_fixed(ju, "fa") == pico8_pj_start.fa);
    assert(get_fixed(ju, "ed") == pico8_fixed_from_int(1));
    assert(get_fixed(ju, "ed") == pico8_pj_start.ed);
    assert(lookup(ju, "me", &me) && me.kind == PICO8_VALUE_STRING && strcmp(me.as.string, "ph") == 0);
    h = fold_field(h, ju, "t");
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(ju, absent[i]));
    }
    assert(cls_of(ju)->kj != NULL && cls_of(ju)->states[PICO8_STATE_PH] != NULL && cls_of(ju)->hb != NULL &&
           cls_of(ju)->hk != NULL);
    assert(cls_of(ju)->hd == NULL && cls_of(ju)->ev == NULL);
    assert(get_ref(ju, "ea") == pico8_pj_ea());
    h = fold_field(h, ju, "ea");
    assert(get_ref(ju, "gs") == &pico8_pj_gs_row);
    h = fold_field(h, ju, "gs");
    assert(get_ref(ju, "cr") == pico8_pj_cr());
    h = fold_cr_field(h, ju);
    assert(get_ref(ju, "i_") == &pico8_pj_i__row);
    h = fold_field(h, ju, "i_");
    assert(get_ref(ju, "ou") == pico8_pj_ou());
    h = fold_field(h, ju, "ou");
    check("built", h, BUILT_HASH);
}

/* A second panel shares the class's `ou`/`ea`/`cr`/`gs`/`i_` and sees what
 * `hb`/`hk` wrote into them through the first; `fa`/`ed`/`c_`/`gz` stay
 * each panel's own. */
static void test_shared(void) {
    uint32_t h = 2166136261u;
    void *ju = start_game();
    void *other = pico8_pj_new();
    const pico8_pj_ea_t *ea;
    size_t i;

    assert(other != NULL && other != ju);
    for (i = 0; i < sizeof tables / sizeof tables[0]; i++) {
        assert(get_ref(other, tables[i]) == get_ref(ju, tables[i]));
    }
    assert(get_ref(other, "cr") == get_ref(ju, "cr"));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), other));

    cls_of(ju)->hb(ju);
    h = fold_ou_heads(h, other);
    assert(strcmp(string_at(row_of(get_ref(other, "ou"), 1), 1), string_at(row_of(get_ref(ju, "ou"), 1), 1)) == 0);

    cls_of(ju)->hk(ju, pico8_fixed_from_int(3425));
    ea = get_ref(other, "ea");
    assert(strcmp(string_at(row_of(&ea->row, 2), 1), "year 3425") == 0);
    assert(ea->row.count == 3 && ea->lh != NULL);
    h = fold_field(h, other, "ea");

    pico8_fm_hy()->hl = "final score";
    pico8_fm_hy()->has_hl = 1;
    cls_of(ju)->states[PICO8_STATE_PH](ju, pico8_fixed_from_int(0));
    assert(get_fixed(ju, "fa") == pico8_fixed_from_int(13));
    assert(bool_state(ju, "c_") == 2);
    assert(get_fixed(other, "fa") == pico8_fixed_from_int(10));
    assert(get_fixed(other, "ed") == pico8_fixed_from_int(1));
    assert(!has(other, "c_") && !has(other, "gz"));
    h = fold_field(h, other, "fa");
    h = fold_field(h, other, "ed");
    h = fold_field(h, ju, "gz");
    check("shared", h, SHARED_HASH);
}

/* `hb`; `hk` on a year with a message and two without; `cr()`; `kj` closed;
 * the game's end through `ph`, twice; `kj` open; `de.my` twice. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *ju = start_game();
    const pico8_pj_ea_t *ea = get_ref(ju, "ea");
    const void *cr = get_ref(ju, "cr");
    static const int years[] = {3425, 3400, 3401};
    pico8_value_t ep;
    int fq_before;
    int js_before;
    int boxes;
    size_t i;

    cls_of(ju)->hb(ju);
    h = fold_ou_heads(h, ju);

    for (i = 0; i < sizeof years / sizeof years[0]; i++) {
        cls_of(ju)->hk(ju, pico8_fixed_from_int(years[i]));
        h = fnv(h, (uint32_t)(ea->row.count >= 3));
        h = fnv(h, (uint32_t)(ea->lh != NULL));
        h = fnv_str(h, string_at(row_of(&ea->row, 2), 1));
        h = fnv(h, (uint32_t)count_of(pico8_cg("mh")));
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), pico8_kc()));
    }
    assert(ea->row.count == 2 && ea->lh != NULL);

    pico8_cr();
    assert(get_ref(ju, "cr") == cr && cr == pico8_pj_cr());
    assert(lookup(cr, "ep", &ep) && ep.kind == PICO8_VALUE_STRING);
    h = fnv_str(h, ep.as.string);

    for (i = 0; i < 2; i++) {
        cls_of(ju)->kj(ju, NULL);
        h = fnv(h, screen_hash());
    }
    assert(!has(ju, "gz"));

    fq_before = fq_count();
    js_before = js15_count();
    pico8_fm_hy()->hl = "final score";
    pico8_fm_hy()->has_hl = 1;
    cls_of(ju)->states[PICO8_STATE_PH](ju, pico8_fixed_from_int(0));
    assert(bool_state(ju, "c_") == 2);
    assert(get_fixed(ju, "gz") == 0);
    assert(get_fixed(ju, "fa") == pico8_fixed_from_int(13));
    assert(fq_count() == fq_before + 1 && js15_count() == js_before + 2);
    assert(get_ref(ju, "ed") == pico8_kc() && pico8_kc() != NULL);
    boxes = count_of(pico8_cg("mh"));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), pico8_kc()));
    h = fnv(h, (uint32_t)pico8_hv()->count);

    cls_of(ju)->states[PICO8_STATE_PH](ju, pico8_fixed_from_int(0));
    assert(count_of(pico8_cg("mh")) == boxes);
    assert(get_ref(ju, "ed") == pico8_kc());

    for (i = 0; i < 3; i++) {
        cls_of(ju)->kj(ju, NULL);
        h = fold_field(h, ju, "gz");
        h = fnv(h, screen_hash());
    }
    assert(get_fixed(ju, "gz") == PICO8_FIXED_LIT(0x00009999, 0.6));

    assert(pico8_value_equal(de_my(), pico8_value_bool(1)));
    assert(bool_state(ju, "c_") == 1);
    assert(pico8_value_equal(de_my(), pico8_value_bool(1)));
    assert(bool_state(ju, "c_") == 2);
    h = fold_field(h, ju, "fa");
    check("life", h, LIFE_HASH);
}

/* `pj.cr` read as `pz` reads it. */
static int cr_get(const void *cr, const char *name, pico8_value_t *out) {
    return lookup(cr, name, out);
}

static pico8_vec_t vec_of(const void *e, const char *name) {
    pico8_value_t v;
    assert(lookup(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return *(const pico8_vec_t *)v.as.ref;
}

/* A row's `lq` as `js:pq` built it: its `w`, its `c`, its positional items. */
static uint32_t fold_lq(uint32_t h, const void *lq) {
    int i;

    assert(is_lq(lq));
    h = fold_field(h, lq, "w");
    h = fold_field(h, lq, "c");
    for (i = 0; i < row_count(lq); i++) {
        pico8_value_t item = row_item(lq, i);
        h = fnv(h, (uint32_t)(i + 1));
        h = item.kind == PICO8_VALUE_REF ? fnv(h, 0x12u) : fold_value(h, item);
    }
    return h;
}

/* What `pz(pj.cr, bs)` left in `ok.le`: the `ep` head row, then one row per
 * tech, all at `fa` = `ca`, sharing one `ra` = `hi`, `qf` stepped by `gq`. */
static uint32_t fold_cr_le(uint32_t h, const char **head) {
    const pico8_list_t *le;
    pico8_value_t v;
    const void *lq;
    const void *ra;
    int i;

    assert(lookup(pico8_ok(), "le", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    le = v.as.ref;
    assert(le->count >= 2);
    h = fnv(h, (uint32_t)le->count);
    lq = get_ref(le->items[0].as.ref, "lq");
    assert(lq != NULL && get_fixed(lq, "c") == pico8_fixed_from_int(13));
    assert(row_count(lq) > 0 && row_item(lq, 0).kind == PICO8_VALUE_STRING);
    *head = row_item(lq, 0).as.string;
    h = fnv_str(h, *head);
    ra = get_ref(le->items[0].as.ref, "ra");
    for (i = 0; i < le->count; i++) {
        const void *b = le->items[i].as.ref;
        pico8_vec_t qf = vec_of(b, "qf");
        assert(get_ref(b, "ra") == ra);
        assert(get_fixed(b, "fa") == pico8_fixed_from_int(10));
        assert(qf.x == 0 && qf.y == pico8_fixed_from_int(11 * (i + 1)));
        h = fold_field(h, b, "fa");
        h = fnv(fnv(h, (uint32_t)qf.x), (uint32_t)qf.y);
        h = fold_lq(h, get_ref(b, "lq"));
    }
    assert(vec_of(le->items[0].as.ref, "ra").x == 0 &&
           vec_of(le->items[0].as.ref, "ra").y == pico8_fixed_from_int(11));
    return h;
}

/* `cr()` twice on one game, then on the next game started in the same
 * process: one shared `pj.cr`, `ep` absent until the first `cr()` and kept
 * across games, the same head row each time. */
static void test_cr(void) {
    uint32_t h = 2166136261u;
    void *ju = start_game();
    void *cr = get_ref(ju, "cr");
    const char *head1;
    const char *head2;
    pico8_value_t ep;

    assert(cr != NULL && !cr_get(cr, "ep", &ep));
    pico8_cr();
    h = fold_cr_le(h, &head1);
    assert(cr_get(cr, "ep", &ep) && ep.kind == PICO8_VALUE_STRING && strcmp(ep.as.string, head1) == 0);
    pico8_cr();
    h = fold_cr_le(h, &head2);
    assert(strcmp(head1, head2) == 0);
    assert(get_ref(pico8_pj_new(), "cr") == cr);

    ju = start_game();
    assert(get_ref(ju, "cr") == cr);
    assert(cr_get(cr, "ep", &ep) && strcmp(ep.as.string, head2) == 0);
    pico8_cr();
    h = fold_cr_le(h, &head1);
    check("cr", h, CR_HASH);
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

static void write_c_(void *e) {
    pico8_entity_set_field(e, "c_", pico8_value_bool(1));
}

static pico8_value_t pico8_pj_ea_lh_witness(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_bool(1);
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
    assert(pico8_pj_start.ou == &pico8_pj_ou_row && pico8_pj_start.ea == &pico8_pj_ea_row);
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
 * `hk`, `ph` and no writer; it takes no construction key. `ea`, `cr`, `ou`
 * are the class's shared state; `gs`, `i_` are the generated rows. */
static void test_pj_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_pj_struct_class();
    pico8_pj_t *ju = start_game();
    int j;

    assert(is_of(ju, cls));
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        assert(cls->methods[j] == NULL);
    }
    assert(cls->hb != NULL && cls->kj != NULL && cls->hk != NULL && cls->states[PICO8_STATE_PH] != NULL);
    assert(cls->ev == NULL && cls->hd == NULL && cls->has_hb == NULL && cls->ldown == NULL && cls->rdown == NULL &&
           cls->gv == NULL);
    for (j = 0; j < PICO8_STATE_COUNT; j++) {
        assert((cls->states[j] != NULL) == (j == PICO8_STATE_PH));
    }
    assert(cls->field != NULL && cls->set_field == NULL);
    assert(pico8_pj_ready());
    assert(pico8_pj_ou() == get_ref(ju, "ou") && pico8_pj_ea() == get_ref(ju, "ea"));
    assert(pico8_pj_cr() == get_ref(ju, "cr"));
    assert(pico8_pj_i_() == &pico8_pj_i__row && pico8_pj_gs() == &pico8_pj_gs_row);
    assert(pico8_pj_i_() == pico8_pj_start.i_ && pico8_pj_gs() == pico8_pj_start.gs);
    assert(get_ref(ju, "gs") == &pico8_pj_gs_row && get_ref(ju, "i_") == &pico8_pj_i__row);

    must_abort(write_c_, ju);
}

static void write_ep(void *e) {
    pico8_entity_set_field(e, "ep", pico8_value_string("level 1"));
}

/* `pj.cr` is one static struct without method nor writer, holding `hi`,
 * `gq`, `ca` and, once `cr()` ran, `ep`; the init resets `ep`, the shutdown
 * hides it. */
static void test_cr_is_a_struct(void) {
    static const char *const never[] = {"eo", "ra", "ff", "fo", "nu", "lq", "fa"};
    const pico8_struct_class_t *cls = pico8_pj_cr_struct_class();
    void *ju = start_game();
    pico8_pj_cr_t *cr = pico8_pj_cr();
    pico8_value_t v;
    size_t i;
    int j;

    assert(cr != NULL && get_ref(ju, "cr") == cr && is_of(cr, cls));
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        assert(cls->methods[j] == NULL);
    }
    assert(cls->kj == NULL && cls->ev == NULL && cls->hd == NULL && cls->hk == NULL && cls->hb == NULL &&
           cls->ldown == NULL && cls->rdown == NULL && cls->gv == NULL);
    for (j = 0; j < PICO8_STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(cls->field != NULL && cls->set_field == NULL);
    assert(lookup(cr, "hi", &v) && v.kind == PICO8_VALUE_REF);
    assert(lookup(cr, "gq", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != get_ref(cr, "hi"));
    assert(lookup(cr, "ca", &v) && pico8_value_equal(v, pico8_value_fixed(pico8_fixed_from_int(10))));
    assert(!has(cr, "ep"));
    for (i = 0; i < sizeof never / sizeof never[0]; i++) {
        assert(!has(cr, never[i]));
    }
    must_abort(write_ep, cr);

    pico8_cr();
    assert(lookup(cr, "ep", &v) && v.kind == PICO8_VALUE_STRING);
    assert(get_ref(pico8_pj_new(), "cr") == cr);

    pico8_score_panel_init();
    assert(pico8_pj_cr() == cr && !has(cr, "ep") && has(cr, "ca"));
    pico8_score_panel_shutdown();
    assert(pico8_pj_cr() == NULL);
}

static void cr_vec_is_0_11(const void *cr, const char *name, const void *ref) {
    pico8_value_t v;
    assert(lookup(cr, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref == ref);
    assert(vec_of(cr, name).x == 0 && vec_of(cr, name).y == pico8_fixed_from_int(11));
}

static void cr_ca_is_10(const void *cr) {
    pico8_value_t v;
    assert(lookup(cr, "ca", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(10));
}

static void cr_ep_is(const void *cr, const char *text) {
    pico8_value_t v;
    assert(lookup(cr, "ep", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, text) == 0);
}

static void cr_misses(const pico8_struct_class_t *cls, const void *cr, const char *name) {
    pico8_value_t out = pico8_value_fixed(0x1234);
    assert(cls->field(cr, name, &out) == 0);
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == 0x1234);
}

static void test_cr_members_by_name(void) {
    const pico8_struct_class_t *cls = pico8_pj_cr_struct_class();
    void *cr;
    void *hi;
    void *gq;
    pico8_value_t v;

    start_game();
    cr = pico8_pj_cr();
    hi = get_ref(cr, "hi");
    gq = get_ref(cr, "gq");
    assert(hi != NULL && gq != NULL && hi != gq);
    assert(hi == &pico8_pj_cr_start.hi && gq == &pico8_pj_cr_start.gq);
    assert(((const pico8_pj_cr_t *)cr)->ca == pico8_pj_cr_start.ca);
    cr_vec_is_0_11(cr, "hi", hi);
    cr_vec_is_0_11(cr, "gq", gq);
    cr_ca_is_10(cr);
    assert(!lookup(cr, "ep", &v));
    cr_misses(cls, cr, "ep");
    cr_misses(cls, cr, "x");
    cr_misses(cls, cr, "");
    cr_misses(cls, cr, "eo");
    cr_misses(cls, cr, "ra");

    pico8_cr();
    cr_ep_is(cr, "level 1 (0/2)");
    cr_vec_is_0_11(cr, "hi", hi);
    cr_vec_is_0_11(cr, "gq", gq);
    cr_ca_is_10(cr);
    pico8_cr();
    cr_ep_is(cr, "level 1 (0/2)");

    pico8_score_panel_init();
    assert(!lookup(cr, "ep", &v));
    cr_misses(cls, cr, "ep");
    cr_vec_is_0_11(cr, "hi", hi);
    cr_vec_is_0_11(cr, "gq", gq);
    cr_ca_is_10(cr);
}

/* The raw reader and the entity reader agree; a miss leaves the witness. */
static uint32_t pj_read(uint32_t h, const void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(0x1234);
    pico8_value_t via = pico8_value_fixed(0x1234);
    int got = pico8_pj_struct_class()->field(e, name, &raw);

    assert(pico8_entity_get_field(e, name, &via) == got);
    assert(pico8_value_equal(raw, via));
    if (!got) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
    }
    h = fnv(h, (uint32_t)got);
    h = fnv(h, (uint32_t)raw.kind);
    if (raw.kind == PICO8_VALUE_FIXED) {
        h = fnv(h, (uint32_t)raw.as.fixed);
    } else if (raw.kind == PICO8_VALUE_BOOL) {
        h = fnv(h, (uint32_t)raw.as.boolean);
    }
    *out = raw;
    return h;
}

static uint32_t pj_fixed_is(uint32_t h, const void *e, const char *name, pico8_fixed_t want) {
    pico8_value_t v;
    h = pj_read(h, e, name, &v);
    assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == want);
    return h;
}

static uint32_t pj_bool_is(uint32_t h, const void *e, const char *name, int want) {
    pico8_value_t v;
    h = pj_read(h, e, name, &v);
    assert(v.kind == PICO8_VALUE_BOOL && v.as.boolean == want);
    return h;
}

static uint32_t pj_absent(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    h = pj_read(h, e, name, &v);
    assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x1234);
    return h;
}

static uint32_t pj_ref_is(uint32_t h, const void *e, const char *name, const void *want, pico8_type_tag_t tag) {
    pico8_value_t v;
    h = pj_read(h, e, name, &v);
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    h = fnv(h, (uint32_t)(v.as.ref == want));
    h = fnv(h, (uint32_t)(pico8_type_tag_of(v.as.ref) == tag));
    assert(v.as.ref == want && pico8_type_tag_of(v.as.ref) == tag);
    return h;
}

static uint32_t pj_fresh(uint32_t h, const void *e) {
    static const char *const strangers[] = {"",   "x",  "ED",     "e",      "ed ", "has_c_",
                                            "has_gz", "c", "ke", "header", "member"};
    pico8_value_t v;
    size_t i;

    h = pj_fixed_is(h, e, "fa", pico8_fixed_from_int(10));
    h = pj_ref_is(h, e, "gs", &pico8_pj_gs_row, PICO8_TYPE_TAG_UNKNOWN);
    h = pj_ref_is(h, e, "gs", pico8_pj_gs(), PICO8_TYPE_TAG_UNKNOWN);
    h = pj_ref_is(h, e, "i_", &pico8_pj_i__row, PICO8_TYPE_TAG_UNKNOWN);
    h = pj_ref_is(h, e, "i_", pico8_pj_i_(), PICO8_TYPE_TAG_UNKNOWN);
    h = pj_fixed_is(h, e, "ed", pico8_fixed_from_int(1));
    h = pj_absent(h, e, "c_");
    h = pj_absent(h, e, "gz");
    h = pj_ref_is(h, e, "ou", pico8_pj_ou(), PICO8_TYPE_TAG_UNKNOWN);
    h = pj_ref_is(h, e, "ea", pico8_pj_ea(), PICO8_TYPE_TAG_UNKNOWN);
    h = pj_ref_is(h, e, "cr", pico8_pj_cr(), PICO8_TYPE_TAG_STRUCT);
    h = pj_read(h, e, "me", &v);
    assert(v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, "ph") == 0);
    h = pj_read(h, e, "t", &v);
    for (i = 0; i < sizeof strangers / sizeof strangers[0]; i++) {
        h = pj_absent(h, e, strangers[i]);
    }
    return h;
}

static void write_c_nil(void *e) {
    pico8_entity_set_field(e, "c_", nil());
}

static void write_ed_zero_false(void *e) {
    pico8_value_t v;
    memset(&v, 0, sizeof v);
    v.kind = PICO8_VALUE_BOOL;
    pico8_entity_set_field(e, "ed", v);
}

static void write_gz_zero_fixed(void *e) {
    pico8_value_t v;
    memset(&v, 0, sizeof v);
    v.kind = PICO8_VALUE_FIXED;
    pico8_entity_set_field(e, "gz", v);
}

static void write_fa(void *e) {
    pico8_entity_set_field(e, "fa", pico8_value_fixed(pico8_fixed_from_int(3)));
}

static pico8_value_t ix3_my(void) {
    return pico8_ix_my(3)(NULL, 0, nil(), nil());
}

/* The six members by name on a fresh panel, then through `de.my`, `ix[3].my`,
 * `ph` with `kc` NULL, and `kj`; no writer by name. */
static void test_pj_members_by_name(void) {
    uint32_t h = 2166136261u;
    void *ju = start_game();
    void *b = pico8_pj_new();
    void *ed;
    int boxes;
    int i;

    assert(b != ju);
    h = pj_fresh(h, ju);
    h = pj_fresh(h, b);

    (void)de_my();
    h = pj_bool_is(h, ju, "c_", 1);
    (void)de_my();
    h = pj_bool_is(h, ju, "c_", 0);
    (void)ix3_my();
    h = pj_bool_is(h, ju, "c_", 1);
    h = pj_absent(h, b, "c_");

    pico8_msgbox_set_kc(NULL);
    pico8_fm_hy()->hl = "final score";
    pico8_fm_hy()->has_hl = 1;
    cls_of(ju)->states[PICO8_STATE_PH](ju, pico8_fixed_from_int(0));
    h = pj_bool_is(h, ju, "c_", 1);
    h = pj_fixed_is(h, ju, "gz", 0);
    h = pj_fixed_is(h, ju, "fa", pico8_fixed_from_int(13));
    assert(pico8_kc() != NULL);
    h = pj_ref_is(h, ju, "ed", pico8_kc(), PICO8_TYPE_TAG_STRUCT);
    ed = pico8_kc();
    h = pj_fixed_is(h, b, "fa", pico8_fixed_from_int(10));
    h = pj_fixed_is(h, b, "ed", pico8_fixed_from_int(1));
    h = pj_absent(h, b, "c_");
    h = pj_absent(h, b, "gz");
    boxes = count_of(pico8_cg("mh"));
    h = fnv(h, (uint32_t)boxes);

    cls_of(ju)->states[PICO8_STATE_PH](ju, pico8_fixed_from_int(0));
    assert(count_of(pico8_cg("mh")) == boxes);
    h = pj_ref_is(h, ju, "ed", ed, PICO8_TYPE_TAG_STRUCT);

    for (i = 0; i < 3; i++) {
        cls_of(ju)->kj(ju, NULL);
    }
    h = pj_fixed_is(h, ju, "gz", PICO8_FIXED_LIT(0x00009999, 0.6));
    h = pj_fixed_is(h, ju, "fa", pico8_fixed_from_int(13));
    h = pj_ref_is(h, ju, "ed", ed, PICO8_TYPE_TAG_STRUCT);
    h = pj_bool_is(h, ju, "c_", 1);

    assert(pico8_pj_struct_class()->set_field == NULL);
    must_abort(write_c_nil, ju);
    must_abort(write_ed_zero_false, ju);
    must_abort(write_gz_zero_fixed, ju);
    must_abort(write_fa, ju);
    check("pj members", h, PJ_MEMBERS_HASH);
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
        pico8_list_add(fm->invented, pico8_value_fixed(pico8_fixed_from_int(i)));
    }
    fm->happiness = pico8_fixed_from_int(1);
    fm->has_happiness = 1;
    s = pico8_ky();
    assert(s.di == 0 && s.gl == 0 && s.el == pico8_fixed_from_int(1430));
    assert(strcmp(s.happiness, "1%") == 0);
    assert(s.total == pico8_fixed_from_int(14) && s.tier == 0);
}

/* Item `i` (1-based) of the line that `ph` put in the box's row `k`
 * (0-based), an article `ld`. */
static pico8_value_t ph_row_item(const pico8_mh_t *mh, int k, int i) {
    const pico8_jb_row_t *ld;
    const pico8_lq_t *lq;

    assert(k < mh->items.count && mh->items.items[k].kind == PICO8_VALUE_REF);
    ld = mh->items.items[k].as.ref;
    assert(ld->fn != NULL && strcmp(ld->fn, "ld") == 0 && ld->items[0].is_ref);
    lq = ld->items[0].ref;
    assert(i <= lq->items.count);
    return lq->items.items[i - 1];
}

/* `ph` shows `ky()`: the score-5 row (box row 10) carries `s[5]`, the total,
 * and the tier row (11) lights `s[6]` of its five icons. */
static void test_ph_rows_show_ky(void) {
    void *ju = start_game();
    pico8_hy_t *fm = pico8_fm_hy();
    const pico8_mh_t *mh;
    pico8_value_t total;
    int i;

    for (i = 0; i < 10; i++) {
        pico8_list_add(fm->invented, pico8_value_fixed(pico8_fixed_from_int(i)));
    }
    fm->happiness = pico8_fixed_from_int(1);
    fm->has_happiness = 1;
    fm->hl = "final score";
    fm->has_hl = 1;
    cls_of(ju)->states[PICO8_STATE_PH](ju, pico8_fixed_from_int(0));
    mh = pico8_kc();
    assert(mh != NULL && mh->items.count == PICO8_BZ_COUNT);
    total = ph_row_item(mh, 10, 3);
    assert(total.kind == PICO8_VALUE_STRING && strcmp(total.as.string, "14") == 0);
    for (i = 1; i <= 5; i++) {
        assert(pico8_value_equal(ph_row_item(mh, 11, i), pico8_value_fixed(pico8_fixed_from_int(57))));
    }
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
    in_fresh_child(test_shared);
    in_fresh_child(test_life);
    in_fresh_child(test_cr);
    in_fresh_child(test_ou_ea_start_as_their_rows);
    in_fresh_child(test_pj_is_a_struct);
    in_fresh_child(test_cr_is_a_struct);
    in_fresh_child(test_cr_members_by_name);
    in_fresh_child(test_pj_members_by_name);
    in_fresh_child(test_ky_reads_invented_and_happiness);
    in_fresh_child(test_ph_rows_show_ky);

    printf("test_pico8_pj: all tests passed\n");
    return 0;
}
