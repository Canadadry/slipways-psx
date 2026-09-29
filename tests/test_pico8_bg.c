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
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
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
#define BG_MEMBERS_HASH 0x8410F8B5u

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

static int listed(const pico8_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i].kind == PICO8_VALUE_REF && list->items[i].as.ref == e) {
            return 1;
        }
    }
    return 0;
}

static int list_count(const pico8_list_t *list) {
    return list == NULL ? 0 : list->count;
}

static int has(const void *e, const char *name) {
    pico8_value_t v;
    return pico8_entity_get_field(e, name, &v);
}

static pico8_value_t get(const void *e, const char *name) {
    pico8_value_t v = pico8_value_ref(NULL);
    (void)pico8_entity_get_field(e, name, &v);
    return v;
}

static pico8_fixed_t must_fixed(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static int is_string(const void *e, const char *name, const char *want) {
    pico8_value_t v;
    return pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, want) == 0;
}

static uint32_t fold_vec(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    const pico8_vec_t *p;
    if (e == NULL || !pico8_entity_get_field(e, name, &v)) {
        return fnv(h, 0xDEADu);
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    p = v.as.ref;
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

/* 0 absent, 1 false, 2 true, 3 other. */
static uint32_t bool_state(const void *e, const char *name) {
    pico8_value_t v;
    if (e == NULL || !pico8_entity_get_field(e, name, &v)) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_BOOL) {
        return v.as.boolean ? 2 : 1;
    }
    return 3;
}

static const void *ok_ref(const char *name) {
    pico8_value_t v;
    if (!pico8_entity_get_field(pico8_ok(), name, &v) || v.kind != PICO8_VALUE_REF) {
        return NULL;
    }
    return v.as.ref;
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

/* The background is the entity of cv.hd whose `hd` and `fa` are both the
 * plain value 1. */
static void *find_bg(int *count) {
    const pico8_list_t *hd = pico8_cv("hd");
    void *found = NULL;
    int i;

    *count = 0;
    for (i = 0; hd != NULL && i < hd->count; i++) {
        pico8_value_t v;
        void *e = hd->items[i].as.ref;

        if (pico8_entity_get_field(e, "hd", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(1) &&
            pico8_entity_get_field(e, "fa", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(1)) {
            found = e;
            (*count)++;
        }
    }
    return found;
}

static void *the_bg(void) {
    int count;
    void *bg = find_bg(&count);
    assert(count == 1 && bg != NULL);
    return bg;
}

static pico8_value_t vec_ref(int x, int y) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
    return pico8_value_ref(v);
}

static pico8_vec_t vec_of(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return *(const pico8_vec_t *)v.as.ref;
}

static int mget(int x, int y) {
    return pico8_peek(&mem, PICO8_ADDR_SPRITE_MAP_SHARED + y * PICO8_MAP_WIDTH + x);
}

static const void *pl_of(const void *bg) {
    pico8_value_t v;
    assert(pico8_entity_get_field(bg, "pl", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

/* A fresh `bg`: ke's `me`/`t`, the spec's `fa`/`hd`/`pl`, the four
 * methods, nothing written yet, registered in hv, cv.kj and cv.hd only. */
static void check_fresh_bg(const void *bg) {
    static const char *const absent[] = {"mq", "m_", "ra", "p_", "nu", "fg", "ql", "qu", "fx"};
    const pico8_struct_class_t *cls = ((const pico8_struct_header_t *)bg)->cls;
    const pico8_int_seq_t *pl;
    size_t i;

    assert(is_string(bg, "me", "ph"));
    assert(must_fixed(bg, "t") == 0);
    assert(must_fixed(bg, "fa") == pico8_fixed_from_int(1));
    assert(must_fixed(bg, "hd") == pico8_fixed_from_int(1));
    pl = pl_of(bg);
    assert(pl == &pico8_bg_pl_seq && pl->count == 15);
    for (i = 0; i < 15; i++) {
        assert(pl->items[i] == want_pl[i]);
    }
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(bg, absent[i]));
    }
    assert(cls->gv == NULL && cls->states[PICO8_STATE_PH] == NULL);
    assert(cls->kj != NULL && cls->ldown != NULL && cls->rdown != NULL && cls->hq != NULL);
    assert(!has(bg, "hq") && cls->ey == NULL);
    assert(listed(pico8_hv(), bg) && listed(pico8_cv("kj"), bg) && listed(pico8_cv("hd"), bg));
    assert(!listed(pico8_cv("ev"), bg) && !listed(pico8_cv("hk"), bg) && !listed(pico8_cv("hb"), bg));
}

static void test_construction(void) {
    void *from_eu;
    void *built;
    void *generic;

    start_game();
    from_eu = the_bg();
    check_fresh_bg(from_eu);
    built = (void *)pico8_bg_new();
    check_fresh_bg(built);
    generic = (void *)pico8_bg_new();
    check_fresh_bg(generic);
    assert(pico8_cg("bg") == NULL);
    assert(pl_of(from_eu) == pl_of(built) && pl_of(built) == pl_of(generic));
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

static void hq(void *bg, pico8_value_t rp) {
    pico8_bg_struct_class()->hq(bg, vec_ref(1, 2).as.ref, rp.as.ref);
}

/* `hq` with no `mq` yet reads the zero vector; `rdown` keeps `rp` itself
 * and hands back the background; each `hq` then moves `mo.v` by the drag
 * delta and replaces `mq`. */
static void test_drag(void) {
    uint32_t h = 2166136261u;
    void *bg;
    pico8_value_t rp;
    pico8_down_t out;
    pico8_vec_t v;
    int i;

    start_game();
    bg = the_bg();
    rp = vec_ref(10, 20);
    hq(bg, rp);
    v = vec_of(pico8_mo(), "v");
    assert(v.x == pico8_fixed_from_int(-10) && v.y == pico8_fixed_from_int(-20));
    assert(get(bg, "mq").as.ref == rp.as.ref);
    h = fold_vec(h, pico8_mo(), "v");

    rp = vec_ref(30, 40);
    out = pico8_bg_struct_class()->rdown(bg, vec_ref(5, 6).as.ref, rp.as.ref);
    assert(out.stop && out.held == bg);
    assert(get(bg, "mq").kind == PICO8_VALUE_REF && get(bg, "mq").as.ref == rp.as.ref);
    h = fold_vec(h, bg, "mq");

    for (i = 0; i < 3; i++) {
        rp = vec_ref(33 - 7 * i, 41 + 5 * i * i);
        hq(bg, rp);
        assert(get(bg, "mq").as.ref == rp.as.ref);
        h = fold_vec(h, pico8_mo(), "v");
        h = fold_vec(h, bg, "mq");
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

    out = pico8_bg_struct_class()->ldown(bg, vec_ref(1, 1).as.ref, vec_ref(2, 2).as.ref);
    assert(out.stop && out.held != NULL);
    assert(pico8_type_tag_of(out.held) == PICO8_TYPE_TAG_STRUCT);
    assert(out.held->cls == pico8_g__struct_class());
    assert(listed(pico8_cg("g_"), out.held));

    out = pico8_bg_struct_class()->ldown(bg, vec_ref(1, 1).as.ref, vec_ref(2, 2).as.ref);
    assert(out.stop && out.held != NULL);
    assert(pico8_type_tag_of(out.held) == PICO8_TYPE_TAG_STRUCT);
    assert(out.held->cls == pico8_mz_struct_class());
    assert(listed(pico8_cg("mz"), out.held));
    assert(is_string(out.held, "me", "kz"));

    check_int("ldown g_ after", list_count(pico8_cg("g_")), LDOWN_G_AFTER);
    check_int("ldown mz after", list_count(pico8_cg("mz")), LDOWN_MZ_AFTER);
}

static int in_layer(int layer, const void *e) {
    pico8_value_t ic;

    if (!pico8_entity_get_field(pico8_ok(), "ic", &ic) || ic.kind != PICO8_VALUE_REF || ic.as.ref == NULL) {
        return 0;
    }
    return listed(&((const pico8_ui_ic_t *)ic.as.ref)->layers[layer], e);
}

/* Nothing else hovered: the background is `o_`, marked `m_`, in layer 1,
 * and answers both `ldown` and `rdown`; a veil built over it takes `o_`,
 * and the next hover pass unmarks it through `gj`. */
static pico8_input_frame_t g_void;

static void test_hover(void) {
    uint32_t h = 2166136261u;
    void *bg;
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
    assert(ok_ref("o_") == bg);
    assert(bool_state(bg, "m_") == 2);
    assert(in_layer(1, bg));
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    h = fnv(h, bool_state(bg, "m_"));

    assert(pico8_ui_jz(pico8_ok(), PICO8_JZ_LDOWN, vec_ref(1, 1).as.ref, vec_ref(2, 2).as.ref, NULL, &kv, &kn));
    assert((void *)kv == bg && kn.stop && kn.held != NULL && (void *)kn.held != bg);
    kv = NULL;
    assert(pico8_ui_jz(pico8_ok(), PICO8_JZ_RDOWN, vec_ref(1, 1).as.ref, vec_ref(2, 2).as.ref, NULL, &kv, &kn));
    assert((void *)kv == bg && kn.stop && (void *)kn.held == bg);
    h = fold_vec(h, bg, "mq");

    veil = (void *)pico8_fq_new(NULL);
    pico8_ui_lj(&mem, pico8_ok());
    assert(ok_ref("o_") == veil);
    assert(bool_state(bg, "m_") == 1);
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    h = fnv(h, bool_state(bg, "m_"));
    check("hover", h, HOVER_HASH);
}

static void *first_with_me(const char *tag, const char *me) {
    const pico8_list_t *list = pico8_cg(tag);
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (is_string(list->items[i].as.ref, "me", me)) {
            return list->items[i].as.ref;
        }
    }
    return NULL;
}

/* Released over a colonized planet or a slipgate, `jz("gv",held)` reads
 * `s.qu` off the background, which it never has. */
static void test_gv(void) {
    void *bg;
    void *ly;
    void *nr;
    pico8_jo_keys_t keys;
    pico8_down_t r;

    start_game();
    pico8_jo_hu(pico8_cg("jo")->items[0].as.ref, pico8_hp_new(NULL));
    bg = the_bg();
    ly = first_with_me("jo", "ly");
    assert(ly != NULL);
    r = pico8_jo_gv(ly, bg);
    assert(r.stop && r.held == NULL);
    assert(is_string(ly, "me", "ly"));

    keys.id = NULL;
    keys.p = get(ly, "p").as.ref;
    keys.ra = get(ly, "ra").as.ref;
    nr = pico8_nr_new(&keys);
    r = pico8_nr_gv(nr, bg);
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
    const void *held = ok_ref("lheld");

    if (held == NULL) {
        return HELD_ABSENT;
    }
    if (pico8_type_tag_of(held) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)held)->cls == pico8_g__struct_class()) {
        return HELD_G_STRUCT;
    }
    if (pico8_type_tag_of(held) == PICO8_TYPE_TAG_STRUCT &&
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
    void *bg;
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
        assert((ok_ref("rheld") == bg) == (i < 3));
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
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_bg_struct_class();
}

static void must_abort(void *e, const char *name) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        pico8_entity_set_field(e, name, pico8_value_bool(1));
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

/* Built by `eu()`, `pico8_bg_new` or through its class, a background is a
 * struct with `kj`, `ldown`, `rdown` and `hq` as methods; a generic write
 * of a name other than `m_` stops the run instead of growing a member. */
static void test_bg_is_a_struct(void) {
    const pico8_struct_class_t *cls;
    void *from_eu;
    void *built;
    void *generic;
    void *veil;
    pico8_value_t out;

    start_game();
    from_eu = the_bg();
    built = (void *)pico8_bg_new();
    generic = (void *)pico8_bg_new();
    assert(is_a_bg_struct(from_eu) && is_a_bg_struct(built) && is_a_bg_struct(generic));
    cls = ((const pico8_struct_header_t *)built)->cls;
    assert(cls->kj != NULL && cls->ldown != NULL && cls->rdown != NULL && cls->hq != NULL);
    assert(!pico8_entity_get_field(built, "hq", &out));
    assert(cls->states[PICO8_STATE_PH] == NULL && cls->gv == NULL && cls->ev == NULL);
    assert(cls->ey == NULL && cls->ol == NULL);
    pico8_entity_set_field(built, "m_", pico8_value_bool(1));
    assert(bool_state(built, "m_") == 2);
    must_abort(built, "mq");
    must_abort(built, "fa");

    veil = (void *)pico8_fq_new(NULL);
    assert(((const pico8_struct_header_t *)veil)->cls->rdown == NULL);
}

#define WITNESS 0x1234

/* Reads `name` through the raw reader and through pico8_entity_get_field,
 * which must agree; a miss leaves the witness intact in both. */
static uint32_t fold_member(uint32_t h, const void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(WITNESS);
    pico8_value_t gen = pico8_value_fixed(WITNESS);
    int got_raw = pico8_bg_struct_class()->field(e, name, &raw);
    int got_gen = pico8_entity_get_field(e, name, &gen);

    assert(got_raw == got_gen && raw.kind == gen.kind);
    h = fnv(h, (uint32_t)got_raw);
    h = fnv(h, (uint32_t)raw.kind);
    if (!got_raw) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == WITNESS && gen.as.fixed == WITNESS);
    } else if (raw.kind == PICO8_VALUE_FIXED) {
        assert(raw.as.fixed == gen.as.fixed);
        h = fnv(h, (uint32_t)raw.as.fixed);
    } else if (raw.kind == PICO8_VALUE_BOOL) {
        assert(raw.as.boolean == gen.as.boolean);
        h = fnv(h, (uint32_t)raw.as.boolean);
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

static uint32_t fold_bg(uint32_t h, const void *bg, const void *want_mq) {
    static const char *const intruders[] = {"", "x", "M_", "m", "has_m_", "ke", "header", "items"};
    pico8_value_t v;
    size_t i;

    h = fold_member(h, bg, "fa", &v);
    assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(1));
    h = fold_member(h, bg, "hd", &v);
    assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(1));
    h = fold_member(h, bg, "pl", &v);
    assert(v.kind == PICO8_VALUE_REF && v.as.ref == &pico8_bg_pl_seq);
    h = fnv(h, (uint32_t)pico8_bg_pl_seq.count);
    h = fold_member(h, bg, "mq", &v);
    if (want_mq == NULL) {
        assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == WITNESS);
    } else {
        assert(v.kind == PICO8_VALUE_REF && v.as.ref == want_mq);
        h = fold_vec(h, bg, "mq");
    }
    h = fold_member(h, bg, "m_", NULL);
    assert(is_string(bg, "me", "ph"));
    h = fold_member(h, bg, "me", NULL);
    h = fold_member(h, bg, "t", &v);
    assert(v.kind == PICO8_VALUE_FIXED);
    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        h = fold_member(h, bg, intruders[i], &v);
        assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == WITNESS);
    }
    return h;
}

static uint32_t write_m_(uint32_t h, void *bg, pico8_value_t value, int raw, uint32_t want_state, const void *want_mq) {
    if (raw) {
        assert(pico8_bg_struct_class()->set_field(bg, "m_", value) == 1);
    } else {
        pico8_entity_set_field(bg, "m_", value);
    }
    assert(bool_state(bg, "m_") == want_state);
    h = fnv(h, bool_state(bg, "m_"));
    return fold_bg(h, bg, want_mq);
}

/* Every member read by name on the game's `bg` and on a fresh one; `m_`
 * written by name (true, false, a false with a zeroed union, nil) and `mq`
 * through `rdown`/`hq`, its only writers; `gw` sets the hovered `m_`. */
static void test_bg_members_by_name(void) {
    static const char *const unwritten[] = {"fa", "hd", "pl", "mq", "me", "t", "", "m_ ", "has_m_"};
    uint32_t h = 2166136261u;
    void *a;
    void *b;
    pico8_value_t rp1;
    pico8_value_t rp2;
    pico8_down_t down;
    pico8_value_t zero_false;
    pico8_vec_t v;
    size_t i;

    start_game();
    a = the_bg();
    h = fold_bg(h, a, NULL);

    memset(&g_void, 0, sizeof g_void);
    g_void.mouse_x = 64;
    g_void.mouse_y = 100;
    input.frames = &g_void;
    input.frame_count = 1;
    input.current_frame = 0;
    pico8_dispatch_set_input(&input);
    pico8_dg();
    pico8_ui_lj(&mem, pico8_ok());
    assert(ok_ref("o_") == a);
    assert(bool_state(a, "m_") == 2);
    h = fold_bg(h, a, NULL);

    b = (void *)pico8_bg_new();
    h = fold_bg(h, b, NULL);
    assert(pl_of(a) == pl_of(b));

    rp1 = vec_ref(3, 4);
    down = pico8_bg_struct_class()->rdown(b, vec_ref(0, 0).as.ref, rp1.as.ref);
    assert(down.stop && (void *)down.held == b);
    h = fold_bg(h, b, rp1.as.ref);
    rp2 = vec_ref(5, 9);
    pico8_bg_struct_class()->hq(b, vec_ref(0, 0).as.ref, rp2.as.ref);
    v = vec_of(pico8_mo(), "v");
    assert(v.x == pico8_fixed_from_int(-2) && v.y == pico8_fixed_from_int(-5));
    h = fold_vec(h, pico8_mo(), "v");
    h = fold_bg(h, b, rp2.as.ref);
    down = pico8_bg_struct_class()->rdown(b, vec_ref(0, 0).as.ref, NULL);
    assert(down.stop && (void *)down.held == b);
    h = fold_bg(h, b, NULL);
    (void)pico8_bg_struct_class()->rdown(b, vec_ref(0, 0).as.ref, rp1.as.ref);

    memset(&zero_false, 0, sizeof zero_false);
    zero_false.kind = PICO8_VALUE_BOOL;
    h = write_m_(h, b, pico8_value_bool(1), 0, 2, rp1.as.ref);
    h = write_m_(h, b, pico8_value_bool(0), 1, 1, rp1.as.ref);
    h = write_m_(h, b, pico8_value_bool(1), 1, 2, rp1.as.ref);
    h = write_m_(h, b, zero_false, 1, 1, rp1.as.ref);
    h = write_m_(h, b, pico8_value_ref(NULL), 0, 0, rp1.as.ref);
    h = write_m_(h, b, pico8_value_ref(NULL), 0, 0, rp1.as.ref);
    h = write_m_(h, b, pico8_value_bool(1), 0, 2, rp1.as.ref);
    h = write_m_(h, b, pico8_value_ref(NULL), 1, 0, rp1.as.ref);
    h = write_m_(h, b, zero_false, 0, 1, rp1.as.ref);

    for (i = 0; i < sizeof unwritten / sizeof unwritten[0]; i++) {
        assert(pico8_bg_struct_class()->set_field(b, unwritten[i], pico8_value_bool(1)) == 0);
        assert(bool_state(b, "m_") == 1);
        h = fold_bg(h, b, rp1.as.ref);
    }
    assert(pl_of(a) == pl_of(b));
    check("bg members", h, BG_MEMBERS_HASH);
}

static void write_m_by_name(void *e, pico8_value_t v) {
    pico8_entity_set_field(e, "m_", v);
}

static void must_abort_value(void *e, void (*write)(void *, pico8_value_t), pico8_value_t v) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        write(e, v);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

/* `m_` takes nil or a boolean only; a value whose whole union is zero is
 * not nil unless it is a REF. */
static void test_bg_writer_stops_off_row(void) {
    void *a;
    void *b;
    pico8_value_t zero_fixed;
    pico8_value_t zero_string;

    start_game();
    a = the_bg();
    b = (void *)pico8_bg_new();
    memset(&zero_fixed, 0, sizeof zero_fixed);
    zero_fixed.kind = PICO8_VALUE_FIXED;
    memset(&zero_string, 0, sizeof zero_string);
    zero_string.kind = PICO8_VALUE_STRING;

    must_abort_value(b, write_m_by_name, pico8_value_fixed(pico8_fixed_from_int(1)));
    must_abort_value(b, write_m_by_name, zero_fixed);
    must_abort_value(b, write_m_by_name, zero_string);
    must_abort_value(b, write_m_by_name, pico8_value_string("x"));
    must_abort_value(b, write_m_by_name, pico8_eu_field_value());
    must_abort_value(b, write_m_by_name, pico8_value_ref(a));
    must_abort_value(b, write_m_by_name, vec_ref(1, 1));
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
    in_fresh_child(test_bg_members_by_name);
    in_fresh_child(test_bg_writer_stops_off_row);

    printf("test_pico8_bg: all tests passed\n");
    return 0;
}
