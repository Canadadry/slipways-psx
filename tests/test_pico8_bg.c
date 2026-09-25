/* The background `bg`, read and written only through the dual-form field
 * API and driven through the real `mv`/`dg`/`dc`/`ok`. The frozen values
 * were taken on the code where `bg` was still a class instance and must not
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
#include "../src/pico8_class.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"
#include "../src/pico8_wormhole.h"

#define PLAYTHROUGH "tests/fixtures/phase4_playthrough_events.json"
#define TITLE_REPLAY "tests/fixtures/long_session_events.json"
#define TITLE_LAST_FRAME 4984

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
#define TITLE_HASH 0x27DAAF8Fu
#define TITLE_RHELD_FRAMES 131
#define PLAYTHROUGH_FRAMES 224
#define PLAYTHROUGH_HASH 0x9F387594u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

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

static uint32_t fold_fixed(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    if (e == NULL || !pico8_entity_get_field(e, name, &v)) {
        return fnv(h, 0xDEADu);
    }
    assert(v.kind == PICO8_VALUE_FIXED);
    return fnv(h, (uint32_t)v.as.fixed);
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

static void start_game(const char *events) {
    pico8_value_t difficulty;

    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
    pico8__init();
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty) && difficulty.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty.as.ref));
    pico8_cheat_init(&cheat);
}

static void play_frame(void) {
    pico8_dispatch_set_input(&input);
    pico8_cheat_step(&cheat, &input);
    pico8__update60();
    pico8__draw();
    if (input.current_frame + 1 < input.frame_count) {
        pico8_input_step(&input);
    }
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
    static const char *const absent[] = {"mq", "m_", "ra", "p_", "nu", "fg", "ql", "qu", "gv", "ey", "fx", "ph"};
    static const char *const methods[] = {"kj", "ldown", "rdown", "hq"};
    const pico8_ob_table_t *pl;
    size_t i;

    assert(is_string(bg, "me", "ph"));
    assert(must_fixed(bg, "t") == 0);
    assert(must_fixed(bg, "fa") == pico8_fixed_from_int(1));
    assert(must_fixed(bg, "hd") == pico8_fixed_from_int(1));
    pl = pl_of(bg);
    assert(pico8_type_tag_of(pl) == PICO8_TYPE_TAG_OB_TABLE && pl->count == 15);
    for (i = 0; i < 15; i++) {
        pico8_value_t entry;
        assert(pico8_ob_table_get_index(pl, (int)i + 1, &entry) && entry.kind == PICO8_VALUE_FIXED);
        assert(entry.as.fixed == pico8_fixed_from_int(want_pl[i]));
    }
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(bg, absent[i]));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        assert(get(bg, methods[i]).kind == PICO8_VALUE_FUNCTION);
    }
    assert(listed(pico8_hv(), bg) && listed(pico8_cv("kj"), bg) && listed(pico8_cv("hd"), bg));
    assert(!listed(pico8_cv("ev"), bg) && !listed(pico8_cv("hk"), bg) && !listed(pico8_cv("hb"), bg));
}

static void test_construction(void) {
    void *from_eu;
    void *built;
    void *generic;

    start_game(PLAYTHROUGH);
    from_eu = the_bg();
    check_fresh_bg(from_eu);
    built = (void *)pico8_bg_new(NULL);
    check_fresh_bg(built);
    generic = (void *)pico8_class_instantiate(pico8_bg_class(), NULL);
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

    start_game(PLAYTHROUGH);
    first = map_hash(&stars, &zeros);
    check("map", first, MAP_HASH);
    check_int("map stars", stars, MAP_STARS);
    check_int("map zeros", zeros, MAP_ZEROS);
    (void)pico8_bg_new(NULL);
    check("second map", map_hash(&stars, &zeros), SECOND_MAP_HASH);
}

static void hq(void *bg, pico8_value_t rp) {
    pico8_value_t out;
    assert(pico8_mv(bg, "hq", 2, vec_ref(1, 2), rp, &out));
}

/* `hq` with no `mq` yet reads the zero vector; `rdown` keeps `rp` itself
 * and hands back the background; each `hq` then moves `mo.v` by the drag
 * delta and replaces `mq`. */
static void test_drag(void) {
    uint32_t h = 2166136261u;
    void *bg;
    pico8_value_t rp;
    pico8_value_t out;
    pico8_vec_t v;
    int i;

    start_game(PLAYTHROUGH);
    bg = the_bg();
    rp = vec_ref(10, 20);
    hq(bg, rp);
    v = vec_of(pico8_mo(), "v");
    assert(v.x == pico8_fixed_from_int(-10) && v.y == pico8_fixed_from_int(-20));
    assert(get(bg, "mq").as.ref == rp.as.ref);
    h = fold_vec(h, pico8_mo(), "v");

    rp = vec_ref(30, 40);
    assert(pico8_mv(bg, "rdown", 2, vec_ref(5, 6), rp, &out));
    assert(out.kind == PICO8_VALUE_REF && out.as.ref == bg);
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
    pico8_value_t out;
    pico8_class_t *g_class;
    int g_before;
    int mz_before;

    start_game(PLAYTHROUGH);
    bg = the_bg();
    g_class = pico8_class_lookup_named("g_");
    g_before = list_count(pico8_cg("g_"));
    mz_before = list_count(pico8_cg("mz"));
    check_int("ldown g_ before", g_before, LDOWN_G_BEFORE);
    check_int("ldown mz before", mz_before, LDOWN_MZ_BEFORE);

    assert(pico8_mv(bg, "ldown", 2, vec_ref(1, 1), vec_ref(2, 2), &out));
    assert(out.kind == PICO8_VALUE_REF && out.as.ref != NULL);
    assert(pico8_type_tag_of(out.as.ref) == PICO8_TYPE_TAG_INSTANCE);
    assert(((const pico8_instance_t *)out.as.ref)->cls == g_class);
    assert(listed(pico8_cg("g_"), out.as.ref));

    assert(pico8_mv(bg, "ldown", 2, vec_ref(1, 1), vec_ref(2, 2), &out));
    assert(out.kind == PICO8_VALUE_REF && out.as.ref != NULL);
    assert(pico8_type_tag_of(out.as.ref) == PICO8_TYPE_TAG_STRUCT);
    assert(((const pico8_struct_header_t *)out.as.ref)->cls == pico8_mz_struct_class());
    assert(listed(pico8_cg("mz"), out.as.ref));
    assert(is_string(out.as.ref, "me", "kz"));

    check_int("ldown g_ after", list_count(pico8_cg("g_")), LDOWN_G_AFTER);
    check_int("ldown mz after", list_count(pico8_cg("mz")), LDOWN_MZ_AFTER);
}

static int in_layer(int layer, const void *e) {
    pico8_value_t ic;
    pico8_value_t bucket;

    if (!pico8_entity_get_field(pico8_ok(), "ic", &ic) || ic.kind != PICO8_VALUE_REF || ic.as.ref == NULL) {
        return 0;
    }
    if (!pico8_ob_table_get_index(ic.as.ref, layer, &bucket) || bucket.kind != PICO8_VALUE_REF) {
        return 0;
    }
    return listed(bucket.as.ref, e);
}

/* Nothing else hovered: the background is `o_`, marked `m_`, in layer 1,
 * and answers both `ldown` and `rdown`; a veil built over it takes `o_`,
 * and the next hover pass unmarks it through `gj`. */
static pico8_input_frame_t g_void;

static void test_hover(void) {
    uint32_t h = 2166136261u;
    void *bg;
    void *veil;
    pico8_instance_t *kv = NULL;
    pico8_value_t kn;

    start_game(PLAYTHROUGH);
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

    assert(pico8_ui_jz(pico8_ok(), "ldown", 2, vec_ref(1, 1), vec_ref(2, 2), &kv, &kn));
    assert((void *)kv == bg && kn.kind == PICO8_VALUE_REF && kn.as.ref != NULL && kn.as.ref != bg);
    kv = NULL;
    assert(pico8_ui_jz(pico8_ok(), "rdown", 2, vec_ref(1, 1), vec_ref(2, 2), &kv, &kn));
    assert((void *)kv == bg && kn.kind == PICO8_VALUE_REF && kn.as.ref == bg);
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
    pico8_instance_t *nr;
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    pico8_value_t r;
    int i;

    start_game(PLAYTHROUGH);
    for (i = 0; i < input.frame_count; i++) {
        play_frame();
    }
    bg = the_bg();
    ly = first_with_me("jo", "ly");
    assert(ly != NULL);
    r = pico8_jo_gv(ly, bg);
    assert(r.kind == PICO8_VALUE_BOOL && r.as.boolean);
    assert(is_string(ly, "me", "ly"));

    pico8_ob_table_set_name(overrides, "p", get(ly, "p"));
    pico8_ob_table_set_name(overrides, "ra", get(ly, "ra"));
    nr = pico8_nr_new(overrides);
    pico8_ob_table_free(overrides);
    r = pico8_nr_gv(nr, bg);
    assert(r.kind == PICO8_VALUE_BOOL && r.as.boolean);
}

/* `bg` alone in cv.kj, drawn by `dc` with the camera `mo` puts at its
 * initial position. */
static void test_draw(void) {
    pico8_value_t out;
    void *bg;

    pico8_jl();
    bg = (void *)pico8_bg_new(NULL);
    assert(list_count(pico8_cv("kj")) == 1 && listed(pico8_cv("kj"), bg));
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    assert(pico8_mv(pico8_mo(), "md", 1, pico8_value_fixed(pico8_fixed_from_int(1)), pico8_value_ref(NULL), &out));
    pico8_dc("kj");
    check("draw", screen_hash(), DRAW_HASH);
}

/* From the title screen through the first two right-button drags. */
static void test_replay_from_title(void) {
    uint32_t h = 2166136261u;
    int rheld = 0;
    int i;

    pico8_input_init(&input);
    assert(pico8_input_load(&input, TITLE_REPLAY));
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_cheat_init(&cheat);
    for (i = 0; i <= TITLE_LAST_FRAME; i++) {
        int count;
        void *bg;

        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();

        bg = find_bg(&count);
        assert(count <= 1);
        h = fnv(h, (uint32_t)count);
        h = fold_vec(h, bg, "mq");
        h = fnv(h, bool_state(bg, "m_"));
        h = fold_fixed(h, bg, "t");
        h = fnv(h, (uint32_t)(bg != NULL && ok_ref("rheld") == bg));
        h = fnv(h, (uint32_t)(bg != NULL && ok_ref("o_") == bg));
        h = fold_vec(h, pico8_mo(), "v");
        rheld += bg != NULL && ok_ref("rheld") == bg;
        pico8_input_step(&input);
    }
    pico8_input_free(&input);
    check_int("title rheld frames", rheld, TITLE_RHELD_FRAMES);
    check("title replay", h, TITLE_HASH);
}

enum { HELD_ABSENT, HELD_G_INSTANCE, HELD_MZ_STRUCT, HELD_OTHER };

static uint32_t lheld_kind(void) {
    const void *held = ok_ref("lheld");

    if (held == NULL) {
        return HELD_ABSENT;
    }
    if (pico8_type_tag_of(held) == PICO8_TYPE_TAG_INSTANCE &&
        ((const pico8_instance_t *)held)->cls == pico8_class_lookup_named("g_")) {
        return HELD_G_INSTANCE;
    }
    if (pico8_type_tag_of(held) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)held)->cls == pico8_mz_struct_class()) {
        return HELD_MZ_STRUCT;
    }
    return HELD_OTHER;
}

static void test_playthrough(void) {
    uint32_t h = 2166136261u;
    int held[4] = {0, 0, 0, 0};
    int frames;
    int i;

    start_game(PLAYTHROUGH);
    frames = input.frame_count;
    for (i = 0; i < frames; i++) {
        void *bg;
        uint32_t kind;

        play_frame();
        bg = the_bg();
        kind = lheld_kind();
        held[kind]++;
        h = fnv(h, (uint32_t)(ok_ref("o_") == bg));
        h = fnv(h, bool_state(bg, "m_"));
        h = fnv(h, kind);
    }
    printf("[playthrough] lheld absent %d, g_ %d, mz %d, other %d\n", held[0], held[1], held[2], held[3]);
    assert(held[HELD_G_INSTANCE] > 0);
    check_int("playthrough frames", frames, PLAYTHROUGH_FRAMES);
    check("playthrough", h, PLAYTHROUGH_HASH);
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
    static const char *const methods[] = {"kj", "ldown", "rdown", "hq"};
    static const char *const missing[] = {"ph", "ey", "gv", "md", "ol", "ev"};
    void *from_eu;
    void *built;
    void *generic;
    void *veil;
    pico8_value_t out;
    size_t i;

    start_game(PLAYTHROUGH);
    from_eu = the_bg();
    built = (void *)pico8_bg_new(NULL);
    generic = (void *)pico8_class_instantiate(pico8_bg_class(), NULL);
    assert(is_a_bg_struct(from_eu) && is_a_bg_struct(built) && is_a_bg_struct(generic));
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        assert(pico8_entity_get_field(built, methods[i], &out) && out.kind == PICO8_VALUE_FUNCTION);
    }
    for (i = 0; i < sizeof missing / sizeof missing[0]; i++) {
        assert(!pico8_mv(built, missing[i], 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    }
    pico8_entity_set_field(built, "m_", pico8_value_bool(1));
    assert(bool_state(built, "m_") == 2);
    must_abort(built, "mq");
    must_abort(built, "fa");

    veil = (void *)pico8_fq_new(NULL);
    assert(!pico8_mv(veil, "rdown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
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
    in_fresh_child(test_replay_from_title);
    in_fresh_child(test_playthrough);
    in_fresh_child(test_bg_is_a_struct);

    printf("test_pico8_bg: all tests passed\n");
    return 0;
}
