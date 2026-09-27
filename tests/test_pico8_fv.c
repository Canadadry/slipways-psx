/* The help screen `fv`, built by its only road (`pico8__init()` on the
 * title screen, `pico8_fv_new(NULL)`), read only through the dual-form field
 * API and `pico8_fv_default()`/`pico8_g__default()`
 * for spec defaults, and driven through the real `mv`/`dc`/`dg`/`lj`. Its
 * methods are compared to the public functions. The frozen values were
 * taken on the code where `fv` was still a class instance and must not be
 * re-taken. */

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
#include "../src/pico8_entity.h"
#include "../src/pico8_entity_geom.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_geom.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_particle.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define RECORDING "tests/fixtures/trading_hub_session_events.json"

#define BUILT_HASH 0x0B80F384u
#define LIFE_HASH 0x3BFADEC0u
#define HOVER_HASH 0x90AFEB68u
#define REPLAY_HASH 0xDBEC801Au
#define REPLAY_EXIT 114

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

/* fv's 22 spec defaults, in spec order; then the 6 of g_ it does not
 * override. */
static const char *const own[] = {"fa", "ra", "hi", "ca", "gq", "eo", "ff", "fc", "k_", "qo", "ro",
                                  "qg", "q_", "pn", "du", "py", "pk", "ki", "kq", "ql", "fg", "rb"};
static const char *const inherited[] = {"nu", "fx", "dq", "od", "fp", "dw"};
static const char *const tables[] = {"ra", "hi", "gq", "k_", "kq", "rb"};
static const char *const absent[] = {"m_", "ep", "p", "et", "p_", "hk", "hb", "ph", "ldown", "fo", "ma"};

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

static void *class_ref(int (*default_of)(const char *, pico8_value_t *), const char *name) {
    pico8_value_t v;
    assert(default_of(name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

static pico8_value_function_t method(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_FUNCTION);
    return v.as.function;
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

static uint32_t fold_value(uint32_t h, pico8_value_t v);

/* A table's entries, in order, nested tables included. */
static uint32_t fold_table(uint32_t h, const pico8_ob_table_t *t) {
    int i;

    assert(pico8_type_tag_of(t) == PICO8_TYPE_TAG_OB_TABLE);
    for (i = 0; i < t->count; i++) {
        const pico8_ob_key_t *key = &t->entries[i].key;
        h = key->kind == PICO8_OB_KEY_NAME ? fnv_str(h, key->name) : fnv(h, (uint32_t)key->index);
        h = fold_value(h, t->entries[i].value);
    }
    return fnv(h, (uint32_t)t->count);
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
        if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE) {
            return fold_table(fnv(h, 0x7Au), v.as.ref);
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

static uint32_t fold_vec(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    const pico8_vec_t *p;
    if (!lookup(e, name, &v)) {
        return fnv(h, 0xDEADu);
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    p = v.as.ref;
    return fnv(fnv(h, (uint32_t)p->x), (uint32_t)p->y);
}

static uint32_t fold_box(uint32_t h, const void *e, const char *name) {
    const pico8_qz_box_t *b = get_ref(e, name);
    assert(b != NULL);
    h = fnv(fnv(h, (uint32_t)b->xl), (uint32_t)b->yt);
    return fnv(fnv(h, (uint32_t)b->xr), (uint32_t)b->yb);
}

/* The item count of a list field, -1 when absent. */
static int size_of(const void *e, const char *name) {
    pico8_value_t v;
    if (!lookup(e, name, &v)) {
        return -1;
    }
    assert(v.kind == PICO8_VALUE_REF && pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST);
    return ((const pico8_list_t *)v.as.ref)->count;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static void check_int(const char *label, int got, int want) {
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

static pico8_value_t vec_ref(int x, int y) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
    return pico8_value_ref(v);
}

static pico8_value_t call(void *e, const char *name, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, argc, arg0, arg1, &out));
    return out;
}

static void start_title(const char *events) {
    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_cheat_init(&cheat);
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

/* The help, whatever its form: `g_`'s `nu` and `fa`=10 (a probe has 2). */
static int is_help(const void *e) {
    pico8_value_t fa;
    return e != NULL && get_ref(e, "nu") == class_ref(pico8_g__default, "nu") && lookup(e, "fa", &fa) &&
           fa.kind == PICO8_VALUE_FIXED && fa.as.fixed == pico8_fixed_from_int(10);
}

static void *the_help(void) {
    const pico8_list_t *hv = pico8_hv();
    void *found = NULL;
    int i;

    for (i = 0; i < hv->count; i++) {
        if (is_help(hv->items[i].as.ref)) {
            assert(found == NULL);
            found = hv->items[i].as.ref;
        }
    }
    assert(found != NULL);
    return found;
}

static const pico8_ob_table_t *bo_row(int i) {
    pico8_value_t row;
    assert(pico8_ob_table_get_index(pico8_bo(), i, &row) && row.kind == PICO8_VALUE_REF);
    return row.as.ref;
}

static int in_layer(int layer, const void *e) {
    pico8_value_t ic;
    pico8_value_t bucket;

    if (!pico8_entity_get_field(pico8_ok(), "ic", &ic) || ic.kind != PICO8_VALUE_REF || ic.as.ref == NULL) {
        return 0;
    }
    if (!pico8_ob_table_get_index(ic.as.ref, layer, &bucket) || bucket.kind != PICO8_VALUE_REF) {
        return 0;
    }
    return listed(bucket.as.ref, e);
}

/* The title's help as `fv:pq`, `f_`, `lt` left it: its 22 defaults and the
 * 6 of g_, their tables' contents and identity, `me`, `t`, its empty `qy`;
 * no name only read or written elsewhere; its methods; the 4 buttons `pz`
 * built before it; where `lt` registered it; `bo` stamped, `ok.le` emptied. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *hv;
    void *fv;
    int rank;
    size_t i;
    int k;

    start_title(RECORDING);
    fv = the_help();
    hv = pico8_hv();

    assert(get_fixed(fv, "fa") == pico8_fixed_from_int(10));
    assert(get_fixed(fv, "ki") == pico8_fixed_from_int(17));
    assert(get_fixed(fv, "ca") == pico8_fixed_from_int(11));
    assert(get_ref(fv, "nu") == class_ref(pico8_g__default, "nu"));
    for (i = 0; i < sizeof own / sizeof own[0]; i++) {
        assert(has(fv, own[i]));
        h = fold_field(h, fv, own[i]);
    }
    for (i = 0; i < sizeof inherited / sizeof inherited[0]; i++) {
        assert(has(fv, inherited[i]));
        h = fold_field(h, fv, inherited[i]);
    }
    h = fold_vec(h, fv, "ra");
    h = fold_vec(h, fv, "hi");
    h = fold_vec(h, fv, "gq");
    h = fold_box(h, fv, "kq");
    for (i = 0; i < sizeof tables / sizeof tables[0]; i++) {
        assert(get_ref(fv, tables[i]) == class_ref(pico8_fv_default, tables[i]));
    }
    assert(get_ref(fv, "k_") != class_ref(pico8_g__default, "k_"));
    h = fold_field(h, fv, "me");
    h = fold_field(h, fv, "t");
    h = fnv(h, (uint32_t)size_of(fv, "qy"));
    assert(size_of(fv, "qy") == 0);
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(fv, absent[i]));
    }

    assert(method(fv, "kj") == pico8_fv_kj_dispatch);
    assert(method(fv, "ev") == pico8_dl_dispatch);
    assert(method(fv, "mm") == pico8_g__mm);
    assert(method(fv, "hq") == pico8_bv_dispatch);
    assert(method(fv, "ey") == pico8_d__dispatch);
    assert(method(fv, "hd") != NULL);

    rank = rank_in(hv, fv);
    h = fnv(h, (uint32_t)rank);
    h = fnv(h, (uint32_t)hv->count);
    assert(rank >= 4);
    for (k = 0; k < 4; k++) {
        const void *b = hv->items[rank - 4 + k].as.ref;
        pico8_vec_t qf;
        assert(pico8_type_tag_of(b) == PICO8_TYPE_TAG_STRUCT &&
               ((const pico8_struct_header_t *)b)->cls == pico8_js_struct_class());
        assert(get_fixed(b, "fa") == pico8_fixed_from_int(11));
        qf = *(const pico8_vec_t *)get_ref(b, "qf");
        assert(qf.x == pico8_fixed_from_int(34) && qf.y == pico8_fixed_from_int(56 + 11 * k));
        h = fold_vec(h, b, "qf");
        h = fold_vec(h, b, "ra");
        h = fold_field(h, b, "fa");
    }
    for (i = 0; i < 5; i++) {
        static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
        h = fnv(h, (uint32_t)rank_in(pico8_cv(bm[i]), fv));
    }
    assert(listed(pico8_cv("kj"), fv) && listed(pico8_cv("ev"), fv) && listed(pico8_cv("hd"), fv));
    assert(!listed(pico8_cv("hk"), fv) && !listed(pico8_cv("hb"), fv));
    h = fnv(h, (uint32_t)rank_in(pico8_cg("g_"), fv));
    h = fnv(h, (uint32_t)rank_in(pico8_cg("prober"), fv));
    assert(listed(pico8_cg("g_"), fv) && listed(pico8_cg("prober"), fv));

    for (k = 1; k <= 4; k++) {
        pico8_value_t my;
        assert(pico8_ob_table_get_name(bo_row(k), "my", &my));
        assert(pico8_value_equal(my, pico8_eu_field_value()));
    }
    assert(size_of(pico8_ok(), "le") == 0);
    check("built", h, BUILT_HASH);
}

/* `dc("kj")` x3 with the screen and `qy`; `dc("ev")`; `dg` x3; `hd` in and
 * out of `kq`, `rp` retained since `fa` >= 10. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *fv;
    int i;

    start_title(RECORDING);
    fv = the_help();
    for (i = 0; i < 3; i++) {
        pico8_dc("kj");
        h = fnv(h, screen_hash());
        h = fnv(h, (uint32_t)size_of(fv, "qy"));
    }
    pico8_dc("ev");
    h = fnv(h, screen_hash());
    for (i = 0; i < 3; i++) {
        pico8_dg();
        h = fold_field(h, fv, "t");
        h = fold_field(h, fv, "me");
        assert(listed(pico8_hv(), fv));
    }
    assert(pico8_value_equal(call(fv, "hd", 2, vec_ref(64, 10), vec_ref(64, 112)), pico8_value_bool(1)));
    assert(pico8_value_equal(call(fv, "hd", 2, vec_ref(64, 112), vec_ref(64, 10)), pico8_value_bool(0)));
    assert(pico8_value_equal(call(fv, "hd", 2, vec_ref(24, 106), vec_ref(24, 106)), pico8_value_bool(1)));
    assert(pico8_value_equal(call(fv, "hd", 2, vec_ref(23, 106), vec_ref(23, 106)), pico8_value_bool(0)));
    h = fnv(h, (uint32_t)size_of(fv, "qy"));
    check("life", h, LIFE_HASH);
}

static pico8_input_frame_t g_mouse;

static void mouse_at(int x, int y) {
    memset(&g_mouse, 0, sizeof g_mouse);
    g_mouse.mouse_x = x;
    g_mouse.mouse_y = y;
    input.frames = &g_mouse;
    input.frame_count = 1;
    input.current_frame = 0;
    pico8_dispatch_set_input(&input);
    pico8_dg();
}

/* The mouse over "need help?": `ok.o_`, `m_`, `fg` poked at 0x5f80, the
 * help in layer `fa` of `ic`, the cursor sprite `ql`; then out, `m_` false. */
static void test_hover(void) {
    uint32_t h = 2166136261u;
    void *fv;
    uint32_t drawn;

    start_title(RECORDING);
    fv = the_help();
    mouse_at(64, 112);
    pico8_ui_lj(&mem, pico8_ok());
    assert(get_ref(pico8_ok(), "o_") == fv);
    assert(bool_state(fv, "m_") == 2);
    check_int("hover 0x5f80", pico8_peek(&mem, 0x5f80), 5);
    assert(in_layer(10, fv));
    h = fnv(h, bool_state(fv, "m_"));
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));

    pico8_cls(&mem);
    pico8_ui_ev(&gfx, &mem, pico8_ok());
    drawn = screen_hash();
    pico8_cls(&mem);
    pico8_spr_default(&gfx, &mem, 11, 64, 112);
    assert(drawn == screen_hash());
    h = fnv(h, drawn);

    mouse_at(64, 10);
    pico8_ui_lj(&mem, pico8_ok());
    assert(get_ref(pico8_ok(), "o_") != fv);
    assert(bool_state(fv, "m_") == 1);
    assert(!in_layer(10, fv));
    h = fnv(h, bool_state(fv, "m_"));
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    check("hover", h, HOVER_HASH);
}

/* The recording from the title until the help leaves `hv`: per frame its
 * rank, `t`, `qy`'s size and `m_`; the exit frame; the screen then. */
static void test_replay(void) {
    uint32_t h = 2166136261u;
    void *fv;
    int exit_frame = -1;
    int i;

    start_title(RECORDING);
    fv = the_help();
    for (i = 0; i < input.frame_count; i++) {
        play_frame(i);
        if (!listed(pico8_hv(), fv)) {
            exit_frame = i;
            break;
        }
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), fv));
        h = fold_field(h, fv, "t");
        h = fnv(h, (uint32_t)size_of(fv, "qy"));
        h = fnv(h, bool_state(fv, "m_"));
    }
    assert(!listed(pico8_cg("g_"), fv) && !listed(pico8_cv("kj"), fv) && !listed(pico8_cv("hd"), fv));
    h = fnv(h, screen_hash());
    check_int("replay exit", exit_frame, REPLAY_EXIT);
    check("replay", h, REPLAY_HASH);
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

static void write_fa(void *e) {
    write_fixed(e, "fa");
}

static void write_hi(void *e) {
    write_fixed(e, "hi");
}

static void write_rb(void *e) {
    write_fixed(e, "rb");
}

static void write_ql(void *e) {
    write_fixed(e, "ql");
}

static void write_nu(void *e) {
    write_fixed(e, "nu");
}

static void write_fo(void *e) {
    write_fixed(e, "fo");
}

static void write_me(void *e) {
    write_fixed(e, "me");
}

static void build_with_named_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "fa", pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_fv_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_fv_new(b);
}

/* The title's help is a struct whose descriptor shares `g_`'s 4 inherited
 * entries and holds its own `kj`, `hd`; it takes exactly `m_` and `g_`'s 5
 * written names, and no construction key. Its class keeps only its spec. */
static void test_fv_is_a_struct(void) {
    static const char *const written[] = {"m_", "qy", "ra", "p", "et", "p_"};
    const pico8_struct_class_t *cls = pico8_fv_struct_class();
    const pico8_struct_class_t *g_ = pico8_g__struct_class();
    void *fv;
    pico8_value_t v;
    size_t i;
    int j;

    start_title(RECORDING);
    fv = the_help();
    assert(pico8_type_tag_of(fv) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)fv)->cls == cls);
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        if (j == PICO8_METHOD_HQ || j == PICO8_METHOD_EY || j == PICO8_METHOD_EV || j == PICO8_METHOD_MM) {
            assert(cls->methods[j] != NULL && cls->methods[j] == g_->methods[j]);
        } else if (j == PICO8_METHOD_KJ) {
            assert(cls->methods[j] == pico8_fv_kj_dispatch);
        } else if (j == PICO8_METHOD_HD) {
            assert(cls->methods[j] != NULL);
        } else {
            assert(cls->methods[j] == NULL);
        }
    }
    assert(cls->field != NULL && cls->set_field != NULL);
    assert(cls->field != g_->field && cls->set_field != g_->set_field);
    assert(pico8_fv_ready());
    assert(!pico8_fv_default("hd", &v));
    assert(!pico8_fv_default("kj", &v));

    must_abort(write_fa, fv);
    must_abort(write_hi, fv);
    must_abort(write_rb, fv);
    must_abort(write_ql, fv);
    must_abort(write_nu, fv);
    must_abort(write_fo, fv);
    must_abort(write_me, fv);
    must_abort(build_with_named_key, NULL);
    must_abort(build_with_positional_key, NULL);
    for (i = 0; i < sizeof written / sizeof written[0]; i++) {
        pico8_entity_set_field(fv, written[i], pico8_value_fixed(pico8_fixed_from_int((int)i + 1)));
        assert(lookup(fv, written[i], &v) && v.kind == PICO8_VALUE_FIXED &&
               v.as.fixed == pico8_fixed_from_int((int)i + 1));
    }
}

static void call_f_(void *e) {
    pico8_f_(e);
}

static void call_ez(void *e) {
    pico8_ez(e, pico8_fixed_from_int(1));
}

static void call_ei(void *e) {
    pico8_ei(&gfx, &mem, e);
}

static void call_g__mm(void *e) {
    (void)pico8_g__mm(e, 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
}

static void call_bv(void *e) {
    pico8_bv(e, vec_ref(1, 1), vec_ref(2, 2));
}

static void call_dl(void *e) {
    pico8_dl(&gfx, &mem, e);
}

static void call_d_(void *e) {
    (void)pico8_d_(e);
}

static void call_gk(void *e) {
    (void)pico8_gk(e, vec_ref(1, 1), vec_ref(2, 2));
}

static void call_na(void *e) {
    pico8_na(e, pico8_fixed_from_int(1));
}

/* `g_`'s spec holds no method, and `g_`'s functions take a struct or NULL
 * as `self`: an ob table stops each of them. */
static void test_g__closed(void) {
    static const char *const installs[] = {"hq", "ey", "ev", "kj", "mm"};
    static void (*const fns[])(void *) = {call_f_, call_ez, call_ei, call_g__mm, call_bv,
                                          call_dl, call_d_, call_gk, call_na};
    pico8_ob_table_t *table = pico8_ob_table_new();
    pico8_vec_t ra = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
    pico8_value_t v;
    size_t i;

    start_title(RECORDING);
    assert(pico8_g__ready());
    for (i = 0; i < sizeof installs / sizeof installs[0]; i++) {
        assert(!pico8_g__default(installs[i], &v));
    }
    pico8_ob_table_set_name(table, "ra", pico8_value_ref(&ra));
    pico8_ob_table_set_name(table, "fa", pico8_value_fixed(pico8_fixed_from_int(10)));
    for (i = 0; i < sizeof fns / sizeof fns[0]; i++) {
        must_abort(fns[i], table);
        fns[i](NULL);
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
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_hover);
    in_fresh_child(test_replay);
    in_fresh_child(test_fv_is_a_struct);
    in_fresh_child(test_g__closed);

    printf("test_pico8_fv: all tests passed\n");
    return 0;
}
