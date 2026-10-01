/* The floating toast `iv`, through the real `lt`/`dg`/`dc("kj")` it goes
 * through in the game. The sequence and screen hashes were taken on the code
 * where `iv` was still a class instance and must not be re-taken. */

#define _POSIX_C_SOURCE 200809L

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
#include "../src/pico8_ke.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_vec.h"
#include "../src/pico8_wormhole.h"

#define NO_MONEY_TICKS 51
#define NO_MONEY_SEQUENCE_HASH 0x0F5C11D3u
#define NO_MONEY_FIRST_DRAW_HASH 0x5D21D95Du
#define PRICE_TICKS 51
#define PRICE_SEQUENCE_HASH 0xA692DEE9u
#define PRICE_FIRST_DRAW_HASH 0x884AD566u

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

static uint32_t draw_hash(void) {
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    pico8_dc("kj");
    return screen_hash();
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

/* `jl()` again after `_init()`: `hv` holds only `mo`, `je` and `ok`, as at
 * the start of a game. The mouse `je` is put mid-screen, where a toast
 * placed off it is drawn under `mo`'s camera. */
static void fresh_world(void) {
    pico8_vec_t *mouse = malloc(sizeof *mouse);

    pico8_jl();
    *mouse = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
    assert(pico8_je() != NULL && pico8_je()->cls == pico8_nf_struct_class());
    ((pico8_nf_t *)pico8_je())->ra = mouse;
}

static pico8_iv_t *build(const pico8_vec_t *ra, const char *text, int color) {
    pico8_iv_keys_t keys = {0};

    if (ra != NULL) {
        pico8_vec_t *heap_ra = malloc(sizeof *heap_ra);
        *heap_ra = *ra;
        keys.ra = heap_ra;
    }
    keys.at1 = text;
    keys.at2 = pico8_fixed_from_int(color);
    return pico8_iv_new(&keys);
}

/* Lives the toast through `dg` until it leaves `hv`, folding every tick's
 * `ra`, `l`, `t`, list membership and `dc("kj")` screen into one hash. */
static void live(pico8_iv_t *e, int want_ticks, uint32_t want_sequence, uint32_t want_first_draw, const char *label) {
    uint32_t h = 2166136261u;
    uint32_t first_draw;
    uint32_t blank;
    int ticks = 0;

    pico8_cls(&mem);
    blank = screen_hash();
    first_draw = draw_hash();

    while (ents_listed(pico8_hv(), e)) {
        pico8_vec_t ra;
        assert(e->ra != NULL);
        ra = *e->ra;
        assert(ticks < 1000);
        h = fnv(h, (uint32_t)ra.x);
        h = fnv(h, (uint32_t)ra.y);
        h = fnv(h, (uint32_t)e->l);
        h = fnv(h, (uint32_t)e->header.t);
        h = fnv(h, (uint32_t)ents_listed(pico8_cv("kj"), e));
        h = fnv(h, draw_hash());
        pico8_dg();
        ticks++;
    }
    assert(!ents_listed(pico8_cv("kj"), e));
    h = fnv(h, (uint32_t)e->l);
    h = fnv(h, (uint32_t)e->header.t);

    printf("[%s] ticks=%d sequence=0x%08Xu first_draw=0x%08Xu\n", label, ticks, (unsigned)h, (unsigned)first_draw);
    fflush(stdout);
    assert(first_draw != blank);
    assert(ticks == want_ticks);
    assert(h == want_sequence);
    assert(first_draw == want_first_draw);
}

/* `hy:lu`'s "-no money-": no `ra`, so `iv:pq` places it off `je` and `mo`. */
static void test_no_money_toast_lives_as_before(void) {
    fresh_world();
    live(build(NULL, "-no money-", 8), NO_MONEY_TICKS, NO_MONEY_SEQUENCE_HASH, NO_MONEY_FIRST_DRAW_HASH, "no money");
}

/* `jo:hk`'s price: the planet's own `ra` is kept. */
static void test_price_toast_lives_as_before(void) {
    pico8_vec_t ra = pico8_vec_v(pico8_fixed_from_int(250), pico8_fixed_from_int(260));
    fresh_world();
    live(build(&ra, "+12$", 11), PRICE_TICKS, PRICE_SEQUENCE_HASH, PRICE_FIRST_DRAW_HASH, "price");
}

/* A toast alive when a wormhole is built is scanned by `mz:pq` for `fx`,
 * which it does not have. */
static void test_mz_scan_skips_a_live_toast(void) {
    pico8_vec_t ra = pico8_vec_v(pico8_fixed_from_int(40), pico8_fixed_from_int(60));
    void *e;
    pico8_mz_t *mz;

    fresh_world();
    e = build(&ra, "+1$", 11);
    mz = pico8_mz_new();
    assert(mz->ox != NULL);
    assert(!ents_listed(mz->ox, e));
    assert(mz->ox->count == 0);
}

/* dg on a ke struct: the method named by `me` is called with `t`, `t` is
 * bumped on the struct itself, and a truthy `p_` takes it out of hv. */
typedef struct {
    pico8_struct_header_t header;
    int p_;
} fake_toast_t;

static int g_fake_ph_calls = 0;
static pico8_fixed_t g_fake_ph_last_t;

static void fake_toast_ph(pico8_struct_header_t *self, pico8_fixed_t t) {
    fake_toast_t *f = (fake_toast_t *)self;
    g_fake_ph_calls++;
    g_fake_ph_last_t = t;
    if (t >= pico8_fixed_from_int(2)) {
        f->p_ = 1;
    }
}

static const pico8_struct_class_t g_fake_toast_class = {
    .states = {[PICO8_STATE_PH] = fake_toast_ph},
};

static void test_dg_drives_a_ke_struct(void) {
    fake_toast_t *f = malloc(sizeof *f);
    int i;

    fresh_world();
    pico8_ke_struct_init(&f->header, &g_fake_toast_class, "ph", 0);
    f->header.p_ = &f->p_;
    f->p_ = 0;
    pico8_ke_struct_pq(&f->header);
    assert(ents_listed(pico8_hv(), f));

    for (i = 0; i < 2; i++) {
        pico8_dg();
        assert(g_fake_ph_calls == i + 1 && g_fake_ph_last_t == pico8_fixed_from_int(i));
        assert(f->header.t == pico8_fixed_from_int(i + 1));
        assert(ents_listed(pico8_hv(), f));
    }
    pico8_dg();
    assert(g_fake_ph_calls == 3 && g_fake_ph_last_t == pico8_fixed_from_int(2));
    assert(!ents_listed(pico8_hv(), f));
    assert(f->header.t == pico8_fixed_from_int(3));
}

/* The struct's defaults are its archetype's, whose `fa` and `v` are the
 * spec's. */
static void test_defaults_match_the_parsed_spec(void) {
    const pico8_iv_t *e;

    fresh_world();
    e = build(NULL, "x", 7);
    assert(e->def == &pico8_iv_def);
    assert(e->def->v.x == 0 && e->def->v.y == -0x570a);
    assert(e->def->fa == pico8_fixed_from_int(9));
    assert(e->l == e->def->l);
    assert(e->header.me != NULL && strcmp(e->header.me, "ph") == 0);
    assert(e->header.t == 0);
    assert(build(NULL, "y", 7)->def == e->def);
    assert(pico8_iv_ready());
    pico8_iv_shutdown();
    assert(!pico8_iv_ready());
    pico8_iv_init();
}

/* Built directly or through its class, an iv is a struct that lands in hv
 * and cv.kj only. */
static void test_iv_is_a_struct_registered_under_kj(void) {
    const char *others[] = {"ev", "hd", "hk", "hb"};
    pico8_iv_keys_t keys = {0};
    void *direct;
    void *generic;
    int i;

    fresh_world();
    keys.at1 = "z";
    direct = build(NULL, "z", 7);
    generic = pico8_iv_new(&keys);

    assert(direct != NULL);
    assert(generic != NULL);
    assert(ents_listed(pico8_hv(), direct) && ents_listed(pico8_cv("kj"), direct));
    assert(ents_listed(pico8_hv(), generic) && ents_listed(pico8_cv("kj"), generic));
    for (i = 0; i < 4; i++) {
        assert(!ents_listed(pico8_cv(others[i]), direct) && !ents_listed(pico8_cv(others[i]), generic));
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
    pico8__init();

    test_no_money_toast_lives_as_before();
    test_price_toast_lives_as_before();
    test_mz_scan_skips_a_live_toast();
    test_dg_drives_a_ke_struct();
    test_defaults_match_the_parsed_spec();
    test_iv_is_a_struct_registered_under_kj();

    printf("test_pico8_iv: all tests passed\n");
    return 0;
}
