/* The mouse reader `nf` (`je` once `jl()` has run), read only through the
 * dual-form field reader and driven through the real `dg`. The frozen values
 * were taken on the code where `nf` was still a class instance and must not
 * be re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_cheat.h"
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
#define TITLE_REPLAY "tests/fixtures/trading_hub_session_events.json"
#define TITLE_FRAMES 180
#define TITLE_HASH 0xBDC26772u

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

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)pico8_peek(&mem, PICO8_ADDR_SCREEN + i);
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
    ok = pico8_ui_new(NULL);
    pico8_ui_lj(&mem, ok);
    assert(!pico8_entity_get_field(ok, "lheld", &v) && !pico8_entity_get_field(ok, "rheld", &v));
    assert(pico8_entity_get_field(ok, "qa", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
}

/* A raylib recording from the title screen, through the click that starts a
 * game with the left button still held. */
static void test_replay_from_title(void) {
    uint32_t h = 2166136261u;
    pico8_cheat_t cheat;
    const void *first_je;
    int launched_at = -1;
    int i;

    pico8_cart_load();
    pico8_input_init(&input);
    assert(pico8_input_load(&input, TITLE_REPLAY));
    pico8_dispatch_set_input(&input);
    pico8__init();
    first_je = pico8_je();
    pico8_cheat_init(&cheat);
    for (i = 0; i < input.frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();
        if (launched_at < 0 && pico8_je() != first_je) {
            launched_at = i;
            assert(flag("l", "on") == 1);
        }
        h = fold_je(h);
        h = fnv(h, screen_hash());
        if (i + 1 == TITLE_FRAMES || i + 1 == input.frame_count) {
            break;
        }
        pico8_input_step(&input);
    }
    pico8_input_free(&input);
    unscript();
    printf("[title replay] launched at frame %d, frames=%d\n", launched_at, i + 1);
    fflush(stdout);
    assert(i + 1 == TITLE_FRAMES);
    fflush(stdout);
    assert(launched_at >= 0 && launched_at < TITLE_FRAMES);
    check("title replay", h, TITLE_HASH);
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
    built = pico8_nf_new(NULL);
    generic = pico8_nf_new(NULL);

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
    test_replay_from_title();
    test_nf_is_a_struct_sharing_its_buttons();

    printf("test_pico8_nf: all tests passed\n");
    return 0;
}
