/* The research project `hp`, the 34 entries of `gb` built at cart load, read
 * only through pico8_entity_get_field, pico8_mv and `po`/`od`/`jb`/`lq`, then
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
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet.h"
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
#define DEFAULTS_HASH 0x6DDA7FDEu
#define FUNCTIONS_HASH 0x538CF904u
#define FUNCTIONS_INVENTED_HASH 0xA64C8C0Fu
#define INSTALL_HASH 0x3158A685u
#define BUILT_WORLD_HASH 0xDF3A48A1u
#define HARVESTED_WORLD_HASH 0xE7F2768Cu
#define RAW_PRESENCE_HASH 0x6AC3DFD6u
#define IL_STRING_COUNT 6
#define IL_FALSE_COUNT 4
#define IL_ABSENT_COUNT 24

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static const char *const own[] = {"px", "lr", "fj", "ja", "o", "il", "dm", "rr", "ii", "cw", "go", "rn"};
static const char *const absent[] = {"me", "nu", "fa", "id", "hp"};

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
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

/* The entry index in `t` whose value is the REF `e`, -1 when none is. */
static int entry_of(const pico8_ob_table_t *t, const void *e) {
    int i;
    for (i = 0; t != NULL && i < t->count; i++) {
        if (t->entries[i].value.kind == PICO8_VALUE_REF && t->entries[i].value.as.ref == e) {
            return i;
        }
    }
    return -1;
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

static void *get_ref(const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v) && v.kind == PICO8_VALUE_REF ? v.as.ref : NULL;
}

/* The positional entries of `gb`, in order. */
static int gb_count(void) {
    const pico8_ob_table_t *gb = pico8_gb();
    int i;
    int n = 0;
    for (i = 0; i < gb->count; i++) {
        n += gb->entries[i].key.kind == PICO8_OB_KEY_INDEX;
    }
    return n;
}

static void *project(int k) {
    pico8_value_t v;
    assert(pico8_ob_table_get_index(pico8_gb(), k, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
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

/* Kind, then the string's bytes, the number, the boolean; a list or an
 * ob_table is folded through (`depth` levels), an entity by its rank in
 * `hv` or in `gb`. */
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
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), v.as.ref));
        return fnv(h, (uint32_t)gb_rank(v.as.ref));
    default:
        return h;
    }
}

static uint32_t fold_field(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    if (!lookup(e, name, &v)) {
        return fnv(h, 0xFFFFFFFFu);
    }
    return fold_value(h, v, 4);
}

static pico8_ob_table_t *row_view(const void *lq) {
    assert(pico8_type_tag_of(lq) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)lq)->cls == pico8_lq_struct_class());
    return pico8_lq_table_view(lq);
}

/* A row through its table view, positionals and names alike. */
static uint32_t fold_lq_instance(uint32_t h, const void *l) {
    pico8_ob_table_t *view;

    if (l == NULL) {
        return fnv(h, 0xDEADu);
    }
    view = row_view(l);
    h = fold_value(h, pico8_value_ref(view), 4);
    pico8_ob_table_free(view);
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

static pico8_vec_t pos_of(const void *e) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, "p", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return *(const pico8_vec_t *)v.as.ref;
}

static pico8_fixed_t dist2(pico8_vec_t a, pico8_vec_t b) {
    pico8_vec_t d = pico8_vec_sub(a, b);
    return pico8_fixed_add(pico8_fixed_mul(d.x, d.x), pico8_fixed_mul(d.y, d.y));
}

static pico8_fixed_t must_fixed(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

/* A generated planet colonized by COLONY, its nearest one by EXPORTER, the
 * two joined by a slipway paid by `ey` as a drag pays it (`jc:hq` reads the
 * hovered planet off `ok.o_`), then left to settle. */
static void built_world(void) {
    const pico8_list_t *jos;
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    pico8_vec_t *mp = malloc(sizeof *mp);
    pico8_value_t out;
    void *qu;
    void *re = NULL;
    void *jc;
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

    pico8_ob_table_set_name(overrides, "qu", pico8_value_ref(qu));
    jc = pico8_jc_new(overrides);
    pico8_ob_table_free(overrides);
    *mp = pico8_vec_add(pico8_vec_mul(pos_of(re), pico8_fixed_from_int(128)),
                        pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(1)));
    pico8_ok()->o_ = re;
    assert(pico8_mv(jc, "hq", 1, pico8_value_ref(mp), pico8_value_ref(NULL), &out));
    pico8_ok()->o_ = NULL;
    while (must_fixed(jc, "t") < pico8_fixed_from_int(15)) {
        pico8_dg();
    }
    assert(pico8_mv(jc, "ey", 1, pico8_value_ref(re), pico8_value_ref(NULL), &out));
    for (i = 0; i < 120; i++) {
        pico8_dg();
    }
}

/* Several projects share a `px`: `gb[px]` is the last one `hp:pq` saw. */
static void *last_with_px(const char *px) {
    void *found = NULL;
    pico8_value_t v;
    int k;
    for (k = 1; k <= gb_count(); k++) {
        if (lookup(project(k), "px", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, px) == 0) {
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
    const pico8_ob_table_t *gb = pico8_gb();
    void *mr = NULL;
    void *gp = NULL;
    void *fk = NULL;
    int named = 0;
    int k;
    size_t n;

    check_count("gb", gb_count(), GB_COUNT);
    for (k = 0; k < gb->count; k++) {
        named += gb->entries[k].key.kind == PICO8_OB_KEY_NAME;
    }
    check_count("gb named", named, GB_NAMED);

    for (k = 1; k <= GB_COUNT; k++) {
        void *b = project(k);
        const pico8_list_t *i_list;
        pico8_value_t v;
        pico8_value_t px;
        int j;

        h = fnv(h, (uint32_t)k);
        for (n = 0; n < sizeof own / sizeof own[0]; n++) {
            h = fold_field(h, b, own[n]);
        }
        i_list = get_ref(b, "i");
        assert(i_list != NULL && pico8_type_tag_of(i_list) == PICO8_TYPE_TAG_LIST);
        h = fnv(h, (uint32_t)i_list->count);
        for (j = 0; j < i_list->count; j++) {
            pico8_value_t qe = pico8_value_ref(NULL);
            assert(i_list->items[j].kind == PICO8_VALUE_REF);
            (void)pico8_ob_table_get_name(i_list->items[j].as.ref, "qe", &qe);
            h = fold_value(h, qe, 0);
        }
        assert(lookup(b, "lq", &v));
        h = fnv(h, (uint32_t)v.kind);
        if (v.kind == PICO8_VALUE_REF) {
            const pico8_ob_table_t *lq = v.as.ref;
            pico8_value_t e;
            assert(pico8_type_tag_of(lq) == PICO8_TYPE_TAG_OB_TABLE && has(b, "ii"));
            for (j = 1; j <= 3; j++) {
                h = pico8_ob_table_get_index(lq, j, &e) ? fold_value(h, e, 2) : fnv(h, 0xFFFFFFFFu);
            }
        } else {
            assert(v.kind == PICO8_VALUE_FUNCTION && !has(b, "ii"));
        }

        if (k == 1) {
            mr = get_ref(b, "mr");
            gp = get_ref(b, "gp");
            fk = get_ref(b, "fk");
            assert(mr != NULL && gp != NULL && fk != NULL);
        }
        assert(get_ref(b, "mr") == mr && get_ref(b, "gp") == gp && get_ref(b, "fk") == fk);
        for (n = 0; n < sizeof absent / sizeof absent[0]; n++) {
            assert(!has(b, absent[n]));
        }
        if (lookup(b, "px", &px) && px.kind == PICO8_VALUE_STRING) {
            assert(pico8_ob_table_get_name(gb, px.as.string, &v) && v.kind == PICO8_VALUE_REF);
            assert(v.as.ref == last_with_px(px.as.string));
            h = fnv(h, (uint32_t)gb_rank(v.as.ref));
        }
    }
    d = fold_value(d, pico8_value_ref(mr), 2);
    d = fold_value(d, pico8_value_ref(gp), 2);
    d = fold_value(d, pico8_value_ref(fk), 2);
    check("gb", h, GB_HASH);
    check("mr gp fk", d, DEFAULTS_HASH);
}

/* Per project: `po` as its entry in `hc`, `od`, `jb`, `lq`, and `mv(b,"lq")`,
 * the member for a project with `ii`, a call of `lq` for the others. */
static uint32_t fold_functions(uint32_t h) {
    int k;

    for (k = 1; k <= GB_COUNT; k++) {
        void *b = project(k);
        const pico8_hc_entry_t *po = pico8_hp_po(b);
        pico8_ob_table_t *jb = pico8_hp_jb(b);
        pico8_value_t member;
        pico8_value_t via_mv;
        uint32_t lq_h;

        h = fnv(h, (uint32_t)k);
        h = fnv(h, (uint32_t)(po == NULL ? -2 : (int)(po - pico8_hc_table)));
        h = fnv(h, (uint32_t)pico8_hp_od(b));
        h = jb == NULL ? fnv(h, 0xDEADu) : fold_value(h, pico8_value_ref(jb), 4);
        assert((jb == NULL) == has(b, "ii"));
        lq_h = fold_lq_instance(2166136261u, pico8_hp_lq(b));
        h = fnv(h, lq_h);
        assert(pico8_mv(b, "lq", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &via_mv));
        assert(lookup(b, "lq", &member));
        if (member.kind == PICO8_VALUE_REF) {
            assert(via_mv.kind == PICO8_VALUE_REF && via_mv.as.ref == member.as.ref);
        } else {
            assert(via_mv.kind == PICO8_VALUE_REF && via_mv.as.ref != NULL);
            assert(fold_lq_instance(2166136261u, via_mv.as.ref) == lq_h);
        }
    }
    return h;
}

static void invent(const char *px) {
    pico8_ob_table_t *t = pico8_ob_table_new();
    pico8_ob_table_set_name(t, "px", pico8_value_string(px));
    pico8_hy_kt(pico8_fm_hy(), t);
    assert(pico8_oj(pico8_value_string(px)));
}

static void test_functions(void) {
    pico8_value_t il;
    int k;

    start_game();
    assert(!pico8_oj(pico8_value_string("nanomaterials")));
    check("functions", fold_functions(2166136261u), FUNCTIONS_HASH);

    invent("nanomaterials");
    for (k = 1; k <= GB_COUNT; k++) {
        if (lookup(project(k), "il", &il) && il.kind == PICO8_VALUE_STRING && !pico8_oj(il)) {
            invent(il.as.string);
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
    pico8_ob_table_t *b = pico8_ob_table_new();
    const pico8_vec_t *mo_ra = get_ref(pico8_mo(), "ra");
    void *made;

    assert(mo_ra != NULL);
    pico8_ob_table_set_name(b, "id", pico8_value_string(id));
    pico8_ob_table_set_name(b, "p", vec_ref(vec_xy(x / 2, y / 2)));
    pico8_ob_table_set_name(b, "ra", vec_ref(pico8_vec_add(*mo_ra, vec_xy(x, y))));
    made = pico8_jo_new(b);
    pico8_ob_table_free(b);
    return made;
}

/* The first project that has `name` (or has none of cw/go/ii for NULL). */
static void *project_with(const char *name, int nth) {
    int k;
    for (k = 1; k <= GB_COUNT; k++) {
        void *b = project(k);
        int match = name != NULL ? has(b, name) : !has(b, "cw") && !has(b, "go") && !has(b, "ii");
        if (match && nth-- == 0) {
            return b;
        }
    }
    assert(0 && "no such project");
    return NULL;
}

static uint32_t fold_planet(uint32_t h, const void *p) {
    static const char *const names[] = {"ep", "jy", "kw", "me", "fx", "p_", "ma"};
    size_t n;
    pico8_value_t nk;

    h = fnv(h, (uint32_t)gb_rank(get_ref(p, "hn")));
    h = fnv(h, (uint32_t)has(p, "hn"));
    for (n = 0; n < sizeof names / sizeof names[0]; n++) {
        h = fold_field(h, p, names[n]);
    }
    h = lookup(p, "nk", &nk) && nk.kind == PICO8_VALUE_REF ? fnv(h, (uint32_t)entry_of(pico8_df(), nk.as.ref))
                                                            : fnv(h, 0xFFFFFFFFu);
    return fnv(h, (uint32_t)rank_in(pico8_hv(), p));
}

static void test_install(void) {
    uint32_t h = 2166136261u;
    void *ordinary;
    void *a;
    void *go_project;
    int nth;

    start_game();
    ordinary = project_with(NULL, 0);

    a = new_planet("p901", 10, 12);
    pico8_jo_lz(a, ordinary);
    assert(get_ref(a, "hn") == ordinary && !has(a, "ep"));
    h = fold_planet(h, a);

    a = new_planet("p902", 30, 14);
    pico8_jo_hu(a, ordinary);
    assert(get_ref(a, "hn") == ordinary);
    h = fold_planet(h, a);

    go_project = project_with("go", 0);
    a = new_planet("p903", 50, 16);
    pico8_jo_hu(a, go_project);
    assert(has(a, "nk"));
    h = fold_planet(h, a);

    for (nth = 0; nth < 2; nth++) {
        int before;
        void *cw = project_with("cw", nth);
        a = new_planet(nth == 0 ? "p904" : "p905", 70 + 20 * nth, 18);
        before = pico8_hv()->count;
        pico8_jo_hu(a, cw);
        h = fold_field(h, cw, "cw");
        h = fnv(h, (uint32_t)(pico8_hv()->count - before));
        h = fold_planet(h, a);
        h = fold_planet(h, pico8_hv()->items[pico8_hv()->count - 1].as.ref);
    }

    a = new_planet("p906", 110, 20);
    pico8_jo_hu(a, NULL);
    assert(!has(a, "hn"));
    h = fold_planet(h, a);
    check("install", h, INSTALL_HASH);
}

/* Each planet of `cg.jo`: the rank in `gb` of its `hn`; then the screen,
 * once a frame is played and drawn. */
static uint32_t fold_world(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *jo;
    int i;

    pico8__update60();
    pico8__draw();
    jo = pico8_cg("jo");
    for (i = 0; jo != NULL && i < jo->count; i++) {
        h = fnv(h, (uint32_t)gb_rank(get_ref(jo->items[i].as.ref, "hn")));
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
    p = pico8_cg("jo")->items[0].as.ref;
    pico8_jo_hh(p);
    pico8_jo_hu(p, project_with("cw", 1));
    last = pico8_hv()->items[pico8_hv()->count - 1].as.ref;
    assert(pico8_type_tag_of(last) == PICO8_TYPE_TAG_STRUCT && last->cls == pico8_ga_struct_class());
    for (i = 0; i < 60; i++) {
        pico8_dg();
    }
    check("harvested world", fold_world(), HARVESTED_WORLD_HASH);
}

/* `gb` rebuilt from `fd` frees the projects it held and gives the same
 * projects; shutdown leaves no class, no `fk`, no `gb`; init again, then a
 * rebuild, gives them once more. */
static void test_rebuild_and_shutdown(void) {
    pico8_research_build_gb(pico8_fd());
    test_gb();

    pico8_research_shutdown();
    assert(pico8_gb() == NULL && !pico8_hp_ready());
    assert(pico8_hp_fk().kind == PICO8_VALUE_REF && pico8_hp_fk().as.ref == NULL);
    pico8_research_shutdown();

    pico8_research_init();
    assert(pico8_hp_ready() && pico8_hp_fk().as.ref != NULL);
    pico8_research_build_gb(pico8_fd());
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

static void write_px(void *e) {
    pico8_entity_set_field(e, "px", pico8_value_string("x"));
}

static void build_with_named_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "x", pico8_value_bool(1));
    (void)pico8_hp_new(b);
}

static void build_with_positional_9(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 9, pico8_value_bool(1));
    (void)pico8_hp_new(b);
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
    pico8_jo_lz(pico8_cg("jo")->items[0].as.ref, e);
}

static void hu_on_other(void *e) {
    pico8_jo_hu(pico8_cg("jo")->items[0].as.ref, e);
}

/* A project is a struct whose descriptor has no method and no writer, its
 * `mr`/`gp`/`fk` the class's; it takes no key but the positionals 1-8 and
 * rr, ii, cw, go, rn, and its functions refuse anything that is not a
 * project. With no key, only `mr`/`gp`/`fk`, an empty `i` and the `lq`
 * method. */
static void test_hp_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_hp_struct_class();
    pico8_hp_t *b = project(1);
    pico8_hp_t *bare;
    pico8_value_t v;
    size_t n;
    int j;

    assert(pico8_type_tag_of(b) == PICO8_TYPE_TAG_STRUCT && b->header.cls == cls);
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        assert(cls->methods[j] == NULL);
    }
    assert(cls->field != NULL && cls->set_field == NULL);
    assert(pico8_hp_ready());
    assert(!pico8_hp_default("lq", &v));
    assert(lookup(b, "fk", &v) && v.kind == pico8_hp_fk().kind && v.as.ref == pico8_hp_fk().as.ref);

    bare = pico8_hp_new(NULL);
    assert(pico8_type_tag_of(bare) == PICO8_TYPE_TAG_STRUCT && bare->header.cls == cls);
    assert(get_ref(bare, "mr") == get_ref(b, "mr") && get_ref(bare, "gp") == get_ref(b, "gp"));
    assert(get_ref(bare, "fk") == get_ref(b, "fk"));
    assert(get_ref(bare, "i") != NULL && ((const pico8_list_t *)get_ref(bare, "i"))->count == 0);
    assert(lookup(bare, "lq", &v) && v.kind == PICO8_VALUE_FUNCTION);
    for (n = 0; n < sizeof own / sizeof own[0]; n++) {
        assert(!has(bare, own[n]));
    }

    start_game();
    must_abort(write_px, b);
    must_abort(build_with_named_key, NULL);
    must_abort(build_with_positional_9, NULL);
    must_abort(od_on_other, pico8_ok());
    must_abort(po_on_other, pico8_ok());
    must_abort(jb_on_other, pico8_ok());
    must_abort(lq_on_other, pico8_ok());
    must_abort(lz_on_other, pico8_ok());
    must_abort(hu_on_other, pico8_ok());
}

static const char *const hp_members[] = {"mr", "gp", "fk", "px", "lr", "fj", "ja", "i", "o",
                                         "il", "dm", "rr", "ii", "cw", "go", "rn", "lq"};
static const char *const hp_intruders[] = {"", "x", "hp", "me", "has_ii", "has_il", "il_is_bool"};

/* The descriptor's reader, with a witness in `out`: 0 must leave it. */
static int raw_read(const void *e, const char *name, pico8_value_t *out) {
    int got;
    *out = pico8_value_fixed(0x1234);
    got = pico8_hp_struct_class()->field(e, name, out);
    assert(got || (out->kind == PICO8_VALUE_FIXED && out->as.fixed == 0x1234));
    return got;
}

static int member_kind_ok(const char *name, pico8_value_kind_t kind) {
    if (strcmp(name, "mr") == 0 || strcmp(name, "gp") == 0 || strcmp(name, "fk") == 0 || strcmp(name, "i") == 0) {
        return kind == PICO8_VALUE_REF;
    }
    if (strcmp(name, "fj") == 0 || strcmp(name, "o") == 0 || strcmp(name, "dm") == 0 || strcmp(name, "ii") == 0 ||
        strcmp(name, "rn") == 0) {
        return kind == PICO8_VALUE_FIXED;
    }
    if (strcmp(name, "il") == 0) {
        return kind == PICO8_VALUE_BOOL || kind == PICO8_VALUE_STRING;
    }
    if (strcmp(name, "lq") == 0) {
        return kind == PICO8_VALUE_REF || kind == PICO8_VALUE_FUNCTION;
    }
    return kind == PICO8_VALUE_STRING;
}

static const char *record_string(const pico8_ob_table_t *rec, int index, const char *name) {
    pico8_value_t v;
    int got = name != NULL ? pico8_ob_table_get_name(rec, name, &v) : pico8_ob_table_get_index(rec, index, &v);
    return got && v.kind == PICO8_VALUE_STRING ? v.as.string : NULL;
}

/* Every member of every project read by name, as the reader answers it:
 * present with a kind the member holds, or absent with `out` untouched;
 * strings the very pointers of `fd`'s record; the REFs stable; and `hn`/
 * `_jo_fe_b` still pointing at a project of `gb`. */
static void test_hp_members_by_name(void) {
    uint32_t h = 2166136261u;
    int il_string = 0;
    int il_false = 0;
    int il_absent = 0;
    const pico8_ob_table_t *fd = pico8_fd();
    pico8_hp_t *bare = pico8_hp_new(NULL);
    pico8_value_t v;
    pico8_value_t w;
    size_t n;
    int k;
    int f;

    for (n = 0; n < sizeof hp_members / sizeof hp_members[0]; n++) {
        const char *m = hp_members[n];
        if (strcmp(m, "mr") == 0 || strcmp(m, "gp") == 0 || strcmp(m, "fk") == 0) {
            assert(raw_read(bare, m, &v) && v.kind == PICO8_VALUE_REF && raw_read(project(1), m, &w) &&
                   v.as.ref == w.as.ref);
        } else if (strcmp(m, "i") == 0) {
            assert(raw_read(bare, m, &v) && v.kind == PICO8_VALUE_REF &&
                   pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST && ((const pico8_list_t *)v.as.ref)->count == 0);
        } else if (strcmp(m, "lq") == 0) {
            assert(raw_read(bare, m, &v) && v.kind == PICO8_VALUE_FUNCTION);
        } else {
            assert(!raw_read(bare, m, &v));
        }
    }
    assert(raw_read(bare, "fk", &v) && v.as.ref == pico8_hp_fk().as.ref);
    for (n = 0; n < sizeof hp_intruders / sizeof hp_intruders[0]; n++) {
        assert(!raw_read(bare, hp_intruders[n], &v));
        assert(!raw_read(project(1), hp_intruders[n], &v));
    }

    k = 0;
    for (f = 0; f < fd->count; f++) {
        const pico8_ob_table_t *rec;
        void *b;
        if (fd->entries[f].key.kind != PICO8_OB_KEY_INDEX || fd->entries[f].value.kind != PICO8_VALUE_REF ||
            fd->entries[f].value.as.ref == NULL) {
            continue;
        }
        rec = fd->entries[f].value.as.ref;
        b = project(++k);
        for (n = 0; n < sizeof hp_members / sizeof hp_members[0]; n++) {
            int got = raw_read(b, hp_members[n], &v);
            h = fnv(h, (uint32_t)k);
            h = fnv(h, (uint32_t)n);
            h = fnv(h, (uint32_t)got);
            if (got) {
                assert(!(v.kind == PICO8_VALUE_REF && v.as.ref == NULL));
                assert(member_kind_ok(hp_members[n], v.kind));
                h = fnv(h, (uint32_t)v.kind);
            }
        }
        if (!raw_read(b, "il", &v)) {
            il_absent++;
        } else if (v.kind == PICO8_VALUE_BOOL) {
            assert(v.as.boolean == 0);
            il_false++;
        } else {
            il_string++;
            assert(v.as.string == record_string(rec, 7, NULL));
        }
        assert(raw_read(b, "px", &v) == (record_string(rec, 1, NULL) != NULL));
        assert(record_string(rec, 1, NULL) == NULL || v.as.string == record_string(rec, 1, NULL));
        assert(raw_read(b, "lr", &v) == (record_string(rec, 2, NULL) != NULL));
        assert(record_string(rec, 2, NULL) == NULL || v.as.string == record_string(rec, 2, NULL));
        assert(raw_read(b, "ja", &v) == (record_string(rec, 4, NULL) != NULL));
        assert(record_string(rec, 4, NULL) == NULL || v.as.string == record_string(rec, 4, NULL));
        assert(raw_read(b, "rr", &v) == (record_string(rec, 0, "rr") != NULL));
        assert(record_string(rec, 0, "rr") == NULL || v.as.string == record_string(rec, 0, "rr"));
        assert(raw_read(b, "cw", &v) == (record_string(rec, 0, "cw") != NULL));
        assert(record_string(rec, 0, "cw") == NULL || v.as.string == record_string(rec, 0, "cw"));
        assert(raw_read(b, "go", &v) == (record_string(rec, 0, "go") != NULL));
        assert(record_string(rec, 0, "go") == NULL || v.as.string == record_string(rec, 0, "go"));
        for (n = 0; n < sizeof hp_members / sizeof hp_members[0]; n++) {
            const char *m = hp_members[n];
            if (strcmp(m, "i") == 0 || strcmp(m, "lq") == 0 || strcmp(m, "mr") == 0 || strcmp(m, "gp") == 0 ||
                strcmp(m, "fk") == 0) {
                assert(raw_read(b, m, &v) && raw_read(b, m, &w) && v.kind == w.kind);
                assert(v.kind == PICO8_VALUE_FUNCTION || v.as.ref == w.as.ref);
            }
        }
        if (raw_read(b, "ii", &v)) {
            assert(raw_read(b, "lq", &v) && v.kind == PICO8_VALUE_REF);
            assert(pico8_mv(b, "lq", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &w) &&
                   w.kind == PICO8_VALUE_REF && w.as.ref == v.as.ref);
        }
    }
    check_count("members by name: projects", k, GB_COUNT);
    check("members by name: raw presence", h, RAW_PRESENCE_HASH);
    check_count("members by name: il string", il_string, IL_STRING_COUNT);
    check_count("members by name: il false", il_false, IL_FALSE_COUNT);
    check_count("members by name: il absent", il_absent, IL_ABSENT_COUNT);

    {
        void *ordinary;
        int rank;
        void *a;
        int seen = 0;

        start_game();
        ordinary = project_with(NULL, 0);
        rank = gb_rank(ordinary);
        a = new_planet("p911", 10, 12);
        pico8_jo_lz(a, ordinary);
        assert(get_ref(a, "hn") == ordinary && gb_rank(ordinary) == rank);

        a = pico8_cg("jo")->items[0].as.ref;
        pico8_jo_hh(a);
        (void)pico8_jo_ldown(a);
        {
            const pico8_list_t *hv;
            int j;
            hv = pico8_hv();
            for (j = 0; j < hv->count && !seen; j++) {
                void *target;
                if (hv->items[j].kind != PICO8_VALUE_REF || hv->items[j].as.ref == NULL ||
                    !pico8_entity_get_field(hv->items[j].as.ref, "_jo_fe_b", &v) || v.kind != PICO8_VALUE_REF ||
                    v.as.ref == NULL) {
                    continue;
                }
                target = v.as.ref;
                assert(gb_rank(target) >= 1);
                assert(pico8_hp_od(target) == pico8_hp_od(project(gb_rank(target))));
                seen = 1;
            }
        }
        check_count("members by name: _jo_fe_b seen", seen, 1);
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
    in_fresh_child(test_built_world);
    in_fresh_child(test_harvested_world);
    in_fresh_child(test_hp_is_a_struct);
    in_fresh_child(test_hp_members_by_name);
    in_fresh_child(test_rebuild_and_shutdown);

    printf("test_pico8_hp: all tests passed\n");
    return 0;
}
