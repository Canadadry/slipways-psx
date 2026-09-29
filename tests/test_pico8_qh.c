/* The laboratory `qh`, read and written only through the dual-form field API
 * and the generated defaults (pico8_game_data.h), and driven through the
 * real `ie:ldown`/`mv`/`dg`. Its unconverted descendants are read the same
 * way. Games are started directly: no events file. */

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


/* The tag class instances carried, which the frozen hashes fold: any
 * other value moves every *_HASH. */
#define FROZEN_INSTANCE_TAG 0x9E5C1451u

#define BUILT_HASH 0xA67C1FC6u
#define LIFE_HASH 0x0ECCCC0Au
#define DESCENDANTS_HASH 0x19708F66u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

/* qh's own members, then the 31 of `jo`. */
static const char *const own[] = {"hp", "px", "dq", "qh", "od", "fp", "dw", "s", "ba", "et"};
static const char *const jo_members[] = {"nu", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq",
                                         "ca", "em", "id", "p",  "ra", "oe", "hn", "ep", "jy", "nk", "fx",
                                         "lq", "fo", "jh", "p_", "kr", "cc", "by", "mi", "m_"};
static const char *const methods[] = {"hd", "kj", "ldown", "hb", "gg", "gv", "hk", "kf",
                                      "cn", "ib", "ll", "hq", "ev", "ey", "ly"};
static const char *const absent[] = {"ph", "kz", "rdown", "md", "ol", "pr", "eb"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"qh", "jo", "np", "prober"};

/* A table member of the jo tree and the generated data it points at. */
typedef struct {
    const char *name;
    const void *data;
} member_data_t;
static const member_data_t jo_data[] = {
    {"em", &pico8_jo_em_row}, {"ka", &pico8_jo_ka_seq}, {"dz", pico8_jo_dz_rows}, {"cq", &pico8_jo_cq_seq}};
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

static pico8_value_t call(void *e, const char *name, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, argc, arg0, arg1, &out));
    return out;
}

/* The research icon whose `ke` is `gx[0]`, the laboratory. */
static void *qh_icon(void) {
    const pico8_list_t *hb = pico8_cv("hb");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i].as.ref;
        pico8_value_t ke;
        if (lookup(e, "ke", &ke) && ke.kind == PICO8_VALUE_REF && ke.as.ref == pico8_qh_kind()) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the laboratory's icon, as the shop does. */
static void *build_lab(void) {
    void *icon = qh_icon();
    pico8_value_t made;

    assert(icon != NULL);
    made = call(icon, "ldown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(made.kind == PICO8_VALUE_REF && made.as.ref != NULL);
    return made.as.ref;
}

/* A laboratory as `ie:ldown` and `jo:pq` left it: its scalars, which of
 * qh's and jo's member names it has, its methods (the very functions of
 * the class chain), which default tables it shares with `qh` and `jo`, and
 * where `lt` registered it. */
static uint32_t fold_lab(uint32_t h, const void *e) {
    static const char *const scalars[] = {"id", "me", "t",  "hp", "dq", "qh", "od", "fp",
                                          "dw", "mi", "s",  "hf", "hs", "fa", "ma", "ca"};
    size_t i;

    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fold_vec(h, e, "gq");
    for (i = 0; i < sizeof own / sizeof own[0]; i++) {
        h = fnv(h, (uint32_t)has(e, own[i]));
    }
    for (i = 0; i < sizeof jo_members / sizeof jo_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, jo_members[i]));
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
    for (i = 0; i < sizeof qh_data / sizeof qh_data[0]; i++) {
        assert(get_ref(e, qh_data[i].name) == qh_data[i].data);
    }
    assert(get_ref(e, "kq") == &pico8_jo_start.kq);
    assert(get_ref(e, "gq") == &pico8_jo_start.gq);
    for (i = 0; i < sizeof jo_data / sizeof jo_data[0]; i++) {
        assert(get_ref(e, jo_data[i].name) == jo_data[i].data);
    }
    assert(get_ref(e, "kw") != NULL && get_ref(e, "kw") != pico8_jo_shared_kw());
    assert(pico8_gb_at(1) != NULL);
    assert(get_ref(e, "hn") == pico8_gb_at(1));
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
    return h;
}

/* Two laboratories built by the shop's `ie:ldown` on `gx[0]`. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game();
    a = build_lab();
    b = build_lab();
    assert(a != b);
    h = fold_lab(h, a);
    h = fold_lab(h, b);
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

/* What `bv` left on a laboratory: `ra` (the very REF passed), `p`, and
 * what `et` is. */
static uint32_t fold_held(uint32_t h, const void *e, pico8_value_t mp) {
    assert(get_ref(e, "ra") == mp.as.ref);
    h = fold_vec(h, e, "p");
    h = fnv(h, bool_state(e, "et"));
    return fold_field(h, e, "et");
}

static uint32_t fold_ra_y(uint32_t h, const void *e) {
    const pico8_vec_t *ra = get_ref(e, "ra");
    assert(ra != NULL);
    return fnv(h, (uint32_t)ra->y);
}

/* `hq` (`bv`) at an empty point, in the panel and out of it, then next to
 * a planet; `ey` (`d_`) with and without the credits; `dg` on a laboratory
 * in "ly" (`qh.ly` = `na`) and on one still in "kz". */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *out_lab;
    void *in_lab;
    void *poor_lab;
    void *planet;
    pico8_value_t mp;
    pico8_value_t planet_p;
    pico8_value_t ra_before;
    pico8_fixed_t before;
    pico8_fixed_t in_y;
    int i;

    start_game();

    out_lab = build_lab();
    mp = vec_ref(vec_xy(7680, 7680));
    (void)call(out_lab, "hq", 2, mp, vec_ref(vec_xy(0, 0)));
    h = fold_held(h, out_lab, mp);
    assert(bool_state(out_lab, "et") == 3 && !listed(pico8_hv(), get_ref(out_lab, "et")));

    planet = pico8_cg("jo")->items[0].as.ref;
    assert(lookup(planet, "p", &planet_p));
    mp = vec_ref(pico8_vec_add(pico8_vec_mul(*(const pico8_vec_t *)planet_p.as.ref, pico8_fixed_from_int(128)),
                               vec_xy(6, 0)));
    (void)call(out_lab, "hq", 2, mp, vec_ref(vec_xy(64, 64)));
    h = fold_held(h, out_lab, mp);
    assert(get_ref(out_lab, "et") == planet && get_ref(planet, "fo") == out_lab);

    poor_lab = build_lab();
    mp = vec_ref(vec_xy(7680, 7680));
    (void)call(poor_lab, "hq", 2, mp, vec_ref(vec_xy(64, 64)));
    h = fold_held(h, poor_lab, mp);
    assert(bool_state(poor_lab, "et") == 1);
    set_credits(3);
    h = fold_value(h, call(poor_lab, "ey", 1, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fold_field(h, poor_lab, "me");
    h = fold_field(h, poor_lab, "t");
    h = fnv(h, bool_state(poor_lab, "p_"));
    h = fnv(h, (uint32_t)credits());
    assert(bool_state(poor_lab, "p_") == 2 && credits() == pico8_fixed_from_int(3));

    in_lab = build_lab();
    mp = vec_ref(vec_xy(7680, 7552));
    (void)call(in_lab, "hq", 2, mp, vec_ref(vec_xy(64, 64)));
    set_credits(100);
    h = fold_value(h, call(in_lab, "ey", 1, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fold_field(h, in_lab, "me");
    h = fold_field(h, in_lab, "t");
    h = fnv(h, bool_state(in_lab, "p_"));
    h = fnv(h, (uint32_t)credits());
    assert(strcmp(get(in_lab, "me").as.string, "ly") == 0 && credits() == pico8_fixed_from_int(100 - 15));
    in_y = ((const pico8_vec_t *)get_ref(in_lab, "ra"))->y;

    ra_before = get(out_lab, "ra");
    before = ((const pico8_vec_t *)ra_before.as.ref)->y;
    for (i = 0; i < 4; i++) {
        h = fold_ra_y(h, in_lab);
        pico8_dg();
        h = fold_field(h, in_lab, "t");
        assert(get_ref(out_lab, "ra") == ra_before.as.ref && ((const pico8_vec_t *)ra_before.as.ref)->y == before);
    }
    h = fold_ra_y(h, in_lab);
    assert(((const pico8_vec_t *)get_ref(in_lab, "ra"))->y != in_y);
    h = fnv(h, (uint32_t)listed(pico8_hv(), poor_lab));
    check("life", h, LIFE_HASH);
}

enum { NL, OS, HJ, PM, MX, NR, FR, DESCENDANTS };

static int descendant_of(const void *e) {
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_nl_struct_class()) {
        return NL;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_os_struct_class()) {
        return OS;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_hj_struct_class()) {
        return HJ;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_pm_struct_class()) {
        return PM;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_mx_struct_class()) {
        return MX;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_nr_struct_class()) {
        return NR;
    }
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_fr_struct_class()) {
        return FR;
    }
    return -1;
}

/* What a descendant inherits from `qh`: `dq`, `fp`, `dw`, `hf`, whether
 * its `nu`/`px`/`ba` are qh's own tables, `hq`/`ev`/`ey` (qh's functions),
 * and its `ly` called once (`ra.y` before, after, then put back). */
static uint32_t fold_descendant(uint32_t h, void *d) {
    static const char *const inherited[] = {"dq", "fp", "dw", "hf", "mi"};
    static const char *const fns[] = {"hq", "ev", "ey"};
    size_t i;
    pico8_value_t ra;
    pico8_value_t qw;
    int had_qw;

    for (i = 0; i < sizeof inherited / sizeof inherited[0]; i++) {
        h = fold_field(h, d, inherited[i]);
    }
    for (i = 0; i < sizeof qh_data / sizeof qh_data[0]; i++) {
        h = fnv(h, (uint32_t)(get_ref(d, qh_data[i].name) == qh_data[i].data));
    }
    for (i = 0; i < sizeof fns / sizeof fns[0]; i++) {
        pico8_value_t v;
        assert(lookup(d, fns[i], &v) && v.kind == PICO8_VALUE_FUNCTION);
        assert(v.as.function == pico8_qh_struct_class()->methods[pico8_method_of(fns[i])]);
    }
    had_qw = lookup(d, "qw", &qw);
    if (lookup(d, "ra", &ra)) {
        pico8_vec_t *v = ra.as.ref;
        pico8_fixed_t y = v->y;
        h = fnv(fnv(h, 1u), (uint32_t)y);
        (void)call(d, "ly", 1, pico8_value_fixed(pico8_fixed_from_int(50)), pico8_value_ref(NULL));
        h = fnv(h, (uint32_t)v->y);
        v->y = y;
    } else {
        (void)call(d, "ly", 1, pico8_value_fixed(pico8_fixed_from_int(50)), pico8_value_ref(NULL));
        h = fnv(h, 0u);
    }
    h = fold_field(h, d, "qw");
    if (had_qw) {
        pico8_entity_set_field(d, "qw", qw);
    }
    return h;
}

/* The shop's `ie:ldown` on the icon of `kind`, as the player buys it, then
 * dragged by `hq` to the middle of the screen. */
static void *bought(const pico8_entity_kind_t *kind) {
    const pico8_list_t *hb = pico8_cv("hb");
    const pico8_vec_t *mo_ra = get_ref(pico8_mo(), "ra");
    int i;

    assert(mo_ra != NULL);
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *icon = hb->items[i].as.ref;
        if (get_ref(icon, "ke") == kind) {
            pico8_value_t made = call(icon, "ldown", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
            assert(made.kind == PICO8_VALUE_REF && made.as.ref != NULL);
            (void)call(made.as.ref, "hq", 2, vec_ref(pico8_vec_add(*mo_ra, vec_xy(64, 64))), vec_ref(vec_xy(64, 64)));
            return made.as.ref;
        }
    }
    assert(0 && "no icon for that kind");
    return NULL;
}

/* The project of `gb` whose `cw` names `cw`. */
static pico8_hp_t *cw_project(const char *cw) {
    int k;

    for (k = 1;; k++) {
        pico8_hp_t *hp = pico8_gb_at(k);
        assert(hp != NULL);
        if (hp->cw != NULL && strcmp(hp->cw, cw) == 0) {
            return hp;
        }
    }
}

/* Each descendant by the road a game takes to it: the six of the shop
 * bought from their icons, `pm` turned from a generated planet by `jo:hu`
 * on the collapse project (the newest entity of `hv`). */
static void test_descendants(void) {
    static const struct {
        int d;
        const pico8_entity_kind_t *(*kind)(void);
    } shop[] = {{OS, pico8_os_kind}, {NR, pico8_nr_kind}, {FR, pico8_fr_kind},
                {NL, pico8_nl_kind}, {MX, pico8_mx_kind}, {HJ, pico8_hj_kind}};
    void *made[DESCENDANTS] = {NULL};
    uint32_t h = 2166136261u;
    void *p;
    size_t i;

    start_game();
    for (i = 0; i < sizeof shop / sizeof shop[0]; i++) {
        made[shop[i].d] = bought(shop[i].kind());
    }
    p = pico8_cg("jo")->items[0].as.ref;
    pico8_jo_hh(p);
    pico8_jo_hu(p, cw_project("pm"));
    made[PM] = pico8_hv()->items[pico8_hv()->count - 1].as.ref;
    for (i = 0; i < DESCENDANTS; i++) {
        assert(made[i] != NULL && descendant_of(made[i]) == (int)i && listed(pico8_hv(), made[i]));
        h = fold_descendant(fnv(h, (uint32_t)i), made[i]);
    }
    check("descendants", h, DESCENDANTS_HASH);
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

static void write_hp(void *e) {
    pico8_entity_set_field(e, "hp", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void write_id(void *e) {
    pico8_entity_set_field(e, "id", pico8_value_string("s0"));
}

static void write_s(void *e) {
    pico8_entity_set_field(e, "s", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void write_od(void *e) {
    pico8_entity_set_field(e, "od", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void write_et(void *e) {
    pico8_entity_set_field(e, "et", pico8_value_bool(0));
}

static void build_with_unknown_key(void *e) {
    pico8_jo_keys_t keys;
    (void)e;
    keys.id = NULL;
    keys.p = vec_ref(vec_xy(1, 1)).as.ref;
    keys.ra = NULL;
    (void)pico8_qh_new(&keys);
}

static int is_a_qh_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_qh_struct_class();
}

/* Built by `ie:ldown` or pico8_qh_new, a `qh` is a struct whose descriptor
 * holds jo's very dispatchers and qh's own `hq`, `ev`, `ey`, `ly`;
 * it takes exactly the 17 names of `jo` plus `ra`, `p`, `et`, and a
 * construction key other than `id` stops the run. A bare `jo` still
 * refuses `et`; every descendant is a struct under its own descriptor. */
static void test_qh_is_a_struct(void) {
    static const char *const jo_methods[] = {"hd", "kj", "ldown", "hb", "gg", "gv", "hk", "kf", "cn", "ib", "ll"};
    const pico8_entity_kind_t *const descendants[] = {pico8_nl_kind(), pico8_os_kind(), pico8_hj_kind(), pico8_pm_kind(),
                                                      pico8_mx_kind(), pico8_nr_kind(), pico8_fr_kind()};
    const pico8_struct_class_t *cls = pico8_qh_struct_class();
    void *a;
    size_t i;
    int j;

    start_game();
    a = build_lab();
    assert(is_a_qh_struct(a) && listed(pico8_cg("qh"), a));
    for (i = 0; i < sizeof jo_methods / sizeof jo_methods[0]; i++) {
        int m = pico8_method_of(jo_methods[i]);
        assert(cls->methods[m] != NULL && cls->methods[m] == pico8_jo_struct_class()->methods[m]);
    }
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        int k;
        if (cls->methods[j] == NULL) {
            continue;
        }
        for (k = 0; k < (int)(sizeof methods / sizeof methods[0]) && pico8_method_of(methods[k]) != j; k++) {
        }
        assert(k < (int)(sizeof methods / sizeof methods[0]));
    }

    must_abort(write_hp, a);
    must_abort(write_id, a);
    must_abort(write_s, a);
    must_abort(write_od, a);
    must_abort(write_et, pico8_jo_new(NULL));
    must_abort(build_with_unknown_key, NULL);
    {
        void *bare = pico8_qh_new(NULL);
        assert(is_a_qh_struct(bare) && !has(bare, "id") && has(bare, "oe") && listed(pico8_cg("qh"), bare));
    }
    for (i = 0; i < sizeof descendants / sizeof descendants[0]; i++) {
        pico8_jo_keys_t keys;
        void *d;
        keys.id = "s99";
        keys.p = NULL;
        keys.ra = NULL;
        d = descendants[i]->new_entity(&keys);
        if (descendants[i] == pico8_nl_kind()) {
            assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_STRUCT &&
                   ((const pico8_struct_header_t *)d)->cls == pico8_nl_struct_class());
            continue;
        }
        if (descendants[i] == pico8_os_kind()) {
            assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_STRUCT &&
                   ((const pico8_struct_header_t *)d)->cls == pico8_os_struct_class());
            continue;
        }
        if (descendants[i] == pico8_hj_kind()) {
            assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_STRUCT &&
                   ((const pico8_struct_header_t *)d)->cls == pico8_hj_struct_class());
            continue;
        }
        if (descendants[i] == pico8_pm_kind()) {
            assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_STRUCT &&
                   ((const pico8_struct_header_t *)d)->cls == pico8_pm_struct_class());
            continue;
        }
        if (descendants[i] == pico8_mx_kind()) {
            assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_STRUCT &&
                   ((const pico8_struct_header_t *)d)->cls == pico8_mx_struct_class());
            continue;
        }
        if (descendants[i] == pico8_nr_kind()) {
            assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_STRUCT &&
                   ((const pico8_struct_header_t *)d)->cls == pico8_nr_struct_class());
            continue;
        }
        assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_STRUCT &&
               ((const pico8_struct_header_t *)d)->cls == pico8_fr_struct_class());
    }
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
    in_fresh_child(test_descendants);
    in_fresh_child(test_qh_is_a_struct);

    printf("test_pico8_qh: all tests passed\n");
    return 0;
}
