/* The defaults of the ten classes of the `jo` tree, on a fresh struct of
 * each, against its kind's archetype; and each module's `_ready` over init
 * and shutdown. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_ke.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

/* The very string of the generated data, not a copy. */
static void assert_same_string(const char *a, const char *b) {
    assert(a != NULL && a == b && strcmp(a, b) == 0);
}

/* The members of jo every class of the tree inherits, and its kind's
 * archetype; kw aside, which `lz` points at the planet's own when it has an
 * `hp`. */
static void assert_jo_tail(const pico8_jo_t *jo, const pico8_jo_def_t *def) {
    assert(jo->def == def);
    assert(jo->ma == def->ma);
    assert(jo->mi == def->mi && jo->has_mi == def->has_mi);
}

static void assert_box(const pico8_qz_box_t *kq, int xl, int yt, int xr, int yb) {
    assert(kq->xl == pico8_fixed_from_int(xl) && kq->yt == pico8_fixed_from_int(yt) &&
           kq->xr == pico8_fixed_from_int(xr) && kq->yb == pico8_fixed_from_int(yb));
}

/* jo's box, which ga and nr replace. */
static void assert_jo_box(const pico8_qz_box_t *kq) {
    assert_box(kq, -7, -7, 7, 7);
}

/* The members of qh every class below it inherits, its probe's archetype, and
 * the probe's init literals. */
static void assert_qh_tail(const pico8_qh_t *qh, const pico8_jo_def_t *def) {
    assert(qh->probe->def == &def->probe);
    assert(qh->jo.mi == def->mi && qh->jo.has_mi == 1);
    assert(qh->probe->has_od == 1 && qh->probe->def->ik == 0 && qh->probe->def->has_ik == 0);
}

/* The emitter of an ascension, pm or mx, and its archetype. */
static void assert_emitter(const pico8_emitter_t *em, const pico8_jo_def_t *def) {
    assert(em->def == &def->emitter);
    assert(em->py == def->emitter.py && em->pn == def->emitter.pn && em->k_ == def->emitter.k_);
    assert(em->du == def->emitter.du && em->qo == def->emitter.qo && em->ro == def->emitter.ro);
    assert(em->qg == def->emitter.qg && em->q_ == def->emitter.q_);
}

static void test_jo_defaults(void) {
    pico8_jo_t *jo = pico8_jo_new(NULL);

    assert(pico8_jo_ready());
    assert_same_string(jo->header.me, jo->def->me);
    assert_jo_box(&jo->def->kq);
    assert_jo_tail(jo, &pico8_jo_def);
    assert(jo->def->ka == &pico8_jo_ka_seq && jo->def->dz == pico8_jo_dz_rows && jo->def->cq == &pico8_jo_cq_seq);
    assert(jo->mi == 0 && !jo->has_mi);
    assert(jo->kw == pico8_jo_shared_kw());
    assert(jo->kw->mu == 0 && !jo->kw->has_qe);
    assert(jo->kw_own.mu == 0 && !jo->kw_own.has_qe && jo->kw_own.qe == 0);
}

static void test_qh_defaults(void) {
    pico8_qh_t *qh = pico8_qh_new(NULL);

    assert(pico8_qh_ready());
    assert_same_string(qh->jo.header.me, pico8_qh_def.me);
    assert(qh->probe->od == pico8_qh_def.probe.od);
    assert(qh->s == pico8_qh_def.s);
    assert_qh_tail(qh, &pico8_qh_def);
    assert_jo_box(&qh->jo.def->kq);
    assert_jo_tail(&qh->jo, &pico8_qh_def);
}

static void test_nl_defaults(void) {
    pico8_nl_t *nl = pico8_nl_new(NULL);

    assert(pico8_nl_ready());
    assert(nl->qh.s == pico8_nl_def.s);
    assert(nl->qh.probe->od == pico8_nl_def.probe.od);
    assert(nl->qw == pico8_nl_def.qw);
    assert_same_string(nl->qh.jo.header.me, pico8_nl_def.me);
    assert_qh_tail(&nl->qh, &pico8_nl_def);
    assert_jo_box(&nl->qh.jo.def->kq);
    assert_jo_tail(&nl->qh.jo, &pico8_nl_def);
}

static void test_os_defaults(void) {
    pico8_os_t *os = pico8_os_new(NULL);

    assert(pico8_os_ready());
    assert(os->qh.s == pico8_os_def.s);
    assert(os->qh.probe->od == pico8_os_def.probe.od);
    assert_same_string(os->qh.jo.header.me, pico8_os_def.me);
    assert_qh_tail(&os->qh, &pico8_os_def);
    assert_jo_box(&os->qh.jo.def->kq);
    assert_jo_tail(&os->qh.jo, &pico8_os_def);
}

static void test_hj_defaults(void) {
    pico8_hj_t *hj = pico8_hj_new(NULL);

    assert(pico8_hj_ready());
    assert(hj->qh.probe->od == pico8_hj_def.probe.od);
    assert(hj->qh.s == pico8_hj_def.s);
    assert_emitter(hj->emitter, &pico8_hj_def);
    assert_same_string(hj->qh.jo.header.me, pico8_hj_def.me);
    assert_qh_tail(&hj->qh, &pico8_hj_def);
    assert_jo_box(&hj->qh.jo.def->kq);
    assert_jo_tail(&hj->qh.jo, &pico8_hj_def);
}

static void test_pm_defaults(void) {
    pico8_pm_t *pm = pico8_pm_new(NULL);

    assert(pico8_pm_ready());
    assert_same_string(pm->hj.qh.jo.header.me, pico8_pm_def.me);
    assert(pm->hj.qh.s == pico8_pm_def.s);
    assert_emitter(pm->hj.emitter, &pico8_pm_def);
    assert(pm->hj.qh.probe->od == pico8_pm_def.probe.od);
    assert_qh_tail(&pm->hj.qh, &pico8_pm_def);
    assert_jo_box(&pm->hj.qh.jo.def->kq);
    assert_jo_tail(&pm->hj.qh.jo, &pico8_pm_def);
}

static void test_ga_defaults(void) {
    pico8_ga_t *ga = pico8_ga_new(NULL);
    pico8_ga_t *other = pico8_ga_new(NULL);

    assert(pico8_ga_ready());
    assert(ga->jo.by != NULL && ga->jo.by == other->jo.by);
    assert(ga->emitter->def == &pico8_ga_def.emitter);
    assert(ga->emitter->qo == pico8_ga_def.emitter.qo);
    assert(ga->emitter->ro == pico8_ga_def.emitter.ro);
    assert(ga->emitter->qg == pico8_ga_def.emitter.qg);
    assert(ga->emitter->q_ == pico8_ga_def.emitter.q_);
    assert(ga->emitter->k_ == pico8_ga_def.emitter.k_);
    assert(ga->iu == pico8_ga_def.iu);
    assert(ga->emitter->py == pico8_ga_def.emitter.py);
    assert(ga->ms == pico8_ga_def.ms);
    assert_box(&ga->jo.def->kq, 0, 0, -1, -1);
    assert_same_string(ga->jo.header.me, ga->jo.def->me);
    assert_jo_tail(&ga->jo, &pico8_ga_def);
    assert(ga->jo.def->hs == pico8_fixed_from_int(1));
    assert(ga->jo.mi == 0 && !ga->jo.has_mi);
    assert(ga->jo.kw == pico8_jo_shared_kw());
    assert(ga->jo.kw->mu == 0 && !ga->jo.kw->has_qe);
    assert(ga->jo.kw_own.mu == 0 && !ga->jo.kw_own.has_qe && ga->jo.kw_own.qe == 0);
}

/* What nr and fr inherit from qh and jo, and nr's box, which fr keeps. */
static void assert_nr_inherited(const pico8_nr_t *nr, const pico8_jo_def_t *def) {
    assert(nr->qh.probe->od == def->probe.od);
    assert(nr->qh.s == def->s);
    assert_box(&nr->qh.jo.def->kq, -4, -4, 5, 5);
    assert_same_string(nr->qh.jo.header.me, def->me);
    assert_qh_tail(&nr->qh, def);
    assert_jo_tail(&nr->qh.jo, def);
}

static void test_nr_defaults(void) {
    pico8_nr_t *nr = pico8_nr_new(NULL);

    assert(pico8_nr_ready());
    assert_nr_inherited(nr, &pico8_nr_def);
}

static void test_fr_defaults(void) {
    pico8_fr_t *fr = pico8_fr_new(NULL);

    assert(pico8_fr_ready());
    assert_nr_inherited(&fr->nr, &pico8_fr_def);
}

static void test_mx_defaults(void) {
    pico8_mx_t *mx = pico8_mx_new(NULL);

    assert(pico8_mx_ready());
    assert(mx->hj.qh.s == pico8_mx_def.s);
    assert(mx->hj.qh.probe->od == pico8_mx_def.probe.od);
    assert(mx->ht == pico8_mx_def.ht);
    assert_emitter(mx->hj.emitter, &pico8_mx_def);
    assert_same_string(mx->hj.qh.jo.header.me, pico8_mx_def.me);
    assert_qh_tail(&mx->hj.qh, &pico8_mx_def);
    assert_jo_box(&mx->hj.qh.jo.def->kq);
    assert_jo_tail(&mx->hj.qh.jo, &pico8_mx_def);
}

/* The second `jl()` (the one `eu` runs) finds `ke` ready and leaves it
 * alone: a fresh struct of each module still holds ke's `t`. */
static void test_second_jl_keeps_ke(void) {
    assert(pico8_ke_ready());
    pico8_jl();
    assert(pico8_ke_ready());
    assert(pico8_jo_new(NULL)->header.t == pico8_ke_start.t);
    assert(pico8_qh_new(NULL)->jo.header.t == pico8_ke_start.t);
    assert(pico8_nl_new(NULL)->qh.jo.header.t == pico8_ke_start.t);
    assert(pico8_os_new(NULL)->qh.jo.header.t == pico8_ke_start.t);
    assert(pico8_hj_new(NULL)->qh.jo.header.t == pico8_ke_start.t);
    assert(pico8_pm_new(NULL)->hj.qh.jo.header.t == pico8_ke_start.t);
    assert(pico8_ga_new(NULL)->jo.header.t == pico8_ke_start.t);
    assert(pico8_nr_new(NULL)->qh.jo.header.t == pico8_ke_start.t);
    assert(pico8_fr_new(NULL)->nr.qh.jo.header.t == pico8_ke_start.t);
    assert(pico8_mx_new(NULL)->hj.qh.jo.header.t == pico8_ke_start.t);
    assert(strcmp(pico8_ke_start.me, "ph") == 0 && pico8_ke_start.t == 0);
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

    assert(!pico8_jo_ready());
    assert(!pico8_qh_ready());
    assert(!pico8_nl_ready());
    assert(!pico8_os_ready());
    assert(!pico8_hj_ready());
    assert(!pico8_pm_ready());
    assert(!pico8_ga_ready());
    assert(!pico8_nr_ready());
    assert(!pico8_fr_ready());
    assert(!pico8_mx_ready());
    pico8_cart_load();
    pico8_jl();

    test_second_jl_keeps_ke();
    test_jo_defaults();
    test_qh_defaults();
    test_nl_defaults();
    test_os_defaults();
    test_hj_defaults();
    test_pm_defaults();
    test_ga_defaults();
    test_nr_defaults();
    test_fr_defaults();
    test_mx_defaults();

    pico8_planet_family_trade_shutdown();
    assert(!pico8_nr_ready());
    assert(!pico8_fr_ready());
    assert(!pico8_mx_ready());
    assert(pico8_qh_ready());
    pico8_planet_family_shutdown();
    assert(!pico8_qh_ready());
    assert(!pico8_nl_ready());
    assert(!pico8_os_ready());
    assert(!pico8_hj_ready());
    assert(!pico8_pm_ready());
    assert(!pico8_ga_ready());
    assert(pico8_jo_ready());
    pico8_jo_shutdown();
    assert(!pico8_jo_ready());
    assert(pico8_ke_ready());
    pico8_ke_shutdown();
    assert(!pico8_ke_ready());

    printf("test_pico8_jo_spec_defaults: all tests passed\n");
    return 0;
}
