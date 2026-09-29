/* The slipgate `nr`, read and written only through the dual-form field API
 * and the generated defaults (pico8_game_data.h), built as the shop builds it (`ie:ldown` on `gx[2]`, `id`) and driven
 * through the real `mv`/`dg`/`dc`, in games started directly: no events file.
 * Only test_nr_is_a_struct looks at the form. */

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
#include "../src/pico8_economy_score.h"
#include "../src/pico8_economy_tiers.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
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


#define BUILT_HASH 0x4194DF7Bu
#define LIFE_HASH 0x6D8FAD96u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

/* nr's own `hz` and `rr`, then the 10 members of `qh`, then the 31 of
 * `jo`. */
static const char *const nr_members[] = {"hz", "rr"};
static const char *const qh_members[] = {"hp", "px", "dq", "qh", "od", "fp", "dw", "s", "ba", "et"};
static const char *const jo_members[] = {"nu", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq",
                                         "ca", "em", "id", "p",  "ra", "oe", "hn", "ep", "jy", "nk", "fx",
                                         "lq", "fo", "jh", "p_", "kr", "cc", "by", "mi", "m_"};
static const char *const methods[] = {"gg", "kf", "cn", "ib", "hq", "ey"};
static const char *const own_methods[] = {"cn"};
static const char *const absent[] = {"ol", "eb", "mm"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"nr", "np", "prober"};

/* A table member of the jo tree and the generated data it points at. */
typedef struct {
    const char *name;
    const void *data;
} member_data_t;
static const member_data_t jo_data[] = {
    {"em", &pico8_jo_em_row}, {"ka", &pico8_jo_ka_seq}, {"dz", pico8_jo_dz_rows}, {"cq", &pico8_jo_cq_seq}};
static const member_data_t nr_data[] = {{"nu", &pico8_nr_nu_tags}, {"hz", &pico8_nr_hz_seq}};
static const member_data_t qh_data[] = {{"px", &pico8_qh_px_row}, {"ba", &pico8_qh_ba_seq}};

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

/* The instance reader keeps a field written nil (`ep`: TROU-9), the struct
 * reader reads a nil member as absent: nil and absent are one state here. */
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

static pico8_value_function_t method(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_FUNCTION);
    return v.as.function;
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

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    pico8_eu(NULL);
}

static pico8_value_t vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return pico8_value_ref(v);
}

static pico8_vec_t vec_xy(int x, int y) {
    return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
}

static const pico8_struct_class_t *cls_of(const void *e) {
    return ((const pico8_struct_header_t *)e)->cls;
}

/* The members a planet of qh's tree holds: `kj`, `ev`, `hd`, `hk`, `hb`,
 * `ldown`, `gv` and the states `ll`, `ly`; no other. */
static void assert_qh_shape(const pico8_struct_class_t *c) {
    assert(c->kj != NULL && c->ev != NULL && c->hd != NULL && c->hk != NULL && c->hb != NULL && c->ldown != NULL &&
           c->gv != NULL && c->states[PICO8_STATE_LL] != NULL && c->states[PICO8_STATE_LY] != NULL);
    assert(c->has_hb == NULL && c->rdown == NULL && c->states[PICO8_STATE_PH] == NULL &&
           c->states[PICO8_STATE_KZ] == NULL && c->states[PICO8_STATE_PR] == NULL);
}

static pico8_value_t call(void *e, const char *name, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, argc, arg0, arg1, &out));
    return out;
}

/* The research icon whose `ke` is `kind`. */
static void *icon_of(const pico8_entity_kind_t *kind) {
    const pico8_list_t *hb = pico8_cv("hb");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i].as.ref;
        pico8_value_t ke;
        if (lookup(e, "ke", &ke) && ke.kind == PICO8_VALUE_REF && ke.as.ref == kind) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the icon of `kind`, as the shop does. */
static void *build(const pico8_entity_kind_t *kind) {
    void *icon = icon_of(kind);
    pico8_down_t made;

    assert(icon != NULL);
    made = cls_of(icon)->ldown(icon, NULL, NULL);
    assert(made.stop && made.held != NULL);
    return made.held;
}

static void *build_slipgate(void) {
    return build(pico8_nr_kind());
}

enum { NR, FR };

/* `hz` is never written on an entity: one whose `hz` is the very table of
 * nr's class is a slipgate, of fr's class an infraspace gate, whatever its
 * form. -1 for anything else. */
static int gate_kind(const void *e) {
    const void *hz = e == NULL ? NULL : get_ref(e, "hz");
    if (hz == NULL) {
        return -1;
    }
    if (hz == &pico8_nr_hz_seq) {
        return NR;
    }
    if (hz == &pico8_fr_hz_seq) {
        return FR;
    }
    return -1;
}

/* A slipgate as `ie:ldown` and `jo:pq` left it: its scalars, which of its
 * own, qh's and jo's names it has, its methods (qh's very functions but its
 * own `gv`, `ldown`, `hb`, `cn`), which default tables it shares with `nr`,
 * `qh` and `jo`, and where `lt` registered it -- in `cg.nr`/`np`/`prober`,
 * never in `cg.jo` or `cg.qh`. */
static uint32_t fold_slipgate(uint32_t h, const void *e) {
    static const char *const scalars[] = {"id", "me", "t",  "rr", "hp", "s",  "od", "fp", "dw",
                                          "dq", "qh", "mi", "hf", "hs", "fa", "ma", "ca"};
    static const char *const not_nr[] = {"mt", "qu", "qw", "ht", "qy", "et"};
    const pico8_struct_class_t *qh = pico8_qh_struct_class();
    size_t i;

    assert(gate_kind(e) == NR);
    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fold_vec(h, e, "gq");
    h = fnv(h, (uint32_t)((const pico8_int_seq_t *)get_ref(e, "hz"))->count);
    for (i = 0; i < sizeof nr_members / sizeof nr_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, nr_members[i]));
    }
    for (i = 0; i < sizeof qh_members / sizeof qh_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, qh_members[i]));
    }
    for (i = 0; i < sizeof jo_members / sizeof jo_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, jo_members[i]));
    }
    for (i = 0; i < sizeof not_nr / sizeof not_nr[0]; i++) {
        assert(!has(e, not_nr[i]));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        pico8_value_function_t got = method(e, methods[i]);
        size_t k;
        int own = 0;
        for (k = 0; k < sizeof own_methods / sizeof own_methods[0]; k++) {
            own |= strcmp(methods[i], own_methods[k]) == 0;
        }
        if (own) {
            assert(got == pico8_nr_struct_class()->methods[pico8_method_of(methods[i])]);
            assert(got != qh->methods[pico8_method_of(methods[i])]);
            assert(got != pico8_jo_struct_class()->methods[pico8_method_of(methods[i])]);
        } else {
            assert(got == qh->methods[pico8_method_of(methods[i])]);
        }
    }
    assert_qh_shape(cls_of(e));
    assert(cls_of(e)->kj == qh->kj && cls_of(e)->ev == qh->ev && cls_of(e)->hd == qh->hd && cls_of(e)->hk == qh->hk);
    assert(cls_of(e)->states[PICO8_STATE_LL] == qh->states[PICO8_STATE_LL] &&
           cls_of(e)->states[PICO8_STATE_LY] == qh->states[PICO8_STATE_LY]);
    assert(cls_of(e)->gv != qh->gv && cls_of(e)->ldown != qh->ldown && cls_of(e)->hb != qh->hb);
    assert(cls_of(e)->gv != pico8_jo_struct_class()->gv && cls_of(e)->ldown != pico8_jo_struct_class()->ldown &&
           cls_of(e)->hb != pico8_jo_struct_class()->hb);
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(e, absent[i]));
    }
    assert(get_ref(e, "kq") == &pico8_nr_start.kq);
    for (i = 0; i < sizeof nr_data / sizeof nr_data[0]; i++) {
        assert(get_ref(e, nr_data[i].name) == nr_data[i].data);
    }
    assert(get_ref(e, "nu") != &pico8_qh_nu_tags);
    assert(get_ref(e, "kq") != &pico8_jo_start.kq);
    for (i = 0; i < sizeof qh_data / sizeof qh_data[0]; i++) {
        assert(get_ref(e, qh_data[i].name) == qh_data[i].data);
    }
    assert(get_ref(e, "gq") == &pico8_jo_start.gq);
    for (i = 0; i < sizeof jo_data / sizeof jo_data[0]; i++) {
        assert(get_ref(e, jo_data[i].name) == jo_data[i].data);
    }
    assert(get_ref(e, "kw") != NULL && get_ref(e, "kw") != pico8_jo_shared_kw());
    assert(pico8_gb_at(1) != NULL);
    assert(get_ref(e, "hn") == pico8_gb_at(1));
    h = fnv(h, (uint32_t)list_size(e, "oe"));
    h = fnv(h, (uint32_t)list_size(e, "jy"));
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cv(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cg(tags[i]), e));
    }
    assert(listed(pico8_cg("nr"), e) && !listed(pico8_cg("jo"), e) && !listed(pico8_cg("qh"), e));
    return h;
}

/* Two slipgates built by the shop's `ie:ldown` on `gx[2]`. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game();
    a = build_slipgate();
    b = build_slipgate();
    assert(a != b && get_ref(a, "oe") != get_ref(b, "oe") && get_ref(a, "kw") != get_ref(b, "kw"));
    h = fold_slipgate(h, a);
    h = fold_slipgate(h, b);
    check("built", h, BUILT_HASH);
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = pico8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const void *e) {
    const pico8_vec_t *ra = get_ref(e, "ra");
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the slipgate. */
static uint32_t fold_dg(uint32_t h, const void *e) {
    h = fold_ra_y(h, e);
    pico8_dg();
    h = fold_field(h, e, "t");
    return fold_ra_y(h, e);
}

/* What `bb`'s `dh`, `ib`, `gg` and `da` give on a slipgate. */
static uint32_t fold_economy(uint32_t h, void *e) {
    pico8_value_t ib = call(e, "ib", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_value(h, ib);
    h = fold_value(h, call(e, "gg", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fnv(h, (uint32_t)pico8_da(e));
    return fold_field(h, e, "mi");
}

/* A list of `n` copies of `item`, for `oe`: `gv`, `ldown` and `hb` only
 * count it. */
static pico8_value_t list_of(int n, void *item) {
    pico8_list_t *l = malloc(sizeof *l);
    int i;
    pico8_list_init(l);
    for (i = 0; i < n; i++) {
        pico8_list_add(l, pico8_value_ref(item));
    }
    return pico8_value_ref(l);
}

/* Placed on screen by `hq` (`bv`) and `ey` (`d_`); `dg` on one still in
 * "kz", then in "ly"; `kj` through `dc`; `gv` against no argument, another
 * slipgate and a slipway drawn from itself; `ldown` with no link (a `jc`
 * whose `qu` is the slipgate) and with 3 (refused); `hb` with 0 to 3
 * links; an `fr`'s `hb`; `pico8_nr_new` with no key, then `p` and `ra`. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *placed;
    void *idle;
    void *fr;
    void *bare;
    pico8_value_t mp;
    pico8_down_t jc;
    pico8_value_t oe;
    const pico8_vec_t *mo_ra;
    int i;

    start_game();
    idle = build_slipgate();
    h = fold_field(h, idle, "me");
    h = fold_dg(h, idle);

    placed = build_slipgate();
    mo_ra = get_ref(pico8_mo(), "ra");
    assert(mo_ra != NULL);
    mp = vec_ref(pico8_vec_add(*mo_ra, vec_xy(12, 108)));
    (void)call(placed, "hq", 2, mp, vec_ref(vec_xy(64, 64)));
    assert(get_ref(placed, "ra") == mp.as.ref);
    set_credits(200);
    h = fold_value(h, call(placed, "ey", 1, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fold_field(h, placed, "me");
    assert(strcmp(get(placed, "me").as.string, "ly") == 0);
    h = fold_economy(h, placed);
    for (i = 0; i < 3; i++) {
        h = fold_dg(h, placed);
    }
    for (i = 0; i < 2; i++) {
        pico8_dc("kj");
        h = fnv(h, screen_hash());
    }

    h = fold_value(h, pico8_value_bool(cls_of(placed)->gv(placed, NULL).stop));
    h = fold_value(h, pico8_value_bool(cls_of(placed)->gv(placed, idle).stop));
    jc = cls_of(placed)->ldown(placed, NULL, NULL);
    assert(jc.stop && jc.held != NULL);
    assert(get_ref(jc.held, "qu") == placed);
    h = fnv(h, (uint32_t)pico8_type_tag_of(jc.held));
    h = fold_value(h, pico8_value_bool(cls_of(placed)->gv(placed, jc.held).stop));
    h = fold_value(h, pico8_value_bool(cls_of(idle)->gv(idle, jc.held).stop));

    oe = get(placed, "oe");
    for (i = 0; i <= 3; i++) {
        pico8_entity_set_field(placed, "oe", list_of(i, idle));
        cls_of(placed)->hb(placed);
        h = fold_field(h, placed, "s");
        h = fold_value(h, pico8_value_bool(cls_of(placed)->gv(placed, NULL).stop));
    }
    assert(get(placed, "s").as.fixed == pico8_fixed_from_int(168));
    jc = cls_of(placed)->ldown(placed, NULL, NULL);
    assert(jc.held == NULL);
    h = fold_value(h, pico8_value_bool(jc.stop));
    pico8_entity_set_field(placed, "oe", oe);
    cls_of(placed)->hb(placed);
    h = fold_field(h, placed, "s");
    h = fold_economy(h, placed);

    fr = build(pico8_fr_kind());
    assert(gate_kind(fr) == FR);
    h = fold_field(h, fr, "s");
    cls_of(fr)->hb(fr);
    h = fold_field(h, fr, "s");
    assert(get(fr, "s").as.fixed == pico8_fixed_from_int(78));

    bare = pico8_nr_new(NULL);
    assert(gate_kind(bare) == NR && !has(bare, "id") && has(bare, "oe") && has(bare, "kw"));
    h = fnv(h, (uint32_t)rank_in(pico8_cg("nr"), bare));
    {
        pico8_jo_keys_t keys;
        pico8_value_t p = vec_ref(vec_xy(3, 4));
        pico8_value_t ra = vec_ref(vec_xy(5, 6));
        void *with;
        keys.id = NULL;
        keys.p = p.as.ref;
        keys.ra = ra.as.ref;
        with = pico8_nr_new(&keys);
        assert(get_ref(with, "p") == p.as.ref && get_ref(with, "ra") == ra.as.ref && !has(with, "id"));
        h = fnv(h, (uint32_t)rank_in(pico8_hv(), with));
    }
    check("life", h, LIFE_HASH);
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

static void write_fixed(void *e, const char *name) {
    pico8_entity_set_field(e, name, pico8_value_fixed(pico8_fixed_from_int(9)));
}

static void write_hz(void *e) {
    write_fixed(e, "hz");
}

static void write_rr(void *e) {
    write_fixed(e, "rr");
}

static void write_mt(void *e) {
    write_fixed(e, "mt");
}

static void write_qw(void *e) {
    write_fixed(e, "qw");
}

static void write_ht(void *e) {
    write_fixed(e, "ht");
}

static int is_an_nr_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_nr_struct_class();
}

/* Built by `ie:ldown` or pico8_nr_new, a slipgate is a struct whose
 * descriptor holds qh's very entries but its own `gv`, `ldown`, `hb`, `cn`,
 * which no class of its chain installs any more; qh's reader sees the 6 defaults of nr that fall on an
 * inherited member; it takes exactly qh's 20 names plus `s`. */
static void test_nr_is_a_struct(void) {
    static const char *const inherited[] = {"nu", "kq", "od", "fp", "s", "dw"};
    pico8_value_t inherited_want[6];
    const pico8_struct_class_t *cls = pico8_nr_struct_class();
    const pico8_struct_class_t *qh = pico8_qh_struct_class();
    pico8_value_t v;
    void *a;
    void *fr;
    size_t i;
    int j;

    start_game();
    a = build_slipgate();
    assert(is_an_nr_struct(a) && cls != qh && listed(pico8_cg("nr"), a));
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        int named = 0;
        int own = 0;
        for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
            named |= pico8_method_of(methods[i]) == j;
        }
        for (i = 0; i < sizeof own_methods / sizeof own_methods[0]; i++) {
            if (pico8_method_of(own_methods[i]) == j) {
                own = 1;
            }
        }
        assert((cls->methods[j] != NULL) == named);
        if (own) {
            assert(cls->methods[j] != qh->methods[j]);
        } else {
            assert(cls->methods[j] == qh->methods[j]);
        }
    }
    assert_qh_shape(cls);
    assert(cls->kj == qh->kj && cls->ev == qh->ev && cls->hd == qh->hd && cls->hk == qh->hk);
    assert(cls->states[PICO8_STATE_LL] == qh->states[PICO8_STATE_LL] &&
           cls->states[PICO8_STATE_LY] == qh->states[PICO8_STATE_LY]);
    assert(cls->gv != qh->gv && cls->ldown != qh->ldown && cls->hb != qh->hb);
    assert(cls->gv != pico8_jo_struct_class()->gv && cls->ldown != pico8_jo_struct_class()->ldown &&
           cls->hb != pico8_jo_struct_class()->hb);
    assert(cls->field != qh->field && cls->set_field != qh->set_field);
    inherited_want[0] = pico8_value_ref((void *)&pico8_nr_nu_tags);
    inherited_want[1] = pico8_value_ref((void *)&pico8_nr_start.kq);
    inherited_want[2] = pico8_value_fixed(pico8_nr_start.od);
    inherited_want[3] = pico8_value_fixed(pico8_nr_start.fp);
    inherited_want[4] = pico8_value_fixed(pico8_nr_start.s);
    inherited_want[5] = pico8_value_fixed(pico8_nr_start.dw);
    for (i = 0; i < sizeof inherited / sizeof inherited[0]; i++) {
        pico8_value_t own;
        pico8_value_t through_qh;
        assert(cls->field(a, inherited[i], &own) && qh->field(a, inherited[i], &through_qh));
        assert(pico8_value_equal(own, through_qh) &&
               pico8_value_equal(own, inherited_want[i]));
    }
    assert(!qh->field(a, "hz", &v) && cls->field(a, "hz", &v) && !qh->field(a, "rr", &v) && cls->field(a, "rr", &v));
    assert(pico8_nr_ready() && pico8_fr_ready());

    must_abort(write_hz, a);
    must_abort(write_rr, a);
    must_abort(write_mt, a);
    must_abort(write_qw, a);
    must_abort(write_ht, a);
    {
        pico8_nr_t *bare = pico8_nr_new(NULL);
        assert(is_an_nr_struct(bare) && !has(bare, "id") && has(bare, "oe") && has(bare, "kw"));
        bare->hz = NULL;
        bare->rr = NULL;
        assert(!cls->field(bare, "hz", &v) && !cls->field(bare, "rr", &v));
    }
    fr = build(pico8_fr_kind());
    assert(pico8_type_tag_of(fr) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)fr)->cls == pico8_fr_struct_class());
    cls_of(fr)->hb(fr);
    assert(get(fr, "s").as.fixed == pico8_fixed_from_int(78));
}

/* The galaxy depends on the rnd state: run each scenario in a child forked
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
    in_fresh_child(test_life);
    in_fresh_child(test_nr_is_a_struct);

    printf("test_pico8_nr: all tests passed\n");
    return 0;
}
