/* The defaults of the ten classes of the `jo` tree, on a fresh struct of
 * each: its own keys are its class's generated data (pico8_game_data.h),
 * the others its parent's; and each module's `_ready` over init and
 * shutdown. */

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

/* The very string, vector or box of the generated data, not a copy. */
static void assert_same_string(const char *a, const char *b) {
    assert(a != NULL && a == b && strcmp(a, b) == 0);
}

static void assert_same_vec(const pico8_vec_t *a, const pico8_vec_t *b) {
    assert(a != NULL && a == b);
}

static void assert_same_box(const pico8_qz_box_t *a, const pico8_qz_box_t *b) {
    assert(a != NULL && a == b);
}

/* The members of jo every class of the tree inherits; kw aside, which `lz`
 * points at the planet's own when it has an `hp`. */
static void assert_jo_tail(const pico8_jo_t *jo) {
    assert(jo->ma == pico8_jo_start.ma);
    assert(jo->ka == pico8_jo_start.ka);
    assert(jo->dz == pico8_jo_start.dz);
    assert(jo->cq == pico8_jo_start.cq);
    assert_same_vec(jo->gq, &pico8_jo_start.gq);
    assert(jo->ca == pico8_jo_start.ca);
    assert(jo->em == pico8_jo_start.em);
}

/* The members of qh every class below it inherits. */
static void assert_qh_tail(const pico8_qh_t *qh) {
    assert_same_string(qh->probe->dq, pico8_qh_start.dq);
    assert(qh->jo.hf == pico8_qh_start.hf && qh->jo.hf_is_bool == pico8_qh_start.hf_is_bool);
    assert(qh->jo.mi == pico8_qh_start.mi && qh->jo.has_mi == 1);
}

static void test_jo_defaults(void) {
    pico8_jo_t *jo = pico8_jo_new(NULL);

    assert(pico8_jo_ready());
    assert(jo->nu == pico8_jo_start.nu);
    assert_same_string(jo->ke.me, pico8_jo_start.me);
    assert(jo->hf == pico8_jo_start.hf && jo->hf_is_bool == pico8_jo_start.hf_is_bool);
    assert(jo->hs == pico8_jo_start.hs);
    assert(jo->fa == pico8_jo_start.fa);
    assert_same_box(jo->kq, &pico8_jo_start.kq);
    assert_jo_tail(jo);
    assert(jo->kw == pico8_jo_shared_kw());
    assert(jo->kw->mu == pico8_jo_start.kw->mu && jo->kw->has_qe == pico8_jo_start.kw->has_qe);
    assert(jo->kw_own.mu == pico8_jo_start.kw->mu && jo->kw_own.has_qe == pico8_jo_start.kw->has_qe);
}

static void test_qh_defaults(void) {
    pico8_qh_t *qh = pico8_qh_new(NULL);

    assert(pico8_qh_ready());
    assert(qh->jo.nu == pico8_qh_start.nu);
    assert(qh->hp == pico8_qh_start.hp);
    assert(qh->px == pico8_qh_start.px);
    assert_same_string(qh->jo.ke.me, pico8_qh_start.me);
    assert(qh->qh == pico8_qh_start.qh && qh->qh_is_bool == pico8_qh_start.qh_is_bool);
    assert(qh->probe->od == pico8_qh_start.od);
    assert(qh->probe->fp == pico8_qh_start.fp);
    assert(qh->probe->dw == pico8_qh_start.dw);
    assert(qh->s == pico8_qh_start.s);
    assert(qh->ba == pico8_qh_start.ba);
    assert_qh_tail(qh);
    assert(qh->jo.hs == pico8_jo_start.hs);
    assert(qh->jo.fa == pico8_jo_start.fa);
    assert_same_box(qh->jo.kq, &pico8_jo_start.kq);
    assert_jo_tail(&qh->jo);
}

static void test_nl_defaults(void) {
    pico8_nl_t *nl = pico8_nl_new(NULL);

    assert(pico8_nl_ready());
    assert(nl->qh.jo.nu == pico8_nl_start.nu);
    assert_same_string(nl->rr, pico8_nl_start.rr);
    assert(nl->qh.qh == pico8_nl_start.qh && nl->qh.qh_is_bool == pico8_nl_start.qh_is_bool);
    assert(nl->nl == pico8_nl_start.nl);
    assert(nl->qh.hp == pico8_nl_start.hp);
    assert(nl->qh.s == pico8_nl_start.s);
    assert(nl->qh.px == pico8_nl_start.px);
    assert(nl->qh.probe->od == pico8_nl_start.od);
    assert(nl->qw == pico8_nl_start.qw);
    assert(nl->qh.ba == pico8_nl_start.ba);
    assert_same_string(nl->qh.jo.ke.me, pico8_qh_start.me);
    assert(nl->qh.probe->fp == pico8_qh_start.fp);
    assert(nl->qh.probe->dw == pico8_qh_start.dw);
    assert_qh_tail(&nl->qh);
    assert(nl->qh.jo.hs == pico8_jo_start.hs);
    assert(nl->qh.jo.fa == pico8_jo_start.fa);
    assert_same_box(nl->qh.jo.kq, &pico8_jo_start.kq);
    assert_jo_tail(&nl->qh.jo);
}

static void test_os_defaults(void) {
    pico8_os_t *os = pico8_os_new(NULL);

    assert(pico8_os_ready());
    assert(os->qh.jo.nu == pico8_os_start.nu);
    assert(os->qh.hp == pico8_os_start.hp);
    assert(os->qh.px == pico8_os_start.px);
    assert(os->qh.qh == pico8_os_start.qh && os->qh.qh_is_bool == pico8_os_start.qh_is_bool);
    assert(os->qh.s == pico8_os_start.s);
    assert(os->qh.probe->od == pico8_os_start.od);
    assert(os->qh.jo.hs == pico8_os_start.hs);
    assert_same_string(os->qh.jo.ke.me, pico8_qh_start.me);
    assert(os->qh.probe->fp == pico8_qh_start.fp);
    assert(os->qh.probe->dw == pico8_qh_start.dw);
    assert(os->qh.ba == pico8_qh_start.ba);
    assert_qh_tail(&os->qh);
    assert(os->qh.jo.fa == pico8_jo_start.fa);
    assert_same_box(os->qh.jo.kq, &pico8_jo_start.kq);
    assert_jo_tail(&os->qh.jo);
}

static void test_hj_defaults(void) {
    pico8_hj_t *hj = pico8_hj_new(NULL);

    assert(pico8_hj_ready());
    assert(hj->qh.jo.nu == pico8_hj_start.nu);
    assert_same_string(hj->rr, pico8_hj_start.rr);
    assert(hj->qh.hp == pico8_hj_start.hp);
    assert(hj->qh.px == pico8_hj_start.px);
    assert(hj->qh.probe->od == pico8_hj_start.od);
    assert(hj->qh.s == pico8_hj_start.s);
    assert(hj->emitter->py == pico8_hj_start.py);
    assert(hj->emitter->ki == pico8_hj_start.ki);
    assert(hj->emitter->pn == pico8_hj_start.pn);
    assert(hj->emitter->pk == pico8_hj_start.pk && hj->emitter->pk_is_bool == pico8_hj_start.pk_is_bool);
    assert(hj->emitter->fc == pico8_hj_start.fc);
    assert(hj->emitter->k_ == pico8_hj_start.k_);
    assert(hj->emitter->du == pico8_hj_start.du);
    assert(hj->emitter->qo == pico8_hj_start.qo);
    assert(hj->emitter->ro == pico8_hj_start.ro);
    assert(hj->emitter->qg == pico8_hj_start.qg);
    assert(hj->emitter->q_ == pico8_hj_start.q_);
    assert(hj->qh.qh == pico8_hj_start.qh && hj->qh.qh_is_bool == pico8_hj_start.qh_is_bool);
    assert_same_string(hj->qh.jo.ke.me, pico8_qh_start.me);
    assert(hj->qh.probe->fp == pico8_qh_start.fp);
    assert(hj->qh.probe->dw == pico8_qh_start.dw);
    assert(hj->qh.ba == pico8_qh_start.ba);
    assert_qh_tail(&hj->qh);
    assert(hj->qh.jo.hs == pico8_jo_start.hs);
    assert(hj->qh.jo.fa == pico8_jo_start.fa);
    assert_same_box(hj->qh.jo.kq, &pico8_jo_start.kq);
    assert_jo_tail(&hj->qh.jo);
}

static void test_pm_defaults(void) {
    pico8_pm_t *pm = pico8_pm_new(NULL);

    assert(pico8_pm_ready());
    assert_same_string(pm->fz, pico8_pm_start.fz);
    assert(pm->hj.qh.jo.nu == pico8_pm_start.nu);
    assert(pm->hj.qh.hp == pico8_pm_start.hp);
    assert(pm->hj.qh.px == pico8_pm_start.px);
    assert_same_string(pm->hj.qh.jo.ke.me, pico8_pm_start.me);
    assert(pm->hj.qh.s == pico8_pm_start.s);
    assert(pm->hj.emitter->py == pico8_pm_start.py);
    assert(pm->hj.emitter->ki == pico8_pm_start.ki);
    assert(pm->hj.emitter->pn == pico8_pm_start.pn);
    assert(pm->hj.emitter->pk == pico8_pm_start.pk && pm->hj.emitter->pk_is_bool == pico8_pm_start.pk_is_bool);
    assert_same_vec(pm->ng, &pico8_pm_start.ng);
    assert(pm->hj.emitter->du == pico8_pm_start.du);
    assert(pm->hj.emitter->fc == pico8_pm_start.fc);
    assert(pm->hj.emitter->k_ == pico8_pm_start.k_);
    assert(pm->hj.emitter->qo == pico8_pm_start.qo);
    assert(pm->hj.emitter->ro == pico8_pm_start.ro);
    assert(pm->hj.emitter->qg == pico8_pm_start.qg);
    assert(pm->hj.emitter->q_ == pico8_pm_start.q_);
    assert_same_string(pm->hj.rr, pico8_hj_start.rr);
    assert(pm->hj.qh.qh == pico8_hj_start.qh && pm->hj.qh.qh_is_bool == pico8_hj_start.qh_is_bool);
    assert(pm->hj.qh.probe->od == pico8_hj_start.od);
    assert(pm->hj.qh.probe->fp == pico8_qh_start.fp);
    assert(pm->hj.qh.probe->dw == pico8_qh_start.dw);
    assert(pm->hj.qh.ba == pico8_qh_start.ba);
    assert_qh_tail(&pm->hj.qh);
    assert(pm->hj.qh.jo.hs == pico8_jo_start.hs);
    assert(pm->hj.qh.jo.fa == pico8_jo_start.fa);
    assert_same_box(pm->hj.qh.jo.kq, &pico8_jo_start.kq);
    assert_jo_tail(&pm->hj.qh.jo);
}

static void test_ga_defaults(void) {
    pico8_ga_t *ga = pico8_ga_new(NULL);
    pico8_ga_t *other = pico8_ga_new(NULL);

    assert(pico8_ga_ready());
    assert_same_string(ga->fz, pico8_ga_start.fz);
    assert(ga->jo.nu == pico8_ga_start.nu);
    assert(ga->jo.hf == pico8_ga_start.hf && ga->jo.hf_is_bool == pico8_ga_start.hf_is_bool);
    assert(ga->jo.by != NULL && ga->jo.by == other->jo.by);
    assert_same_box(ga->jo.kq, &pico8_ga_start.kq);
    assert(ga->jo.fa == pico8_ga_start.fa);
    assert(ga->emitter->fc == pico8_ga_start.fc);
    assert(ga->emitter->qo == pico8_ga_start.qo);
    assert(ga->emitter->ro == pico8_ga_start.ro);
    assert(ga->emitter->qg == pico8_ga_start.qg);
    assert(ga->emitter->q_ == pico8_ga_start.q_);
    assert(ga->emitter->k_ == pico8_ga_start.k_);
    assert(ga->iu == pico8_ga_start.iu);
    assert(ga->emitter->py == pico8_ga_start.py);
    assert(ga->ms == pico8_ga_start.ms);
    assert_same_string(ga->jo.ke.me, pico8_jo_start.me);
    assert(ga->jo.hs == pico8_jo_start.hs);
    assert_jo_tail(&ga->jo);
    assert(ga->jo.kw == pico8_jo_shared_kw());
    assert(ga->jo.kw->mu == pico8_jo_start.kw->mu && ga->jo.kw->has_qe == pico8_jo_start.kw->has_qe);
    assert(ga->jo.kw_own.mu == pico8_jo_start.kw->mu && ga->jo.kw_own.has_qe == pico8_jo_start.kw->has_qe);
}

/* nr's own keys on a struct built by nr_struct_init, under nr or fr. */
static void assert_nr_own(const pico8_nr_t *nr) {
    assert(nr->qh.jo.nu == pico8_nr_start.nu);
    assert(nr->qh.probe->fp == pico8_nr_start.fp);
    assert_same_box(nr->qh.jo.kq, &pico8_nr_start.kq);
    assert(nr->qh.probe->dw == pico8_nr_start.dw);
}

/* What nr and fr inherit from qh and jo. */
static void assert_nr_inherited(const pico8_nr_t *nr) {
    assert(nr->qh.hp == pico8_qh_start.hp);
    assert(nr->qh.px == pico8_qh_start.px);
    assert_same_string(nr->qh.jo.ke.me, pico8_qh_start.me);
    assert(nr->qh.qh == pico8_qh_start.qh && nr->qh.qh_is_bool == pico8_qh_start.qh_is_bool);
    assert(nr->qh.ba == pico8_qh_start.ba);
    assert_qh_tail(&nr->qh);
    assert(nr->qh.jo.hs == pico8_jo_start.hs);
    assert(nr->qh.jo.fa == pico8_jo_start.fa);
    assert_jo_tail(&nr->qh.jo);
}

static void test_nr_defaults(void) {
    pico8_nr_t *nr = pico8_nr_new(NULL);

    assert(pico8_nr_ready());
    assert_nr_own(nr);
    assert(nr->qh.probe->od == pico8_nr_start.od);
    assert(nr->qh.s == pico8_nr_start.s);
    assert(nr->hz == pico8_nr_start.hz);
    assert_same_string(nr->rr, pico8_nr_start.rr);
    assert_nr_inherited(nr);
}

static void test_fr_defaults(void) {
    pico8_fr_t *fr = pico8_fr_new(NULL);

    assert(pico8_fr_ready());
    assert(fr->mt == pico8_fr_start.mt);
    assert_same_string(fr->nr.rr, pico8_fr_start.rr);
    assert(fr->nr.qh.probe->od == pico8_fr_start.od);
    assert(fr->nr.qh.s == pico8_fr_start.s);
    assert(fr->nr.hz == pico8_fr_start.hz);
    assert_nr_own(&fr->nr);
    assert_nr_inherited(&fr->nr);
}

static void test_mx_defaults(void) {
    pico8_mx_t *mx = pico8_mx_new(NULL);

    assert(pico8_mx_ready());
    assert(mx->hj.qh.jo.nu == pico8_mx_start.nu);
    assert_same_string(mx->hj.rr, pico8_mx_start.rr);
    assert(mx->hj.qh.hp == pico8_mx_start.hp);
    assert(mx->hj.qh.s == pico8_mx_start.s);
    assert(mx->hj.qh.px == pico8_mx_start.px);
    assert(mx->hj.qh.probe->od == pico8_mx_start.od);
    assert(mx->ht == pico8_mx_start.ht);
    assert(mx->hj.emitter->py == pico8_mx_start.py);
    assert(mx->hj.emitter->ki == pico8_mx_start.ki);
    assert(mx->hj.emitter->pn == pico8_mx_start.pn);
    assert(mx->hj.emitter->pk == pico8_mx_start.pk && mx->hj.emitter->pk_is_bool == pico8_mx_start.pk_is_bool);
    assert(mx->hj.emitter->fc == pico8_mx_start.fc);
    assert(mx->hj.emitter->k_ == pico8_mx_start.k_);
    assert(mx->hj.emitter->du == pico8_mx_start.du);
    assert(mx->hj.emitter->qo == pico8_mx_start.qo);
    assert(mx->hj.emitter->ro == pico8_mx_start.ro);
    assert(mx->hj.emitter->qg == pico8_mx_start.qg);
    assert(mx->hj.emitter->q_ == pico8_mx_start.q_);
    assert(mx->hj.qh.qh == pico8_hj_start.qh && mx->hj.qh.qh_is_bool == pico8_hj_start.qh_is_bool);
    assert_same_string(mx->hj.qh.jo.ke.me, pico8_qh_start.me);
    assert(mx->hj.qh.probe->fp == pico8_qh_start.fp);
    assert(mx->hj.qh.probe->dw == pico8_qh_start.dw);
    assert(mx->hj.qh.ba == pico8_qh_start.ba);
    assert_qh_tail(&mx->hj.qh);
    assert(mx->hj.qh.jo.hs == pico8_jo_start.hs);
    assert(mx->hj.qh.jo.fa == pico8_jo_start.fa);
    assert_same_box(mx->hj.qh.jo.kq, &pico8_jo_start.kq);
    assert_jo_tail(&mx->hj.qh.jo);
}

/* The second `jl()` (the one `eu` runs) finds `ke` ready and leaves it
 * alone: a fresh struct of each module still holds ke's `t`. */
static void test_second_jl_keeps_ke(void) {
    assert(pico8_ke_ready());
    pico8_jl();
    assert(pico8_ke_ready());
    assert(pico8_jo_new(NULL)->ke.t == pico8_ke_start.t);
    assert(pico8_qh_new(NULL)->jo.ke.t == pico8_ke_start.t);
    assert(pico8_nl_new(NULL)->qh.jo.ke.t == pico8_ke_start.t);
    assert(pico8_os_new(NULL)->qh.jo.ke.t == pico8_ke_start.t);
    assert(pico8_hj_new(NULL)->qh.jo.ke.t == pico8_ke_start.t);
    assert(pico8_pm_new(NULL)->hj.qh.jo.ke.t == pico8_ke_start.t);
    assert(pico8_ga_new(NULL)->jo.ke.t == pico8_ke_start.t);
    assert(pico8_nr_new(NULL)->qh.jo.ke.t == pico8_ke_start.t);
    assert(pico8_fr_new(NULL)->nr.qh.jo.ke.t == pico8_ke_start.t);
    assert(pico8_mx_new(NULL)->hj.qh.jo.ke.t == pico8_ke_start.t);
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
