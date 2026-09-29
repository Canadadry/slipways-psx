/* The help screen `fv`, built by its only road (`pico8__init()` on the
 * title screen, `pico8_fv_new()`), read only through the dual-form field
 * API, its defaults compared to `pico8_fv_start`, and driven through the
 * real `mv`/`dc`/`dg`/`lj`. Its
 * methods are compared to the public functions. No events file. */

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
#include "../src/pico8_entity_geom.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_geom.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_particle.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"


#define BUILT_HASH 0x0B80F384u
#define LIFE_HASH 0x3BFADEC0u
#define HOVER_HASH 0x90AFEB68u
#define FV_MEMBERS_HASH 0xE0819DECu

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

/* fv's 22 spec defaults, in spec order; then the 6 of g_ it does not
 * override. */
static const char *const own[] = {"fa", "ra", "hi", "ca", "gq", "eo", "ff", "fc", "k_", "qo", "ro",
                                  "qg", "q_", "pn", "du", "py", "pk", "ki", "kq", "ql", "fg", "rb"};
static const char *const inherited[] = {"nu", "fx", "dq", "od", "fp", "dw"};
static const char *const absent[] = {"m_", "ep", "p", "et", "p_", "fo", "ma"};

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

/* A tag sequence: each position, then its string, then the count. */
static uint32_t fold_tags(uint32_t h, const pico8_str_seq_t *s) {
    int i;

    for (i = 0; i < s->count; i++) {
        h = fnv(h, (uint32_t)(i + 1));
        h = fold_value(h, pico8_value_string(s->items[i]));
    }
    return fnv(h, (uint32_t)s->count);
}

/* An integer sequence: each position, then its value, then the count. */
static uint32_t fold_seq(uint32_t h, const pico8_int_seq_t *s) {
    int i;

    for (i = 0; i < s->count; i++) {
        h = fnv(h, (uint32_t)(i + 1));
        h = fold_value(h, pico8_value_fixed(pico8_fixed_from_int(s->items[i])));
    }
    return fnv(h, (uint32_t)s->count);
}

/* A generated row: items (a nested row folded the same way), then `c`,
 * then `fn`, then the entry count. */
static uint32_t fold_row(uint32_t h, const pico8_jb_row_t *row) {
    int i;

    for (i = 0; i < row->count; i++) {
        const pico8_jb_item_t *item = &row->items[i];

        h = fnv(h, (uint32_t)(i + 1));
        if (item->row != NULL) {
            h = fold_row(fnv(h, 0x7Au), item->row);
        } else if (item->s != NULL) {
            h = fold_value(h, pico8_value_string(item->s));
        } else if (item->is_false) {
            h = fold_value(h, pico8_value_bool(0));
        } else {
            h = fold_value(h, pico8_value_fixed(item->is_fixed ? item->fixed : pico8_fixed_from_int(item->n)));
        }
    }
    if (row->has_c) {
        h = fold_value(fnv_str(h, "c"), pico8_value_fixed(pico8_fixed_from_int(row->c)));
    }
    if (row->fn != NULL) {
        h = fold_value(fnv_str(h, "fn"), pico8_value_string(row->fn));
    }
    return fnv(h, (uint32_t)(row->count + (row->has_c != 0) + (row->fn != NULL)));
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
        if (v.as.ref == (const void *)&pico8_g__nu_tags) {
            return fold_tags(fnv(h, 0x7Au), v.as.ref);
        }
        if (v.as.ref == (const void *)&pico8_fv_k__seq) {
            return fold_seq(fnv(h, 0x7Au), v.as.ref);
        }
        if (v.as.ref == (const void *)&pico8_fv_rb_row) {
            return fold_row(fnv(h, 0x7Au), v.as.ref);
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

static const pico8_struct_class_t *cls_of(const void *e) {
    return ((const pico8_struct_header_t *)e)->cls;
}

static void start_title(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
}

/* The help, whatever its form: `g_`'s `nu` and `fa`=10 (a probe has 2). */
static int is_help(const void *e) {
    pico8_value_t fa;
    return e != NULL && get_ref(e, "nu") == &pico8_g__nu_tags && lookup(e, "fa", &fa) &&
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

static int in_layer(int layer, const void *e) {
    pico8_value_t ic;

    if (!pico8_entity_get_field(pico8_ok(), "ic", &ic) || ic.kind != PICO8_VALUE_REF || ic.as.ref == NULL) {
        return 0;
    }
    return listed(&((const pico8_ui_ic_t *)ic.as.ref)->layers[layer], e);
}

/* The title's help as `fv:pq`, `f_`, `lt` left it: its 22 defaults and the
 * 6 of g_, their tables' contents and identity, `me`, `t`, its empty `qy`;
 * no name only read or written elsewhere; its methods; the 4 buttons `pz`
 * built before it, each carrying the `my` stamped on its `bo` row; where
 * `lt` registered it; `ok.le` emptied. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *hv;
    void *fv;
    int rank;
    size_t i;
    int k;

    start_title();
    fv = the_help();
    hv = pico8_hv();

    assert(get_fixed(fv, "fa") == pico8_fixed_from_int(10));
    assert(get_fixed(fv, "ki") == pico8_fixed_from_int(17));
    assert(get_fixed(fv, "ca") == pico8_fixed_from_int(11));
    assert(get_ref(fv, "nu") == &pico8_g__nu_tags);
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
    assert(get_ref(fv, "ra") == &pico8_fv_start.ra);
    assert(get_ref(fv, "hi") == &pico8_fv_start.hi);
    assert(get_ref(fv, "gq") == &pico8_fv_start.gq);
    assert(get_ref(fv, "kq") == &pico8_fv_start.kq);
    assert(get_ref(fv, "k_") == &pico8_fv_k__seq);
    assert(get_ref(fv, "k_") != &pico8_g__k__seq);
    assert(get_ref(fv, "rb") == &pico8_fv_rb_row);
    h = fold_field(h, fv, "me");
    h = fold_field(h, fv, "t");
    h = fnv(h, (uint32_t)size_of(fv, "qy"));
    assert(size_of(fv, "qy") == 0);
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(fv, absent[i]));
    }
    assert(cls_of(fv)->hk == NULL && cls_of(fv)->hb == NULL && cls_of(fv)->states[PICO8_STATE_PH] == NULL &&
           cls_of(fv)->ldown == NULL);

    assert(cls_of(fv)->kj == pico8_fv_kj_dispatch);
    assert(cls_of(fv)->ev == pico8_dl_dispatch);
    assert(method(fv, "mm") == pico8_g__mm);
    assert(method(fv, "hq") == pico8_bv_dispatch);
    assert(method(fv, "ey") == pico8_d__dispatch);
    assert(cls_of(fv)->hd != NULL);

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

    for (k = 0; k < 4; k++) {
        pico8_value_t my;
        assert(lookup(hv->items[rank - 4 + k].as.ref, "my", &my));
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

    start_title();
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
    assert(cls_of(fv)->hd(fv, vec_ref(64, 10).as.ref, vec_ref(64, 112).as.ref) == 1);
    assert(cls_of(fv)->hd(fv, vec_ref(64, 112).as.ref, vec_ref(64, 10).as.ref) == 0);
    assert(cls_of(fv)->hd(fv, vec_ref(24, 106).as.ref, vec_ref(24, 106).as.ref) == 1);
    assert(cls_of(fv)->hd(fv, vec_ref(23, 106).as.ref, vec_ref(23, 106).as.ref) == 0);
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

    start_title();
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

/* `eu`, as a difficulty button starts it, takes the help out of `hv`,
 * `cg.g_` and `cv.kj`/`cv.hd`. */
/* A particle of the help is drawn in the help's own `k_`: at `l`=6.5, less
 * the frame's 0.14, index 6 is colour 9, where `g_`'s five have none. A
 * button built just before it has neither `k_` nor `qy`: nothing drawn. */
static void test_particle_takes_the_help_palette(void) {
    static pico8_qy_t particle;
    pico8_list_t *qy;
    pico8_vec_t ra;
    void *fv;
    void *button;
    uint32_t before;
    int at;

    start_title();
    fv = the_help();
    button = pico8_hv()->items[rank_in(pico8_hv(), fv) - 1].as.ref;
    assert(!has(button, "k_") && !has(button, "qy") && get_fixed(button, "fa") >= pico8_fixed_from_int(10));
    before = screen_hash();
    pico8_ei(&gfx, &mem, button);
    assert(screen_hash() == before);
    ra = *(const pico8_vec_t *)get_ref(fv, "ra");
    particle.p = pico8_vec_v(0, 0);
    particle.v = pico8_vec_v(0, 0);
    particle.has_a = 0;
    particle.l = PICO8_FIXED_LIT(0x00068000, 6.5);
    qy = get_ref(fv, "qy");
    qy->count = 0;
    pico8_list_add(qy, pico8_value_ref(&particle));
    pico8_camera_reset(&gfx);
    at = PICO8_ADDR_SCREEN + pico8_fixed_flr_int(ra.y) * PICO8_SCREEN_WIDTH + pico8_fixed_flr_int(ra.x);
    pico8_poke(&mem, at, 3);
    pico8_ei(&gfx, &mem, fv);
    assert(pico8_peek(&mem, at) == pico8_fv_k__seq.items[5] && pico8_fv_k__seq.items[5] == 9);
}

static void test_eu_retires_the_help(void) {
    void *fv;

    start_title();
    fv = the_help();
    pico8_eu(NULL);
    assert(!listed(pico8_hv(), fv));
    assert(!listed(pico8_cg("g_"), fv) && !listed(pico8_cv("kj"), fv) && !listed(pico8_cv("hd"), fv));
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

/* The title's help is a struct whose descriptor shares `g_`'s 4 inherited
 * entries and holds its own `kj`, `hd`; a write of a name that is neither
 * `m_` nor one of `g_`'s 5 stops. Ready from its init to its shutdown. */
static void test_fv_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_fv_struct_class();
    const pico8_struct_class_t *g_ = pico8_g__struct_class();
    void *fv;
    int j;

    start_title();
    fv = the_help();
    assert(pico8_type_tag_of(fv) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)fv)->cls == cls);
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        if (j == PICO8_METHOD_HQ || j == PICO8_METHOD_EY || j == PICO8_METHOD_MM) {
            assert(cls->methods[j] != NULL && cls->methods[j] == g_->methods[j]);
        } else {
            assert(cls->methods[j] == NULL);
        }
    }
    assert(cls->ev != NULL && cls->ev == g_->ev);
    assert(cls->kj == pico8_fv_kj_dispatch && cls->hd != NULL);
    assert(cls->hk == NULL && cls->hb == NULL && cls->has_hb == NULL && cls->ldown == NULL && cls->rdown == NULL &&
           cls->gv == NULL);
    for (j = 0; j < PICO8_STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(cls->field != NULL && cls->set_field != NULL);
    assert(cls->field != g_->field && cls->set_field != g_->set_field);
    assert(pico8_fv_ready());

    must_abort(write_fa, fv);
    must_abort(write_hi, fv);
    must_abort(write_rb, fv);
    must_abort(write_ql, fv);
    must_abort(write_nu, fv);
    must_abort(write_fo, fv);
    must_abort(write_me, fv);
    pico8_help_shutdown();
    assert(!pico8_fv_ready());
    pico8_help_init();
    assert(pico8_fv_ready());
}

static const char *const fv_names[] = {"hi", "ca", "gq", "eo", "ff", "kq", "ql", "fg", "rb", "m_"};

static pico8_value_t zero_union(pico8_value_kind_t kind) {
    pico8_value_t v;

    memset(&v, 0, sizeof v);
    v.kind = kind;
    return v;
}

/* The raw reader and the name reader agree; absent leaves both witnesses
 * untouched. */
static int read_both(const void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(0x1234);
    pico8_value_t named = pico8_value_fixed(0x1234);
    int got_raw = pico8_fv_struct_class()->field(e, name, &raw);
    int got_named = pico8_entity_get_field(e, name, &named);

    assert(got_raw == got_named);
    if (!got_raw) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        assert(named.kind == PICO8_VALUE_FIXED && named.as.fixed == 0x1234);
        return 0;
    }
    assert(pico8_value_equal(raw, named));
    *out = raw;
    return 1;
}

static void assert_same(const void *e, const char *name, pico8_value_t want) {
    pico8_value_t v;

    assert(read_both(e, name, &v) && pico8_value_equal(v, want));
}

static void assert_absent(const void *e, const char *name) {
    pico8_value_t v;

    assert(!read_both(e, name, &v));
}

/* The 10 names by both readers: presence, kind, value; vectors and the box
 * by content, never by address. */
static uint32_t fold_fv_members(uint32_t h, const void *e) {
    size_t i;

    for (i = 0; i < sizeof fv_names / sizeof fv_names[0]; i++) {
        const char *name = fv_names[i];
        pico8_value_t v;

        if (!read_both(e, name, &v)) {
            h = fnv(h, 0u);
            continue;
        }
        h = fnv(fnv(h, 1u), (uint32_t)v.kind);
        if (strcmp(name, "hi") == 0 || strcmp(name, "gq") == 0) {
            h = fold_vec(h, e, name);
        } else if (strcmp(name, "kq") == 0) {
            h = fold_box(h, e, name);
        } else {
            h = fold_value(h, v);
        }
    }
    return h;
}

/* A fresh help: its 3 tables `pico8_fv_start`'s own, `rb` the generated
 * row, its 5 fixed at the cart's values, `m_` absent. */
static uint32_t fold_fresh_fv(uint32_t h, const void *e) {
    assert_same(e, "hi", pico8_value_ref((void *)&pico8_fv_start.hi));
    assert_same(e, "gq", pico8_value_ref((void *)&pico8_fv_start.gq));
    assert_same(e, "kq", pico8_value_ref((void *)&pico8_fv_start.kq));
    assert_same(e, "rb", pico8_value_ref((void *)&pico8_fv_rb_row));
    assert_same(e, "ca", pico8_value_fixed(pico8_fixed_from_int(11)));
    assert_same(e, "eo", pico8_value_fixed(pico8_fixed_from_int(-1)));
    assert_same(e, "ff", pico8_value_fixed(pico8_fixed_from_int(1)));
    assert_same(e, "ql", pico8_value_fixed(pico8_fixed_from_int(11)));
    assert_same(e, "fg", pico8_value_fixed(pico8_fixed_from_int(5)));
    assert_absent(e, "m_");
    return fold_fv_members(h, e);
}

static void write_m_(void *e, pico8_value_t v) {
    assert(pico8_fv_struct_class()->set_field(e, "m_", v) == 1);
    pico8_entity_set_field(e, "m_", v);
}

/* `m_` takes true, false (a zeroed union too) and nil; nil puts out any
 * state. The 9 defaults and any other name are no writer's: the raw writer
 * returns 0 and touches nothing. */
static uint32_t fold_written_by_name(uint32_t h, void *e) {
    static const char *const unwritten[] = {"hi", "ca", "gq", "eo", "ff", "kq", "ql", "fg",
                                            "rb", "",   "M_", "m_ ", "m",  "has_m_"};
    size_t i;

    write_m_(e, pico8_value_bool(1));
    assert_same(e, "m_", pico8_value_bool(1));
    h = fold_fv_members(h, e);
    write_m_(e, pico8_value_bool(0));
    assert_same(e, "m_", pico8_value_bool(0));
    h = fold_fv_members(h, e);
    write_m_(e, pico8_value_bool(1));
    write_m_(e, zero_union(PICO8_VALUE_BOOL));
    assert_same(e, "m_", pico8_value_bool(0));
    h = fold_fv_members(h, e);
    write_m_(e, pico8_value_bool(1));
    write_m_(e, pico8_value_ref(NULL));
    assert_absent(e, "m_");
    h = fold_fv_members(h, e);
    write_m_(e, pico8_value_bool(0));
    write_m_(e, pico8_value_ref(NULL));
    assert_absent(e, "m_");
    write_m_(e, pico8_value_ref(NULL));
    assert_absent(e, "m_");
    h = fold_fv_members(h, e);

    write_m_(e, pico8_value_bool(1));
    for (i = 0; i < sizeof unwritten / sizeof unwritten[0]; i++) {
        uint32_t before = fold_fv_members(2166136261u, e);
        int got = pico8_fv_struct_class()->set_field(e, unwritten[i], pico8_value_fixed(pico8_fixed_from_int(9)));
        assert(got == 0);
        assert(fold_fv_members(2166136261u, e) == before);
        h = fnv(h, (uint32_t)got);
    }
    assert_same(e, "m_", pico8_value_bool(1));
    return fold_fv_members(h, e);
}

/* The title's help and a `pico8_fv_new()`, by name only: fresh; `gw`
 * hovering and leaving the title's; every write of `m_`; the names no
 * writer takes; strangers to the reader. */
static void test_fv_members_by_name(void) {
    static const char *const strangers[] = {"", "h", "hi ", "HI", "m__", "has_m_", "rb_", "kq2"};
    uint32_t h = 2166136261u;
    void *fv[2];
    pico8_value_t v;
    size_t i;
    int k;

    start_title();
    fv[0] = the_help();
    h = fold_fresh_fv(h, fv[0]);

    mouse_at(64, 112);
    pico8_ui_lj(&mem, pico8_ok());
    assert(get_ref(pico8_ok(), "o_") == fv[0]);
    assert_same(fv[0], "m_", pico8_value_bool(1));
    h = fold_fv_members(h, fv[0]);
    mouse_at(64, 10);
    pico8_ui_lj(&mem, pico8_ok());
    assert_same(fv[0], "m_", pico8_value_bool(0));
    h = fold_fv_members(h, fv[0]);

    fv[1] = pico8_fv_new();
    assert(fv[1] != fv[0]);
    h = fold_fresh_fv(h, fv[1]);

    for (k = 0; k < 2; k++) {
        h = fold_written_by_name(h, fv[k]);
        for (i = 0; i < sizeof strangers / sizeof strangers[0]; i++) {
            assert(!read_both(fv[k], strangers[i], &v));
            h = fnv(h, 0u);
        }
    }
    check("fv members", h, FV_MEMBERS_HASH);
}

static pico8_value_t g_off_row;

static void write_off_row(void *e) {
    pico8_entity_set_field(e, "m_", g_off_row);
}

/* `m_` takes nil or a boolean only, a zeroed union included. */
static void test_fv_writer_stops_off_row(void) {
    pico8_value_t off[9];
    void *fv[2];
    size_t i;
    int k;

    start_title();
    fv[0] = the_help();
    fv[1] = pico8_fv_new();
    off[0] = pico8_value_fixed(pico8_fixed_from_int(1));
    off[1] = zero_union(PICO8_VALUE_FIXED);
    off[2] = pico8_value_string("x");
    off[3] = zero_union(PICO8_VALUE_STRING);
    off[4] = zero_union(PICO8_VALUE_FUNCTION);
    off[5] = vec_ref(1, 1);
    off[6] = pico8_value_ref((void *)pico8_hv());
    off[7] = pico8_value_ref(pico8_g__new(NULL));
    for (k = 0; k < 2; k++) {
        off[8] = pico8_value_ref(fv[1 - k]);
        for (i = 0; i < sizeof off / sizeof off[0]; i++) {
            g_off_row = off[i];
            must_abort(write_off_row, fv[k]);
        }
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
    (void)pico8_gk(e, vec_ref(1, 1).as.ref, vec_ref(2, 2).as.ref);
}

static void call_na(void *e) {
    pico8_na(e, pico8_fixed_from_int(1));
}

/* `g_`'s functions take a struct or NULL as `self`: a list stops each of
 * them. */
static void test_g__closed(void) {
    static void (*const fns[])(void *) = {call_f_, call_ez, call_ei, call_g__mm, call_bv,
                                          call_dl, call_d_, call_gk, call_na};
    pico8_list_t list;
    size_t i;

    pico8_list_init(&list);
    start_title();
    assert(pico8_g__ready());
    for (i = 0; i < sizeof fns / sizeof fns[0]; i++) {
        must_abort(fns[i], &list);
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
    in_fresh_child(test_particle_takes_the_help_palette);
    in_fresh_child(test_eu_retires_the_help);
    in_fresh_child(test_fv_is_a_struct);
    in_fresh_child(test_fv_members_by_name);
    in_fresh_child(test_fv_writer_stops_off_row);
    in_fresh_child(test_g__closed);

    printf("test_pico8_fv: all tests passed\n");
    return 0;
}
