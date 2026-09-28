/* The particle burst `ga`, read and written only through the dual-form field
 * API and the generated defaults (pico8_game_data.h), built by its three roads (`jo:hu`
 * on the harvest recipe, `pm:pq`, `pico8_ga_new(NULL)`) and driven through
 * the real `mv`/`dc`. jo's methods are named by `pico8_jo_struct_class()`.
 * No events file. */

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
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_particle.h"
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


#define BUILT_HASH 0xEA365129u
#define LIFE_HASH 0x85D7319Eu
#define GA_MEMBERS_HASH 0xF15FD5ADu
#define BURST_FRAMES 86

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

/* ga's 11 own names, then the 31 members of `jo`. */
static const char *const ga_own[] = {"fz", "fc", "qo", "ro", "qg", "q_", "k_", "iu", "py", "ms", "qy"};
static const char *const jo_members[] = {"nu", "hf", "hs", "fa", "kq", "ma", "ka", "dz", "cq", "kw", "gq",
                                         "ca", "em", "id", "p",  "ra", "oe", "hn", "ep", "jy", "nk", "fx",
                                         "lq", "fo", "jh", "p_", "kr", "cc", "by", "mi", "m_"};
static const char *const inherited[] = {"hd", "ldown", "hb", "gg", "gv", "hk", "kf", "cn", "ib", "ll"};
static const char *const own_methods[] = {"kj", "mm"};
static const char *const absent_methods[] = {"ol", "ph", "ev", "md", "hq", "ey", "kz", "pr", "rdown", "eb", "ly"};
static const char *const not_ga[] = {"hp", "jy", "kr", "qu", "ht", "et", "ng", "du"};
static const char *const tags[] = {"jo", "np", "prober", "hj", "qh"};
static const char *const bm[] = {"kj", "ev", "hd", "hk", "hb"};

/* A table member of the jo tree and the generated data it points at. */
typedef struct {
    const char *name;
    const void *data;
} member_data_t;
static const member_data_t jo_data[] = {{"kw", NULL},
    {"em", &pico8_jo_em_row}, {"ka", &pico8_jo_ka_seq}, {"dz", pico8_jo_dz_rows}, {"cq", &pico8_jo_cq_seq}};

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

static pico8_value_function_t method(const void *e, const char *name) {
    pico8_value_t v;
    assert(pico8_entity_get_field(e, name, &v) && v.kind == PICO8_VALUE_FUNCTION);
    return v.as.function;
}

static pico8_value_function_t jo_method(const char *name) {
    return pico8_jo_struct_class()->methods[pico8_method_of(name)];
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

/* The item count of a list or ob_table field, -1 when absent. */
static int size_of(const void *e, const char *name) {
    pico8_value_t v;
    if (!lookup(e, name, &v)) {
        return -1;
    }
    assert(v.kind == PICO8_VALUE_REF);
    if (strcmp(name, "k_") == 0) {
        return ((const pico8_int_seq_t *)v.as.ref)->count;
    }
    if (strcmp(name, "nu") == 0) {
        return ((const pico8_str_seq_t *)v.as.ref)->count;
    }
    if (pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST) {
        return ((const pico8_list_t *)v.as.ref)->count;
    }
    assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE);
    return ((const pico8_ob_table_t *)v.as.ref)->count;
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

static pico8_vec_t near_mo(int x, int y) {
    const pico8_vec_t *mo_ra = get_ref(pico8_mo(), "ra");
    assert(mo_ra != NULL);
    return pico8_vec_add(*mo_ra, vec_xy(x, y));
}

static pico8_value_t call(void *e, const char *name, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t out = pico8_value_ref(NULL);
    assert(pico8_mv(e, name, argc, arg0, arg1, &out));
    return out;
}

/* `fz` is "ga" on a burst and on nothing else, whatever its form. */
static int is_ga(const void *e) {
    pico8_value_t fz;
    return e != NULL && lookup(e, "fz", &fz) && fz.kind == PICO8_VALUE_STRING && strcmp(fz.as.string, "ga") == 0;
}

/* The first burst of `hv` at rank `from` or later, NULL when none. */
static void *ga_from(int from) {
    const pico8_list_t *hv = pico8_hv();
    int i;
    for (i = from; i < hv->count; i++) {
        if (is_ga(hv->items[i].as.ref)) {
            return hv->items[i].as.ref;
        }
    }
    return NULL;
}

/* The recipe of `gb` whose `cw` names "ga": the harvest. */
static void *harvest_recipe(void) {
    const pico8_ob_table_t *gb = pico8_gb();
    void *found = NULL;
    int i;

    for (i = 0; i < gb->count; i++) {
        pico8_value_t cw;
        void *hp = gb->entries[i].value.as.ref;
        if (gb->entries[i].value.kind == PICO8_VALUE_REF && hp != NULL && lookup(hp, "cw", &cw) &&
            cw.kind == PICO8_VALUE_STRING && strcmp(cw.as.string, "ga") == 0) {
            assert(found == NULL || found == hp);
            found = hp;
        }
    }
    assert(found != NULL);
    return found;
}

/* Road (a): a bare planet on `id`/`p`/`ra`, harvested by `jo:hu`; the burst
 * is the one `hv` gained. */
static void *harvest(int x, int y, const char *id) {
    pico8_jo_keys_t keys;
    void *planet;
    void *made;
    int before = pico8_hv()->count;

    keys.id = id;
    keys.p = vec_ref(vec_xy(x / 2, y / 2)).as.ref;
    keys.ra = vec_ref(near_mo(x, y)).as.ref;
    planet = pico8_jo_new(&keys);
    assert(!is_ga(planet) && pico8_hv()->count == before + 1);
    pico8_jo_hu(planet, harvest_recipe());
    made = ga_from(before + 1);
    assert(made != NULL && pico8_hv()->count == before + 2 && ga_from(0) == made);
    assert(get_ref(made, "ra") == get_ref(planet, "ra") && get_ref(made, "p") == get_ref(planet, "p"));
    return made;
}

/* Road (b): `kv["pm"]` on `p`/`ra`/`id`; its burst is registered just
 * before the protostar. */
static void *spawn(int x, int y, const char *id, void **protostar) {
    pico8_jo_keys_t keys;
    void *pm;
    int at;

    keys.id = id;
    keys.p = vec_ref(vec_xy(x / 2, y / 2)).as.ref;
    keys.ra = vec_ref(near_mo(x, y)).as.ref;
    pm = pico8_pm_new(&keys);
    at = rank_in(pico8_hv(), pm);
    assert(at >= 1 && is_ga(pico8_hv()->items[at - 1].as.ref));
    if (protostar != NULL) {
        *protostar = pm;
    }
    return pico8_hv()->items[at - 1].as.ref;
}

/* A burst as its road and `ga:pq`, `jo:pq`, `ke:pq` left it: its scalars,
 * which of its own and jo's names it has, its 12 methods (jo's very
 * functions but `kj` and `mm`), which default tables it shares with `ga`
 * and `jo`, where `lt` registered it -- in `hv` alone -- and `mo.ma`. */
static uint32_t fold_burst(uint32_t h, const void *e, int from_pm) {
    static const char *const scalars[] = {"fz", "fa", "fc", "qo", "ro", "qg", "q_", "iu",
                                          "py", "ms", "hf", "me", "t",  "id", "ma", "p_"};
    size_t i;
    int j;

    assert(is_ga(e));
    for (i = 0; i < sizeof scalars / sizeof scalars[0]; i++) {
        h = fold_field(h, e, scalars[i]);
    }
    h = fold_vec(h, e, "ra");
    h = fold_vec(h, e, "p");
    h = fnv(h, (uint32_t)size_of(e, "k_"));
    h = fnv(h, (uint32_t)size_of(e, "nu"));
    h = fnv(h, (uint32_t)size_of(e, "by"));
    h = fnv(h, (uint32_t)size_of(e, "qy"));
    h = fnv(h, (uint32_t)size_of(e, "oe"));
    h = fold_field(h, pico8_mo(), "ma");
    for (i = 0; i < sizeof ga_own / sizeof ga_own[0]; i++) {
        h = fnv(h, (uint32_t)has(e, ga_own[i]));
    }
    for (i = 0; i < sizeof jo_members / sizeof jo_members[0]; i++) {
        h = fnv(h, (uint32_t)has(e, jo_members[i]));
    }
    for (i = 0; i < sizeof not_ga / sizeof not_ga[0]; i++) {
        assert(!has(e, not_ga[i]));
    }
    for (i = 0; i < sizeof inherited / sizeof inherited[0]; i++) {
        assert(method(e, inherited[i]) == jo_method(inherited[i]));
    }
    for (i = 0; i < sizeof own_methods / sizeof own_methods[0]; i++) {
        assert(method(e, own_methods[i]) != jo_method(own_methods[i]));
    }
    for (i = 0; i < sizeof absent_methods / sizeof absent_methods[0]; i++) {
        assert(!has(e, absent_methods[i]));
    }
    assert(get_ref(e, "nu") == &pico8_ga_nu_tags);
    assert(get_ref(e, "kq") == &pico8_ga_start.kq);
    assert(pico8_type_tag_of(get_ref(e, "by")) == PICO8_TYPE_TAG_LIST);
    if (from_pm) {
        assert(get_ref(e, "k_") != &pico8_ga_k__seq);
        assert(get_ref(e, "k_") != &pico8_pm_k__seq);
    } else {
        assert(get_ref(e, "k_") == &pico8_ga_k__seq);
    }
    assert(get_ref(e, "gq") == &pico8_jo_start.gq);
    for (i = 0; i < sizeof jo_data / sizeof jo_data[0]; i++) {
        assert(get_ref(e, jo_data[i].name) == (jo_data[i].data != NULL ? jo_data[i].data : pico8_jo_shared_kw()));
    }
    h = fnv(h, (uint32_t)rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)rank_in(pico8_cv(bm[i]), e));
    }
    for (j = 0; j < (int)(sizeof tags / sizeof tags[0]); j++) {
        assert(!listed(pico8_cg(tags[j]), e));
    }
    return h;
}

/* One burst by each road, the first two on screen. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;
    void *c;

    start_game();
    assert(ga_from(0) == NULL);
    a = harvest(40, 40, "s61");
    h = fold_burst(h, a, 0);
    b = spawn(80, 70, "s62", NULL);
    h = fold_burst(h, b, 1);
    c = pico8_ga_new(NULL);
    h = fold_burst(h, c, 0);
    assert(a != b && b != c && a != c);
    assert(get_ref(a, "qy") != get_ref(b, "qy") && get_ref(b, "qy") != get_ref(c, "qy"));
    assert(get_ref(a, "oe") != get_ref(b, "oe"));
    assert(get_ref(a, "by") == get_ref(b, "by") && get_ref(b, "by") == get_ref(c, "by"));
    assert(!has(c, "ra") && !has(c, "p") && !has(c, "id"));
    assert(get(a, "fa").as.fixed == pico8_fixed_from_int(3) && get(a, "ms").as.fixed == pico8_fixed_from_int(3));
    assert(pico8_value_equal(get(a, "hf"), pico8_value_bool(0)));
    assert(get(b, "ms").as.fixed == PICO8_FIXED_LIT(0x00018000, 1.5));
    check("built", h, BUILT_HASH);
}

static uint32_t fold_particle(uint32_t h, pico8_value_t pt) {
    static const char *const vecs[] = {"p", "v", "a"};
    pico8_value_t v;
    size_t i;

    assert(pt.kind == PICO8_VALUE_REF && pt.as.ref != NULL && pico8_type_tag_of(pt.as.ref) == PICO8_TYPE_TAG_OB_TABLE);
    for (i = 0; i < sizeof vecs / sizeof vecs[0]; i++) {
        const pico8_vec_t *p;
        assert(pico8_ob_table_get_name(pt.as.ref, vecs[i], &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
        p = v.as.ref;
        h = fnv(fnv(h, (uint32_t)p->x), (uint32_t)p->y);
    }
    assert(pico8_ob_table_get_name(pt.as.ref, "l", &v) && v.kind == PICO8_VALUE_FIXED);
    return fnv(h, (uint32_t)v.as.fixed);
}

/* A table's entries, in order, nested tables one level down. */
static uint32_t fold_table(uint32_t h, const pico8_ob_table_t *t, int depth) {
    int i;

    for (i = 0; i < t->count; i++) {
        const pico8_ob_key_t *key = &t->entries[i].key;
        pico8_value_t v = t->entries[i].value;
        h = key->kind == PICO8_OB_KEY_NAME ? fnv_str(h, key->name) : fnv(h, (uint32_t)key->index);
        if (depth > 0 && v.kind == PICO8_VALUE_REF && v.as.ref != NULL &&
            pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE) {
            h = fold_table(h, v.as.ref, depth - 1);
        } else {
            h = fold_value(h, v);
        }
    }
    return fnv(h, (uint32_t)t->count);
}

static pico8_ob_table_t *row_view(const void *lq) {
    assert(pico8_type_tag_of(lq) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)lq)->cls == pico8_lq_struct_class());
    return pico8_lq_table_view(lq);
}

/* The row `jo:ex` built into `lq`, through its table view. */
static uint32_t fold_lq(uint32_t h, const void *e) {
    const void *lq = get_ref(e, "lq");
    pico8_ob_table_t *view;

    assert(lq != NULL);
    view = row_view(lq);
    h = fold_table(h, view, 1);
    pico8_ob_table_free(view);
    return h;
}

/* `kj` on `e` with its `qy` cut in place to `n` particles: `p_` after. */
static uint32_t fold_cut(uint32_t h, void *e, int n) {
    pico8_list_t *qy = get_ref(e, "qy");

    assert(qy != NULL && qy->count >= n);
    qy->count = n;
    (void)call(e, "kj", 1, get(e, "ra"), pico8_value_ref(NULL));
    h = fold_field(h, e, "p_");
    return fnv(h, (uint32_t)qy->count);
}

/* `mm` on each burst; `dc("kj")` three times, then three more with the
 * protostar burst cut to 3 particles, so that its circles, drawn in the
 * class `k_` (TROU-21), are not all covered by `ei`'s dots in its own; `p_` once `qy` holds 3, 2, 0 particles; `hd` on and off the burst,
 * `gg`; `hb` on jo's `kw` (no row), then on one with a `qe`, before and
 * after two routes land in the shared `by`; `hk` with no `kr`. */
static void test_life(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;
    void *c;
    void *pm;
    pico8_list_t *by;
    static pico8_kw_t kw;
    uint32_t row;
    int hv_count;
    int i;

    start_game();
    a = harvest(40, 40, "s71");
    b = spawn(84, 60, "s72", &pm);
    c = pico8_ga_new(NULL);
    h = fold_particle(h, call(a, "mm", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fold_particle(h, call(b, "mm", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    h = fold_particle(h, call(c, "mm", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    for (i = 0; i < 3; i++) {
        pico8_dc("kj");
        h = fnv(h, screen_hash());
        h = fold_field(h, a, "p_");
        h = fold_field(h, b, "p_");
        h = fold_field(h, c, "p_");
        h = fnv(h, (uint32_t)size_of(b, "qy"));
    }

    ((pico8_list_t *)get_ref(b, "qy"))->count = 3;
    for (i = 0; i < 3; i++) {
        pico8_dc("kj");
        h = fnv(h, screen_hash());
    }

    h = fold_cut(h, a, 3);
    assert(!pico8_value_equal(get(a, "p_"), pico8_value_bool(1)));
    h = fold_cut(h, a, 2);
    assert(pico8_value_equal(get(a, "p_"), pico8_value_bool(1)));
    h = fold_cut(h, a, 0);
    assert(pico8_value_equal(get(a, "p_"), pico8_value_bool(1)));

    h = fold_value(h, call(b, "hd", 2, get(b, "ra"), get(b, "ra")));
    h = fold_value(h, call(b, "hd", 2, vec_ref(near_mo(0, 0)), vec_ref(near_mo(0, 0))));
    h = fold_value(h, call(b, "gg", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)));
    assert(pico8_value_equal(call(a, "gg", 0, pico8_value_ref(NULL), pico8_value_ref(NULL)), pico8_value_bool(0)));

    (void)call(a, "hb", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    h = fold_field(h, a, "lq");
    assert(!has(a, "lq"));
    kw.has_qe = 1;
    kw.qe = pico8_fixed_from_int(12);
    kw.mu = pico8_fixed_from_int(2);
    pico8_entity_set_field(a, "kw", pico8_value_ref(&kw));
    (void)call(a, "hb", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    row = fold_lq(2166136261u, a);
    h = fnv(h, row);
    by = get_ref(c, "by");
    assert(by == get_ref(a, "by") && by->count == 0);
    pico8_list_add(by, pico8_value_ref(pico8_ob_table_new()));
    pico8_list_add(by, pico8_value_ref(pico8_ob_table_new()));
    (void)call(a, "hb", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(fold_lq(2166136261u, a) != row);
    h = fold_lq(h, a);
    assert(size_of(b, "by") == 2);

    hv_count = pico8_hv()->count;
    (void)call(b, "hk", 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    assert(pico8_hv()->count == hv_count && !has(b, "kr"));
    h = fnv(h, screen_hash());
    check("life", h, LIFE_HASH);
}

/* A harvest played frame by frame: one live burst at a time, gone after
 * BURST_FRAMES frames. */
static void test_burst(void) {
    int frames = 0;
    void *e;

    start_game();
    e = harvest(40, 40, "s64");
    assert(ga_from(0) == e);
    while (ga_from(0) != NULL) {
        e = ga_from(0);
        assert(ga_from(rank_in(pico8_hv(), e) + 1) == NULL);
        pico8__update60();
        pico8__draw();
        frames++;
        assert(frames < 600);
    }
    check_int("burst frames", frames, BURST_FRAMES);
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

static void write_fz(void *e) {
    write_fixed(e, "fz");
}

static void write_k_(void *e) {
    write_fixed(e, "k_");
}

static void write_ms(void *e) {
    write_fixed(e, "ms");
}

static void write_py(void *e) {
    write_fixed(e, "py");
}

static void write_hp(void *e) {
    write_fixed(e, "hp");
}

static void write_ra(void *e) {
    write_fixed(e, "ra");
}

static void write_id(void *e) {
    write_fixed(e, "id");
}

static void write_ht(void *e) {
    write_fixed(e, "ht");
}

static int is_a_ga_struct(const void *e) {
    return pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == pico8_ga_struct_class();
}

/* Built by any road, a burst is a struct whose descriptor holds jo's very
 * entries but its own `kj`, plus its own `mm`; jo's reader sees the 5
 * defaults of ga that fall on one of jo's members; it takes exactly `qy`
 * and jo's 17 names. Neither ga's class nor jo's installs a function or a
 * `pq`. */
static void test_ga_is_a_struct(void) {
    static const char *const on_jo[] = {"nu", "hf", "by", "kq", "fa"};
    pico8_value_t on_jo_want[5];
    const pico8_struct_class_t *cls = pico8_ga_struct_class();
    const pico8_struct_class_t *jo = pico8_jo_struct_class();
    void *roads[3];
    pico8_value_t v;
    size_t i;
    int j;

    start_game();
    roads[0] = harvest(40, 40, "s81");
    roads[1] = spawn(80, 70, "s82", NULL);
    roads[2] = pico8_ga_new(NULL);
    for (i = 0; i < 3; i++) {
        assert(is_a_ga_struct(roads[i]));
    }
    for (j = 0; j < PICO8_METHOD_COUNT; j++) {
        if (j == PICO8_METHOD_KJ) {
            assert(cls->methods[j] != NULL && cls->methods[j] != jo->methods[j]);
        } else if (j == PICO8_METHOD_MM) {
            assert(cls->methods[j] == pico8_ga_mm && jo->methods[j] == NULL);
        } else {
            assert(cls->methods[j] == jo->methods[j]);
        }
    }
    assert(cls->field != jo->field && cls->set_field != jo->set_field);
    on_jo_want[0] = pico8_value_ref((void *)&pico8_ga_nu_tags);
    on_jo_want[1] = pico8_value_bool(0);
    on_jo_want[2] = pico8_value_ref(((const pico8_ga_t *)roads[2])->jo.by);
    on_jo_want[3] = pico8_value_ref((void *)&pico8_ga_start.kq);
    on_jo_want[4] = pico8_value_fixed(pico8_ga_start.fa);
    for (i = 0; i < sizeof on_jo / sizeof on_jo[0]; i++) {
        pico8_value_t through_jo;
        assert(jo->field(roads[0], on_jo[i], &through_jo));
        assert(pico8_value_equal(through_jo, on_jo_want[i]));
    }
    assert(!jo->field(roads[0], "ms", &v) && cls->field(roads[0], "ms", &v));
    assert(pico8_ga_ready());
    assert(pico8_jo_ready());

    must_abort(write_fz, roads[0]);
    must_abort(write_k_, roads[0]);
    must_abort(write_ms, roads[0]);
    must_abort(write_py, roads[0]);
    must_abort(write_hp, roads[0]);
    must_abort(write_ra, roads[0]);
    must_abort(write_id, roads[0]);
    must_abort(write_ht, roads[0]);
    ((pico8_ga_t *)roads[2])->fz = NULL;
    assert(!cls->field(roads[2], "fz", &v) && !is_ga(roads[2]));
}

#define WITNESS 0x1234

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
    case PICO8_VALUE_FUNCTION:
        return a.as.function == b.as.function;
    default:
        return a.as.ref == b.as.ref;
    }
}

/* The raw reader of ga's descriptor and the name API agree on presence,
 * kind, value and pointer; on absence both leave the witness untouched. */
static int read_both(const void *e, const char *name, pico8_value_t *out) {
    pico8_value_t raw = pico8_value_fixed(WITNESS);
    pico8_value_t named = pico8_value_fixed(WITNESS);
    int got_raw = pico8_ga_struct_class()->field(e, name, &raw);
    int got_named = pico8_entity_get_field(e, name, &named);

    assert(got_raw == got_named && same_value(raw, named));
    if (!got_raw) {
        assert(raw.kind == PICO8_VALUE_FIXED && raw.as.fixed == WITNESS);
    }
    *out = raw;
    return got_raw;
}

static uint32_t fold_member(uint32_t h, const void *e, const char *name) {
    pico8_value_t v;
    if (!read_both(e, name, &v)) {
        return fnv(h, 0u);
    }
    if (strcmp(name, "k_") == 0) {
        /* A pico8_int_seq_t has no tag to fold: its items stand in. */
        const pico8_int_seq_t *seq = v.as.ref;
        int j;

        h = fnv(h, 1u);
        for (j = 0; j < seq->count; j++) {
            h = fnv(h, (uint32_t)seq->items[j]);
        }
    } else {
        h = fold_value(fnv(h, 1u), v);
    }
    if (v.kind == PICO8_VALUE_REF) {
        h = fnv(h, (uint32_t)size_of(e, name));
    }
    return h;
}

static void *read_ref(const void *e, const char *name) {
    pico8_value_t v;
    assert(read_both(e, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

static pico8_fixed_t read_fixed(const void *e, const char *name) {
    pico8_value_t v;
    assert(read_both(e, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static void assert_spec_defaults(const void *e) {
    pico8_value_t v;
    assert(read_both(e, "fz", &v) && v.kind == PICO8_VALUE_STRING &&
           v.as.string == pico8_ga_start.fz);
    assert(read_both(e, "fc", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0);
    assert(pico8_type_tag_of(read_ref(e, "qy")) == PICO8_TYPE_TAG_LIST);
}

static void test_ga_members_by_name(void) {
    static const char *const intruders[] = {"", "q", "qy ", "QY", "k", "k__", "fz_", "m"};
    static const pico8_fixed_t keyed[] = {0, 2, 3, -1, 5, 7, 4};
    static const char *const keyed_names[] = {"qo", "ro", "qg", "q_", "iu", "py", "ms"};
    const pico8_struct_class_t *cls = pico8_ga_struct_class();
    uint32_t h = 2166136261u;
    void *roads[3];
    void *keyed_ga;
    void *nil_k;
    void *jo;
    void *own_qy;
    pico8_list_t *fresh;
    static const int k_items[] = {5, 6};
    static const pico8_int_seq_t k_ = {2, k_items};
    pico8_ga_pq_keys_t pq;
    pico8_ga_keys_t keys;
    pico8_value_t v;
    size_t i;
    size_t r;

    start_game();
    roads[0] = harvest(40, 40, "s91");
    roads[1] = spawn(80, 70, "s92", NULL);
    roads[2] = pico8_ga_new(NULL);
    for (r = 0; r < 3; r++) {
        for (i = 0; i < sizeof ga_own / sizeof ga_own[0]; i++) {
            h = fold_member(h, roads[r], ga_own[i]);
        }
        assert_spec_defaults(roads[r]);
    }
    assert(read_ref(roads[0], "k_") == &pico8_ga_k__seq);
    assert(read_ref(roads[2], "k_") == &pico8_ga_k__seq);
    assert(read_ref(roads[1], "k_") != &pico8_ga_k__seq && size_of(roads[1], "k_") == 13);
    assert(read_ref(roads[1], "k_") == &pico8_pm_ga_k__seq);
    assert(read_ref(roads[0], "qy") != read_ref(roads[1], "qy") &&
           read_ref(roads[1], "qy") != read_ref(roads[2], "qy") &&
           read_ref(roads[0], "qy") != read_ref(roads[2], "qy"));

    pq.qo = pico8_fixed_from_int(keyed[0]);
    pq.ro = pico8_fixed_from_int(keyed[1]);
    pq.qg = pico8_fixed_from_int(keyed[2]);
    pq.q_ = pico8_fixed_from_int(keyed[3]);
    pq.iu = pico8_fixed_from_int(keyed[4]);
    pq.py = pico8_fixed_from_int(keyed[5]);
    pq.ms = pico8_fixed_from_int(keyed[6]);
    pq.k_ = &k_;
    keys.jo.id = NULL;
    keys.jo.p = NULL;
    keys.jo.ra = NULL;
    keys.pq = &pq;
    keyed_ga = pico8_ga_new(&keys);
    for (i = 0; i < sizeof keyed / sizeof keyed[0]; i++) {
        assert(read_fixed(keyed_ga, keyed_names[i]) == pico8_fixed_from_int(keyed[i]));
    }
    assert(read_ref(keyed_ga, "k_") == &k_);
    assert_spec_defaults(keyed_ga);
    for (i = 0; i < sizeof ga_own / sizeof ga_own[0]; i++) {
        h = fold_member(h, keyed_ga, ga_own[i]);
    }
    pq.k_ = NULL;
    nil_k = pico8_ga_new(&keys);
    assert(!read_both(nil_k, "k_", &v));
    h = fold_member(h, nil_k, "k_");

    own_qy = read_ref(roads[2], "qy");
    fresh = malloc(sizeof *fresh);
    pico8_list_init(fresh);
    assert(cls->set_field(roads[2], "qy", pico8_value_ref(fresh)) == 1);
    assert(read_ref(roads[2], "qy") == fresh);
    pico8_entity_set_field(roads[2], "qy", pico8_value_ref(NULL));
    assert(!read_both(roads[2], "qy", &v) && !has(roads[2], "qy"));
    h = fold_member(h, roads[2], "qy");
    pico8_entity_set_field(roads[2], "qy", pico8_value_ref(own_qy));
    assert(read_ref(roads[2], "qy") == own_qy);
    for (i = 0; i < sizeof ga_own / sizeof ga_own[0]; i++) {
        pico8_value_t before;
        pico8_value_t after;
        if (strcmp(ga_own[i], "qy") == 0) {
            continue;
        }
        assert(read_both(roads[2], ga_own[i], &before));
        assert(cls->set_field(roads[2], ga_own[i], pico8_value_fixed(pico8_fixed_from_int(9))) == 0);
        assert(read_both(roads[2], ga_own[i], &after) && same_value(before, after));
    }
    pico8_ez((pico8_struct_header_t *)roads[2], pico8_fixed_from_int(2));
    h = fnv(h, (uint32_t)size_of(roads[2], "qy"));

    for (i = 0; i < sizeof intruders / sizeof intruders[0]; i++) {
        assert(!read_both(roads[0], intruders[i], &v));
    }
    jo = pico8_jo_new(NULL);
    for (i = 0; i < sizeof ga_own / sizeof ga_own[0]; i++) {
        assert(!has(jo, ga_own[i]));
    }
    check("members", h, GA_MEMBERS_HASH);
}

static const char *g_off_row_name;
static pico8_value_t g_off_row_value;

static void write_off_row(void *e) {
    pico8_entity_set_field(e, g_off_row_name, g_off_row_value);
}

static pico8_value_t zero_union(pico8_value_kind_t kind) {
    pico8_value_t v;

    memset(&v, 0, sizeof v);
    v.kind = kind;
    return v;
}

/* `qy` takes nil or a list and nothing else. */
static void test_ga_writer_stops_off_row(void) {
    pico8_value_t off_row[11];
    void *roads[3];
    size_t n_off;
    size_t i;
    size_t r;

    start_game();
    roads[0] = harvest(40, 40, "s93");
    roads[1] = spawn(80, 70, "s94", NULL);
    roads[2] = pico8_ga_new(NULL);
    n_off = 0;
    off_row[n_off++] = pico8_value_fixed(pico8_fixed_from_int(1));
    off_row[n_off++] = zero_union(PICO8_VALUE_FIXED);
    off_row[n_off++] = pico8_value_bool(1);
    off_row[n_off++] = pico8_value_bool(0);
    off_row[n_off++] = zero_union(PICO8_VALUE_BOOL);
    off_row[n_off++] = pico8_value_string("x");
    off_row[n_off++] = zero_union(PICO8_VALUE_STRING);
    off_row[n_off++] = zero_union(PICO8_VALUE_FUNCTION);
    off_row[n_off++] = pico8_value_ref(pico8_ob_table_new());
    off_row[n_off++] = pico8_value_ref(pico8_jo_new(NULL));
    off_row[n_off++] = vec_ref(vec_xy(1, 1));
    g_off_row_name = "qy";
    for (r = 0; r < 3; r++) {
        for (i = 0; i < n_off; i++) {
            g_off_row_value = off_row[i];
            must_abort(write_off_row, roads[r]);
        }
    }
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
    in_fresh_child(test_life);
    in_fresh_child(test_burst);
    in_fresh_child(test_ga_is_a_struct);
    in_fresh_child(test_ga_members_by_name);
    in_fresh_child(test_ga_writer_stops_off_row);

    printf("test_pico8_ga: all tests passed\n");
    return 0;
}
