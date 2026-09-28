/* The mouse reader `nf` (`je` once `jl()` has run), read only through the
 * dual-form field reader and driven through the real `dg`, in games built
 * directly: no events file. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bg.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define SCRIPTED_TICKS 12
#define SCRIPTED_HASH 0xEB03D064u
#define NF_MEMBERS_HASH 0x8D1F75D8u

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

static int listed(const pico8_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i].kind == PICO8_VALUE_REF && list->items[i].as.ref == e) {
            return 1;
        }
    }
    return 0;
}

static int in_any_cv(const void *e) {
    const char *names[] = {"kj", "ev", "hd", "hk", "hb"};
    int i;

    for (i = 0; i < 5; i++) {
        if (listed(pico8_cv(names[i]), e)) {
            return 1;
        }
    }
    return 0;
}

/* NULL when `je` has no such field or it is not a table. */
static const pico8_ob_table_t *button(const char *bt) {
    pico8_value_t v;

    if (!pico8_entity_get_field(pico8_je(), bt, &v) || v.kind != PICO8_VALUE_REF || v.as.ref == NULL) {
        return NULL;
    }
    assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE);
    return v.as.ref;
}

/* -1 absent, else the boolean. */
static int flag(const char *bt, const char *name) {
    const pico8_ob_table_t *b = button(bt);
    pico8_value_t v;

    if (b == NULL || !pico8_ob_table_get_name(b, name, &v)) {
        return -1;
    }
    assert(v.kind == PICO8_VALUE_BOOL);
    return v.as.boolean;
}

static int ra_of_je(pico8_vec_t *out) {
    pico8_value_t v;

    if (!pico8_entity_get_field(pico8_je(), "ra", &v)) {
        return 0;
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    *out = *(const pico8_vec_t *)v.as.ref;
    return 1;
}

static uint32_t fold_je(uint32_t h) {
    const char *bts[] = {"l", "r"};
    const char *names[] = {"on", "jn", "hm"};
    pico8_vec_t ra;
    int i;
    int j;

    if (ra_of_je(&ra)) {
        h = fnv(h, (uint32_t)ra.x);
        h = fnv(h, (uint32_t)ra.y);
    } else {
        h = fnv(h, 0xDEADu);
    }
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 3; j++) {
            h = fnv(h, (uint32_t)flag(bts[i], names[j]));
        }
    }
    return h;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static pico8_input_frame_t g_frames[64];

/* Points the dispatch input at `n` frames of g_frames, positioned at 0. */
static void script(int n) {
    input.frames = g_frames;
    input.frame_count = n;
    input.current_frame = 0;
    pico8_dispatch_set_input(&input);
}

static void unscript(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
}

static void frame(int i, int x, int y, int buttons) {
    memset(&g_frames[i], 0, sizeof g_frames[i]);
    g_frames[i].mouse_x = x;
    g_frames[i].mouse_y = y;
    g_frames[i].mouse_buttons = buttons;
}

/* `jl()` builds `je`: devkit mouse on, in hv only, `ra` the zero default,
 * `l`/`r` present and still empty. */
static void test_jl_builds_a_mouse_reader(void) {
    const void *je;
    pico8_value_t v;
    pico8_vec_t ra;

    pico8_poke(&mem, 0x5f2d, 0);
    pico8_jl();
    je = pico8_je();
    assert(pico8_peek(&mem, 0x5f2d) == 1);
    assert(listed(pico8_hv(), je) && !in_any_cv(je));
    assert(!pico8_entity_get_field(je, "nu", &v));
    assert(ra_of_je(&ra) && ra.x == 0 && ra.y == 0);
    assert(button("l") != NULL && button("r") != NULL && button("l") != button("r"));
    assert(flag("l", "on") == -1 && flag("l", "jn") == -1 && flag("l", "hm") == -1);
    assert(flag("r", "on") == -1 && flag("r", "jn") == -1 && flag("r", "hm") == -1);
}

/* Left alone, right, middle, left+right, releases, moving each tick. */
static void test_scripted_ticks(void) {
    static const int buttons[SCRIPTED_TICKS] = {0, 1, 1, 0, 2, 2, 4, 0, 3, 1, 2, 0};
    uint32_t h = 2166136261u;
    int i;

    pico8_jl();
    for (i = 0; i < SCRIPTED_TICKS; i++) {
        frame(i, 7 + 9 * i, 120 - 5 * i, buttons[i]);
    }
    script(SCRIPTED_TICKS);
    for (i = 0; i < SCRIPTED_TICKS; i++) {
        pico8_dg();
        h = fold_je(h);
        if (i < SCRIPTED_TICKS - 1) {
            pico8_input_step(&input);
        }
    }
    unscript();
    check("scripted ticks", h, SCRIPTED_HASH);
}

/* Every `nf` shares the two button tables: a left button held across `jl()`
 * gives the new `je` no fresh press. */
static void test_buttons_survive_jl(void) {
    const pico8_ob_table_t *l_before;

    pico8_jl();
    frame(0, 40, 40, 1);
    frame(1, 41, 41, 1);
    frame(2, 42, 42, 1);
    script(3);
    pico8_dg();
    pico8_input_step(&input);
    pico8_dg();
    assert(flag("l", "on") == 1);
    l_before = button("l");

    pico8_jl();
    pico8_input_step(&input);
    pico8_dg();
    unscript();
    assert(button("l") == l_before);
    assert(flag("l", "jn") == 0 && flag("l", "on") == 1);
}

/* Rebuilding the class starts both tables afresh. */
static void test_nf_init_starts_the_tables_afresh(void) {
    pico8_jl();
    frame(0, 10, 10, 1);
    script(1);
    pico8_dg();
    unscript();
    assert(flag("l", "on") == 1);

    pico8_nf_init();
    pico8_jl();
    assert(flag("l", "on") == -1 && flag("r", "on") == -1);
}

/* Before `jl()`, `je` is the field-less placeholder: no buttons, no `ra`,
 * and `ok`'s frame dispatches no press. */
static void test_placeholder_je(void) {
    pico8_ui_t *ok;
    pico8_vec_t ra;
    pico8_value_t v;

    pico8_jl();
    pico8_ui_init();
    assert(button("l") == NULL && button("r") == NULL && !ra_of_je(&ra));
    ok = pico8_ui_new();
    pico8_ui_lj(&mem, ok);
    assert(!pico8_entity_get_field(ok, "lheld", &v) && !pico8_entity_get_field(ok, "rheld", &v));
    assert(pico8_entity_get_field(ok, "qa", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
}

/* The title screen's `je`, then `eu` starting a game with the left button
 * still held: the new `je` sees it held. */
static void test_launch_with_left_held(void) {
    const void *first_je;

    pico8__init();
    first_je = pico8_je();
    frame(0, 64, 64, 1);
    frame(1, 64, 64, 1);
    frame(2, 64, 64, 1);
    script(3);
    pico8_dg();
    pico8_input_step(&input);
    pico8_eu(NULL);
    assert(pico8_je() != first_je);
    pico8_input_step(&input);
    pico8_dg();
    unscript();
    assert(flag("l", "on") == 1);
}

static int is_an_nf_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_nf_struct_class();
}

/* Built by `pico8_nf_new` or through its class, an nf is a struct in hv
 * only; `ph` is its method, `kj` and `p_` are absent, and every nf shares
 * the same `l` and `r`. */
static void test_nf_is_a_struct_sharing_its_buttons(void) {
    void *built;
    void *generic;
    pico8_value_t out;
    pico8_value_t l_built;
    pico8_value_t r_built;
    pico8_value_t l_generic;
    pico8_value_t r_generic;

    pico8_jl();
    built = pico8_nf_new();
    generic = pico8_nf_new();

    assert(is_an_nf_struct(built) && is_an_nf_struct(generic) && is_an_nf_struct(pico8_je()));
    assert(listed(pico8_hv(), built) && !in_any_cv(built));
    assert(listed(pico8_hv(), generic) && !in_any_cv(generic));
    assert(pico8_mv(generic, "ph", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(!pico8_mv(generic, "kj", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(!pico8_mv(generic, "p_", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(pico8_entity_get_field(built, "l", &l_built) && pico8_entity_get_field(built, "r", &r_built));
    assert(pico8_entity_get_field(generic, "l", &l_generic) && pico8_entity_get_field(generic, "r", &r_generic));
    assert(l_built.as.ref == l_generic.as.ref && r_built.as.ref == r_generic.as.ref);
    assert(l_built.as.ref != r_built.as.ref);
}

#define WITNESS 0x1234

/* Reads `name` through the raw reader and through pico8_entity_get_field,
 * which must agree; a miss leaves the witness intact in both. */
static uint32_t fold_member(uint32_t h, const void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(WITNESS);
    pico8_value_t gen = pico8_value_fixed(WITNESS);
    int got_raw = pico8_nf_struct_class()->field(e, name, &raw);
    int got_gen = pico8_entity_get_field(e, name, &gen);

    assert(got_raw == got_gen && raw.kind == gen.kind);
    h = fnv(h, (uint32_t)got_raw);
    h = fnv(h, (uint32_t)raw.kind);
    if (!got_raw) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == WITNESS && gen.as.fixed == WITNESS);
    } else if (raw.kind == PICO8_VALUE_FIXED) {
        assert(raw.as.fixed == gen.as.fixed);
        h = fnv(h, (uint32_t)raw.as.fixed);
    } else if (raw.kind == PICO8_VALUE_STRING) {
        assert(strcmp(raw.as.string, gen.as.string) == 0);
        h = fnv(h, (uint32_t)strlen(raw.as.string));
    } else {
        assert(raw.kind == PICO8_VALUE_REF && raw.as.ref == gen.as.ref);
    }
    if (out != NULL) {
        *out = raw;
    }
    return h;
}

/* `ra` present, a vector the type tag does not know, folded by value; its
 * address is returned for the caller to judge. */
static uint32_t fold_ra(uint32_t h, const void *e, const void **addr) {
    pico8_value_t v;
    const pico8_vec_t *ra;

    h = fold_member(h, e, "ra", &v);
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN);
    ra = v.as.ref;
    h = fnv(h, (uint32_t)ra->x);
    h = fnv(h, (uint32_t)ra->y);
    *addr = ra;
    return h;
}

static uint32_t fold_nf(uint32_t h, const void *e, const void **ra_addr) {
    static const char *const intruders[] = {"", "x", "RA", "p", "v", "ke", "header", "has_ra"};
    pico8_value_t v;
    pico8_value_t l;
    pico8_value_t r;
    size_t i;

    h = fold_ra(h, e, ra_addr);
    h = fold_member(h, e, "l", &l);
    h = fold_member(h, e, "r", &r);
    assert(l.kind == PICO8_VALUE_REF && r.kind == PICO8_VALUE_REF && l.as.ref != r.as.ref);
    assert(l.as.ref == button("l") && r.as.ref == button("r"));
    h = fold_member(h, e, "me", NULL);
    h = fold_member(h, e, "t", NULL);
    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        h = fold_member(h, e, intruders[i], &v);
        assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == WITNESS);
    }
    return h;
}

static void must_abort(void *e, const char *name, pico8_value_t value) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        pico8_entity_set_field(e, name, value);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

static pico8_value_t vec_ref(int x, int y) {
    pico8_vec_t *p = malloc(sizeof *p);

    *p = pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
    return pico8_value_ref(p);
}

/* `ra` read by name on `je` and on fresh nfs, which all share one copy of
 * the zero vector `pico8_nf_start.ra`; `ph` rebinds a fresh vector each call and
 * leaves the old one intact; `bg.mq` keeps the very vector `je.ra` hands
 * out; no name can be written. */
static void test_nf_members_by_name(void) {
    uint32_t h = 2166136261u;
    void *je;
    void *a;
    void *b;
    void *bg;
    const void *ra_je;
    const void *ra_a;
    const void *ra_b;
    const void *ra_prev;
    pico8_value_t r;
    pico8_value_t out;
    pico8_value_t zero_false;
    pico8_value_t zero_fixed;
    int i;

    pico8_jl();
    je = pico8_je();
    a = pico8_nf_new();
    b = pico8_nf_new();
    h = fold_nf(h, je, &ra_je);
    h = fold_nf(h, a, &ra_a);
    h = fold_nf(h, b, &ra_b);
    assert(ra_je == ra_a && ra_a == ra_b);
    assert(((const pico8_vec_t *)ra_a)->x == 0 && ((const pico8_vec_t *)ra_a)->y == 0);
    assert(ra_a != (const void *)&pico8_nf_start.ra);
    assert(((const pico8_vec_t *)ra_a)->x == pico8_nf_start.ra.x);
    assert(((const pico8_vec_t *)ra_a)->y == pico8_nf_start.ra.y);

    frame(0, 17, 93, 0);
    frame(1, 101, 5, 0);
    script(2);
    ra_prev = ra_b;
    for (i = 0; i < 2; i++) {
        pico8_nf_ph(&input, a);
        h = fold_nf(h, a, &ra_a);
        assert(ra_a != ra_b && ra_a != ra_prev && ra_a != (const void *)&pico8_nf_start.ra);
        assert(((const pico8_vec_t *)ra_a)->x == pico8_fixed_from_int(g_frames[i].mouse_x));
        assert(((const pico8_vec_t *)ra_a)->y == pico8_fixed_from_int(g_frames[i].mouse_y));
        if (i == 1) {
            assert(((const pico8_vec_t *)ra_prev)->x == pico8_fixed_from_int(17));
            assert(((const pico8_vec_t *)ra_prev)->y == pico8_fixed_from_int(93));
        }
        ra_prev = ra_a;
        h = fold_nf(h, b, &ra_b);
        assert(ra_b == ra_je && ((const pico8_vec_t *)ra_b)->x == 0 && ((const pico8_vec_t *)ra_b)->y == 0);
        if (i == 0) {
            pico8_input_step(&input);
        }
    }
    unscript();

    bg = (void *)pico8_bg_new();
    assert(pico8_entity_get_field(je, "ra", &r) && r.kind == PICO8_VALUE_REF);
    assert(pico8_mv(bg, "rdown", 2, vec_ref(0, 0), r, &out));
    assert(pico8_entity_get_field(bg, "mq", &out) && out.kind == PICO8_VALUE_REF && out.as.ref == ra_je);
    h = fold_nf(h, je, &ra_je);
    assert(ra_je == out.as.ref);

    memset(&zero_false, 0, sizeof zero_false);
    zero_false.kind = PICO8_VALUE_BOOL;
    memset(&zero_fixed, 0, sizeof zero_fixed);
    zero_fixed.kind = PICO8_VALUE_FIXED;
    must_abort(je, "ra", pico8_value_ref(NULL));
    must_abort(je, "ra", vec_ref(1, 1));
    must_abort(je, "ra", zero_false);
    must_abort(je, "ra", zero_fixed);
    h = fold_nf(h, je, &ra_je);
    assert(ra_je == out.as.ref);
    check("nf members", h, NF_MEMBERS_HASH);
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

    test_jl_builds_a_mouse_reader();
    test_scripted_ticks();
    test_buttons_survive_jl();
    test_nf_init_starts_the_tables_afresh();
    test_placeholder_je();
    test_launch_with_left_held();
    test_nf_is_a_struct_sharing_its_buttons();
    test_nf_members_by_name();

    printf("test_pico8_nf: all tests passed\n");
    return 0;
}
