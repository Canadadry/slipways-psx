/* The click dispatcher `ui`, the singleton `ok`, built by its only road
 * (`jl()`, `pico8_ui_new()`, on a game started directly: no events
 * file), read only through the dual-form field API and compared with the
 * archetype `pico8_ui_def` and `pico8_ui_le_shared()`, and
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

#include "p8/assets.h"
#include "p8/audio.h"
#include "game/background.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/help.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "p8/mem.h"
#include "game/menu.h"
#include "game/mouse.h"
#include "game/message_box.h"
#include "game/particle.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/probe.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/score_panel.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "engine/vec.h"
#include "game/wormhole.h"

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
#define KE_ME_HASH 0xD2ED2BC5u
#define KE_T_HASH 0x5DB51DEEu

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

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

static int ents_rank_in(const pico8_ents_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int ents_listed(const pico8_ents_t *list, const void *e) {
    return ents_rank_in(list, e) >= 0;
}

static const pico8_ents_t *le_of(const pico8_ui_t *ok) {
    assert(ok->le != NULL);
    return ok->le;
}

static pico8_vec_t vec_at(const pico8_vec_t *p) {
    assert(p != NULL);
    return *p;
}

static const pico8_nf_t *je_nf(void) {
    const pico8_nf_t *je = pico8_je_nf();
    assert(je != NULL);
    return je;
}

static const pico8_qb_t *mo_qb(void) {
    const pico8_qb_t *mo = pico8_mo_qb();
    assert(mo != NULL);
    return mo;
}

static pico8_jo_t *as_jo(void *e) {
    assert(pico8_jo_is_family(e));
    return e;
}

/* 0 absent or false, 2 true. */
static uint32_t m__state(const pico8_jo_t *jo) {
    return jo->m_ ? 2 : 0;
}

static uint32_t fold_vec(uint32_t h, pico8_vec_t v) {
    return fnv(fnv(h, (uint32_t)v.x), (uint32_t)v.y);
}

/* Presence, then the rank in `hv` of an entity. */
static uint32_t fold_entity(uint32_t h, const void *ref) {
    h = fnv(h, (uint32_t)(ref != NULL));
    return fnv(h, (uint32_t)ents_rank_in(pico8_hv(), ref));
}

static int layer_count(const pico8_ui_t *ok, int mf) {
    return ok->ic == NULL ? 0 : ok->ic->layers[mf].count;
}

static int in_layer(const pico8_ui_t *ok, int mf, const void *e) {
    return ok->ic != NULL && ents_listed(&ok->ic->layers[mf], e);
}

static uint32_t fold_layers(uint32_t h, const pico8_ui_t *ok) {
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
    pico8_vec_t at = vec_at(je_nf()->ra);
    uint32_t drawn;

    pico8_cls(&mem);
    pico8_ui_ev(&gfx, &mem, pico8_ok());
    drawn = screen_hash();
    pico8_cls(&mem);
    pico8_spr_default(&gfx, &mem, id, pico8_fixed_flr_int(at.x), pico8_fixed_flr_int(at.y));
    assert(drawn == screen_hash());
    return drawn;
}

static int is_of(const void *e, const entity_class_t *cls) {
    return e != NULL && ((const entity_t *)e)->class == cls;
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static pico8_ui_t *start_game(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
    pico8_cart_vider_file_sortie();
    assert(pico8_ok() != NULL);
    return pico8_ok();
}

static pico8_vec_t pos_of(void *e) {
    return vec_at(as_jo(e)->p);
}

static pico8_fixed_t dist2(pico8_vec_t a, pico8_vec_t b) {
    pico8_vec_t d = pico8_vec_sub(a, b);
    return pico8_fixed_add(pico8_fixed_mul(d.x, d.x), pico8_fixed_mul(d.y, d.y));
}

static int placed(void *e) {
    return as_jo(e)->p != NULL;
}

/* The planet of `cg.jo` nearest to `qu`, among those with a `p`. */
static void *nearest_to(void *qu) {
    const pico8_ents_t *jos = pico8_cg("jo");
    void *re = NULL;
    int i;

    for (i = 0; i < jos->count; i++) {
        void *e = jos->items[i];
        if (e != qu && placed(e) && (re == NULL || dist2(pos_of(e), pos_of(qu)) < dist2(pos_of(re), pos_of(qu)))) {
            re = e;
        }
    }
    assert(re != NULL);
    return re;
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
    const pico8_ents_t *hv = pico8_hv();
    void *bg = NULL;
    int i;
    for (i = 0; i < hv->count; i++) {
        if (is_of(hv->items[i], pico8_bg_struct_class())) {
            assert(bg == NULL);
            bg = hv->items[i];
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
    pico8_ui_t *ok = start_game();
    size_t i;

    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), ok));
    h = fnv(h, (uint32_t)pico8_hv()->count);
    assert(ents_listed(pico8_cv("ev"), ok));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cv("ev"), ok));
    for (i = 0; i < sizeof not_listed / sizeof not_listed[0]; i++) {
        assert(!ents_listed(pico8_cv(not_listed[i]), ok));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        assert(!ents_listed(pico8_cg(tags[i]), ok));
    }

    assert(ok->def->ql == pico8_fixed_from_int(58));
    assert(ok->def->fa == pico8_fixed_from_int(15));
    assert(ok->header.state != NULL && strcmp(ok->header.state, "ph") == 0);
    /* The presence word is constant by the struct's shape, kept so that
     * BUILT_HASH still proves the typed read. */
    h = fnv(fnv(h, 1u), (uint32_t)ok->header.state_time);
    assert(ok->qa == NULL && ok->ic == NULL && ok->o_ == NULL && ok->gj == NULL && ok->lheld == NULL &&
           ok->rheld == NULL);
    assert(pico8_ui_struct_class()->draw_overlay != NULL);
    assert(pico8_ui_struct_class()->states[ENTITY_STATE_NORMAL] == NULL && pico8_ui_struct_class()->left_press == NULL &&
           pico8_ui_struct_class()->right_press == NULL && pico8_ui_struct_class()->hit_test == NULL);

    assert(ok->le == pico8_ui_le_shared());
    assert(ok->def == &pico8_ui_def);
    assert(ok->def->hw.xl == pico8_fixed_from_int(10) && ok->def->hw.yt == pico8_fixed_from_int(10));
    assert(ok->def->hw.xr == pico8_fixed_from_int(118) && ok->def->hw.yb == pico8_fixed_from_int(118));
    assert(le_of(ok)->count == 0);
    check("built", h, BUILT_HASH);
}

static void call_ix_my(int i) {
    pico8_ix_my(i)(NULL);
}

/* A second `ui` shares the class's empty `le` until its own `cj`; `pz`
 * replaces `ok.le`, and a `p` with `ff` empties it. */
static void test_shared(void) {
    uint32_t h = 2166136261u;
    pico8_ui_t *ok = start_game();
    pico8_ui_t *other = pico8_ui_new();
    const pico8_ents_t *le = ok->le;

    assert(other != NULL && other != ok);
    assert(other->header.cursor_sprite == &other->def->ql);
    assert(other->le == le && le == pico8_ui_le_shared());
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), other));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_cv("ev"), other));

    pico8_ui_cj(other);
    assert(other->le != le && le_of(other)->count == 0);
    assert(ok->le == le && pico8_ui_le_shared() == le && le->count == 0);

    call_ix_my(4);
    assert(ok->le != le && other->le != ok->le);
    h = fnv(h, (uint32_t)le_of(ok)->count);
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), le_of(ok)->items[0]));
    assert(pico8_ui_le_shared() == le && le->count == 0);

    (void)pico8_fv_new();
    assert(le_of(ok)->count == 0 && ok->le != le);
    h = fnv(h, (uint32_t)pico8_hv()->count);
    check("shared", h, SHARED_HASH);
}

/* A fresh game's first `lj`: `qa` = `je.ra` + `mo.ra`, `ic` built, `gj` is
 * `o_`, and `ev` draws sprite 58 at `je.ra`. */
static void test_fresh(void) {
    uint32_t h = 2166136261u;
    pico8_ui_t *ok = start_game();
    pico8_vec_t want;

    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, pico8_ok());
    want = pico8_vec_add(vec_at(je_nf()->ra), vec_at(mo_qb()->ra));
    assert(vec_at(ok->qa).x == want.x && vec_at(ok->qa).y == want.y);
    assert(ok->ic != NULL && ok->o_ != NULL);
    assert(ok->gj == ok->o_);
    assert((void *)ok->o_ == the_bg());
    h = fold_vec(h, vec_at(ok->qa));
    h = fold_layers(h, ok);
    h = fold_entity(h, ok->o_);
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    h = fnv(h, cursor_is(58));
    assert(ok->le == pico8_ui_le_shared());
    check("fresh", h, FRESH_HASH);
}

/* The first planet of `cg.jo` whose screen position is well inside. */
static pico8_jo_t *planet_on_screen(int *sx, int *sy) {
    const pico8_ents_t *jo = pico8_cg("jo");
    pico8_vec_t cam = vec_at(mo_qb()->ra);
    int i;

    for (i = 0; jo != NULL && i < jo->count; i++) {
        pico8_vec_t at = vec_at(as_jo(jo->items[i])->ra);
        int x = pico8_fixed_flr_int(pico8_fixed_sub(at.x, cam.x));
        int y = pico8_fixed_flr_int(pico8_fixed_sub(at.y, cam.y));
        if (x >= 16 && x <= 112 && y >= 16 && y <= 112) {
            *sx = x;
            *sy = y;
            return (pico8_jo_t *)jo->items[i];
        }
    }
    assert(0 && "no planet on screen");
    return NULL;
}

/* The mouse over a planet: `o_` is the planet, marked `m_`, its `fg` poked
 * at 0x5f80, `gj` on the next pass; the mouse away unmarks it. */
static void test_hover(void) {
    uint32_t h = 2166136261u;
    pico8_ui_t *ok = start_game();
    int x;
    int y;
    pico8_jo_t *planet;

    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, ok);
    planet = planet_on_screen(&x, &y);
    mouse_at(x, y, 0);
    pico8_ui_lj(&mem, ok);
    assert((void *)ok->o_ == planet);
    assert(m__state(planet) == 2);
    /* A planet has no `fg`: 0x5f80 is poked with 0. */
    assert(pico8_peek(&mem, 0x5f80) == 0);
    assert(in_layer(ok, 6, planet) && in_layer(ok, 1, the_bg()));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), planet));
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    h = fold_layers(h, ok);

    pico8_ui_lj(&mem, ok);
    assert((void *)ok->gj == planet && (void *)ok->o_ == planet);
    assert(m__state(planet) == 2);
    h = fnv(h, cursor_is(58));

    mouse_at(x + 40 > 127 ? x - 40 : x + 40, y, 0);
    pico8_ui_lj(&mem, ok);
    assert((void *)ok->o_ != planet);
    assert(m__state(planet) == 0);
    h = fold_entity(h, ok->o_);
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    check("hover", h, HOVER_HASH);
}

/* `jz` walks the layers from 15 down: over a planet, `hd` answers from the
 * planet (layer 6) before the background (layer 1); `rdown` only from the
 * background, which answers itself. */
static void test_layers(void) {
    uint32_t h = 2166136261u;
    pico8_ui_t *ok = start_game();
    entity_t *kv = NULL;
    down_t kn;
    pico8_vec_t *qa;
    pico8_vec_t *ra;
    pico8_jo_t *planet;
    int x;
    int y;

    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, ok);
    planet = planet_on_screen(&x, &y);
    mouse_at(x, y, 0);
    pico8_ui_lj(&mem, ok);
    qa = ok->qa;
    ra = je_nf()->ra;
    assert(qa != NULL && ra != NULL);

    pico8_ke_ks(&planet->header, "known");
    assert(pico8_ui_jz(pico8_ok(), PICO8_JZ_LDOWN, qa, ra, NULL, &kv, &kn));
    assert((void *)kv == planet && kn.stop && kn.held == NULL);
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), kv));
    kv = NULL;
    assert(pico8_ui_jz(pico8_ok(), PICO8_JZ_RDOWN, qa, ra, NULL, &kv, &kn));
    assert((void *)kv == the_bg() && kn.stop && (void *)kn.held == the_bg());
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), kv));
    check("layers", h, LAYERS_HASH);
}

/* The right button held scrolls through the background's `rdown`/`hq`:
 * `rheld` is the background and the cursor is sprite 9; released, `rheld`
 * goes. */
static void test_right(void) {
    uint32_t h = 2166136261u;
    pico8_ui_t *ok = start_game();
    void *bg;

    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, ok);
    bg = the_bg();
    mouse_at(64, 100, 2);
    pico8_ui_lj(&mem, ok);
    assert((void *)ok->rheld == bg && ok->lheld == NULL);
    h = fnv(h, cursor_is(9));

    mouse_at(70, 90, 2);
    pico8_ui_lj(&mem, ok);
    assert((void *)ok->rheld == bg);
    h = fold_vec(h, vec_at(mo_qb()->ra));
    h = fnv(h, cursor_is(9));

    mouse_at(70, 90, 0);
    pico8_ui_lj(&mem, ok);
    assert(ok->rheld == NULL && ok->lheld == NULL);
    h = fold_vec(h, vec_at(mo_qb()->ra));
    h = fnv(h, cursor_is(58));
    check("right", h, RIGHT_HASH);
}

static int oe_count(void *planet) {
    const pico8_ents_t *oe = as_jo(planet)->oe;
    return oe == NULL ? 0 : oe->count;
}

/* Where `e` is on screen under `mo`'s camera. */
static void screen_of(void *e, int *x, int *y) {
    pico8_vec_t at = pico8_vec_sub(vec_at(as_jo(e)->ra), vec_at(mo_qb()->ra));

    *x = pico8_fixed_flr_int(at.x);
    *y = pico8_fixed_flr_int(at.y);
}

static int on_screen(void *e) {
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
    const pico8_ents_t *jos;
    pico8_ui_t *ok;
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
        void *e = jos->items[i];
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
    held = ok->lheld;
    assert(held != NULL && is_of(held, pico8_jc_struct_class()) && ok->rheld == NULL);
    assert((void *)((const pico8_jc_t *)held)->qu == origin && oe_count(origin) == 0);
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), held));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), origin));
    h = fnv(h, cursor_is(58));

    screen_of(other, &x, &y);
    mouse_at(x, y, 1);
    pico8_ui_lj(&mem, ok);
    assert((void *)ok->lheld == held);
    target = ok->o_;
    assert(target == other);
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), target));

    mouse_at(x, y, 0);
    pico8_ui_lj(&mem, ok);
    assert(ok->lheld == NULL && ok->rheld == NULL);
    assert(oe_count(origin) == 1 && oe_count(target) == 1);
    h = fnv(h, (uint32_t)pico8_cg("jc")->count);
    check("left", h, LEFT_HASH);
}

/* `cj` unregisters every entity of the old `le` and starts a new list. */
static void test_cj(void) {
    uint32_t h = 2166136261u;
    pico8_ui_t *ok = start_game();
    const pico8_ents_t *le;
    int count;
    int i;

    call_ix_my(4);
    le = le_of(ok);
    count = le->count;
    assert(count > 0);
    for (i = 0; i < count; i++) {
        assert(ents_listed(pico8_hv(), le->items[i]));
        h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), le->items[i]));
    }
    h = fnv(h, (uint32_t)pico8_hv()->count);
    pico8_ui_cj(pico8_ok());
    assert(ok->le != le && le_of(ok)->count == 0 && ok->le != pico8_ui_le_shared());
    for (i = 0; i < count; i++) {
        assert(!ents_listed(pico8_hv(), le->items[i]));
    }
    h = fnv(h, (uint32_t)pico8_hv()->count);
    check("cj", h, CJ_HASH);
}

/* On the title screen, a click on a difficulty button runs `eu()` from
 * inside `lj`: `jl()` rebuilds `ok`, and the end of `lj` still runs on the
 * old one, whose `cj` and `gj` land on it, not on the new `ok`. */
static void test_stale(void) {
    uint32_t h = 2166136261u;
    pico8_ui_t *old;
    void *button;
    const pico8_ents_t *le;

    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    old = pico8_ok();
    mouse_at(76, 71, 0);
    pico8_ui_lj(&mem, old);
    button = old->o_;
    le = le_of(old);
    assert(button != NULL && is_of(button, pico8_js_struct_class()) && le->count == 0);
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), button));

    mouse_at(76, 71, 1);
    pico8_ui_lj(&mem, old);
    assert(pico8_ok() != old && pico8_ok() != NULL);
    assert(old->gj == old->o_ && old->gj != NULL && is_of(old->gj, pico8_js_struct_class()));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), old->gj));
    assert(old->le != le && le_of(old)->count == 0);
    assert(pico8_ok()->gj == NULL && pico8_ok()->qa == NULL);
    h = fnv(h, (uint32_t)le_of(pico8_ok())->count);
    h = fnv(h, (uint32_t)(pico8_ok()->le == pico8_ui_le_shared()));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), pico8_ok()));
    h = fnv(h, (uint32_t)pico8_hv()->count);
    check("stale", h, STALE_HASH);
}

/* With a panel up, `lj` hands the left click to an entity of `le`: the
 * third row of `ha` runs its `my`, which flips `ih`. */
static void test_click_reaches_a_row_of_le(void) {
    pico8_ui_t *ok = start_game();
    void *row;
    int ih;
    int x;
    int y;
    int hx = -1;
    int hy = -1;

    call_ix_my(4);
    assert(le_of(ok)->count == 4);
    row = le_of(ok)->items[2];
    for (y = 0; y < 128 && hx < 0; y += 2) {
        for (x = 0; x < 128 && hx < 0; x += 2) {
            mouse_at(x, y, 0);
            pico8_ui_lj(&mem, ok);
            if ((void *)ok->o_ == row) {
                hx = x;
                hy = y;
            }
        }
    }
    assert(hx >= 0);
    ih = pico8_ih();
    mouse_at(hx, hy, 1);
    pico8_ui_lj(&mem, ok);
    assert(pico8_ih() == !ih);
    pico8_set_ih(ih);
}

/* `pico8_ui_init()` empties the shared `le`. */
static void test_init_empties_shared_le(void) {
    static entity_t mark;

    (void)start_game();
    pico8_ents_add(pico8_ui_le_shared(), &mark);
    assert(pico8_ui_le_shared()->count == 1);
    pico8_ui_init();
    assert(pico8_ui_le_shared()->count == 0);
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

static void bh_on_x(void *e) {
    pico8_ui_bh(e, "x");
}

/* The game's `ok` is a struct whose descriptor holds only `ev`; it takes
 * no construction key and no button but `l`/`r`. Every
 * new `ui` starts on the class's generated defaults and its shared `le`. */
static void test_ui_is_a_struct(void) {
    const entity_class_t *cls = pico8_ui_struct_class();
    pico8_ui_t *ok = start_game();
    pico8_ui_t *other;
    int j;

    assert(is_of(ok, cls));
    assert(cls->draw_overlay != NULL && cls->draw == NULL && cls->hit_test == NULL && cls->new_year == NULL && cls->refresh == NULL &&
           cls->left_press == NULL && cls->right_press == NULL && cls->accept_drop == NULL && cls->drag == NULL && cls->drop == NULL &&
           cls->attach_slipway == NULL && cls->offer_trade == NULL && cls->draw_at == NULL && cls->spawn_particle == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(pico8_ui_ready());
    assert(ok->def == &pico8_ui_def);
    assert(pico8_ui_hw() == &ok->def->hw);
    assert(ok->le == pico8_ui_le_shared() && ok->le->count == 0);
    other = pico8_ui_new();
    assert(is_of(other, cls) && other->le == pico8_ui_le_shared() && other->def == ok->def);

    must_abort(bh_on_x, ok);
}

typedef struct {
    entity_t header;
} giver_t;

static entity_t *g_given;
static int g_giver_hovered = 1;

static down_t giver_down(entity_t *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    down_t out;

    (void)self;
    (void)qa;
    (void)ra;
    out.stop = g_given != NULL;
    out.held = g_given;
    return out;
}

static int giver_hd(entity_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
    (void)self;
    (void)qa;
    (void)ra;
    return g_giver_hovered;
}

static const entity_class_t g_giver_class = {
    .hit_test = giver_hd,
    .left_press = giver_down,
    .right_press = giver_down,
};

static const pico8_fixed_t g_giver_fa = 0xf0000;
static int g_giver_m_;
static giver_t g_giver = {{.class = &g_giver_class, .layer = &g_giver_fa, .is_hovered = &g_giver_m_}};

static void press(void *ok, int buttons) {
    mouse_at(64, 100, 0);
    pico8_ui_lj(&mem, ok);
    mouse_at(64, 100, buttons);
    pico8_ui_lj(&mem, ok);
}

static int g_hold_buttons;

static void hold_given(void *ok) {
    press(ok, g_hold_buttons);
}

static void test_ui_held_stops_off_row(void) {
    pico8_ui_t *ok = start_game();
    void *left[8];
    void *right[6];
    size_t i;

    pico8_lt(&g_giver.header);
    left[0] = pico8_pm_new(NULL);
    left[1] = pico8_ga_new(NULL);
    left[2] = pico8_cg("jo")->items[0];
    left[3] = the_bg();
    left[4] = pico8_fv_new();
    left[5] = pico8_js_new(NULL);
    left[6] = pico8_ie_new(NULL);
    left[7] = pico8_mz_jj_wrapper();
    right[0] = pico8_jc_new(NULL);
    right[1] = pico8_g__new(NULL);
    right[2] = pico8_mz_new();
    right[3] = pico8_qh_new(NULL);
    right[4] = left[2];
    right[5] = left[4];

    g_hold_buttons = 1;
    for (i = 0; i < 8; i++) {
        g_given = left[i];
        must_abort(hold_given, ok);
    }
    g_hold_buttons = 2;
    for (i = 0; i < 6; i++) {
        g_given = right[i];
        must_abort(hold_given, ok);
    }
}

/* `ks` takes a struct or NULL, and does nothing on NULL. */
static void test_ke_closed(void) {
    (void)start_game();
    pico8_ke_ks(NULL, "ph");
}

/* Every entity struct begins with its header. */
static entity_t *ke_of(void *e) {
    assert(e != NULL);
    return e;
}

static uint32_t fold_string(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0u);
}

static int identity_of(const char *s) {
    if (s == pico8_bg_def.me) {
        return 1;
    }
    if (s == pico8_jo_def.me) {
        return 2;
    }
    if (s == pico8_qh_def.me) {
        return 3;
    }
    if (s == pico8_pm_def.me) {
        return 4;
    }
    return 0;
}

static uint32_t fold_ks_back(uint32_t h, void *e) {
    assert(ke_of(e)->state_time == 0);
    return fnv(h, (uint32_t)ke_of(e)->state_time);
}

/* The component a host stores in itself: not shared with `other`, of the
 * same class, and inside the host's own `size` bytes. */
static void assert_own_emitter(entity_t *self, entity_t *other, size_t size) {
    entity_t *e = self;
    entity_t *o = other;
    const char *at = (const char *)e->emitter;

    assert(e->class == o->class);
    assert((const void *)at != (const void *)o->emitter);
    assert(at >= (const char *)self && at < (const char *)self + size);
}

static void assert_own_probe(entity_t *self, entity_t *other, size_t size) {
    entity_t *e = self;
    entity_t *o = other;
    const char *at = (const char *)e->targeting;

    assert(e->class == o->class);
    assert((const void *)at != (const void *)o->targeting);
    assert(at >= (const char *)self && at < (const char *)self + size);
}

/* A second help, built once `first` gave back its list: the qy_fv sort
 * holds one help's list at a time. */
static entity_t *help_alone(entity_t *first) {
    static entity_t *previous;
    entity_t *fresh;

    pico8_qy_rendre(first->emitter);
    if (previous != NULL) {
        pico8_qy_rendre(previous->emitter);
    }
    fresh = &pico8_fv_new()->header;
    previous = fresh;
    return fresh;
}

/* Which of the 24 classes carry a particle emitter (exactly hj, pm, ga, mx,
 * g_, fv) and a probe (qh's line, g_, fv, jc, mz), each in its own storage. */
static void test_components_carried(void) {
    static const int emitter[24] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
    static const int probe[24] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1};
    void *e[24];
    size_t i;

    (void)start_game();
    e[0] = pico8_bg_new();
    e[1] = pico8_nf_new();
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
    e[15] = pico8_fv_new();
    e[16] = pico8_ie_new(NULL);
    e[17] = pico8_pj_new();
    e[18] = pico8_qb_new();
    e[19] = pico8_jc_new(NULL);
    e[20] = pico8_ui_new();
    e[21] = pico8_js_new(NULL);
    e[22] = pico8_fq_new(NULL);
    e[23] = pico8_mz_new();
    for (i = 0; i < 24; i++) {
        const entity_t *header = e[i];

        assert((header->emitter != NULL) == emitter[i]);
        assert((header->targeting != NULL) == probe[i]);
    }

    assert_own_emitter(e[8], &pico8_hj_new(NULL)->header, sizeof(pico8_jo_t));
    assert_own_emitter(e[9], &pico8_pm_new(NULL)->header, sizeof(pico8_jo_t));
    assert_own_emitter(e[10], &pico8_ga_new(NULL)->header, sizeof(pico8_jo_t));
    assert_own_emitter(e[13], &pico8_mx_new(NULL)->header, sizeof(pico8_jo_t));
    assert_own_emitter(e[14], &pico8_g__new(NULL)->header, sizeof(pico8_g__t));
    assert_own_emitter(e[15], help_alone(e[15]), sizeof(pico8_g__t));
    assert_own_probe(e[5], &pico8_qh_new(NULL)->header, sizeof(pico8_jo_t));
    assert_own_probe(e[6], &pico8_nl_new(NULL)->header, sizeof(pico8_jo_t));
    assert_own_probe(e[7], &pico8_os_new(NULL)->header, sizeof(pico8_jo_t));
    assert_own_probe(e[8], &pico8_hj_new(NULL)->header, sizeof(pico8_jo_t));
    assert_own_probe(e[9], &pico8_pm_new(NULL)->header, sizeof(pico8_jo_t));
    assert_own_probe(e[11], &pico8_nr_new(NULL)->header, sizeof(pico8_jo_t));
    assert_own_probe(e[12], &pico8_fr_new(NULL)->header, sizeof(pico8_jo_t));
    assert_own_probe(e[13], &pico8_mx_new(NULL)->header, sizeof(pico8_jo_t));
    assert_own_probe(e[14], &pico8_g__new(NULL)->header, sizeof(pico8_g__t));
    assert_own_probe(e[15], help_alone(e[15]), sizeof(pico8_g__t));
    assert_own_probe(e[19], &pico8_jc_new(NULL)->header, sizeof(pico8_jc_t));
    assert_own_probe(e[23], &pico8_mz_new()->header, sizeof(pico8_mz_t));
}

/* The kind words the frozen hashes fold before a value: any other value
 * moves them. */
enum { FIXED_KIND = 0, STRING_KIND = 1 };

static void test_ke_me_by_name(void) {
    static const char *const want[24] = {"ph", "ph",      "ph", "ph", "unknown", "kz", "kz", "kz",
                                         "kz", "ly",      "unknown", "kz", "kz", "kz", "ph", "ph",
                                         "ph", "ph",      "ph", "kz", "ph", "ph", "ph", "kz"};
    static const int identity[24] = {1, 1, 1, 1, 2, 3, 3, 3, 3, 4, 2, 3, 3, 3, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0};
    char s[] = "ll";
    void *e[24];
    const pico8_ents_t *hv;
    const char *me;
    uint32_t h = 2166136261u;
    size_t i;

    (void)start_game();
    hv = pico8_hv();
    assert(hv != NULL);
    h = fnv(h, (uint32_t)hv->count);
    /* The kind word is constant by the struct's shape (`me` is a string or
     * absent), kept so that KE_ME_HASH still proves the typed reads. */
    for (i = 0; i < (size_t)hv->count; i++) {
        me = ke_of(hv->items[i])->state;
        if (me != NULL) {
            h = fold_string(fnv(fnv(h, 1u), (uint32_t)STRING_KIND), me);
        } else {
            h = fnv(h, 0u);
        }
    }

    e[0] = pico8_bg_new();
    e[1] = pico8_nf_new();
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
    e[15] = pico8_fv_new();
    e[16] = pico8_ie_new(NULL);
    e[17] = pico8_pj_new();
    e[18] = pico8_qb_new();
    e[19] = pico8_jc_new(NULL);
    e[20] = pico8_ui_new();
    e[21] = pico8_js_new(NULL);
    e[22] = pico8_fq_new(NULL);
    e[23] = pico8_mz_new();
    for (i = 0; i < 24; i++) {
        me = ke_of(e[i])->state;
        assert(me != NULL);
        assert(strcmp(me, want[i]) == 0);
        assert(identity_of(me) == identity[i]);
        h = fold_string(fnv(h, (uint32_t)STRING_KIND), me);
        h = fnv(h, (uint32_t)identity_of(me));
    }

    /* The six strangers the reader refused, constant by the struct's shape. */
    for (i = 0; i < 6; i++) {
        h = fnv(h, 0u);
    }

    pico8_ke_ks(e[0], s);
    me = ke_of(e[0])->state;
    assert(me == s);
    h = fold_ks_back(fold_string(fnv(h, 1u), me), e[0]);
    pico8_ke_ks(e[0], NULL);
    assert(ke_of(e[0])->state == NULL);
    h = fold_ks_back(fnv(h, 0u), e[0]);
    pico8_ke_ks(e[0], "ph");
    me = ke_of(e[0])->state;
    assert(me != NULL && strcmp(me, "ph") == 0);
    h = fold_ks_back(fold_string(fnv(h, 1u), me), e[0]);

    pico8_ke_ks(e[9], s);
    me = ke_of(e[9])->state;
    assert(me == s);
    h = fold_ks_back(fold_string(fnv(h, 1u), me), e[9]);

    check("ke_me", h, KE_ME_HASH);
}

/* The kind word is constant by the struct's shape (`t` is always a fixed),
 * kept so that KE_T_HASH still proves the typed reads. */
static uint32_t fold_t(uint32_t h, void *e) {
    return fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)ke_of(e)->state_time);
}

static uint32_t fold_hv_t(uint32_t h) {
    const pico8_ents_t *hv = pico8_hv();
    int i;

    assert(hv != NULL);
    h = fnv(h, (uint32_t)hv->count);
    for (i = 0; i < hv->count; i++) {
        h = fold_t(h, hv->items[i]);
    }
    return h;
}

static void assert_t(void *e, int want) {
    assert(ke_of(e)->state_time == pico8_fixed_from_int(want));
}

static void test_ke_t_by_name(void) {
    void *e[24];
    pico8_g__keys_t keyed_keys = {0};
    pico8_g__keys_t zero_keys = {0};
    pico8_g__t *keyed;
    pico8_g__t *zero;
    uint32_t h = 2166136261u;
    size_t i;

    (void)start_game();
    h = fold_hv_t(h);
    pico8_dg();
    h = fold_hv_t(h);

    e[0] = pico8_bg_new();
    e[1] = pico8_nf_new();
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
    e[15] = pico8_fv_new();
    e[16] = pico8_ie_new(NULL);
    e[17] = pico8_pj_new();
    e[18] = pico8_qb_new();
    e[19] = pico8_jc_new(NULL);
    e[20] = pico8_ui_new();
    e[21] = pico8_js_new(NULL);
    e[22] = pico8_fq_new(NULL);
    e[23] = pico8_mz_new();
    for (i = 0; i < 24; i++) {
        assert_t(e[i], 0);
        h = fold_t(h, e[i]);
    }

    /* The six strangers the reader refused, constant by the struct's shape. */
    for (i = 0; i < 6; i++) {
        h = fnv(h, 0u);
    }

    keyed_keys.has_pn = 1;
    keyed_keys.pn = pico8_fixed_from_int(7);
    keyed_keys.t = pico8_fixed_from_int(3);
    keyed_keys.has_du = 1;
    keyed_keys.du = pico8_fixed_from_int(10);
    keyed_keys.has_py = 1;
    keyed_keys.py = pico8_fixed_from_int(1);
    keyed = pico8_g__new(&keyed_keys);
    assert_t(keyed, 3);
    assert(keyed->emitter->pn == pico8_fixed_from_int(7));
    assert(keyed->emitter->du == pico8_fixed_from_int(10));
    assert(keyed->emitter->py == pico8_fixed_from_int(1));
    h = fold_t(h, keyed);

    zero_keys.t = 0;
    zero = pico8_g__new(&zero_keys);
    assert_t(zero, 0);
    assert(zero->emitter->pn == pico8_g__def.emitter.pn);
    assert(zero->emitter->du == pico8_g__def.emitter.du);
    assert(zero->emitter->py == pico8_g__def.emitter.py);
    h = fold_t(h, zero);

    pico8_ke_ks(&keyed->header, "ll");
    assert_t(keyed, 0);
    assert(keyed->header.state != NULL && strcmp(keyed->header.state, "ll") == 0);
    h = fold_string(fold_t(h, keyed), keyed->header.state);

    pico8_ke_ks(e[9], "ll");
    assert_t(e[9], 0);
    h = fold_t(h, e[9]);

    check("ke_t", h, KE_T_HASH);
}

static void test_g__t_key_stops_off_row(void) {
    pico8_g__keys_t keys = {0};
    void *made;

    (void)start_game();
    keys.t = 0;
    made = pico8_g__new(&keys);
    assert_t(made, 0);
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
    in_fresh_child(test_shared);
    in_fresh_child(test_fresh);
    in_fresh_child(test_hover);
    in_fresh_child(test_layers);
    in_fresh_child(test_right);
    in_fresh_child(test_left);
    in_fresh_child(test_cj);
    in_fresh_child(test_stale);
    in_fresh_child(test_click_reaches_a_row_of_le);
    in_fresh_child(test_init_empties_shared_le);
    in_fresh_child(test_ui_is_a_struct);
    in_fresh_child(test_ui_held_stops_off_row);
    in_fresh_child(test_ke_closed);
    in_fresh_child(test_ke_me_by_name);
    in_fresh_child(test_components_carried);
    in_fresh_child(test_ke_t_by_name);
    in_fresh_child(test_g__t_key_stops_off_row);

    printf("test_pico8_ui: all tests passed\n");
    return 0;
}
