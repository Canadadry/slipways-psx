/* The research icon `ie`, read and written only through the dual-form field
 * API and driven through the real `mv`/`dg`/`dc`/`ok`/`_init`/`eu`. The
 * frozen values were taken on the code where `ie` was still a class
 * instance and must not be re-taken. */

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
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_geom.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"
#include "../src/pico8_wormhole.h"

#define RECORDED "tests/fixtures/events_recorded_1790198294.json"
#define HUB "tests/fixtures/trading_hub_session_events.json"
#define LONG "tests/fixtures/long_session_events.json"
#define PLAYTHROUGH "tests/fixtures/phase4_playthrough_events.json"

#define CONSTRUCTION_HASH 0x37EED64Du
#define START_HASH 0x8C839F39u
#define RECORDED_FRAMES 19832
#define RECORDED_HASH 0xCFA6611Du
#define HUB_FRAMES 4921
#define HUB_HASH 0x854184AEu
#define LONG_FRAMES 27273
#define LONG_HASH 0xF1990810u
#define PLAYTHROUGH_FRAMES 224
#define PLAYTHROUGH_HASH 0xD7FCAE25u
#define DIRECTED_HASH 0xA5C469BDu
#define DRAW_HASH 0xA740E905u
#define RESTART_HASH 0xB33A149Au
#define KE_VIEW_HASH 0xD0C93314u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static uint32_t fnv_str(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFu);
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

/* The instance reader keeps a field written nil, the struct reader reads a
 * nil member as absent: nil and absent are one state here. */
static int lookup(const void *e, const char *name, pico8_value_t *out) {
    return pico8_entity_get_field(e, name, out) && !(out->kind == PICO8_VALUE_REF && out->as.ref == NULL);
}

static int has(const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v);
}

static pico8_value_t get(const void *e, const char *name) {
    pico8_value_t v = pico8_value_ref(NULL);
    (void)pico8_entity_get_field(e, name, &v);
    return v;
}

static void *get_ref(const void *e, const char *name) {
    pico8_value_t v = get(e, name);
    return v.kind == PICO8_VALUE_REF ? v.as.ref : NULL;
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

/* 0 absent, 1 false, 2 true, 3 other. */
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

static pico8_vec_t vec_of(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return *(const pico8_vec_t *)v.as.ref;
}

static uint32_t fold_vec(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    const pico8_vec_t *p;
    if (!lookup(e, name, &v)) {
        return fnv(h, 0xDEADu);
    }
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    p = v.as.ref;
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

static uint32_t fold_value(uint32_t h, pico8_value_t v) {
    switch (v.kind) {
    case PICO8_VALUE_BOOL:
        return fnv(fnv(h, 0xB0u), (uint32_t)v.as.boolean);
    case PICO8_VALUE_FIXED:
        return fnv(fnv(h, 0xF1u), (uint32_t)v.as.fixed);
    case PICO8_VALUE_STRING:
        return fnv_str(fnv(h, 0x57u), v.as.string);
    case PICO8_VALUE_REF:
        if (v.as.ref == NULL) {
            return fnv(h, 0xDEADu);
        }
        if (listed(pico8_hv(), v.as.ref)) {
            return fnv(fnv(h, 0x4Eu), (uint32_t)rank_in(pico8_hv(), v.as.ref));
        }
        return fnv(h, 0xEEEEu);
    default:
        return fnv(h, 0xF0u);
    }
}

/* An icon is the one entity of `cv.hb` whose `ke` holds a kind, not an
 * entity: there is no `cg.ie`. */
static int is_icon(const void *e) {
    pico8_value_t ke;
    return e != NULL && lookup(e, "ke", &ke) && ke.kind == PICO8_VALUE_REF &&
           pico8_type_tag_of(ke.as.ref) == PICO8_TYPE_TAG_UNKNOWN;
}

static int icons(void *out[], int cap) {
    const pico8_list_t *hb = pico8_cv("hb");
    int n = 0;
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i].as.ref;
        if (is_icon(e)) {
            assert(n < cap);
            out[n++] = e;
        }
    }
    return n;
}

static int icon_rank(const void *e) {
    void *all[64];
    int n = icons(all, 64);
    int i;
    for (i = 0; i < n; i++) {
        if (all[i] == e) {
            return i;
        }
    }
    return -1;
}

static void gx(const pico8_entity_kind_t *out[7]) {
    out[0] = pico8_qh_kind();
    out[1] = pico8_os_kind();
    out[2] = pico8_nr_kind();
    out[3] = pico8_fr_kind();
    out[4] = pico8_nl_kind();
    out[5] = pico8_mx_kind();
    out[6] = pico8_hj_kind();
}

/* `gy` and `id` read off any icon: the cursor and counter every icon
 * shares. */
static uint32_t fold_shared(uint32_t h, const void *icon) {
    h = fold_vec(h, icon, "gy");
    return fold_value(h, get(icon, "id"));
}

static uint32_t fold_icon(uint32_t h, const void *icon) {
    h = fnv(h, bool_state(icon, "kh"));
    h = fold_vec(h, icon, "ra");
    h = fnv(h, bool_state(icon, "m_"));
    return fold_value(h, get(icon, "t"));
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

static void new_game(void) {
    pico8_value_t difficulty;
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty) && difficulty.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty.as.ref));
}

static void start_game(const char *events) {
    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
    pico8__init();
    new_game();
    pico8_cheat_init(&cheat);
}

static void start_from_title(const char *events) {
    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
    pico8__init();
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

static pico8_value_t call(void *e, const char *name, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, 2, arg0, arg1, &out));
    return out;
}

static pico8_value_t vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return pico8_value_ref(v);
}

static pico8_vec_t v2(int x, int y) {
    return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
}

static const pico8_ob_table_t *dr_row(int i) {
    pico8_value_t row;
    assert(pico8_ob_table_get_index(pico8_dr(), i + 1, &row) && row.kind == PICO8_VALUE_REF);
    return row.as.ref;
}

static void *build_icon(const pico8_entity_kind_t *ke, pico8_jb_t *jb) {
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    void *made;

    pico8_ob_table_set_name(overrides, "ke", pico8_value_ref((void *)ke));
    pico8_ob_table_set_name(overrides, "jb", pico8_value_ref(jb));
    made = pico8_ie_new(overrides);
    pico8_ob_table_free(overrides);
    return made;
}

/* The `kq` box by which of a ring of points around its corners it holds. */
static uint32_t fold_kq(uint32_t h, const void *icon) {
    static const int probe[][2] = {{0, 0},  {-1, 0}, {0, -1}, {8, 7},  {9, 8},  {10, 8}, {9, 9},
                                   {-1, -1}, {4, 4},  {8, 0},  {9, 0},  {0, 7},  {0, 8},  {0, 9}};
    const pico8_qz_box_t *kq = get_ref(icon, "kq");
    size_t i;

    assert(kq != NULL);
    for (i = 0; i < sizeof probe / sizeof probe[0]; i++) {
        h = fnv(h, (uint32_t)pico8_qz_call(kq, v2(probe[i][0], probe[i][1])));
    }
    return h;
}

/* A fresh icon: `kq`/`fa`/`ra`/`eh` off the spec, ke's `me`/`t`, the two
 * construction keys, and nothing written by `hb` or the hover loop yet. */
static uint32_t check_fresh_icon(uint32_t h, const void *icon, const void *ke, const void *jb) {
    static const char *const absent[] = {"kh", "m_", "p_", "nu", "fx", "ph", "ev", "hk", "rdown", "gv", "hq", "md"};
    static const char *const methods[] = {"hd", "hb", "kj", "ldown"};
    pico8_value_t v;
    size_t i;

    assert(is_string(icon, "me", "ph"));
    assert(must_fixed(icon, "t") == 0);
    assert(must_fixed(icon, "fa") == pico8_fixed_from_int(12));
    assert(vec_of(icon, "ra").x == pico8_fixed_from_int(999) && vec_of(icon, "ra").y == pico8_fixed_from_int(999));
    assert(vec_of(icon, "eh").x == pico8_fixed_from_int(118) && vec_of(icon, "eh").y == pico8_fixed_from_int(120));
    assert(get_ref(icon, "ke") == ke);
    assert(get_ref(icon, "jb") == jb);
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(icon, absent[i]));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        assert(pico8_entity_get_field(icon, methods[i], &v) && v.kind == PICO8_VALUE_FUNCTION);
    }
    assert(listed(pico8_hv(), icon) && listed(pico8_cv("kj"), icon) && listed(pico8_cv("hd"), icon) &&
           listed(pico8_cv("hb"), icon));
    assert(!listed(pico8_cv("ev"), icon) && !listed(pico8_cv("hk"), icon));

    h = fold_value(h, get(icon, "me"));
    h = fold_value(h, get(icon, "fa"));
    h = fold_kq(h, icon);
    h = fold_vec(h, icon, "ra");
    h = fold_vec(h, icon, "eh");
    return fold_shared(h, icon);
}

static void test_construction(void) {
    uint32_t h = 2166136261u;
    const pico8_entity_kind_t *classes[7];
    void *all[16];
    void *a;
    void *b;
    pico8_jb_t *jb_a;
    pico8_jb_t *jb_b;
    int n;
    int i;

    start_game(PLAYTHROUGH);
    gx(classes);
    n = icons(all, 16);
    check_int("eu icons", n, 7);
    for (i = 0; i < n; i++) {
        pico8_value_t v;
        assert(get_ref(all[i], "ke") == classes[i]);
        assert(get_ref(all[i], "jb") != NULL);
        assert(is_string(all[i], "me", "ph"));
        assert(must_fixed(all[i], "fa") == pico8_fixed_from_int(12));
        assert(!has(all[i], "p_") && !has(all[i], "nu") && !has(all[i], "fx"));
        assert(pico8_entity_get_field(all[i], "hd", &v) && v.kind == PICO8_VALUE_FUNCTION);
        assert(listed(pico8_hv(), all[i]) && listed(pico8_cv("kj"), all[i]) && listed(pico8_cv("hd"), all[i]));
        assert(!listed(pico8_cv("ev"), all[i]) && !listed(pico8_cv("hk"), all[i]));
        h = fold_kq(h, all[i]);
    }

    jb_a = pico8_jb_new(dr_row(0));
    jb_b = pico8_jb_new(dr_row(1));
    a = build_icon(classes[0], jb_a);
    b = build_icon(classes[1], jb_b);
    h = check_fresh_icon(h, a, classes[0], jb_a);
    h = check_fresh_icon(h, b, classes[1], jb_b);
    assert(get_ref(a, "ra") == get_ref(b, "ra"));
    assert(get_ref(a, "kq") == get_ref(b, "kq"));
    assert(get_ref(a, "eh") == get_ref(b, "eh"));
    assert(get_ref(a, "kq") == get_ref(all[0], "kq"));
    assert(icon_rank(a) == 7 && icon_rank(b) == 8);
    check("construction", h, CONSTRUCTION_HASH);
}

/* `_init` points `gy` at `eh` itself, then `eu`'s `fm:lj` reveals every icon
 * whose research is done, right to left from there. */
static void test_start(void) {
    uint32_t h = 2166136261u;
    void *all[16];
    int n;
    int i;
    int first = -1;

    start_game(PLAYTHROUGH);
    n = icons(all, 16);
    for (i = 0; i < n; i++) {
        if (first < 0 && bool_state(all[i], "kh") == 2) {
            first = i;
        }
        h = fold_icon(h, all[i]);
    }
    printf("[start] first revealed icon %d\n", first);
    assert(first >= 0);
    assert(get_ref(all[first], "ra") == get_ref(all[first], "eh"));
    for (i = 0; i < n; i++) {
        if (bool_state(all[i], "kh") != 2) {
            assert(get_ref(all[i], "ra") != get_ref(all[first], "ra"));
        }
    }
    h = fold_shared(h, all[0]);
    check("start", h, START_HASH);
}

/* Per frame: every icon, the shared cursor and counter, the hovered icon,
 * and each placement `ie:ldown` hands to `ok.lheld`. */
static void replay(const char *label, const char *events, int from_title, int want_frames, uint32_t want_hash) {
    uint32_t h = 2166136261u;
    void *last_held = NULL;
    int placed = 0;
    int frames;
    int i;

    if (from_title) {
        start_from_title(events);
    } else {
        start_game(events);
    }
    frames = input.frame_count;
    for (i = 0; i < frames; i++) {
        void *all[64];
        void *held;
        void *hovered;
        int n;
        int j;

        play_frame();
        n = icons(all, 64);
        h = fnv(h, (uint32_t)n);
        for (j = 0; j < n; j++) {
            h = fold_icon(h, all[j]);
        }
        if (n > 0) {
            h = fold_shared(h, all[0]);
        }
        hovered = get_ref(pico8_ok(), "o_");
        h = fnv(h, (uint32_t)icon_rank(hovered));

        held = get_ref(pico8_ok(), "lheld");
        if (held != NULL && held != last_held) {
            pico8_value_t id;
            if (lookup(held, "id", &id) && id.kind == PICO8_VALUE_STRING && id.as.string[0] == 's' &&
                is_icon(hovered)) {
                if (pico8_type_tag_of(held) == PICO8_TYPE_TAG_INSTANCE) {
                    assert(((const pico8_instance_t *)held)->cls == get_ref(hovered, "ke"));
                }
                printf("[%s] frame %d: %s placed from icon %d\n", label, i, id.as.string, icon_rank(hovered));
                h = fnv_str(h, id.as.string);
                placed++;
            }
        }
        last_held = held;
    }
    printf("[%s] %d placements from an icon\n", label, placed);
    check_int(label, frames, want_frames);
    check(label, h, want_hash);
}

static void test_recorded(void) {
    replay("recorded", RECORDED, 1, RECORDED_FRAMES, RECORDED_HASH);
}

static void test_hub(void) {
    replay("hub", HUB, 1, HUB_FRAMES, HUB_HASH);
}

static void test_long(void) {
    replay("long", LONG, 1, LONG_FRAMES, LONG_HASH);
}

static void test_playthrough(void) {
    replay("playthrough", PLAYTHROUGH, 0, PLAYTHROUGH_FRAMES, PLAYTHROUGH_HASH);
}

static void *first_icon_with_kh(uint32_t kh) {
    void *all[16];
    int n = icons(all, 16);
    int i;
    for (i = 0; i < n; i++) {
        if (bool_state(all[i], "kh") == kh) {
            return all[i];
        }
    }
    return NULL;
}

/* `hb` reveals once; `ldown` bumps the one counter every icon shares and
 * builds the held class; `hd` tests `kq` around `ra` in screen space. */
static void test_directed(void) {
    uint32_t h = 2166136261u;
    void *shown;
    void *hidden;
    void *all[16];
    pico8_value_t gy_before;
    pico8_value_t ra_before;
    pico8_value_t made;
    pico8_fixed_t id0;
    pico8_vec_t ra;
    int n;

    start_game(PLAYTHROUGH);
    n = icons(all, 16);
    shown = first_icon_with_kh(2);
    hidden = first_icon_with_kh(0);
    assert(shown != NULL && hidden != NULL);

    gy_before = get(shown, "gy");
    ra_before = get(shown, "ra");
    (void)call(shown, "hb", pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(get_ref(shown, "ra") == ra_before.as.ref && get_ref(shown, "gy") == gy_before.as.ref);
    (void)call(hidden, "hb", pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(bool_state(hidden, "kh") == 0 && get_ref(hidden, "gy") == gy_before.as.ref);
    h = fold_icon(h, shown);
    h = fold_icon(h, hidden);
    h = fold_shared(h, hidden);

    id0 = must_fixed(all[0], "id");
    made = call(all[0], "ldown", pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(made.kind == PICO8_VALUE_REF && made.as.ref != NULL);
    assert(is_string(made.as.ref, "id", "s1"));
    assert(pico8_type_tag_of(made.as.ref) != PICO8_TYPE_TAG_INSTANCE ||
           ((const pico8_instance_t *)made.as.ref)->cls == get_ref(all[0], "ke"));
    assert(must_fixed(all[n - 1], "id") == pico8_fixed_add(id0, pico8_fixed_from_int(1)));
    h = fold_value(h, get(made.as.ref, "id"));
    made = call(all[n - 1], "ldown", pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(is_string(made.as.ref, "id", "s2"));
    assert(pico8_type_tag_of(made.as.ref) != PICO8_TYPE_TAG_INSTANCE ||
           ((const pico8_instance_t *)made.as.ref)->cls == get_ref(all[n - 1], "ke"));
    assert(must_fixed(all[0], "id") == pico8_fixed_add(id0, pico8_fixed_from_int(2)));
    h = fold_value(h, get(made.as.ref, "id"));
    h = fold_shared(h, all[3]);

    ra = vec_of(shown, "ra");
    assert(call(shown, "hd", vec_ref(v2(-500, -500)), vec_ref(pico8_vec_add(ra, v2(1, 1)))).as.boolean);
    assert(!call(shown, "hd", vec_ref(pico8_vec_add(ra, v2(1, 1))), vec_ref(pico8_vec_add(ra, v2(20, 20))))
                .as.boolean);
    assert(!call(hidden, "hd", vec_ref(v2(0, 0)), vec_ref(pico8_vec_add(ra, v2(1, 1)))).as.boolean);
    {
        int dx;
        for (dx = -2; dx <= 11; dx++) {
            pico8_value_t hit = call(shown, "hd", vec_ref(v2(0, 0)), vec_ref(pico8_vec_add(ra, v2(dx, dx - 1))));
            assert(hit.kind == PICO8_VALUE_BOOL);
            h = fnv(h, (uint32_t)hit.as.boolean);
        }
    }
    check("directed", h, DIRECTED_HASH);
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

static uint32_t draw_alone(void *icon) {
    pico8_camera(&gfx, 0, 0);
    pico8_cls(&mem);
    (void)call(icon, "kj", get(icon, "ra"), pico8_value_ref(NULL));
    return screen_hash();
}

/* Hidden: nothing; revealed: the sprite at `ra`; hovered (`m_`): the build
 * cost and the tooltip as well. */
static void test_draw(void) {
    uint32_t h = 2166136261u;
    uint32_t blank;
    uint32_t hidden_px;
    uint32_t shown_px;
    uint32_t hovered_px;
    void *shown;
    void *hidden;

    start_game(PLAYTHROUGH);
    shown = first_icon_with_kh(2);
    hidden = first_icon_with_kh(0);
    pico8_camera(&gfx, 0, 0);
    pico8_cls(&mem);
    blank = screen_hash();

    hidden_px = draw_alone(hidden);
    assert(hidden_px == blank);
    pico8_entity_set_field(hidden, "m_", pico8_value_bool(1));
    assert(bool_state(hidden, "m_") == 2);
    assert(draw_alone(hidden) == blank);
    shown_px = draw_alone(shown);
    assert(shown_px != blank);
    pico8_entity_set_field(shown, "m_", pico8_value_bool(1));
    assert(bool_state(shown, "m_") == 2);
    hovered_px = draw_alone(shown);
    assert(hovered_px != shown_px);
    pico8_entity_set_field(shown, "m_", pico8_value_bool(0));
    assert(draw_alone(shown) == shown_px);

    h = fnv(h, hidden_px);
    h = fnv(h, shown_px);
    h = fnv(h, hovered_px);
    check("draw", h, DRAW_HASH);
}

/* `rr` of each `gx[]` class, in `gx[]` order: nil for `qh` and `os`. */
static const char *const gx_rr[7] = {NULL, NULL, "slipgates", "infraspace", "trade league", "void synthesis",
                                     "ascension"};

static void invent(const char *rr) {
    pico8_value_t v;
    assert(pico8_fm() != NULL && pico8_entity_get_field(pico8_fm(), "invented", &v) &&
           v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    pico8_list_add((pico8_list_t *)v.as.ref, pico8_value_string(rr));
}

/* Per icon, in `gx[]` order: `hb` reveals it exactly when `oj(self.ke.rr)`,
 * and `kj` draws the held class's sprite `s`, then with `m_` its build cost
 * `od$` off `dl(self.ke)`. */
static void test_ke_view(void) {
    uint32_t h = 2166136261u;
    void *all[16];
    int n;
    int i;

    start_game(PLAYTHROUGH);
    n = icons(all, 16);
    check_int("ke view icons", n, 7);
    for (i = 0; i < n; i++) {
        uint32_t before;
        (void)call(all[i], "hb", pico8_value_ref(NULL), pico8_value_ref(NULL));
        before = bool_state(all[i], "kh");
        assert(before == (gx_rr[i] == NULL ? 2u : 0u));
        if (gx_rr[i] != NULL) {
            invent(gx_rr[i]);
            (void)call(all[i], "hb", pico8_value_ref(NULL), pico8_value_ref(NULL));
        }
        assert(bool_state(all[i], "kh") == 2);
        h = fnv(h, before);
        h = fold_icon(h, all[i]);
    }
    h = fold_shared(h, all[0]);
    {
        /* `dl` prints at `je.ra`, (0,0) here: its label would land off
         * screen. */
        pico8_value_t mouse;
        assert(pico8_entity_get_field(pico8_je(), "ra", &mouse) && mouse.kind == PICO8_VALUE_REF &&
               mouse.as.ref != NULL);
        *(pico8_vec_t *)mouse.as.ref = v2(64, 64);
    }
    for (i = 0; i < n; i++) {
        uint32_t plain = draw_alone(all[i]);
        uint32_t hovered;
        pico8_entity_set_field(all[i], "m_", pico8_value_bool(1));
        hovered = draw_alone(all[i]);
        pico8_entity_set_field(all[i], "m_", pico8_value_bool(0));
        printf("[ke view] icon %d: 0x%08Xu / 0x%08Xu\n", i, (unsigned)plain, (unsigned)hovered);
        assert(hovered != plain);
        h = fnv(h, plain);
        h = fnv(h, hovered);
    }
    check("ke view", h, KE_VIEW_HASH);
}

/* A second `eu` keeps `gy` and `id` where the last game left them; only
 * `_init` puts `gy` back on `eh`, and nothing resets `id`. */
static void test_restart(void) {
    uint32_t h = 2166136261u;
    void *all[16];
    pico8_value_t gy;
    pico8_fixed_t id;
    int n;
    int i;

    start_game(PLAYTHROUGH);
    n = icons(all, 16);
    (void)call(all[0], "ldown", pico8_value_ref(NULL), pico8_value_ref(NULL));
    gy = get(all[0], "gy");
    id = must_fixed(all[0], "id");
    assert(id == pico8_fixed_from_int(1));
    assert(gy.as.ref != get_ref(all[0], "eh"));

    new_game();
    n = icons(all, 16);
    check_int("restart icons", n, 7);
    assert(must_fixed(all[0], "id") == id);
    assert(get_ref(all[0], "gy") != gy.as.ref);
    for (i = 0; i < n; i++) {
        if (bool_state(all[i], "kh") == 2) {
            assert(get_ref(all[i], "ra") != get_ref(all[i], "eh"));
        }
        h = fold_icon(h, all[i]);
    }
    h = fold_shared(h, all[0]);

    pico8__init();
    assert(get_ref(all[0], "gy") == get_ref(all[0], "eh"));
    assert(must_fixed(all[0], "id") == id);
    h = fold_shared(h, all[0]);
    check("restart", h, RESTART_HASH);
}

static int is_an_ie_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_ie_struct_class();
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

static void write_kh(void *e) {
    pico8_entity_set_field(e, "kh", pico8_value_bool(1));
}

static void write_ra(void *e) {
    pico8_entity_set_field(e, "ra", pico8_value_ref(NULL));
}

static void write_gy(void *e) {
    pico8_entity_set_field(e, "gy", pico8_value_ref(NULL));
}

static void write_id(void *e) {
    pico8_entity_set_field(e, "id", pico8_value_fixed(0));
}

static void build_with_ra(void *ke) {
    pico8_ob_table_t *overrides = pico8_ob_table_new();

    pico8_ob_table_set_name(overrides, "ke", pico8_value_ref(ke));
    pico8_ob_table_set_name(overrides, "jb", pico8_value_ref(pico8_jb_new(NULL)));
    pico8_ob_table_set_name(overrides, "ra", pico8_value_ref(NULL));
    (void)pico8_ie_new(overrides);
}

/* Built by `eu` or directly, an `ie` is a struct with its four methods; a
 * generic write of a name other than `m_`, or a construction key other
 * than `ke`/`jb`, stops the run instead of growing a member. `hd` stays the
 * plain `1` of `fq` and `bg`, and no other struct answers to `hb`. */
static void test_ie_is_a_struct(void) {
    static const char *const methods[] = {"hd", "hb", "kj", "ldown"};
    static const char *const not_methods[] = {"ph", "ev", "hk", "rdown", "gv", "eb"};
    const pico8_entity_kind_t *classes[7];
    void *all[16];
    void *built;
    void *others[4];
    int n;
    int i;

    start_game(PLAYTHROUGH);
    gx(classes);
    n = icons(all, 16);
    built = build_icon(classes[2], pico8_jb_new(dr_row(2)));
    all[n++] = built;
    for (i = 0; i < n; i++) {
        size_t k;
        assert(is_an_ie_struct(all[i]));
        for (k = 0; k < sizeof methods / sizeof methods[0]; k++) {
            pico8_value_t m;
            assert(pico8_entity_get_field(all[i], methods[k], &m) && m.kind == PICO8_VALUE_FUNCTION);
        }
        for (k = 0; k < sizeof not_methods / sizeof not_methods[0]; k++) {
            pico8_value_t out;
            assert(!pico8_mv(all[i], not_methods[k], 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
        }
    }

    pico8_entity_set_field(built, "m_", pico8_value_bool(1));
    assert(bool_state(built, "m_") == 2);
    {
        pico8_ob_table_t *jb_only = pico8_ob_table_new();
        void *no_ke;
        pico8_fixed_t id0 = must_fixed(built, "id");

        pico8_ob_table_set_name(jb_only, "jb", pico8_value_ref(pico8_jb_new(NULL)));
        no_ke = pico8_ie_new(jb_only);
        pico8_ob_table_free(jb_only);
        assert(is_an_ie_struct(no_ke) && !has(no_ke, "ke"));
        assert(must_fixed(no_ke, "id") == id0);
        (void)call(no_ke, "ldown", pico8_value_ref(NULL), pico8_value_ref(NULL));
        assert(must_fixed(built, "id") == pico8_fixed_add(id0, pico8_fixed_from_int(1)));
    }
    must_abort(write_kh, built);
    must_abort(write_ra, built);
    must_abort(write_gy, built);
    must_abort(write_id, built);
    must_abort(build_with_ra, (void *)classes[0]);

    others[0] = pico8_bg_new(NULL);
    others[1] = pico8_fq_new(NULL);
    others[2] = pico8_jc_new(NULL);
    others[3] = pico8_mz_new(NULL);
    for (i = 0; i < 2; i++) {
        pico8_value_t hd;
        assert(pico8_entity_get_field(others[i], "hd", &hd) && hd.kind == PICO8_VALUE_FIXED &&
               hd.as.fixed == pico8_fixed_from_int(1));
    }
    for (i = 0; i < 4; i++) {
        pico8_value_t out;
        assert(!pico8_mv(others[i], "hb", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    }
}

static void write_od(void *e) {
    pico8_entity_set_field(e, "od", pico8_value_fixed(0));
}

/* `self.ke` read through a struct without parent that carries only the
 * `rr`/`s`/`me`/`od` of the held class's chain, and no writer. */
static void test_ke_view_is_a_struct(void) {
    static const int s[7] = {174, 100, 170, 78, 68, 66, 166};
    static const int od[7] = {15, 15, 10, 25, 20, 40, 60};
    static const char *const unknown[] = {"et", "ke", "nu", "hq"};
    const pico8_entity_kind_t *classes[7];
    void *all[16];
    pico8_ie_ke_t view;
    int n;
    int i;

    start_game(PLAYTHROUGH);
    gx(classes);
    n = icons(all, 16);
    check_int("ke view struct icons", n, 7);
    for (i = 0; i < n; i++) {
        pico8_value_t v;
        size_t k;

        assert(get_ref(all[i], "ke") == classes[i]);
        view = pico8_ie_ke_view(all[i]);
        assert(pico8_type_tag_of(&view) == PICO8_TYPE_TAG_STRUCT &&
               view.header.cls == pico8_ie_ke_view_struct_class());
        if (gx_rr[i] == NULL) {
            assert(!has(&view, "rr"));
        } else {
            assert(is_string(&view, "rr", gx_rr[i]));
        }
        assert(must_fixed(&view, "s") == pico8_fixed_from_int(s[i]));
        assert(must_fixed(&view, "od") == pico8_fixed_from_int(od[i]));
        assert(is_string(&view, "me", "kz"));
        for (k = 0; k < sizeof unknown / sizeof unknown[0]; k++) {
            assert(!pico8_entity_get_field(&view, unknown[k], &v));
        }
    }
    must_abort(write_od, &view);

    {
        pico8_ob_table_t *jb_only = pico8_ob_table_new();
        void *no_ke;
        static const char *const members[] = {"rr", "s", "me", "od"};
        size_t k;

        pico8_ob_table_set_name(jb_only, "jb", pico8_value_ref(pico8_jb_new(NULL)));
        no_ke = pico8_ie_new(jb_only);
        pico8_ob_table_free(jb_only);
        view = pico8_ie_ke_view(no_ke);
        assert(pico8_type_tag_of(&view) == PICO8_TYPE_TAG_STRUCT);
        for (k = 0; k < sizeof members / sizeof members[0]; k++) {
            assert(!has(&view, members[k]));
        }
    }
}

/* A replay depends on the rnd state: run each scenario in a child forked
 * right after pico8_cart_load(), as a fresh process would start it. */
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
    in_fresh_child(test_start);
    in_fresh_child(test_recorded);
    in_fresh_child(test_hub);
    in_fresh_child(test_long);
    in_fresh_child(test_playthrough);
    in_fresh_child(test_directed);
    in_fresh_child(test_draw);
    in_fresh_child(test_restart);
    in_fresh_child(test_ke_view);
    in_fresh_child(test_ke_view_is_a_struct);
    in_fresh_child(test_ie_is_a_struct);

    printf("test_pico8_ie: all tests passed\n");
    return 0;
}
