/* The click dispatcher `ui`, the singleton `ok`, built by its only road
 * (`jl()`, `pico8_ui_new(NULL)`, on a game started as the net starts it),
 * read only through the dual-form field API and
 * `pico8_ui_default(...)` for spec defaults, and
 * driven through the real `dg`/`lj`/`ev`/`jz`/`cj`/`pz`. The frozen values
 * were taken on the code where `ui` was still a class instance and must not
 * be re-taken. */

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
#include "../src/pico8_bg.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_cheat.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_menu.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define DRAG "tests/fixtures/slipway_drag_events.json"
#define TECH "tests/fixtures/tech_screen_invent_events.json"
#define BEFORE_DRAG_PRESS 195

#define BUILT_HASH 0x86CE257Cu
#define SHARED_HASH 0x690E4805u
#define FRESH_HASH 0xE44E0786u
#define HOVER_HASH 0xB61A6E40u
#define LAYERS_HASH 0xDBA4EEF8u
#define RIGHT_HASH 0xF02024DCu
#define LEFT_HASH 0x216F4CA9u
#define CJ_HASH 0xD215FB81u
#define STALE_HASH 0x5B066E86u
#define DRAG_REPLAY_HASH 0xA3868481u
#define TECH_REPLAY_HASH 0xAB5A4DE6u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

static const char *const absent[] = {"qa", "ic",    "o_",    "gj", "lheld", "rheld", "ra",
                                     "p_", "nu",    "ph",    "ldown", "rdown", "hd", "fx"};
static const char *const not_listed[] = {"kj", "hd", "hk", "hb"};
static const char *const tags[] = {"jo", "g_", "qh", "jc", "mz", "mh", "np", "hj", "prober", "nr", "bg"};

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

static void *get_ref(const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v) && v.kind == PICO8_VALUE_REF ? v.as.ref : NULL;
}

static pico8_fixed_t get_fixed(const void *e, const char *name) {
    pico8_value_t v;
    assert(lookup(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static void *class_ref(const char *name) {
    pico8_value_t v;
    assert(pico8_ui_default(name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
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

static const pico8_list_t *le_of(const void *ok) {
    const pico8_list_t *le = get_ref(ok, "le");
    assert(le != NULL);
    return le;
}

static pico8_vec_t vec_of(const void *e, const char *name) {
    pico8_value_t v;
    assert(lookup(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return *(const pico8_vec_t *)v.as.ref;
}

static uint32_t fold_vec(uint32_t h, pico8_vec_t v) {
    return fnv(fnv(h, (uint32_t)v.x), (uint32_t)v.y);
}

/* Presence, then the rank in `hv` of an entity-valued field. */
static uint32_t fold_entity_field(uint32_t h, const void *e, const char *name) {
    void *ref = get_ref(e, name);
    h = fnv(h, (uint32_t)(ref != NULL));
    return fnv(h, (uint32_t)rank_in(pico8_hv(), ref));
}

static uint32_t fold_fixed_field(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    if (!lookup(e, name, &v)) {
        return fnv(h, 0u);
    }
    assert(v.kind == PICO8_VALUE_FIXED);
    return fnv(fnv(h, 1u), (uint32_t)v.as.fixed);
}

static int layer_count(const void *ok, int mf) {
    const pico8_ob_table_t *ic = get_ref(ok, "ic");
    pico8_value_t bucket;
    if (ic == NULL || !pico8_ob_table_get_index(ic, mf, &bucket) || bucket.kind != PICO8_VALUE_REF ||
        bucket.as.ref == NULL) {
        return 0;
    }
    return ((const pico8_list_t *)bucket.as.ref)->count;
}

static int in_layer(const void *ok, int mf, const void *e) {
    const pico8_ob_table_t *ic = get_ref(ok, "ic");
    pico8_value_t bucket;
    if (ic == NULL || !pico8_ob_table_get_index(ic, mf, &bucket) || bucket.kind != PICO8_VALUE_REF) {
        return 0;
    }
    return listed(bucket.as.ref, e);
}

static uint32_t fold_layers(uint32_t h, const void *ok) {
    int mf;
    for (mf = 0; mf <= 15; mf++) {
        h = fnv(h, (uint32_t)layer_count(ok, mf));
    }
    return h;
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

/* `ev` drawn alone on a cleared screen is exactly sprite `id` at `je.ra`. */
static uint32_t cursor_is(int id) {
    pico8_vec_t at = vec_of(pico8_je(), "ra");
    uint32_t drawn;

    pico8_cls(&mem);
    pico8_ui_ev(&gfx, &mem, pico8_ok());
    drawn = screen_hash();
    pico8_cls(&mem);
    pico8_spr_default(&gfx, &mem, id, pico8_fixed_flr_int(at.x), pico8_fixed_flr_int(at.y));
    assert(drawn == screen_hash());
    return drawn;
}

static int is_of(const void *e, const pico8_struct_class_t *cls) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)e)->cls == cls;
}

static void start_input(const char *events) {
    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
}

/* A game started as the net starts it: `eu()` on `bo[2]` before frame 0. */
static void *start_game(const char *events) {
    pico8_value_t difficulty;

    start_input(events);
    pico8__init();
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty) && difficulty.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty.as.ref));
    pico8_cheat_init(&cheat);
    assert(pico8_ok() != NULL);
    return pico8_ok();
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

static pico8_input_frame_t g_mouse;

/* One frame's mouse, read by `je` through `dg`. */
static void mouse_at(int x, int y, int buttons) {
    memset(&g_mouse, 0, sizeof g_mouse);
    g_mouse.mouse_x = x;
    g_mouse.mouse_y = y;
    g_mouse.mouse_buttons = buttons;
    input.frames = &g_mouse;
    input.frame_count = 1;
    input.current_frame = 0;
    pico8_dispatch_set_input(&input);
    pico8_dg();
}

static void *the_bg(void) {
    const pico8_list_t *hv = pico8_hv();
    void *bg = NULL;
    int i;
    for (i = 0; i < hv->count; i++) {
        if (is_of(hv->items[i].as.ref, pico8_bg_struct_class())) {
            assert(bg == NULL);
            bg = hv->items[i].as.ref;
        }
    }
    assert(bg != NULL);
    return bg;
}

/* The game's `ok` as `jl()` and `lt` left it: where it registered, its
 * defaults, the names only read elsewhere, its one method, and the class's
 * `le`/`hw` by identity. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *ok = start_game(DRAG);
    pico8_value_t v;
    size_t i;

    h = fnv(h, (uint32_t)rank_in(pico8_hv(), ok));
    h = fnv(h, (uint32_t)pico8_hv()->count);
    assert(listed(pico8_cv("ev"), ok));
    h = fnv(h, (uint32_t)rank_in(pico8_cv("ev"), ok));
    for (i = 0; i < sizeof not_listed / sizeof not_listed[0]; i++) {
        assert(!listed(pico8_cv(not_listed[i]), ok));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        assert(!listed(pico8_cg(tags[i]), ok));
    }

    assert(get_fixed(ok, "ql") == pico8_fixed_from_int(58));
    assert(get_fixed(ok, "fa") == pico8_fixed_from_int(15));
    assert(lookup(ok, "me", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, "ph") == 0);
    h = fold_fixed_field(h, ok, "t");
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(ok, absent[i]));
    }
    assert(lookup(ok, "ev", &v) && v.kind == PICO8_VALUE_FUNCTION);

    assert(get_ref(ok, "le") == class_ref("le"));
    assert(get_ref(ok, "hw") == class_ref("hw"));
    assert(pico8_type_tag_of(class_ref("le")) == PICO8_TYPE_TAG_LIST && le_of(ok)->count == 0);
    check("built", h, BUILT_HASH);
}

static pico8_value_t nil(void) {
    return pico8_value_ref(NULL);
}

static void call_row_my(const pico8_ob_table_t *t, int i) {
    pico8_value_t row;
    pico8_value_t my;
    assert(pico8_ob_table_get_index(t, i, &row) && row.kind == PICO8_VALUE_REF && row.as.ref != NULL);
    assert(pico8_ob_table_get_name(row.as.ref, "my", &my) && my.kind == PICO8_VALUE_FUNCTION);
    (void)my.as.function(NULL, 0, nil(), nil());
}

/* A second `ui` shares the class's empty `le` until its own `cj`; `pz`
 * replaces `ok.le`, and a `p` with `ff` empties it. */
static void test_shared(void) {
    uint32_t h = 2166136261u;
    void *ok = start_game(DRAG);
    void *other = pico8_ui_new(NULL);
    const pico8_list_t *le = get_ref(ok, "le");

    assert(other != NULL && other != ok);
    assert(get_ref(other, "le") == le && le == class_ref("le"));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), other));
    h = fnv(h, (uint32_t)rank_in(pico8_cv("ev"), other));

    pico8_ui_cj(other);
    assert(get_ref(other, "le") != le && le_of(other)->count == 0);
    assert(get_ref(ok, "le") == le && class_ref("le") == le && le->count == 0);

    call_row_my(pico8_ix(), 4);
    assert(get_ref(ok, "le") != le && get_ref(other, "le") != get_ref(ok, "le"));
    h = fnv(h, (uint32_t)le_of(ok)->count);
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), le_of(ok)->items[0].as.ref));
    assert(class_ref("le") == le && le->count == 0);

    (void)pico8_fv_new(NULL);
    assert(le_of(ok)->count == 0 && get_ref(ok, "le") != le);
    h = fnv(h, (uint32_t)pico8_hv()->count);
    check("shared", h, SHARED_HASH);
}

/* A fresh game's first `lj`: `qa` = `je.ra` + `mo.ra`, `ic` built, `gj` is
 * `o_`, and `ev` draws sprite 58 at `je.ra`. */
static void test_fresh(void) {
    uint32_t h = 2166136261u;
    void *ok = start_game(DRAG);
    pico8_vec_t want;

    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, pico8_ok());
    want = pico8_vec_add(vec_of(pico8_je(), "ra"), vec_of(pico8_mo(), "ra"));
    assert(vec_of(ok, "qa").x == want.x && vec_of(ok, "qa").y == want.y);
    assert(has(ok, "ic") && has(ok, "o_"));
    assert(get_ref(ok, "gj") == get_ref(ok, "o_"));
    assert(get_ref(ok, "o_") == the_bg());
    h = fold_vec(h, vec_of(ok, "qa"));
    h = fold_layers(h, ok);
    h = fold_entity_field(h, ok, "o_");
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    h = fnv(h, cursor_is(58));
    assert(get_ref(ok, "le") == class_ref("le"));
    check("fresh", h, FRESH_HASH);
}

/* The first planet of `cg.jo` whose screen position is well inside. */
static void *planet_on_screen(int *sx, int *sy) {
    const pico8_list_t *jo = pico8_cg("jo");
    pico8_vec_t cam = vec_of(pico8_mo(), "ra");
    int i;

    for (i = 0; jo != NULL && i < jo->count; i++) {
        pico8_vec_t at = vec_of(jo->items[i].as.ref, "ra");
        int x = pico8_fixed_flr_int(pico8_fixed_sub(at.x, cam.x));
        int y = pico8_fixed_flr_int(pico8_fixed_sub(at.y, cam.y));
        if (x >= 16 && x <= 112 && y >= 16 && y <= 112) {
            *sx = x;
            *sy = y;
            return jo->items[i].as.ref;
        }
    }
    assert(0 && "no planet on screen");
    return NULL;
}

/* The mouse over a planet: `o_` is the planet, marked `m_`, its `fg` poked
 * at 0x5f80, `gj` on the next pass; the mouse away unmarks it. */
static void test_hover(void) {
    uint32_t h = 2166136261u;
    void *ok = start_game(DRAG);
    int x;
    int y;
    void *planet;
    pico8_value_t fg;

    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, ok);
    planet = planet_on_screen(&x, &y);
    mouse_at(x, y, 0);
    pico8_ui_lj(&mem, ok);
    assert(get_ref(ok, "o_") == planet);
    assert(bool_state(planet, "m_") == 2);
    assert(pico8_peek(&mem, 0x5f80) == (lookup(planet, "fg", &fg) ? pico8_fixed_flr_int(fg.as.fixed) : 0));
    assert(in_layer(ok, 6, planet) && in_layer(ok, 1, the_bg()));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), planet));
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    h = fold_layers(h, ok);

    pico8_ui_lj(&mem, ok);
    assert(get_ref(ok, "gj") == planet && get_ref(ok, "o_") == planet);
    assert(bool_state(planet, "m_") == 2);
    h = fnv(h, cursor_is(58));

    mouse_at(x + 40 > 127 ? x - 40 : x + 40, y, 0);
    pico8_ui_lj(&mem, ok);
    assert(get_ref(ok, "o_") != planet);
    assert(bool_state(planet, "m_") == 1);
    h = fold_entity_field(h, ok, "o_");
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    check("hover", h, HOVER_HASH);
}

/* `jz` walks the layers from 15 down: over a planet, `hd` answers from the
 * planet (layer 6) before the background (layer 1); `rdown` only from the
 * background, which answers itself. */
static void test_layers(void) {
    uint32_t h = 2166136261u;
    void *ok = start_game(DRAG);
    pico8_struct_header_t *kv = NULL;
    pico8_value_t kn;
    pico8_value_t qa;
    pico8_value_t ra;
    void *planet;
    int x;
    int y;

    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, ok);
    planet = planet_on_screen(&x, &y);
    mouse_at(x, y, 0);
    pico8_ui_lj(&mem, ok);
    assert(lookup(ok, "qa", &qa) && lookup(pico8_je(), "ra", &ra));

    assert(pico8_ui_jz(pico8_ok(), "hd", 2, qa, ra, &kv, &kn));
    assert((void *)kv == planet);
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), kv));
    kv = NULL;
    assert(pico8_ui_jz(pico8_ok(), "rdown", 2, qa, ra, &kv, &kn));
    assert((void *)kv == the_bg() && kn.kind == PICO8_VALUE_REF && kn.as.ref == the_bg());
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), kv));
    kv = NULL;
    assert(!pico8_ui_jz(pico8_ok(), "zz", 2, qa, ra, &kv, &kn) && kv == NULL);
    check("layers", h, LAYERS_HASH);
}

/* The right button held scrolls through the background's `rdown`/`hq`:
 * `rheld` is the background and the cursor is sprite 9; released, `rheld`
 * goes. */
static void test_right(void) {
    uint32_t h = 2166136261u;
    void *ok = start_game(DRAG);
    void *bg;

    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, ok);
    bg = the_bg();
    mouse_at(64, 100, 2);
    pico8_ui_lj(&mem, ok);
    assert(get_ref(ok, "rheld") == bg && !has(ok, "lheld"));
    h = fnv(h, cursor_is(9));

    mouse_at(70, 90, 2);
    pico8_ui_lj(&mem, ok);
    assert(get_ref(ok, "rheld") == bg);
    h = fold_vec(h, vec_of(pico8_mo(), "ra"));
    h = fnv(h, cursor_is(9));

    mouse_at(70, 90, 0);
    pico8_ui_lj(&mem, ok);
    assert(!has(ok, "rheld") && !has(ok, "lheld"));
    h = fold_vec(h, vec_of(pico8_mo(), "ra"));
    h = fnv(h, cursor_is(58));
    check("right", h, RIGHT_HASH);
}

static int oe_count(const void *planet) {
    const pico8_list_t *oe = get_ref(planet, "oe");
    return oe == NULL ? 0 : oe->count;
}

/* The drag replay from the title screen, as its regression test drives
 * it, up to its last press, then by hand: the left button on the built
 * planet holds a new slipway `jc` in `lheld`; released over the other
 * planet, `ey` gets that planet and `lheld` goes. */
static void test_left(void) {
    uint32_t h = 2166136261u;
    void *ok;
    void *held;
    void *origin;
    void *target;
    int i;

    start_input(DRAG);
    pico8__init();
    pico8_cheat_init(&cheat);
    for (i = 0; i < BEFORE_DRAG_PRESS; i++) {
        play_frame(i);
    }
    ok = pico8_ok();
    mouse_at(88, 37, 1);
    pico8_ui_lj(&mem, ok);
    held = get_ref(ok, "lheld");
    assert(held != NULL && is_of(held, pico8_jc_struct_class()) && !has(ok, "rheld"));
    origin = get_ref(held, "qu");
    assert(origin != NULL && oe_count(origin) == 0);
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), held));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), origin));
    h = fnv(h, cursor_is(58));

    mouse_at(78, 73, 1);
    pico8_ui_lj(&mem, ok);
    assert(get_ref(ok, "lheld") == held);
    target = get_ref(ok, "o_");
    assert(target != NULL && target != origin);
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), target));

    mouse_at(78, 73, 0);
    pico8_ui_lj(&mem, ok);
    assert(!has(ok, "lheld") && !has(ok, "rheld"));
    assert(oe_count(origin) == 1 && oe_count(target) == 1);
    h = fnv(h, (uint32_t)pico8_cg("jc")->count);
    check("left", h, LEFT_HASH);
}

/* `cj` unregisters every entity of the old `le` and starts a new list. */
static void test_cj(void) {
    uint32_t h = 2166136261u;
    void *ok = start_game(DRAG);
    const pico8_list_t *le;
    int count;
    int i;

    call_row_my(pico8_ix(), 4);
    le = le_of(ok);
    count = le->count;
    assert(count > 0);
    for (i = 0; i < count; i++) {
        assert(listed(pico8_hv(), le->items[i].as.ref));
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), le->items[i].as.ref));
    }
    h = fnv(h, (uint32_t)pico8_hv()->count);
    pico8_ui_cj(pico8_ok());
    assert(get_ref(ok, "le") != le && le_of(ok)->count == 0 && get_ref(ok, "le") != class_ref("le"));
    for (i = 0; i < count; i++) {
        assert(!listed(pico8_hv(), le->items[i].as.ref));
    }
    h = fnv(h, (uint32_t)pico8_hv()->count);
    check("cj", h, CJ_HASH);
}

/* On the title screen, a click on a difficulty button runs `eu()` from
 * inside `lj`: `jl()` rebuilds `ok`, and the end of `lj` still runs on the
 * old one, whose `cj` and `gj` land on it, not on the new `ok`. */
static void test_stale(void) {
    uint32_t h = 2166136261u;
    void *old;
    void *button;
    const pico8_list_t *le;

    start_input(DRAG);
    pico8__init();
    pico8_cheat_init(&cheat);
    old = pico8_ok();
    mouse_at(76, 71, 0);
    pico8_ui_lj(&mem, old);
    button = get_ref(old, "o_");
    le = le_of(old);
    assert(button != NULL && is_of(button, pico8_js_struct_class()) && le->count == 0);
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), button));

    mouse_at(76, 71, 1);
    pico8_ui_lj(&mem, old);
    assert(pico8_ok() != old && pico8_ok() != NULL);
    assert(get_ref(old, "gj") == get_ref(old, "o_") && is_of(get_ref(old, "gj"), pico8_js_struct_class()));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), get_ref(old, "gj")));
    assert(get_ref(old, "le") != le && le_of(old)->count == 0);
    assert(!has(pico8_ok(), "gj") && !has(pico8_ok(), "qa"));
    h = fnv(h, (uint32_t)le_of(pico8_ok())->count);
    h = fnv(h, (uint32_t)(get_ref(pico8_ok(), "le") == class_ref("le")));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), pico8_ok()));
    h = fnv(h, (uint32_t)pico8_hv()->count);
    check("stale", h, STALE_HASH);
}

/* A replay from the net's start to its last frame, folding `ok` each frame:
 * its rank and `t`; presence and rank of `o_`, `gj`, `lheld`, `rheld`;
 * `le`'s count and whether it changed; each layer of `ic`; `qa`. Then the
 * screen. */
static uint32_t replay(const char *events) {
    uint32_t h = 2166136261u;
    const void *le_before;
    int i;

    (void)start_game(events);
    le_before = get_ref(pico8_ok(), "le");
    for (i = 0; i < input.frame_count; i++) {
        const void *ok;
        play_frame(i);
        ok = pico8_ok();
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), ok));
        h = fold_fixed_field(h, ok, "t");
        h = fold_entity_field(h, ok, "o_");
        h = fold_entity_field(h, ok, "gj");
        h = fold_entity_field(h, ok, "lheld");
        h = fold_entity_field(h, ok, "rheld");
        h = fnv(h, (uint32_t)le_of(ok)->count);
        h = fnv(h, (uint32_t)(get_ref(ok, "le") != le_before));
        le_before = get_ref(ok, "le");
        h = fold_layers(h, ok);
        h = fold_vec(h, vec_of(ok, "qa"));
    }
    return fnv(h, screen_hash());
}

static void test_drag_replay(void) {
    check("drag replay", replay(DRAG), DRAG_REPLAY_HASH);
}

static void test_tech_replay(void) {
    check("tech replay", replay(TECH), TECH_REPLAY_HASH);
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

static void write_o_(void *e) {
    pico8_entity_set_field(e, "o_", pico8_value_ref(NULL));
}

static void build_with_named_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "ql", pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_ui_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_ui_new(b);
}

static void bh_on_x(void *e) {
    pico8_ui_bh(e, "x");
}

/* The game's `ok` is a struct whose descriptor holds only `ev` and no
 * writer; it takes no construction key and no button but `l`/`r`. Its
 * class keeps only its spec, whose `le`/`hw` every new `ui` borrows. */
static void test_ui_is_a_struct(void) {
    static const char *const spec[] = {"ql", "fa", "le", "hw"};
    const pico8_struct_class_t *cls = pico8_ui_struct_class();
    pico8_ui_t *ok = start_game(DRAG);
    pico8_ui_t *other;
    pico8_value_t v;
    size_t i;
    int j;

    assert(is_of(ok, cls));
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        assert((cls->methods[j] != NULL) == (j == PICO8_METHOD_EV));
    }
    assert(cls->field != NULL && cls->set_field == NULL);
    assert(pico8_ui_ready());
    assert(!pico8_ui_default("ev", &v));
    for (i = 0; i < sizeof spec / sizeof spec[0]; i++) {
        assert(pico8_ui_default(spec[i], &v));
    }
    assert((const void *)pico8_ui_hw() == class_ref("hw") && ok->hw.as.ref == class_ref("hw"));
    assert(ok->le.as.ref == class_ref("le"));
    other = pico8_ui_new(NULL);
    assert(is_of(other, cls) && other->le.as.ref == class_ref("le") && other->hw.as.ref == class_ref("hw"));

    must_abort(write_o_, ok);
    must_abort(build_with_named_key, NULL);
    must_abort(build_with_positional_key, NULL);
    must_abort(bh_on_x, ok);
}

static void ks_on_a_table(void *e) {
    pico8_ke_ks(e, pico8_value_string("ph"));
}

static void lt_a_table(void *e) {
    pico8_lt(e);
}

/* `ks` takes a struct or NULL, and nothing but a struct enters `hv` for
 * `dg` to walk: an ob table is refused by `ks` and by `lt`. */
static void test_ke_closed(void) {
    pico8_ob_table_t *table = pico8_ob_table_new();

    (void)start_game(DRAG);
    must_abort(ks_on_a_table, table);
    must_abort(lt_a_table, table);
    pico8_ob_table_free(table);
    pico8_ke_ks(NULL, pico8_value_string("ph"));
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
        PICO8_MEMBER_CENSUS_CHILD_EXIT(0);
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
    in_fresh_child(test_shared);
    in_fresh_child(test_fresh);
    in_fresh_child(test_hover);
    in_fresh_child(test_layers);
    in_fresh_child(test_right);
    in_fresh_child(test_left);
    in_fresh_child(test_cj);
    in_fresh_child(test_stale);
    in_fresh_child(test_drag_replay);
    in_fresh_child(test_tech_replay);
    in_fresh_child(test_ui_is_a_struct);
    in_fresh_child(test_ke_closed);

    printf("test_pico8_ui: all tests passed\n");
    return 0;
}
