/* The click dispatcher `ui`, the singleton `ok`, built by its only road
 * (`jl()`, `pico8_ui_new(NULL)`, on a game started directly: no events
 * file), read only through the dual-form field API and
 * `pico8_ui_default(...)` for spec defaults, and
 * driven through the real `dg`/`lj`/`ev`/`jz`/`cj`/`pz`. */

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
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy.h"
#include "../src/pico8_economy_tiers.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_entity_geom.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_menu.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_particle.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_planet_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"
#include "../src/pico8_wormhole.h"

/* Ticks after the slipway is paid, long enough for the colonizing and
 * paying effects to leave `hv`: `dg` skips the entity after one it takes out
 * (TROU-5). */
#define SETTLE_TICKS 120

#define BUILT_HASH 0x86CE257Cu
#define SHARED_HASH 0x690E4805u
#define FRESH_HASH 0xE44E0786u
#define HOVER_HASH 0xB61A6E40u
#define LAYERS_HASH 0xDBA4EEF8u
#define RIGHT_HASH 0xF02024DCu
#define LEFT_HASH 0xBD9BC256u
#define CJ_HASH 0xD215FB81u
#define STALE_HASH 0x5B066E86u
#define KE_ME_HASH 0x691ED215u
#define KE_T_HASH 0x5DB51DEEu
#define JO_MEMBERS_HASH 0x62CB3FF0u
#define JO_WRITTEN_HASH 0x20E44B74u
#define JO_POSITION_HASH 0x05B358F1u
#define JO_STATE_HASH 0x1AE0C77Fu
#define JO_WRITERS_HASH 0xBF686C3Au
#define QH_MEMBERS_HASH 0x73166200u
#define HJ_MEMBERS_HASH 0x50DF104Cu
#define PM_MEMBERS_HASH 0xAE0F61DEu
#define MX_MEMBERS_HASH 0x696C3254u
#define UI_MEMBERS_HASH 0x04206F34u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

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
    const pico8_ui_ic_t *ic = get_ref(ok, "ic");
    return ic == NULL ? 0 : ic->layers[mf].count;
}

static int in_layer(const void *ok, int mf, const void *e) {
    const pico8_ui_ic_t *ic = get_ref(ok, "ic");
    return ic != NULL && listed(&ic->layers[mf], e);
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

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void *start_game(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
    assert(pico8_ok() != NULL);
    return pico8_ok();
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

static int placed(const void *e) {
    pico8_value_t v;
    return pico8_entity_get_field(e, "p", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL;
}

/* The planet of `cg.jo` nearest to `qu`, among those with a `p`. */
static void *nearest_to(const void *qu) {
    const pico8_list_t *jos = pico8_cg("jo");
    void *re = NULL;
    int i;

    for (i = 0; i < jos->count; i++) {
        void *e = jos->items[i].as.ref;
        if (e != qu && placed(e) && (re == NULL || dist2(pos_of(e), pos_of(qu)) < dist2(pos_of(re), pos_of(qu)))) {
            re = e;
        }
    }
    assert(re != NULL);
    return re;
}

/* On the current game: the first planet of `cg.jo` with a `p` and its
 * nearest one colonized by a bare project, joined by a slipway paid by `ey`
 * as a drag pays it (`jc:hq` reads the hovered planet off `ok.o_`), then
 * left to settle. */
static void grow_world(void) {
    const pico8_list_t *jos = pico8_cg("jo");
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    pico8_vec_t *mp = malloc(sizeof *mp);
    pico8_value_t out;
    void *qu = NULL;
    void *re;
    void *jc;
    int i;

    for (i = 0; qu == NULL && i < jos->count; i++) {
        if (placed(jos->items[i].as.ref)) {
            qu = jos->items[i].as.ref;
        }
    }
    assert(qu != NULL);
    re = nearest_to(qu);
    pico8_jo_hu(qu, pico8_hp_new(NULL));
    pico8_jo_hu(re, pico8_hp_new(NULL));
    pico8_ob_table_set_name(overrides, "qu", pico8_value_ref(qu));
    jc = pico8_jc_new(overrides);
    pico8_ob_table_free(overrides);
    *mp = pico8_vec_add(pico8_vec_mul(pos_of(re), pico8_fixed_from_int(128)),
                        pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(1)));
    pico8_ok()->o_ = re;
    assert(pico8_mv(jc, "hq", 1, pico8_value_ref(mp), pico8_value_ref(NULL), &out));
    pico8_ok()->o_ = NULL;
    for (i = 0; i < 15; i++) {
        pico8_dg();
    }
    assert(pico8_mv(jc, "ey", 1, pico8_value_ref(re), pico8_value_ref(NULL), &out));
    for (i = 0; i < SETTLE_TICKS; i++) {
        pico8_dg();
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
    void *ok = start_game();
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
    void *ok = start_game();
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
    void *ok = start_game();
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
    void *ok = start_game();
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
    void *ok = start_game();
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
    void *ok = start_game();
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

/* Where `e` is on screen under `mo`'s camera. */
static void screen_of(const void *e, int *x, int *y) {
    pico8_vec_t at = pico8_vec_sub(vec_of(e, "ra"), vec_of(pico8_mo(), "ra"));

    *x = pico8_fixed_flr_int(at.x);
    *y = pico8_fixed_flr_int(at.y);
}

static int on_screen(const void *e) {
    int x;
    int y;

    screen_of(e, &x, &y);
    return x >= 8 && x < 120 && y >= 8 && y < 120;
}

/* A started game's first planet on screen whose nearest one is on screen
 * too, both colonized by a bare project, then the mouse by hand: the left
 * button on the one holds a new slipway `jc` in `lheld`; released over the
 * other, `ey` gets that planet and `lheld` goes. */
static void test_left(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *jos;
    void *ok;
    void *held;
    void *origin = NULL;
    void *target;
    void *other = NULL;
    int x;
    int y;
    int i;

    ok = start_game();
    jos = pico8_cg("jo");
    for (i = 0; i < jos->count && other == NULL; i++) {
        void *e = jos->items[i].as.ref;
        if (on_screen(e) && on_screen(nearest_to(e))) {
            origin = e;
            other = nearest_to(e);
        }
    }
    assert(other != NULL);
    pico8_jo_hu(origin, pico8_hp_new(NULL));
    pico8_jo_hu(other, pico8_hp_new(NULL));
    for (i = 0; i < SETTLE_TICKS; i++) {
        pico8_dg();
    }

    screen_of(origin, &x, &y);
    mouse_at(x, y, 0);
    pico8_ui_lj(&mem, ok);
    mouse_at(x, y, 1);
    pico8_ui_lj(&mem, ok);
    held = get_ref(ok, "lheld");
    assert(held != NULL && is_of(held, pico8_jc_struct_class()) && !has(ok, "rheld"));
    assert(get_ref(held, "qu") == origin && oe_count(origin) == 0);
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), held));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), origin));
    h = fnv(h, cursor_is(58));

    screen_of(other, &x, &y);
    mouse_at(x, y, 1);
    pico8_ui_lj(&mem, ok);
    assert(get_ref(ok, "lheld") == held);
    target = get_ref(ok, "o_");
    assert(target == other);
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), target));

    mouse_at(x, y, 0);
    pico8_ui_lj(&mem, ok);
    assert(!has(ok, "lheld") && !has(ok, "rheld"));
    assert(oe_count(origin) == 1 && oe_count(target) == 1);
    h = fnv(h, (uint32_t)pico8_cg("jc")->count);
    check("left", h, LEFT_HASH);
}

/* `cj` unregisters every entity of the old `le` and starts a new list. */
static void test_cj(void) {
    uint32_t h = 2166136261u;
    void *ok = start_game();
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

    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
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
    pico8_ui_t *ok = start_game();
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
    assert((const void *)pico8_ui_hw() == class_ref("hw") && ok->hw == class_ref("hw"));
    assert(ok->le == class_ref("le"));
    other = pico8_ui_new(NULL);
    assert(is_of(other, cls) && other->le == class_ref("le") && other->hw == class_ref("hw"));

    must_abort(write_o_, ok);
    must_abort(build_with_named_key, NULL);
    must_abort(build_with_positional_key, NULL);
    must_abort(bh_on_x, ok);
}

static const char *const ui_names[10] = {"ql", "fa", "le", "hw", "qa", "ic", "o_", "gj", "lheld", "rheld"};

static int read_ui(const void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(0x1234);
    pico8_value_t via = pico8_value_fixed(0x1234);
    int got_raw = pico8_ui_struct_class()->field(e, name, &raw);
    int got_via = pico8_entity_get_field(e, name, &via);
    assert(got_raw == got_via && raw.kind == via.kind);
    if (!got_raw) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        assert(via.kind == PICO8_VALUE_FIXED && via.as.fixed == 0x1234);
    } else if (raw.kind == PICO8_VALUE_REF) {
        assert(raw.as.ref != NULL && raw.as.ref == via.as.ref);
    } else {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == via.as.fixed);
    }
    *out = raw;
    return got_raw;
}

static void *ui_ref(const void *ui, const char *name) {
    pico8_value_t v;
    if (!read_ui(ui, name, &v)) {
        return NULL;
    }
    assert(v.kind == PICO8_VALUE_REF);
    return v.as.ref;
}

static uint32_t fold_new_ui(uint32_t h, const void *ui) {
    pico8_value_t v;
    size_t i;

    assert(read_ui(ui, "ql", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(58));
    assert(read_ui(ui, "fa", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(15));
    assert(ui_ref(ui, "le") == class_ref("le") && ((const pico8_list_t *)ui_ref(ui, "le"))->count == 0);
    assert(ui_ref(ui, "hw") == class_ref("hw") && ui_ref(ui, "hw") == (const void *)pico8_ui_hw());
    assert(pico8_type_tag_of(ui_ref(ui, "le")) == PICO8_TYPE_TAG_LIST);
    assert(pico8_type_tag_of(ui_ref(ui, "hw")) == PICO8_TYPE_TAG_UNKNOWN);
    for (i = 4; i < 10; i++) {
        assert(!read_ui(ui, ui_names[i], &v));
    }
    h = fnv(h, (uint32_t)pico8_ui_hw()->xl);
    h = fnv(h, (uint32_t)pico8_ui_hw()->yb);
    return fnv(h, (uint32_t)rank_in(pico8_hv(), ui));
}

typedef struct {
    pico8_struct_header_t header;
} giver_t;

static pico8_value_t g_given;
static int g_giver_hovered = 1;

static pico8_value_t giver_down(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    return g_given;
}

static int giver_field(const void *self, const char *name, pico8_value_t *out_value) {
    (void)self;
    if (strcmp(name, "hd") == 0) {
        *out_value = g_giver_hovered ? pico8_value_fixed(pico8_fixed_from_int(1)) : pico8_value_bool(0);
        return 1;
    }
    if (strcmp(name, "fa") == 0) {
        *out_value = pico8_value_fixed(pico8_fixed_from_int(15));
        return 1;
    }
    return 0;
}

static int giver_set_field(void *self, const char *name, pico8_value_t value) {
    (void)self;
    (void)value;
    return strcmp(name, "m_") == 0;
}

static const pico8_struct_class_t g_giver_class = {
    {
        [PICO8_METHOD_LDOWN] = giver_down,
        [PICO8_METHOD_RDOWN] = giver_down,
    },
    giver_field,
    giver_set_field,
};

static giver_t g_giver = {{PICO8_TYPE_TAG_STRUCT, &g_giver_class}};

static void press(void *ok, int buttons) {
    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, ok);
    mouse_at(64, 100, buttons);
    pico8_ui_lj(&mem, ok);
}

static void release(void *ok) {
    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, ok);
}

static uint32_t fold_given_lheld(uint32_t h, void *ok, void *given) {
    g_given = pico8_value_ref(given);
    press(ok, 1);
    assert(ui_ref(ok, "lheld") == given && ui_ref(ok, "rheld") == NULL);
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), given));
    release(ok);
    assert(ui_ref(ok, "lheld") == NULL);
    return h;
}

static const char *g_write_name;

static void write_named(void *e) {
    pico8_entity_set_field(e, g_write_name, pico8_value_ref(NULL));
}

static void test_ui_members_by_name(void) {
    static const char *const intruders[] = {"", "q", "le ", "LE", "held", "l", "r", "has_le", "header", "ke", "o"};
    uint32_t h = 2166136261u;
    pico8_ui_t *ok = start_game();
    void *made[10];
    void *qa;
    void *ic;
    void *le;
    pico8_vec_t want;
    pico8_value_t v;
    size_t i;

    h = fold_new_ui(h, ok);
    h = fold_new_ui(h, pico8_ui_new(NULL));

    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, ok);
    want = pico8_vec_add(vec_of(pico8_je(), "ra"), vec_of(pico8_mo(), "ra"));
    qa = ui_ref(ok, "qa");
    ic = ui_ref(ok, "ic");
    assert(qa != NULL && ((pico8_vec_t *)qa)->x == want.x && ((pico8_vec_t *)qa)->y == want.y);
    assert(ic == &ok->ic_own && ok->ic_own.layer_count == PICO8_UI_IC_LAYERS);
    assert(ui_ref(ok, "o_") == the_bg() && ui_ref(ok, "gj") == ui_ref(ok, "o_"));
    assert(ui_ref(ok, "le") == class_ref("le") && ui_ref(ok, "lheld") == NULL && ui_ref(ok, "rheld") == NULL);
    h = fold_vec(h, *(pico8_vec_t *)qa);
    h = fold_layers(h, ok);
    pico8_list_add(&ok->ic_own.layers[7], pico8_value_ref(the_bg()));
    pico8_ui_lj(&mem, ok);
    assert(ui_ref(ok, "qa") != qa && ui_ref(ok, "ic") == ic && ui_ref(ok, "gj") == the_bg());
    assert(!in_layer(ok, 7, the_bg()) && in_layer(ok, 1, the_bg()));

    press(ok, 1);
    assert(is_of(ui_ref(ok, "lheld"), pico8_g__struct_class()));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), ui_ref(ok, "lheld")));
    release(ok);
    assert(ui_ref(ok, "lheld") == NULL);
    press(ok, 1);
    assert(is_of(ui_ref(ok, "lheld"), pico8_g__struct_class()));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), ui_ref(ok, "lheld")));
    release(ok);
    assert(ui_ref(ok, "lheld") == NULL);

    press(ok, 2);
    assert(ui_ref(ok, "rheld") == the_bg() && ui_ref(ok, "lheld") == NULL);
    h = fnv(h, cursor_is(9));
    release(ok);
    assert(ui_ref(ok, "rheld") == NULL);
    h = fnv(h, cursor_is(58));

    pico8_lt(&g_giver.header);
    made[0] = pico8_qh_new(NULL);
    made[1] = pico8_os_new(NULL);
    made[2] = pico8_nl_new(NULL);
    made[3] = pico8_hj_new(NULL);
    made[4] = pico8_nr_new(NULL);
    made[5] = pico8_fr_new(NULL);
    made[6] = pico8_mx_new(NULL);
    made[7] = pico8_g__new(NULL);
    made[8] = pico8_mz_new(NULL);
    made[9] = pico8_jc_new(NULL);
    for (i = 0; i < 10; i++) {
        h = fold_given_lheld(h, ok, made[i]);
    }
    g_given = pico8_value_ref(the_bg());
    press(ok, 2);
    assert(ui_ref(ok, "rheld") == the_bg() && ui_ref(ok, "lheld") == NULL);
    h = fnv(h, cursor_is(9));
    release(ok);
    assert(ui_ref(ok, "rheld") == NULL);
    g_given = pico8_value_ref(NULL);

    le = ui_ref(ok, "le");
    call_row_my(pico8_ix(), 4);
    assert(ui_ref(ok, "le") != NULL && ui_ref(ok, "le") != le && ui_ref(ok, "le") != class_ref("le"));
    assert(pico8_type_tag_of(ui_ref(ok, "le")) == PICO8_TYPE_TAG_LIST);
    assert(((const pico8_list_t *)ui_ref(ok, "le"))->count > 0);
    h = fnv(h, (uint32_t)((const pico8_list_t *)ui_ref(ok, "le"))->count);
    le = ui_ref(ok, "le");
    (void)pico8_fv_new(NULL);
    assert(ui_ref(ok, "le") != NULL && ui_ref(ok, "le") != le && ui_ref(ok, "le") != class_ref("le"));
    assert(((const pico8_list_t *)ui_ref(ok, "le"))->count == 0);
    le = ui_ref(ok, "le");
    pico8_ui_cj(ok);
    assert(ui_ref(ok, "le") != NULL && ui_ref(ok, "le") != le && ui_ref(ok, "le") != class_ref("le"));
    assert(((const pico8_list_t *)ui_ref(ok, "le"))->count == 0);

    for (i = 0; i < 10; i++) {
        g_write_name = ui_names[i];
        must_abort(write_named, ok);
    }
    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        v = pico8_value_fixed(0x1234);
        assert(!pico8_ui_struct_class()->field(ok, intruders[i], &v));
        assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x1234);
    }
    h = fnv(h, (uint32_t)pico8_hv()->count);
    g_giver_hovered = 0;
    for (i = 0; i < 10; i++) {
        pico8_he(made[i]);
    }
    pico8_he(the_bg());
    pico8_ui_lj(&mem, ok);
    assert(ui_ref(ok, "o_") == NULL && ui_ref(ok, "gj") == NULL);
    check("ui members", h, UI_MEMBERS_HASH);
}

static int g_hold_buttons;

static void hold_given(void *ok) {
    press(ok, g_hold_buttons);
}

static void test_ui_held_stops_off_row(void) {
    pico8_ui_t *ok = start_game();
    pico8_vec_t *vec = malloc(sizeof *vec);
    void *left[11];
    void *right[9];
    size_t i;

    *vec = pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(2));
    pico8_lt(&g_giver.header);
    left[0] = pico8_pm_new(NULL);
    left[1] = pico8_ga_new(NULL);
    left[2] = pico8_cg("jo")->items[0].as.ref;
    left[3] = the_bg();
    left[4] = pico8_fv_new(NULL);
    left[5] = pico8_js_new(NULL);
    left[6] = pico8_ie_new(NULL);
    left[7] = pico8_mz_jj_wrapper();
    left[8] = pico8_ob_table_new();
    left[9] = (void *)pico8_hv();
    left[10] = vec;
    right[0] = pico8_jc_new(NULL);
    right[1] = pico8_g__new(NULL);
    right[2] = pico8_mz_new(NULL);
    right[3] = pico8_qh_new(NULL);
    right[4] = left[2];
    right[5] = left[4];
    right[6] = left[8];
    right[7] = left[9];
    right[8] = vec;

    g_hold_buttons = 1;
    for (i = 0; i < 11; i++) {
        g_given = pico8_value_ref(left[i]);
        must_abort(hold_given, ok);
    }
    g_hold_buttons = 2;
    for (i = 0; i < 9; i++) {
        g_given = pico8_value_ref(right[i]);
        must_abort(hold_given, ok);
    }
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

    (void)start_game();
    must_abort(ks_on_a_table, table);
    must_abort(lt_a_table, table);
    pico8_ob_table_free(table);
    pico8_ke_ks(NULL, pico8_value_string("ph"));
}

static int me_of(const void *e, const char *name, pico8_value_t *out) {
    const pico8_struct_header_t *header = e;
    pico8_value_t raw = pico8_value_fixed(0x1234);
    pico8_value_t got = pico8_value_fixed(0x1234);
    int raw_found = header->cls->field(e, name, &raw);
    int found = pico8_entity_get_field(e, name, &got);

    assert(raw_found == found);
    if (!found) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        assert(got.kind == PICO8_VALUE_FIXED && got.as.fixed == 0x1234);
        return 0;
    }
    assert(raw.kind == got.kind && pico8_value_equal(raw, got));
    *out = got;
    return 1;
}

static uint32_t fold_string(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0u);
}

static const char *spec_me(int (*reader)(const char *, pico8_value_t *)) {
    pico8_value_t v;

    assert(reader("me", &v) && v.kind == PICO8_VALUE_STRING);
    return v.as.string;
}

static int identity_of(const char *s) {
    if (s == spec_me(pico8_ke_default)) {
        return 1;
    }
    if (s == spec_me(pico8_jo_default)) {
        return 2;
    }
    if (s == spec_me(pico8_qh_default)) {
        return 3;
    }
    if (s == spec_me(pico8_pm_default)) {
        return 4;
    }
    return 0;
}

static uint32_t fold_ks_back(uint32_t h, void *e) {
    pico8_value_t v;

    assert(me_of(e, "t", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0);
    return fnv(h, (uint32_t)v.as.fixed);
}

static void test_ke_me_by_name(void) {
    static const char *const want[24] = {"ph", "ph",      "ph", "ph", "unknown", "kz", "kz", "kz",
                                         "kz", "ly",      "unknown", "kz", "kz", "kz", "ph", "ph",
                                         "ph", "ph",      "ph", "kz", "ph", "ph", "ph", "kz"};
    static const int identity[24] = {0, 0, 0, 0, 2, 3, 3, 3, 3, 4, 2, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static const char *const intruders[] = {"", "m", "mee", "me ", "has_me", "ke"};
    char s[] = "ll";
    void *e[24];
    const pico8_list_t *hv;
    pico8_value_t v;
    pico8_value_t through_hj;
    uint32_t h = 2166136261u;
    size_t i;

    (void)start_game();
    hv = pico8_hv();
    assert(hv != NULL);
    h = fnv(h, (uint32_t)hv->count);
    for (i = 0; i < (size_t)hv->count; i++) {
        assert(hv->items[i].kind == PICO8_VALUE_REF);
        if (me_of(hv->items[i].as.ref, "me", &v)) {
            h = fnv(fnv(h, 1u), (uint32_t)v.kind);
            h = v.kind == PICO8_VALUE_STRING ? fold_string(h, v.as.string) : h;
        } else {
            h = fnv(h, 0u);
        }
    }

    e[0] = pico8_bg_new(NULL);
    e[1] = pico8_nf_new(NULL);
    e[2] = pico8_mh_new(NULL);
    e[3] = pico8_iv_new(NULL);
    e[4] = pico8_jo_new(NULL);
    e[5] = pico8_qh_new(NULL);
    e[6] = pico8_nl_new(NULL);
    e[7] = pico8_os_new(NULL);
    e[8] = pico8_hj_new(NULL);
    e[9] = pico8_pm_new(NULL);
    e[10] = pico8_ga_new(NULL);
    e[11] = pico8_nr_new(NULL);
    e[12] = pico8_fr_new(NULL);
    e[13] = pico8_mx_new(NULL);
    e[14] = pico8_g__new(NULL);
    e[15] = pico8_fv_new(NULL);
    e[16] = pico8_ie_new(NULL);
    e[17] = pico8_pj_new(NULL);
    e[18] = pico8_qb_new(NULL);
    e[19] = pico8_jc_new(NULL);
    e[20] = pico8_ui_new(NULL);
    e[21] = pico8_js_new(NULL);
    e[22] = pico8_fq_new(NULL);
    e[23] = pico8_mz_new(NULL);
    for (i = 0; i < 24; i++) {
        assert(me_of(e[i], "me", &v) && v.kind == PICO8_VALUE_STRING && v.as.string != NULL);
        assert(strcmp(v.as.string, want[i]) == 0);
        assert(identity_of(v.as.string) == identity[i]);
        h = fold_string(fnv(h, (uint32_t)v.kind), v.as.string);
        h = fnv(h, (uint32_t)identity_of(v.as.string));
    }

    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        v = pico8_value_fixed(0x1234);
        assert(pico8_ke_field(e[0], intruders[i], &v) == 0);
        assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x1234);
        h = fnv(h, 0u);
    }

    pico8_ke_ks(e[0], pico8_value_string(s));
    assert(me_of(e[0], "me", &v) && v.kind == PICO8_VALUE_STRING && v.as.string == s);
    h = fold_ks_back(fold_string(fnv(h, 1u), v.as.string), e[0]);
    pico8_ke_ks(e[0], pico8_value_ref(NULL));
    assert(!me_of(e[0], "me", &v));
    h = fold_ks_back(fnv(h, 0u), e[0]);
    pico8_ke_ks(e[0], pico8_value_string("ph"));
    assert(me_of(e[0], "me", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, "ph") == 0);
    h = fold_ks_back(fold_string(fnv(h, 1u), v.as.string), e[0]);

    pico8_ke_ks(e[9], pico8_value_string(s));
    assert(me_of(e[9], "me", &v) && v.kind == PICO8_VALUE_STRING && v.as.string == s);
    assert(pico8_hj_struct_class()->field(e[9], "me", &through_hj) && pico8_value_equal(v, through_hj));
    h = fold_ks_back(fold_string(fnv(h, 1u), v.as.string), e[9]);

    check("ke_me", h, KE_ME_HASH);
}

static void ks_fixed(void *e) {
    pico8_ke_ks(e, pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void ks_true(void *e) {
    pico8_ke_ks(e, pico8_value_bool(1));
}

static void ks_false(void *e) {
    pico8_ke_ks(e, pico8_value_bool(0));
}

static void ks_zero_false(void *e) {
    pico8_value_t v;

    memset(&v, 0, sizeof v);
    v.kind = PICO8_VALUE_BOOL;
    pico8_ke_ks(e, v);
}

static void ks_zero_fixed(void *e) {
    pico8_value_t v;

    memset(&v, 0, sizeof v);
    v.kind = PICO8_VALUE_FIXED;
    pico8_ke_ks(e, v);
}

static void ks_struct(void *e) {
    pico8_ke_ks(e, pico8_value_ref(e));
}

static void ks_null_string(void *e) {
    pico8_ke_ks(e, pico8_value_string(NULL));
}

static void test_ke_ks_stops_off_row(void) {
    void *bg;

    (void)start_game();
    bg = pico8_bg_new(NULL);
    must_abort(ks_fixed, bg);
    must_abort(ks_true, bg);
    must_abort(ks_false, bg);
    must_abort(ks_zero_false, bg);
    must_abort(ks_zero_fixed, bg);
    must_abort(ks_struct, bg);
    must_abort(ks_null_string, bg);
}

static uint32_t fold_t(uint32_t h, const void *e) {
    pico8_value_t v;

    assert(me_of(e, "t", &v) && v.kind == PICO8_VALUE_FIXED);
    return fnv(fnv(h, (uint32_t)v.kind), (uint32_t)v.as.fixed);
}

static uint32_t fold_hv_t(uint32_t h) {
    const pico8_list_t *hv = pico8_hv();
    int i;

    assert(hv != NULL);
    h = fnv(h, (uint32_t)hv->count);
    for (i = 0; i < hv->count; i++) {
        assert(hv->items[i].kind == PICO8_VALUE_REF);
        h = fold_t(h, hv->items[i].as.ref);
    }
    return h;
}

static void assert_fixed_by_name(const void *e, const char *name, int want) {
    pico8_value_t v;

    assert(me_of(e, name, &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(want));
}

static void test_ke_t_by_name(void) {
    static const char *const intruders[] = {"", "T", "tt", "t ", "has_t", "ke"};
    void *e[24];
    pico8_ob_table_t *b;
    void *keyed;
    void *zero;
    pico8_value_t v;
    pico8_value_t through_hj;
    uint32_t h = 2166136261u;
    size_t i;

    (void)start_game();
    h = fold_hv_t(h);
    pico8_dg();
    h = fold_hv_t(h);

    e[0] = pico8_bg_new(NULL);
    e[1] = pico8_nf_new(NULL);
    e[2] = pico8_mh_new(NULL);
    e[3] = pico8_iv_new(NULL);
    e[4] = pico8_jo_new(NULL);
    e[5] = pico8_qh_new(NULL);
    e[6] = pico8_nl_new(NULL);
    e[7] = pico8_os_new(NULL);
    e[8] = pico8_hj_new(NULL);
    e[9] = pico8_pm_new(NULL);
    e[10] = pico8_ga_new(NULL);
    e[11] = pico8_nr_new(NULL);
    e[12] = pico8_fr_new(NULL);
    e[13] = pico8_mx_new(NULL);
    e[14] = pico8_g__new(NULL);
    e[15] = pico8_fv_new(NULL);
    e[16] = pico8_ie_new(NULL);
    e[17] = pico8_pj_new(NULL);
    e[18] = pico8_qb_new(NULL);
    e[19] = pico8_jc_new(NULL);
    e[20] = pico8_ui_new(NULL);
    e[21] = pico8_js_new(NULL);
    e[22] = pico8_fq_new(NULL);
    e[23] = pico8_mz_new(NULL);
    for (i = 0; i < 24; i++) {
        assert_fixed_by_name(e[i], "t", 0);
        h = fold_t(h, e[i]);
    }

    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        v = pico8_value_fixed(0x1234);
        assert(pico8_ke_field(e[0], intruders[i], &v) == 0);
        assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x1234);
        h = fnv(h, 0u);
    }

    b = pico8_ob_table_new();
    pico8_ob_table_set_name(b, "pn", pico8_value_fixed(pico8_fixed_from_int(7)));
    pico8_ob_table_set_name(b, "t", pico8_value_fixed(pico8_fixed_from_int(3)));
    pico8_ob_table_set_name(b, "du", pico8_value_fixed(pico8_fixed_from_int(10)));
    pico8_ob_table_set_name(b, "py", pico8_value_fixed(pico8_fixed_from_int(1)));
    keyed = pico8_g__new(b);
    assert_fixed_by_name(keyed, "t", 3);
    assert_fixed_by_name(keyed, "pn", 7);
    assert_fixed_by_name(keyed, "du", 10);
    assert_fixed_by_name(keyed, "py", 1);
    h = fold_t(h, keyed);

    b = pico8_ob_table_new();
    pico8_ob_table_set_name(b, "t", pico8_value_fixed(0));
    zero = pico8_g__new(b);
    assert_fixed_by_name(zero, "t", 0);
    h = fold_t(h, zero);

    pico8_ke_ks(keyed, pico8_value_string("ll"));
    assert_fixed_by_name(keyed, "t", 0);
    assert(me_of(keyed, "me", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, "ll") == 0);
    h = fold_string(fold_t(h, keyed), v.as.string);

    pico8_ke_ks(e[9], pico8_value_string("ll"));
    assert_fixed_by_name(e[9], "t", 0);
    assert(me_of(e[9], "t", &v) && pico8_hj_struct_class()->field(e[9], "t", &through_hj) &&
           pico8_value_equal(v, through_hj));
    h = fold_t(h, e[9]);

    check("ke_t", h, KE_T_HASH);
}

static void g__with_t_key(void *value) {
    pico8_ob_table_t *b = pico8_ob_table_new();

    pico8_ob_table_set_name(b, "t", *(const pico8_value_t *)value);
    (void)pico8_g__new(b);
}

static void test_g__t_key_stops_off_row(void) {
    pico8_value_t zero_false;
    pico8_value_t zero_fixed;
    pico8_value_t v;
    pico8_ob_table_t *b;
    void *made;

    (void)start_game();
    memset(&zero_false, 0, sizeof zero_false);
    zero_false.kind = PICO8_VALUE_BOOL;
    v = pico8_value_ref(NULL);
    must_abort(g__with_t_key, &v);
    must_abort(g__with_t_key, &zero_false);
    v = pico8_value_bool(0);
    must_abort(g__with_t_key, &v);
    v = pico8_value_bool(1);
    must_abort(g__with_t_key, &v);
    v = pico8_value_string("3");
    must_abort(g__with_t_key, &v);
    v = pico8_value_string(NULL);
    must_abort(g__with_t_key, &v);
    v = pico8_value_ref(pico8_bg_new(NULL));
    must_abort(g__with_t_key, &v);

    memset(&zero_fixed, 0, sizeof zero_fixed);
    zero_fixed.kind = PICO8_VALUE_FIXED;
    b = pico8_ob_table_new();
    pico8_ob_table_set_name(b, "t", zero_fixed);
    made = pico8_g__new(b);
    assert_fixed_by_name(made, "t", 0);
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

static const char *const jo_names[12] = {"nu", "hf", "hs", "fa", "kq", "ka", "dz", "cq", "kw", "em", "hn", "nk"};

typedef int (*jo_spec_reader_t)(const char *, pico8_value_t *);

/* The generated data (pico8_game_data.h) a bare planet's table member
 * `name` points at; NULL for any other name. */
static const void *jo_member_data(const char *name) {
    static const struct {
        const char *name;
        const void *data;
    } table[] = {{"nu", &pico8_jo_nu_tags}, {"ka", &pico8_jo_ka_seq}, {"dz", pico8_jo_dz_rows},
                 {"cq", &pico8_jo_cq_seq},  {"em", &pico8_jo_em_row}};
    size_t i;

    if (strcmp(name, "kw") == 0) {
        return pico8_jo_shared_kw();
    }
    for (i = 0; i < sizeof table / sizeof table[0]; i++) {
        if (strcmp(table[i].name, name) == 0) {
            return table[i].data;
        }
    }
    return NULL;
}

/* The entry count of the ob table a converted member held: a row's items
 * and its `c`, a kw's `qe` and `mu`. */
static uint32_t data_entries(const char *name, const void *data) {
    if (strcmp(name, "px") == 0 || strcmp(name, "em") == 0) {
        const pico8_jb_row_t *row = data;
        return (uint32_t)(row->count + row->has_c);
    }
    if (strcmp(name, "nu") == 0) {
        return (uint32_t)((const pico8_str_seq_t *)data)->count;
    }
    if (strcmp(name, "dz") == 0) {
        return PICO8_JO_DZ_COUNT;
    }
    if (strcmp(name, "kw") == 0) {
        return (uint32_t)(((const pico8_kw_t *)data)->has_qe + 1);
    }
    return (uint32_t)((const pico8_int_seq_t *)data)->count;
}

/* 1: jo's own default; 2: the one of `own`, the spec of the genre that
 * holds its own default (`own_nu`, its generated `nu`, for that name); 0:
 * any other. A converted name other than `nu` is never a genre's own. */
static int jo_identity(const char *name, const void *ref, jo_spec_reader_t own, const void *own_nu) {
    const void *jo_data = jo_member_data(name);
    pico8_value_t d;

    if (jo_data != NULL ? jo_data == ref
                        : pico8_jo_default(name, &d) && d.kind == PICO8_VALUE_REF && d.as.ref == ref) {
        return 1;
    }
    if (strcmp(name, "nu") == 0) {
        return own_nu != NULL && own_nu == ref ? 2 : 0;
    }
    if (own != NULL && jo_data == NULL && own(name, &d) && d.kind == PICO8_VALUE_REF && d.as.ref == ref) {
        return 2;
    }
    return 0;
}

static uint32_t fold_jo_member(uint32_t h, const void *e, const char *name, jo_spec_reader_t own, const void *own_nu,
                               pico8_value_t *out,
                               int *identity) {
    pico8_value_t v;

    *identity = -1;
    if (!me_of(e, name, &v)) {
        return fnv(h, 0u);
    }
    h = fnv(fnv(h, 1u), (uint32_t)v.kind);
    if (v.kind == PICO8_VALUE_FIXED) {
        h = fnv(h, (uint32_t)v.as.fixed);
    } else if (v.kind == PICO8_VALUE_BOOL) {
        h = fnv(h, (uint32_t)v.as.boolean);
    } else if (v.kind == PICO8_VALUE_REF) {
        *identity = jo_identity(name, v.as.ref, own, own_nu);
        h = fnv(h, (uint32_t)*identity);
    }
    *out = v;
    return h;
}

static int jo_accessor_is(const char *name, const void *got) {
    return got != NULL && jo_member_data(name) == got;
}

static int jo_tree(const void *e) {
    const pico8_struct_class_t *cls = ((const pico8_struct_header_t *)e)->cls;

    return cls == pico8_jo_struct_class() || cls == pico8_qh_struct_class() || cls == pico8_nl_struct_class() ||
           cls == pico8_os_struct_class() || cls == pico8_hj_struct_class() || cls == pico8_pm_struct_class() ||
           cls == pico8_ga_struct_class() || cls == pico8_nr_struct_class() || cls == pico8_fr_struct_class() ||
           cls == pico8_mx_struct_class();
}

static void test_jo_members_by_name(void) {
    static const jo_spec_reader_t own[10] = {NULL,           pico8_qh_default, pico8_nl_default, pico8_os_default,
                                             pico8_hj_default, pico8_pm_default, pico8_ga_default, pico8_nr_default,
                                             pico8_nr_default, pico8_mx_default};
    static const void *const own_nu[10] = {NULL,           &pico8_qh_nu_tags, &pico8_nl_nu_tags, &pico8_os_nu_tags,
                                           &pico8_hj_nu_tags, &pico8_pm_nu_tags, &pico8_ga_nu_tags, &pico8_nr_nu_tags,
                                           &pico8_nr_nu_tags, &pico8_mx_nu_tags};
    static const int nu_identity[10] = {1, 2, 2, 2, 2, 2, 2, 2, 2, 2};
    static const int kq_identity[10] = {1, 1, 1, 1, 1, 1, 2, 2, 2, 1};
    static const int lz_at_pq[10] = {0, 1, 1, 1, 1, 1, 0, 1, 1, 1};
    static const char *const intruders[] = {"", "n", "nuu", "hf ", "hf_is_bool", "has_hf", "jo"};
    const pico8_list_t *hv;
    void *e[10];
    pico8_value_t v;
    int identity;
    uint32_t h = 2166136261u;
    size_t i;
    size_t m;

    (void)start_game();
    hv = pico8_hv();
    assert(hv != NULL);
    h = fnv(h, (uint32_t)hv->count);
    for (i = 0; i < (size_t)hv->count; i++) {
        assert(hv->items[i].kind == PICO8_VALUE_REF);
        if (!jo_tree(hv->items[i].as.ref)) {
            h = fnv(h, 7u);
            continue;
        }
        for (m = 0; m < 12; m++) {
            h = fold_jo_member(h, hv->items[i].as.ref, jo_names[m], NULL, NULL, &v, &identity);
        }
    }

    e[0] = pico8_jo_new(NULL);
    e[1] = pico8_qh_new(NULL);
    e[2] = pico8_nl_new(NULL);
    e[3] = pico8_os_new(NULL);
    e[4] = pico8_hj_new(NULL);
    e[5] = pico8_pm_new(NULL);
    e[6] = pico8_ga_new(NULL);
    e[7] = pico8_nr_new(NULL);
    e[8] = pico8_fr_new(NULL);
    e[9] = pico8_mx_new(NULL);
    for (i = 0; i < 10; i++) {
        for (m = 0; m < 12; m++) {
            const char *n = jo_names[m];

            v = pico8_value_fixed(0x1234);
            h = fold_jo_member(h, e[i], n, own[i], own_nu[i], &v, &identity);
            if (strcmp(n, "hf") == 0) {
                assert(i == 0 ? v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(1)
                              : v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
            } else if (strcmp(n, "hs") == 0) {
                assert(v.kind == PICO8_VALUE_FIXED);
                assert(v.as.fixed == (i == 3 ? pico8_fixed_from_int(1) / 2 : pico8_fixed_from_int(1)));
            } else if (strcmp(n, "fa") == 0) {
                assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(i == 6 ? 3 : 6));
            } else if (strcmp(n, "nu") == 0) {
                assert(v.kind == PICO8_VALUE_REF && identity == nu_identity[i]);
            } else if (strcmp(n, "kq") == 0) {
                assert(v.kind == PICO8_VALUE_REF && identity == kq_identity[i]);
                assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN);
            } else if (strcmp(n, "hn") == 0) {
                assert(lz_at_pq[i] ? v.kind == PICO8_VALUE_REF && identity == 0
                                   : identity == -1 && v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x1234);
            } else if (strcmp(n, "nk") == 0) {
                assert(identity == -1 && v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x1234);
            } else {
                assert(v.kind == PICO8_VALUE_REF && identity == (strcmp(n, "kw") == 0 && lz_at_pq[i] ? 0 : 1));
                assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN);
            }
        }
    }

    assert(jo_accessor_is("dz", pico8_jo_dz()));
    assert(jo_accessor_is("cq", pico8_jo_cq()));
    assert(jo_accessor_is("ka", pico8_jo_ka()));
    assert(jo_accessor_is("em", pico8_jo_em()));

    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        v = pico8_value_fixed(0x1234);
        assert(pico8_jo_struct_class()->field(e[0], intruders[i], &v) == 0);
        assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x1234);
        h = fnv(h, 0u);
    }

    check("jo_members", h, JO_MEMBERS_HASH);
}

static pico8_hp_t *gb_project(int want_go) {
    const pico8_ob_table_t *gb = pico8_gb();
    pico8_value_t v;
    int k;

    for (k = 1; pico8_ob_table_get_index(gb, k, &v); k++) {
        pico8_hp_t *hp;

        assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
        hp = v.as.ref;
        if (want_go ? hp->go != NULL : hp->go == NULL && hp->cw == NULL) {
            return hp;
        }
    }
    assert(0 && "gb holds a project of each kind");
    return NULL;
}

static uint32_t fold_written(uint32_t h, const void *e, const char *name, const void *want) {
    pico8_value_t v;

    if (!me_of(e, name, &v)) {
        assert(want == NULL);
        return fnv(h, 0u);
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref == want);
    return fnv(h, 1u);
}

static void write_by_name(void *e, const char *name, pico8_value_t v) {
    const pico8_struct_header_t *header = e;

    assert(header->cls->set_field(e, name, v) == 1);
    pico8_entity_set_field(e, name, v);
}

static void test_jo_written_by_name(void) {
    static pico8_kw_t kw;
    static const pico8_df_entry_t nk = {"zz", 1, 0, NULL, NULL};
    pico8_hp_t *plain;
    pico8_hp_t *with_go;
    void *e[5];
    void *moved[2];
    pico8_value_t v;
    uint32_t h = 2166136261u;
    size_t i;

    (void)start_game();
    plain = gb_project(0);
    with_go = gb_project(1);

    moved[0] = pico8_jo_new(NULL);
    moved[1] = pico8_pm_new(NULL);
    for (i = 0; i < 2; i++) {
        pico8_jo_lz(moved[i], plain);
        h = fold_written(h, moved[i], "hn", plain);
        assert(me_of(moved[i], "kw", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
        assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN);
        assert(v.as.ref != pico8_jo_shared_kw());
        assert(((const pico8_kw_t *)v.as.ref)->mu == 0);
        h = fnv(h, data_entries("kw", v.as.ref));

        pico8_jo_hh(moved[i]);
        if (me_of(moved[i], "nk", &v)) {
            assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
            assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN);
            h = fnv(h, 1u);
        } else {
            h = fnv(h, 0u);
        }
        h = fnv(h, me_of(moved[i], "ep", &v) ? 1u + (uint32_t)v.kind : 0u);

        pico8_jo_hu(moved[i], with_go);
        assert(pico8_df_lookup(with_go->go) != NULL);
        h = fold_written(h, moved[i], "nk", pico8_df_lookup(with_go->go));
    }

    e[0] = pico8_jo_new(NULL);
    e[1] = pico8_qh_new(NULL);
    e[2] = pico8_pm_new(NULL);
    e[3] = pico8_ga_new(NULL);
    e[4] = pico8_fr_new(NULL);
    for (i = 0; i < 5; i++) {
        write_by_name(e[i], "kw", pico8_value_ref(&kw));
        h = fold_written(h, e[i], "kw", &kw);
        write_by_name(e[i], "kw", pico8_value_ref(NULL));
        h = fold_written(h, e[i], "kw", NULL);
        write_by_name(e[i], "hn", pico8_value_ref(plain));
        h = fold_written(h, e[i], "hn", plain);
        write_by_name(e[i], "hn", pico8_value_ref(NULL));
        h = fold_written(h, e[i], "hn", NULL);
        write_by_name(e[i], "nk", pico8_value_ref((void *)&nk));
        h = fold_written(h, e[i], "nk", &nk);
        write_by_name(e[i], "nk", pico8_value_ref(NULL));
        h = fold_written(h, e[i], "nk", NULL);
    }

    check("jo_written", h, JO_WRITTEN_HASH);
}

static const char *g_off_row_name;
static pico8_value_t g_off_row_value;

static void write_off_row(void *e) {
    pico8_entity_set_field(e, g_off_row_name, g_off_row_value);
}

static pico8_value_t zero_union(pico8_value_kind_t kind) {
    pico8_value_t v;

    memset(&v, 0, sizeof v);
    v.kind = kind;
    return v;
}

static void test_jo_writer_stops_off_row(void) {
    static const char *const names[3] = {"kw", "hn", "nk"};
    pico8_value_t off_row[11];
    void *e[2];
    size_t n_off;
    size_t i;
    size_t k;
    size_t m;

    (void)start_game();
    e[0] = pico8_jo_new(NULL);
    e[1] = pico8_pm_new(NULL);
    for (i = 0; i < 2; i++) {
        for (m = 0; m < 3; m++) {
            n_off = 0;
            off_row[n_off++] = pico8_value_fixed(pico8_fixed_from_int(1));
            off_row[n_off++] = pico8_value_bool(1);
            off_row[n_off++] = pico8_value_string("x");
            off_row[n_off++] = zero_union(PICO8_VALUE_FIXED);
            off_row[n_off++] = zero_union(PICO8_VALUE_BOOL);
            off_row[n_off++] = zero_union(PICO8_VALUE_STRING);
            off_row[n_off++] = zero_union(PICO8_VALUE_FUNCTION);
            off_row[n_off++] = pico8_value_ref((void *)pico8_hv());
            off_row[n_off++] = pico8_value_ref(e[i]);
            off_row[n_off++] = strcmp(names[m], "hn") == 0 ? pico8_value_ref(pico8_ob_table_new())
                                                           : pico8_value_ref(gb_project(0));
            if (strcmp(names[m], "hn") != 0) {
                off_row[n_off++] = pico8_value_ref(pico8_ob_table_new());
            }
            g_off_row_name = names[m];
            for (k = 0; k < n_off; k++) {
                g_off_row_value = off_row[k];
                must_abort(write_off_row, e[i]);
            }
        }
    }
}

static pico8_vec_t *heap_vec(int x, int y) {
    pico8_vec_t *v = malloc(sizeof *v);

    v->x = pico8_fixed_from_int(x);
    v->y = pico8_fixed_from_int(y);
    return v;
}

/* Presence, then `x`/`y` of a vector field; asserts the same pointer as
 * `want` when `want` is not NULL. */
static uint32_t fold_position(uint32_t h, const void *e, const char *name, const void *want) {
    pico8_value_t v;

    if (!me_of(e, name, &v)) {
        assert(want == NULL);
        return fnv(h, 0u);
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN);
    assert(want == NULL || v.as.ref == want);
    return fold_vec(fnv(h, 1u), *(const pico8_vec_t *)v.as.ref);
}

static uint32_t fold_jo_position(uint32_t h, const void *e) {
    pico8_value_t v;
    pico8_value_t d;

    h = fold_position(h, e, "p", NULL);
    h = fold_position(h, e, "ra", NULL);
    h = fold_position(h, e, "gq", NULL);
    assert(me_of(e, "gq", &v) && pico8_jo_default("gq", &d));
    h = fnv(h, (uint32_t)(v.as.ref == d.as.ref));
    assert(me_of(e, "ca", &v) && v.kind == PICO8_VALUE_FIXED);
    return fnv(h, (uint32_t)v.as.fixed);
}

static void *last_of_class(const pico8_struct_class_t *cls) {
    const pico8_list_t *hv = pico8_hv();
    int i;

    for (i = hv->count - 1; i >= 0; i--) {
        if (hv->items[i].kind == PICO8_VALUE_REF && hv->items[i].as.ref != NULL &&
            pico8_type_tag_of(hv->items[i].as.ref) == PICO8_TYPE_TAG_STRUCT &&
            ((const pico8_struct_header_t *)hv->items[i].as.ref)->cls == cls) {
            return hv->items[i].as.ref;
        }
    }
    return NULL;
}

static void *position_new(int k, const pico8_ob_table_t *b) {
    switch (k) {
    case 0:
        return pico8_jo_new(b);
    case 1:
        return pico8_pm_new(b);
    case 2:
        return pico8_ga_new(b);
    default:
        return pico8_nr_new(b);
    }
}

static void test_jo_position_by_name(void) {
    static const char *const intruders[] = {"", "r", "raa", "p ", "P", "gq_", "has_ca", "ca_is_bool"};
    const pico8_list_t *hv;
    void *e[10];
    void *made;
    pico8_vec_t *pv = heap_vec(3, 4);
    pico8_vec_t *rv = heap_vec(384, 512);
    pico8_vec_t before;
    pico8_ob_table_t *b;
    pico8_value_t v;
    pico8_value_t d;
    pico8_hp_t *to_pm = NULL;
    uint32_t h = 2166136261u;
    size_t i;
    int k;

    (void)start_game();
    hv = pico8_hv();
    h = fnv(h, (uint32_t)hv->count);
    for (i = 0; i < (size_t)hv->count; i++) {
        assert(hv->items[i].kind == PICO8_VALUE_REF);
        h = jo_tree(hv->items[i].as.ref) ? fold_jo_position(h, hv->items[i].as.ref) : fnv(h, 7u);
    }

    e[0] = pico8_jo_new(NULL);
    e[1] = pico8_qh_new(NULL);
    e[2] = pico8_nl_new(NULL);
    e[3] = pico8_os_new(NULL);
    e[4] = pico8_hj_new(NULL);
    e[5] = pico8_pm_new(NULL);
    e[6] = pico8_ga_new(NULL);
    e[7] = pico8_nr_new(NULL);
    e[8] = pico8_fr_new(NULL);
    e[9] = pico8_mx_new(NULL);
    assert(pico8_jo_default("gq", &d) && d.kind == PICO8_VALUE_REF && d.as.ref != NULL);
    for (i = 0; i < 10; i++) {
        h = fold_jo_position(h, e[i]);
        assert(me_of(e[i], "gq", &v) && v.as.ref == d.as.ref);
        assert(((const pico8_vec_t *)v.as.ref)->x == 0 && ((const pico8_vec_t *)v.as.ref)->y == pico8_fixed_from_int(11));
        assert(me_of(e[i], "ca", &v) && v.as.fixed == pico8_fixed_from_int(9));
    }
    assert(!me_of(e[0], "p", &v) && !me_of(e[0], "ra", &v));

    for (k = 0; k < 4; k++) {
        b = pico8_ob_table_new();
        pico8_ob_table_set_name(b, "p", pico8_value_ref(pv));
        pico8_ob_table_set_name(b, "ra", pico8_value_ref(rv));
        made = position_new(k, b);
        h = fold_position(h, made, "p", pv);
        h = fold_position(h, made, "ra", rv);
        b = pico8_ob_table_new();
        pico8_ob_table_set_name(b, "p", pico8_value_ref(NULL));
        made = position_new(k, b);
        h = fold_position(h, made, "p", NULL);
        h = fnv(h, (uint32_t)me_of(made, "p", &v));
    }

    for (i = 0; i < 10; i++) {
        const pico8_struct_header_t *header = e[i];
        int takes = i != 0 && i != 6;
        pico8_vec_t *w = heap_vec((int)i, 7);
        pico8_value_t old_p;
        int had_p = me_of(e[i], "p", &old_p);

        assert(header->cls->set_field(e[i], "ra", pico8_value_ref(w)) == takes);
        assert(header->cls->set_field(e[i], "p", pico8_value_ref(w)) == takes);
        if (!takes) {
            assert(me_of(e[i], "p", &v) == had_p && (!had_p || v.as.ref == old_p.as.ref));
            h = fnv(h, 9u);
            continue;
        }
        h = fold_position(h, e[i], "ra", w);
        h = fold_position(h, e[i], "p", w);
        pico8_entity_set_field(e[i], "ra", pico8_value_ref(NULL));
        pico8_entity_set_field(e[i], "p", pico8_value_ref(NULL));
        h = fnv(h, (uint32_t)me_of(e[i], "ra", &v));
        h = fnv(h, (uint32_t)me_of(e[i], "p", &v));
    }

    pico8_bv(e[1], pico8_value_ref(rv), pico8_value_ref(heap_vec(1, 1)));
    h = fold_position(h, e[1], "ra", rv);
    assert(me_of(e[1], "p", &v) && v.as.ref != rv);
    h = fold_position(h, e[1], "p", v.as.ref);
    assert(((const pico8_vec_t *)v.as.ref)->x == pico8_fixed_from_int(3));
    before = *rv;
    pico8_na(e[1], pico8_fixed_from_int(20));
    h = fold_position(h, e[1], "ra", rv);
    assert(rv->x == before.x && rv->y != before.y);

    for (k = 1; pico8_ob_table_get_index(pico8_gb(), k, &v); k++) {
        pico8_hp_t *hp = v.as.ref;

        if (hp->cw != NULL && strcmp(hp->cw, "pm") == 0) {
            to_pm = hp;
            break;
        }
    }
    assert(to_pm != NULL);
    b = pico8_ob_table_new();
    pico8_ob_table_set_name(b, "p", pico8_value_ref(pv));
    pico8_ob_table_set_name(b, "ra", pico8_value_ref(rv));
    made = pico8_jo_new(b);
    pico8_jo_hu(made, to_pm);
    made = last_of_class(pico8_pm_struct_class());
    assert(made != NULL);
    h = fold_position(h, made, "p", pv);
    h = fold_position(h, made, "ra", rv);

    b = pico8_ob_table_new();
    pico8_ob_table_set_name(b, "ra", pico8_value_ref(rv));
    (void)pico8_pm_new(b);
    made = last_of_class(pico8_ga_struct_class());
    h = fnv(h, (uint32_t)(made != NULL));
    if (made != NULL) {
        h = fold_position(h, made, "ra", NULL);
        assert(me_of(made, "ra", &v) && v.as.ref != rv);
    }

    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        v = pico8_value_fixed(0x1234);
        assert(pico8_jo_struct_class()->field(e[0], intruders[i], &v) == 0);
        assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x1234);
        h = fnv(h, 0u);
    }

    check("jo_position", h, JO_POSITION_HASH);
}

static int g_position_new;
static const char *g_position_key;

static void build_off_row(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();

    (void)e;
    pico8_ob_table_set_name(b, g_position_key, g_off_row_value);
    (void)position_new(g_position_new, b);
}

static void test_jo_position_stops_off_row(void) {
    static const char *const names[2] = {"ra", "p"};
    pico8_value_t off_row[10];
    void *e[3];
    size_t n_off;
    size_t i;
    size_t k;
    size_t m;

    (void)start_game();
    e[0] = pico8_qh_new(NULL);
    e[1] = pico8_pm_new(NULL);
    e[2] = pico8_nr_new(NULL);
    for (i = 0; i < 3; i++) {
        n_off = 0;
        off_row[n_off++] = pico8_value_fixed(pico8_fixed_from_int(1));
        off_row[n_off++] = pico8_value_bool(1);
        off_row[n_off++] = pico8_value_string("x");
        off_row[n_off++] = zero_union(PICO8_VALUE_FIXED);
        off_row[n_off++] = zero_union(PICO8_VALUE_BOOL);
        off_row[n_off++] = zero_union(PICO8_VALUE_STRING);
        off_row[n_off++] = zero_union(PICO8_VALUE_FUNCTION);
        off_row[n_off++] = pico8_value_ref(pico8_ob_table_new());
        off_row[n_off++] = pico8_value_ref((void *)pico8_hv());
        off_row[n_off++] = pico8_value_ref(e[i]);
        for (m = 0; m < 2; m++) {
            g_off_row_name = names[m];
            for (k = 0; k < n_off; k++) {
                g_off_row_value = off_row[k];
                must_abort(write_off_row, e[i]);
            }
        }
    }
    for (i = 0; i < 4; i++) {
        g_position_new = (int)i;
        n_off = 0;
        off_row[n_off++] = pico8_value_fixed(pico8_fixed_from_int(1));
        off_row[n_off++] = zero_union(PICO8_VALUE_BOOL);
        off_row[n_off++] = pico8_value_ref(pico8_ob_table_new());
        off_row[n_off++] = pico8_value_ref(e[0]);
        for (m = 0; m < 2; m++) {
            g_position_key = names[1 - m];
            for (k = 0; k < n_off; k++) {
                g_off_row_value = off_row[k];
                must_abort(build_off_row, NULL);
            }
        }
    }
}

static const char *const state_names[8] = {"id", "oe", "ep", "jy", "fx", "lq", "jh", "p_"};

static uint32_t fold_str(uint32_t h, const char *s) {
    while (*s != '\0') {
        h = fnv(h, (uint32_t)(unsigned char)*s++);
    }
    return fnv(h, 0x100u);
}

/* Presence, then the content of each of the 8 state members, each asserted
 * in the member's type. */
static uint32_t fold_jo_state(uint32_t h, const void *e) {
    size_t m;

    for (m = 0; m < 8; m++) {
        const char *n = state_names[m];
        pico8_value_t v;

        if (!me_of(e, n, &v)) {
            h = fnv(h, 0u);
            continue;
        }
        h = fnv(fnv(h, 1u), (uint32_t)v.kind);
        if (strcmp(n, "id") == 0 || strcmp(n, "ep") == 0) {
            assert(v.kind == PICO8_VALUE_STRING && v.as.string != NULL);
            h = fold_str(h, v.as.string);
        } else if (strcmp(n, "oe") == 0 || strcmp(n, "jy") == 0) {
            assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
            assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST);
            h = fnv(h, (uint32_t)((const pico8_list_t *)v.as.ref)->count);
        } else if (strcmp(n, "fx") == 0 || strcmp(n, "p_") == 0) {
            assert(v.kind == PICO8_VALUE_BOOL);
            h = fnv(h, (uint32_t)v.as.boolean);
        } else if (strcmp(n, "jh") == 0) {
            assert(v.kind == PICO8_VALUE_FIXED);
            h = fnv(h, (uint32_t)v.as.fixed);
        } else {
            assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL && is_of(v.as.ref, pico8_lq_struct_class()));
        }
    }
    return h;
}

static uint32_t fold_hv_state(uint32_t h) {
    const pico8_list_t *hv = pico8_hv();
    int i;

    h = fnv(h, (uint32_t)hv->count);
    for (i = 0; i < hv->count; i++) {
        assert(hv->items[i].kind == PICO8_VALUE_REF);
        h = jo_tree(hv->items[i].as.ref) ? fold_jo_state(h, hv->items[i].as.ref) : fnv(h, 7u);
    }
    return h;
}

static void *tree_new(int k, const pico8_ob_table_t *b) {
    switch (k) {
    case 0:
        return pico8_jo_new(b);
    case 1:
        return pico8_qh_new(b);
    case 2:
        return pico8_nl_new(b);
    case 3:
        return pico8_os_new(b);
    case 4:
        return pico8_hj_new(b);
    case 5:
        return pico8_pm_new(b);
    case 6:
        return pico8_ga_new(b);
    case 7:
        return pico8_nr_new(b);
    case 8:
        return pico8_fr_new(b);
    default:
        return pico8_mx_new(b);
    }
}

static uint32_t fold_same(uint32_t h, const void *e, const char *name, const void *want) {
    pico8_value_t v;

    if (!me_of(e, name, &v)) {
        assert(want == NULL);
        return fnv(h, 0u);
    }
    assert(want != NULL);
    assert(v.kind == PICO8_VALUE_STRING ? v.as.string == want : v.kind == PICO8_VALUE_REF && v.as.ref == want);
    return fnv(h, 1u);
}

static uint32_t fold_flag(uint32_t h, const void *e, const char *name, pico8_value_kind_t kind) {
    pico8_value_t v;

    if (!me_of(e, name, &v)) {
        return fnv(h, 0u);
    }
    assert(v.kind == kind);
    return fnv(fnv(h, 1u), kind == PICO8_VALUE_BOOL ? (uint32_t)v.as.boolean : (uint32_t)v.as.fixed);
}

static void test_jo_state_by_name(void) {
    static const int lz_at_pq[10] = {0, 1, 1, 1, 1, 1, 0, 1, 1, 1};
    static const int takes_pos[10] = {1, 0, 0, 0, 0, 1, 1, 1, 0, 0};
    static const int writer_of[6] = {0, 1, 5, 6, 7, 8};
    static const char *const intruders[] = {"", "i", "idd", "p_ ", "has_p_", "has_fx", "has_jh", "fx_", "oe "};
    static const char s_id[] = "p42";
    static const char s_ep[] = "an ep";
    pico8_vec_t *pv = heap_vec(3, 4);
    pico8_vec_t *rv = heap_vec(384, 512);
    pico8_hp_t *plain;
    pico8_hp_t *to_pm = NULL;
    pico8_list_t *l = malloc(sizeof *l);
    pico8_list_t *two = malloc(sizeof *two);
    pico8_lq_t *row;
    pico8_ob_table_t *b;
    pico8_value_t v;
    void *e[10];
    void *made;
    pico8_nr_t *nr;
    uint32_t h = 2166136261u;
    size_t i;
    size_t j;
    int k;

    (void)start_game();
    h = fold_hv_state(h);
    plain = gb_project(0);

    for (i = 0; i < 10; i++) {
        e[i] = tree_new((int)i, NULL);
        h = fold_jo_state(h, e[i]);
        assert(!me_of(e[i], "id", &v) && !me_of(e[i], "ep", &v) && !me_of(e[i], "fx", &v));
        assert(!me_of(e[i], "lq", &v) && !me_of(e[i], "jh", &v) && !me_of(e[i], "p_", &v));
        assert(me_of(e[i], "oe", &v) && ((const pico8_list_t *)v.as.ref)->count == 0);
        for (j = 0; j < i; j++) {
            pico8_value_t w;

            assert(me_of(e[j], "oe", &w) && w.as.ref != v.as.ref);
        }
        assert(me_of(e[i], "jy", &v) == lz_at_pq[i]);
    }

    for (k = 0; k < 10; k++) {
        b = pico8_ob_table_new();
        pico8_ob_table_set_name(b, "id", pico8_value_string(s_id));
        made = tree_new(k, b);
        h = fold_same(h, made, "id", s_id);
        h = fold_jo_state(h, made);
        if (takes_pos[k]) {
            pico8_ob_table_set_name(b, "p", pico8_value_ref(pv));
            pico8_ob_table_set_name(b, "ra", pico8_value_ref(rv));
            made = tree_new(k, b);
            h = fold_same(h, made, "id", s_id);
            h = fold_same(h, made, "p", pv);
            h = fold_same(h, made, "ra", rv);
        }
    }

    row = NULL;
    for (k = 1; row == NULL && k < 10; k++) {
        made = tree_new(k, NULL);
        pico8_jo_hb(made);
        h = fnv(h, (uint32_t)me_of(made, "lq", &v));
        row = me_of(made, "lq", &v) ? v.as.ref : NULL;
    }
    assert(row != NULL);
    h = fold_jo_state(h, made);
    pico8_list_init(l);
    for (i = 0; i < 6; i++) {
        void *w = e[writer_of[i]];
        const pico8_struct_header_t *header = w;
        pico8_value_t own_oe;

        assert(me_of(w, "oe", &own_oe));
        write_by_name(w, "ep", pico8_value_string(s_ep));
        h = fold_same(h, w, "ep", s_ep);
        write_by_name(w, "ep", pico8_value_ref(NULL));
        h = fold_same(h, w, "ep", NULL);
        for (j = 0; j < 2; j++) {
            const char *n = j == 0 ? "oe" : "jy";

            write_by_name(w, n, pico8_value_ref(l));
            h = fold_same(h, w, n, l);
            write_by_name(w, n, pico8_value_ref(NULL));
            h = fold_same(h, w, n, NULL);
        }
        for (j = 0; j < 2; j++) {
            const char *n = j == 0 ? "fx" : "p_";

            write_by_name(w, n, pico8_value_bool(1));
            h = fold_flag(h, w, n, PICO8_VALUE_BOOL);
            write_by_name(w, n, pico8_value_bool(0));
            h = fold_flag(h, w, n, PICO8_VALUE_BOOL);
            assert(me_of(w, n, &v) && v.as.boolean == 0);
            write_by_name(w, n, zero_union(PICO8_VALUE_BOOL));
            h = fold_flag(h, w, n, PICO8_VALUE_BOOL);
            assert(me_of(w, n, &v) && v.as.boolean == 0);
            write_by_name(w, n, pico8_value_ref(NULL));
            h = fold_flag(h, w, n, PICO8_VALUE_BOOL);
            assert(!me_of(w, n, &v));
        }
        write_by_name(w, "jh", pico8_value_fixed(pico8_fixed_from_int(3)));
        h = fold_flag(h, w, "jh", PICO8_VALUE_FIXED);
        write_by_name(w, "jh", zero_union(PICO8_VALUE_FIXED));
        h = fold_flag(h, w, "jh", PICO8_VALUE_FIXED);
        assert(me_of(w, "jh", &v) && v.as.fixed == 0);
        write_by_name(w, "jh", pico8_value_ref(NULL));
        h = fold_flag(h, w, "jh", PICO8_VALUE_FIXED);
        assert(!me_of(w, "jh", &v));
        write_by_name(w, "lq", pico8_value_ref(row));
        h = fold_same(h, w, "lq", row);
        write_by_name(w, "lq", pico8_value_ref(NULL));
        h = fold_same(h, w, "lq", NULL);
        assert(header->cls->set_field(w, "id", pico8_value_string(s_id)) == 0);
        write_by_name(w, "oe", own_oe);
        h = fold_jo_state(h, w);
    }

    made = pico8_jo_new(NULL);
    pico8_jo_ll(made, pico8_fixed_from_int(61));
    h = fold_flag(h, made, "p_", PICO8_VALUE_BOOL);
    assert(me_of(made, "p_", &v) && v.as.boolean == 1);
    pico8_jo_ll(made, pico8_fixed_from_int(60));
    h = fold_flag(h, made, "p_", PICO8_VALUE_BOOL);
    assert(me_of(made, "p_", &v) && v.as.boolean == 0);
    write_by_name(made, "ep", pico8_value_string(s_ep));
    pico8_jo_lz(made, plain);
    assert(!me_of(made, "ep", &v) && me_of(made, "jy", &v));
    h = fold_jo_state(h, made);
    pico8_jo_hh(made);
    h = fold_jo_state(h, made);
    made = pico8_jo_new(NULL);
    pico8_jo_hu(made, plain);
    assert(me_of(made, "fx", &v) && v.as.boolean == 1);
    h = fold_jo_state(h, made);

    for (k = 1; pico8_ob_table_get_index(pico8_gb(), k, &v); k++) {
        pico8_hp_t *hp = v.as.ref;

        if (hp->cw != NULL && strcmp(hp->cw, "pm") == 0) {
            to_pm = hp;
            break;
        }
    }
    assert(to_pm != NULL);
    b = pico8_ob_table_new();
    pico8_ob_table_set_name(b, "id", pico8_value_string(s_id));
    pico8_ob_table_set_name(b, "p", pico8_value_ref(pv));
    pico8_ob_table_set_name(b, "ra", pico8_value_ref(rv));
    made = pico8_jo_new(b);
    pico8_jo_hu(made, to_pm);
    assert(me_of(made, "p_", &v) && v.kind == PICO8_VALUE_BOOL && v.as.boolean == 1);
    h = fold_jo_state(h, made);
    made = last_of_class(pico8_pm_struct_class());
    h = fold_same(h, made, "id", s_id);
    made = last_of_class(pico8_ga_struct_class());
    h = fnv(h, (uint32_t)(made != NULL));
    if (made != NULL) {
        h = fold_jo_state(h, made);
    }

    made = pico8_ga_new(NULL);
    pico8_ga_kj(&gfx, &mem, made, *pv);
    h = fold_jo_state(h, made);

    nr = pico8_nr_new(NULL);
    pico8_list_init(two);
    pico8_list_add(two, pico8_value_fixed(1));
    pico8_list_add(two, pico8_value_fixed(2));
    write_by_name(nr, "oe", pico8_value_ref(two));
    pico8_nr_hb(nr);
    assert(me_of(nr, "s", &v));
    h = fnv(fnv(h, (uint32_t)v.kind), (uint32_t)v.as.fixed);
    v = pico8_nr_gv(nr, NULL);
    h = fnv(h, (uint32_t)v.kind);
    h = fnv(h, v.kind == PICO8_VALUE_FIXED ? (uint32_t)v.as.fixed : 0u);

    made = pico8_qh_new(NULL);
    v = pico8_d_(made);
    h = fnv(h, (uint32_t)v.kind);
    h = fold_jo_state(h, made);

    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        v = pico8_value_fixed(0x1234);
        assert(pico8_jo_struct_class()->field(e[0], intruders[i], &v) == 0);
        assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x1234);
        h = fnv(h, 0u);
    }

    grow_world();
    h = fold_hv_state(h);

    check("jo_state", h, JO_STATE_HASH);
}

static const char *const g_state_written[7] = {"oe", "ep", "jy", "fx", "lq", "jh", "p_"};
static int g_state_new;

static void build_id_off_row(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();

    (void)e;
    pico8_ob_table_set_name(b, "id", g_off_row_value);
    (void)tree_new(g_state_new, b);
}

/* 1 when `v` is in the type of the member `name`. */
static int state_in_row(const char *name, pico8_value_t v) {
    if (strcmp(name, "fx") == 0 || strcmp(name, "p_") == 0) {
        return v.kind == PICO8_VALUE_BOOL;
    }
    if (strcmp(name, "jh") == 0) {
        return v.kind == PICO8_VALUE_FIXED;
    }
    if (strcmp(name, "ep") == 0) {
        return v.kind == PICO8_VALUE_STRING && v.as.string != NULL;
    }
    if (strcmp(name, "lq") == 0) {
        return 0;
    }
    return v.kind == PICO8_VALUE_REF && pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST;
}

static void test_jo_state_stops_off_row(void) {
    pico8_value_t off_row[11];
    void *e[2];
    size_t n_off;
    size_t i;
    size_t k;
    size_t m;

    (void)start_game();
    e[0] = pico8_jo_new(NULL);
    e[1] = pico8_pm_new(NULL);
    for (i = 0; i < 2; i++) {
        n_off = 0;
        off_row[n_off++] = pico8_value_fixed(pico8_fixed_from_int(1));
        off_row[n_off++] = pico8_value_bool(1);
        off_row[n_off++] = pico8_value_string("x");
        off_row[n_off++] = zero_union(PICO8_VALUE_FIXED);
        off_row[n_off++] = zero_union(PICO8_VALUE_BOOL);
        off_row[n_off++] = zero_union(PICO8_VALUE_STRING);
        off_row[n_off++] = zero_union(PICO8_VALUE_FUNCTION);
        off_row[n_off++] = pico8_value_ref(pico8_ob_table_new());
        off_row[n_off++] = pico8_value_ref((void *)pico8_hv());
        off_row[n_off++] = pico8_value_ref(e[i]);
        off_row[n_off++] = pico8_value_ref(gb_project(0));
        for (m = 0; m < 7; m++) {
            g_off_row_name = g_state_written[m];
            for (k = 0; k < n_off; k++) {
                if (state_in_row(g_off_row_name, off_row[k])) {
                    continue;
                }
                g_off_row_value = off_row[k];
                must_abort(write_off_row, e[i]);
            }
        }
    }
    for (i = 0; i < 10; i++) {
        g_state_new = (int)i;
        n_off = 0;
        off_row[n_off++] = pico8_value_fixed(pico8_fixed_from_int(1));
        off_row[n_off++] = zero_union(PICO8_VALUE_STRING);
        off_row[n_off++] = zero_union(PICO8_VALUE_BOOL);
        off_row[n_off++] = pico8_value_ref(pico8_ob_table_new());
        for (k = 0; k < n_off; k++) {
            g_off_row_value = off_row[k];
            must_abort(build_id_off_row, NULL);
        }
    }
}

static const char *const gen_names[7] = {"ma", "fo", "kr", "cc", "by", "mi", "m_"};

/* The 11 classes that may hold a planet's `fo`: the 10 whose `hq` runs `bv`,
 * and `jc`. Index in this list, or an assert. */
static uint32_t fo_holder_index(const void *holder) {
    const pico8_struct_class_t *const holders[11] = {
        pico8_g__struct_class(), pico8_fv_struct_class(), pico8_qh_struct_class(), pico8_nl_struct_class(),
        pico8_os_struct_class(), pico8_hj_struct_class(), pico8_pm_struct_class(), pico8_nr_struct_class(),
        pico8_fr_struct_class(), pico8_mx_struct_class(), pico8_jc_struct_class()};
    uint32_t i;

    assert(pico8_type_tag_of(holder) == PICO8_TYPE_TAG_STRUCT);
    for (i = 0; i < 11; i++) {
        if (((const pico8_struct_header_t *)holder)->cls == holders[i]) {
            return i;
        }
    }
    assert(0 && "jo.fo holds one of the 11 admitted classes");
    return 99;
}

static void *fo_holder_new(int k) {
    switch (k) {
    case 0:
        return pico8_g__new(NULL);
    case 1:
        return pico8_fv_new(NULL);
    case 10:
        return pico8_jc_new(NULL);
    default:
        return tree_new(k == 6 ? 5 : k == 7 ? 7 : k == 8 ? 8 : k == 9 ? 9 : k - 1, NULL);
    }
}

/* Presence, then the content of each of the 7 members written by the
 * generic writers, each asserted in the member's type. */
static uint32_t fold_jo_gen(uint32_t h, const void *e) {
    size_t m;

    for (m = 0; m < 7; m++) {
        const char *n = gen_names[m];
        pico8_value_t v;

        if (!me_of(e, n, &v)) {
            h = fnv(h, 0u);
            continue;
        }
        h = fnv(fnv(h, 1u), (uint32_t)v.kind);
        if (strcmp(n, "ma") == 0 || strcmp(n, "kr") == 0 || strcmp(n, "mi") == 0) {
            assert(v.kind == PICO8_VALUE_FIXED);
            h = fnv(h, (uint32_t)v.as.fixed);
        } else if (strcmp(n, "m_") == 0) {
            assert(v.kind == PICO8_VALUE_BOOL);
            h = fnv(h, (uint32_t)v.as.boolean);
        } else if (strcmp(n, "fo") == 0) {
            assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
            h = fnv(h, fo_holder_index(v.as.ref));
        } else {
            assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
            assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST);
            h = fnv(h, (uint32_t)((const pico8_list_t *)v.as.ref)->count);
        }
    }
    return h;
}

static uint32_t fold_hv_gen(uint32_t h) {
    const pico8_list_t *hv = pico8_hv();
    int i;

    h = fnv(h, (uint32_t)hv->count);
    for (i = 0; i < hv->count; i++) {
        assert(hv->items[i].kind == PICO8_VALUE_REF);
        h = jo_tree(hv->items[i].as.ref) ? fold_jo_gen(h, hv->items[i].as.ref) : fnv(h, 7u);
    }
    return h;
}

static void *ref_of(const void *e, const char *name) {
    pico8_value_t v;

    return me_of(e, name, &v) && v.kind == PICO8_VALUE_REF ? v.as.ref : NULL;
}

static void test_jo_writers_by_name(void) {
    static const int writer_of[6] = {0, 1, 5, 6, 7, 8};
    static const char *const intruders[] = {"", "m", "ma ", "M_", "fo_", "has_kr", "has_mi", "has_m_", "kr_", "cc "};
    pico8_list_t *l = malloc(sizeof *l);
    const pico8_list_t *jos;
    pico8_value_t v;
    pico8_value_t d;
    void *e[10];
    void *made;
    void *planet;
    void *holder;
    pico8_vec_t *at;
    uint32_t h = 2166136261u;
    size_t i;
    size_t j;
    int k;
    int x;
    int y;
    int before;

    (void)start_game();
    h = fold_hv_gen(h);

    for (i = 0; i < 10; i++) {
        e[i] = tree_new((int)i, NULL);
        h = fold_jo_gen(h, e[i]);
        assert(me_of(e[i], "ma", &v) && v.kind == PICO8_VALUE_FIXED);
        assert(!me_of(e[i], "kr", &v) && !me_of(e[i], "cc", &v) && !me_of(e[i], "fo", &v));
        assert(!me_of(e[i], "m_", &v));
        h = fnv(h, (uint32_t)me_of(e[i], "mi", &v));
        h = fnv(h, (uint32_t)me_of(e[i], "by", &v));
    }
    assert(pico8_ga_default("by", &d) && d.kind == PICO8_VALUE_REF);
    assert(ref_of(e[6], "by") == d.as.ref);
    made = pico8_ga_new(NULL);
    assert(ref_of(made, "by") == d.as.ref);

    pico8_list_init(l);
    for (i = 0; i < 6; i++) {
        void *w = e[writer_of[i]];
        const pico8_struct_header_t *header = w;
        void *own_by = ref_of(w, "by");

        write_by_name(w, "ma", pico8_value_fixed(pico8_fixed_from_int(2)));
        h = fold_flag(h, w, "ma", PICO8_VALUE_FIXED);
        for (j = 0; j < 2; j++) {
            const char *n = j == 0 ? "kr" : "mi";

            write_by_name(w, n, pico8_value_fixed(pico8_fixed_from_int(3)));
            h = fold_flag(h, w, n, PICO8_VALUE_FIXED);
            write_by_name(w, n, zero_union(PICO8_VALUE_FIXED));
            h = fold_flag(h, w, n, PICO8_VALUE_FIXED);
            assert(me_of(w, n, &v) && v.as.fixed == 0);
            write_by_name(w, n, pico8_value_ref(NULL));
            h = fold_flag(h, w, n, PICO8_VALUE_FIXED);
            assert(!me_of(w, n, &v));
        }
        write_by_name(w, "m_", pico8_value_bool(1));
        h = fold_flag(h, w, "m_", PICO8_VALUE_BOOL);
        write_by_name(w, "m_", pico8_value_bool(0));
        h = fold_flag(h, w, "m_", PICO8_VALUE_BOOL);
        assert(me_of(w, "m_", &v) && v.as.boolean == 0);
        write_by_name(w, "m_", zero_union(PICO8_VALUE_BOOL));
        h = fold_flag(h, w, "m_", PICO8_VALUE_BOOL);
        assert(me_of(w, "m_", &v) && v.as.boolean == 0);
        write_by_name(w, "m_", pico8_value_ref(NULL));
        h = fold_flag(h, w, "m_", PICO8_VALUE_BOOL);
        assert(!me_of(w, "m_", &v));
        for (j = 0; j < 2; j++) {
            const char *n = j == 0 ? "cc" : "by";

            write_by_name(w, n, pico8_value_ref(l));
            h = fold_same(h, w, n, l);
            write_by_name(w, n, pico8_value_ref(NULL));
            h = fold_same(h, w, n, NULL);
        }
        for (k = 0; k < 11; k++) {
            holder = fo_holder_new(k);
            h = fnv(h, fo_holder_index(holder));
            write_by_name(w, "fo", pico8_value_ref(holder));
            h = fold_same(h, w, "fo", holder);
        }
        write_by_name(w, "fo", pico8_value_ref(NULL));
        h = fold_same(h, w, "fo", NULL);
        assert(header->cls->set_field(w, "id", pico8_value_string("p42")) == 0);
        if (own_by != NULL) {
            write_by_name(w, "by", pico8_value_ref(own_by));
        }
        h = fold_jo_gen(h, w);
    }

    pico8_hy_lj(pico8_fm_hy());
    h = fold_hv_gen(h);
    jos = pico8_cg("jo");
    assert(jos != NULL);
    pico8_bd();
    jos = pico8_cg("jo");
    for (k = 0; k < jos->count; k++) {
        assert(ref_of(jos->items[k].as.ref, "cc") != NULL);
        if (is_of(jos->items[k].as.ref, pico8_ga_struct_class())) {
            assert(ref_of(jos->items[k].as.ref, "by") == d.as.ref);
        }
    }
    h = fold_hv_gen(h);
    h = fnv(h, (uint32_t)pico8_bb());
    h = fold_hv_gen(h);

    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, pico8_ok());
    planet = planet_on_screen(&x, &y);
    mouse_at(x, y, 0);
    pico8_ui_lj(&mem, pico8_ok());
    h = fold_flag(h, planet, "m_", PICO8_VALUE_BOOL);
    assert(me_of(planet, "m_", &v) && v.as.boolean == 1);
    pico8_ui_lj(&mem, pico8_ok());
    mouse_at(x + 40 > 127 ? x - 40 : x + 40, y, 0);
    pico8_ui_lj(&mem, pico8_ok());
    h = fold_flag(h, planet, "m_", PICO8_VALUE_BOOL);
    assert(me_of(planet, "m_", &v) && v.as.boolean == 0);

    pico8_jo_kd(planet);
    h = fold_flag(h, planet, "ma", PICO8_VALUE_FIXED);
    for (k = 0; k < 15; k++) {
        (void)pico8_ma(vec_of(planet, "ra"), planet);
        h = fold_flag(h, planet, "ma", PICO8_VALUE_FIXED);
    }

    at = heap_vec(0, 0);
    *at = vec_of(planet, "ra");
    pico8_bv(e[1], pico8_value_ref(at), pico8_value_ref(at));
    holder = ref_of(planet, "fo");
    h = fnv(h, (uint32_t)(holder != NULL));
    assert(holder == NULL || holder == e[1]);
    h = fold_hv_gen(h);
    pico8_jo_kj(&gfx, &mem, planet, *at);
    assert(!me_of(planet, "fo", &v));

    for (k = 0; k < 2; k++) {
        const pico8_struct_header_t *p = k == 0 ? (void *)pico8_pm_new(NULL) : (void *)pico8_fv_new(NULL);

        (void)p->cls->methods[PICO8_METHOD_HQ]((void *)p, 2, pico8_value_ref(at), pico8_value_ref(at));
        holder = ref_of(planet, "fo");
        h = fnv(h, holder == NULL ? 99u : fo_holder_index(holder));
        write_by_name(planet, "fo", pico8_value_ref((void *)p));
        h = fold_same(h, planet, "fo", p);
        write_by_name(planet, "fo", pico8_value_ref(NULL));
    }

    for (k = 0; k < 3; k++) {
        write_by_name(planet, "kr", k == 0   ? pico8_value_fixed(pico8_fixed_from_int(5))
                                    : k == 1 ? pico8_value_ref(NULL)
                                             : pico8_value_fixed(0));
        before = pico8_hv()->count;
        pico8_jo_hk(planet);
        h = fnv(h, (uint32_t)(pico8_hv()->count - before));
        assert((pico8_hv()->count - before == 1) == (k == 0));
    }
    made = pico8_jo_new(NULL);
    h = fnv(h, (uint32_t)pico8_jo_ib(made)->count);
    assert(!me_of(made, "by", &v));
    made = pico8_ga_new(NULL);
    assert(pico8_jo_ib(made) == d.as.ref);

    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        v = pico8_value_fixed(0x1234);
        assert(pico8_jo_struct_class()->field(e[0], intruders[i], &v) == 0);
        assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x1234);
        h = fnv(h, 0u);
    }

    grow_world();
    h = fold_hv_gen(h);
    made = last_of_class(pico8_jc_struct_class());
    h = fnv(h, (uint32_t)(made != NULL));
    if (made != NULL) {
        jos = pico8_cg("jo");
        for (k = 0; k < jos->count; k++) {
            write_by_name(jos->items[k].as.ref, "fo", pico8_value_ref(NULL));
        }
        pico8_jc_hq(made, pico8_value_ref(at));
        for (k = 0; k < jos->count; k++) {
            holder = ref_of(jos->items[k].as.ref, "fo");
            h = fnv(h, (uint32_t)(holder != NULL));
            assert(holder == NULL || holder == made);
        }
    }

    check("jo_writers", h, JO_WRITERS_HASH);
}

static const char *const g_gen_off_names[7] = {"ma", "kr", "mi", "m_", "cc", "by", "fo"};

static void test_jo_writers_stop_off_row(void) {
    pico8_value_t off_row[20];
    pico8_vec_t *vec = heap_vec(1, 2);
    void *e[3];
    size_t n_off;
    size_t i;
    size_t k;
    size_t m;

    (void)start_game();
    e[0] = pico8_jo_new(NULL);
    e[1] = pico8_pm_new(NULL);
    e[2] = pico8_ga_new(NULL);
    for (i = 0; i < 3; i++) {
        for (m = 0; m < 7; m++) {
            const char *n = g_gen_off_names[m];

            n_off = 0;
            if (strcmp(n, "ma") == 0) {
                off_row[n_off++] = pico8_value_ref(NULL);
            }
            if (strcmp(n, "m_") != 0) {
                off_row[n_off++] = zero_union(PICO8_VALUE_BOOL);
            }
            if (strcmp(n, "ma") != 0 && strcmp(n, "kr") != 0 && strcmp(n, "mi") != 0) {
                off_row[n_off++] = pico8_value_fixed(pico8_fixed_from_int(1));
                off_row[n_off++] = zero_union(PICO8_VALUE_FIXED);
            }
            if (strcmp(n, "cc") != 0 && strcmp(n, "by") != 0 && strcmp(n, "m_") != 0) {
                off_row[n_off++] = pico8_value_bool(1);
            }
            if (strcmp(n, "cc") != 0 && strcmp(n, "by") != 0) {
                off_row[n_off++] = pico8_value_string("x");
            }
            off_row[n_off++] = zero_union(PICO8_VALUE_STRING);
            off_row[n_off++] = zero_union(PICO8_VALUE_FUNCTION);
            off_row[n_off++] = pico8_value_ref(pico8_ob_table_new());
            if (strcmp(n, "kr") == 0 || strcmp(n, "mi") == 0 || strcmp(n, "fo") == 0) {
                off_row[n_off++] = pico8_value_ref((void *)pico8_hv());
            }
            if (strcmp(n, "cc") == 0 || strcmp(n, "by") == 0 || strcmp(n, "fo") == 0) {
                off_row[n_off++] = pico8_value_ref(vec);
            }
            if (strcmp(n, "cc") == 0 || strcmp(n, "by") == 0) {
                off_row[n_off++] = pico8_value_ref(e[i]);
            }
            if (strcmp(n, "fo") == 0) {
                off_row[n_off++] = pico8_value_ref(pico8_mz_new(NULL));
                off_row[n_off++] = pico8_value_ref(pico8_jo_new(NULL));
                off_row[n_off++] = pico8_value_ref(pico8_ga_new(NULL));
                off_row[n_off++] = pico8_value_ref(gb_project(0));
                off_row[n_off++] = pico8_value_ref(the_bg());
                off_row[n_off++] = pico8_value_ref(pico8_ok());
            }
            g_off_row_name = n;
            for (k = 0; k < n_off; k++) {
                g_off_row_value = off_row[k];
                must_abort(write_off_row, e[i]);
            }
        }
    }
}

typedef int (*qh_spec_reader_t)(const char *, pico8_value_t *);

static const char *const qh_names[10] = {"hp", "px", "dq", "qh", "od", "fp", "dw", "s", "ba", "et"};

static void *qh_tree_new(int k) {
    switch (k) {
    case 0:
        return pico8_qh_new(NULL);
    case 1:
        return pico8_nl_new(NULL);
    case 2:
        return pico8_os_new(NULL);
    case 3:
        return pico8_hj_new(NULL);
    case 4:
        return pico8_pm_new(NULL);
    case 5:
        return pico8_nr_new(NULL);
    case 6:
        return pico8_fr_new(NULL);
    default:
        return pico8_mx_new(NULL);
    }
}

static qh_spec_reader_t qh_tree_reader(int k) {
    static const qh_spec_reader_t readers[8] = {pico8_qh_default, pico8_nl_default, pico8_os_default,
                                                pico8_hj_default, pico8_pm_default, pico8_nr_default,
                                                pico8_fr_default, pico8_mx_default};
    return readers[k];
}

/* The generated `px` or `ba` a member of qh's tree takes from `own`, one of
 * qh_tree_reader()'s readers. */
static const void *qh_tree_data(qh_spec_reader_t own, const char *name) {
    static const void *const px[8] = {&pico8_qh_px_row, &pico8_nl_px_row, &pico8_os_px_row, &pico8_hj_px_row,
                                      &pico8_pm_px_row, &pico8_qh_px_row, &pico8_qh_px_row, &pico8_mx_px_row};
    static const void *const ba[8] = {&pico8_qh_ba_seq, &pico8_nl_ba_seq, &pico8_qh_ba_seq, &pico8_qh_ba_seq,
                                      &pico8_qh_ba_seq, &pico8_qh_ba_seq, &pico8_qh_ba_seq, &pico8_qh_ba_seq};
    int k;

    for (k = 0; k < 8; k++) {
        if (qh_tree_reader(k) == own) {
            return strcmp(name, "px") == 0 ? px[k] : ba[k];
        }
    }
    assert(0 && "own is one of qh's tree");
    return NULL;
}

static int qh_tree_rank(const void *e) {
    const pico8_struct_class_t *cls[8];
    int k;

    cls[0] = pico8_qh_struct_class();
    cls[1] = pico8_nl_struct_class();
    cls[2] = pico8_os_struct_class();
    cls[3] = pico8_hj_struct_class();
    cls[4] = pico8_pm_struct_class();
    cls[5] = pico8_nr_struct_class();
    cls[6] = pico8_fr_struct_class();
    cls[7] = pico8_mx_struct_class();
    for (k = 0; k < 8; k++) {
        if (is_of(e, cls[k])) {
            return k;
        }
    }
    return -1;
}

/* The eleven classes `bv` aims at: jo, the eight of qh's tree, jc, the
 * throwaway target. */
static int et_target_rank(const void *t) {
    int k = qh_tree_rank(t);

    if (k >= 0) {
        return 1 + k;
    }
    if (is_of(t, pico8_jo_struct_class())) {
        return 0;
    }
    if (is_of(t, pico8_jc_struct_class())) {
        return 9;
    }
    assert(is_of(t, pico8_bv_et_struct_class()));
    return 10;
}

/* 0 absent, 1 false present, 2 + the target's rank. */
static uint32_t et_state(const void *e) {
    pico8_value_t v;

    if (!me_of(e, "et", &v)) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_BOOL) {
        assert(v.as.boolean == 0);
        return 1;
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return 2u + (uint32_t)et_target_rank(v.as.ref);
}

static uint32_t fold_qh_member(uint32_t h, const void *e, const char *name, qh_spec_reader_t own) {
    pico8_value_t v;
    pico8_value_t d;

    if (strcmp(name, "et") == 0) {
        return fnv(h, 100u + et_state(e));
    }
    if (!me_of(e, name, &v)) {
        return fnv(h, 0u);
    }
    h = fnv(h, 1u + (uint32_t)v.kind);
    switch (v.kind) {
    case PICO8_VALUE_FIXED:
        return fnv(h, (uint32_t)v.as.fixed);
    case PICO8_VALUE_BOOL:
        return fnv(h, (uint32_t)v.as.boolean);
    case PICO8_VALUE_STRING:
        assert(own != NULL && own(name, &d) && d.kind == PICO8_VALUE_STRING && d.as.string == v.as.string);
        return fold_string(h, v.as.string);
    case PICO8_VALUE_REF:
        assert(own != NULL && qh_tree_data(own, name) == v.as.ref);
        assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN);
        return fnv(h, data_entries(name, v.as.ref));
    default:
        assert(0 && "no member of qh holds a function");
        return h;
    }
}

static uint32_t fold_qh_all(uint32_t h, const void *e, qh_spec_reader_t own) {
    size_t i;

    for (i = 0; i < 10; i++) {
        h = fold_qh_member(h, e, qh_names[i], own);
    }
    return h;
}

static uint32_t fold_hv_qh(uint32_t h) {
    const pico8_list_t *hv = pico8_hv();
    int i;

    for (i = 0; i < hv->count; i++) {
        int k = qh_tree_rank(hv->items[i].as.ref);
        if (k >= 0) {
            h = fnv(h, (uint32_t)k);
            h = fold_qh_all(h, hv->items[i].as.ref, qh_tree_reader(k));
        }
    }
    return h;
}

static pico8_value_t qh_vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);

    *v = at;
    return pico8_value_ref(v);
}

static pico8_vec_t qh_at(int x, int y) {
    return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
}

/* `hq` of `lab` with the mouse one pixel right of `at` (a position in `p`
 * units), the raw mouse at `rp`. */
static void qh_hq(void *lab, pico8_vec_t at, int rp) {
    pico8_value_t out = pico8_value_ref(NULL);
    pico8_vec_t mp = pico8_vec_add(pico8_vec_mul(at, pico8_fixed_from_int(128)), qh_at(1, 0));

    assert(pico8_mv(lab, "hq", 2, qh_vec_ref(mp), qh_vec_ref(qh_at(rp, rp)), &out));
}

/* A fresh planet of the k-th class of `cg.np` (jo, then qh's tree) at `at`. */
static void *np_target_at(int k, pico8_vec_t at) {
    pico8_ob_table_t *b;
    void *t;

    if (k > 0) {
        t = qh_tree_new(k - 1);
        pico8_entity_set_field(t, "p", qh_vec_ref(at));
        return t;
    }
    b = pico8_ob_table_new();
    pico8_ob_table_set_name(b, "p", qh_vec_ref(at));
    t = pico8_jo_new(b);
    pico8_ob_table_free(b);
    return t;
}

static void test_qh_members_by_name(void) {
    static const char *const strangers[] = {"", "h", "hp ", "HP", "et_", "has_et", "qh_is_bool", "e", "ba_"};
    static const char *const raw_refused[] = {"hp", "s", "od", "id", "qs"};
    void *targets[11];
    void *fresh[8];
    void *lab;
    void *throwaway;
    pico8_value_t v;
    uint32_t h = 2166136261u;
    int k;
    size_t n;

    for (k = 0; k < 8; k++) {
        (void)start_game();
        fresh[k] = qh_tree_new(k);
        h = fnv(h, (uint32_t)k);
        h = fold_qh_all(h, fresh[k], qh_tree_reader(k));
        assert(et_state(fresh[k]) == 0);
    }

    (void)start_game();
    h = fold_hv_qh(h);
    /* `ij` skips an entity on the very point: the gate aims beside it. */
    {
        void *gate = qh_tree_new(5);
        pico8_vec_t *mp = malloc(sizeof *mp);

        *mp = pico8_vec_add(pico8_vec_mul(pos_of(pico8_cg("jo")->items[0].as.ref), pico8_fixed_from_int(128)),
                            qh_at(1, 1));
        assert(pico8_mv(gate, "hq", 2, pico8_value_ref(mp), qh_vec_ref(qh_at(64, 64)), &v));
        assert(et_state(gate) == 2u);
    }
    h = fold_hv_qh(h);

    (void)start_game();
    lab = pico8_qh_new(NULL);
    for (k = 0; k < 9; k++) {
        pico8_vec_t at = qh_at(100 + 2 * k, 100);
        targets[k] = np_target_at(k, at);
        qh_hq(lab, at, 0);
        assert(me_of(lab, "et", &v) && v.kind == PICO8_VALUE_REF && v.as.ref == targets[k]);
        assert(get_ref(targets[k], "fo") == lab);
        h = fnv(h, et_state(lab));
    }
    qh_hq(lab, qh_at(200, 200), 0);
    h = fnv(h, et_state(lab));
    assert(et_state(lab) == 12u);
    throwaway = get_ref(lab, "et");
    qh_hq(lab, qh_at(200, 200), 64);
    h = fnv(h, et_state(lab));
    assert(et_state(lab) == 1u);
    for (k = 0; k < 8; k++) {
        void *holder = qh_tree_new(k);
        pico8_vec_t at = qh_at(100 + 2 * k, 110);
        void *t = np_target_at(0, at);

        qh_hq(holder, at, 0);
        assert(get_ref(holder, "et") == t && get_ref(t, "fo") == holder);
        h = fnv(h, et_state(holder));
    }

    {
        pico8_ob_table_t *b = pico8_ob_table_new();
        pico8_ob_table_set_name(b, "qu", pico8_value_ref(targets[0]));
        targets[9] = pico8_jc_new(b);
        pico8_ob_table_free(b);
    }
    targets[10] = throwaway;
    for (k = 0; k < 4; k++) {
        static const int writers[4] = {0, 1, 4, 6};
        void *e = qh_tree_new(writers[k]);
        const pico8_struct_header_t *header = e;

        for (n = 0; n < 11; n++) {
            write_by_name(e, "et", pico8_value_ref(targets[n]));
            assert(me_of(e, "et", &v) && v.kind == PICO8_VALUE_REF && v.as.ref == targets[n]);
            h = fnv(h, et_state(e));
        }
        write_by_name(e, "et", pico8_value_bool(0));
        h = fnv(h, et_state(e));
        write_by_name(e, "et", pico8_value_ref(NULL));
        h = fnv(h, et_state(e));
        write_by_name(e, "et", zero_union(PICO8_VALUE_BOOL));
        h = fnv(h, et_state(e));
        write_by_name(e, "et", zero_union(PICO8_VALUE_REF));
        h = fnv(h, et_state(e));
        assert(et_state(e) == 0u);
        if (k == 0) {
            for (n = 0; n < sizeof raw_refused / sizeof raw_refused[0]; n++) {
                h = fnv(h, (uint32_t)header->cls->set_field(e, raw_refused[n], pico8_value_fixed(7)));
            }
            h = fold_qh_all(h, e, qh_tree_reader(0));
        }
    }

    for (k = 5; k < 7; k++) {
        void *e = qh_tree_new(k);
        const pico8_struct_header_t *header = e;

        write_by_name(e, "s", pico8_value_fixed(pico8_fixed_from_int(3)));
        h = fold_qh_member(h, e, "s", NULL);
        write_by_name(e, "s", zero_union(PICO8_VALUE_FIXED));
        assert(me_of(e, "s", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0);
        h = fold_qh_member(h, e, "s", NULL);
        h = fnv(h, (uint32_t)header->cls->set_field(e, "od", pico8_value_fixed(7)));
        assert(pico8_mv(e, "hb", 0, nil(), nil(), &v));
        h = fold_qh_member(h, e, "s", NULL);
    }

    for (n = 0; n < sizeof strangers / sizeof strangers[0]; n++) {
        h = fnv(h, (uint32_t)me_of(fresh[0], strangers[n], &v));
    }

    check("qh_members", h, QH_MEMBERS_HASH);
}

static void test_qh_writer_stops_off_row(void) {
    pico8_value_t off_row[20];
    void *e[3];
    void *s_of[2];
    size_t n_off;
    size_t i;
    size_t k;
    pico8_vec_t *vec = malloc(sizeof *vec);

    (void)start_game();
    *vec = qh_at(1, 1);
    e[0] = pico8_qh_new(NULL);
    e[1] = pico8_pm_new(NULL);
    e[2] = pico8_nr_new(NULL);
    n_off = 0;
    off_row[n_off++] = pico8_value_fixed(pico8_fixed_from_int(1));
    off_row[n_off++] = zero_union(PICO8_VALUE_FIXED);
    off_row[n_off++] = pico8_value_bool(1);
    off_row[n_off++] = pico8_value_string("x");
    off_row[n_off++] = zero_union(PICO8_VALUE_STRING);
    off_row[n_off++] = zero_union(PICO8_VALUE_FUNCTION);
    off_row[n_off++] = pico8_value_ref(pico8_ob_table_new());
    off_row[n_off++] = pico8_value_ref((void *)pico8_hv());
    off_row[n_off++] = pico8_value_ref(vec);
    off_row[n_off++] = pico8_value_ref(pico8_ga_new(NULL));
    off_row[n_off++] = pico8_value_ref(pico8_g__new(NULL));
    off_row[n_off++] = pico8_value_ref(pico8_fv_new(NULL));
    off_row[n_off++] = pico8_value_ref(pico8_mz_new(NULL));
    off_row[n_off++] = pico8_value_ref(the_bg());
    off_row[n_off++] = pico8_value_ref(pico8_ok());
    off_row[n_off++] = pico8_value_ref(gb_project(0));
    off_row[n_off++] = pico8_value_ref(pico8_mz_jj_wrapper());
    g_off_row_name = "et";
    for (i = 0; i < 3; i++) {
        for (k = 0; k < n_off; k++) {
            g_off_row_value = off_row[k];
            must_abort(write_off_row, e[i]);
        }
    }

    s_of[0] = e[2];
    s_of[1] = pico8_fr_new(NULL);
    n_off = 0;
    off_row[n_off++] = pico8_value_ref(NULL);
    off_row[n_off++] = pico8_value_bool(1);
    off_row[n_off++] = zero_union(PICO8_VALUE_BOOL);
    off_row[n_off++] = pico8_value_string("x");
    off_row[n_off++] = zero_union(PICO8_VALUE_STRING);
    off_row[n_off++] = zero_union(PICO8_VALUE_FUNCTION);
    off_row[n_off++] = pico8_value_ref(pico8_ob_table_new());
    g_off_row_name = "s";
    for (i = 0; i < 2; i++) {
        for (k = 0; k < n_off; k++) {
            g_off_row_value = off_row[k];
            must_abort(write_off_row, s_of[i]);
        }
    }
}

static const char *const hj_names[13] = {"rr", "py", "ki", "pn", "pk", "fc", "k_",
                                         "du", "qo", "ro", "qg", "q_", "qy"};

/* 0: hj, 1: pm, 2: mx. */
static void *hj_tree_new(int k) {
    return k == 0 ? (void *)pico8_hj_new(NULL) : k == 1 ? (void *)pico8_pm_new(NULL) : (void *)pico8_mx_new(NULL);
}

static qh_spec_reader_t hj_tree_reader(int k) {
    static const qh_spec_reader_t readers[3] = {pico8_hj_default, pico8_pm_default, pico8_mx_default};
    return readers[k];
}

/* The generated `k_` of hj, pm, mx, by hj_tree_reader() rank. */
static const pico8_int_seq_t *hj_tree_k_(int k) {
    static const pico8_int_seq_t *const k_[3] = {&pico8_hj_k__seq, &pico8_pm_k__seq, &pico8_mx_k__seq};
    return k_[k];
}

static int hj_tree_rank_of(qh_spec_reader_t own) {
    int k;

    for (k = 0; k < 3 && hj_tree_reader(k) != own; k++) {
    }
    assert(k < 3 && "own is one of hj's tree");
    return k;
}

/* -1 when `qy` is absent. */
static int hj_qy_size(const void *e) {
    pico8_value_t v;

    if (!me_of(e, "qy", &v)) {
        return -1;
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL && pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST);
    return ((const pico8_list_t *)v.as.ref)->count;
}

static uint32_t fold_hj_member(uint32_t h, const void *e, const char *name, qh_spec_reader_t own) {
    pico8_value_t v;
    pico8_value_t d;

    if (!me_of(e, name, &v)) {
        return fnv(h, 0u);
    }
    h = fnv(h, 1u + (uint32_t)v.kind);
    switch (v.kind) {
    case PICO8_VALUE_FIXED:
        return fnv(h, (uint32_t)v.as.fixed);
    case PICO8_VALUE_BOOL:
        return fnv(h, (uint32_t)v.as.boolean);
    case PICO8_VALUE_STRING:
        assert(own(name, &d) && d.kind == PICO8_VALUE_STRING && d.as.string == v.as.string);
        return fold_string(h, v.as.string);
    case PICO8_VALUE_REF:
        if (strcmp(name, "qy") == 0) {
            return fnv(h, (uint32_t)hj_qy_size(e));
        }
        assert(strcmp(name, "k_") == 0 && hj_tree_k_(hj_tree_rank_of(own)) == v.as.ref);
        assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN);
        return fnv(h, data_entries(name, v.as.ref));
    default:
        assert(0 && "no member of hj holds a function");
        return h;
    }
}

static uint32_t fold_hj_all(uint32_t h, const void *e, qh_spec_reader_t own) {
    size_t i;

    for (i = 0; i < 13; i++) {
        h = fold_hj_member(h, e, hj_names[i], own);
    }
    return h;
}

static uint32_t fold_hj_particle(uint32_t h, pico8_value_t pt) {
    static const char *const vecs[] = {"p", "v"};
    pico8_value_t v;
    size_t i;

    assert(pt.kind == PICO8_VALUE_REF && pt.as.ref != NULL && pico8_type_tag_of(pt.as.ref) == PICO8_TYPE_TAG_OB_TABLE);
    for (i = 0; i < 2; i++) {
        assert(pico8_ob_table_get_name(pt.as.ref, vecs[i], &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
        h = fold_vec(h, *(const pico8_vec_t *)v.as.ref);
    }
    assert(pico8_ob_table_get_name(pt.as.ref, "l", &v) && v.kind == PICO8_VALUE_FIXED);
    return fnv(h, (uint32_t)v.as.fixed);
}

static uint32_t fold_hj_qy(uint32_t h, const void *e) {
    const pico8_list_t *qy = get_ref(e, "qy");
    int i;

    h = fnv(h, (uint32_t)qy->count);
    for (i = 0; i < qy->count; i++) {
        h = fold_hj_particle(h, qy->items[i]);
    }
    return h;
}

static pico8_value_t call_mm(void *e) {
    pico8_value_t out = nil();

    assert(pico8_mv(e, "mm", 0, nil(), nil(), &out));
    return out;
}

static void test_hj_members_by_name(void) {
    static const char *const strangers[] = {"", "q", "qy ", "QY", "pk_is_bool", "k", "k__", "rr_"};
    void *fresh[4];
    void *qy_of[4];
    pico8_value_t v;
    pico8_value_t d;
    uint32_t h = 2166136261u;
    int k;
    int j;
    size_t n;

    (void)start_game();
    for (k = 0; k < 3; k++) {
        fresh[k] = hj_tree_new(k);
        h = fnv(h, (uint32_t)k);
        h = fold_hj_all(h, fresh[k], hj_tree_reader(k));
    }
    {
        pico8_ob_table_t *b = pico8_ob_table_new();
        pico8_ob_table_set_name(b, "p", qh_vec_ref(qh_at(40, 50)));
        pico8_ob_table_set_name(b, "ra", qh_vec_ref(qh_at(7700, 7600)));
        pico8_ob_table_set_name(b, "id", pico8_value_string("s77"));
        fresh[3] = pico8_pm_new(b);
        pico8_ob_table_free(b);
        h = fold_hj_all(h, fresh[3], pico8_pm_default);
    }
    for (k = 0; k < 4; k++) {
        int g = k == 3 ? 1 : k;

        assert(me_of(fresh[k], "rr", &v) && v.kind == PICO8_VALUE_STRING);
        assert(pico8_hj_default("rr", &d) && (g == 2 || v.as.string == d.as.string));
        assert(me_of(fresh[k], "k_", &v) && v.kind == PICO8_VALUE_REF);
        assert(hj_tree_k_(g) == v.as.ref);
        assert((g == 0) == (&pico8_hj_k__seq == v.as.ref));
        assert(me_of(fresh[k], "pk", &v));
        assert(g == 1 ? v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0
                      : v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(1));
        assert(hj_qy_size(fresh[k]) == 0);
        qy_of[k] = get_ref(fresh[k], "qy");
        for (j = 0; j < k; j++) {
            assert(qy_of[j] != qy_of[k]);
        }
    }

    for (k = 0; k < 3; k++) {
        void *e;
        pico8_vec_t at;

        (void)start_game();
        e = hj_tree_new(k);
        h = fold_hj_particle(fnv(h, (uint32_t)k), call_mm(e));
        pico8_ez(e, PICO8_FIXED_LIT(0x00028000, 2.5));
        h = fnv(h, (uint32_t)hj_qy_size(e));
        at = pico8_vec_add(vec_of(pico8_mo(), "ra"), qh_at(64, 64));
        write_by_name(e, "ra", qh_vec_ref(at));
        pico8_entity_set_field(e, "mi", pico8_value_fixed(pico8_fixed_from_int(2)));
        qy_of[0] = get_ref(e, "qy");
        for (j = 0; j < 4; j++) {
            assert(pico8_mv(e, "kj", 1, qh_vec_ref(at), nil(), &v));
            h = fold_hj_qy(h, e);
            assert(get_ref(e, "qy") == qy_of[0]);
        }
        h = fold_hj_all(h, e, hj_tree_reader(k));
    }

    (void)start_game();
    for (k = 0; k < 3; k++) {
        void *e = hj_tree_new(k);
        const pico8_struct_header_t *header = e;
        void *born = get_ref(e, "qy");
        pico8_list_t *mine = malloc(sizeof *mine);
        const pico8_list_t *hv = pico8_hv();

        pico8_list_init(mine);
        write_by_name(e, "qy", pico8_value_ref(mine));
        assert(get_ref(e, "qy") == mine);
        h = fold_hj_member(h, e, "qy", NULL);
        write_by_name(e, "qy", pico8_value_ref((void *)hv));
        assert(get_ref(e, "qy") == hv);
        write_by_name(e, "qy", pico8_value_ref(NULL));
        assert(!me_of(e, "qy", &v) && hj_qy_size(e) == -1);
        h = fold_hj_member(h, e, "qy", NULL);
        write_by_name(e, "qy", pico8_value_ref(born));
        assert(get_ref(e, "qy") == born);
        for (n = 0; n < 12; n++) {
            h = fnv(h, (uint32_t)header->cls->set_field(e, hj_names[n], pico8_value_fixed(7)));
        }
        h = fold_hj_all(h, e, hj_tree_reader(k));
    }

    for (n = 0; n < sizeof strangers / sizeof strangers[0]; n++) {
        h = fnv(h, (uint32_t)me_of(fresh[0], strangers[n], &v));
    }

    check("hj_members", h, HJ_MEMBERS_HASH);
}

static uint32_t fold_pm_own(uint32_t h, const void *e) {
    pico8_value_t v;
    pico8_value_t d;
    const pico8_vec_t *ng;

    assert(me_of(e, "ng", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    ng = v.as.ref;
    assert(pico8_pm_default("ng", &d) && d.kind == PICO8_VALUE_REF && d.as.ref == ng);
    assert(ng->x == pico8_fixed_from_int(1) && ng->y == 0);
    h = fold_vec(h, *ng);
    assert(me_of(e, "fz", &v) && v.kind == PICO8_VALUE_STRING && v.as.string != NULL);
    assert(pico8_pm_default("fz", &d) && d.kind == PICO8_VALUE_STRING && d.as.string == v.as.string);
    assert(strcmp(v.as.string, "pm") == 0);
    for (d.as.string = v.as.string; *d.as.string != '\0'; d.as.string++) {
        h = fnv(h, (uint32_t)(unsigned char)*d.as.string);
    }
    return h;
}

static void test_pm_members_by_name(void) {
    static const char *const strangers[] = {"", "n", "ng ", "NG", "fz_", "f"};
    static const char *const own[] = {"ng", "fz"};
    void *fresh[2];
    void *others[2];
    pico8_value_t v;
    uint32_t h = 2166136261u;
    size_t n;
    int k;

    (void)start_game();
    fresh[0] = pico8_pm_new(NULL);
    {
        pico8_ob_table_t *b = pico8_ob_table_new();
        pico8_ob_table_set_name(b, "p", qh_vec_ref(qh_at(40, 50)));
        pico8_ob_table_set_name(b, "ra", qh_vec_ref(qh_at(7700, 7600)));
        pico8_ob_table_set_name(b, "id", pico8_value_string("s77"));
        fresh[1] = pico8_pm_new(b);
        pico8_ob_table_free(b);
    }
    for (k = 0; k < 2; k++) {
        h = fold_pm_own(fnv(h, (uint32_t)k), fresh[k]);
    }
    assert(get_ref(fresh[0], "ng") == get_ref(fresh[1], "ng"));

    (void)start_game();
    {
        pico8_ob_table_t *b = pico8_ob_table_new();
        const pico8_list_t *hv;
        const pico8_struct_header_t *g;
        const pico8_vec_t *gra;
        const pico8_vec_t *ra;
        const pico8_vec_t *ng;
        void *e;
        int at;

        pico8_ob_table_set_name(b, "ra", qh_vec_ref(qh_at(7650, 7720)));
        e = pico8_pm_new(b);
        pico8_ob_table_free(b);
        hv = pico8_hv();
        at = rank_in(hv, e);
        assert(at >= 1);
        g = hv->items[at - 1].as.ref;
        assert(g->cls == pico8_ga_struct_class());
        gra = get_ref(g, "ra");
        ra = get_ref(e, "ra");
        ng = get_ref(e, "ng");
        assert(gra != NULL && ra != NULL && ng != NULL && gra != ra && gra != ng);
        assert(gra->x == pico8_fixed_add(ra->x, ng->x) && gra->y == pico8_fixed_add(ra->y, ng->y));
        h = fold_vec(h, *gra);
        h = fold_pm_own(h, e);
    }

    for (k = 0; k < 2; k++) {
        const pico8_struct_header_t *header = fresh[k];

        for (n = 0; n < 2; n++) {
            h = fnv(h, (uint32_t)header->cls->set_field(fresh[k], own[n], pico8_value_fixed(9)));
        }
        h = fold_pm_own(h, fresh[k]);
    }

    for (n = 0; n < sizeof strangers / sizeof strangers[0]; n++) {
        h = fnv(h, (uint32_t)me_of(fresh[0], strangers[n], &v));
    }
    others[0] = pico8_hj_new(NULL);
    others[1] = pico8_mx_new(NULL);
    for (k = 0; k < 2; k++) {
        for (n = 0; n < 2; n++) {
            h = fnv(h, (uint32_t)me_of(others[k], own[n], &v));
        }
    }

    check("pm_members", h, PM_MEMBERS_HASH);
}

static void test_hj_writer_stops_off_row(void) {
    pico8_value_t off_row[12];
    void *e[3];
    size_t n_off;
    size_t i;
    size_t k;
    pico8_vec_t *vec = malloc(sizeof *vec);

    (void)start_game();
    *vec = qh_at(1, 1);
    for (i = 0; i < 3; i++) {
        e[i] = hj_tree_new((int)i);
    }
    n_off = 0;
    off_row[n_off++] = pico8_value_fixed(pico8_fixed_from_int(1));
    off_row[n_off++] = zero_union(PICO8_VALUE_FIXED);
    off_row[n_off++] = pico8_value_bool(1);
    off_row[n_off++] = pico8_value_bool(0);
    off_row[n_off++] = zero_union(PICO8_VALUE_BOOL);
    off_row[n_off++] = pico8_value_string("x");
    off_row[n_off++] = zero_union(PICO8_VALUE_STRING);
    off_row[n_off++] = zero_union(PICO8_VALUE_FUNCTION);
    off_row[n_off++] = pico8_value_ref(pico8_ob_table_new());
    off_row[n_off++] = pico8_value_ref(pico8_jo_new(NULL));
    off_row[n_off++] = pico8_value_ref(vec);
    g_off_row_name = "qy";
    for (i = 0; i < 3; i++) {
        for (k = 0; k < n_off; k++) {
            g_off_row_value = off_row[k];
            must_abort(write_off_row, e[i]);
        }
    }
}

static uint32_t fold_mx_ht(uint32_t h, const void *e) {
    pico8_value_t v;
    const pico8_int_seq_t *ht;
    int j;

    if (!me_of(e, "ht", &v)) {
        return fnv(h, 0xA0u);
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    ht = v.as.ref;
    h = fnv(h, (uint32_t)(&pico8_mx_ht_seq == ht));
    h = fnv(h, (uint32_t)ht->count);
    for (j = 0; j < ht->count; j++) {
        h = fnv(h, (uint32_t)pico8_fixed_from_int(ht->items[j]));
    }
    return h;
}

static uint32_t fold_mx_kw_qe(uint32_t h, const void *e) {
    const pico8_kw_t *kw = get_ref(e, "kw");

    assert(kw != NULL);
    if (!kw->has_qe) {
        return fnv(h, 0xA2u);
    }
    return fnv(h, (uint32_t)kw->qe);
}

/* `jc:ey` from `qu` to `mx` runs `mx:kf`, which reads the one row of
 * `qu`'s `jy`. */
static void mx_join_row(void *qu, void *mx, int qe) {
    pico8_ob_table_t *row = pico8_ob_table_new();
    pico8_list_t *jy = malloc(sizeof *jy);
    pico8_ob_table_t *b = pico8_ob_table_new();
    pico8_value_t out;
    void *jc;

    pico8_ob_table_set_name(row, "gc", pico8_value_fixed(0));
    pico8_ob_table_set_name(row, "qe", pico8_value_fixed(pico8_fixed_from_int(qe)));
    pico8_list_init(jy);
    pico8_list_add(jy, pico8_value_ref(row));
    pico8_entity_set_field(qu, "jy", pico8_value_ref(jy));
    pico8_ob_table_set_name(b, "qu", pico8_value_ref(qu));
    jc = pico8_jc_new(b);
    pico8_ob_table_free(b);
    pico8_fm_hy()->nz = pico8_fixed_from_int(999);
    assert(pico8_mv(jc, "ey", 1, pico8_value_ref(mx), pico8_value_ref(NULL), &out));
}

static void test_mx_members_by_name(void) {
    static const char *const strangers[] = {"", "h", "ht ", "HT", "ht_", "hz"};
    void *fresh[2];
    void *others[4];
    void *home;
    const void *spec;
    const void *swapped;
    pico8_value_t v;
    uint32_t h = 2166136261u;
    size_t n;
    int k;

    (void)start_game();
    fresh[0] = pico8_mx_new(NULL);
    {
        pico8_ob_table_t *b = pico8_ob_table_new();
        pico8_ob_table_set_name(b, "id", pico8_value_string("s88"));
        fresh[1] = pico8_mx_new(b);
        pico8_ob_table_free(b);
    }
    spec = &pico8_mx_ht_seq;
    for (k = 0; k < 2; k++) {
        assert(get_ref(fresh[k], "ht") == spec);
        h = fold_mx_ht(fnv(h, (uint32_t)k), fresh[k]);
    }

    home = pico8_hj_new(NULL);
    mx_join_row(home, fresh[0], 7);
    assert(get_ref(fresh[0], "ht") == spec);
    h = fold_mx_kw_qe(fold_mx_ht(h, fresh[0]), fresh[0]);
    mx_join_row(home, fresh[0], 20);
    swapped = get_ref(fresh[0], "ht");
    assert(swapped != NULL && swapped != spec && ((const pico8_int_seq_t *)spec)->count == 4);
    h = fold_mx_kw_qe(fold_mx_ht(h, fresh[0]), fresh[0]);
    mx_join_row(home, fresh[0], 21);
    assert(get_ref(fresh[0], "ht") == swapped);
    h = fold_mx_kw_qe(fold_mx_ht(h, fresh[0]), fresh[0]);
    assert(get_ref(fresh[1], "ht") == spec);
    h = fold_mx_ht(h, fresh[1]);

    {
        const pico8_struct_header_t *header = fresh[1];
        static const pico8_int_seq_t t = {0, NULL};

        h = fnv(h, (uint32_t)header->cls->set_field(fresh[1], "ht", pico8_value_ref(NULL)));
        h = fold_mx_ht(h, fresh[1]);
        h = fnv(h, (uint32_t)header->cls->set_field(fresh[1], "ht", pico8_value_ref((void *)&t)));
        assert(get_ref(fresh[1], "ht") == &t);
        h = fold_mx_ht(h, fresh[1]);
        pico8_entity_set_field(fresh[1], "ht", pico8_value_ref(NULL));
        mx_join_row(home, fresh[1], 20);
        h = fold_mx_kw_qe(fold_mx_ht(h, fresh[1]), fresh[1]);
    }

    for (n = 0; n < sizeof strangers / sizeof strangers[0]; n++) {
        h = fnv(h, (uint32_t)me_of(fresh[0], strangers[n], &v));
    }
    others[0] = pico8_hj_new(NULL);
    others[1] = pico8_pm_new(NULL);
    others[2] = pico8_nr_new(NULL);
    others[3] = pico8_fr_new(NULL);
    for (k = 0; k < 4; k++) {
        h = fnv(h, (uint32_t)me_of(others[k], "ht", &v));
    }

    check("mx_members", h, MX_MEMBERS_HASH);
}

static void test_mx_writer_stops_off_row(void) {
    pico8_value_t off_row[11];
    void *e;
    size_t n_off;
    size_t k;
    pico8_list_t *list = malloc(sizeof *list);

    (void)start_game();
    pico8_list_init(list);
    e = pico8_mx_new(NULL);
    n_off = 0;
    off_row[n_off++] = pico8_value_fixed(pico8_fixed_from_int(1));
    off_row[n_off++] = zero_union(PICO8_VALUE_FIXED);
    off_row[n_off++] = pico8_value_bool(1);
    off_row[n_off++] = pico8_value_bool(0);
    off_row[n_off++] = zero_union(PICO8_VALUE_BOOL);
    off_row[n_off++] = pico8_value_string("x");
    off_row[n_off++] = zero_union(PICO8_VALUE_STRING);
    off_row[n_off++] = zero_union(PICO8_VALUE_FUNCTION);
    off_row[n_off++] = pico8_value_ref(list);
    off_row[n_off++] = pico8_value_ref(pico8_jo_new(NULL));
    off_row[n_off++] = pico8_value_ref(pico8_ob_table_new());
    g_off_row_name = "ht";
    for (k = 0; k < n_off; k++) {
        g_off_row_value = off_row[k];
        must_abort(write_off_row, e);
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
    in_fresh_child(test_fresh);
    in_fresh_child(test_hover);
    in_fresh_child(test_layers);
    in_fresh_child(test_right);
    in_fresh_child(test_left);
    in_fresh_child(test_cj);
    in_fresh_child(test_stale);
    in_fresh_child(test_ui_is_a_struct);
    in_fresh_child(test_ui_members_by_name);
    in_fresh_child(test_ui_held_stops_off_row);
    in_fresh_child(test_ke_closed);
    in_fresh_child(test_ke_me_by_name);
    in_fresh_child(test_ke_ks_stops_off_row);
    in_fresh_child(test_ke_t_by_name);
    in_fresh_child(test_g__t_key_stops_off_row);
    in_fresh_child(test_jo_members_by_name);
    in_fresh_child(test_jo_written_by_name);
    in_fresh_child(test_jo_writer_stops_off_row);
    in_fresh_child(test_jo_position_by_name);
    in_fresh_child(test_jo_position_stops_off_row);
    in_fresh_child(test_jo_state_by_name);
    in_fresh_child(test_jo_state_stops_off_row);
    in_fresh_child(test_jo_writers_by_name);
    in_fresh_child(test_jo_writers_stop_off_row);
    in_fresh_child(test_qh_members_by_name);
    in_fresh_child(test_qh_writer_stops_off_row);
    in_fresh_child(test_hj_members_by_name);
    in_fresh_child(test_hj_writer_stops_off_row);
    in_fresh_child(test_pm_members_by_name);
    in_fresh_child(test_mx_members_by_name);
    in_fresh_child(test_mx_writer_stops_off_row);

    printf("test_pico8_ui: all tests passed\n");
    return 0;
}
