/* The click veil `fq`, built bare (`pj:ph`) and with a `my` (`ix[3]` with
 * `de`), lived through the real `dg`, its `ldown` and the hover loop of
 * `ok`, in a game built directly: no events file. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_score_panel_display.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_zone_interface.h"

#define MY_TRUE_TICKS 6
#define MY_TRUE_SEQUENCE_HASH 0xA6A43C27u
#define BARE_SEQUENCE_HASH 0x52BE382Au
#define HOVER_SEQUENCE_HASH 0x689321F9u
#define HOVER_CURSOR_HASH 0x04C40AB9u

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

static int ents_listed(const pico8_ents_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return 1;
        }
    }
    return 0;
}

/* The kind words the frozen hashes fold before a value: any other value
 * moves them. */
enum { FIXED_KIND = 0, BOOL_KIND = 2 };

static uint32_t fold_bool_member(uint32_t h, int has, int b) {
    if (!has) {
        return fnv(h, 0xDEADu);
    }
    return fnv(fnv(h, (uint32_t)BOOL_KIND), (uint32_t)b);
}

/* The kind words are constant by the struct's shape, kept so that the
 * sequence hashes still prove the typed reads. */
static uint32_t fold_veil(uint32_t h, const pico8_fq_t *e) {
    h = fnv(fnv(h, (uint32_t)FIXED_KIND), (uint32_t)e->header.t);
    h = fold_bool_member(h, (e->p_ != 0), e->p_);
    h = fold_bool_member(h, (e->m_ != 0), e->m_);
    h = fnv(h, (uint32_t)ents_listed(pico8_hv(), e));
    return fnv(h, (uint32_t)ents_listed(pico8_cv("hd"), e));
}

static const void *ok_o_(void) {
    return pico8_ok()->o_;
}

static int g_my_calls = 0;
static int g_my_answer = 1;

static int counting_my(pico8_fq_t *self) {
    (void)self;
    g_my_calls++;
    return g_my_answer;
}

static pico8_fq_t *veil_with_my(int answer) {
    pico8_fq_keys_t keys = {0};

    g_my_calls = 0;
    g_my_answer = answer;
    keys.my = counting_my;
    return pico8_fq_new(&keys);
}

static int ldown(void *veil) {
    pico8_down_t out = pico8_fq_struct_class()->ldown(veil, NULL, NULL);

    assert(out.held == NULL);
    return out.stop;
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
    pico8_fq_t *veil;
    int ticks = 0;

    pico8_jl();
    veil = veil_with_my(1);
    h = fold_veil(h, veil);
    while (ents_listed(pico8_hv(), veil)) {
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
    assert(!ents_listed(pico8_cv("hd"), veil));
    printf("[my true] ticks=%d\n", ticks);
    fflush(stdout);
    assert(ticks == MY_TRUE_TICKS);
    check("my true", h, MY_TRUE_SEQUENCE_HASH);
}

/* `fq()`: a click is swallowed, `p_` stays nil, the veil stays. */
static void test_bare_veil_swallows_the_click_and_stays(void) {
    uint32_t h = 2166136261u;
    pico8_fq_t *veil;
    int i;

    pico8_jl();
    veil = pico8_fq_new(NULL);
    for (i = 0; i < IDLE_TICKS; i++) {
        if (i == LDOWN_TICK) {
            assert(ldown(veil));
            assert(!veil->p_);
        }
        pico8_dg();
        h = fold_veil(h, veil);
    }
    assert(ents_listed(pico8_hv(), veil) && ents_listed(pico8_cv("hd"), veil));
    check("bare", h, BARE_SEQUENCE_HASH);
}

/* A `my` answering false leaves `p_` false, which reads as nil: the veil
 * stays, as a bare one. */
static void test_false_my_keeps_the_veil(void) {
    uint32_t h = 2166136261u;
    pico8_fq_t *veil;
    int i;

    pico8_jl();
    veil = veil_with_my(0);
    for (i = 0; i < IDLE_TICKS; i++) {
        if (i == LDOWN_TICK) {
            assert(ldown(veil));
            assert(g_my_calls == 1);
            assert(!veil->p_);
        }
        pico8_dg();
        h = fold_veil(h, veil);
    }
    assert(ents_listed(pico8_hv(), veil) && ents_listed(pico8_cv("hd"), veil));
    check("my false", h, BARE_SEQUENCE_HASH);
}

static int in_layer(int layer, const void *e) {
    const pico8_ui_ic_t *ic = pico8_ok()->ic;

    if (ic == NULL) {
        return 0;
    }
    return ents_listed(&ic->layers[layer], e);
}

/* A started game, then `fq()`: `ok`'s hover loop makes it `o_` (hd=1 is
 * always a hit) in layer 14, marks it `m_`, takes `ldown` but not `rdown`,
 * and, once the veil is gone, unmarks it through `gj`. */
static void test_hover_loop_on_the_veil(void) {
    uint32_t h = 2166136261u;
    pico8_down_t kn;
    pico8_struct_header_t *kv = NULL;
    pico8_fq_t *veil;

    pico8__init();
    pico8_eu(NULL);
    veil = pico8_fq_new(NULL);

    pico8_ui_lj(&mem, pico8_ok());
    assert(ok_o_() == veil);
    assert(in_layer(14, veil) && !in_layer(15, veil));
    assert(veil->m_);
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    h = fold_veil(h, veil);

    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    pico8_ui_ev(&gfx, &mem, pico8_ok());
    check("hover cursor", screen_hash(), HOVER_CURSOR_HASH);

    assert(pico8_ui_jz(pico8_ok(), PICO8_JZ_LDOWN, NULL, NULL, NULL, &kv, &kn));
    assert((void *)kv == veil && kn.stop && kn.held == NULL);
    kv = NULL;
    if (pico8_ui_jz(pico8_ok(), PICO8_JZ_RDOWN, NULL, NULL, NULL, &kv, &kn)) {
        assert((void *)kv != veil);
    }
    h = fold_veil(h, veil);

    pico8_he(&veil->header);
    pico8_ui_lj(&mem, pico8_ok());
    assert(ok_o_() != veil && !in_layer(14, veil));
    assert(!veil->m_);
    h = fnv(h, (uint32_t)pico8_peek(&mem, 0x5f80));
    h = fold_veil(h, veil);
    check("hover", h, HOVER_SEQUENCE_HASH);
}

/* The veil's defaults are the class's generated defaults. */
static void test_defaults_match_the_parsed_spec(void) {
    pico8_fq_t *veil;

    pico8_jl();
    veil = pico8_fq_new(NULL);
    assert(veil->def == &pico8_fq_def);
    assert(veil->def->fa == pico8_fixed_from_int(14));
    assert(veil->def->hd == pico8_fixed_from_int(1));
    assert(veil->header.me != NULL && strcmp(veil->header.me, "ph") == 0);
    assert(pico8_fq_ready());
    pico8_fq_shutdown();
    assert(!pico8_fq_ready());
    pico8_fq_init();
}

static int is_a_veil_struct(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_fq_struct_class();
}

/* Built by `pico8_fq_new` or through its class, a veil is a struct in hv
 * and cv.hd only; its `hd` is the constant hit of `hd=1`, and it has
 * neither `ph` nor `rdown`. */
static void test_fq_is_a_struct_registered_under_hd(void) {
    const char *others[] = {"kj", "ev", "hk", "hb"};
    const pico8_struct_class_t *cls = pico8_fq_struct_class();
    pico8_fq_keys_t keys = {0};
    void *built;
    void *generic;
    int i;

    pico8_jl();
    keys.my = pico8_de_my();
    built = pico8_fq_new(&keys);
    generic = pico8_fq_new(NULL);

    assert(is_a_veil_struct(built) && is_a_veil_struct(generic));
    assert(ents_listed(pico8_hv(), built) && ents_listed(pico8_cv("hd"), built));
    assert(ents_listed(pico8_hv(), generic) && ents_listed(pico8_cv("hd"), generic));
    for (i = 0; i < 4; i++) {
        assert(!ents_listed(pico8_cv(others[i]), built) && !ents_listed(pico8_cv(others[i]), generic));
    }
    assert(cls->hd == pico8_hd_true && cls->hd(generic, NULL, NULL) == 1);
    assert(cls->states[PICO8_STATE_PH] == NULL && cls->rdown == NULL);
    assert(((const pico8_fq_t *)built)->my == pico8_de_my());
}

/* A veil takes a place; once its `my` lifts `p_` and `dg` takes it out,
 * emptying the exit queue gives the place back. */
static void test_veil_gives_its_place_back(void) {
    pico8_fq_t *veil;
    int base;
    int ticks = 0;

    pico8_jl();
    pico8_cart_vider_file_sortie();
    base = pico8_zone_interface_vivantes(PICO8_SORTE_FQ);
    veil = veil_with_my(1);
    assert(pico8_zone_interface_vivantes(PICO8_SORTE_FQ) == base + 1);
    assert(ldown(veil));
    while (ents_listed(pico8_hv(), veil)) {
        pico8_dg();
        assert(++ticks < 1000);
    }
    assert(pico8_zone_interface_vivantes(PICO8_SORTE_FQ) == base + 1 && "out of hv, still read this frame");
    pico8_cart_vider_file_sortie();
    assert(pico8_zone_interface_vivantes(PICO8_SORTE_FQ) == base);
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

    test_ldown_with_a_true_my_removes_the_veil();
    test_bare_veil_swallows_the_click_and_stays();
    test_false_my_keeps_the_veil();
    test_hover_loop_on_the_veil();
    test_defaults_match_the_parsed_spec();
    test_fq_is_a_struct_registered_under_hd();
    test_veil_gives_its_place_back();

    printf("test_pico8_fq: all tests passed\n");
    return 0;
}
