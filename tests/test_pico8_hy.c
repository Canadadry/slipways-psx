/* The economy `hy`, the one `fm` per game, read only through
 * pico8_entity_get_field: its construction, its five functions `nx`, `hk`,
 * `lu`, `lj`, `kt`, the cheat code on the provisional `fm` of the title
 * screen, and two short replays folded frame by frame. The frozen values
 * were taken on the code where `hy` was still a class instance and must not
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
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_cheat.h"
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"

#define TECH "tests/fixtures/tech_screen_invent_events.json"
#define GAME_END "tests/fixtures/game_end_events.json"
#define GEOHARVEST "tests/fixtures/geoharvest_burst_events.json"

#define CONSTRUCTION_HASH 0x581AC8D6u
#define NX_HASH 0x857718F9u
#define HK_HASH 0x7F5DF0B2u
#define LU_HASH 0xB07FE872u
#define LJ_HASH 0xE169419Du
#define KT_HASH 0x20EA7380u
#define TECH_REPLAY_HASH 0xA8BBE715u
#define GAME_END_REPLAY_HASH 0xA550CCF7u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

static const char *const names[] = {"nz", "ge", "jk", "ew", "invented", "lo", "yr", "po", "fu", "happiness", "hl"};
#define NAME_COUNT ((int)(sizeof names / sizeof names[0]))

/* The only lines that reach `fm` other than through pico8_entity_get_field:
 * the `self` handed to the functions of `hy`, and a write that prepares a
 * scenario. */
static pico8_hy_t *fm_self(void) {
    return pico8_fm_hy();
}

static void fm_set(const char *name, pico8_value_t v) {
    pico8_hy_t *fm = pico8_fm_hy();
    assert(v.kind == PICO8_VALUE_FIXED);
    if (strcmp(name, "nz") == 0) {
        fm->nz = v.as.fixed;
    } else if (strcmp(name, "jk") == 0) {
        fm->jk = v.as.fixed;
    } else {
        assert(strcmp(name, "yr") == 0);
        fm->yr = v.as.fixed;
    }
}

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
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

static void *get_ref(const void *e, const char *name) {
    pico8_value_t v;
    return lookup(e, name, &v) && v.kind == PICO8_VALUE_REF ? v.as.ref : NULL;
}

static pico8_fixed_t get_fixed(const void *e, const char *name) {
    pico8_value_t v;
    assert(lookup(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static const pico8_list_t *invented_of(const void *e) {
    const pico8_list_t *l = get_ref(e, "invented");
    assert(l != NULL && pico8_type_tag_of(l) == PICO8_TYPE_TAG_LIST);
    return l;
}

static uint32_t fold_string(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFu);
}

/* Kind, then the string's bytes, the number, the boolean; a list or an
 * ob_table folded through `depth` levels; any other REF by its tag. */
static uint32_t fold_value(uint32_t h, pico8_value_t v, int depth) {
    h = fnv(h, (uint32_t)v.kind);
    switch (v.kind) {
    case PICO8_VALUE_FIXED:
        return fnv(h, (uint32_t)v.as.fixed);
    case PICO8_VALUE_BOOL:
        return fnv(h, (uint32_t)v.as.boolean);
    case PICO8_VALUE_STRING:
        return fold_string(h, v.as.string);
    case PICO8_VALUE_REF:
        if (v.as.ref == NULL) {
            return fnv(h, 0xDEADu);
        }
        h = fnv(h, (uint32_t)pico8_type_tag_of(v.as.ref));
        if (depth <= 0) {
            return h;
        }
        if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST) {
            const pico8_list_t *l = v.as.ref;
            int i;
            h = fnv(h, (uint32_t)l->count);
            for (i = 0; i < l->count; i++) {
                h = fold_value(h, l->items[i], depth - 1);
            }
            return h;
        }
        if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE) {
            const pico8_ob_table_t *t = v.as.ref;
            int i;
            h = fnv(h, (uint32_t)t->count);
            for (i = 0; i < t->count; i++) {
                h = fnv(h, (uint32_t)t->entries[i].key.kind);
                if (t->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
                    h = fnv(h, (uint32_t)t->entries[i].key.index);
                } else {
                    h = fold_string(h, t->entries[i].key.name);
                }
                h = fold_value(h, t->entries[i].value, depth - 1);
            }
            return h;
        }
        return h;
    default:
        return h;
    }
}

static uint32_t fold_field(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    if (!lookup(e, name, &v)) {
        return fnv(h, 0xFFFFFFFFu);
    }
    return fold_value(h, v, 3);
}

/* The 11 names of `fm`, each with its kind and value. */
static uint32_t fold_fm(uint32_t h) {
    int n;
    for (n = 0; n < NAME_COUNT; n++) {
        h = fold_field(h, pico8_fm(), names[n]);
    }
    return h;
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

static void new_game(void) {
    pico8_value_t difficulty;
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty) && difficulty.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty.as.ref));
}

/* A game started as the net starts it: `eu()` on `bo[2]` before frame 0. */
static void start_game(const char *events) {
    pico8_input_init(&input);
    assert(pico8_input_load(&input, events));
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_cheat_init(&cheat);
    new_game();
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

static void play_all(void) {
    int i;
    for (i = 0; i < input.frame_count; i++) {
        play_frame(i);
    }
}

/* The descriptor every `iv` shares, taken off one built for that purpose;
 * it stays in `hv`. */
static const pico8_struct_class_t *iv_descriptor(void) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    const pico8_iv_t *ref;

    pico8_ob_table_set_index(b, 1, pico8_value_string("ref"));
    ref = pico8_iv_new(b);
    pico8_ob_table_free(b);
    return ref->ke.header.cls;
}

/* Every entry `hv` gained since it held `from` entries: its tag, and for an
 * `iv` the text and color it shows. */
static uint32_t fold_new_entities(uint32_t h, int from, const pico8_struct_class_t *iv_cls) {
    const pico8_list_t *hv = pico8_hv();
    int i;

    h = fnv(h, (uint32_t)(hv->count - from));
    for (i = from; i < hv->count; i++) {
        const void *e = hv->items[i].as.ref;
        h = fnv(h, (uint32_t)pico8_type_tag_of(e));
        if (pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)e)->cls == iv_cls) {
            const pico8_iv_t *iv = e;
            h = fold_value(h, iv->at1, 0);
            h = fold_value(h, iv->at2, 0);
        }
    }
    return h;
}

/* A `hy` built on its own holds the 7 names of its spec, `invented` an empty
 * list, and none of the 4 names `lj` writes; `fm` after `eu()` holds all 11.
 * Two games have two distinct `invented` lists. */
static void test_construction(void) {
    uint32_t h = 2166136261u;
    void *bare;
    const pico8_list_t *first;
    int n;

    pico8__init();
    bare = pico8_hy_new(NULL);
    for (n = 0; n < NAME_COUNT; n++) {
        assert(has(bare, names[n]) == (n < 7));
        h = fold_field(h, bare, names[n]);
    }
    assert(invented_of(bare)->count == 0);
    assert(get_fixed(bare, "nz") == pico8_fixed_from_int(100) && get_fixed(bare, "yr") == pico8_fixed_from_int(3401));

    new_game();
    for (n = 0; n < NAME_COUNT; n++) {
        assert(has(pico8_fm(), names[n]));
    }
    h = fold_fm(h);
    first = invented_of(pico8_fm());
    assert(first != invented_of(bare));

    new_game();
    assert(invented_of(pico8_fm()) != first && invented_of(pico8_fm())->count == 0);
    h = fold_fm(h);
    check("construction", h, CONSTRUCTION_HASH);
}

static uint32_t fold_lo_yr(uint32_t h) {
    h = fold_field(h, pico8_fm(), "lo");
    return fold_field(h, pico8_fm(), "yr");
}

/* `lo` accumulates; past 12 without "time compression", past 15 with it,
 * the year turns once and `lo` drops by that bound. */
static void test_nx(void) {
    uint32_t h = 2166136261u;
    pico8_fixed_t yr;

    start_game(TECH);
    yr = get_fixed(pico8_fm(), "yr");
    assert(get_fixed(pico8_fm(), "lo") == pico8_fixed_from_int(1));

    pico8_hy_nx(fm_self(), pico8_fixed_from_int(3));
    assert(get_fixed(pico8_fm(), "lo") == pico8_fixed_from_int(4) && get_fixed(pico8_fm(), "yr") == yr);
    h = fold_lo_yr(h);
    pico8_hy_nx(fm_self(), pico8_fixed_from_int(8));
    assert(get_fixed(pico8_fm(), "lo") == pico8_fixed_from_int(12) && get_fixed(pico8_fm(), "yr") == yr);
    h = fold_lo_yr(h);
    pico8_hy_nx(fm_self(), pico8_fixed_from_int(1));
    assert(get_fixed(pico8_fm(), "lo") == pico8_fixed_from_int(1));
    assert(get_fixed(pico8_fm(), "yr") == pico8_fixed_add(yr, pico8_fixed_from_int(1)));
    h = fold_fm(h);

    pico8_list_add((pico8_list_t *)invented_of(pico8_fm()), pico8_value_string("time compression"));
    pico8_hy_nx(fm_self(), pico8_fixed_from_int(14));
    assert(get_fixed(pico8_fm(), "lo") == pico8_fixed_from_int(15));
    assert(get_fixed(pico8_fm(), "yr") == pico8_fixed_add(yr, pico8_fixed_from_int(1)));
    h = fold_lo_yr(h);
    pico8_hy_nx(fm_self(), PICO8_FIXED_LIT(0x00008000, 0.5));
    assert(get_fixed(pico8_fm(), "lo") == PICO8_FIXED_LIT(0x00008000, 0.5));
    assert(get_fixed(pico8_fm(), "yr") == pico8_fixed_add(yr, pico8_fixed_from_int(2)));
    h = fold_fm(h);

    pico8_hy_nx(NULL, pico8_fixed_from_int(20));
    h = fold_fm(h);
    check("nx", h, NX_HASH);
}

/* After a game with colonies: `nz += ge`, `jk += po[r.k] + 1`, `yr + 1`,
 * and each planet of `cv.hk` shows its income. */
static void test_hk(void) {
    uint32_t h = 2166136261u;
    const pico8_struct_class_t *iv_cls;
    pico8_fixed_t nz;
    pico8_fixed_t ge;
    pico8_fixed_t jk;
    pico8_fixed_t yr;
    int from;

    start_game(GEOHARVEST);
    play_all();
    iv_cls = iv_descriptor();
    nz = get_fixed(pico8_fm(), "nz");
    ge = get_fixed(pico8_fm(), "ge");
    jk = get_fixed(pico8_fm(), "jk");
    yr = get_fixed(pico8_fm(), "yr");
    h = fold_fm(h);

    from = pico8_hv()->count;
    pico8_hy_hk(fm_self());
    assert(get_fixed(pico8_fm(), "nz") == pico8_fixed_add(nz, ge));
    assert(get_fixed(pico8_fm(), "jk") > jk);
    assert(get_fixed(pico8_fm(), "yr") == pico8_fixed_add(yr, pico8_fixed_from_int(1)));
    h = fold_fm(h);
    h = fold_new_entities(h, from, iv_cls);
    assert(pico8_hv()->count > from);

    pico8_hy_hk(NULL);
    h = fold_fm(h);
    check("hk", h, HK_HASH);
}

static uint32_t lu_step(uint32_t h, void *self, int jt, int want, const pico8_struct_class_t *iv_cls) {
    int from = pico8_hv()->count;
    int got = pico8_hy_lu(self, pico8_fixed_from_int(jt));
    assert(got == want);
    h = fnv(h, (uint32_t)got);
    h = fold_field(h, pico8_fm(), "nz");
    return fold_new_entities(h, from, iv_cls);
}

/* Short of credits: 0 and "-no money-"; 0: paid, nothing shown; a cost and
 * a payment: `nz` and the amount shown; a NULL `hy` holds no credits. */
static void test_lu(void) {
    uint32_t h = 2166136261u;
    const pico8_struct_class_t *iv_cls;
    int from;

    start_game(TECH);
    iv_cls = iv_descriptor();
    assert(get_fixed(pico8_fm(), "nz") == pico8_fixed_from_int(100));

    from = pico8_hv()->count;
    h = lu_step(h, fm_self(), 101, 0, iv_cls);
    assert(get_fixed(pico8_fm(), "nz") == pico8_fixed_from_int(100) && pico8_hv()->count == from + 1);
    assert(strcmp(((const pico8_iv_t *)pico8_hv()->items[from].as.ref)->at1.as.string, "-no money-") == 0);

    from = pico8_hv()->count;
    h = lu_step(h, fm_self(), 0, 1, iv_cls);
    assert(get_fixed(pico8_fm(), "nz") == pico8_fixed_from_int(100) && pico8_hv()->count == from);

    h = lu_step(h, fm_self(), 30, 1, iv_cls);
    assert(get_fixed(pico8_fm(), "nz") == pico8_fixed_from_int(70));
    h = lu_step(h, fm_self(), -20, 1, iv_cls);
    assert(get_fixed(pico8_fm(), "nz") == pico8_fixed_from_int(90));
    h = lu_step(h, fm_self(), 90, 1, iv_cls);
    assert(get_fixed(pico8_fm(), "nz") == pico8_fixed_from_int(0));

    h = lu_step(h, NULL, 5, 0, iv_cls);
    h = lu_step(h, NULL, 0, 1, iv_cls);
    h = lu_step(h, NULL, -3, 1, iv_cls);
    check("lu", h, LU_HASH);
}

static uint32_t fold_planets_kr(uint32_t h) {
    const pico8_list_t *jo = pico8_cg("jo");
    int i;
    for (i = 0; jo != NULL && i < jo->count; i++) {
        h = fold_field(h, jo->items[i].as.ref, "kr");
    }
    return h;
}

static const char *hl_string(void) {
    pico8_value_t v;
    assert(lookup(pico8_fm(), "hl", &v));
    if (v.kind == PICO8_VALUE_BOOL) {
        assert(v.as.boolean == 0);
        return NULL;
    }
    assert(v.kind == PICO8_VALUE_STRING);
    return v.as.string;
}

/* New `po`/`fu` each call, `ge`, `happiness`, each planet's `kr`, and `hl`:
 * false, "final score" at 3426, "bankrupt" with no income and under 3
 * credits. */
static void test_lj(void) {
    uint32_t h = 2166136261u;
    void *po;
    void *fu;

    start_game(GEOHARVEST);
    play_all();
    po = get_ref(pico8_fm(), "po");
    fu = get_ref(pico8_fm(), "fu");
    assert(po != NULL && fu != NULL);
    pico8_hy_lj(fm_self());
    assert(get_ref(pico8_fm(), "po") != po && get_ref(pico8_fm(), "fu") != fu);
    assert(hl_string() == NULL);
    h = fold_fm(h);
    h = fold_planets_kr(h);

    fm_set("yr", pico8_value_fixed(pico8_fixed_from_int(3426)));
    pico8_hy_lj(fm_self());
    assert(hl_string() != NULL && strcmp(hl_string(), "final score") == 0);
    h = fold_fm(h);

    fm_set("yr", pico8_value_fixed(pico8_fixed_from_int(3425)));
    fm_set("nz", pico8_value_fixed(pico8_fixed_from_int(2)));
    pico8_hy_lj(fm_self());
    h = fold_fm(h);
    h = fnv(h, (uint32_t)(hl_string() == NULL));

    new_game();
    fm_set("nz", pico8_value_fixed(pico8_fixed_from_int(2)));
    pico8_hy_lj(fm_self());
    assert(get_fixed(pico8_fm(), "ge") <= pico8_fixed_from_int(0));
    assert(hl_string() != NULL && strcmp(hl_string(), "bankrupt") == 0);
    h = fold_fm(h);
    fm_set("nz", pico8_value_fixed(pico8_fixed_from_int(3)));
    pico8_hy_lj(fm_self());
    assert(hl_string() == NULL);
    h = fold_fm(h);

    pico8_hy_lj(NULL);
    h = fold_fm(h);
    check("lj", h, LJ_HASH);
}

static const pico8_ob_table_t *tech(const char *px, int od) {
    pico8_ob_table_t *t = pico8_ob_table_new();
    if (px != NULL) {
        pico8_ob_table_set_name(t, "px", pico8_value_string(px));
    }
    pico8_ob_table_set_name(t, "od", pico8_value_fixed(pico8_fixed_from_int(od)));
    return t;
}

/* `jk -= gi(t)`, `px` appended (nothing without `px`), `ew` up one at each
 * threshold of `ek` while under 4, never past it. */
static void test_kt(void) {
    static const char *const pxs[] = {"t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9", "t10", "t11", "t12", "t13"};
    uint32_t h = 2166136261u;
    size_t k;

    start_game(TECH);
    fm_set("jk", pico8_value_fixed(pico8_fixed_from_int(500)));

    pico8_hy_kt(fm_self(), tech(NULL, 10));
    assert(get_fixed(pico8_fm(), "jk") == pico8_fixed_from_int(495));
    assert(invented_of(pico8_fm())->count == 0 && get_fixed(pico8_fm(), "ew") == pico8_fixed_from_int(1));
    h = fold_fm(h);

    for (k = 0; k < sizeof pxs / sizeof pxs[0]; k++) {
        pico8_hy_kt(fm_self(), tech(pxs[k], (int)k));
        assert(invented_of(pico8_fm())->count == (int)k + 1);
        h = fnv(h, (uint32_t)get_fixed(pico8_fm(), "ew"));
        h = fnv(h, (uint32_t)get_fixed(pico8_fm(), "jk"));
    }
    assert(get_fixed(pico8_fm(), "ew") == pico8_fixed_from_int(4));
    h = fold_fm(h);

    pico8_hy_kt(fm_self(), NULL);
    pico8_hy_kt(NULL, tech("t14", 3));
    assert(invented_of(pico8_fm())->count == (int)(sizeof pxs / sizeof pxs[0]));
    h = fold_fm(h);
    check("kt", h, KT_HASH);
}

/* The full A A B B A A B B on the title screen writes the provisional `fm`;
 * the `hy` of the game then starts from its own spec. */
static void test_cheat_on_provisional(void) {
    static const int presses[] = {4, 4, 5, 5, 4, 4, 5, 5};
    pico8_input_frame_t frames[16];
    pico8_input_t pad;
    void *provisional;
    int fired = 0;
    int i;

    pico8__init();
    provisional = pico8_fm();
    assert(provisional != NULL && !has(provisional, "jk") && !has(provisional, "nz"));

    memset(frames, 0, sizeof frames);
    for (i = 0; i < 8; i++) {
        frames[2 * i].dpad[presses[i]] = 1;
    }
    pad.frames = frames;
    pad.frame_count = 16;
    pico8_cheat_init(&cheat);
    for (pad.current_frame = 0; pad.current_frame < 16; pad.current_frame++) {
        fired += pico8_cheat_step(&cheat, &pad);
    }
    assert(fired == 1 && pico8_fm() == provisional);
    assert(get_fixed(pico8_fm(), "jk") == pico8_fixed_from_int(99));
    assert(get_fixed(pico8_fm(), "nz") == pico8_fixed_from_int(200));

    new_game();
    assert(get_fixed(pico8_fm(), "jk") == pico8_fixed_from_int(0));
    assert(get_fixed(pico8_fm(), "nz") == pico8_fixed_from_int(100));
}

/* Every frame: the 11 names of `fm`, `invented` by its count and last
 * element, `po`/`fu` by whether their identity changed; then the screen. */
static uint32_t replay(const char *events) {
    uint32_t h = 2166136261u;
    void *po = NULL;
    void *fu = NULL;
    int i;
    int n;

    start_game(events);
    for (i = 0; i < input.frame_count; i++) {
        play_frame(i);
        for (n = 0; n < NAME_COUNT; n++) {
            if (strcmp(names[n], "invented") == 0) {
                const pico8_list_t *l = invented_of(pico8_fm());
                h = fnv(h, (uint32_t)l->count);
                h = l->count > 0 ? fold_value(h, l->items[l->count - 1], 0) : fnv(h, 0xDEADu);
            } else if (strcmp(names[n], "po") == 0 || strcmp(names[n], "fu") == 0) {
                void **seen = names[n][0] == 'p' ? &po : &fu;
                void *now = get_ref(pico8_fm(), names[n]);
                h = fnv(h, (uint32_t)(now != *seen));
                *seen = now;
            } else {
                h = fold_field(h, pico8_fm(), names[n]);
            }
        }
    }
    return fnv(h, screen_hash());
}

static void test_tech_replay(void) {
    check("tech replay", replay(TECH), TECH_REPLAY_HASH);
}

static void test_game_end_replay(void) {
    check("game end replay", replay(GAME_END), GAME_END_REPLAY_HASH);
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

static void write_nz(void *e) {
    pico8_entity_set_field(e, "nz", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void build_with_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "x", pico8_value_bool(1));
    (void)pico8_hy_new(b);
}

static void lu_on_other(void *e) {
    (void)pico8_hy_lu(e, pico8_fixed_from_int(1));
}

static void fm_hy_on_provisional(void *e) {
    (void)e;
    (void)pico8_fm_hy();
}

/* `fm` is a struct whose descriptor has no method, a reader and no writer;
 * `hy` takes no key, its functions refuse anything but a `hy`, and the
 * provisional `fm` is never handed to them. */
static void test_hy_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_hy_struct_class();
    pico8_hy_t *bare;
    int j;

    assert(pico8_hy_ready());
    pico8__init();
    assert(pico8_type_tag_of(pico8_fm()) == PICO8_TYPE_TAG_STRUCT && ((const pico8_struct_header_t *)pico8_fm())->cls == pico8_fm_placeholder_struct_class());
    must_abort(fm_hy_on_provisional, NULL);

    new_game();
    assert(pico8_type_tag_of(pico8_fm()) == PICO8_TYPE_TAG_STRUCT);
    assert(((const pico8_struct_header_t *)pico8_fm())->cls == cls && pico8_fm_hy() == (void *)pico8_fm());
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        assert(cls->methods[j] == NULL);
    }
    assert(cls->field != NULL && cls->set_field == NULL);
    assert(!has(pico8_fm(), "hk") && !has(pico8_fm(), "x"));

    bare = pico8_hy_new(NULL);
    assert(pico8_type_tag_of(bare) == PICO8_TYPE_TAG_STRUCT && bare->header.cls == cls);

    must_abort(write_nz, pico8_fm());
    must_abort(build_with_key, NULL);
    must_abort(lu_on_other, pico8_ok());

    pico8_probe_shutdown();
    assert(pico8_fm() == NULL && pico8_fm_hy() == NULL);
}

#define WITNESS 0x1234

static void read_present(const void *e, const char *name, pico8_value_t *out) {
    const pico8_struct_class_t *cls = ((const pico8_struct_header_t *)e)->cls;
    out->kind = PICO8_VALUE_FIXED;
    out->as.fixed = WITNESS;
    assert(cls->field(e, name, out));
}

static void read_absent(const void *e, const char *name) {
    const pico8_struct_class_t *cls = ((const pico8_struct_header_t *)e)->cls;
    pico8_value_t out;
    out.kind = PICO8_VALUE_FIXED;
    out.as.fixed = WITNESS;
    assert(cls->field(e, name, &out) == 0);
    assert(out.kind == PICO8_VALUE_FIXED && out.as.fixed == WITNESS);
}

static void assert_fixed(const void *e, const char *name, int n) {
    pico8_value_t v;
    read_present(e, name, &v);
    assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(n));
}

static void *read_ref(const void *e, const char *name) {
    pico8_value_t v;
    read_present(e, name, &v);
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

static void assert_hl_false(const void *e) {
    pico8_value_t v;
    read_present(e, "hl", &v);
    assert(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

static void assert_hl_string(const void *e, const char *s) {
    pico8_value_t v;
    read_present(e, "hl", &v);
    assert(v.kind == PICO8_VALUE_STRING && v.as.string != NULL && strcmp(v.as.string, s) == 0);
}

static void cheat_sequence(void) {
    static const int presses[] = {4, 4, 5, 5, 4, 4, 5, 5};
    pico8_input_frame_t frames[16];
    pico8_input_t pad;
    int fired = 0;
    int i;

    memset(frames, 0, sizeof frames);
    for (i = 0; i < 8; i++) {
        frames[2 * i].dpad[presses[i]] = 1;
    }
    pad.frames = frames;
    pad.frame_count = 16;
    pico8_cheat_init(&cheat);
    for (pad.current_frame = 0; pad.current_frame < 16; pad.current_frame++) {
        fired += pico8_cheat_step(&cheat, &pad);
    }
    assert(fired == 1);
}

static void test_hy_members_by_name(void) {
    static const char *const unknown[] = {"x", "", "hk", "kr", "jo"};
    void *bare;
    void *po;
    void *fu;
    pico8_list_t *invented;
    int before;
    pico8_value_t v;
    size_t k;

    pico8__init();
    bare = pico8_hy_new(NULL);
    assert_fixed(bare, "nz", 100);
    assert_fixed(bare, "ge", 0);
    assert_fixed(bare, "jk", 0);
    assert_fixed(bare, "ew", 1);
    assert_fixed(bare, "lo", 1);
    assert_fixed(bare, "yr", 3401);
    invented = read_ref(bare, "invented");
    assert(pico8_type_tag_of(invented) == PICO8_TYPE_TAG_LIST && invented->count == 0);
    read_absent(bare, "po");
    read_absent(bare, "fu");
    read_absent(bare, "happiness");
    read_absent(bare, "hl");

    new_game();
    po = read_ref(pico8_fm(), "po");
    fu = read_ref(pico8_fm(), "fu");
    read_present(pico8_fm(), "happiness", &v);
    assert(v.kind == PICO8_VALUE_FIXED);
    assert_hl_false(pico8_fm());
    assert(read_ref(pico8_fm(), "invented") == read_ref(pico8_fm(), "invented"));
    for (k = 0; k < sizeof unknown / sizeof unknown[0]; k++) {
        read_absent(pico8_fm(), unknown[k]);
    }

    assert(read_ref(pico8_fm(), "po") == po && read_ref(pico8_fm(), "fu") == fu);
    pico8_hy_lj(fm_self());
    assert(read_ref(pico8_fm(), "po") != po && read_ref(pico8_fm(), "fu") != fu);
    invented = read_ref(pico8_fm(), "invented");
    before = invented->count;
    pico8_hy_kt(fm_self(), tech("t1", 0));
    assert(read_ref(pico8_fm(), "invented") == invented && invented->count == before + 1);

    fm_set("yr", pico8_value_fixed(pico8_fixed_from_int(3426)));
    pico8_hy_lj(fm_self());
    assert_hl_string(pico8_fm(), "final score");
    fm_set("yr", pico8_value_fixed(pico8_fixed_from_int(3425)));
    pico8_hy_lj(fm_self());
    assert_hl_false(pico8_fm());
    new_game();
    fm_set("nz", pico8_value_fixed(pico8_fixed_from_int(2)));
    pico8_hy_lj(fm_self());
    assert_hl_string(pico8_fm(), "bankrupt");
    fm_set("nz", pico8_value_fixed(pico8_fixed_from_int(3)));
    pico8_hy_lj(fm_self());
    assert_hl_false(pico8_fm());

    new_game();
    cheat_sequence();
    assert_fixed(pico8_fm(), "jk", 99);
    assert_fixed(pico8_fm(), "nz", 200);
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
        PICO8_MEMBER_CENSUS_CHILD_EXIT(0);
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
    in_fresh_child(test_nx);
    in_fresh_child(test_hk);
    in_fresh_child(test_lu);
    in_fresh_child(test_lj);
    in_fresh_child(test_kt);
    in_fresh_child(test_cheat_on_provisional);
    in_fresh_child(test_tech_replay);
    in_fresh_child(test_game_end_replay);
    in_fresh_child(test_hy_is_a_struct);
    in_fresh_child(test_hy_members_by_name);

    printf("test_pico8_hy: all tests passed\n");
    return 0;
}
