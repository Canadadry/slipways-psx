/* The floating toast `iv`, through the real `lt`/`dg`/`dc("kj")` it goes
 * through in the game. The sequence and screen hashes were taken on the code
 * where `iv` was still a class instance and must not be re-taken. */

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
#define IV_MEMBERS_HASH 0x59C5F13Bu
#define WITNESS 0x1234

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
    ((pico8_nf_t *)pico8_je())->ra = mouse;
}

static void *build(const pico8_vec_t *ra, const char *text, int color) {
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
    mz = pico8_mz_new();
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
        assert(f->ke.t == pico8_fixed_from_int(i + 1));
        assert(listed(pico8_hv(), f));
    }
    pico8_dg();
    assert(g_fake_ph_calls == 3 && g_fake_ph_last_t == pico8_fixed_from_int(2));
    assert(!listed(pico8_hv(), f));
    assert(f->ke.t == pico8_fixed_from_int(3));
}

/* The struct's defaults are, bit for bit, the class's generated defaults;
 * its `v` is the generated vector itself. */
static void test_defaults_match_the_parsed_spec(void) {
    const pico8_iv_t *e;
    pico8_value_t v;

    fresh_world();
    e = build(NULL, "x", 7);
    v = field((void *)e, "v");
    assert(v.kind == PICO8_VALUE_REF && v.as.ref == &pico8_iv_start.v);
    assert(pico8_iv_start.v.x == 0 && pico8_iv_start.v.y == -0x570a);
    assert(field_fixed((void *)e, "fa") == pico8_iv_start.fa);
    assert(field_fixed((void *)e, "l") == pico8_iv_start.l);
    assert(e->ke.me != NULL && strcmp(e->ke.me, "ph") == 0);
    assert(e->ke.t == 0);
    assert(field(build(NULL, "y", 7), "v").as.ref == v.as.ref);
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

    assert(pico8_type_tag_of(direct) == PICO8_TYPE_TAG_STRUCT);
    assert(pico8_type_tag_of(generic) == PICO8_TYPE_TAG_STRUCT);
    assert(listed(pico8_hv(), direct) && listed(pico8_cv("kj"), direct));
    assert(listed(pico8_hv(), generic) && listed(pico8_cv("kj"), generic));
    for (i = 0; i < 4; i++) {
        assert(!listed(pico8_cv(others[i]), direct) && !listed(pico8_cv(others[i]), generic));
    }
}

static uint32_t g_iv_h;

static void fold(uint32_t word) {
    g_iv_h = fnv(g_iv_h, word);
}

static int same_value(pico8_value_t a, pico8_value_t b) {
    if (a.kind != b.kind) {
        return 0;
    }
    switch (a.kind) {
    case PICO8_VALUE_FIXED:
        return a.as.fixed == b.as.fixed;
    case PICO8_VALUE_BOOL:
        return a.as.boolean == b.as.boolean;
    case PICO8_VALUE_STRING:
        return a.as.string == b.as.string;
    case PICO8_VALUE_REF:
        return a.as.ref == b.as.ref;
    default:
        return a.as.function == b.as.function;
    }
}

/* The raw reader and pico8_entity_get_field() agree; on 0 both leave the
 * witness intact. */
static int read_member(void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(WITNESS);
    pico8_value_t via = pico8_value_fixed(WITNESS);
    int r1 = ((const pico8_struct_header_t *)e)->cls->field(e, name, &raw);
    int r2 = pico8_entity_get_field(e, name, &via);

    assert(r1 == r2 && same_value(raw, via));
    if (!r1) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == WITNESS);
    }
    fold((uint32_t)r1);
    fold((uint32_t)raw.kind);
    *out = raw;
    return r1;
}

static pico8_fixed_t read_fixed(void *e, const char *name) {
    pico8_value_t v;
    assert(read_member(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    fold((uint32_t)v.as.fixed);
    return v.as.fixed;
}

static const pico8_vec_t *read_vec(void *e, const char *name) {
    pico8_value_t v;
    const pico8_vec_t *p;
    assert(read_member(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    p = v.as.ref;
    fold((uint32_t)p->x);
    fold((uint32_t)p->y);
    return p;
}

static void *make_iv(pico8_vec_t *ra, const char *at1, pico8_fixed_t at2) {
    pico8_iv_keys_t keys = {0};

    keys.ra = ra;
    keys.at1 = at1;
    keys.at2 = at2;
    return pico8_iv_new(&keys);
}

static uint32_t kj_hash(void *e) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
    uint32_t h;

    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    pico8_iv_kj(&gfx, &mem, e, p);
    h = screen_hash();
    fold(h);
    return h;
}

static pico8_value_t zero_of(pico8_value_kind_t kind) {
    pico8_value_t v;
    memset(&v, 0, sizeof v);
    v.kind = kind;
    return v;
}

static void must_abort_write(void *e, const char *name, pico8_value_t v) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        pico8_entity_set_field(e, name, v);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

static const void *read_members_of_a_fresh_iv(void *e, const pico8_vec_t *want_ra, const pico8_vec_t *v) {
    const char *intruders[] = {"at1", "at2", "has_p_", "ke", "header", "member", "", "P_", "ra "};
    pico8_value_t out;
    const pico8_vec_t *ra;
    size_t i;

    assert(read_fixed(e, "fa") == pico8_fixed_from_int(9));
    assert(read_fixed(e, "l") == pico8_fixed_from_int(0));
    assert(read_vec(e, "v") == v);
    ra = read_vec(e, "ra");
    if (want_ra != NULL) {
        assert(ra == want_ra);
    }
    assert(!read_member(e, "p_", &out));
    assert(read_member(e, "me", &out) && out.kind == PICO8_VALUE_STRING && strcmp(out.as.string, "ph") == 0);
    assert(read_fixed(e, "t") == 0);
    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        assert(!read_member(e, intruders[i], &out));
    }
    return ra;
}

/* `fa`, `l`, `v`, `ra`, `p_` read by name as the toast lives; `at1`/`at2`
 * reach the screen only; every write by name aborts. */
static void test_iv_members_by_name(void) {
    pico8_vec_t *key_ra = malloc(sizeof *key_ra);
    pico8_value_t out;
    pico8_value_t nil = pico8_value_ref(NULL);
    const pico8_vec_t *v;
    const void *ras[5];
    const char *names[] = {"fa", "l", "v", "ra", "p_", "at1", "at2"};
    void *a;
    void *b;
    void *n;
    void *r;
    void *sx;
    void *c;
    void *z;
    void *e;
    int first_p_ = 0;
    int i;
    int j;

    g_iv_h = 2166136261u;
    fresh_world();
    *key_ra = pico8_vec_v(pico8_fixed_from_int(250), pico8_fixed_from_int(260));
    a = make_iv(key_ra, "+12$", pico8_fixed_from_int(11));
    b = build(NULL, "x", 7);
    n = pico8_iv_new(NULL);
    r = make_iv(NULL, "r", pico8_fixed_from_int(3));
    sx = make_iv(NULL, NULL, pico8_fixed_from_int(5));

    assert(read_member(a, "v", &out) && out.kind == PICO8_VALUE_REF);
    v = out.as.ref;
    assert(v->x == 0 && v->y == -PICO8_FIXED_LIT(0x0000570a, 0.34));
    ras[0] = read_members_of_a_fresh_iv(a, key_ra, v);
    ras[1] = read_members_of_a_fresh_iv(b, NULL, v);
    ras[2] = read_members_of_a_fresh_iv(n, NULL, v);
    ras[3] = read_members_of_a_fresh_iv(r, NULL, v);
    ras[4] = read_members_of_a_fresh_iv(sx, NULL, v);
    for (i = 0; i < 5; i++) {
        for (j = i + 1; j < 5; j++) {
            assert(ras[i] != ras[j]);
        }
    }

    for (i = 1; i <= 51; i++) {
        const pico8_vec_t *before = read_vec(b, "ra");
        pico8_vec_t was = *before;
        pico8_fixed_t l = read_fixed(b, "l");
        const pico8_vec_t *after;

        pico8_iv_ph(b);
        after = read_vec(b, "ra");
        assert(after != before);
        assert(after->x == pico8_fixed_add(was.x, v->x) && after->y == pico8_fixed_add(was.y, v->y));
        assert(read_fixed(b, "l") == pico8_fixed_add(l, PICO8_FIXED_LIT(0x00001999, 0.1)));
        assert(read_member(b, "p_", &out) && out.kind == PICO8_VALUE_BOOL);
        fold((uint32_t)out.as.boolean);
        if (out.as.boolean && first_p_ == 0) {
            first_p_ = i;
        }
        assert(read_fixed(b, "fa") == pico8_fixed_from_int(9));
        assert(read_vec(b, "v") == v);
    }
    fold((uint32_t)first_p_);

    c = make_iv(NULL, "c", 0);
    z = make_iv(NULL, "c", pico8_fixed_from_int(0));
    (void)kj_hash(a);
    (void)kj_hash(b);
    (void)kj_hash(n);
    (void)kj_hash(sx);
    assert(kj_hash(c) == kj_hash(z));
    e = build(NULL, "palette", 8);
    for (i = 0; i < 20; i++) {
        pico8_iv_ph(e);
    }
    (void)kj_hash(e);

    for (i = 0; i < 7; i++) {
        must_abort_write(a, names[i], pico8_value_bool(1));
    }
    for (i = 0; i < 2; i++) {
        const char *name = i == 0 ? "p_" : "ra";
        must_abort_write(a, name, nil);
        must_abort_write(a, name, zero_of(PICO8_VALUE_FIXED));
        must_abort_write(a, name, zero_of(PICO8_VALUE_BOOL));
        must_abort_write(a, name, zero_of(PICO8_VALUE_STRING));
        must_abort_write(a, name, zero_of(PICO8_VALUE_FUNCTION));
    }

    printf("test_iv_members_by_name: first p_ at ph %d, hash 0x%08X\n", first_p_, (unsigned)g_iv_h);
    fflush(stdout);
    assert(g_iv_h == IV_MEMBERS_HASH);
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
    test_iv_members_by_name();

    printf("test_pico8_iv: all tests passed\n");
    return 0;
}
