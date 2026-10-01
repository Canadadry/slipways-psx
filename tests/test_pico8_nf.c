/* The mouse reader `nf` (`je` once `jl()` has run), read through
 * pico8_je_nf() and the shared `l`/`r` tables and driven through the real
 * `dg`, in games built directly: no events file. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p8/assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "../src/pico8_mouse.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_vec.h"

#define SCRIPTED_TICKS 12
#define SCRIPTED_HASH 0xEB03D064u

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

static int ents_listed(const pico8_ents_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return 1;
        }
    }
    return 0;
}

static int in_any_cv(const void *e) {
    const char *names[] = {"kj", "ev", "hd", "hk", "hb"};
    int i;

    for (i = 0; i < 5; i++) {
        if (ents_listed(pico8_cv(names[i]), e)) {
            return 1;
        }
    }
    return 0;
}

/* The shared tables while `je` is an nf, NULL on the placeholder. */
static const pico8_nf_button_t *je_l(void) {
    return pico8_je_nf() != NULL ? pico8_nf_l() : NULL;
}

static const pico8_nf_button_t *je_r(void) {
    return pico8_je_nf() != NULL ? pico8_nf_r() : NULL;
}

/* -1 absent, else the boolean. */
static int on_of(const pico8_nf_button_t *b) {
    return b == NULL || !b->has_on ? -1 : b->on;
}

static int jn_of(const pico8_nf_button_t *b) {
    return b == NULL || !b->has_on ? -1 : b->jn;
}

static int hm_of(const pico8_nf_button_t *b) {
    return b == NULL || !b->has_on ? -1 : b->hm;
}

static int ra_of_je(pico8_vec_t *out) {
    const pico8_nf_t *nf = pico8_je_nf();

    if (nf == NULL || nf->ra == NULL) {
        return 0;
    }
    *out = *nf->ra;
    return 1;
}

static uint32_t fold_je(uint32_t h) {
    const pico8_nf_button_t *bts[2];
    pico8_vec_t ra;
    int i;

    bts[0] = je_l();
    bts[1] = je_r();

    if (ra_of_je(&ra)) {
        h = fnv(h, (uint32_t)ra.x);
        h = fnv(h, (uint32_t)ra.y);
    } else {
        h = fnv(h, 0xDEADu);
    }
    for (i = 0; i < 2; i++) {
        h = fnv(h, (uint32_t)on_of(bts[i]));
        h = fnv(h, (uint32_t)jn_of(bts[i]));
        h = fnv(h, (uint32_t)hm_of(bts[i]));
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
    pico8_vec_t ra;

    pico8_poke(&mem, 0x5f2d, 0);
    pico8_jl();
    je = pico8_je();
    assert(pico8_peek(&mem, 0x5f2d) == 1);
    assert(ents_listed(pico8_hv(), je) && !in_any_cv(je));
    assert(ra_of_je(&ra) && ra.x == 0 && ra.y == 0);
    assert(je_l() != NULL && je_r() != NULL && je_l() != je_r());
    assert(on_of(je_l()) == -1 && jn_of(je_l()) == -1 && hm_of(je_l()) == -1);
    assert(on_of(je_r()) == -1 && jn_of(je_r()) == -1 && hm_of(je_r()) == -1);
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
    const pico8_nf_button_t *l_before;

    pico8_jl();
    frame(0, 40, 40, 1);
    frame(1, 41, 41, 1);
    frame(2, 42, 42, 1);
    script(3);
    pico8_dg();
    pico8_input_step(&input);
    pico8_dg();
    assert(on_of(je_l()) == 1);
    l_before = je_l();

    pico8_jl();
    pico8_input_step(&input);
    pico8_dg();
    unscript();
    assert(je_l() == l_before);
    assert(jn_of(je_l()) == 0 && on_of(je_l()) == 1);
}

/* Rebuilding the class starts both tables afresh. */
static void test_nf_init_starts_the_tables_afresh(void) {
    pico8_jl();
    frame(0, 10, 10, 1);
    script(1);
    pico8_dg();
    unscript();
    assert(on_of(je_l()) == 1);

    pico8_nf_init();
    pico8_jl();
    assert(on_of(je_l()) == -1 && on_of(je_r()) == -1);
}

/* Before `jl()`, `je` is the field-less placeholder: no buttons, no `ra`,
 * and `ok`'s frame dispatches no press. */
static void test_placeholder_je(void) {
    pico8_ui_t *ok;
    pico8_vec_t ra;

    pico8_jl();
    pico8_ui_init();
    assert(je_l() == NULL && je_r() == NULL && !ra_of_je(&ra));
    ok = pico8_ui_new();
    pico8_ui_lj(&mem, ok);
    assert(ok->lheld == NULL && ok->rheld == NULL);
    assert(ok->qa != NULL);
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
    assert(on_of(je_l()) == 1);
}

static int is_an_nf_struct(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_nf_struct_class();
}

/* Built by `pico8_nf_new` or through its class, an nf is a struct in hv
 * only; `ph` is its method, `kj` is absent, and every nf shares the class's
 * two button tables. */
static void test_nf_is_a_struct_sharing_its_buttons(void) {
    void *built;
    void *generic;

    pico8_jl();
    built = pico8_nf_new();
    generic = pico8_nf_new();

    assert(is_an_nf_struct(built) && is_an_nf_struct(generic) && is_an_nf_struct(pico8_je()));
    assert(ents_listed(pico8_hv(), built) && !in_any_cv(built));
    assert(ents_listed(pico8_hv(), generic) && !in_any_cv(generic));
    assert(((const pico8_struct_header_t *)generic)->cls->states[PICO8_STATE_PH] != NULL);
    assert(((const pico8_struct_header_t *)generic)->cls->kj == NULL);
    assert(pico8_nf_l() != NULL && pico8_nf_r() != NULL);
    assert(pico8_nf_l() != pico8_nf_r());
    assert(*((const pico8_struct_header_t *)built)->ra == ((const pico8_nf_t *)built)->ra);
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

    printf("test_pico8_nf: all tests passed\n");
    return 0;
}
