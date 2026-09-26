/* The score panel `pj`, built by its only road (`eu()`, `pico8_pj_new(NULL)`,
 * on a game started as the net starts it), read only through the dual-form
 * field API and `pico8_class_get_field(pico8_pj_class(), ...)` for spec
 * defaults, and driven through the real `mv`/`cr()`/`de.my`. The frozen
 * values were taken on the code where `pj` was still a class instance and
 * must not be re-taken. */

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
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_menu.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_score_panel_display.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"

#define RECORDING "tests/fixtures/game_end_events.json"

#define BUILT_HASH 0x9BE763EBu
#define SHARED_HASH 0xAC16627Bu
#define LIFE_HASH 0xDDBCC765u
#define REPLAY_HASH 0xBB8AD732u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

/* The 5 tables of pj's spec, read on the class by the cart. */
static const char *const tables[] = {"ea", "gs", "cr", "i_", "ou"};
static const char *const absent[] = {"c_", "gz", "ra", "p_", "nu", "fx", "hd", "ev"};
static const char *const methods[] = {"kj", "ph", "hb", "hk"};
static const char *const tags[] = {"jo", "g_", "qh", "jc", "mz", "mh", "np", "hj", "prober", "nr", "bg"};

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

static int count_of(const pico8_list_t *list) {
    return list == NULL ? 0 : list->count;
}

/* The instance reader keeps a field written nil (TROU-9), the struct reader
 * reads a nil member as absent: nil and absent are one state here. */
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

static pico8_fixed_t get_fixed(const void *e, const char *name) {
    pico8_value_t v;
    assert(lookup(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static void *class_ref(const char *name) {
    pico8_value_t v;
    assert(pico8_class_get_field(pico8_pj_class(), name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

/* 0 absent, 1 false, 2 true, 3 anything else. */
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

static uint32_t fold_value(uint32_t h, pico8_value_t v);

/* A table's entries, in order, nested tables included. */
static uint32_t fold_table(uint32_t h, const pico8_ob_table_t *t) {
    int i;

    assert(pico8_type_tag_of(t) == PICO8_TYPE_TAG_OB_TABLE);
    for (i = 0; i < t->count; i++) {
        const pico8_ob_key_t *key = &t->entries[i].key;
        h = key->kind == PICO8_OB_KEY_NAME ? fnv_str(h, key->name) : fnv(h, (uint32_t)key->index);
        h = fold_value(h, t->entries[i].value);
    }
    return fnv(h, (uint32_t)t->count);
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
        if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE) {
            return fold_table(fnv(h, 0x7Au), v.as.ref);
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

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
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

static pico8_value_t call(void *e, const char *name, int argc, pico8_value_t arg0) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, argc, arg0, pico8_value_ref(NULL), &out));
    return out;
}

static pico8_value_t nil(void) {
    return pico8_value_ref(NULL);
}

/* A game started as the net starts it: `eu()` on `bo[2]` before frame 0. */
static void *start_game(void) {
    pico8_value_t difficulty;

    pico8_input_init(&input);
    assert(pico8_input_load(&input, RECORDING));
    pico8_dispatch_set_input(&input);
    pico8__init();
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty) && difficulty.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty.as.ref));
    pico8_cheat_init(&cheat);
    assert(pico8_ju() != NULL);
    return pico8_ju();
}

static void play_frame(int i) {
    pico8_dispatch_set_input(&input);
    pico8_cheat_step(&cheat, &input);
    pico8__update60();
    pico8__draw();
    if (i < input.frame_count - 1) {
        pico8_input_step(&input);
    }
}

static const pico8_ob_table_t *row_of(const pico8_ob_table_t *t, int i) {
    pico8_value_t row;
    assert(pico8_ob_table_get_index(t, i, &row) && row.kind == PICO8_VALUE_REF && row.as.ref != NULL);
    return row.as.ref;
}

static const char *string_at(const pico8_ob_table_t *t, int i) {
    pico8_value_t v;
    assert(pico8_ob_table_get_index(t, i, &v) && v.kind == PICO8_VALUE_STRING);
    return v.as.string;
}

/* The 7 `ou[i][1]` as `hb` wrote them. */
static uint32_t fold_ou_heads(uint32_t h, const void *e) {
    const pico8_ob_table_t *ou = get_ref(e, "ou");
    int i;

    for (i = 1; i <= 7; i++) {
        pico8_value_t head = nil();
        (void)pico8_ob_table_get_index(row_of(ou, i), 1, &head);
        h = fold_value(h, head);
    }
    return h;
}

static int has_name(const pico8_ob_table_t *t, const char *name) {
    pico8_value_t v;
    return pico8_ob_table_get_name(t, name, &v);
}

static int has_index(const pico8_ob_table_t *t, int i) {
    pico8_value_t v;
    return pico8_ob_table_get_index(t, i, &v);
}

static int is_of(const void *e, const pico8_struct_class_t *cls) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)e)->cls == cls;
}

static int fq_count(void) {
    const pico8_list_t *hv = pico8_hv();
    int n = 0;
    int i;
    for (i = 0; i < hv->count; i++) {
        n += is_of(hv->items[i].as.ref, pico8_fq_struct_class());
    }
    return n;
}

static int js15_count(void) {
    const pico8_list_t *hv = pico8_hv();
    int n = 0;
    int i;
    for (i = 0; i < hv->count; i++) {
        const void *e = hv->items[i].as.ref;
        n += is_of(e, pico8_js_struct_class()) && get_fixed(e, "fa") == pico8_fixed_from_int(15);
    }
    return n;
}

static pico8_value_t de_my(void) {
    pico8_value_t my;
    assert(pico8_ob_table_get_name(pico8_de(), "my", &my) && my.kind == PICO8_VALUE_FUNCTION);
    return my.as.function(NULL, 0, nil(), nil());
}

/* The game's panel as `eu()` and `lt` left it: where it registered, its
 * defaults, the names only read elsewhere, its 4 methods, and the 5 tables
 * of the class, by identity and content. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *ju = start_game();
    pico8_value_t me;
    size_t i;

    h = fnv(h, (uint32_t)rank_in(pico8_hv(), ju));
    h = fnv(h, (uint32_t)pico8_hv()->count);
    assert(listed(pico8_cv("kj"), ju) && listed(pico8_cv("hk"), ju) && listed(pico8_cv("hb"), ju));
    assert(!listed(pico8_cv("ev"), ju) && !listed(pico8_cv("hd"), ju));
    h = fnv(h, (uint32_t)rank_in(pico8_cv("kj"), ju));
    h = fnv(h, (uint32_t)rank_in(pico8_cv("hk"), ju));
    h = fnv(h, (uint32_t)rank_in(pico8_cv("hb"), ju));
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        assert(!listed(pico8_cg(tags[i]), ju));
    }

    assert(get_fixed(ju, "fa") == pico8_fixed_from_int(10));
    assert(get_fixed(ju, "ed") == pico8_fixed_from_int(1));
    assert(lookup(ju, "me", &me) && me.kind == PICO8_VALUE_STRING && strcmp(me.as.string, "ph") == 0);
    h = fold_field(h, ju, "t");
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(ju, absent[i]));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        pico8_value_t v;
        assert(lookup(ju, methods[i], &v) && v.kind == PICO8_VALUE_FUNCTION);
    }
    for (i = 0; i < sizeof tables / sizeof tables[0]; i++) {
        assert(get_ref(ju, tables[i]) == class_ref(tables[i]));
        h = fold_field(h, ju, tables[i]);
    }
    check("built", h, BUILT_HASH);
}

/* A second panel shares the class's `ou`/`ea`/`cr`/`gs`/`i_` and sees what
 * `hb`/`hk` wrote into them through the first; `fa`/`ed`/`c_`/`gz` stay
 * each panel's own. */
static void test_shared(void) {
    uint32_t h = 2166136261u;
    void *ju = start_game();
    void *other = pico8_pj_new(NULL);
    const pico8_ob_table_t *ea;
    size_t i;

    assert(other != NULL && other != ju);
    for (i = 0; i < sizeof tables / sizeof tables[0]; i++) {
        assert(get_ref(other, tables[i]) == get_ref(ju, tables[i]));
    }
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), other));

    (void)call(ju, "hb", 0, nil());
    h = fold_ou_heads(h, other);
    assert(strcmp(string_at(row_of(get_ref(other, "ou"), 1), 1), string_at(row_of(get_ref(ju, "ou"), 1), 1)) ==
           0);

    (void)call(ju, "hk", 1, pico8_value_fixed(pico8_fixed_from_int(3425)));
    ea = get_ref(other, "ea");
    assert(strcmp(string_at(row_of(ea, 2), 1), "year 3425") == 0);
    assert(has_index(ea, 3) && has_name(ea, "lh"));
    h = fold_field(h, other, "ea");

    pico8_entity_set_field(pico8_fm(), "hl", pico8_value_string("final score"));
    (void)call(ju, "ph", 0, nil());
    assert(get_fixed(ju, "fa") == pico8_fixed_from_int(13));
    assert(bool_state(ju, "c_") == 2);
    assert(get_fixed(other, "fa") == pico8_fixed_from_int(10));
    assert(get_fixed(other, "ed") == pico8_fixed_from_int(1));
    assert(!has(other, "c_") && !has(other, "gz"));
    h = fold_field(h, other, "fa");
    h = fold_field(h, other, "ed");
    h = fold_field(h, ju, "gz");
    check("shared", h, SHARED_HASH);
}

/* `hb`; `hk` on a year with a message and two without; `cr()`; `kj` closed;
 * the game's end through `ph`, twice; `kj` open; `de.my` twice. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *ju = start_game();
    const pico8_ob_table_t *ea = get_ref(ju, "ea");
    const pico8_ob_table_t *cr = get_ref(ju, "cr");
    static const int years[] = {3425, 3400, 3401};
    pico8_value_t ep;
    int fq_before;
    int js_before;
    int boxes;
    size_t i;

    (void)call(ju, "hb", 0, nil());
    h = fold_ou_heads(h, ju);

    for (i = 0; i < sizeof years / sizeof years[0]; i++) {
        (void)call(ju, "hk", 1, pico8_value_fixed(pico8_fixed_from_int(years[i])));
        h = fnv(h, (uint32_t)has_index(ea, 3));
        h = fnv(h, (uint32_t)has_name(ea, "lh"));
        h = fnv_str(h, string_at(row_of(ea, 2), 1));
        h = fnv(h, (uint32_t)count_of(pico8_cg("mh")));
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), pico8_kc()));
    }
    assert(has_index(ea, 3) == 0 && has_name(ea, "lh"));

    pico8_cr();
    assert(get_ref(ju, "cr") == cr && cr == class_ref("cr"));
    assert(pico8_ob_table_get_name(cr, "ep", &ep) && ep.kind == PICO8_VALUE_STRING);
    h = fnv_str(h, ep.as.string);

    for (i = 0; i < 2; i++) {
        (void)call(ju, "kj", 0, nil());
        h = fnv(h, screen_hash());
    }
    assert(!has(ju, "gz"));

    fq_before = fq_count();
    js_before = js15_count();
    pico8_entity_set_field(pico8_fm(), "hl", pico8_value_string("final score"));
    (void)call(ju, "ph", 0, nil());
    assert(bool_state(ju, "c_") == 2);
    assert(get_fixed(ju, "gz") == 0);
    assert(get_fixed(ju, "fa") == pico8_fixed_from_int(13));
    assert(fq_count() == fq_before + 1 && js15_count() == js_before + 2);
    assert(get_ref(ju, "ed") == pico8_kc() && pico8_kc() != NULL);
    boxes = count_of(pico8_cg("mh"));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), pico8_kc()));
    h = fnv(h, (uint32_t)pico8_hv()->count);

    (void)call(ju, "ph", 0, nil());
    assert(count_of(pico8_cg("mh")) == boxes);
    assert(get_ref(ju, "ed") == pico8_kc());

    for (i = 0; i < 3; i++) {
        (void)call(ju, "kj", 0, nil());
        h = fold_field(h, ju, "gz");
        h = fnv(h, screen_hash());
    }
    assert(get_fixed(ju, "gz") == PICO8_FIXED_LIT(0x00009999, 0.6));

    assert(pico8_value_equal(de_my(), pico8_value_bool(1)));
    assert(bool_state(ju, "c_") == 1);
    assert(pico8_value_equal(de_my(), pico8_value_bool(1)));
    assert(bool_state(ju, "c_") == 2);
    h = fold_field(h, ju, "fa");
    check("life", h, LIFE_HASH);
}

/* game_end_events.json from the start to its last frame: per frame the
 * panel's rank and `t`, its `c_`, `gz`, `fa`, and whether `ed` is `kc`;
 * then the screen and `ou`. */
static void test_replay(void) {
    uint32_t h = 2166136261u;
    void *ju = start_game();
    int i;

    for (i = 0; i < input.frame_count; i++) {
        play_frame(i);
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), ju));
        h = fold_field(h, ju, "t");
        h = fold_field(h, ju, "c_");
        h = fold_field(h, ju, "gz");
        h = fold_field(h, ju, "fa");
        h = fnv(h, (uint32_t)(get_ref(ju, "ed") == pico8_kc() && pico8_kc() != NULL));
    }
    assert(bool_state(ju, "c_") == 2 && get_fixed(ju, "fa") == pico8_fixed_from_int(13));
    h = fnv(h, screen_hash());
    h = fold_field(h, ju, "ou");
    check("replay", h, REPLAY_HASH);
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

static void write_c_(void *e) {
    pico8_entity_set_field(e, "c_", pico8_value_bool(1));
}

static void build_with_named_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "fa", pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_pj_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_pj_new(b);
}

/* The game's panel is a struct whose descriptor holds exactly `hb`, `kj`,
 * `hk`, `ph` and no writer; it takes no construction key. Its class keeps
 * only its spec, whose 5 tables are the shared state. */
static void test_pj_is_a_struct(void) {
    static const char *const spec[] = {"fa", "ea", "gs", "cr", "i_", "ou", "ed"};
    const pico8_struct_class_t *cls = pico8_pj_struct_class();
    pico8_pj_t *ju = start_game();
    pico8_value_t v;
    size_t i;
    int j;

    assert(is_of(ju, cls));
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        if (j == PICO8_METHOD_HB || j == PICO8_METHOD_KJ || j == PICO8_METHOD_HK || j == PICO8_METHOD_PH) {
            assert(cls->methods[j] != NULL);
        } else {
            assert(cls->methods[j] == NULL);
        }
    }
    assert(cls->field != NULL && cls->set_field == NULL);
    assert(pico8_pj_class()->struct_new != NULL && pico8_pj_class()->pq == NULL);
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        assert(!pico8_ob_table_get_name(pico8_pj_class()->defaults, methods[i], &v));
    }
    for (i = 0; i < sizeof spec / sizeof spec[0]; i++) {
        assert(pico8_ob_table_get_name(pico8_pj_class()->defaults, spec[i], &v));
    }
    assert(pico8_pj_ou() == class_ref("ou") && pico8_pj_ea() == class_ref("ea") && pico8_pj_cr() == class_ref("cr"));
    assert(pico8_pj_i_() == class_ref("i_") && pico8_pj_gs() == class_ref("gs"));
    assert(ju->gs.as.ref == class_ref("gs") && ju->i_.as.ref == class_ref("i_"));

    must_abort(write_c_, ju);
    must_abort(build_with_named_key, NULL);
    must_abort(build_with_positional_key, NULL);
}

/* Each scenario runs in a child forked right after pico8_cart_load(), as a
 * fresh process would start it: the rnd state is the same for all. */
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
    in_fresh_child(test_built);
    in_fresh_child(test_shared);
    in_fresh_child(test_life);
    in_fresh_child(test_replay);
    in_fresh_child(test_pj_is_a_struct);

    printf("test_pico8_pj: all tests passed\n");
    return 0;
}
