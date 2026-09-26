/* The trading hub `nl`, read and written only through the dual-form field
 * API and `pico8_class_get_field(pico8_nl_class()/pico8_qh_class()/
 * pico8_jo_class(), ...)`, and driven through the real `ie:ldown`/`mv`/`dg`.
 * The frozen values were taken on the code where `nl` was still a class
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
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define EMPTY "tests/fixtures/empty_planet_cleared_events.json"
#define HUB "tests/fixtures/trading_hub_session_events.json"

#define BUILT_HASH 0x8355850Bu
#define LIFE_HASH 0xC8987757u
#define HUB_FRAMES 4921
#define HUB_PLATES 1
#define HUB_HASH 0x6E90A079u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;
static pico8_cheat_t cheat;

/* nl's own members, then the 10 of `qh`, then the 31 of `jo`. */
static const char *const own[] = {"rr", "nl", "qw"};
static const char *const qh_members[] = {"hp", "px", "dq", "qh", "od", "fp", "dw", "s", "ba", "et"};
static const char *const jo_members[] = {"nu", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq",
                                         "ca", "em", "id", "p",  "ra", "oe", "hn", "ep", "jy", "nk", "fx",
                                         "lq", "fo", "jh", "p_", "kr", "cc", "by", "mi", "m_"};
static const char *const inherited_methods[] = {"hd", "kj", "ldown", "hb", "gg", "gv", "hk",
                                                "kf", "cn", "ib", "ll", "hq", "ev", "ey"};
static const char *const absent[] = {"ph", "kz", "rdown", "md", "ol", "pr", "eb"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"jo", "np", "prober"};
static const char *const nl_tables[] = {"nu", "px", "ba"};
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

static pico8_value_t class_value(pico8_class_t *cls, const char *name) {
    pico8_value_t v;
    assert(pico8_class_get_field(cls, name, &v));
    return v;
}

static void *class_ref(pico8_class_t *cls, const char *name) {
    pico8_value_t v = class_value(cls, name);
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
    return fnv(h, (uint32_t)(kw.as.ref == class_ref(pico8_jo_class(), "kw")));
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

/* The research icon whose `ke` is `gx[4]`, the trading hub. */
static void *nl_icon(void) {
    const pico8_list_t *hb = pico8_cv("hb");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i].as.ref;
        pico8_value_t ke;
        if (lookup(e, "ke", &ke) && ke.kind == PICO8_VALUE_REF && ke.as.ref == pico8_nl_class()) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the trading hub's icon, as the shop does. */
static void *build_plate(void) {
    void *icon = nl_icon();
    pico8_value_t made;

    assert(icon != NULL);
    made = call(icon, "ldown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(made.kind == PICO8_VALUE_REF && made.as.ref != NULL);
    return made.as.ref;
}

/* A plate is the only entity whose chain holds `nl`: recognized by what it
 * reads, not by its form. */
static int is_plate(const void *e) {
    pico8_value_t v;
    return e != NULL && lookup(e, "nl", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(1);
}

/* A plate as `ie:ldown` and `jo:pq` left it: its scalars, which of nl's,
 * qh's and jo's member names it has, its methods (qh's chain's very
 * functions, but its own `ly`), which default tables it shares with `nl`
 * and `jo`, and where `lt` registered it -- never in `cg.qh`. */
static uint32_t fold_plate(uint32_t h, const void *e) {
    static const char *const scalars[] = {"id", "me", "t",  "rr", "nl", "qh", "hp", "s",  "od", "qw",
                                          "dq", "fp", "dw", "mi", "hf", "hs", "fa", "ma", "ca"};
    pico8_value_t gb6;
    pico8_value_t ly;
    size_t i;

    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fold_vec(h, e, "gq");
    for (i = 0; i < sizeof own / sizeof own[0]; i++) {
        h = fnv(h, (uint32_t)has(e, own[i]));
    }
    for (i = 0; i < sizeof qh_members / sizeof qh_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, qh_members[i]));
    }
    for (i = 0; i < sizeof jo_members / sizeof jo_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, jo_members[i]));
    }
    for (i = 0; i < sizeof inherited_methods / sizeof inherited_methods[0]; i++) {
        pico8_value_t v;
        pico8_value_t want = class_value(pico8_qh_class(), inherited_methods[i]);
        assert(pico8_entity_get_field(e, inherited_methods[i], &v) && v.kind == PICO8_VALUE_FUNCTION);
        assert(want.kind == PICO8_VALUE_FUNCTION && v.as.function == want.as.function);
    }
    assert(pico8_entity_get_field(e, "ly", &ly) && ly.kind == PICO8_VALUE_FUNCTION);
    assert(ly.as.function != class_value(pico8_qh_class(), "ly").as.function);
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(e, absent[i]));
    }
    for (i = 0; i < sizeof nl_tables / sizeof nl_tables[0]; i++) {
        assert(get_ref(e, nl_tables[i]) == class_ref(pico8_nl_class(), nl_tables[i]));
        assert(get_ref(e, nl_tables[i]) != class_ref(pico8_qh_class(), nl_tables[i]));
    }
    for (i = 0; i < sizeof jo_tables / sizeof jo_tables[0]; i++) {
        assert(get_ref(e, jo_tables[i]) == class_ref(pico8_jo_class(), jo_tables[i]));
    }
    assert(get_ref(e, "kw") != NULL && get_ref(e, "kw") != class_ref(pico8_jo_class(), "kw"));
    assert(pico8_ob_table_get_index(pico8_gb(), 6, &gb6) && gb6.kind == PICO8_VALUE_REF);
    assert(get_ref(e, "hn") == gb6.as.ref);
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

/* Two plates built by the shop's `ie:ldown` on `gx[4]`. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game(EMPTY);
    a = build_plate();
    b = build_plate();
    assert(a != b && is_plate(a) && is_plate(b));
    h = fold_plate(h, a);
    h = fold_plate(h, b);
    check("built", h, BUILT_HASH);
}

static pico8_fixed_t credits(void) {
    pico8_value_t nz;
    assert(pico8_entity_get_field(pico8_fm(), "nz", &nz) && nz.kind == PICO8_VALUE_FIXED);
    return nz.as.fixed;
}

static void set_credits(int n) {
    pico8_entity_set_field(pico8_fm(), "nz", pico8_value_fixed(pico8_fixed_from_int(n)));
}

static uint32_t fold_ra_y(uint32_t h, const void *e) {
    const pico8_vec_t *ra = get_ref(e, "ra");
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what `nl:ly` left on the plate. */
static uint32_t fold_dg(uint32_t h, const void *e) {
    h = fold_ra_y(h, e);
    pico8_dg();
    h = fold_field(h, e, "t");
    h = fold_ra_y(h, e);
    return fold_field(h, e, "qw");
}

/* What `bb`'s `dh` (the `pt.nl` branch), `ib`, `gg` and `da` (qh false)
 * give on a plate. */
static uint32_t fold_economy(uint32_t h, void *e) {
    pico8_value_t ib = call(e, "ib", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_value(h, ib);
    h = fold_value(h, call(e, "gg", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fnv(h, (uint32_t)pico8_da(e));
    return fold_field(h, e, "mi");
}

/* `hq` (`bv`) in the panel then `ey` (`d_`) with the credits; `dg` in "ly"
 * with `mi`=1 (`qw` stays), then `mi` at 2 and 3 through the generic
 * writer (`qw` = 8+t%42/6); `dg` on a plate still in "kz". */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *placed;
    void *idle;
    pico8_value_t mp;
    int i;

    start_game(EMPTY);

    placed = build_plate();
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
    assert(strcmp(get(placed, "me").as.string, "ly") == 0 && credits() == pico8_fixed_from_int(100 - 20));

    h = fold_economy(h, placed);

    for (i = 0; i < 3; i++) {
        h = fold_dg(h, placed);
    }
    assert(get(placed, "qw").as.fixed == pico8_fixed_from_int(15));
    pico8_entity_set_field(placed, "mi", pico8_value_fixed(pico8_fixed_from_int(2)));
    h = fold_dg(h, placed);
    h = fold_dg(h, placed);
    pico8_entity_set_field(placed, "mi", pico8_value_fixed(pico8_fixed_from_int(3)));
    h = fold_dg(h, placed);
    assert(get(placed, "qw").as.fixed != pico8_fixed_from_int(15));
    h = fold_economy(h, placed);

    idle = build_plate();
    h = fold_field(h, idle, "me");
    h = fold_dg(h, idle);
    assert(get(idle, "qw").as.fixed == pico8_fixed_from_int(15));
    check("life", h, LIFE_HASH);
}

/* Per frame, every plate in `hv`: what the shop, `bv`, `d_`, `dg`, `nl:ly`
 * and `jo`'s functions write on it; plus the screen. */
static uint32_t fold_live(uint32_t h, const void *e) {
    h = fold_field(h, e, "id");
    h = fold_field(h, e, "me");
    h = fold_field(h, e, "t");
    h = fold_vec(h, e, "ra");
    h = fold_vec(h, e, "p");
    h = fnv(h, bool_state(e, "et"));
    h = fold_field(h, e, "et");
    h = fold_field(h, e, "mi");
    h = fold_field(h, e, "qw");
    h = fnv(h, bool_state(e, "p_"));
    h = fold_kw(h, e);
    h = fnv(h, (uint32_t)list_size(e, "oe"));
    h = fnv(h, (uint32_t)list_size(e, "jy"));
    h = fnv(h, (uint32_t)list_size(e, "cc"));
    h = fnv(h, (uint32_t)list_size(e, "by"));
    return fnv(h, (uint32_t)has(e, "m_"));
}

#define MAX_SEEN 16

/* From the title screen, as a raylib recording starts. */
static void test_hub(void) {
    uint32_t h = 2166136261u;
    void *seen[MAX_SEEN];
    int seen_count = 0;
    int frames;
    int i;

    start_from_title(HUB);
    frames = input.frame_count;
    for (i = 0; i < frames; i++) {
        const pico8_list_t *hv;
        int j;

        play_frame();
        hv = pico8_hv();
        for (j = 0; j < hv->count; j++) {
            void *e = hv->items[j].as.ref;
            int k;

            if (!is_plate(e)) {
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
    check_int("hub frames", frames, HUB_FRAMES);
    check_int("hub plates", seen_count, HUB_PLATES);
    check("hub", h, HUB_HASH);
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

static void write_rr(void *e) {
    pico8_entity_set_field(e, "rr", pico8_value_string("x"));
}

static void write_nl(void *e) {
    pico8_entity_set_field(e, "nl", pico8_value_fixed(pico8_fixed_from_int(2)));
}

static void write_hp(void *e) {
    pico8_entity_set_field(e, "hp", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void write_id(void *e) {
    pico8_entity_set_field(e, "id", pico8_value_string("s0"));
}

static void write_qw(void *e) {
    pico8_entity_set_field(e, "qw", pico8_value_fixed(pico8_fixed_from_int(9)));
}

static void build_with_unknown_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "p", vec_ref(vec_xy(1, 1)));
    (void)pico8_nl_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_nl_new(b);
}

static int is_an_nl_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_nl_struct_class();
}

/* Built by `ie:ldown` or pico8_nl_new, a plate is a struct whose descriptor
 * holds qh's very entries but its own `ly`, which nl's class no longer
 * carries; it takes exactly qh's 20 names plus `qw`, and a construction key
 * other than `id` stops the run. A bare `qh` still refuses `qw`; qh's
 * unconverted descendants stay instances. */
static void test_nl_is_a_struct(void) {
    static const char *const written[] = {"by", "cc", "ep", "fo", "fx", "hn", "jh", "jy", "kr", "kw", "lq",
                                          "m_", "ma", "mi", "nk", "oe", "p_", "ra", "p",  "et", "qw"};
    pico8_class_t *const others[] = {pico8_hj_class(), pico8_pm_class(), pico8_mx_class(), pico8_nr_class(),
                                     pico8_fr_class()};
    const pico8_struct_class_t *cls = pico8_nl_struct_class();
    const pico8_struct_class_t *qh = pico8_qh_struct_class();
    void *a;
    size_t i;
    int j;

    start_game(EMPTY);
    a = build_plate();
    assert(is_an_nl_struct(a) && listed(pico8_cg("jo"), a) && !listed(pico8_cg("qh"), a));
    for (i = 0; i < sizeof inherited_methods / sizeof inherited_methods[0]; i++) {
        int m = pico8_method_of(inherited_methods[i]);
        assert(cls->methods[m] != NULL && cls->methods[m] == qh->methods[m]);
    }
    assert(cls->methods[PICO8_METHOD_LY] != NULL && cls->methods[PICO8_METHOD_LY] != qh->methods[PICO8_METHOD_LY]);
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        size_t k;
        if (cls->methods[j] == NULL || j == PICO8_METHOD_LY) {
            continue;
        }
        for (k = 0; k < sizeof inherited_methods / sizeof inherited_methods[0] &&
                    pico8_method_of(inherited_methods[k]) != j;
             k++) {
        }
        assert(k < sizeof inherited_methods / sizeof inherited_methods[0]);
    }
    assert(class_value(pico8_nl_class(), "ly").as.function == class_value(pico8_qh_class(), "ly").as.function);

    for (i = 0; i < sizeof written / sizeof written[0]; i++) {
        pico8_value_t v;
        pico8_entity_set_field(a, written[i], pico8_value_fixed(pico8_fixed_from_int((int)i + 1)));
        assert(lookup(a, written[i], &v) && v.kind == PICO8_VALUE_FIXED &&
               v.as.fixed == pico8_fixed_from_int((int)i + 1));
    }
    must_abort(write_rr, a);
    must_abort(write_nl, a);
    must_abort(write_hp, a);
    must_abort(write_id, a);
    must_abort(write_qw, pico8_qh_new(NULL));
    must_abort(build_with_unknown_key, NULL);
    must_abort(build_with_positional_key, NULL);
    {
        void *bare = pico8_nl_new(NULL);
        assert(is_an_nl_struct(bare) && !has(bare, "id") && has(bare, "oe") && listed(pico8_cg("jo"), bare));
    }
    for (i = 0; i < sizeof others / sizeof others[0]; i++) {
        pico8_ob_table_t *b = pico8_ob_table_new();
        void *d;
        pico8_ob_table_set_name(b, "id", pico8_value_string("s99"));
        d = pico8_class_instantiate(others[i], b);
        pico8_ob_table_free(b);
        assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_INSTANCE && ((const pico8_instance_t *)d)->cls == others[i]);
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
    pid_t hub;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    hub = spawn(test_hub);
    in_fresh_child(test_built);
    in_fresh_child(test_life);
    in_fresh_child(test_nl_is_a_struct);
    reap(hub);

    printf("test_pico8_nl: all tests passed\n");
    return 0;
}
