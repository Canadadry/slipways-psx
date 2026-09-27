/* The click veil `fq`, built bare (`pj:ph`) and with a `my` (`ix[3]` with
 * `de`), lived through the real `dg`, `mv("ldown")`, the hover loop of `ok`
 * and the two replays that raise it. The frozen values were taken on the
 * code where `fq` was still a class instance and must not be re-taken. */

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
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_score_panel_display.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"

#define MY_TRUE_TICKS 6
#define MY_TRUE_SEQUENCE_HASH 0xA6A43C27u
#define BARE_SEQUENCE_HASH 0x52BE382Au
#define MY_FALSE_SEQUENCE_HASH 0xE43E9427u
#define HOVER_SEQUENCE_HASH 0x27C4BCB8u
#define HOVER_CURSOR_HASH 0x4A878EEAu
#define SCORE_PANEL_CLOSE_FRAMES 40
#define SCORE_PANEL_CLOSE_HASH 0xD27CD6EBu
#define GAME_END_FRAMES 74
#define GAME_END_HASH 0xE0F8CA4Du

#define LDOWN_TICK 5
#define IDLE_TICKS 30

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

static uint32_t fold_field(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;

    if (!pico8_entity_get_field(e, name, &v)) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)v.kind);
    if (v.kind == PICO8_VALUE_FIXED) {
        return fnv(h, (uint32_t)v.as.fixed);
    }
    if (v.kind == PICO8_VALUE_BOOL) {
        return fnv(h, (uint32_t)v.as.boolean);
    }
    return h;
}

static uint32_t fold_veil(uint32_t h, const void *e) {
    h = fold_field(h, e, "t");
    h = fold_field(h, e, "p_");
    h = fold_field(h, e, "m_");
    h = fnv(h, (uint32_t)listed(pico8_hv(), e));
    return fnv(h, (uint32_t)listed(pico8_cv("hd"), e));
}

static const void *ok_o_(void) {
    pico8_value_t v;

    if (!pico8_entity_get_field(pico8_ok(), "o_", &v) || v.kind != PICO8_VALUE_REF) {
        return NULL;
    }
    return v.as.ref;
}

static int g_my_calls = 0;
static int g_my_answer = 1;

static pico8_value_t counting_my(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    g_my_calls++;
    return pico8_value_bool(g_my_answer);
}

static void *veil_with_my(int answer) {
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    void *veil;

    g_my_calls = 0;
    g_my_answer = answer;
    pico8_ob_table_set_name(overrides, "my", pico8_value_function(counting_my));
    veil = pico8_fq_new(overrides);
    pico8_ob_table_free(overrides);
    return veil;
}

static int ldown(void *veil) {
    pico8_value_t out;

    assert(pico8_mv(veil, "ldown", 2, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    return out.kind == PICO8_VALUE_BOOL && out.as.boolean;
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

/* `fq(de)`: the click at tick LDOWN_TICK stores `my()` in `p_`, and the
 * next `dg` takes the veil out. */
static void test_ldown_with_a_true_my_removes_the_veil(void) {
    uint32_t h = 2166136261u;
    void *veil;
    int ticks = 0;

    pico8_jl();
    veil = veil_with_my(1);
    h = fold_veil(h, veil);
    while (listed(pico8_hv(), veil)) {
        if (ticks == LDOWN_TICK) {
            assert(ldown(veil));
            assert(g_my_calls == 1);
            h = fold_veil(h, veil);
        }
        pico8_dg();
        ticks++;
        h = fold_veil(h, veil);
        assert(ticks < 1000);
    }
    assert(g_my_calls == 1);
    assert(!listed(pico8_cv("hd"), veil));
    printf("[my true] ticks=%d\n", ticks);
    fflush(stdout);
    assert(ticks == MY_TRUE_TICKS);
    check("my true", h, MY_TRUE_SEQUENCE_HASH);
}

/* `fq()`: a click is swallowed, `p_` stays nil, the veil stays. */
static void test_bare_veil_swallows_the_click_and_stays(void) {
    uint32_t h = 2166136261u;
    void *veil;
    pico8_value_t p_;
    int i;

    pico8_jl();
    veil = pico8_fq_new(NULL);
    for (i = 0; i < IDLE_TICKS; i++) {
        if (i == LDOWN_TICK) {
            assert(ldown(veil));
            assert(!pico8_entity_get_field(veil, "p_", &p_));
        }
        pico8_dg();
        h = fold_veil(h, veil);
    }
    assert(listed(pico8_hv(), veil) && listed(pico8_cv("hd"), veil));
    check("bare", h, BARE_SEQUENCE_HASH);
}

/* A `my` answering false leaves `p_` false: the veil stays. */
static void test_false_my_keeps_the_veil(void) {
    uint32_t h = 2166136261u;
    void *veil;
    pico8_value_t p_;
    int i;

    pico8_jl();
    veil = veil_with_my(0);
    for (i = 0; i < IDLE_TICKS; i++) {
        if (i == LDOWN_TICK) {
            assert(ldown(veil));
            assert(g_my_calls == 1);
            assert(pico8_entity_get_field(veil, "p_", &p_) && p_.kind == PICO8_VALUE_BOOL && !p_.as.boolean);
        }
        pico8_dg();
        h = fold_veil(h, veil);
    }
    assert(listed(pico8_hv(), veil) && listed(pico8_cv("hd"), veil));
    check("my false", h, MY_FALSE_SEQUENCE_HASH);
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

/* A started game, then `fq()`: `ok`'s hover loop makes it `o_` (hd=1 is
 * always a hit) in layer 14, marks it `m_`, takes `ldown` but not `rdown`,
 * and, once the veil is gone, unmarks it through `gj`. */
static void test_hover_loop_on_the_veil(void) {
    uint32_t h = 2166136261u;
    pico8_value_t difficulty;
    pico8_value_t kn;
    pico8_value_t m_;
    pico8_struct_header_t *kv = NULL;
    void *veil;

    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty) && difficulty.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty.as.ref));
    veil = pico8_fq_new(NULL);

    pico8_ui_lj(&mem, pico8_ok());
    assert(ok_o_() == veil);
    assert(in_layer(14, veil) && !in_layer(15, veil));
    assert(pico8_entity_get_field(veil, "m_", &m_) && m_.kind == PICO8_VALUE_BOOL && m_.as.boolean);
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    h = fold_veil(h, veil);

    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    pico8_ui_ev(&gfx, &mem, pico8_ok());
    check("hover cursor", screen_hash(), HOVER_CURSOR_HASH);

    assert(pico8_ui_jz(pico8_ok(), "ldown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &kv, &kn));
    assert((void *)kv == veil && kn.kind == PICO8_VALUE_BOOL && kn.as.boolean);
    kv = NULL;
    if (pico8_ui_jz(pico8_ok(), "rdown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &kv, &kn)) {
        assert((void *)kv != veil);
    }
    h = fold_veil(h, veil);

    pico8_he(veil);
    pico8_ui_lj(&mem, pico8_ok());
    assert(ok_o_() != veil && !in_layer(14, veil));
    assert(pico8_entity_get_field(veil, "m_", &m_) && m_.kind == PICO8_VALUE_BOOL && !m_.as.boolean);
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    h = fold_veil(h, veil);
    check("hover", h, HOVER_SEQUENCE_HASH);
}

/* The veil is the entity of cv.hd whose `hd` is the plain value 1. */
static const void *find_veil(int *count) {
    const pico8_list_t *hd = pico8_cv("hd");
    const void *found = NULL;
    int i;

    *count = 0;
    for (i = 0; hd != NULL && i < hd->count; i++) {
        pico8_value_t v;
        const void *e = hd->items[i].as.ref;

        if (pico8_entity_get_field(e, "hd", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(1) &&
            pico8_entity_get_field(e, "fa", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(14)) {
            found = e;
            (*count)++;
        }
    }
    return found;
}

/* Replays `path` the way the scenario tests do, folding per frame whether a
 * veil is up, whether it is `ok.o_`, and the screen. */
static void replay(const char *path, int want_frames, uint32_t want_hash, const char *label) {
    uint32_t h = 2166136261u;
    pico8_value_t difficulty;
    pico8_cheat_t cheat;
    int frames;
    int seen = 0;
    int i;

    pico8_input_init(&input);
    assert(pico8_input_load(&input, path));
    pico8_dispatch_set_input(&input);
    pico8__init();
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty) && difficulty.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty.as.ref));

    frames = input.frame_count;
    pico8_cheat_init(&cheat);
    for (i = 0; i < frames; i++) {
        const void *veil;
        int count;

        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();

        veil = find_veil(&count);
        seen |= count > 0;
        h = fnv(h, (uint32_t)count);
        h = fnv(h, (uint32_t)(veil != NULL && listed(pico8_hv(), veil)));
        h = fnv(h, (uint32_t)(veil != NULL && ok_o_() == veil));
        h = fnv(h, screen_hash());

        if (i < frames - 1) {
            pico8_input_step(&input);
        }
    }
    pico8_input_free(&input);
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);

    assert(seen);
    printf("[%s] frames=%d\n", label, frames);
    fflush(stdout);
    assert(frames == want_frames);
    check(label, h, want_hash);
}

/* The veil's defaults are what the spec parser makes of fq's spec string,
 * which the class still carries. */
static void test_defaults_match_the_parsed_spec(void) {
    void *veil;
    pico8_value_t got;
    pico8_value_t parsed;

    pico8_jl();
    veil = pico8_fq_new(NULL);
    assert(pico8_fq_default("fa", &parsed));
    assert(pico8_entity_get_field(veil, "fa", &got) && got.kind == PICO8_VALUE_FIXED && got.as.fixed == parsed.as.fixed);
    assert(got.as.fixed == pico8_fixed_from_int(14));
    assert(pico8_fq_default("hd", &parsed));
    assert(pico8_entity_get_field(veil, "hd", &got) && got.kind == PICO8_VALUE_FIXED && got.as.fixed == parsed.as.fixed);
    assert(got.as.fixed == pico8_fixed_from_int(1));
    assert(pico8_entity_get_field(veil, "me", &got) && got.kind == PICO8_VALUE_STRING && strcmp(got.as.string, "ph") == 0);
    assert(!pico8_entity_get_field(veil, "my", &got));
}

static int is_a_veil_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_fq_struct_class();
}

/* Built by `pico8_fq_new` or through its class, a veil is a struct in hv
 * and cv.hd only; `hd` is a value `mv` hands back, and it has neither `ph`
 * nor `rdown`. */
static void test_fq_is_a_struct_registered_under_hd(void) {
    const char *others[] = {"kj", "ev", "hk", "hb"};
    void *built;
    void *generic;
    pico8_value_t out;
    int i;

    pico8_jl();
    built = pico8_fq_new(pico8_de());
    generic = pico8_fq_new(NULL);

    assert(is_a_veil_struct(built) && is_a_veil_struct(generic));
    assert(listed(pico8_hv(), built) && listed(pico8_cv("hd"), built));
    assert(listed(pico8_hv(), generic) && listed(pico8_cv("hd"), generic));
    for (i = 0; i < 4; i++) {
        assert(!listed(pico8_cv(others[i]), built) && !listed(pico8_cv(others[i]), generic));
    }
    assert(pico8_mv(generic, "hd", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == pico8_fixed_from_int(1));
    assert(!pico8_mv(generic, "ph", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(!pico8_mv(generic, "rdown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out));
    assert(pico8_entity_get_field(built, "my", &out) && out.kind == PICO8_VALUE_FUNCTION);
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

    replay("tests/fixtures/score_panel_close_events.json", SCORE_PANEL_CLOSE_FRAMES, SCORE_PANEL_CLOSE_HASH,
           "score panel close");
    replay("tests/fixtures/game_end_events.json", GAME_END_FRAMES, GAME_END_HASH, "game end");

    test_ldown_with_a_true_my_removes_the_veil();
    test_bare_veil_swallows_the_click_and_stays();
    test_false_my_keeps_the_veil();
    test_hover_loop_on_the_veil();
    test_defaults_match_the_parsed_spec();
    test_fq_is_a_struct_registered_under_hd();

    printf("test_pico8_fq: all tests passed\n");
    return 0;
}
