/* The processor `os`, read and written only through the dual-form field API
 * and `pico8_os_default()`, `pico8_qh_default()` and `pico8_jo_default()`,
 * and driven through the real `ie:ldown`/`mv`/`dg`. The frozen values were
 * taken on the code where `os` was still a class instance and must not be
 * re-taken. */

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
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy_score.h"
#include "../src/pico8_economy_tiers.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_trade.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define EMPTY "tests/fixtures/empty_planet_cleared_events.json"
#define LONG "tests/fixtures/long_session_events.json"

/* The tag class instances carried, which the frozen hashes fold: any
 * other value moves every *_HASH. */
#define FROZEN_INSTANCE_TAG 0x9E5C1451u

#define BUILT_HASH 0x0FBFD36Bu
#define LIFE_HASH 0x56980D6Du
#define LONG_FRAMES 27273
#define LONG_PROCESSORS 1
#define LONG_HASH 0x47B58E4Eu

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

/* The 10 members of `qh`, then the 31 of `jo`; `os` has none of its own. */
static const char *const qh_members[] = {"hp", "px", "dq", "qh", "od", "fp", "dw", "s", "ba", "et"};
static const char *const jo_members[] = {"nu", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq",
                                         "ca", "em", "id", "p",  "ra", "oe", "hn", "ep", "jy", "nk", "fx",
                                         "lq", "fo", "jh", "p_", "kr", "cc", "by", "mi", "m_"};
static const char *const methods[] = {"hd", "kj", "ldown", "hb", "gg", "gv", "hk", "kf",
                                      "cn", "ib", "ll", "hq", "ev", "ey", "ly"};
static const char *const absent[] = {"ph", "kz", "rdown", "md", "ol", "pr", "eb"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"jo", "np", "prober"};
static const char *const os_tables[] = {"nu", "px"};
static const char *const jo_tables[] = {"kq", "em", "ka", "dz", "cq", "gq"};

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

static pico8_value_t class_value(int (*default_of)(const char *, pico8_value_t *), const char *name) {
    pico8_value_t v;
    assert(default_of(name, &v));
    return v;
}

static void *class_ref(int (*default_of)(const char *, pico8_value_t *), const char *name) {
    pico8_value_t v = class_value(default_of, name);
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
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

static int is_bv_target(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_bv_et_struct_class();
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
        /* The frozen hashes folded the INSTANCE tag the throwaway had. */
        if (is_bv_target(v.as.ref)) {
            return fnv(fnv(h, 0xEEEEu), FROZEN_INSTANCE_TAG);
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
    return fnv(h, (uint32_t)(kw.as.ref == class_ref(pico8_jo_default, "kw")));
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

static pico8_vec_t vec_xy(int x, int y) {
    return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
}

static pico8_value_t call(void *e, const char *name, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, argc, arg0, arg1, &out));
    return out;
}

/* The research icon whose `ke` is `gx[1]`, the processor. */
static void *os_icon(void) {
    const pico8_list_t *hb = pico8_cv("hb");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i].as.ref;
        pico8_value_t ke;
        if (lookup(e, "ke", &ke) && ke.kind == PICO8_VALUE_REF && ke.as.ref == pico8_os_kind()) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the processor's icon, as the shop does. */
static void *build_processor(void) {
    void *icon = os_icon();
    pico8_value_t made;

    assert(icon != NULL);
    made = call(icon, "ldown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(made.kind == PICO8_VALUE_REF && made.as.ref != NULL);
    return made.as.ref;
}

/* A processor is the only entity whose `px` is os's own table: recognized by
 * what it reads, not by its form. */
static int is_processor(const void *e) {
    return e != NULL && get_ref(e, "px") == class_ref(pico8_os_default, "px");
}

/* A processor as `ie:ldown` and `jo:pq` left it: its scalars (`hs` is jo's
 * name, overridden by os), which of qh's and jo's member names it has, its
 * methods (qh's chain's very functions, `ly` included), which default tables
 * it shares with `os`, `qh` and `jo`, and where `lt` registered it -- never
 * in `cg.qh`. */
static uint32_t fold_processor(uint32_t h, const void *e) {
    static const char *const scalars[] = {"id", "me", "t",  "qh", "hp", "s",  "od", "hs", "dq",
                                          "fp", "dw", "mi", "hf", "fa", "ma", "ca"};
    static const char *const not_os[] = {"rr", "nl", "qw"};
    pico8_value_t gb4;
    size_t i;

    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fold_vec(h, e, "gq");
    for (i = 0; i < sizeof qh_members / sizeof qh_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, qh_members[i]));
    }
    for (i = 0; i < sizeof jo_members / sizeof jo_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, jo_members[i]));
    }
    for (i = 0; i < sizeof not_os / sizeof not_os[0]; i++) {
        assert(!has(e, not_os[i]));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        pico8_value_t v;
        pico8_value_function_t want = pico8_qh_struct_class()->methods[pico8_method_of(methods[i])];
        assert(pico8_entity_get_field(e, methods[i], &v) && v.kind == PICO8_VALUE_FUNCTION);
        assert(want != NULL && v.as.function == want);
    }
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(e, absent[i]));
    }
    for (i = 0; i < sizeof os_tables / sizeof os_tables[0]; i++) {
        assert(get_ref(e, os_tables[i]) == class_ref(pico8_os_default, os_tables[i]));
        assert(get_ref(e, os_tables[i]) != class_ref(pico8_qh_default, os_tables[i]));
    }
    assert(get_ref(e, "ba") == class_ref(pico8_qh_default, "ba"));
    for (i = 0; i < sizeof jo_tables / sizeof jo_tables[0]; i++) {
        assert(get_ref(e, jo_tables[i]) == class_ref(pico8_jo_default, jo_tables[i]));
    }
    assert(get_ref(e, "kw") != NULL && get_ref(e, "kw") != class_ref(pico8_jo_default, "kw"));
    assert(pico8_ob_table_get_index(pico8_gb(), 4, &gb4) && gb4.kind == PICO8_VALUE_REF);
    assert(get_ref(e, "hn") == gb4.as.ref);
    h = fold_kw(h, e);
    h = fnv(h, (uint32_t)list_size(e, "oe"));
    h = fnv(h, (uint32_t)list_size(e, "jy"));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cv(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cg(tags[i]), e));
    }
    assert(!listed(pico8_cg("qh"), e));
    return h;
}

/* Two processors built by the shop's `ie:ldown` on `gx[1]`. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game(EMPTY);
    a = build_processor();
    b = build_processor();
    assert(a != b && is_processor(a) && is_processor(b));
    h = fold_processor(h, a);
    h = fold_processor(h, b);
    check("built", h, BUILT_HASH);
}

static pico8_fixed_t credits(void) {
    pico8_value_t nz;
    assert(pico8_entity_get_field(pico8_fm(), "nz", &nz) && nz.kind == PICO8_VALUE_FIXED);
    return nz.as.fixed;
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = pico8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const void *e) {
    const pico8_vec_t *ra = get_ref(e, "ra");
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the processor. */
static uint32_t fold_dg(uint32_t h, const void *e) {
    h = fold_ra_y(h, e);
    pico8_dg();
    h = fold_field(h, e, "t");
    return fold_ra_y(h, e);
}

/* What `bb`'s `dh` (no `pt.nl`, `pt.hf` false), `ib`, `gg` and `da` (qh
 * false) give on a processor. */
static uint32_t fold_economy(uint32_t h, void *e) {
    pico8_value_t ib = call(e, "ib", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_value(h, ib);
    h = fold_value(h, call(e, "gg", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fnv(h, (uint32_t)pico8_da(e));
    return fold_field(h, e, "mi");
}

/* `pt.by` holding one export from `pt` to `oz`, as `jo:ib` leaves it: a
 * trade whose `lp` starts at `pt` and ends at `oz`. */
static void export_to(void *pt, void *oz) {
    pico8_ob_table_t *o = pico8_ob_table_new();
    pico8_list_t *lp = malloc(sizeof *lp);
    pico8_list_t *by = malloc(sizeof *by);
    pico8_trade_t *t;

    pico8_list_init(lp);
    pico8_list_add(lp, pico8_value_ref(pt));
    pico8_list_add(lp, pico8_value_ref(oz));
    pico8_ob_table_set_name(o, "lp", pico8_value_ref(lp));
    t = pico8_trade_new(o);
    pico8_list_init(by);
    pico8_list_add(by, pico8_value_ref(t));
    pico8_entity_set_field(pt, "by", pico8_value_ref(by));
}

/* `db` of a processor and of a bare planet, at the same tier, each exporting
 * to the same bare planet (`hf` true): only `pt.hs` tells them apart. */
static uint32_t fold_db(uint32_t h, void *processor) {
    void *bare = pico8_jo_new(NULL);
    void *oz = pico8_jo_new(NULL);
    pico8_fixed_t os_db;
    pico8_fixed_t bare_db;

    assert(bool_state(oz, "hf") == 3 || bool_state(oz, "hf") == 2);
    pico8_entity_set_field(bare, "mi", get(processor, "mi"));
    export_to(processor, oz);
    export_to(bare, oz);
    os_db = pico8_db(processor);
    bare_db = pico8_db(bare);
    printf("[db] processor %d, bare planet %d\n", (int)os_db, (int)bare_db);
    assert(os_db != bare_db && 2 * os_db == bare_db);
    h = fnv(h, (uint32_t)os_db);
    return fnv(h, (uint32_t)bare_db);
}

/* `hq` (`bv`) in the panel then `ey` (`d_`) with the credits; `dg` in "ly"
 * (qh's `ly`, `na`); `bb`/`ib`/`gg`/`da` on it; `db` with an export to a
 * planet whose `hf` is true; `dg` on a processor still in "kz". */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *placed;
    void *idle;
    pico8_value_t mp;
    int i;

    start_game(EMPTY);

    placed = build_processor();
    mp = vec_ref(vec_xy(7680, 7552));
    (void)call(placed, "hq", 2, mp, vec_ref(vec_xy(64, 64)));
    assert(get_ref(placed, "ra") == mp.as.ref);
    h = fold_vec(h, placed, "p");
    h = fnv(h, bool_state(placed, "et"));
    h = fold_field(h, placed, "et");
    set_credits(100);
    h = fold_value(h, call(placed, "ey", 1, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fold_field(h, placed, "me");
    h = fold_field(h, placed, "t");
    h = fnv(h, (uint32_t)credits());
    assert(strcmp(get(placed, "me").as.string, "ly") == 0);

    h = fold_economy(h, placed);
    for (i = 0; i < 3; i++) {
        h = fold_dg(h, placed);
    }
    h = fold_economy(h, placed);
    h = fold_db(h, placed);

    idle = build_processor();
    h = fold_field(h, idle, "me");
    h = fold_dg(h, idle);
    check("life", h, LIFE_HASH);
}

/* Per frame, every processor in `hv`: what the shop, `bv`, `d_`, `dg`, qh's
 * `ly` and `jo`'s functions write on it, and its `hs`; plus the screen. */
static uint32_t fold_live(uint32_t h, const void *e) {
    h = fold_field(h, e, "id");
    h = fold_field(h, e, "me");
    h = fold_field(h, e, "t");
    h = fold_vec(h, e, "ra");
    h = fold_vec(h, e, "p");
    h = fnv(h, bool_state(e, "et"));
    h = fold_field(h, e, "et");
    h = fold_field(h, e, "mi");
    h = fnv(h, bool_state(e, "p_"));
    h = fold_kw(h, e);
    h = fnv(h, (uint32_t)list_size(e, "oe"));
    h = fnv(h, (uint32_t)list_size(e, "jy"));
    h = fnv(h, (uint32_t)list_size(e, "cc"));
    h = fnv(h, (uint32_t)list_size(e, "by"));
    h = fnv(h, (uint32_t)has(e, "m_"));
    return fold_field(h, e, "hs");
}

#define MAX_SEEN 16

/* From the title screen, as a raylib recording starts. */
static void test_long(void) {
    uint32_t h = 2166136261u;
    void *seen[MAX_SEEN];
    int seen_count = 0;
    int frames;
    int i;

    start_from_title(LONG);
    frames = input.frame_count;
    for (i = 0; i < frames; i++) {
        const pico8_list_t *hv;
        int j;

        play_frame();
        hv = pico8_hv();
        for (j = 0; j < hv->count; j++) {
            void *e = hv->items[j].as.ref;
            int k;

            if (!is_processor(e)) {
                continue;
            }
            for (k = 0; k < seen_count && seen[k] != e; k++) {
            }
            if (k == seen_count) {
                assert(seen_count < MAX_SEEN);
                seen[seen_count++] = e;
            }
            h = fnv(h, (uint32_t)j);
            h = fold_live(h, e);
        }
        h = fnv(h, screen_hash());
    }
    check_int("long frames", frames, LONG_FRAMES);
    check_int("long processors", seen_count, LONG_PROCESSORS);
    check("long", h, LONG_HASH);
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

static void write_qw(void *e) {
    pico8_entity_set_field(e, "qw", pico8_value_fixed(pico8_fixed_from_int(9)));
}

static void write_rr(void *e) {
    pico8_entity_set_field(e, "rr", pico8_value_string("x"));
}

static void write_hs(void *e) {
    pico8_entity_set_field(e, "hs", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void write_hp(void *e) {
    pico8_entity_set_field(e, "hp", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void write_id(void *e) {
    pico8_entity_set_field(e, "id", pico8_value_string("s0"));
}

static void build_with_unknown_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "p", vec_ref(vec_xy(1, 1)));
    (void)pico8_os_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_os_new(b);
}

static int is_an_os_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_os_struct_class();
}

/* Built by `ie:ldown` or pico8_os_new, a processor is a struct whose
 * descriptor is its own but holds qh's very entries, reader and writer: it
 * takes exactly qh's 20 names, and a construction key other than `id` stops
 * the run. */
static void test_os_is_a_struct(void) {
    static const char *const written[] = {"by", "cc", "ep", "fo", "fx", "hn", "jh", "jy", "kr", "kw",
                                          "lq", "m_", "ma", "mi", "nk", "oe", "p_", "ra", "p",  "et"};
    const pico8_struct_class_t *cls = pico8_os_struct_class();
    const pico8_struct_class_t *qh = pico8_qh_struct_class();
    void *a;
    size_t i;
    int j;

    start_game(EMPTY);
    a = build_processor();
    assert(is_an_os_struct(a) && cls != qh && listed(pico8_cg("jo"), a) && !listed(pico8_cg("qh"), a));
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        assert(cls->methods[pico8_method_of(methods[i])] != NULL);
    }
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        assert(cls->methods[j] == qh->methods[j]);
    }
    assert(cls->field == qh->field && cls->set_field == qh->set_field);

    for (i = 0; i < sizeof written / sizeof written[0]; i++) {
        pico8_value_t v;
        pico8_entity_set_field(a, written[i], pico8_value_fixed(pico8_fixed_from_int((int)i + 1)));
        assert(lookup(a, written[i], &v) && v.kind == PICO8_VALUE_FIXED &&
               v.as.fixed == pico8_fixed_from_int((int)i + 1));
    }
    must_abort(write_qw, a);
    must_abort(write_rr, a);
    must_abort(write_hs, a);
    must_abort(write_hp, a);
    must_abort(write_id, a);
    must_abort(build_with_unknown_key, NULL);
    must_abort(build_with_positional_key, NULL);
    {
        void *bare = pico8_os_new(NULL);
        assert(is_an_os_struct(bare) && !has(bare, "id") && has(bare, "oe") && listed(pico8_cg("jo"), bare));
    }
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
    pid_t replay;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    replay = spawn(test_long);
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_os_is_a_struct);
    reap(replay);

    printf("test_pico8_os: all tests passed\n");
    return 0;
}
