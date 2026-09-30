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

#include "../src/pico8_assets.h"
#include "../src/pico8_bg.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_vec.h"
#include "../src/pico8_wormhole.h"


#define MAP_HASH 0xBBDC5BFAu
#define MAP_STARS 4
#define MAP_ZEROS 625
#define SECOND_MAP_HASH 0x2E2CECCAu
#define DRAG_HASH 0x2CB62643u
#define LDOWN_G_BEFORE 0
#define LDOWN_G_AFTER 1
#define LDOWN_MZ_BEFORE 0
#define LDOWN_MZ_AFTER 1
#define HOVER_HASH 0xF751E116u
#define DRAW_HASH 0xE18E0472u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

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

    for (i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)pico8_peek(&mem, PICO8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
}

static int listed(const pico8_ptrs_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return 1;
        }
    }
    return 0;
}

static int list_count(const pico8_ptrs_t *list) {
    return list == NULL ? 0 : list->count;
}

static int is_me(const pico8_struct_header_t *ke, const char *want) {
    return ke->me != NULL && strcmp(ke->me, want) == 0;
}

static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

/* 0 absent, 1 false, 2 true. */
static uint32_t m__state(const pico8_bg_t *bg) {
    if (!bg->has_m_) {
        return 0;
    }
    return bg->m_ ? 2 : 1;
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
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
}

/* The background is the `bg` of cv.hd whose `hd` and `fa` are both 1. */
static pico8_bg_t *find_bg(int *count) {
    const pico8_ptrs_t *hd = pico8_cv("hd");
    pico8_bg_t *found = NULL;
    int i;

    *count = 0;
    for (i = 0; hd != NULL && i < hd->count; i++) {
        const pico8_struct_header_t *e = hd->items[i];
        pico8_bg_t *bg;

        if (e->cls != pico8_bg_struct_class()) {
            continue;
        }
        bg = (pico8_bg_t *)e;
        if (bg->hd == pico8_fixed_from_int(1) && bg->fa == pico8_fixed_from_int(1)) {
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
    *v = pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
    return v;
}

static int mget(int x, int y) {
    return pico8_peek(&mem, PICO8_ADDR_SPRITE_MAP_SHARED + y * PICO8_MAP_WIDTH + x);
}

/* A fresh `bg`: ke's `me`/`t`, the spec's `fa`/`hd`/`pl`, the four
 * methods, nothing written yet, registered in hv, cv.kj and cv.hd only. */
static void check_fresh_bg(const pico8_bg_t *bg) {
    const pico8_struct_class_t *cls = bg->ke.header.cls;
    const pico8_int_seq_t *pl;
    size_t i;

    assert(is_me(&bg->ke.header, "ph"));
    assert(bg->ke.header.t == 0);
    assert(bg->fa == pico8_fixed_from_int(1));
    assert(bg->hd == pico8_fixed_from_int(1));
    pl = bg->pl;
    assert(pl == &pico8_bg_pl_seq && pl->count == 15);
    for (i = 0; i < 15; i++) {
        assert(pl->items[i] == want_pl[i]);
    }
    assert(bg->mq == NULL && !bg->has_m_);
    assert(cls->gv == NULL && cls->states[PICO8_STATE_PH] == NULL);
    assert(cls->kj != NULL && cls->ldown != NULL && cls->rdown != NULL && cls->hq != NULL);
    assert(cls->ey == NULL);
    assert(listed(pico8_hv(), bg) && listed(pico8_cv("kj"), bg) && listed(pico8_cv("hd"), bg));
    assert(!listed(pico8_cv("ev"), bg) && !listed(pico8_cv("hk"), bg) && !listed(pico8_cv("hb"), bg));
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
    assert(pico8_cg("bg") == NULL);
    assert(from_eu->pl == built->pl && built->pl == generic->pl);
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
    pico8_bg_struct_class()->hq(bg, vec_ref(1, 2), rp);
}

/* `hq` with no `mq` yet reads the zero vector; `rdown` keeps `rp` itself
 * and hands back the background; each `hq` then moves `mo.v` by the drag
 * delta and replaces `mq`. */
static void test_drag(void) {
    uint32_t h = 2166136261u;
    pico8_bg_t *bg;
    pico8_vec_t *rp;
    pico8_down_t out;
    pico8_vec_t v;
    int i;

    start_game();
    bg = the_bg();
    rp = vec_ref(10, 20);
    hq(bg, rp);
    assert(mo_qb()->v != NULL);
    v = *mo_qb()->v;
    assert(v.x == pico8_fixed_from_int(-10) && v.y == pico8_fixed_from_int(-20));
    assert(bg->mq == rp);
    h = fold_vec(h, mo_qb()->v);

    rp = vec_ref(30, 40);
    out = pico8_bg_struct_class()->rdown(bg, vec_ref(5, 6), rp);
    assert(out.stop && (void *)out.held == bg);
    assert(bg->mq != NULL && bg->mq == rp);
    h = fold_vec(h, bg->mq);

    for (i = 0; i < 3; i++) {
        rp = vec_ref(33 - 7 * i, 41 + 5 * i * i);
        hq(bg, rp);
        assert(bg->mq == rp);
        h = fold_vec(h, mo_qb()->v);
        h = fold_vec(h, bg->mq);
    }
    check("drag", h, DRAG_HASH);
}

/* `ldown` builds a `g_` while cg.g_ is empty, an `mz` after. */
static void test_ldown(void) {
    void *bg;
    pico8_down_t out;
    int g_before;
    int mz_before;

    start_game();
    bg = the_bg();
    g_before = list_count(pico8_cg("g_"));
    mz_before = list_count(pico8_cg("mz"));
    check_int("ldown g_ before", g_before, LDOWN_G_BEFORE);
    check_int("ldown mz before", mz_before, LDOWN_MZ_BEFORE);

    out = pico8_bg_struct_class()->ldown(bg, vec_ref(1, 1), vec_ref(2, 2));
    assert(out.stop && out.held != NULL);
    assert(out.held->cls == pico8_g__struct_class());
    assert(listed(pico8_cg("g_"), out.held));

    out = pico8_bg_struct_class()->ldown(bg, vec_ref(1, 1), vec_ref(2, 2));
    assert(out.stop && out.held != NULL);
    assert(out.held->cls == pico8_mz_struct_class());
    assert(listed(pico8_cg("mz"), out.held));
    assert(is_me(&((const pico8_mz_t *)out.held)->ke.header, "kz"));

    check_int("ldown g_ after", list_count(pico8_cg("g_")), LDOWN_G_AFTER);
    check_int("ldown mz after", list_count(pico8_cg("mz")), LDOWN_MZ_AFTER);
}

static int in_layer(int layer, const void *e) {
    const pico8_ui_ic_t *ic = pico8_ok()->ic;

    if (ic == NULL) {
        return 0;
    }
    return listed(&ic->layers[layer], e);
}

/* Nothing else hovered: the background is `o_`, marked `m_`, in layer 1,
 * and answers both `ldown` and `rdown`; a veil built over it takes `o_`,
 * and the next hover pass unmarks it through `gj`. */
static pico8_input_frame_t g_void;

static void test_hover(void) {
    uint32_t h = 2166136261u;
    pico8_bg_t *bg;
    void *veil;
    pico8_struct_header_t *kv = NULL;
    pico8_down_t kn;

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
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    h = fnv(h, m__state(bg));

    assert(pico8_ui_jz(pico8_ok(), PICO8_JZ_LDOWN, vec_ref(1, 1), vec_ref(2, 2), NULL, &kv, &kn));
    assert((void *)kv == bg && kn.stop && kn.held != NULL && (void *)kn.held != bg);
    kv = NULL;
    assert(pico8_ui_jz(pico8_ok(), PICO8_JZ_RDOWN, vec_ref(1, 1), vec_ref(2, 2), NULL, &kv, &kn));
    assert((void *)kv == bg && kn.stop && (void *)kn.held == bg);
    h = fold_vec(h, bg->mq);

    veil = (void *)pico8_fq_new(NULL);
    pico8_ui_lj(&mem, pico8_ok());
    assert((void *)pico8_ok()->o_ == veil);
    assert(m__state(bg) == 1);
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    h = fnv(h, m__state(bg));
    check("hover", h, HOVER_HASH);
}

static pico8_jo_t *first_jo_with_me(const char *me) {
    const pico8_ptrs_t *list = pico8_cg("jo");
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        pico8_struct_header_t *e = list->items[i];

        assert(pico8_jo_is_family(e));
        if (is_me(&((const pico8_jo_t *)e)->ke.header, me)) {
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
    pico8_jo_keys_t keys;
    pico8_down_t r;

    start_game();
    pico8_jo_hu(pico8_cg("jo")->items[0], pico8_hp_new(NULL));
    bg = the_bg();
    ly = first_jo_with_me("ly");
    assert(ly != NULL);
    r = pico8_jo_gv(ly, &bg->ke.header);
    assert(r.stop && r.held == NULL);
    assert(is_me(&ly->ke.header, "ly"));

    keys.id = NULL;
    keys.p = ly->p;
    keys.ra = ly->ra;
    nr = pico8_nr_new(&keys);
    r = pico8_nr_gv(nr, &bg->ke.header);
    assert(r.stop && r.held == NULL);
}

/* `bg` alone in cv.kj, drawn by `dc` with the camera `mo` puts at its
 * initial position. */
static void test_draw(void) {
    void *bg;

    pico8_jl();
    bg = (void *)pico8_bg_new();
    assert(list_count(pico8_cv("kj")) == 1 && listed(pico8_cv("kj"), bg));
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    assert(pico8_mo()->cls == pico8_qb_struct_class());
    pico8_qb_md(pico8_dispatch_gfx(), (pico8_qb_t *)pico8_mo(), pico8_fixed_from_int(1));
    pico8_dc("kj");
    check("draw", screen_hash(), DRAW_HASH);
}

enum { HELD_ABSENT, HELD_G_STRUCT, HELD_MZ_STRUCT, HELD_OTHER };

static uint32_t lheld_kind(void) {
    const void *held = pico8_ok()->lheld;

    if (held == NULL) {
        return HELD_ABSENT;
    }
    if (held != NULL &&
        ((const pico8_struct_header_t *)held)->cls == pico8_g__struct_class()) {
        return HELD_G_STRUCT;
    }
    if (held != NULL &&
        ((const pico8_struct_header_t *)held)->cls == pico8_mz_struct_class()) {
        return HELD_MZ_STRUCT;
    }
    return HELD_OTHER;
}

static pico8_input_frame_t g_drag[8];

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
            pico8_input_step(&input);
        }
    }
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
}

static int is_a_bg_struct(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_bg_struct_class();
}

/* Built by `eu()`, `pico8_bg_new` or through its class, a background is a
 * struct with `kj`, `ldown`, `rdown` and `hq` as methods. */
static void test_bg_is_a_struct(void) {
    const pico8_struct_class_t *cls;
    void *from_eu;
    void *built;
    void *generic;
    void *veil;

    start_game();
    from_eu = the_bg();
    built = (void *)pico8_bg_new();
    generic = (void *)pico8_bg_new();
    assert(is_a_bg_struct(from_eu) && is_a_bg_struct(built) && is_a_bg_struct(generic));
    cls = ((const pico8_struct_header_t *)built)->cls;
    assert(cls->kj != NULL && cls->ldown != NULL && cls->rdown != NULL && cls->hq != NULL);
    assert(cls->states[PICO8_STATE_PH] == NULL && cls->gv == NULL && cls->ev == NULL);
    assert(cls->ey == NULL && cls->ol == NULL);

    veil = (void *)pico8_fq_new(NULL);
    assert(((const pico8_struct_header_t *)veil)->cls->rdown == NULL);
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
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
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
