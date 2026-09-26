/* The planet `jo`, read and written only through the dual-form field API
 * and `pico8_class_get_field(pico8_jo_class(), ...)`, and driven through the
 * real `bw`/`dg`/`dc`/`mv`. Its daughters are read the same way.
 * The frozen values were taken on the code where `jo` was still a class
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
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_research.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define BUILT "tests/fixtures/built_planet_panel_events.json"
#define EMPTY "tests/fixtures/empty_planet_cleared_events.json"
#define DRAG "tests/fixtures/slipway_drag_events.json"
#define PLAYTHROUGH "tests/fixtures/phase4_playthrough_events.json"
#define SESSION "tests/fixtures/session_events.json"
#define RECORDED "tests/fixtures/events_recorded_1790198294.json"
#define HUB "tests/fixtures/trading_hub_session_events.json"
#define LONG "tests/fixtures/long_session_events.json"

#define GALAXY_PLANETS 90
#define GALAXY_HASH 0xE70E2EE3u
#define BUILT_FRAMES 278
#define BUILT_HASH 0x6B570BB7u
#define EMPTY_FRAMES 229
#define EMPTY_HASH 0xFC103987u
#define DRAG_FRAMES 226
#define DRAG_HASH 0xFE21B43Bu
#define PLAYTHROUGH_FRAMES 224
#define PLAYTHROUGH_HASH 0x381052BFu
#define SESSION_FRAMES 116045
#define SESSION_HASH 0xD61190F7u
#define RECORDED_FRAMES 19832
#define RECORDED_HASH 0xF078D26Au
#define HUB_FRAMES 4921
#define HUB_HASH 0xAC712029u
#define LONG_FRAMES 27273
#define LONG_HASH 0x61ADB81Cu
#define NEW_PLANET_HASH 0x48C32418u
#define LL_DG_CALLS 62

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

/* The 31 members a `jo` struct has after its pico8_ke_t. */
static const char *const members[] = {"nu", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq",
                                      "ca", "em", "id", "p",  "ra", "oe", "hn", "ep", "jy", "nk", "fx",
                                      "lq", "fo", "jh", "p_", "kr", "cc", "by", "mi", "m_"};
static const char *const methods[] = {"hd", "kj", "ldown", "hb", "gg", "gv", "hk", "kf", "cn", "ib", "ll"};
static const char *const absent[] = {"ev", "ph", "ly", "rdown", "hq", "ey", "md"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"jo", "np", "prober"};
static const char *const class_refs[] = {"nu", "kq", "kw", "gq", "em"};

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

static void *class_ref(const char *name) {
    pico8_value_t v;
    assert(pico8_class_get_field(pico8_jo_class(), name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
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
    case PICO8_VALUE_FUNCTION:
        return fnv(h, 0xFCu);
    case PICO8_VALUE_REF:
        if (v.as.ref == NULL) {
            return fnv(h, 0xDEADu);
        }
        if (listed(pico8_hv(), v.as.ref)) {
            return fnv(fnv(h, 0x4Eu), (uint32_t)rank_in(pico8_hv(), v.as.ref));
        }
        return fnv(fnv(h, 0xEEEEu), (uint32_t)pico8_type_tag_of(v.as.ref));
    default:
        return fnv(h, 0xF0u);
    }
}

static uint32_t fold_field(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v) ? fold_value(fnv(h, 1u), v) : fnv(h, 0u);
}

/* The item count of a list field, -1 when absent. */
static int list_size(const void *e, const char *name) {
    pico8_value_t v;
    if (!lookup(e, name, &v)) {
        return -1;
    }
    assert(v.kind == PICO8_VALUE_REF);
    return ((const pico8_list_t *)v.as.ref)->count;
}

static uint32_t fold_kw(uint32_t h, const void *e) {
    pico8_value_t kw;
    pico8_value_t v;
    if (!lookup(e, "kw", &kw)) {
        return fnv(h, 0xA0u);
    }
    assert(kw.kind == PICO8_VALUE_REF && pico8_type_tag_of(kw.as.ref) == PICO8_TYPE_TAG_OB_TABLE);
    h = pico8_ob_table_get_name(kw.as.ref, "qe", &v) ? fold_value(fnv(h, 1u), v) : fnv(h, 0u);
    h = pico8_ob_table_get_name(kw.as.ref, "mu", &v) ? fold_value(fnv(h, 1u), v) : fnv(h, 0u);
    return fnv(h, (uint32_t)(kw.as.ref == class_ref("kw")));
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

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)pico8_peek(&mem, PICO8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
}

static const pico8_ob_table_t *bo_row(int i) {
    pico8_value_t row;
    assert(pico8_ob_table_get_index(pico8_bo(), i, &row) && row.kind == PICO8_VALUE_REF);
    return row.as.ref;
}

static void start_from_title(const char *events) {
    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_cheat_init(&cheat);
}

static void start_game(const char *events) {
    start_from_title(events);
    pico8_eu(pico8_js_new(bo_row(2)));
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

static pico8_value_t vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return pico8_value_ref(v);
}

static pico8_value_t call(void *e, const char *name, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, argc, arg0, arg1, &out));
    return out;
}

/* A bare planet as `jo(...)` and `jo:pq` left it: the scalars, which of
 * the 31 member names it has, its methods, which default tables it shares
 * with the class, and where `lt` registered it. */
static uint32_t fold_planet(uint32_t h, const void *e) {
    static const char *const scalars[] = {"me", "t", "id", "hf", "hs", "fa", "ma", "ca"};
    size_t i;

    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fold_vec(h, e, "p");
    h = fold_vec(h, e, "ra");
    h = fold_vec(h, e, "gq");
    for (i = 0; i < sizeof members / sizeof members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, members[i]));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        pico8_value_t v;
        assert(pico8_entity_get_field(e, methods[i], &v) && v.kind == PICO8_VALUE_FUNCTION);
        assert(v.as.function == pico8_jo_struct_class()->methods[pico8_method_of(methods[i])]);
    }
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(e, absent[i]));
    }
    for (i = 0; i < sizeof class_refs / sizeof class_refs[0]; i++) {
        assert(get_ref(e, class_refs[i]) == class_ref(class_refs[i]));
    }
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cv(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cg(tags[i]), e));
    }
    return h;
}

/* After `eu`: the 90 planets `bw` built. */
static void test_galaxy(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *jo;
    int i;

    start_game(BUILT);
    jo = pico8_cg("jo");
    for (i = 0; i < jo->count; i++) {
        h = fold_planet(h, jo->items[i].as.ref);
    }
    check_int("galaxy planets", jo->count, GALAXY_PLANETS);
    check("galaxy", h, GALAXY_HASH);
}

static uint32_t fold_live(uint32_t h, const void *e) {
    static const char *const scalars[] = {"me", "t", "ma", "fx", "mi", "kr", "ep"};
    static const char *const lists[] = {"oe", "jy", "by", "cc"};
    static const char *const refs[] = {"fo", "nk", "lq", "m_"};
    size_t i;

    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fnv(h, bool_state(e, "p_"));
    h = fold_kw(h, e);
    for (i = 0; i < sizeof lists / sizeof lists[0]; i++) {
        h = fnv(h, (uint32_t)list_size(e, lists[i]));
    }
    for (i = 0; i < sizeof refs / sizeof refs[0]; i++) {
        h = fnv(h, (uint32_t)has(e, refs[i]));
    }
    return h;
}

#define MAX_STATES 16

/* Per frame: every entry of `cg.jo` and the screen; prints the `me` states
 * the planets went through. */
static void replay(const char *label, const char *events, int want_frames, uint32_t want_hash) {
    uint32_t h = 2166136261u;
    const char *states[MAX_STATES];
    int state_count = 0;
    int frames;
    int i;

    start_game(events);
    frames = input.frame_count;
    for (i = 0; i < frames; i++) {
        const pico8_list_t *jo;
        int j;

        play_frame();
        jo = pico8_cg("jo");
        h = fnv(h, (uint32_t)(jo == NULL ? 0 : jo->count));
        for (j = 0; jo != NULL && j < jo->count; j++) {
            pico8_value_t me;
            h = fold_live(h, jo->items[j].as.ref);
            if (lookup(jo->items[j].as.ref, "me", &me) && me.kind == PICO8_VALUE_STRING) {
                int k;
                for (k = 0; k < state_count && strcmp(states[k], me.as.string) != 0; k++) {
                }
                if (k == state_count && state_count < MAX_STATES) {
                    states[state_count++] = me.as.string;
                }
            }
        }
        h = fnv(h, screen_hash());
    }
    printf("[%s] states:", label);
    for (i = 0; i < state_count; i++) {
        printf(" %s", states[i]);
    }
    printf("\n");
    check_int(label, frames, want_frames);
    check(label, h, want_hash);
}

static void test_replay_built(void) {
    replay("built replay", BUILT, BUILT_FRAMES, BUILT_HASH);
}

static void test_replay_empty(void) {
    replay("empty replay", EMPTY, EMPTY_FRAMES, EMPTY_HASH);
}

static void test_replay_drag(void) {
    replay("drag replay", DRAG, DRAG_FRAMES, DRAG_HASH);
}

static void test_replay_playthrough(void) {
    replay("playthrough replay", PLAYTHROUGH, PLAYTHROUGH_FRAMES, PLAYTHROUGH_HASH);
}

enum { QH, NL, OS, HJ, PM, MX, NR, FR, GA, DAUGHTERS };

static const char *const daughter_names[DAUGHTERS] = {"qh", "nl", "os", "hj", "pm", "mx", "nr", "fr", "ga"};

static int daughter_of(const void *e) {
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_qh_struct_class()) {
        return QH;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_nl_struct_class()) {
        return NL;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_os_struct_class()) {
        return OS;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_hj_struct_class()) {
        return HJ;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_pm_struct_class()) {
        return PM;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_mx_struct_class()) {
        return MX;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_nr_struct_class()) {
        return NR;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_fr_struct_class()) {
        return FR;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_ga_struct_class()) {
        return GA;
    }
    return -1;
}

/* What a daughter inherits from `jo`: `ca`, `gq`, the `em` table itself,
 * and `jo`'s `gg` and `hd` called on it. */
static uint32_t fold_daughter(uint32_t h, void *d) {
    pico8_vec_t ra;
    pico8_value_t ra_val;

    h = fold_field(h, d, "ca");
    h = fold_vec(h, d, "gq");
    assert(get_ref(d, "em") == class_ref("em"));
    h = fold_value(h, call(d, "gg", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    assert(lookup(d, "ra", &ra_val) && ra_val.kind == PICO8_VALUE_REF);
    ra = *(const pico8_vec_t *)ra_val.as.ref;
    h = fold_value(h, call(d, "hd", 2, vec_ref(ra), vec_ref(ra)));
    ra = pico8_vec_add(ra, pico8_vec_v(pico8_fixed_from_int(8), pico8_fixed_from_int(8)));
    return fold_value(h, call(d, "hd", 2, vec_ref(ra), vec_ref(ra)));
}

#define MAX_SEEN 256

/* From the title screen, as a raylib recording starts. The first instance
 * of each daughter is read at the frame it first appears in `hv`, then at
 * the end of the replay; `required` lists the daughters it must meet. */
static void replay_daughters(const char *label, const char *events, int want_frames, uint32_t want_hash,
                             const int *required, int required_count) {
    uint32_t h = 2166136261u;
    void *first[DAUGHTERS] = {NULL};
    void *seen[MAX_SEEN];
    int seen_count = 0;
    int counts[DAUGHTERS] = {0};
    int frames;
    int i;

    start_from_title(events);
    frames = input.frame_count;
    for (i = 0; i < frames; i++) {
        const pico8_list_t *hv;
        int j;

        play_frame();
        hv = pico8_hv();
        for (j = 0; j < hv->count; j++) {
            void *e = hv->items[j].as.ref;
            int d = daughter_of(e);
            int k;

            if (d < 0) {
                continue;
            }
            for (k = 0; k < seen_count && seen[k] != e; k++) {
            }
            if (k == seen_count) {
                assert(seen_count < MAX_SEEN);
                seen[seen_count++] = e;
                counts[d]++;
            }
            if (first[d] == NULL) {
                first[d] = e;
                printf("[%s] first %s at frame %d\n", label, daughter_names[d], i);
                h = fnv(fnv(h, (uint32_t)d), (uint32_t)i);
                h = fold_daughter(h, e);
            }
        }
    }
    for (i = 0; i < DAUGHTERS; i++) {
        printf("[%s] %s: %d\n", label, daughter_names[i], counts[i]);
        h = fnv(h, (uint32_t)counts[i]);
        if (first[i] != NULL) {
            h = fold_daughter(fnv(h, (uint32_t)i), first[i]);
        }
    }
    for (i = 0; i < required_count; i++) {
        assert(first[required[i]] != NULL);
    }
    check_int(label, frames, want_frames);
    check(label, h, want_hash);
}

static void test_session(void) {
    static const int required[] = {QH, NR, FR, PM, GA};
    replay_daughters("session", SESSION, SESSION_FRAMES, SESSION_HASH, required, 5);
}

static void test_recorded(void) {
    static const int required[] = {HJ, MX};
    replay_daughters("recorded", RECORDED, RECORDED_FRAMES, RECORDED_HASH, required, 2);
}

static void test_hub(void) {
    static const int required[] = {NL};
    replay_daughters("hub", HUB, HUB_FRAMES, HUB_HASH, required, 1);
}

static void test_long(void) {
    static const int required[] = {OS};
    replay_daughters("long", LONG, LONG_FRAMES, LONG_HASH, required, 1);
}

static void *new_planet(const char *id, int x, int y) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    void *made;

    pico8_ob_table_set_name(b, "id", pico8_value_string(id));
    pico8_ob_table_set_name(b, "p", vec_ref(pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y))));
    pico8_ob_table_set_name(b, "ra",
                            vec_ref(pico8_vec_v(pico8_fixed_from_int(x * 128), pico8_fixed_from_int(y * 128))));
    made = pico8_jo_new(b);
    pico8_ob_table_free(b);
    return made;
}

static void test_new_planet(void) {
    uint32_t h = 2166136261u;
    void *a;

    start_from_title(BUILT);
    a = new_planet("p900", 1, 2);
    assert(listed(pico8_hv(), a) && listed(pico8_cg("jo"), a));
    h = fold_planet(h, a);
    h = fold_live(h, a);
    check("new planet", h, NEW_PLANET_HASH);
}

/* `dg` calls `qc[qc.me]`: a planet whose `me` is "ll" runs `jo:ll` until
 * its `t` passes 60, then leaves `hv`. */
static void test_ll(void) {
    void *a;
    int calls = 0;

    start_from_title(BUILT);
    a = new_planet("p901", 3, 4);
    pico8_dg();
    assert(bool_state(a, "p_") == 0);
    pico8_ke_ks(a, pico8_value_string("ll"));
    while (listed(pico8_hv(), a) && calls < 200) {
        pico8_dg();
        calls++;
    }
    assert(bool_state(a, "p_") == 2);
    assert(!listed(pico8_hv(), a) && !listed(pico8_cg("jo"), a));
    check_int("ll dg calls", calls, LL_DG_CALLS);
}

/* `kw` of a planet that never ran `lz` is the class's own table: an
 * in-place write to it is seen by every such planet and by the class. */
static void test_kw_shared(void) {
    void *a;
    void *b;
    pico8_ob_table_t *kw;
    pico8_value_t v;
    pico8_value_t hp;

    start_from_title(BUILT);
    a = new_planet("p902", 5, 6);
    b = new_planet("p903", 7, 8);
    kw = get_ref(a, "kw");
    assert(kw == class_ref("kw") && get_ref(b, "kw") == kw);
    pico8_ob_table_set_name(kw, "mu", pico8_value_fixed(pico8_fixed_from_int(7)));
    assert(pico8_ob_table_get_name(get_ref(b, "kw"), "mu", &v) && v.kind == PICO8_VALUE_FIXED &&
           v.as.fixed == pico8_fixed_from_int(7));
    assert(pico8_ob_table_get_name(class_ref("kw"), "mu", &v) && v.as.fixed == pico8_fixed_from_int(7));

    assert(pico8_gb() != NULL && pico8_gb()->count > 0);
    hp = pico8_gb()->entries[0].value;
    assert(hp.kind == PICO8_VALUE_REF && hp.as.ref != NULL);
    pico8_jo_lz(a, hp.as.ref);
    assert(get_ref(a, "kw") != NULL && get_ref(a, "kw") != class_ref("kw"));
    assert(pico8_ob_table_get_name(get_ref(a, "kw"), "mu", &v) && v.as.fixed == pico8_fixed_from_int(0));
    assert(get_ref(b, "kw") == class_ref("kw"));
    assert(get_ref(a, "hn") == hp.as.ref && list_size(a, "jy") >= 0);
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

static void write_ra(void *e) {
    pico8_entity_set_field(e, "ra", pico8_value_ref(NULL));
}

static void write_id(void *e) {
    pico8_entity_set_field(e, "id", pico8_value_string("p0"));
}

static void write_hp(void *e) {
    pico8_entity_set_field(e, "hp", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void build_with_unknown_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "hp", pico8_value_fixed(pico8_fixed_from_int(1)));
    (void)pico8_jo_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_jo_new(b);
}

static int is_a_jo_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_jo_struct_class();
}

/* Built by `bw` or pico8_jo_new, a `jo` is a struct whose descriptor holds
 * jo's 11 dispatchers, and its class installs none of them, nor a `pq`; a
 * generic write outside the 17 measured names, or a construction key other
 * than `id`/`p`/`ra`, stops the run. */
static void test_jo_is_a_struct(void) {
    static const char *const written[] = {"by", "cc", "ep", "fo", "fx", "hn", "jh", "jy", "kr",
                                          "kw", "lq", "m_", "ma", "mi", "nk", "oe", "p_"};
    const pico8_list_t *jo;
    void *a;
    size_t i;
    int j;

    start_game(EMPTY);
    jo = pico8_cg("jo");
    for (j = 0; j < jo->count; j++) {
        assert(is_a_jo_struct(jo->items[j].as.ref));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        pico8_value_t v;
        assert(pico8_jo_struct_class()->methods[pico8_method_of(methods[i])] != NULL);
        assert(!pico8_ob_table_get_name(pico8_jo_class()->defaults, methods[i], &v));
    }
    assert(pico8_jo_class()->pq == NULL);
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        if (pico8_jo_struct_class()->methods[j] != NULL) {
            int k;
            for (k = 0; k < (int)(sizeof methods / sizeof methods[0]) && pico8_method_of(methods[k]) != j; k++) {
            }
            assert(k < (int)(sizeof methods / sizeof methods[0]));
        }
    }

    assert(pico8_jo_dz() == class_ref("dz") && pico8_jo_cq() == class_ref("cq") && pico8_jo_ka() == class_ref("ka") &&
           pico8_jo_em() == class_ref("em"));

    a = new_planet("p904", 9, 10);
    assert(is_a_jo_struct(a));
    for (i = 0; i < sizeof written / sizeof written[0]; i++) {
        pico8_value_t v;
        pico8_entity_set_field(a, written[i], pico8_value_fixed(pico8_fixed_from_int((int)i + 1)));
        assert(lookup(a, written[i], &v) && v.kind == PICO8_VALUE_FIXED &&
               v.as.fixed == pico8_fixed_from_int((int)i + 1));
    }
    must_abort(write_ra, a);
    must_abort(write_id, a);
    must_abort(write_hp, a);
    must_abort(build_with_unknown_key, NULL);
    must_abort(build_with_positional_key, NULL);
    {
        void *bare = pico8_jo_new(NULL);
        assert(is_a_jo_struct(bare) && !has(bare, "id") && !has(bare, "ra") && has(bare, "oe"));
    }

    for (j = 0; j < input.frame_count; j++) {
        play_frame();
    }
    jo = pico8_cg("qh");
    assert(jo != NULL && jo->count == 1);
    assert(pico8_type_tag_of(jo->items[0].as.ref) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)jo->items[0].as.ref)->cls == pico8_qh_struct_class());
}

/* A replay depends on the rnd state: run each scenario in a child forked
 * right after pico8_cart_load(), as a fresh process would start it. */
static pid_t spawn(void (*test)(void)) {
    pid_t child;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        test();
        fflush(stdout);
        _exit(0);
    }
    return child;
}

static void reap(pid_t child) {
    int status;
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static void in_fresh_child(void (*test)(void)) {
    reap(spawn(test));
}

int main(void) {
    pid_t recordings[4];
    int i;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    /* The four recordings take minutes each: they run side by side. */
    recordings[0] = spawn(test_session);
    recordings[1] = spawn(test_recorded);
    recordings[2] = spawn(test_hub);
    recordings[3] = spawn(test_long);
    in_fresh_child(test_galaxy);
    in_fresh_child(test_replay_built);
    in_fresh_child(test_replay_empty);
    in_fresh_child(test_replay_drag);
    in_fresh_child(test_replay_playthrough);
    in_fresh_child(test_new_planet);
    in_fresh_child(test_ll);
    in_fresh_child(test_kw_shared);
    in_fresh_child(test_jo_is_a_struct);
    for (i = 0; i < 4; i++) {
        reap(recordings[i]);
    }

    printf("test_pico8_jo: all tests passed\n");
    return 0;
}
