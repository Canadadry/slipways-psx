/* The processor `os`, read and written on its struct members and the
 * generated defaults (pico8_game_data.h), and driven through the real `ie:ldown`/`mv`/`dg`, in games started
 * directly: no events file. */

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

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "game/economy_score.h"
#include "game/economy_tiers.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/trade.h"
#include "engine/game_zone.h"
#include "engine/entity_class.h"
#include "game/ui_button.h"
#include "engine/vec.h"
#include "zone_tas.h"

/* The cart keeps the box of `mp`, the port a copy of it (nil stays nil). */
static int ra_copies(const pico8_vec_t *ra, const pico8_vec_t *mp) {
    return mp == NULL ? ra == NULL : ra != NULL && ra != mp && ra->x == mp->x && ra->y == mp->y;
}


/* The tag class instances carried, which the frozen hashes fold: any
 * other value moves every *_HASH. */
#define FROZEN_INSTANCE_TAG 0x9E5C1451u

/* The tag entity structs carried, which the frozen hashes fold: any other
 * value moves every *_HASH. */
#define FROZEN_STRUCT_TAG 0x9E5C57C7u

#define BUILT_HASH 0x0FBFD36Bu
#define LIFE_HASH 0x56980D6Du

/* The tag `jo:ib`'s list carried as a tagged list, which the frozen hashes
 * fold: any other value moves every *_HASH. */
#define FROZEN_LIST_TAG 0x9E5C1157u

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

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

static uint32_t fold_absent(uint32_t h) {
    return fnv(h, 0u);
}

static uint32_t fold_fixed_member(uint32_t h, pico8_fixed_t v) {
    return fnv(fnv(fnv(h, 1u), 0xF1u), (uint32_t)v);
}

static uint32_t fold_bool_member(uint32_t h, int b) {
    return fnv(fnv(fnv(h, 1u), 0xB0u), (uint32_t)(b != 0));
}

/* A NULL string folds as absent. */
static uint32_t fold_string_member(uint32_t h, const char *s) {
    return s == NULL ? fold_absent(h) : fnv_str(fnv(fnv(h, 1u), 0x57u), s);
}

static uint32_t fold_hf(uint32_t h, const pico8_jo_t *jo) {
    return jo->def->hf ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

static uint32_t fold_vec(uint32_t h, const pico8_vec_t *p) {
    if (p == NULL) {
        return fnv(h, 0xDEADu);
    }
    h = fnv(h, (uint32_t)p->x);
    return fnv(h, (uint32_t)p->y);
}

static int is_bv_target(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_bv_et_struct_class();
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (ents_listed(pico8_hv(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)ents_rank_in(pico8_hv(), ref));
    }
    /* The frozen hashes folded the INSTANCE tag the throwaway had. */
    if (is_bv_target(ref)) {
        return fnv(fnv(h, 0xEEEEu), FROZEN_INSTANCE_TAG);
    }
    return fnv(fnv(h, 0xEEEEu), FROZEN_STRUCT_TAG);
}

/* The item count of a list member, -1 when NULL. */
static int list_size(const pico8_ptrs_t *l) {
    return l == NULL ? -1 : l->count;
}

static int ents_list_size(const pico8_ents_t *l) {
    return l == NULL ? -1 : l->count;
}

static uint32_t fold_kw(uint32_t h, const pico8_jo_t *jo) {
    const pico8_kw_t *k = jo->kw;

    if (k == NULL) {
        return fnv(h, 0xA0u);
    }
    h = k->has_qe ? fold_fixed_member(h, k->qe) : fold_absent(h);
    h = fold_fixed_member(h, k->mu);
    return fnv(h, (uint32_t)(k == pico8_jo_shared_kw()));
}

/* `e` as a planet of qh's lineage. */
static pico8_jo_t *as_qh(void *e) {
    const pico8_struct_header_t *h = e;

    assert(h != NULL && (h->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)));
    return e;
}

static uint32_t fold_qh(uint32_t h, const pico8_jo_t *qh) {
    return qh->def->qh ? fold_fixed_member(h, pico8_fixed_from_int(1)) : fold_bool_member(h, 0);
}

/* 1 no target, 3 a target: the bytes the frozen hashes fold. */
static uint32_t et_state(const pico8_jo_t *qh) {
    return qh->probe->et == NULL ? 1u : 3u;
}

/* `et` as qh's reader gave it: false or the target. */
static uint32_t fold_et(uint32_t h, const pico8_jo_t *qh) {
    switch (et_state(qh)) {
    case 1u:
        return fold_bool_member(h, 0);
    default:
        return fold_ref(fnv(h, 1u), qh->probe->et);
    }
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

/* A game started from the title screen as a click on its second
 * difficulty button starts it: `js:ldown` runs the `my` that `fv:pq`
 * stamped, `eu` on that button. `db` reads `dk.trade`, which a NULL `dk`
 * lacks. */
static void clicked_game(void) {
    const pico8_ents_t *hd;
    int buttons = 0;
    int i;

    pico8_input_init(&input);
    pico8_dispatch_set_input(&input);
    pico8__init();
    hd = pico8_cv("hd");
    for (i = 0; i < hd->count; i++) {
        if (((const pico8_struct_header_t *)hd->items[i])->cls == pico8_js_struct_class() && ++buttons == 2) {
            (void)pico8_js_ldown((pico8_js_t *)hd->items[i]);
            break;
        }
    }
    assert(buttons == 2 && pico8_dk() != NULL);
}

static pico8_vec_t *vec_ref(pico8_vec_t at) {
    pico8_vec_t *v = malloc(sizeof *v);
    *v = at;
    return v;
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

/* The research icon whose `ke` is `gx[1]`, the processor. */
static void *os_icon(void) {
    const pico8_ents_t *hb = pico8_cv("hb");
    int i;
    for (i = 0; hb != NULL && i < hb->count; i++) {
        void *e = hb->items[i];
        if (cls_of(e) == pico8_ie_struct_class() && ((const pico8_ie_t *)e)->ke == pico8_os_kind()) {
            return e;
        }
    }
    return NULL;
}

/* `ie:ldown` on the processor's icon, as the shop does. */
static void *build_processor(void) {
    void *icon = os_icon();
    pico8_down_t made;

    assert(icon != NULL);
    made = cls_of(icon)->ldown(icon, NULL, NULL);
    assert(made.stop && made.held != NULL);
    return made.held;
}

/* A processor is the only entity whose `px` is os's own table. */
static int is_processor(const void *e) {
    const pico8_jo_t *jo = e != NULL && (cls_of(e)->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) ? e : NULL;
    return jo != NULL && jo->def->px == &pico8_os_px_row;
}

/* A processor as `ie:ldown` and `jo:pq` left it: its scalars (`hs` is jo's
 * name, overridden by os), which of qh's and jo's members are set, its
 * methods (qh's chain's very functions, `ly` included), which default tables
 * it shares with `os`, `qh` and `jo`, and where `lt` registered it -- never
 * in `cg.qh`. */
static uint32_t fold_processor(uint32_t h, void *e) {
    const pico8_jo_t *qh = as_qh(e);
    const pico8_jo_t *jo = qh;
    size_t i;

    h = fold_string_member(h, jo->id);
    h = fold_string_member(h, jo->header.me);
    h = fold_fixed_member(h, jo->header.t);
    h = fold_qh(h, qh);
    h = fold_fixed_member(h, qh->def->hp);
    h = fold_fixed_member(h, qh->s);
    h = fold_fixed_member(h, qh->probe->od);
    h = fold_fixed_member(h, jo->def->hs);
    h = fold_string_member(h, qh->probe->def->dq);
    h = fold_fixed_member(h, qh->probe->def->fp);
    h = fold_fixed_member(h, qh->probe->def->dw);
    h = jo->has_mi ? fold_fixed_member(h, jo->mi) : fold_absent(h);
    h = fold_hf(h, jo);
    h = fold_fixed_member(h, jo->def->fa);
    h = fold_fixed_member(h, jo->ma);
    h = fold_fixed_member(h, jo->def->ca);
    h = fold_vec(h, &jo->def->gq);
    /* The 1s of `hp`, `qh`, `od`, `fp`, `dw`, `s`, then of `hf`, `hs`, `fa`,
     * `ma` and `ca`: constant by the struct's shape, kept so that BUILT_HASH
     * still proves the typed reads. */
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->def->px != NULL));
    h = fnv(h, (uint32_t)(qh->probe->def->dq != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(qh->def->ba != NULL));
    h = fnv(h, qh->probe->et != NULL ? 1u : 0u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(jo->def->ka != NULL));
    h = fnv(h, (uint32_t)(jo->def->dz != NULL));
    h = fnv(h, (uint32_t)(jo->def->cq != NULL));
    h = fnv(h, (uint32_t)(jo->kw != NULL));
    h = fnv(h, 1u);
    h = fnv(h, 1u);
    h = fnv(h, (uint32_t)(jo->def->em != NULL));
    h = fnv(h, (uint32_t)(jo->id != NULL));
    h = fnv(h, (uint32_t)(jo->p != NULL));
    h = fnv(h, (uint32_t)(jo->ra != NULL));
    h = fnv(h, (uint32_t)(jo->oe != NULL));
    h = fnv(h, (uint32_t)(jo->hn != NULL));
    h = fnv(h, (uint32_t)(jo->ep != NULL));
    h = fnv(h, (uint32_t)(jo->jy != NULL));
    h = fnv(h, (uint32_t)(jo->nk != NULL));
    h = fnv(h, (uint32_t)(jo->fx != 0));
    h = fnv(h, (uint32_t)(jo->lq != NULL));
    h = fnv(h, (uint32_t)(jo->fo != NULL));
    h = fnv(h, (uint32_t)jo->has_jh);
    h = fnv(h, (uint32_t)(jo->p_ != 0));
    h = fnv(h, (uint32_t)jo->has_kr);
    h = fnv(h, (uint32_t)(jo->cc != NULL));
    h = fnv(h, (uint32_t)(jo->by != NULL));
    h = fnv(h, (uint32_t)jo->has_mi);
    h = fnv(h, (uint32_t)(jo->m_ != 0));
    assert_qh_shape(cls_of(e));
    assert_members_of(cls_of(e), pico8_qh_struct_class(), 1, 1);
    assert(cls_of(e)->kf == pico8_qh_struct_class()->kf && cls_of(e)->cn == pico8_qh_struct_class()->cn &&
           cls_of(e)->mm == NULL && pico8_jo_is_family(e));
    assert(jo->header.cls->tags == (PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER)));
    assert(qh->def->px == &pico8_os_px_row && qh->def->px != &pico8_qh_px_row);
    assert(qh->def->ba == &pico8_qh_ba_seq);
    assert(jo->def == &pico8_os_def);
    assert(jo->def == &pico8_os_def);
    assert(jo->def->em == &pico8_jo_em_row && jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows &&
           jo->def->cq == &pico8_jo_cq_seq);
    assert(jo->kw != NULL && jo->kw != pico8_jo_shared_kw());
    assert(pico8_gb_at(4) != NULL);
    assert(jo->hn == pico8_gb_at(4));
    h = fold_kw(h, jo);
    h = fnv(h, (uint32_t)ents_list_size(jo->oe));
    h = fnv(h, (uint32_t)list_size(jo->jy));
    h = fnv(h, (uint32_t)ents_rank_in(pico8_hv(), e));
    for (i = 0; i < sizeof bm / sizeof bm[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(pico8_cv(bm[i]), e));
    }
    for (i = 0; i < sizeof tags / sizeof tags[0]; i++) {
        h = fnv(h, (uint32_t)ents_rank_in(pico8_cg(tags[i]), e));
    }
    assert(!ents_listed(pico8_cg("qh"), e));
    return h;
}

/* Two processors built by the shop's `ie:ldown` on `gx[1]`. */
static void test_built(void) {
    uint32_t h = 2166136261u;
    void *a;
    void *b;

    start_game();
    a = build_processor();
    b = build_processor();
    assert(a != b && is_processor(a) && is_processor(b));
    h = fold_processor(h, a);
    h = fold_processor(h, b);
    check("built", h, BUILT_HASH);
}

static pico8_fixed_t credits(void) {
    assert(pico8_fm_hy() != NULL);
    return pico8_fm_hy()->nz;
}

static void set_credits(int n) {
    pico8_fm_hy()->nz = pico8_fixed_from_int(n);
}

static uint32_t fold_ra_y(uint32_t h, const pico8_jo_t *qh) {
    const pico8_vec_t *ra = qh->ra;
    return ra == NULL ? fnv(h, 0xDEADu) : fnv(h, (uint32_t)ra->y);
}

/* `dg` once, then what qh's `ly` (`na`) left on the processor. */
static uint32_t fold_dg(uint32_t h, void *e) {
    const pico8_jo_t *qh = as_qh(e);

    h = fold_ra_y(h, qh);
    pico8_dg();
    h = fold_fixed_member(h, qh->header.t);
    return fold_ra_y(h, qh);
}

/* `jo:ib`'s container, folded as fold_ref folded the tagged list it was:
 * never in `hv`, so the tag branch. */
static uint32_t fold_ib(uint32_t h, const pico8_ents_t *ib) {
    if (ib == NULL) {
        return fnv(h, 0xDEADu);
    }
    return fnv(fnv(h, 0xEEEEu), FROZEN_LIST_TAG);
}

/* What `bb`'s `dh` (no `pt.nl`, `pt.hf` false), `ib`, `gg` and `da` (qh
 * false) give on a processor. */
static uint32_t fold_economy(uint32_t h, void *e) {
    pico8_ents_t *ib = pico8_jo_ib(e);
    h = fnv(h, (uint32_t)pico8_dh(e));
    h = fold_ib(h, ib);
    h = fnv(fnv(h, 0xB0u), pico8_jo_gg(e) ? 1u : 0u);
    h = fnv(h, (uint32_t)pico8_da(e));
    return as_qh(e)->has_mi ? fold_fixed_member(h, as_qh(e)->mi) : fold_absent(h);
}

/* `pt.by` holding one export from `pt` to `oz`, as `jo:ib` leaves it: a
 * trade whose `lp` starts at `pt` and ends at `oz`. */
static void export_to(void *pt, void *oz) {
    pico8_trade_keys_t keys = {0};
    pico8_ents_t *lp = malloc(sizeof *lp);
    pico8_ents_t *by = malloc(sizeof *by);
    pico8_trade_t *t;

    pico8_ents_init(lp, &zone_tas);
    pico8_ents_add(lp, pt);
    pico8_ents_add(lp, oz);
    keys.lp = lp;
    t = pico8_trade_new(pico8_zone_partie(), &keys);
    pico8_ents_init(by, &zone_tas);
    pico8_ents_add(by, &t->header);
    assert(pico8_jo_is_family(pt));
    ((pico8_jo_t *)pt)->by = by;
}

/* `db` of a processor and of a bare planet, at the same tier, each exporting
 * to the same bare planet (`hf` true): only `pt.hs` tells them apart. */
static uint32_t fold_db(uint32_t h, void *processor) {
    pico8_jo_t *bare = pico8_jo_new(NULL);
    pico8_jo_t *oz = pico8_jo_new(NULL);
    const pico8_jo_t *from = as_qh(processor);
    pico8_fixed_t os_db;
    pico8_fixed_t bare_db;

    assert(oz->def->hf);
    bare->mi = from->has_mi ? from->mi : 0;
    bare->has_mi = from->has_mi;
    export_to(processor, oz);
    export_to(bare, oz);
    os_db = pico8_db(processor);
    bare_db = pico8_db(&bare->header);
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
    pico8_vec_t *mp;
    int i;

    clicked_game();

    placed = build_processor();
    mp = vec_ref(vec_xy(7680, 7552));
    cls_of(placed)->hq(placed, mp, vec_ref(vec_xy(64, 64)));
    assert(ra_copies(as_qh(placed)->ra, mp));
    h = fold_vec(h, as_qh(placed)->p);
    h = fnv(h, et_state(as_qh(placed)));
    h = fold_et(h, as_qh(placed));
    set_credits(100);
    assert(cls_of(placed)->ey == pico8_d__dispatch);
    h = pico8_d_(placed) ? fnv(fnv(h, 0xB0u), 1u) : fold_ref(h, NULL);
    h = fold_string_member(h, as_qh(placed)->header.me);
    h = fold_fixed_member(h, as_qh(placed)->header.t);
    h = fnv(h, (uint32_t)credits());
    assert(strcmp(as_qh(placed)->header.me, "ly") == 0);

    h = fold_economy(h, placed);
    for (i = 0; i < 3; i++) {
        h = fold_dg(h, placed);
    }
    h = fold_economy(h, placed);
    h = fold_db(h, placed);

    idle = build_processor();
    h = fold_string_member(h, as_qh(idle)->header.me);
    h = fold_dg(h, idle);
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

static void build_with_unknown_key(void *e) {
    pico8_jo_keys_t keys;
    (void)e;
    keys.id = NULL;
    keys.p = vec_ref(vec_xy(1, 1));
    keys.ra = NULL;
    (void)pico8_os_new(&keys);
}

static int is_an_os_struct(const void *e) {
    return e != NULL &&
           ((const pico8_struct_header_t *)e)->cls == pico8_os_struct_class();
}

/* Built by `ie:ldown` or pico8_os_new, a processor is a struct whose
 * descriptor is its own but holds qh's very entries; a construction key
 * other than `id` stops the run. */
static void test_os_is_a_struct(void) {
    const pico8_struct_class_t *cls = pico8_os_struct_class();
    const pico8_struct_class_t *qh = pico8_qh_struct_class();
    void *a;

    start_game();
    a = build_processor();
    assert(is_an_os_struct(a) && cls != qh && ents_listed(pico8_cg("jo"), a) && !ents_listed(pico8_cg("qh"), a));
    assert_qh_shape(cls);
    assert_members_of(cls, qh, 1, 1);
    assert(cls->kf == qh->kf && cls->cn == qh->cn && cls->mm == qh->mm);

    must_abort(build_with_unknown_key, NULL);
    {
        pico8_jo_t *bare = pico8_os_new(NULL);
        assert(is_an_os_struct(bare) && bare->id == NULL && bare->oe != NULL &&
               ents_listed(pico8_cg("jo"), bare));
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
    in_fresh_child(test_os_is_a_struct);

    printf("test_pico8_os: all tests passed\n");
    return 0;
}
