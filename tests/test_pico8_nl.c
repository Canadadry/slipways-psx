/* The trading hub `nl`, read and written only through the dual-form field
 * API and the generated defaults (pico8_game_data.h), and driven through the real `ie:ldown`/`mv`/`dg`, in
 * games built directly: no events file. */

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


#define BUILT_HASH 0x8355850Bu
#define LIFE_HASH 0xC8987757u
#define NL_MEMBERS_HASH 0xEDD234A8u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

/* nl's own members, then the 10 of `qh`, then the 31 of `jo`. */
static const char *const own[] = {"rr", "nl", "qw"};
static const char *const qh_members[] = {"hp", "px", "dq", "qh", "od", "fp", "dw", "s", "ba", "et"};
static const char *const jo_members[] = {"nu", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq",
                                         "ca", "em", "id", "p",  "ra", "oe", "hn", "ep", "jy", "nk", "fx",
                                         "lq", "fo", "jh", "p_", "kr", "cc", "by", "mi", "m_"};
static const char *const absent[] = {"ol", "eb"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"jo", "np", "prober"};

/* A table member of the jo tree and the generated data it points at. */
typedef struct {
    const char *name;
    const void *data;
} member_data_t;
static const member_data_t jo_data[] = {
    {"em", &pico8_jo_em_row}, {"ka", &pico8_jo_ka_seq}, {"dz", pico8_jo_dz_rows}, {"cq", &pico8_jo_cq_seq}};
static const member_data_t nl_data[] = {
    {"nu", &pico8_nl_nu_tags}, {"px", &pico8_nl_px_row}, {"ba", &pico8_nl_ba_seq}};
static const member_data_t qh_data[] = {
    {"nu", &pico8_qh_nu_tags}, {"px", &pico8_qh_px_row}, {"ba", &pico8_qh_ba_seq}};

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
    const pico8_kw_t *k;
    if (!lookup(e, "kw", &kw)) {
        return fnv(h, 0xA0u);
    }
    assert(kw.kind == PICO8_VALUE_REF && pico8_type_tag_of(kw.as.ref) == PICO8_TYPE_TAG_UNKNOWN);
    k = kw.as.ref;
    h = k->has_qe ? fold_value(fnv(h, 1u), pico8_value_fixed(k->qe)) : fnv(h, 0u);
    h = fold_value(fnv(h, 1u), pico8_value_fixed(k->mu));
    return fnv(h, (uint32_t)(kw.as.ref == pico8_jo_shared_kw()));
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
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
 * `ldown`, `gv`, `hq`, `ey`, `kf`, `cn` and the states `ll`, `ly`; `mm`
 * aside, no other. */
static void assert_qh_shape(const pico8_struct_class_t *c) {
    assert(c->kj != NULL && c->ev != NULL && c->hd != NULL && c->hk != NULL && c->hb != NULL && c->ldown != NULL &&
           c->gv != NULL && c->states[PICO8_STATE_LL] != NULL && c->states[PICO8_STATE_LY] != NULL);
    assert(c->has_hb == NULL && c->rdown == NULL && c->states[PICO8_STATE_PH] == NULL &&
           c->states[PICO8_STATE_KZ] == NULL && c->states[PICO8_STATE_PR] == NULL);
    assert(c->hq != NULL && c->ey != NULL && c->kf != NULL && c->cn != NULL && c->ol == NULL);
}

/* `c` holds `ref`'s very members but `kf`, `cn` and `mm`, `kj` and the
 * state `ly` included when `with_kj`/`with_ly`. */
static void assert_members_of(const pico8_struct_class_t *c, const pico8_struct_class_t *ref, int with_kj,
                              int with_ly) {
    assert(c->ev == ref->ev && c->hd == ref->hd && c->hk == ref->hk && c->hb == ref->hb && c->has_hb == ref->has_hb &&
           c->ldown == ref->ldown && c->rdown == ref->rdown && c->gv == ref->gv);
    assert(c->states[PICO8_STATE_LL] == ref->states[PICO8_STATE_LL] && c->hq == ref->hq && c->ey == ref->ey);
    assert(!with_kj || c->kj == ref->kj);
    assert(!with_ly || c->states[PICO8_STATE_LY] == ref->states[PICO8_STATE_LY]);
}

/* The research icon whose `ke` is `gx[4]`, the trading hub. */
static void *nl_icon(void) {
    const pico8_list_t *hb = pico8_cv("hb");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i].as.ref;
        pico8_value_t ke;
        if (lookup(e, "ke", &ke) && ke.kind == PICO8_VALUE_REF && ke.as.ref == pico8_nl_kind()) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the trading hub's icon, as the shop does. */
static void *build_plate(void) {
    void *icon = nl_icon();
    pico8_down_t made;

    assert(icon != NULL);
    made = cls_of(icon)->ldown(icon, NULL, NULL);
    assert(made.stop && made.held != NULL);
    return made.held;
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
    assert(!pico8_entity_get_field(e, "ly", &ly) && !pico8_entity_get_field(e, "hq", &ly) && pico8_jo_is_family(e));
    assert_qh_shape(cls_of(e));
    assert_members_of(cls_of(e), pico8_qh_struct_class(), 1, 0);
    assert(cls_of(e)->kf == pico8_qh_struct_class()->kf && cls_of(e)->cn == pico8_qh_struct_class()->cn &&
           cls_of(e)->mm == NULL);
    assert(cls_of(e)->states[PICO8_STATE_LY] != pico8_qh_struct_class()->states[PICO8_STATE_LY]);
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(e, absent[i]));
    }
    for (i = 0; i < sizeof nl_data / sizeof nl_data[0]; i++) {
        assert(get_ref(e, nl_data[i].name) == nl_data[i].data);
        assert(get_ref(e, qh_data[i].name) != qh_data[i].data);
    }
    assert(get_ref(e, "kq") == &pico8_jo_start.kq);
    assert(get_ref(e, "gq") == &pico8_jo_start.gq);
    for (i = 0; i < sizeof jo_data / sizeof jo_data[0]; i++) {
        assert(get_ref(e, jo_data[i].name) == jo_data[i].data);
    }
    assert(get_ref(e, "kw") != NULL && get_ref(e, "kw") != pico8_jo_shared_kw());
    assert(pico8_gb_at(6) != NULL);
    assert(get_ref(e, "hn") == pico8_gb_at(6));
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

    start_game();
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
    pico8_fm_hy()->nz = pico8_fixed_from_int(n);
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
    pico8_value_t ib = pico8_value_ref(pico8_jo_ib(e));
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_value(h, ib);
    h = fold_value(h, pico8_jo_gg(e));
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

    start_game();

    placed = build_plate();
    mp = vec_ref(vec_xy(7680, 7552));
    cls_of(placed)->hq(placed, mp.as.ref, vec_ref(vec_xy(64, 64)).as.ref);
    assert(get_ref(placed, "ra") == mp.as.ref);
    h = fold_vec(h, placed, "p");
    h = fnv(h, bool_state(placed, "et"));
    h = fold_field(h, placed, "et");
    set_credits(100);
    assert(cls_of(placed)->ey == pico8_d__dispatch);
    h = fold_value(h, pico8_d_(placed));
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
    pico8_jo_keys_t keys;
    (void)e;
    keys.id = NULL;
    keys.p = vec_ref(vec_xy(1, 1)).as.ref;
    keys.ra = NULL;
    (void)pico8_nl_new(&keys);
}

static int is_an_nl_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_nl_struct_class();
}

/* Built by `ie:ldown` or pico8_nl_new, a plate is a struct whose descriptor
 * holds qh's very entries but its own `ly`, which nl's class no longer
 * carries; it takes exactly qh's 20 names plus `qw`, and a construction key
 * other than `id` stops the run. A bare `qh` still refuses `qw`. */
static void test_nl_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_nl_struct_class();
    const pico8_struct_class_t *qh = pico8_qh_struct_class();
    void *a;

    start_game();
    a = build_plate();
    assert(is_an_nl_struct(a) && listed(pico8_cg("jo"), a) && !listed(pico8_cg("qh"), a));
    assert_qh_shape(cls);
    assert_members_of(cls, qh, 1, 0);
    assert(cls->kf == qh->kf && cls->cn == qh->cn && cls->mm == NULL);
    assert(cls->states[PICO8_STATE_LY] != qh->states[PICO8_STATE_LY]);

    must_abort(write_rr, a);
    must_abort(write_nl, a);
    must_abort(write_hp, a);
    must_abort(write_id, a);
    must_abort(write_qw, pico8_qh_new(NULL));
    must_abort(build_with_unknown_key, NULL);
    {
        void *bare = pico8_nl_new(NULL);
        assert(is_an_nl_struct(bare) && !has(bare, "id") && has(bare, "oe") && listed(pico8_cg("jo"), bare));
    }
}

/* The descriptor's own reader and pico8_entity_get_field agree on each of
 * nl's names; a name the reader refuses leaves the witness untouched. */
static uint32_t fold_raw(uint32_t h, const void *e, const char *name) {
    const pico8_struct_class_t *cls = pico8_nl_struct_class();
    pico8_value_t raw = pico8_value_fixed(0x1234);
    int found = cls->field(e, name, &raw);
    pico8_value_t by_name;
    int by_name_found = lookup(e, name, &by_name);

    assert(found == by_name_found);
    if (!found) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
        return fnv(h, 0u);
    }
    assert(raw.kind == by_name.kind);
    assert(raw.kind != PICO8_VALUE_STRING || raw.as.string == by_name.as.string);
    return fold_value(fnv(h, 1u), raw);
}

static uint32_t fold_own(uint32_t h, const void *e) {
    size_t i;
    for (i = 0; i < sizeof own / sizeof own[0]; i++) {
        h = fold_raw(h, e, own[i]);
    }
    return h;
}

static void assert_spec_rr(const void *e) {
    assert(get(e, "rr").kind == PICO8_VALUE_STRING &&
           get(e, "rr").as.string == pico8_nl_start.rr);
}

static void test_nl_members_by_name(void) {
    static const char *const intruders[] = {"", "q", "qw ", "QW", "rr_", "nl_"};
    const pico8_struct_class_t *cls = pico8_nl_struct_class();
    uint32_t h = 2166136261u;
    pico8_value_t zero_fixed;
    pico8_value_t v;
    void *bare;
    void *plate;
    int i;
    int m;

    start_game();
    bare = pico8_nl_new(NULL);
    plate = build_plate();
    h = fold_own(h, bare);
    h = fold_own(h, plate);
    assert_spec_rr(bare);
    assert_spec_rr(plate);

    for (m = 1; m <= 3; m++) {
        pico8_entity_set_field(bare, "mi", pico8_value_fixed(pico8_fixed_from_int(m)));
        for (i = 0; i < 3; i++) {
            cls_of(bare)->states[PICO8_STATE_LY](bare, pico8_fixed_from_int(5 + 17 * i + m));
            h = fold_raw(h, bare, "qw");
        }
    }

    pico8_entity_set_field(plate, "qw", pico8_value_fixed(pico8_fixed_from_int(9)));
    assert(lookup(plate, "qw", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_fixed_from_int(9));
    h = fold_raw(h, plate, "qw");
    memset(&zero_fixed, 0, sizeof zero_fixed);
    zero_fixed.kind = PICO8_VALUE_FIXED;
    pico8_entity_set_field(plate, "qw", zero_fixed);
    assert(lookup(plate, "qw", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0);
    h = fold_raw(h, plate, "qw");
    assert(cls->set_field(plate, "qw", pico8_value_fixed(pico8_fixed_from_int(4))) == 1);
    assert(cls->set_field(plate, "rr", pico8_value_string("x")) == 0);
    assert(cls->set_field(plate, "nl", pico8_value_fixed(pico8_fixed_from_int(2))) == 0);
    h = fold_own(h, plate);
    assert_spec_rr(plate);

    for (i = 0; i < (int)(sizeof intruders / sizeof intruders[0]); i++) {
        pico8_value_t raw = pico8_value_fixed(0x1234);
        assert(cls->field(plate, intruders[i], &raw) == 0);
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == 0x1234);
    }

    check("nl members", h, NL_MEMBERS_HASH);
}

static void write_qw_value(void *e, pico8_value_t v) {
    pico8_entity_set_field(e, "qw", v);
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

/* `qw` takes a fixed only, nil included in what stops; a value whose whole
 * union is zero is not nil unless it is a REF. */
static void test_nl_writer_stops_off_row(void) {
    pico8_value_t zero_false;
    pico8_value_t null_string;
    void *a;

    start_game();
    a = pico8_nl_new(NULL);
    memset(&zero_false, 0, sizeof zero_false);
    zero_false.kind = PICO8_VALUE_BOOL;
    memset(&null_string, 0, sizeof null_string);
    null_string.kind = PICO8_VALUE_STRING;
    must_abort_value(a, write_qw_value, pico8_value_ref(NULL));
    must_abort_value(a, write_qw_value, pico8_value_bool(1));
    must_abort_value(a, write_qw_value, pico8_value_bool(0));
    must_abort_value(a, write_qw_value, zero_false);
    must_abort_value(a, write_qw_value, pico8_value_string("x"));
    must_abort_value(a, write_qw_value, null_string);
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
    in_fresh_child(test_nl_is_a_struct);
    in_fresh_child(test_nl_members_by_name);
    in_fresh_child(test_nl_writer_stops_off_row);

    printf("test_pico8_nl: all tests passed\n");
    return 0;
}
