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

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_display.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_tech_gate.h"
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

#define GB_COUNT 34
#define GB_NAMED 26
#define GB_HASH 0x1339FD84u
#define DEFAULTS_HASH 0xC841FB8Au
#define FUNCTIONS_HASH 0x538CF904u
#define FUNCTIONS_INVENTED_HASH 0xA64C8C0Fu
#define INSTALL_HASH 0xE9A9797Cu
#define BUILT_WORLD_HASH 0xDF3A48A1u
#define HARVESTED_WORLD_HASH 0xE7F2768Cu

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

static int rank_in(const pico8_ptrs_t *list, const void *e) {
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

/* The kind the name reader gave `lq` when it fell back to the method,
 * frozen in GB_HASH; the enum entry is gone.
 * Changing it reddens them. */
enum { FUNCTION_KIND = 4 };

/* Kind, then the string's bytes, the number, the boolean; a REF by its
 * tag and, above `depth` 0, an entity by its rank in `hv` or in `gb`. */
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
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), v.as.ref));
        return fnv(h, (uint32_t)gb_rank(v.as.ref));
    default:
        return h;
    }
}

/* A want's `qe` with the bytes fold_value folded for it, which the frozen
 * hashes hold: kind word 0 then the number; kind word 3 then 0xDEAD for nil. */
static uint32_t fold_qe(uint32_t h, pico8_qe_t qe) {
    assert(qe.kind != PICO8_QE_FALSE && "a want's qe is never false");
    return qe.kind == PICO8_QE_FIXED ? fnv(fnv(h, 0u), qe.n) : fnv(fnv(h, 3u), 0xDEADu);
}

/* The frozen hashes were taken when rows were ob views and name lists ob
 * tables: these are the bytes they folded for their tag and their two kinds
 * of key, frozen values of a tag and an enum since deleted. Changing one
 * reddens every constant that folds it. */
enum { VIEW_TAG = 0x9E5C0B7Au, VIEW_INDEX_KEY = 0, VIEW_NAME_KEY = 1, VIEW_LIST_TAG = 0x9E5C1157u };

/* `REF`, the count, then per name the index-key kind, its position and
 * the string: the format DEFAULTS_HASH froze. */
static uint32_t fold_names(uint32_t h, const pico8_str_seq_t *names) {
    int i;

    h = fnv(h, (uint32_t)PICO8_VALUE_REF);
    h = fnv(h, (uint32_t)names->count);
    for (i = 0; i < names->count; i++) {
        h = fnv(h, (uint32_t)VIEW_INDEX_KEY);
        h = fnv(h, (uint32_t)(i + 1));
        h = fold_value(h, pico8_value_string(names->items[i]), 1);
    }
    return h;
}

static uint32_t fold_absent(uint32_t h) {
    return fnv(h, 0xFFFFFFFFu);
}

/* fold_value's cases for a member of one kind: the kind word is constant by
 * the struct's shape, kept so that GB_HASH and INSTALL_HASH still prove the
 * typed reads. A NULL string, or an unset flag, folds as absent. */
static uint32_t fold_string_member(uint32_t h, const char *s) {
    if (s == NULL) {
        return fold_absent(h);
    }
    h = fnv(h, (uint32_t)PICO8_VALUE_STRING);
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFu);
}

static uint32_t fold_fixed_member(uint32_t h, int has, pico8_fixed_t v) {
    return has ? fnv(fnv(h, (uint32_t)PICO8_VALUE_FIXED), (uint32_t)v) : fold_absent(h);
}

static uint32_t fold_bool_member(uint32_t h, int has, int b) {
    return has ? fnv(fnv(h, (uint32_t)PICO8_VALUE_BOOL), (uint32_t)(b != 0)) : fold_absent(h);
}

/* `il` false is `has_il` with a NULL `il`. */
static uint32_t fold_il(uint32_t h, const pico8_hp_t *b) {
    if (!b->has_il) {
        return fold_absent(h);
    }
    return b->il == NULL ? fold_bool_member(h, 1, 0) : fold_string_member(h, b->il);
}

static uint32_t fold_view_head(uint32_t h, int count) {
    h = fnv(h, (uint32_t)PICO8_VALUE_REF);
    h = fnv(h, (uint32_t)VIEW_TAG);
    return fnv(h, (uint32_t)count);
}

static uint32_t fold_view_index(uint32_t h, int i) {
    return fnv(fnv(h, (uint32_t)VIEW_INDEX_KEY), (uint32_t)i);
}

static uint32_t fold_view_name(uint32_t h, const char *name) {
    return fold_value(fnv(h, (uint32_t)VIEW_NAME_KEY), pico8_value_string(name), 0);
}

static uint32_t fold_row(uint32_t h, const pico8_jb_row_t *row, int depth);

static uint32_t fold_item(uint32_t h, const pico8_jb_item_t *item, int depth) {
    if (item->row != NULL) {
        return fold_row(h, item->row, depth);
    }
    if (item->s != NULL) {
        return fold_value(h, pico8_value_string(item->s), depth);
    }
    if (item->is_false) {
        return fold_value(h, pico8_value_bool(0), depth);
    }
    if (item->is_ref) {
        return fold_value(h, pico8_value_ref((void *)item->ref), depth);
    }
    return fold_value(h, pico8_value_fixed(item->is_fixed ? item->fixed : pico8_fixed_from_int(item->n)), depth);
}

/* A generated or built row, folded as fold_value folded its view: items,
 * then `c`, then `fn`. */
static uint32_t fold_row(uint32_t h, const pico8_jb_row_t *row, int depth) {
    int i;

    if (depth <= 0) {
        return fnv(fnv(h, (uint32_t)PICO8_VALUE_REF), (uint32_t)VIEW_TAG);
    }
    h = fold_view_head(h, row->count + (row->has_c != 0) + (row->fn != NULL));
    for (i = 0; i < row->count; i++) {
        h = fold_item(fold_view_index(h, i + 1), &row->items[i], depth - 1);
    }
    if (row->has_c) {
        h = fold_value(fold_view_name(h, "c"), pico8_value_fixed(pico8_fixed_from_int(row->c)), depth - 1);
    }
    if (row->fn != NULL) {
        h = fold_value(fold_view_name(h, "fn"), pico8_value_string(row->fn), depth - 1);
    }
    return h;
}

/* A row's positional as the value it was sorted from folded; NOTHING as
 * false (no row folded here holds a nil at the top level). */
static uint32_t fold_lq_item(uint32_t h, pico8_lq_cell_t c, int depth) {
    switch (c.kind) {
    case PICO8_LQ_CELL_ROW:
        return fold_row(h, c.as.row, depth);
    case PICO8_LQ_CELL_FIXED:
        return fold_value(h, pico8_value_fixed(c.as.fixed), depth);
    case PICO8_LQ_CELL_STRING:
        return fold_value(h, pico8_value_string(c.as.s), depth);
    default:
        return fold_value(h, pico8_value_bool(c.kind == PICO8_LQ_CELL_TRUE), depth);
    }
}

/* A row folded as fold_value folded its table view: its first `#fs`
 * positionals, then `c`, `bg`, `score`, `ra`, `fs`, `w`, then the rest. */
static uint32_t fold_lq_instance(uint32_t h, const void *l) {
    const pico8_lq_t *lq = l;
    int k;
    int i;

    if (l == NULL) {
        return fnv(h, 0xDEADu);
    }
    assert(pico8_type_tag_of(lq) == PICO8_TYPE_TAG_STRUCT && lq->header.cls == pico8_lq_struct_class());
    assert(!lq->has_bg);
    assert(lq->fs != NULL);
    k = lq->fs_count;
    h = fold_view_head(h, lq->items_count + (lq->has_c != 0) +
                              (lq->has_score != 0) + (lq->ra != NULL) + 2);
    for (i = 0; i < k; i++) {
        h = fold_lq_item(fold_view_index(h, i + 1), lq->items[i], 3);
    }
    if (lq->has_c) {
        h = fold_value(fold_view_name(h, "c"), pico8_value_fixed(lq->c), 3);
    }
    if (lq->has_score) {
        h = fold_value(fold_view_name(h, "score"), pico8_value_fixed(lq->score), 3);
    }
    if (lq->ra != NULL) {
        h = fold_value(fold_view_name(h, "ra"), pico8_value_ref(lq->ra), 3);
    }
    /* `fs` as fold_value folded the tagged list it was: REF, its tag, its count, each entry a REF. */
    h = fnv(fnv(fold_view_name(h, "fs"), (uint32_t)PICO8_VALUE_REF), (uint32_t)VIEW_LIST_TAG);
    h = fnv(h, (uint32_t)lq->fs_count);
    for (i = 0; i < lq->fs_count; i++) {
        h = fold_value(h, pico8_value_ref((void *)&lq->fs[i]), 2);
    }
    h = fold_value(fold_view_name(h, "w"), pico8_value_fixed(lq->w), 3);
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

    for (i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)pico8_peek(&mem, PICO8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
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
 * hovered planet off `ok.o_`), then left to settle. */
static void built_world(void) {
    const pico8_ptrs_t *jos;
    pico8_jc_keys_t jc_keys = {0};
    pico8_vec_t *mp = malloc(sizeof *mp);
    void *qu;
    void *re = NULL;
    pico8_jc_t *jc;
    int i;

    start_game();
    jos = pico8_cg("jo");
    assert(jos != NULL && jos->count > 1);
    qu = jos->items[0];
    for (i = 1; i < jos->count; i++) {
        void *e = jos->items[i];
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
 * class's `mr`/`gp`/`fk` shared by all, and `gb[px]` aliasing the last
 * project of that `px`. */
static void test_gb(void) {
    uint32_t h = 2166136261u;
    uint32_t d = 2166136261u;
    void *mr = NULL;
    void *gp = NULL;
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
        const pico8_ptrs_t *i_list;
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
            pico8_qe_t qe;
            qe = pico8_jy_qe(i_list->items[j]);
            h = fold_qe(h, qe);
        }
        if (b->lq != NULL) {
            const pico8_jb_row_t *lq = b->lq;
            /* Constant by the struct's shape, kept so that GB_HASH still
             * proves the typed reads. */
            h = fnv(h, (uint32_t)PICO8_VALUE_REF);
            assert(lq == &b->lq_own && b->has_ii);
            for (j = 1; j <= 3; j++) {
                h = j <= lq->count ? fold_item(h, &lq->items[j - 1], 2) : fnv(h, 0xFFFFFFFFu);
            }
        } else {
            h = fnv(h, (uint32_t)FUNCTION_KIND);
            assert(b->lq == NULL && !b->has_ii);
        }

        if (k == 1) {
            mr = (void *)b->mr;
            gp = (void *)b->gp;
            fk = (void *)b->fk;
            assert(mr != NULL && gp != NULL && fk != NULL);
        }
        assert((void *)b->mr == mr && (void *)b->gp == gp && (void *)b->fk == fk);
        if (b->px != NULL) {
            found = pico8_gb_find(b->px);
            assert(found != NULL);
            assert(found == last_with_px(b->px));
            h = fnv(h, (uint32_t)gb_rank(found));
        }
    }
    assert(mr == &pico8_hp_mr_names && gp == &pico8_hp_gp_names && fk == &pico8_hp_fk_row);
    d = fold_names(d, mr);
    d = fold_names(d, gp);
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
        const pico8_hc_entry_t *po = pico8_hp_po(b);
        const pico8_jb_row_t *jb = pico8_hp_jb(b);
        uint32_t lq_h;

        h = fnv(h, (uint32_t)k);
        h = fnv(h, (uint32_t)(po == NULL ? -2 : (int)(po - pico8_hc_table)));
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
    pico8_nb_entry_t *t = malloc(sizeof *t);

    *t = (pico8_nb_entry_t){px, 0, NULL, 0, 0, NULL};
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

static pico8_value_t vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return pico8_value_ref(v);
}

static pico8_vec_t vec_xy(int x, int y) {
    return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
}

/* A bare planet on `id`/`p`/`ra`, `ra` offset from `mo`'s. */
static void *new_planet(const char *id, int x, int y) {
    pico8_jo_keys_t keys;
    const pico8_vec_t *mo_ra;

    assert(pico8_mo_qb() != NULL);
    mo_ra = pico8_mo_qb()->ra;
    assert(mo_ra != NULL);
    keys.id = id;
    keys.p = vec_ref(vec_xy(x / 2, y / 2)).as.ref;
    keys.ra = vec_ref(pico8_vec_add(*mo_ra, vec_xy(x, y))).as.ref;
    return pico8_jo_new(&keys);
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
static uint32_t fold_kw(uint32_t h, const pico8_jo_t *p) {
    const pico8_kw_t *k = p->kw;

    if (k == NULL) {
        return fnv(h, 0xFFFFFFFFu);
    }
    h = fnv(h, 1u);
    h = k->has_qe ? fnv(fnv(h, 1u), (uint32_t)k->qe) : fnv(h, 0u);
    return fnv(h, (uint32_t)k->mu);
}

/* A planet's `jy`, folded as fold_value folded the list of tables it was:
 * each want, `qe` (nil included), then `gc` when written. */
static uint32_t fold_jy(uint32_t h, const pico8_jo_t *p) {
    const pico8_ptrs_t *jy = p->jy;
    int i;

    if (jy == NULL) {
        return fnv(h, 0xFFFFFFFFu);
    }
    h = fnv(fnv(h, (uint32_t)PICO8_VALUE_REF), (uint32_t)VIEW_LIST_TAG);
    h = fnv(h, (uint32_t)jy->count);
    for (i = 0; i < jy->count; i++) {
        const pico8_jy_t *want = jy->items[i];

        h = fold_view_head(h, 1 + (want->has_gc != 0));
        h = fold_qe(fold_view_name(h, "qe"), pico8_jy_qe(want));
        if (want->has_gc) {
            h = fold_value(fold_view_name(h, "gc"), pico8_value_fixed(want->gc), 2);
        }
    }
    return h;
}

static uint32_t fold_planet(uint32_t h, void *e) {
    const pico8_jo_t *p = as_jo(e);

    h = fnv(h, (uint32_t)gb_rank(p->hn));
    h = fnv(h, (uint32_t)(p->hn != NULL));
    h = fold_string_member(h, p->ep);
    h = fold_jy(h, p);
    h = fold_kw(h, p);
    h = fold_string_member(h, p->ke.me);
    h = fold_bool_member(h, p->has_fx, p->fx);
    h = fold_bool_member(h, p->has_p_, p->p_);
    h = fold_fixed_member(h, 1, p->ma);
    h = p->nk != NULL ? fnv(h, (uint32_t)(p->nk - pico8_df_table)) : fnv(h, 0xFFFFFFFFu);
    return fnv(h, (uint32_t)rank_in(pico8_hv(), p));
}

/* `hp:pq` makes a want per character of `i`, one of a character outside
 * `r` wanting no resource; `jo:lz` copies each want, never shares it. */
static void test_wants(void) {
    pico8_fd_entry_t ri = {0};
    pico8_hp_keys_t keys = {0};
    const pico8_ptrs_t *jy;
    pico8_hp_t *b;
    void *a;
    int p;
    int k;

    start_game();
    ri.npos = 5;
    ri.px = "x";
    ri.lr = "y";
    ri.fj = 1;
    ri.ja = "z";
    ri.i = "pz";
    keys.ri = &ri;
    b = pico8_hp_new(&keys);
    assert(b->i->count == 2 && pico8_r_lookup("p", &p));
    assert(((const pico8_jy_t *)b->i->items[0])->row.has_qe);
    assert(((const pico8_jy_t *)b->i->items[0])->row.qe == pico8_fixed_from_int(p));
    assert(!((const pico8_jy_t *)b->i->items[1])->row.has_qe);

    a = new_planet("p950", 12, 16);
    pico8_jo_lz(a, b);
    jy = as_jo(a)->jy;
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
    pico8_jo_lz(a, plain);
    assert((void *)as_jo(a)->hn == plain && as_jo(a)->ep == NULL);
    h = fold_planet(h, a);

    a = new_planet("p902", 30, 14);
    pico8_jo_hu(a, plain);
    assert((void *)as_jo(a)->hn == plain);
    h = fold_planet(h, a);

    go_project = project_with(with_go, 0);
    a = new_planet("p903", 50, 16);
    pico8_jo_hu(a, go_project);
    assert(as_jo(a)->nk != NULL);
    h = fold_planet(h, a);

    for (nth = 0; nth < 2; nth++) {
        int before;
        pico8_hp_t *cw = project_with(with_cw, nth);
        a = new_planet(nth == 0 ? "p904" : "p905", 70 + 20 * nth, 18);
        before = pico8_hv()->count;
        pico8_jo_hu(a, cw);
        h = fold_string_member(h, cw->cw);
        h = fnv(h, (uint32_t)(pico8_hv()->count - before));
        h = fold_planet(h, a);
        h = fold_planet(h, pico8_hv()->items[pico8_hv()->count - 1]);
    }

    a = new_planet("p906", 110, 20);
    pico8_jo_hu(a, NULL);
    assert(as_jo(a)->hn == NULL);
    h = fold_planet(h, a);
    check("install", h, INSTALL_HASH);
}

/* Each planet of `cg.jo`: the rank in `gb` of its `hn`; then the screen,
 * once a frame is played and drawn. */
static uint32_t fold_world(void) {
    uint32_t h = 2166136261u;
    const pico8_ptrs_t *jo;
    int i;

    pico8__update60();
    pico8__draw();
    jo = pico8_cg("jo");
    for (i = 0; jo != NULL && i < jo->count; i++) {
        h = fnv(h, (uint32_t)gb_rank(as_jo(jo->items[i])->hn));
    }
    return fnv(h, screen_hash());
}

static void test_built_world(void) {
    built_world();
    check("built world", fold_world(), BUILT_WORLD_HASH);
}

/* A discovered planet harvested (`cw` "ga"): `jo:hu` swaps it for a `ga`. */
static void test_harvested_world(void) {
    const pico8_struct_header_t *last;
    void *p;
    int i;

    start_game();
    p = pico8_cg("jo")->items[0];
    pico8_jo_hh(p);
    pico8_jo_hu(p, project_with(with_cw, 1));
    last = pico8_hv()->items[pico8_hv()->count - 1];
    assert(pico8_type_tag_of(last) == PICO8_TYPE_TAG_STRUCT && last->cls == pico8_ga_struct_class());
    for (i = 0; i < 60; i++) {
        pico8_dg();
    }
    check("harvested world", fold_world(), HARVESTED_WORLD_HASH);
}

/* `gb` rebuilt from `fd` frees the projects it held and gives the same
 * projects; shutdown leaves no class and no `gb`, `fk` still the generated
 * row; init again, then a rebuild, gives them once more. */
static void test_rebuild_and_shutdown(void) {
    pico8_research_build_gb();
    test_gb();

    pico8_research_shutdown();
    assert(pico8_gb_count() == 0 && pico8_gb_at(1) == NULL && pico8_gb_find("colony") == NULL);
    assert(!pico8_hp_ready());
    assert(pico8_hp_fk() == &pico8_hp_fk_row);
    pico8_research_shutdown();

    pico8_research_init();
    assert(pico8_hp_ready() && pico8_hp_fk() == &pico8_hp_fk_row);
    pico8_research_build_gb();
    test_gb();
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
    pico8_fd_entry_t header = {0};
    pico8_hp_keys_t keys = {0};

    (void)e;
    header.header = "x";
    keys.ri = &header;
    (void)pico8_hp_new(&keys);
}

static void build_with_o_missing_from_r(void *e) {
    pico8_fd_entry_t ri = {0};
    pico8_hp_keys_t keys = {0};

    (void)e;
    ri.npos = 6;
    ri.px = "x";
    ri.lr = "y";
    ri.fj = 1;
    ri.ja = "z";
    ri.i = "";
    ri.o = "not a key of r";
    keys.ri = &ri;
    (void)pico8_hp_new(&keys);
}

static void build_with_ii_and_no_px(void *e) {
    pico8_fd_entry_t ri = {0};
    pico8_hp_keys_t keys = {0};

    (void)e;
    ri.has_ii = 1;
    ri.ii = 3;
    keys.ri = &ri;
    (void)pico8_hp_new(&keys);
}

static void build_with_nine_positions(void *e) {
    pico8_fd_entry_t ri = {0};
    pico8_hp_keys_t keys = {0};

    (void)e;
    ri.npos = 9;
    ri.px = "x";
    keys.ri = &ri;
    (void)pico8_hp_new(&keys);
}

static void build_with_no_px_string(void *e) {
    pico8_fd_entry_t ri = {0};
    pico8_hp_keys_t keys = {0};

    (void)e;
    ri.npos = 1;
    keys.ri = &ri;
    (void)pico8_hp_new(&keys);
}

static void build_with_no_o_number(void *e) {
    pico8_fd_entry_t ri = {0};
    pico8_hp_keys_t keys = {0};

    (void)e;
    ri.npos = 6;
    ri.px = "x";
    ri.lr = "y";
    ri.fj = 1;
    ri.ja = "z";
    ri.i = "";
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
    pico8_jo_lz(pico8_cg("jo")->items[0], e);
}

static void hu_on_other(void *e) {
    pico8_jo_hu(pico8_cg("jo")->items[0], e);
}

/* A project is a struct whose descriptor has no method and no writer, its
 * `mr`/`gp`/`fk` the class's; a header entry, a value `gp` maps that `r`
 * lacks, `ii` without `px`, more positions than `mr`, or a position of the
 * wrong type stops its construction, and its functions
 * refuse anything that is not a project. With no key, only `mr`/`gp`/`fk`, an empty `i` and the `lq`
 * method. */
static void test_hp_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_hp_struct_class();
    pico8_hp_t *b = project(1);
    pico8_hp_t *bare;
    int j;

    assert(pico8_type_tag_of(b) == PICO8_TYPE_TAG_STRUCT && b->header.cls == cls);
    assert(cls->kj == NULL && cls->ev == NULL && cls->hd == NULL && cls->hk == NULL && cls->hb == NULL &&
           cls->has_hb == NULL && cls->ldown == NULL && cls->rdown == NULL && cls->gv == NULL);
    assert(cls->hq == NULL && cls->ey == NULL && cls->kf == NULL && cls->cn == NULL && cls->ol == NULL &&
           cls->mm == NULL);
    for (j = 0; j < PICO8_STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(pico8_hp_ready());
    assert(b->fk != NULL && b->fk == pico8_hp_fk());

    bare = pico8_hp_new(NULL);
    assert(pico8_type_tag_of(bare) == PICO8_TYPE_TAG_STRUCT && bare->header.cls == cls);
    assert(bare->mr != NULL && bare->mr == b->mr && bare->gp != NULL && bare->gp == b->gp);
    assert(bare->fk != NULL && bare->fk == b->fk);
    assert(bare->mr == pico8_hp_start.mr && bare->gp == pico8_hp_start.gp && bare->fk == pico8_hp_start.fk);
    assert(bare->i != NULL && bare->i->count == 0);
    assert(bare->lq == NULL);
    assert(bare->px == NULL && bare->lr == NULL && !bare->has_fj && bare->ja == NULL && !bare->has_o);
    assert(!bare->has_il && !bare->has_dm && bare->rr == NULL && !bare->has_ii && bare->cw == NULL);
    assert(bare->go == NULL && !bare->has_rn);

    start_game();
    must_abort(build_with_header, NULL);
    must_abort(build_with_o_missing_from_r, NULL);
    must_abort(build_with_ii_and_no_px, NULL);
    must_abort_saying(build_with_nine_positions, NULL, "ri->npos <= self->mr->count");
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
        assert(i < PICO8_FD_COUNT);
        if (pico8_fd_table[i].header == NULL && strcmp(pico8_fd_table[i].px, first->px) == 0) {
            keys.ri = &pico8_fd_table[i];
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
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

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

    printf("test_pico8_hp: all tests passed\n");
    return 0;
}
