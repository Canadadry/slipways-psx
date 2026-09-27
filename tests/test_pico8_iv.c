/* The floating toast `iv`, through the real `lt`/`dg`/`dc("kj")` it goes
 * through in the game. The sequence and screen hashes were taken on the code
 * where `iv` was still a class instance and must not be re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_class.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_value.h"
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

static int listed(const pico8_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i].kind == PICO8_VALUE_REF && list->items[i].as.ref == e) {
            return 1;
        }
    }
    return 0;
}

/* A field as `mv` sees it: its value, or nil when absent. */
static pico8_value_t field(void *e, const char *name) {
    pico8_value_t v = pico8_value_ref(NULL);
    (void)pico8_mv((pico8_struct_header_t *)e, name, 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &v);
    return v;
}

static pico8_fixed_t field_fixed(void *e, const char *name) {
    pico8_value_t v = field(e, name);
    assert(v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static pico8_vec_t field_vec(void *e, const char *name) {
    pico8_value_t v = field(e, name);
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return *(const pico8_vec_t *)v.as.ref;
}

/* `jl()` again after `_init()`: `hv` holds only `mo`, `je` and `ok`, as at
 * the start of a game. The mouse `je` is put mid-screen, where a toast
 * placed off it is drawn under `mo`'s camera. */
static void fresh_world(void) {
    pico8_vec_t *mouse = malloc(sizeof *mouse);

    pico8_jl();
    *mouse = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
    assert(pico8_type_tag_of(pico8_je()) == PICO8_TYPE_TAG_STRUCT);
    ((pico8_nf_t *)pico8_je())->ra = pico8_value_ref(mouse);
}

static void *build(const pico8_vec_t *ra, const char *text, int color) {
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    void *e;

    if (ra != NULL) {
        pico8_vec_t *heap_ra = malloc(sizeof *heap_ra);
        *heap_ra = *ra;
        pico8_ob_table_set_name(overrides, "ra", pico8_value_ref(heap_ra));
    }
    pico8_ob_table_set_index(overrides, 1, pico8_value_string(text));
    pico8_ob_table_set_index(overrides, 2, pico8_value_fixed(pico8_fixed_from_int(color)));
    e = pico8_iv_new(overrides);
    pico8_ob_table_free(overrides);
    return e;
}

/* Lives the toast through `dg` until it leaves `hv`, folding every tick's
 * `ra`, `l`, `t`, list membership and `dc("kj")` screen into one hash. */
static void live(void *e, int want_ticks, uint32_t want_sequence, uint32_t want_first_draw, const char *label) {
    uint32_t h = 2166136261u;
    uint32_t first_draw;
    uint32_t blank;
    int ticks = 0;

    pico8_cls(&mem);
    blank = screen_hash();
    first_draw = draw_hash();

    while (listed(pico8_hv(), e)) {
        pico8_vec_t ra = field_vec(e, "ra");
        assert(ticks < 1000);
        h = fnv(h, (uint32_t)ra.x);
        h = fnv(h, (uint32_t)ra.y);
        h = fnv(h, (uint32_t)field_fixed(e, "l"));
        h = fnv(h, (uint32_t)field_fixed(e, "t"));
        h = fnv(h, (uint32_t)listed(pico8_cv("kj"), e));
        h = fnv(h, draw_hash());
        pico8_dg();
        ticks++;
    }
    assert(!listed(pico8_cv("kj"), e));
    h = fnv(h, (uint32_t)field_fixed(e, "l"));
    h = fnv(h, (uint32_t)field_fixed(e, "t"));

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
    pico8_value_t ox;

    fresh_world();
    e = build(&ra, "+1$", 11);
    mz = pico8_mz_new(NULL);
    assert(pico8_entity_get_field(mz, "ox", &ox) && ox.kind == PICO8_VALUE_REF && ox.as.ref != NULL);
    assert(!listed((const pico8_list_t *)ox.as.ref, e));
    assert(((const pico8_list_t *)ox.as.ref)->count == 0);
}

/* dg on a ke struct: the method named by `me` is called with `t`, `t` is
 * bumped on the struct itself, and a truthy `p_` takes it out of hv. */
typedef struct {
    pico8_ke_t ke;
    pico8_value_t p_;
} fake_toast_t;

static int g_fake_ph_calls = 0;
static pico8_fixed_t g_fake_ph_last_t;

static pico8_value_t fake_toast_ph(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    fake_toast_t *f = self;
    (void)argc;
    (void)arg1;
    assert(arg0.kind == PICO8_VALUE_FIXED);
    g_fake_ph_calls++;
    g_fake_ph_last_t = arg0.as.fixed;
    if (arg0.as.fixed >= pico8_fixed_from_int(2)) {
        f->p_ = pico8_value_bool(1);
    }
    return pico8_value_ref(NULL);
}

static int fake_toast_field(const void *self, const char *name, pico8_value_t *out_value) {
    const fake_toast_t *f = self;
    if (strcmp(name, "p_") == 0) {
        if (f->p_.kind == PICO8_VALUE_REF && f->p_.as.ref == NULL) {
            return 0;
        }
        *out_value = f->p_;
        return 1;
    }
    return pico8_ke_field(self, name, out_value);
}

static const pico8_struct_class_t g_fake_toast_class = {
    {[PICO8_METHOD_PH] = fake_toast_ph},
    fake_toast_field,
    NULL,
};

static void test_dg_drives_a_ke_struct(void) {
    fake_toast_t *f = malloc(sizeof *f);
    int i;

    fresh_world();
    pico8_ke_struct_init(&f->ke, &g_fake_toast_class);
    f->p_ = pico8_value_ref(NULL);
    pico8_ke_struct_pq(&f->ke);
    assert(listed(pico8_hv(), f));

    for (i = 0; i < 2; i++) {
        pico8_dg();
        assert(g_fake_ph_calls == i + 1 && g_fake_ph_last_t == pico8_fixed_from_int(i));
        assert(f->ke.t.kind == PICO8_VALUE_FIXED && f->ke.t.as.fixed == pico8_fixed_from_int(i + 1));
        assert(listed(pico8_hv(), f));
    }
    pico8_dg();
    assert(g_fake_ph_calls == 3 && g_fake_ph_last_t == pico8_fixed_from_int(2));
    assert(!listed(pico8_hv(), f));
    assert(f->ke.t.as.fixed == pico8_fixed_from_int(3));
}

/* The struct's defaults are, bit for bit, what the spec parser makes of
 * iv's spec string, which the class still carries. */
static void test_defaults_match_the_parsed_spec(void) {
    const pico8_iv_t *e;
    pico8_value_t parsed;

    fresh_world();
    e = build(NULL, "x", 7);
    assert(pico8_iv_default("v", &parsed) && parsed.kind == PICO8_VALUE_REF);
    assert(e->v.kind == PICO8_VALUE_REF && e->v.as.ref != NULL);
    assert(((const pico8_vec_t *)e->v.as.ref)->x == ((const pico8_vec_t *)parsed.as.ref)->x);
    assert(((const pico8_vec_t *)e->v.as.ref)->y == ((const pico8_vec_t *)parsed.as.ref)->y);
    assert(pico8_iv_default("fa", &parsed));
    assert(e->fa.kind == PICO8_VALUE_FIXED && e->fa.as.fixed == parsed.as.fixed);
    assert(pico8_iv_default("l", &parsed));
    assert(e->l.kind == PICO8_VALUE_FIXED && e->l.as.fixed == parsed.as.fixed);
    assert(e->ke.me.kind == PICO8_VALUE_STRING && strcmp(e->ke.me.as.string, "ph") == 0);
    assert(e->ke.t.kind == PICO8_VALUE_FIXED && e->ke.t.as.fixed == 0);
    assert(((const pico8_iv_t *)build(NULL, "y", 7))->v.as.ref == e->v.as.ref);
}

/* Built directly or through its class, an iv is a struct that lands in hv
 * and cv.kj only. */
static void test_iv_is_a_struct_registered_under_kj(void) {
    const char *others[] = {"ev", "hd", "hk", "hb"};
    pico8_ob_table_t *overrides = pico8_ob_table_new();
    void *direct;
    void *generic;
    int i;

    fresh_world();
    pico8_ob_table_set_index(overrides, 1, pico8_value_string("z"));
    direct = build(NULL, "z", 7);
    generic = pico8_iv_new(overrides);
    pico8_ob_table_free(overrides);

    assert(pico8_type_tag_of(direct) == PICO8_TYPE_TAG_STRUCT);
    assert(pico8_type_tag_of(generic) == PICO8_TYPE_TAG_STRUCT);
    assert(listed(pico8_hv(), direct) && listed(pico8_cv("kj"), direct));
    assert(listed(pico8_hv(), generic) && listed(pico8_cv("kj"), generic));
    for (i = 0; i < 4; i++) {
        assert(!listed(pico8_cv(others[i]), direct) && !listed(pico8_cv(others[i]), generic));
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
