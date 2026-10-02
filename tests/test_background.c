/* The background `bg`, read and written only through the dual-form field
 * API and driven through the real `mv`/`dg`/`dc`/`ok`, in games built
 * directly: no events file. */

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
#include "game/background.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/scroll.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "engine/vec.h"
#include "game/wormhole.h"


#define MAP_HASH 0xBBDC5BFAu
#define MAP_STARS 4
#define MAP_ZEROS 625
#define SECOND_MAP_HASH 0x2E2CECCAu
#define DRAG_HASH 0x2CB62643u
#define LDOWN_G_BEFORE 0
#define LDOWN_G_AFTER 1
#define LDOWN_MZ_BEFORE 0
#define LDOWN_MZ_AFTER 1
#define HOVER_HASH 0x477E2027u
#define DRAW_HASH 0xE18E0472u

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static const int want_pl[15] = {0, 0, 0, 72, 73, 74, 75, 88, 89, 90, 91, 88, 91, 89, 91};

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

    for (i = 0; i < P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)p8_peek(&mem, P8_ADDR_SCREEN + i);
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

static int ents_list_count(const pico8_ents_t *list) {
    return list == NULL ? 0 : list->count;
}

static int is_me(const entity_t *ke, const char *want) {
    return ke->state != NULL && strcmp(ke->state, want) == 0;
}

static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

/* `mq` as the box the cart keeps: NULL while nil. */
static const pico8_vec_t *mq_of(const pico8_bg_t *bg) {
    return bg->has_mq ? &bg->mq : NULL;
}

/* `mq` holds the value of `rp`, a copy. */
static int mq_is(const pico8_bg_t *bg, const pico8_vec_t *rp) {
    return bg->has_mq && &bg->mq != rp && bg->mq.x == rp->x && bg->mq.y == rp->y;
}

/* 0 absent or false, 2 true. */
static uint32_t m__state(const pico8_bg_t *bg) {
    return bg->m_ ? 2 : 0;
}

static const pico8_qb_t *mo_qb(void) {
    const pico8_qb_t *mo = pico8_mo_qb();
    assert(mo != NULL);
    return mo;
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

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    p8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
}

/* The background is the `bg` of cv.hd whose `hd` and `fa` are both 1. */
static pico8_bg_t *find_bg(int *count) {
    const pico8_ents_t *hd = pico8_cv("hit_test");
    pico8_bg_t *found = NULL;
    int i;

    *count = 0;
    for (i = 0; hd != NULL && i < hd->count; i++) {
        const entity_t *e = hd->items[i];
        pico8_bg_t *bg;

        if (e->class != pico8_bg_struct_class()) {
            continue;
        }
        bg = (pico8_bg_t *)e;
        if (bg->def->hd == p8_fixed_from_int(1) && bg->def->fa == p8_fixed_from_int(1)) {
            found = bg;
            (*count)++;
        }
    }
    return found;
}

static pico8_bg_t *the_bg(void) {
    int count;
    pico8_bg_t *bg = find_bg(&count);
    assert(count == 1 && bg != NULL);
    return bg;
}

static pico8_vec_t *vec_ref(int x, int y) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = pico8_vec_v(p8_fixed_from_int(x), p8_fixed_from_int(y));
    return v;
}

static int mget(int x, int y) {
    return p8_peek(&mem, P8_ADDR_SPRITE_MAP_SHARED + y * P8_MAP_WIDTH + x);
}

/* A fresh `bg`: ke's `me`/`t`, the spec's `fa`/`hd`/`pl`, the four
 * methods, nothing written yet, registered in hv, cv.kj and cv.hd only. */
static void check_fresh_bg(const pico8_bg_t *bg) {
    const entity_class_t *cls = bg->header.class;
    const pico8_int_seq_t *pl;
    size_t i;

    assert(is_me(&bg->header, "normal"));
    assert(bg->header.state_time == 0);
    assert(bg->def == &pico8_bg_def);
    assert(bg->def->fa == p8_fixed_from_int(1));
    assert(bg->def->hd == p8_fixed_from_int(1));
    pl = bg->def->pl;
    assert(pl == &pico8_bg_pl_seq && pl->count == 15);
    for (i = 0; i < 15; i++) {
        assert(pl->items[i] == want_pl[i]);
    }
    assert(!bg->has_mq && !bg->m_);
    assert(cls->accept_drop == NULL && cls->states[ENTITY_STATE_NORMAL] == NULL);
    assert(cls->draw != NULL && cls->left_press != NULL && cls->right_press != NULL && cls->drag != NULL);
    assert(cls->drop == NULL);
    assert(ents_listed(pico8_hv(), bg) && ents_listed(pico8_cv("draw"), bg) && ents_listed(pico8_cv("hit_test"), bg));
    assert(!ents_listed(pico8_cv("draw_overlay"), bg) && !ents_listed(pico8_cv("new_year"), bg) && !ents_listed(pico8_cv("refresh"), bg));
}

static void test_construction(void) {
    pico8_bg_t *from_eu;
    pico8_bg_t *built;
    pico8_bg_t *generic;

    start_game();
    from_eu = the_bg();
    check_fresh_bg(from_eu);
    built = pico8_bg_new();
    check_fresh_bg(built);
    generic = pico8_bg_new();
    check_fresh_bg(generic);
    assert(pico8_cg("background") == NULL);
    assert(from_eu->def == built->def && built->def == generic->def);
}

static int in_pl(int tile) {
    int i;
    for (i = 0; i < 15; i++) {
        if (want_pl[i] == tile) {
            return 1;
        }
    }
    return 0;
}

static uint32_t map_hash(int *stars, int *zeros) {
    uint32_t h = 2166136261u;
    int x, y;

    *stars = 0;
    *zeros = 0;
    for (y = 0; y < 32; y++) {
        for (x = 0; x < 32; x++) {
            int tile = mget(x, y);
            assert(tile == 0 || in_pl(tile) || (tile >= 122 && tile <= 126));
            *stars += tile >= 122 && tile <= 126;
            *zeros += tile == 0;
            h = fnv(h, (uint32_t)tile);
        }
    }
    return h;
}

static void test_map(void) {
    int stars;
    int zeros;
    uint32_t first;

    start_game();
    first = map_hash(&stars, &zeros);
    check("map", first, MAP_HASH);
    check_int("map stars", stars, MAP_STARS);
    check_int("map zeros", zeros, MAP_ZEROS);
    (void)pico8_bg_new();
    check("second map", map_hash(&stars, &zeros), SECOND_MAP_HASH);
}

static void hq(void *bg, pico8_vec_t *rp) {
    pico8_bg_struct_class()->drag(bg, vec_ref(1, 2), rp);
}

/* `hq` with no `mq` yet reads the zero vector; `rdown` keeps a copy of `rp`
 * and hands back the background; each `hq` then moves `mo.v` by the drag
 * delta and replaces `mq`. */
static void test_drag(void) {
    uint32_t h = 2166136261u;
    pico8_bg_t *bg;
    pico8_vec_t *rp;
    down_t out;
    pico8_vec_t v;
    int i;

    start_game();
    bg = the_bg();
    rp = vec_ref(10, 20);
    hq(bg, rp);
    assert(mo_qb()->v != NULL);
    v = *mo_qb()->v;
    assert(v.x == p8_fixed_from_int(-10) && v.y == p8_fixed_from_int(-20));
    assert(mq_is(bg, rp));
    h = fold_vec(h, mo_qb()->v);

    rp = vec_ref(30, 40);
    out = pico8_bg_struct_class()->right_press(&bg->header, vec_ref(5, 6), rp);
    assert(out.stop && (void *)out.held == bg);
    assert(mq_is(bg, rp));
    h = fold_vec(h, mq_of(bg));

    for (i = 0; i < 3; i++) {
        rp = vec_ref(33 - 7 * i, 41 + 5 * i * i);
        hq(bg, rp);
        assert(mq_is(bg, rp));
        h = fold_vec(h, mo_qb()->v);
        h = fold_vec(h, mq_of(bg));
    }
    check("drag", h, DRAG_HASH);
}

/* `ldown` builds a `g_` while cg.g_ is empty, an `mz` after. */
static void test_ldown(void) {
    void *bg;
    down_t out;
    int g_before;
    int mz_before;

    start_game();
    bg = the_bg();
    g_before = ents_list_count(pico8_cg("probe"));
    mz_before = ents_list_count(pico8_cg("wormhole"));
    check_int("ldown g_ before", g_before, LDOWN_G_BEFORE);
    check_int("ldown mz before", mz_before, LDOWN_MZ_BEFORE);

    out = pico8_bg_struct_class()->left_press(bg, vec_ref(1, 1), vec_ref(2, 2));
    assert(out.stop && out.held != NULL);
    assert(out.held->class == pico8_g__struct_class());
    assert(ents_listed(pico8_cg("probe"), out.held));

    out = pico8_bg_struct_class()->left_press(bg, vec_ref(1, 1), vec_ref(2, 2));
    assert(out.stop && out.held != NULL);
    assert(out.held->class == pico8_mz_struct_class());
    assert(ents_listed(pico8_cg("wormhole"), out.held));
    assert(is_me(&((const pico8_mz_t *)out.held)->header, "placing"));

    check_int("ldown g_ after", ents_list_count(pico8_cg("probe")), LDOWN_G_AFTER);
    check_int("ldown mz after", ents_list_count(pico8_cg("wormhole")), LDOWN_MZ_AFTER);
}

static int in_layer(int layer, const void *e) {
    const pico8_ui_ic_t *ic = pico8_ok()->ic;

    if (ic == NULL) {
        return 0;
    }
    return ents_listed(&ic->layers[layer], e);
}

/* Nothing else hovered: the background is `o_`, marked `m_`, in layer 1,
 * and answers both `ldown` and `rdown`; a veil built over it takes `o_`,
 * and the next hover pass unmarks it through `gj`. */
static p8_input_frame_t g_void;

static void test_hover(void) {
    uint32_t h = 2166136261u;
    pico8_bg_t *bg;
    void *veil;
    entity_t *kv = NULL;
    down_t kn;

    start_game();
    bg = the_bg();
    memset(&g_void, 0, sizeof g_void);
    g_void.mouse_x = 64;
    g_void.mouse_y = 100;
    input.frames = &g_void;
    input.frame_count = 1;
    input.current_frame = 0;
    pico8_dispatch_set_input(&input);
    pico8_dg();

    pico8_ui_lj(&mem, pico8_ok());
    assert((void *)pico8_ok()->o_ == bg);
    assert(m__state(bg) == 2);
    assert(in_layer(1, bg));
    h = fnv(h, (uint32_t)p8_peek(&mem, 0x5f80));
    h = fnv(h, m__state(bg));

    assert(pico8_ui_jz(pico8_ok(), PICO8_JZ_LDOWN, vec_ref(1, 1), vec_ref(2, 2), NULL, &kv, &kn));
    assert((void *)kv == bg && kn.stop && kn.held != NULL && (void *)kn.held != bg);
    kv = NULL;
    assert(pico8_ui_jz(pico8_ok(), PICO8_JZ_RDOWN, vec_ref(1, 1), vec_ref(2, 2), NULL, &kv, &kn));
    assert((void *)kv == bg && kn.stop && (void *)kn.held == bg);
    h = fold_vec(h, mq_of(bg));

    veil = (void *)pico8_fq_new(NULL);
    pico8_ui_lj(&mem, pico8_ok());
    assert((void *)pico8_ok()->o_ == veil);
    assert(m__state(bg) == 0);
    h = fnv(h, (uint32_t)p8_peek(&mem, 0x5f80));
    h = fnv(h, m__state(bg));
    check("hover", h, HOVER_HASH);
}

static pico8_jo_t *first_jo_with_me(const char *me) {
    const pico8_ents_t *list = pico8_cg("planet");
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        entity_t *e = list->items[i];

        assert(pico8_jo_is_family(e));
        if (is_me(&((const pico8_jo_t *)e)->header, me)) {
            return (pico8_jo_t *)e;
        }
    }
    return NULL;
}

/* Released over a colonized planet or a slipgate, `jz("gv",held)` reads
 * `s.qu` off the background, which it never has. */
static void test_gv(void) {
    pico8_bg_t *bg;
    pico8_jo_t *ly;
    void *nr;
    planet_keys_t keys;
    down_t r;

    start_game();
    pico8_jo_hu((pico8_jo_t *)pico8_cg("planet")->items[0], pico8_hp_new(NULL));
    bg = the_bg();
    ly = first_jo_with_me("settled");
    assert(ly != NULL);
    r = pico8_jo_gv(ly, &bg->header);
    assert(r.stop && r.held == NULL);
    assert(is_me(&ly->header, "settled"));

    keys.planet_id = NULL;
    keys.map_pos = ly->p;
    keys.pos = ly->ra;
    nr = pico8_nr_new(&keys);
    r = pico8_nr_gv(nr, &bg->header);
    assert(r.stop && r.held == NULL);
}

/* `bg` alone in cv.kj, drawn by `dc` with the camera `mo` puts at its
 * initial position. */
static void test_draw(void) {
    void *bg;

    pico8_jl();
    bg = (void *)pico8_bg_new();
    assert(ents_list_count(pico8_cv("draw")) == 1 && ents_listed(pico8_cv("draw"), bg));
    p8_gfx_init(&gfx);
    p8_cls(&mem);
    assert(pico8_mo()->class == pico8_qb_struct_class());
    pico8_qb_md(pico8_dispatch_gfx(), (pico8_qb_t *)pico8_mo(), p8_fixed_from_int(1));
    pico8_dc("draw");
    check("draw", screen_hash(), DRAW_HASH);
}

enum { HELD_ABSENT, HELD_G_STRUCT, HELD_MZ_STRUCT, HELD_OTHER };

static uint32_t lheld_kind(void) {
    const void *held = pico8_ok()->lheld;

    if (held == NULL) {
        return HELD_ABSENT;
    }
    if (held != NULL &&
        ((const entity_t *)held)->class == pico8_g__struct_class()) {
        return HELD_G_STRUCT;
    }
    if (held != NULL &&
        ((const entity_t *)held)->class == pico8_mz_struct_class()) {
        return HELD_MZ_STRUCT;
    }
    return HELD_OTHER;
}

static p8_input_frame_t g_drag[8];

static void drag_frame(int i, int x, int y, int buttons) {
    memset(&g_drag[i], 0, sizeof g_drag[i]);
    g_drag[i].mouse_x = x;
    g_drag[i].mouse_y = y;
    g_drag[i].mouse_buttons = buttons;
}

/* Over empty space, the right button held makes the background `ok.rheld`
 * until released; the left button pressed there holds the `g_` its `ldown`
 * builds. */
static void test_scripted_drags(void) {
    pico8_bg_t *bg;
    int i;

    start_game();
    bg = the_bg();
    for (i = 0; i < 4; i++) {
        drag_frame(i, 64 + 3 * i, 100 - 2 * i, i < 3 ? 2 : 0);
    }
    drag_frame(4, 64, 100, 0);
    drag_frame(5, 64, 100, 1);
    drag_frame(6, 70, 96, 1);
    input.frames = g_drag;
    input.frame_count = 7;
    input.current_frame = 0;
    pico8_dispatch_set_input(&input);
    for (i = 0; i < 7; i++) {
        int count;

        pico8__update60();
        pico8__draw();
        assert(find_bg(&count) == bg && count == 1);
        assert((pico8_ok()->rheld == bg) == (i < 3));
        if (i == 6) {
            assert(lheld_kind() == HELD_G_STRUCT);
        }
        if (i < 6) {
            p8_input_step(&input);
        }
    }
    p8_input_init(&input);
    pico8_dispatch_set_input(&input);
}

static int is_a_bg_struct(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == pico8_bg_struct_class();
}

/* Built by `eu()`, `pico8_bg_new` or through its class, a background is a
 * struct with `kj`, `ldown`, `rdown` and `hq` as methods. */
static void test_bg_is_a_struct(void) {
    const entity_class_t *cls;
    void *from_eu;
    void *built;
    void *generic;
    void *veil;

    start_game();
    from_eu = the_bg();
    built = (void *)pico8_bg_new();
    generic = (void *)pico8_bg_new();
    assert(is_a_bg_struct(from_eu) && is_a_bg_struct(built) && is_a_bg_struct(generic));
    cls = ((const entity_t *)built)->class;
    assert(cls->draw != NULL && cls->left_press != NULL && cls->right_press != NULL && cls->drag != NULL);
    assert(cls->states[ENTITY_STATE_NORMAL] == NULL && cls->accept_drop == NULL && cls->draw_overlay == NULL);
    assert(cls->drop == NULL && cls->draw_at == NULL);

    veil = (void *)pico8_fq_new(NULL);
    assert(((const entity_t *)veil)->class->right_press == NULL);
}

/* A scenario depends on the rnd state: run each in a child forked right
 * after pico8_cart_load(), as a fresh process would start it. */
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
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    in_fresh_child(test_construction);
    in_fresh_child(test_map);
    in_fresh_child(test_drag);
    in_fresh_child(test_ldown);
    in_fresh_child(test_hover);
    in_fresh_child(test_gv);
    in_fresh_child(test_draw);
    in_fresh_child(test_scripted_drags);
    in_fresh_child(test_bg_is_a_struct);

    printf("test_pico8_bg: all tests passed\n");
    return 0;
}
