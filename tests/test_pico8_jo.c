/* The planet `jo`, read and written only through the dual-form field API and
 * its generated defaults (pico8_game_data.h), and driven through the real `bw`/`dg`/`dc`/`mv`. Its
 * daughters are read the same way. Games are built directly: no events
 * file. */

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
#include "../src/pico8_ke.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_research.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define GALAXY_PLANETS 90
#define GALAXY_HASH 0xE70E2EE3u
#define DAUGHTERS_HASH 0x668D1EBEu
#define NEW_PLANET_HASH 0x48C32418u
#define LL_DG_CALLS 62

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

/* The 31 members a `jo` struct has after its pico8_ke_t. */
static const char *const members[] = {"nu", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq",
                                      "ca", "em", "id", "p",  "ra", "oe", "hn", "ep", "jy", "nk", "fx",
                                      "lq", "fo", "jh", "p_", "kr", "cc", "by", "mi", "m_"};
static const char *const methods[] = {"hd", "kj", "ldown", "hb", "gg", "gv", "hk", "kf", "cn", "ib", "ll"};
static const char *const absent[] = {"ev", "ph", "ly", "rdown", "hq", "ey", "md"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};
static const char *const tags[] = {"jo", "np", "prober"};

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

static void check_int(const char *label, int got, int want) {
    printf("[%s] %d\n", label, got);
    fflush(stdout);
    assert(got == want);
}

static void start_title(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    start_title();
    pico8_eu(NULL);
}

static pico8_value_t vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return pico8_value_ref(v);
}

static pico8_value_t call(void *e, const char *name, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, argc, arg0, arg1, &out));
    return out;
}

/* A bare planet as `jo(...)` and `jo:pq` left it: the scalars, which of
 * the 31 member names it has, its methods, which default tables it shares
 * with the class, and where `lt` registered it. */
static uint32_t fold_planet(uint32_t h, const void *e) {
    static const char *const scalars[] = {"me", "t", "id", "hf", "hs", "fa", "ma", "ca"};
    size_t i;

    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fold_vec(h, e, "p");
    h = fold_vec(h, e, "ra");
    h = fold_vec(h, e, "gq");
    for (i = 0; i < sizeof members / sizeof members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, members[i]));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        pico8_value_t v;
        assert(pico8_entity_get_field(e, methods[i], &v) && v.kind == PICO8_VALUE_FUNCTION);
        assert(v.as.function == pico8_jo_struct_class()->methods[pico8_method_of(methods[i])]);
    }
    for (i = 0; i < sizeof absent / sizeof absent[0]; i++) {
        assert(!has(e, absent[i]));
    }
    assert(get_ref(e, "kq") == &pico8_jo_start.kq);
    assert(get_ref(e, "gq") == &pico8_jo_start.gq);
    assert(get_ref(e, "nu") == &pico8_jo_nu_tags && get_ref(e, "kw") == pico8_jo_shared_kw() &&
           get_ref(e, "em") == &pico8_jo_em_row);
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cv(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cg(tags[i]), e));
    }
    return h;
}

/* After `eu`: the 90 planets `bw` built. */
static void test_galaxy(void) {
    uint32_t h = 2166136261u;
    const pico8_list_t *jo;
    int i;

    start_game();
    jo = pico8_cg("jo");
    for (i = 0; i < jo->count; i++) {
        h = fold_planet(h, jo->items[i].as.ref);
    }
    check_int("galaxy planets", jo->count, GALAXY_PLANETS);
    check("galaxy", h, GALAXY_HASH);
}

static uint32_t fold_live(uint32_t h, const void *e) {
    static const char *const scalars[] = {"me", "t", "ma", "fx", "mi", "kr", "ep"};
    static const char *const lists[] = {"oe", "jy", "by", "cc"};
    static const char *const refs[] = {"fo", "nk", "lq", "m_"};
    size_t i;

    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fnv(h, bool_state(e, "p_"));
    h = fold_kw(h, e);
    for (i = 0; i < sizeof lists / sizeof lists[0]; i++) {
        h = fnv(h, (uint32_t)list_size(e, lists[i]));
    }
    for (i = 0; i < sizeof refs / sizeof refs[0]; i++) {
        h = fnv(h, (uint32_t)has(e, refs[i]));
    }
    return h;
}

enum { QH, NL, OS, HJ, PM, MX, NR, FR, GA, DAUGHTERS };

static int daughter_of(const void *e) {
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_qh_struct_class()) {
        return QH;
    }
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
    if (e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
        ((const pico8_struct_header_t *)e)->cls == pico8_ga_struct_class()) {
        return GA;
    }
    return -1;
}

/* What a daughter inherits from `jo`: `ca`, `gq`, the `em` table itself,
 * and `jo`'s `gg` and `hd` called on it. */
static uint32_t fold_daughter(uint32_t h, void *d) {
    pico8_vec_t ra;
    pico8_value_t ra_val;

    h = fold_field(h, d, "ca");
    h = fold_vec(h, d, "gq");
    assert(get_ref(d, "em") == &pico8_jo_em_row);
    h = fold_value(h, call(d, "gg", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    assert(lookup(d, "ra", &ra_val) && ra_val.kind == PICO8_VALUE_REF);
    ra = *(const pico8_vec_t *)ra_val.as.ref;
    h = fold_value(h, call(d, "hd", 2, vec_ref(ra), vec_ref(ra)));
    ra = pico8_vec_add(ra, pico8_vec_v(pico8_fixed_from_int(8), pico8_fixed_from_int(8)));
    return fold_value(h, call(d, "hd", 2, vec_ref(ra), vec_ref(ra)));
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
            pico8_vec_t middle = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
            assert(made.kind == PICO8_VALUE_REF && made.as.ref != NULL);
            (void)call(made.as.ref, "hq", 2, vec_ref(pico8_vec_add(*mo_ra, middle)), vec_ref(middle));
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
        pico8_value_t v;
        const pico8_hp_t *hp;
        assert(pico8_ob_table_get_index(pico8_gb(), k, &v) && v.kind == PICO8_VALUE_REF);
        hp = v.as.ref;
        if (hp->cw != NULL && strcmp(hp->cw, cw) == 0) {
            return v.as.ref;
        }
    }
}

/* A generated planet turned by `jo:hu` into the daughter `cw` names: the
 * newest entity of `hv`. */
static void *turned(int planet, const char *cw) {
    void *p = pico8_cg("jo")->items[planet].as.ref;

    pico8_jo_hh(p);
    pico8_jo_hu(p, cw_project(cw));
    return pico8_hv()->items[pico8_hv()->count - 1].as.ref;
}

/* Each daughter by the road a game takes to it: the seven of the shop
 * bought from their icons, `pm` and `ga` turned from planets by the
 * collapse and harvest projects. */
static void test_daughters(void) {
    static const struct {
        int d;
        const pico8_entity_kind_t *(*kind)(void);
    } shop[] = {{QH, pico8_qh_kind}, {OS, pico8_os_kind}, {NR, pico8_nr_kind}, {FR, pico8_fr_kind},
                {NL, pico8_nl_kind}, {MX, pico8_mx_kind}, {HJ, pico8_hj_kind}};
    void *made[DAUGHTERS] = {NULL};
    uint32_t h = 2166136261u;
    size_t i;

    start_game();
    for (i = 0; i < sizeof shop / sizeof shop[0]; i++) {
        made[shop[i].d] = bought(shop[i].kind());
    }
    made[PM] = turned(0, "pm");
    made[GA] = turned(1, "ga");
    for (i = 0; i < DAUGHTERS; i++) {
        assert(made[i] != NULL && daughter_of(made[i]) == (int)i && listed(pico8_hv(), made[i]));
        h = fold_daughter(fnv(h, (uint32_t)i), made[i]);
    }
    check("daughters", h, DAUGHTERS_HASH);
}

static void *new_planet(const char *id, int x, int y) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    void *made;

    pico8_ob_table_set_name(b, "id", pico8_value_string(id));
    pico8_ob_table_set_name(b, "p", vec_ref(pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y))));
    pico8_ob_table_set_name(b, "ra",
                            vec_ref(pico8_vec_v(pico8_fixed_from_int(x * 128), pico8_fixed_from_int(y * 128))));
    made = pico8_jo_new(b);
    pico8_ob_table_free(b);
    return made;
}

static void test_new_planet(void) {
    uint32_t h = 2166136261u;
    void *a;

    start_title();
    a = new_planet("p900", 1, 2);
    assert(listed(pico8_hv(), a) && listed(pico8_cg("jo"), a));
    h = fold_planet(h, a);
    h = fold_live(h, a);
    check("new planet", h, NEW_PLANET_HASH);
}

/* `dg` calls `qc[qc.me]`: a planet whose `me` is "ll" runs `jo:ll` until
 * its `t` passes 60, then leaves `hv`. */
static void test_ll(void) {
    void *a;
    int calls = 0;

    start_title();
    a = new_planet("p901", 3, 4);
    pico8_dg();
    assert(bool_state(a, "p_") == 0);
    pico8_ke_ks(a, pico8_value_string("ll"));
    while (listed(pico8_hv(), a) && calls < 200) {
        pico8_dg();
        calls++;
    }
    assert(bool_state(a, "p_") == 2);
    assert(!listed(pico8_hv(), a) && !listed(pico8_cg("jo"), a));
    check_int("ll dg calls", calls, LL_DG_CALLS);
}

/* `kw` of a planet that never ran `lz` is the class's shared one: an
 * in-place write to it is seen by every such planet and by the class, until
 * the next pico8_jo_init() puts it back to `{mu=0}`. */
static void test_kw_shared(void) {
    void *a;
    void *b;
    pico8_kw_t *kw;
    pico8_value_t hp;

    start_title();
    a = new_planet("p902", 5, 6);
    b = new_planet("p903", 7, 8);
    kw = get_ref(a, "kw");
    assert(kw == pico8_jo_shared_kw() && get_ref(b, "kw") == kw);
    kw->mu = pico8_fixed_from_int(7);
    assert(((const pico8_kw_t *)get_ref(b, "kw"))->mu == pico8_fixed_from_int(7));
    assert(pico8_jo_shared_kw()->mu == pico8_fixed_from_int(7));

    assert(pico8_gb() != NULL && pico8_gb()->count > 0);
    hp = pico8_gb()->entries[0].value;
    assert(hp.kind == PICO8_VALUE_REF && hp.as.ref != NULL);
    pico8_jo_lz(a, hp.as.ref);
    assert(get_ref(a, "kw") != NULL && get_ref(a, "kw") != pico8_jo_shared_kw());
    assert(((const pico8_kw_t *)get_ref(a, "kw"))->mu == pico8_fixed_from_int(0));
    assert(get_ref(b, "kw") == pico8_jo_shared_kw());
    assert(get_ref(a, "hn") == hp.as.ref && list_size(a, "jy") >= 0);

    kw->has_qe = 1;
    kw->qe = pico8_fixed_from_int(28);
    pico8_jo_init();
    assert(get_ref(b, "kw") == pico8_jo_shared_kw());
    assert(!kw->has_qe && kw->mu == pico8_fixed_from_int(0));
}

/* `jo:hh` takes `df.ll` itself on a roll of `jo.ir()` under 0.45, and its
 * `me` for the planet's state; any other draw is a `jo.oc()` entry, and
 * `known`. */
static void test_hh_takes_ll(void) {
    char id[8];
    int took_ll = 0;
    int i;

    start_game();
    for (i = 0; i < 20 && !took_ll; i++) {
        pico8_jo_t *p;

        snprintf(id, sizeof id, "p%d", 950 + i);
        p = new_planet(id, i, i);
        pico8_jo_hh(p);
        assert(p->nk != NULL && p->ep == p->nk->d);
        took_ll = p->nk == pico8_df_lookup("ll");
        assert(strcmp(p->ke.me, took_ll ? "ll" : "known") == 0);
    }
    assert(took_ll);
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

static void write_ra(void *e) {
    pico8_entity_set_field(e, "ra", pico8_value_ref(NULL));
}

static void write_id(void *e) {
    pico8_entity_set_field(e, "id", pico8_value_string("p0"));
}

static void write_hp(void *e) {
    pico8_entity_set_field(e, "hp", pico8_value_fixed(pico8_fixed_from_int(1)));
}

static void build_with_unknown_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_name(b, "hp", pico8_value_fixed(pico8_fixed_from_int(1)));
    (void)pico8_jo_new(b);
}

static void build_with_positional_key(void *e) {
    pico8_ob_table_t *b = pico8_ob_table_new();
    (void)e;
    pico8_ob_table_set_index(b, 1, pico8_value_fixed(pico8_fixed_from_int(3)));
    (void)pico8_jo_new(b);
}

static int is_a_jo_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_jo_struct_class();
}

/* Built by `bw` or pico8_jo_new, a `jo` is a struct whose descriptor holds
 * jo's 11 dispatchers, and its class installs none of them, nor a `pq`; a
 * generic write outside the 17 measured names, or a construction key other
 * than `id`/`p`/`ra`, stops the run. */
static void test_jo_is_a_struct(void) {
    const pico8_list_t *jo;
    void *a;
    size_t i;
    int j;

    start_game();
    jo = pico8_cg("jo");
    for (j = 0; j < jo->count; j++) {
        assert(is_a_jo_struct(jo->items[j].as.ref));
    }
    for (i = 0; i < sizeof methods / sizeof methods[0]; i++) {
        assert(pico8_jo_struct_class()->methods[pico8_method_of(methods[i])] != NULL);
    }
    assert(pico8_jo_ready());
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        if (pico8_jo_struct_class()->methods[j] != NULL) {
            int k;
            for (k = 0; k < (int)(sizeof methods / sizeof methods[0]) && pico8_method_of(methods[k]) != j; k++) {
            }
            assert(k < (int)(sizeof methods / sizeof methods[0]));
        }
    }

    assert(pico8_jo_dz() == pico8_jo_dz_rows && pico8_jo_cq() == &pico8_jo_cq_seq &&
           pico8_jo_ka() == &pico8_jo_ka_seq && pico8_jo_em() == &pico8_jo_em_row);

    a = new_planet("p904", 9, 10);
    assert(is_a_jo_struct(a));
    must_abort(write_ra, a);
    must_abort(write_id, a);
    must_abort(write_hp, a);
    must_abort(build_with_unknown_key, NULL);
    must_abort(build_with_positional_key, NULL);
    {
        void *bare = pico8_jo_new(NULL);
        assert(is_a_jo_struct(bare) && !has(bare, "id") && !has(bare, "ra") && has(bare, "oe"));
    }

    (void)bought(pico8_qh_kind());
    jo = pico8_cg("qh");
    assert(jo != NULL && jo->count == 1);
    assert(pico8_type_tag_of(jo->items[0].as.ref) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)jo->items[0].as.ref)->cls == pico8_qh_struct_class());
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
    in_fresh_child(test_galaxy);
    in_fresh_child(test_daughters);
    in_fresh_child(test_new_planet);
    in_fresh_child(test_ll);
    in_fresh_child(test_kw_shared);
    in_fresh_child(test_hh_takes_ll);
    in_fresh_child(test_jo_is_a_struct);

    printf("test_pico8_jo: all tests passed\n");
    return 0;
}
