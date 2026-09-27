/* The research project `hp`, the 34 entries of `gb` built at cart load, read
 * only through pico8_entity_get_field, pico8_mv and `po`/`od`/`jb`/`lq`, then
 * installed on fresh planets by `jo:lz`/`jo:hu` and seen through two short
 * replays. The frozen values were taken on the code where `hp` was still a
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
#include "../src/pico8_class.h"
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
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_display.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_tech_gate.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define BUILT "tests/fixtures/built_planet_panel_events.json"
#define GEOHARVEST "tests/fixtures/geoharvest_burst_events.json"

#define GB_COUNT 34
#define GB_NAMED 26
#define GB_HASH 0x1339FD84u
#define DEFAULTS_HASH 0x6DDA7FDEu
#define FUNCTIONS_HASH 0x538CF904u
#define FUNCTIONS_INVENTED_HASH 0xA64C8C0Fu
#define INSTALL_HASH 0x3158A685u
#define BUILT_REPLAY_HASH 0x7004D233u
#define GEOHARVEST_REPLAY_HASH 0xB3DFE52Du

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

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

static void play_frame(int i) {
    pico8_dispatch_set_input(&input);
    pico8_cheat_step(&cheat, &input);
    pico8__update60();
    pico8__draw();
    if (i < input.frame_count - 1) {
        pico8_input_step(&input);
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
        const pico8_ob_table_t *po = pico8_hp_po(b);
        pico8_ob_table_t *jb = pico8_hp_jb(b);
        pico8_value_t member;
        pico8_value_t via_mv;
        uint32_t lq_h;

        h = fnv(h, (uint32_t)k);
        h = fnv(h, (uint32_t)(po == NULL ? -2 : entry_of(pico8_hc(), po)));
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
    pico8_hy_kt(pico8_fm(), t);
    assert(pico8_oj(pico8_value_string(px)));
}

static void test_functions(void) {
    pico8_value_t il;
    int k;

    start_game(BUILT);
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

    start_game(BUILT);
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

/* At the last frame, each planet of `cg.jo`: the rank in `gb` of its `hn`;
 * then the screen. */
static uint32_t replay(const char *events) {
    uint32_t h = 2166136261u;
    const pico8_list_t *jo;
    int i;

    start_game(events);
    for (i = 0; i < input.frame_count; i++) {
        play_frame(i);
    }
    jo = pico8_cg("jo");
    for (i = 0; jo != NULL && i < jo->count; i++) {
        h = fnv(h, (uint32_t)gb_rank(get_ref(jo->items[i].as.ref, "hn")));
    }
    return fnv(h, screen_hash());
}

static void test_built_replay(void) {
    check("built replay", replay(BUILT), BUILT_REPLAY_HASH);
}

static void test_geoharvest_replay(void) {
    check("geoharvest replay", replay(GEOHARVEST), GEOHARVEST_REPLAY_HASH);
}

/* `gb` rebuilt from `fd` frees the projects it held and gives the same
 * projects; shutdown leaves no class, no `fk`, no `gb`; init again, then a
 * rebuild, gives them once more. */
static void test_rebuild_and_shutdown(void) {
    pico8_research_build_gb(pico8_fd());
    test_gb();

    pico8_research_shutdown();
    assert(pico8_gb() == NULL && pico8_hp_class() == NULL);
    assert(pico8_hp_fk().kind == PICO8_VALUE_REF && pico8_hp_fk().as.ref == NULL);
    pico8_research_shutdown();

    pico8_research_init();
    assert(pico8_hp_class() != NULL && pico8_hp_fk().as.ref != NULL);
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
    assert(pico8_hp_class()->struct_new != NULL && pico8_hp_class()->pq == NULL);
    assert(!pico8_ob_table_get_name(pico8_hp_class()->defaults, "lq", &v));
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

    start_game(BUILT);
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
    in_fresh_child(test_built_replay);
    in_fresh_child(test_geoharvest_replay);
    in_fresh_child(test_hp_is_a_struct);
    in_fresh_child(test_rebuild_and_shutdown);

    printf("test_pico8_hp: all tests passed\n");
    return 0;
}
