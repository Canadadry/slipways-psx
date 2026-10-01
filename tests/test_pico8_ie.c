/* The research icon `ie`, read and written on its struct members and driven
 * through the real `mv`/`dg`/`dc`/`ok`/`_init`/`eu`, in games
 * started directly: no events file. */

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
#include "../src/pico8_bg.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_fm.h"
#include "../src/pico8_geom.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_vec.h"
#include "../src/pico8_wormhole.h"

#define CONSTRUCTION_HASH 0x37EED64Du
#define START_HASH 0x8C839F39u
#define DIRECTED_HASH 0xA5C469BDu
#define DRAW_HASH 0xA740E905u
#define RESTART_HASH 0xB33A149Au
#define KE_VIEW_HASH 0xD0C93314u

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

static uint32_t fnv_str(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFu);
}

static int ents_rank_in(const pico8_ents_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int ents_listed(const pico8_ents_t *list, const void *e) {
    return ents_rank_in(list, e) >= 0;
}

static pico8_ie_t *as_ie(void *e) {
    assert(e != NULL && ((const pico8_struct_header_t *)e)->cls == pico8_ie_struct_class());
    return e;
}

/* `gy` and `id` are the class's shared state, no member of any icon: the
 * same answer whatever icon asks. */
static const pico8_vec_t *shared_gy(const pico8_ie_t *icon) {
    (void)icon;
    return pico8_ie_gy();
}

/* 0 while `id` is nil. */
static int shared_id(const pico8_ie_t *icon, pico8_fixed_t *out) {
    (void)icon;
    return pico8_ie_id(out);
}

static pico8_fixed_t shared_id_fixed(const pico8_ie_t *icon) {
    pico8_fixed_t id = 0;
    int present = shared_id(icon, &id);
    assert(present);
    return id;
}

/* 0 absent, 1 false, 2 true. */
static uint32_t bool_state(int has, int b) {
    if (!has) {
        return 0;
    }
    return b ? 2 : 1;
}

static uint32_t kh_state(const void *icon) {
    const pico8_ie_t *ie = as_ie((void *)icon);
    return bool_state(ie->has_kh, ie->kh);
}

static uint32_t m__state(const void *icon) {
    const pico8_ie_t *ie = as_ie((void *)icon);
    return bool_state((ie->m_ != 0), ie->m_);
}

static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

/* The kind bytes the frozen constants were folded with (0xF1 a number,
 * 0x57 a string, 0xDEAD absent); a NULL string is absent. */
static uint32_t fold_fixed(uint32_t h, pico8_fixed_t v) {
    return fnv(fnv(h, 0xF1u), (uint32_t)v);
}

static uint32_t fold_string(uint32_t h, const char *s) {
    return s == NULL ? fnv(h, 0xDEADu) : fnv_str(fnv(h, 0x57u), s);
}

/* An icon is the one entity of `cv.hb` whose `ke` holds a kind, not an
 * entity: there is no `cg.ie`. */
static int is_icon(const void *e) {
    return e != NULL && ((const pico8_struct_header_t *)e)->cls == pico8_ie_struct_class() &&
           ((const pico8_ie_t *)e)->ke != NULL;
}

static int icons(void *out[], int cap) {
    const pico8_ents_t *hb = pico8_cv("hb");
    int n = 0;
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i];
        if (is_icon(e)) {
            assert(n < cap);
            out[n++] = e;
        }
    }
    return n;
}

static int icon_rank(const void *e) {
    void *all[64];
    int n = icons(all, 64);
    int i;
    for (i = 0; i < n; i++) {
        if (all[i] == e) {
            return i;
        }
    }
    return -1;
}

static void gx(const pico8_entity_kind_t *out[7]) {
    out[0] = pico8_qh_kind();
    out[1] = pico8_os_kind();
    out[2] = pico8_nr_kind();
    out[3] = pico8_fr_kind();
    out[4] = pico8_nl_kind();
    out[5] = pico8_mx_kind();
    out[6] = pico8_hj_kind();
}

/* `gy` and `id` read off any icon: the cursor and counter every icon
 * shares. */
static uint32_t fold_shared(uint32_t h, void *icon) {
    pico8_fixed_t id = 0;

    h = fold_vec(h, shared_gy(as_ie(icon)));
    return shared_id(as_ie(icon), &id) ? fold_fixed(h, id) : fnv(h, 0xDEADu);
}

/* `t` is always a number: its kind byte is constant by the struct's shape,
 * kept so that the frozen constants still prove the typed reads. */
static uint32_t fold_icon(uint32_t h, void *icon) {
    const pico8_ie_t *ie = as_ie(icon);
    h = fnv(h, kh_state(ie));
    h = fold_vec(h, ie->ra);
    h = fnv(h, m__state(ie));
    return fold_fixed(h, ie->header.t);
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

static void new_game(void) {
    pico8_eu(NULL);
}

/* A game started as `eu` starts it, with no difficulty picked: `dk` NULL
 * reads as difficulty 0. */
static void start_game(void) {
    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    new_game();
}

static const pico8_struct_class_t *cls_of(const void *e) {
    return ((const pico8_struct_header_t *)e)->cls;
}

static pico8_vec_t *vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
}

static pico8_vec_t v2(int x, int y) {
    return pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
}

static const pico8_jb_row_t *dr_row(int i) {
    assert(i < PICO8_DR_COUNT);
    return &pico8_dr_table[i];
}

static void *build_icon(const pico8_entity_kind_t *ke, pico8_jb_t *jb) {
    pico8_ie_keys_t keys = {0};

    keys.ke = ke;
    keys.jb = jb;
    return pico8_ie_new(&keys);
}

/* The `kq` box by which of a ring of points around its corners it holds. */
static uint32_t fold_kq(uint32_t h, void *icon) {
    static const int probe[][2] = {{0, 0},  {-1, 0}, {0, -1}, {8, 7},  {9, 8},  {10, 8}, {9, 9},
                                   {-1, -1}, {4, 4},  {8, 0},  {9, 0},  {0, 7},  {0, 8},  {0, 9}};
    const pico8_qz_box_t *kq = &as_ie(icon)->def->kq;
    size_t i;

    assert(kq != NULL);
    for (i = 0; i < sizeof probe / sizeof probe[0]; i++) {
        h = fnv(h, (uint32_t)pico8_qz_call(kq, v2(probe[i][0], probe[i][1])));
    }
    return h;
}

/* A fresh icon: its archetype's `kq`/`fa`/`ra`/`eh`, ke's `me`/`t`, the two
 * construction keys, and nothing written by `hb` or the hover loop yet. */
static uint32_t check_fresh_icon(uint32_t h, void *icon, const void *ke, const void *jb) {
    const pico8_ie_t *ie = as_ie(icon);

    assert(ie->header.me != NULL && strcmp(ie->header.me, "ph") == 0);
    assert(ie->header.t == 0);
    assert(ie->def == &pico8_ie_def);
    assert(ie->def->fa == pico8_fixed_from_int(12));
    assert(ie->ra == &ie->def->ra);
    assert(ie->ra->x == pico8_fixed_from_int(999) && ie->ra->y == pico8_fixed_from_int(999));
    assert(ie->def->eh.x == pico8_fixed_from_int(118) && ie->def->eh.y == pico8_fixed_from_int(120));
    assert((const void *)ie->ke == ke);
    assert((const void *)ie->jb == jb);
    assert(!ie->has_kh && !ie->m_);
    assert(cls_of(icon)->states[PICO8_STATE_PH] == NULL && cls_of(icon)->ev == NULL && cls_of(icon)->hk == NULL &&
           cls_of(icon)->rdown == NULL && cls_of(icon)->gv == NULL);
    assert(cls_of(icon)->hd != NULL && cls_of(icon)->hb != NULL && cls_of(icon)->kj != NULL &&
           cls_of(icon)->ldown != NULL);
    assert(ents_listed(pico8_hv(), icon) && ents_listed(pico8_cv("kj"), icon) && ents_listed(pico8_cv("hd"), icon) &&
           ents_listed(pico8_cv("hb"), icon));
    assert(!ents_listed(pico8_cv("ev"), icon) && !ents_listed(pico8_cv("hk"), icon));

    h = fold_string(h, ie->header.me);
    h = fold_fixed(h, ie->def->fa);
    h = fold_kq(h, icon);
    h = fold_vec(h, ie->ra);
    h = fold_vec(h, &ie->def->eh);
    return fold_shared(h, icon);
}

static void test_construction(void) {
    uint32_t h = 2166136261u;
    const pico8_entity_kind_t *classes[7];
    void *all[16];
    void *a;
    void *b;
    pico8_jb_t *jb_a;
    pico8_jb_t *jb_b;
    int n;
    int i;

    start_game();
    gx(classes);
    n = icons(all, 16);
    check_int("eu icons", n, 7);
    for (i = 0; i < n; i++) {
        const pico8_ie_t *ie = as_ie(all[i]);
        assert(ie->ke == classes[i]);
        assert(ie->jb != NULL);
        assert(ie->header.me != NULL && strcmp(ie->header.me, "ph") == 0);
        assert(ie->def->fa == pico8_fixed_from_int(12));
        assert(cls_of(all[i])->hd != NULL);
        assert(ents_listed(pico8_hv(), all[i]) && ents_listed(pico8_cv("kj"), all[i]) && ents_listed(pico8_cv("hd"), all[i]));
        assert(!ents_listed(pico8_cv("ev"), all[i]) && !ents_listed(pico8_cv("hk"), all[i]));
        h = fold_kq(h, all[i]);
    }

    jb_a = pico8_jb_new(dr_row(0));
    jb_b = pico8_jb_new(dr_row(1));
    a = build_icon(classes[0], jb_a);
    b = build_icon(classes[1], jb_b);
    h = check_fresh_icon(h, a, classes[0], jb_a);
    h = check_fresh_icon(h, b, classes[1], jb_b);
    assert(as_ie(a)->ra == as_ie(b)->ra);
    assert(as_ie(a)->def == as_ie(b)->def);
    assert(as_ie(a)->def == as_ie(all[0])->def);
    assert(icon_rank(a) == 7 && icon_rank(b) == 8);
    check("construction", h, CONSTRUCTION_HASH);
}

/* `_init` points `gy` at `eh` itself, then `eu`'s `fm:lj` reveals every icon
 * whose research is done, right to left from there. */
static void test_start(void) {
    uint32_t h = 2166136261u;
    void *all[16];
    int n;
    int i;
    int first = -1;

    start_game();
    n = icons(all, 16);
    for (i = 0; i < n; i++) {
        if (first < 0 && kh_state(all[i]) == 2) {
            first = i;
        }
        h = fold_icon(h, all[i]);
    }
    printf("[start] first revealed icon %d\n", first);
    assert(first >= 0);
    assert(as_ie(all[first])->ra == &as_ie(all[first])->def->eh);
    for (i = 0; i < n; i++) {
        if (kh_state(all[i]) != 2) {
            assert(as_ie(all[i])->ra != as_ie(all[first])->ra);
        }
    }
    h = fold_shared(h, all[0]);
    check("start", h, START_HASH);
}

static void *first_icon_with_kh(uint32_t kh) {
    void *all[16];
    int n = icons(all, 16);
    int i;
    for (i = 0; i < n; i++) {
        if (kh_state(all[i]) == kh) {
            return all[i];
        }
    }
    return NULL;
}

/* `hb` reveals once; `ldown` bumps the one counter every icon shares and
 * builds the held class; `hd` tests `kq` around `ra` in screen space. */
static void test_directed(void) {
    uint32_t h = 2166136261u;
    void *shown;
    void *hidden;
    void *all[16];
    const pico8_vec_t *gy_before;
    const pico8_vec_t *ra_before;
    pico8_down_t made;
    pico8_fixed_t id0;
    pico8_vec_t ra;
    int n;

    start_game();
    n = icons(all, 16);
    shown = first_icon_with_kh(2);
    hidden = first_icon_with_kh(0);
    assert(shown != NULL && hidden != NULL);

    gy_before = shared_gy(as_ie(shown));
    ra_before = as_ie(shown)->ra;
    cls_of(shown)->hb(shown);
    assert(as_ie(shown)->ra == ra_before && shared_gy(as_ie(shown)) == gy_before);
    cls_of(hidden)->hb(hidden);
    assert(kh_state(hidden) == 0 && shared_gy(as_ie(hidden)) == gy_before);
    h = fold_icon(h, shown);
    h = fold_icon(h, hidden);
    h = fold_shared(h, hidden);

    id0 = shared_id_fixed(as_ie(all[0]));
    made = cls_of(all[0])->ldown(all[0], NULL, NULL);
    assert(made.stop && made.held != NULL && pico8_jo_is_family(made.held));
    assert(strcmp(((const pico8_jo_t *)made.held)->id, "s1") == 0);
    assert(shared_id_fixed(as_ie(all[n - 1])) == pico8_fixed_add(id0, pico8_fixed_from_int(1)));
    h = fold_string(h, ((const pico8_jo_t *)made.held)->id);
    made = cls_of(all[n - 1])->ldown(all[n - 1], NULL, NULL);
    assert(made.held != NULL && pico8_jo_is_family(made.held));
    assert(strcmp(((const pico8_jo_t *)made.held)->id, "s2") == 0);
    assert(shared_id_fixed(as_ie(all[0])) == pico8_fixed_add(id0, pico8_fixed_from_int(2)));
    h = fold_string(h, ((const pico8_jo_t *)made.held)->id);
    h = fold_shared(h, all[3]);

    assert(as_ie(shown)->ra != NULL);
    ra = *as_ie(shown)->ra;
    assert(cls_of(shown)->hd(shown, vec_ref(v2(-500, -500)), vec_ref(pico8_vec_add(ra, v2(1, 1)))));
    assert(!cls_of(shown)->hd(shown, vec_ref(pico8_vec_add(ra, v2(1, 1))),
                              vec_ref(pico8_vec_add(ra, v2(20, 20)))));
    assert(!cls_of(hidden)->hd(hidden, vec_ref(v2(0, 0)), vec_ref(pico8_vec_add(ra, v2(1, 1)))));
    {
        int dx;
        for (dx = -2; dx <= 11; dx++) {
            int hit = cls_of(shown)->hd(shown, vec_ref(v2(0, 0)),
                                        vec_ref(pico8_vec_add(ra, v2(dx, dx - 1))));
            h = fnv(h, (uint32_t)hit);
        }
    }
    check("directed", h, DIRECTED_HASH);
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

static uint32_t draw_alone(void *icon) {
    pico8_camera(&gfx, 0, 0);
    pico8_cls(&mem);
    cls_of(icon)->kj(icon, as_ie(icon)->ra);
    return screen_hash();
}

/* Hidden: nothing; revealed: the sprite at `ra`; hovered (`m_`): the build
 * cost and the tooltip as well. */
static void test_draw(void) {
    uint32_t h = 2166136261u;
    uint32_t blank;
    uint32_t hidden_px;
    uint32_t shown_px;
    uint32_t hovered_px;
    void *shown;
    void *hidden;

    start_game();
    shown = first_icon_with_kh(2);
    hidden = first_icon_with_kh(0);
    pico8_camera(&gfx, 0, 0);
    pico8_cls(&mem);
    blank = screen_hash();

    hidden_px = draw_alone(hidden);
    assert(hidden_px == blank);
    as_ie(hidden)->m_ = 1;
    assert(m__state(hidden) == 2);
    assert(draw_alone(hidden) == blank);
    shown_px = draw_alone(shown);
    assert(shown_px != blank);
    as_ie(shown)->m_ = 1;
    assert(m__state(shown) == 2);
    hovered_px = draw_alone(shown);
    assert(hovered_px != shown_px);
    as_ie(shown)->m_ = 0;
    assert(draw_alone(shown) == shown_px);
    *as_ie(shown)->header.m_ = 1;
    assert(m__state(shown) == 2);
    *as_ie(shown)->header.m_ = 0;
    assert(m__state(shown) == 0);

    h = fnv(h, hidden_px);
    h = fnv(h, shown_px);
    h = fnv(h, hovered_px);
    check("draw", h, DRAW_HASH);
}

/* `rr` of each `gx[]` class, in `gx[]` order: nil for `qh` and `os`. */
static char *const gx_rr[7] = {NULL, NULL, "slipgates", "infraspace", "trade league", "void synthesis",
                                     "ascension"};

static void invent(char *rr) {
    assert(pico8_fm_hy() != NULL && pico8_fm_hy()->invented != NULL);
    pico8_ptrs_add(pico8_fm_hy()->invented, rr);
}

/* Per icon, in `gx[]` order: `hb` reveals it exactly when `oj(self.ke.rr)`,
 * and `kj` draws the held class's sprite `s`, then with `m_` its build cost
 * `od$` off `dl(self.ke)`. */
static void test_ke_view(void) {
    uint32_t h = 2166136261u;
    void *all[16];
    int n;
    int i;

    start_game();
    n = icons(all, 16);
    check_int("ke view icons", n, 7);
    for (i = 0; i < n; i++) {
        uint32_t before;
        cls_of(all[i])->hb(all[i]);
        before = kh_state(all[i]);
        assert(before == (gx_rr[i] == NULL ? 2u : 0u));
        if (gx_rr[i] != NULL) {
            invent(gx_rr[i]);
            cls_of(all[i])->hb(all[i]);
        }
        assert(kh_state(all[i]) == 2);
        h = fnv(h, before);
        h = fold_icon(h, all[i]);
    }
    h = fold_shared(h, all[0]);
    {
        /* `dl` prints at `je.ra`, (0,0) here: its label would land off
         * screen. */
        assert(pico8_je_nf() != NULL && pico8_je_nf()->ra != NULL);
        *pico8_je_nf()->ra = v2(64, 64);
    }
    for (i = 0; i < n; i++) {
        uint32_t plain = draw_alone(all[i]);
        uint32_t hovered;
        as_ie(all[i])->m_ = 1;
        hovered = draw_alone(all[i]);
        as_ie(all[i])->m_ = 0;
        printf("[ke view] icon %d: 0x%08Xu / 0x%08Xu\n", i, (unsigned)plain, (unsigned)hovered);
        assert(hovered != plain);
        h = fnv(h, plain);
        h = fnv(h, hovered);
    }
    check("ke view", h, KE_VIEW_HASH);
}

/* A second `eu` keeps `gy` and `id` where the last game left them; only
 * `_init` puts `gy` back on `eh`, and nothing resets `id`. */
static void test_restart(void) {
    uint32_t h = 2166136261u;
    void *all[16];
    const pico8_vec_t *gy;
    pico8_fixed_t id;
    int n;
    int i;

    start_game();
    n = icons(all, 16);
    (void)cls_of(all[0])->ldown(all[0], NULL, NULL);
    gy = shared_gy(as_ie(all[0]));
    id = shared_id_fixed(as_ie(all[0]));
    assert(id == pico8_fixed_from_int(1));
    assert(gy != &as_ie(all[0])->def->eh);

    new_game();
    n = icons(all, 16);
    check_int("restart icons", n, 7);
    assert(shared_id_fixed(as_ie(all[0])) == id);
    assert(shared_gy(as_ie(all[0])) != gy);
    for (i = 0; i < n; i++) {
        if (kh_state(all[i]) == 2) {
            assert(as_ie(all[i])->ra != &as_ie(all[i])->def->eh);
        }
        h = fold_icon(h, all[i]);
    }
    h = fold_shared(h, all[0]);

    pico8__init();
    assert(shared_gy(as_ie(all[0])) == &as_ie(all[0])->def->eh);
    assert(shared_id_fixed(as_ie(all[0])) == id);
    h = fold_shared(h, all[0]);
    check("restart", h, RESTART_HASH);
}

static int is_an_ie_struct(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_ie_struct_class();
}

/* Built by `eu` or directly, an `ie` is a struct with its four methods, and
 * no other struct answers to `hb`. */
static void test_ie_is_a_struct(void) {
    const pico8_entity_kind_t *classes[7];
    void *all[16];
    void *built;
    void *others[4];
    int n;
    int i;

    start_game();
    gx(classes);
    n = icons(all, 16);
    built = build_icon(classes[2], pico8_jb_new(dr_row(2)));
    all[n++] = built;
    for (i = 0; i < n; i++) {
        assert(is_an_ie_struct(all[i]));
        assert(cls_of(all[i])->hd != NULL && cls_of(all[i])->hb != NULL && cls_of(all[i])->kj != NULL &&
               cls_of(all[i])->ldown != NULL);
        assert(cls_of(all[i])->states[PICO8_STATE_PH] == NULL && cls_of(all[i])->ev == NULL &&
               cls_of(all[i])->hk == NULL && cls_of(all[i])->rdown == NULL && cls_of(all[i])->gv == NULL);
    }

    {
        pico8_ie_keys_t jb_only = {0};
        pico8_ie_t *no_ke;
        pico8_fixed_t id0 = shared_id_fixed(as_ie(built));

        jb_only.jb = pico8_jb_new(NULL);
        no_ke = pico8_ie_new(&jb_only);
        assert(is_an_ie_struct(no_ke) && no_ke->ke == NULL);
        assert(shared_id_fixed(no_ke) == id0);
        (void)cls_of(no_ke)->ldown(no_ke, NULL, NULL);
        assert(shared_id_fixed(as_ie(built)) == pico8_fixed_add(id0, pico8_fixed_from_int(1)));
    }

    others[0] = pico8_bg_new();
    others[1] = pico8_fq_new(NULL);
    others[2] = pico8_jc_new(NULL);
    others[3] = pico8_mz_new();
    for (i = 0; i < 4; i++) {
        assert(cls_of(others[i])->hb == NULL);
    }
}

/* `self.ke` read through a struct without parent that carries only the
 * `rr`/`s`/`me`/`od` of the held class's chain. */
static void test_ke_view_is_a_struct(void) {
    static const int s[7] = {174, 100, 170, 78, 68, 66, 166};
    static const int od[7] = {15, 15, 10, 25, 20, 40, 60};
    const pico8_entity_kind_t *classes[7];
    void *all[16];
    pico8_ie_ke_t view;
    int n;
    int i;

    start_game();
    gx(classes);
    n = icons(all, 16);
    check_int("ke view struct icons", n, 7);
    for (i = 0; i < n; i++) {
        assert(as_ie(all[i])->ke == classes[i]);
        view = pico8_ie_ke_view(all[i]);
        assert(view.header.cls == pico8_ie_ke_view_struct_class());
        if (gx_rr[i] == NULL) {
            assert(view.rr == NULL);
        } else {
            assert(view.rr != NULL && strcmp(view.rr, gx_rr[i]) == 0);
        }
        assert(view.has_s && view.s == pico8_fixed_from_int(s[i]));
        assert(view.has_od && view.od == pico8_fixed_from_int(od[i]));
        assert(view.me != NULL && strcmp(view.me, "kz") == 0);
    }

    {
        pico8_ie_keys_t jb_only = {0};
        void *no_ke;

        jb_only.jb = pico8_jb_new(NULL);
        no_ke = pico8_ie_new(&jb_only);
        view = pico8_ie_ke_view(no_ke);
        assert(view.header.cls == pico8_ie_ke_view_struct_class());
        assert(view.rr == NULL && !view.has_s && view.me == NULL && !view.has_od);
    }
}

/* The galaxy depends on the rnd state: run each scenario in a child forked
 * right after pico8_cart_load(), as a fresh process would start it. */
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
    in_fresh_child(test_construction);
    in_fresh_child(test_start);
    in_fresh_child(test_directed);
    in_fresh_child(test_draw);
    in_fresh_child(test_restart);
    in_fresh_child(test_ke_view);
    in_fresh_child(test_ke_view_is_a_struct);
    in_fresh_child(test_ie_is_a_struct);

    printf("test_pico8_ie: all tests passed\n");
    return 0;
}
