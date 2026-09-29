/* The click veil `fq`, built bare (`pj:ph`) and with a `my` (`ix[3]` with
 * `de`), lived through the real `dg`, `mv("ldown")` and the hover loop of
 * `ok`, in a game built directly: no events file. */

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
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_score_panel_display.h"
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
#define HOVER_CURSOR_HASH 0x04C40AB9u
#define FQ_MEMBERS_HASH 0x76872F8Du

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
    pico8_fq_keys_t keys = {0};

    g_my_calls = 0;
    g_my_answer = answer;
    keys.my = counting_my;
    return pico8_fq_new(&keys);
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

    if (!pico8_entity_get_field(pico8_ok(), "ic", &ic) || ic.kind != PICO8_VALUE_REF || ic.as.ref == NULL) {
        return 0;
    }
    return listed(&((const pico8_ui_ic_t *)ic.as.ref)->layers[layer], e);
}

/* A started game, then `fq()`: `ok`'s hover loop makes it `o_` (hd=1 is
 * always a hit) in layer 14, marks it `m_`, takes `ldown` but not `rdown`,
 * and, once the veil is gone, unmarks it through `gj`. */
static void test_hover_loop_on_the_veil(void) {
    uint32_t h = 2166136261u;
    pico8_value_t kn;
    pico8_value_t m_;
    pico8_struct_header_t *kv = NULL;
    void *veil;

    pico8__init();
    pico8_eu(NULL);
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

/* The veil's defaults are the class's generated defaults. */
static void test_defaults_match_the_parsed_spec(void) {
    void *veil;
    pico8_value_t got;

    pico8_jl();
    veil = pico8_fq_new(NULL);
    assert(pico8_entity_get_field(veil, "fa", &got) && got.kind == PICO8_VALUE_FIXED);
    assert(got.as.fixed == pico8_fq_start.fa);
    assert(got.as.fixed == pico8_fixed_from_int(14));
    assert(pico8_entity_get_field(veil, "hd", &got) && got.kind == PICO8_VALUE_FIXED);
    assert(got.as.fixed == pico8_fq_start.hd);
    assert(got.as.fixed == pico8_fixed_from_int(1));
    assert(pico8_entity_get_field(veil, "me", &got) && got.kind == PICO8_VALUE_STRING && strcmp(got.as.string, "ph") == 0);
    assert(!pico8_entity_get_field(veil, "my", &got));
    assert(pico8_fq_ready());
    pico8_fq_shutdown();
    assert(!pico8_fq_ready());
    pico8_fq_init();
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
    pico8_fq_keys_t keys = {0};
    void *built;
    void *generic;
    pico8_value_t out;
    int i;

    pico8_jl();
    keys.my = pico8_de_my();
    built = pico8_fq_new(&keys);
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

static pico8_value_t g_script[8];
static int g_script_len = 0;
static int g_script_calls = 0;

static pico8_value_t scripted_my(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    assert(g_script_calls < g_script_len);
    return g_script[g_script_calls++];
}

static void *veil_with_key(pico8_value_function_t my) {
    pico8_fq_keys_t keys = {0};

    keys.my = my;
    return pico8_fq_new(&keys);
}

/* Raw reader and entity reader must agree; a miss leaves the witness. */
static uint32_t fold_raw(uint32_t h, const void *e, const char *name) {
    pico8_value_t raw = pico8_value_fixed(0x1234);
    pico8_value_t via = pico8_value_fixed(0x1234);
    int got_raw = pico8_fq_struct_class()->field(e, name, &raw);
    int got_via = pico8_entity_get_field(e, name, &via);

    assert(got_raw == got_via);
    assert(raw.kind == via.kind);
    if (!got_raw) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)raw.kind);
    if (raw.kind == PICO8_VALUE_FIXED) {
        assert(raw.as.fixed == via.as.fixed);
        return fnv(h, (uint32_t)raw.as.fixed);
    }
    if (raw.kind == PICO8_VALUE_BOOL) {
        assert(raw.as.boolean == via.as.boolean);
        return fnv(h, (uint32_t)raw.as.boolean);
    }
    if (raw.kind == PICO8_VALUE_FUNCTION) {
        assert(raw.as.function == via.as.function);
    }
    if (raw.kind == PICO8_VALUE_STRING) {
        assert(strcmp(raw.as.string, via.as.string) == 0);
        h = fnv(h, (uint32_t)strlen(raw.as.string));
    }
    return h;
}

static uint32_t fold_members(uint32_t h, const void *e) {
    static const char *const names[] = {"fa", "hd", "my", "p_", "m_", "me", "t"};
    size_t i;

    for (i = 0; i < sizeof names / sizeof names[0]; i++) {
        h = fold_raw(h, e, names[i]);
    }
    return h;
}

static pico8_value_t zero_of(pico8_value_kind_t kind) {
    pico8_value_t v;

    memset(&v, 0, sizeof v);
    v.kind = kind;
    return v;
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

/* Every member of a bare, a built and a nil-keyed veil read by name; `p_`
 * through `ldown` only; `m_` written by name. */
static void test_fq_members_by_name(void) {
    static const char *const intruders[] = {"", "x", "M_", "P_", "m", "p", "has_m_", "has_p_", "ke", "header"};
    static const char *const unwritten[] = {"fa", "hd", "my", "p_", "me", "t", "", "m_ ", "has_m_", "has_p_"};
    pico8_fq_keys_t keys = {0};
    pico8_value_t de_my;
    pico8_value_t got;
    void *bare;
    void *built;
    void *nil_key;
    void *scripted;
    uint32_t h = 2166136261u;
    uint32_t before;
    size_t i;
    int step;

    pico8_jl();
    de_my = pico8_value_function(pico8_de_my());
    bare = pico8_fq_new(NULL);
    keys.my = pico8_de_my();
    built = pico8_fq_new(&keys);
    nil_key = veil_with_key(NULL);

    h = fold_members(h, bare);
    h = fold_members(h, built);
    h = fold_members(h, nil_key);
    assert(pico8_entity_get_field(built, "my", &got) && got.kind == PICO8_VALUE_FUNCTION &&
           got.as.function == de_my.as.function);
    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        h = fold_raw(h, bare, intruders[i]);
        h = fold_raw(h, built, intruders[i]);
    }

    h = fnv(h, (uint32_t)ldown(bare));
    h = fold_members(h, bare);

    g_script[0] = pico8_value_bool(1);
    g_script[1] = pico8_value_ref(NULL);
    g_script[2] = zero_of(PICO8_VALUE_BOOL);
    g_script[3] = pico8_value_bool(0);
    g_script[4] = pico8_value_bool(1);
    g_script_len = 5;
    g_script_calls = 0;
    scripted = veil_with_key(scripted_my);
    for (step = 0; step < g_script_len; step++) {
        h = fnv(h, (uint32_t)ldown(scripted));
        h = fnv(h, (uint32_t)g_script_calls);
        h = fold_members(h, scripted);
    }
    assert(g_script_calls == 5);

    pico8_entity_set_field(bare, "m_", pico8_value_bool(1));
    h = fold_members(h, bare);
    h = fnv(h, (uint32_t)pico8_fq_struct_class()->set_field(bare, "m_", pico8_value_bool(0)));
    h = fold_members(h, bare);
    {
        pico8_value_t writes[6];

        writes[0] = zero_of(PICO8_VALUE_BOOL);
        writes[1] = pico8_value_ref(NULL);
        writes[2] = pico8_value_ref(NULL);
        writes[3] = pico8_value_bool(1);
        writes[4] = pico8_value_ref(NULL);
        writes[5] = zero_of(PICO8_VALUE_BOOL);
        for (step = 0; step < 6; step++) {
            h = fnv(h, (uint32_t)pico8_fq_struct_class()->set_field(bare, "m_", writes[step]));
            h = fold_members(h, bare);
        }
    }

    before = fold_members(2166136261u, bare);
    for (i = 0; i < sizeof unwritten / sizeof unwritten[0]; i++) {
        h = fnv(h, (uint32_t)pico8_fq_struct_class()->set_field(bare, unwritten[i], pico8_value_bool(1)));
        assert(fold_members(2166136261u, bare) == before);
    }
    h = fold_members(h, bare);
    must_abort(bare, "p_");
    must_abort(bare, "my");
    must_abort(bare, "fa");
    must_abort(bare, "hd");
    check("fq members", h, FQ_MEMBERS_HASH);
}

static void write_m_by_name(void *e, pico8_value_t v) {
    pico8_entity_set_field(e, "m_", v);
}

static void ldown_returning(void *e, pico8_value_t v) {
    void *veil;
    (void)e;
    g_script[0] = v;
    g_script_len = 1;
    g_script_calls = 0;
    veil = veil_with_key(scripted_my);
    (void)ldown(veil);
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

/* `m_` and `p_` take nil or a boolean only; a value whose whole union is
 * zero is not nil unless it is a REF. */
static void test_fq_writer_stops_off_row(void) {
    pico8_value_t one = pico8_value_fixed(pico8_fixed_from_int(1));
    static const pico8_jb_row_t plain = {0};
    void *bare;
    void *other;

    pico8_jl();
    bare = pico8_fq_new(NULL);
    other = pico8_fq_new(NULL);

    must_abort_value(bare, write_m_by_name, one);
    must_abort_value(bare, write_m_by_name, zero_of(PICO8_VALUE_FIXED));
    must_abort_value(bare, write_m_by_name, zero_of(PICO8_VALUE_STRING));
    must_abort_value(bare, write_m_by_name, pico8_value_string("x"));
    must_abort_value(bare, write_m_by_name, pico8_value_function(counting_my));
    must_abort_value(bare, write_m_by_name, zero_of(PICO8_VALUE_FUNCTION));
    must_abort_value(bare, write_m_by_name, pico8_value_ref(other));
    must_abort_value(bare, write_m_by_name, pico8_value_ref((void *)&plain));

    must_abort_value(bare, ldown_returning, one);
    must_abort_value(bare, ldown_returning, zero_of(PICO8_VALUE_FIXED));
    must_abort_value(bare, ldown_returning, zero_of(PICO8_VALUE_STRING));
    must_abort_value(bare, ldown_returning, pico8_value_string("x"));
    must_abort_value(bare, ldown_returning, pico8_value_ref(other));
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
    test_fq_members_by_name();
    test_fq_writer_stops_off_row();

    printf("test_pico8_fq: all tests passed\n");
    return 0;
}
