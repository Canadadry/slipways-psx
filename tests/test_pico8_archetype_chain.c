/* Each archetype against the value its class's init chain leaves in a fresh
 * instance, field by field. */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bg.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy.h"
#include "../src/pico8_game_data.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_wormhole.h"

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static int g_compared;

static void same(int ok, const char *what) {
    if (!ok) {
        fprintf(stderr, "archetype differs from its chain: %s\n", what);
    }
    assert(ok);
    g_compared++;
}

static void same_in(int ok, const char *kind, const char *field) {
    char what[32];

    snprintf(what, sizeof what, "%s.%s", kind, field);
    same(ok, what);
}

static int same_text(const char *got, const char *want) {
    return got != NULL && want != NULL && strcmp(got, want) == 0;
}

static int same_text_or_nil(const char *got, const char *want) {
    return (got == NULL && want == NULL) || same_text(got, want);
}

static int same_vec(const pico8_vec_t *got, const pico8_vec_t *want) {
    return got != NULL && got->x == want->x && got->y == want->y;
}

static int same_vec_or_nil(const pico8_vec_t *got, const pico8_vec_t *want) {
    return (got == NULL && want == NULL) || (want != NULL && same_vec(got, want));
}

static int same_box(const pico8_qz_box_t *got, const pico8_qz_box_t *want) {
    return got != NULL && got->xl == want->xl && got->yt == want->yt && got->xr == want->xr && got->yb == want->yb;
}

static void test_bg(void) {
    const pico8_bg_t *bg = pico8_bg_new();

    same(same_text(bg->header.me, pico8_bg_def.me), "bg.me");
    same(bg->header.t == pico8_bg_def.t, "bg.t");
    same(bg->fa == pico8_bg_def.fa, "bg.fa");
    same(bg->hd == pico8_bg_def.hd, "bg.hd");
    same(bg->pl == pico8_bg_def.pl, "bg.pl");
}

/* `le` is the module's shared list, not the archetype's. */
static void test_ui(void) {
    const pico8_ui_t *ui = pico8_ui_new();

    same(same_text(ui->header.me, pico8_ui_def.me), "ui.me");
    same(ui->header.t == pico8_ui_def.t, "ui.t");
    same(ui->ql == pico8_ui_def.ql, "ui.ql");
    same(ui->fa == pico8_ui_def.fa, "ui.fa");
    same(same_box(ui->hw, &pico8_ui_def.hw), "ui.hw");
    assert(ui->le == pico8_ui_le_shared());
}

static void test_js(void) {
    const pico8_js_t *js = pico8_js_new(NULL);

    same(same_text(js->header.me, pico8_js_def.me), "js.me");
    same(js->header.t == pico8_js_def.t, "js.t");
    same(js->fa == pico8_js_def.fa, "js.fa");
}

static void test_fq(void) {
    const pico8_fq_t *fq = pico8_fq_new(NULL);

    same(same_text(fq->header.me, pico8_fq_def.me), "fq.me");
    same(fq->header.t == pico8_fq_def.t, "fq.t");
    same(fq->fa == pico8_fq_def.fa, "fq.fa");
    same(fq->hd == pico8_fq_def.hd, "fq.hd");
}

static void test_qb(void) {
    const pico8_qb_t *qb = pico8_qb_new();

    same(same_text(qb->header.me, pico8_qb_def.me), "qb.me");
    same(qb->header.t == pico8_qb_def.t, "qb.t");
    same(same_vec(qb->v, &pico8_qb_def.v), "qb.v");
    same(qb->ma == pico8_qb_def.ma, "qb.ma");
    same(same_vec(qb->p, &pico8_qb_def.p), "qb.p");
    same(same_vec(qb->ra, &pico8_qb_def.ra), "qb.ra");
    same(qb->pc == pico8_qb_def.pc, "qb.pc");
}

/* `ra` is the module's shared vector, not the archetype's: two fresh
 * readers point at the same one. */
static void test_nf(void) {
    const pico8_nf_t *nf = pico8_nf_new();
    const pico8_nf_t *other = pico8_nf_new();

    same(same_text(nf->header.me, pico8_nf_def.me), "nf.me");
    same(nf->header.t == pico8_nf_def.t, "nf.t");
    assert(nf->ra != NULL && nf->ra == other->ra);
}

static void test_mh(void) {
    const pico8_mh_t *mh = pico8_mh_new(NULL);

    same(same_text(mh->header.me, pico8_mh_def.me), "mh.me");
    same(mh->header.t == pico8_mh_def.t, "mh.t");
    same(mh->fa == pico8_mh_def.fa, "mh.fa");
    same(mh->rh == pico8_mh_def.rh, "mh.rh");
}

static void test_iv(void) {
    const pico8_iv_t *iv = pico8_iv_new(NULL);

    same(same_text(iv->header.me, pico8_iv_def.me), "iv.me");
    same(iv->header.t == pico8_iv_def.t, "iv.t");
    same(iv->fa == pico8_iv_def.fa, "iv.fa");
    same(iv->l == pico8_iv_def.l, "iv.l");
    same(same_vec(iv->v, &pico8_iv_def.v), "iv.v");
}

/* hp does not descend from ke: no `me`/`t`. */
static void test_hp(void) {
    const pico8_hp_t *hp = pico8_hp_new(NULL);

    same(hp->fk == pico8_hp_def.fk, "hp.fk");
}

static void test_ie(void) {
    const pico8_ie_t *ie = pico8_ie_new(NULL);

    same(same_text(ie->header.me, pico8_ie_def.me), "ie.me");
    same(ie->header.t == pico8_ie_def.t, "ie.t");
    same(same_box(ie->kq, &pico8_ie_def.kq), "ie.kq");
    same(ie->fa == pico8_ie_def.fa, "ie.fa");
    same(same_vec(ie->ra, &pico8_ie_def.ra), "ie.ra");
    same(same_vec(ie->eh, &pico8_ie_def.eh), "ie.eh");
}

static void test_pj(void) {
    const pico8_pj_t *pj = pico8_pj_new();

    same(same_text(pj->header.me, pico8_pj_def.me), "pj.me");
    same(pj->header.t == pico8_pj_def.t, "pj.t");
    same(pj->fa == pico8_pj_def.fa, "pj.fa");
    same(pj->gs == pico8_pj_def.gs, "pj.gs");
    same(pj->i_ == pico8_pj_def.i_, "pj.i_");
}

/* hy does not descend from ke: no `me`/`t`. */
static void test_hy(void) {
    const pico8_hy_t *hy = pico8_hy_new();

    same(hy->nz == pico8_hy_def.nz, "hy.nz");
    same(hy->ew == pico8_hy_def.ew, "hy.ew");
    same(hy->lo == pico8_hy_def.lo, "hy.lo");
    same(hy->yr == pico8_hy_def.yr, "hy.yr");
}

/* `jj` is the module's shared wrapper, not the archetype's: two fresh
 * wormholes point at the same one. */
static void test_mz(void) {
    const pico8_mz_t *mz = pico8_mz_new();
    const pico8_mz_t *other = pico8_mz_new();

    same(same_text(mz->header.me, pico8_mz_def.me), "mz.me");
    same(mz->header.t == pico8_mz_def.t, "mz.t");
    same(mz->spawn_snd == pico8_mz_def.spawn_snd, "mz.spawn_snd");
    same(mz->oi == pico8_mz_def.oi, "mz.oi");
    same(mz->fa == pico8_mz_def.fa, "mz.fa");
    same(mz->kk == pico8_mz_def.kk, "mz.kk");
    same(mz->probe->dw == pico8_mz_def.probe.dw, "mz.probe.dw");
    same(mz->probe->od == pico8_mz_def.probe.od, "mz.probe.od");
    same(mz->probe->has_od == pico8_mz_def.probe.has_od, "mz.probe.has_od");
    same(same_text_or_nil(mz->probe->dq, pico8_mz_def.probe.dq), "mz.probe.dq");
    same(mz->probe->fp == pico8_mz_def.probe.fp, "mz.probe.fp");
    same(mz->probe->ik == pico8_mz_def.probe.ik, "mz.probe.ik");
    same(mz->probe->has_ik == pico8_mz_def.probe.has_ik, "mz.probe.has_ik");
    assert(mz->jj != NULL && mz->jj == other->jj);
    assert(mz->probe->et == NULL);
}

static void test_jc(void) {
    const pico8_jc_t *jc = pico8_jc_new(NULL);

    same(same_text(jc->header.me, pico8_jc_def.me), "jc.me");
    same(jc->header.t == pico8_jc_def.t, "jc.t");
    same(jc->fa == pico8_jc_def.fa, "jc.fa");
    same(jc->k_ == pico8_jc_def.k_, "jc.k_");
    same(jc->nh == pico8_jc_def.nh, "jc.nh");
    same(jc->probe->dw == pico8_jc_def.probe.dw, "jc.probe.dw");
    same(jc->probe->od == pico8_jc_def.probe.od, "jc.probe.od");
    same(jc->probe->has_od == pico8_jc_def.probe.has_od, "jc.probe.has_od");
    same(same_text_or_nil(jc->probe->dq, pico8_jc_def.probe.dq), "jc.probe.dq");
    same(jc->probe->fp == pico8_jc_def.probe.fp, "jc.probe.fp");
    same(jc->probe->ik == pico8_jc_def.probe.ik, "jc.probe.ik");
    same(jc->probe->has_ik == pico8_jc_def.probe.has_ik, "jc.probe.has_ik");
    assert(jc->probe->et == NULL);
}

/* `qy` is the fresh particle list `f_` gives every emitter. */
static void test_g_(void) {
    const pico8_g__t *g_ = pico8_g__new(NULL);

    same(same_text(g_->header.me, pico8_g__def.me), "g_.me");
    same(g_->header.t == pico8_g__def.t, "g_.t");
    same(g_->fx == pico8_g__def.fx, "g_.fx");
    same(g_->fa == pico8_g__def.fa, "g_.fa");
    same(same_vec_or_nil(g_->ra, pico8_g__def.has_ra ? &pico8_g__def.ra : NULL), "g_.ra");
    same(g_->emitter->pn == pico8_g__def.emitter.pn, "g_.emitter.pn");
    same(g_->emitter->du == pico8_g__def.emitter.du, "g_.emitter.du");
    same(g_->emitter->py == pico8_g__def.emitter.py, "g_.emitter.py");
    same(g_->emitter->ki == pico8_g__def.emitter.ki, "g_.emitter.ki");
    same(g_->emitter->pk == pico8_g__def.emitter.pk, "g_.emitter.pk");
    same(g_->emitter->fc == pico8_g__def.emitter.fc, "g_.emitter.fc");
    same(g_->emitter->k_ == pico8_g__def.emitter.k_, "g_.emitter.k_");
    same(g_->emitter->qo == pico8_g__def.emitter.qo, "g_.emitter.qo");
    same(g_->emitter->ro == pico8_g__def.emitter.ro, "g_.emitter.ro");
    same(g_->emitter->qg == pico8_g__def.emitter.qg, "g_.emitter.qg");
    same(g_->emitter->q_ == pico8_g__def.emitter.q_, "g_.emitter.q_");
    same(g_->probe->dw == pico8_g__def.probe.dw, "g_.probe.dw");
    same(g_->probe->od == pico8_g__def.probe.od, "g_.probe.od");
    same(g_->probe->has_od == pico8_g__def.probe.has_od, "g_.probe.has_od");
    same(same_text_or_nil(g_->probe->dq, pico8_g__def.probe.dq), "g_.probe.dq");
    same(g_->probe->fp == pico8_g__def.probe.fp, "g_.probe.fp");
    same(g_->probe->ik == pico8_g__def.probe.ik, "g_.probe.ik");
    same(g_->probe->has_ik == pico8_g__def.probe.has_ik, "g_.probe.has_ik");
    assert(g_->emitter->qy != NULL && g_->emitter->qy->count == 0);
    assert(g_->probe->et == NULL);
}

static void test_fv(void) {
    const pico8_fv_t *fv = pico8_fv_new();

    same(same_text(fv->g_.header.me, pico8_fv_def.me), "fv.me");
    same(fv->g_.header.t == pico8_fv_def.t, "fv.t");
    same(fv->g_.fx == pico8_fv_def.fx, "fv.fx");
    same(fv->g_.fa == pico8_fv_def.fa, "fv.fa");
    same(same_vec_or_nil(fv->g_.ra, pico8_fv_def.has_ra ? &pico8_fv_def.ra : NULL), "fv.ra");
    same(same_vec(fv->hi, &pico8_fv_def.hi), "fv.hi");
    same(fv->ca == pico8_fv_def.ca, "fv.ca");
    same(same_vec(fv->gq, &pico8_fv_def.gq), "fv.gq");
    same(fv->eo == pico8_fv_def.eo, "fv.eo");
    same(fv->ff == pico8_fv_def.ff, "fv.ff");
    same(same_box(fv->kq, &pico8_fv_def.kq), "fv.kq");
    same(fv->ql == pico8_fv_def.ql, "fv.ql");
    same(fv->fg == pico8_fv_def.fg, "fv.fg");
    same(fv->rb == pico8_fv_def.rb, "fv.rb");
    same(fv->g_.emitter->pn == pico8_fv_def.emitter.pn, "fv.emitter.pn");
    same(fv->g_.emitter->du == pico8_fv_def.emitter.du, "fv.emitter.du");
    same(fv->g_.emitter->py == pico8_fv_def.emitter.py, "fv.emitter.py");
    same(fv->g_.emitter->ki == pico8_fv_def.emitter.ki, "fv.emitter.ki");
    same(fv->g_.emitter->pk == pico8_fv_def.emitter.pk, "fv.emitter.pk");
    same(fv->g_.emitter->fc == pico8_fv_def.emitter.fc, "fv.emitter.fc");
    same(fv->g_.emitter->k_ == pico8_fv_def.emitter.k_, "fv.emitter.k_");
    same(fv->g_.emitter->qo == pico8_fv_def.emitter.qo, "fv.emitter.qo");
    same(fv->g_.emitter->ro == pico8_fv_def.emitter.ro, "fv.emitter.ro");
    same(fv->g_.emitter->qg == pico8_fv_def.emitter.qg, "fv.emitter.qg");
    same(fv->g_.emitter->q_ == pico8_fv_def.emitter.q_, "fv.emitter.q_");
    same(fv->g_.probe->dw == pico8_fv_def.probe.dw, "fv.probe.dw");
    same(fv->g_.probe->od == pico8_fv_def.probe.od, "fv.probe.od");
    same(fv->g_.probe->has_od == pico8_fv_def.probe.has_od, "fv.probe.has_od");
    same(same_text_or_nil(fv->g_.probe->dq, pico8_fv_def.probe.dq), "fv.probe.dq");
    same(fv->g_.probe->fp == pico8_fv_def.probe.fp, "fv.probe.fp");
    same(fv->g_.probe->ik == pico8_fv_def.probe.ik, "fv.probe.ik");
    same(fv->g_.probe->has_ik == pico8_fv_def.probe.has_ik, "fv.probe.has_ik");
    assert(fv->g_.emitter->qy != NULL && fv->g_.emitter->qy->count == 0);
    assert(fv->g_.probe->et == NULL);
}

/* `kw`, `kw_own` and `by` are not the archetype's: left to each kind's test. */
static void same_jo_part(const pico8_jo_t *jo, const pico8_jo_def_t *def, const char *kind) {
    same_in(same_text(jo->header.me, def->me), kind, "me");
    same_in(jo->header.t == def->t, kind, "t");
    same_in(jo->hf == def->hf, kind, "hf");
    same_in(jo->hs == def->hs, kind, "hs");
    same_in(jo->fa == def->fa, kind, "fa");
    same_in(same_box(jo->kq, &def->kq), kind, "kq");
    same_in(jo->ma == def->ma, kind, "ma");
    same_in(jo->ka == def->ka, kind, "ka");
    same_in(jo->dz == def->dz, kind, "dz");
    same_in(jo->cq == def->cq, kind, "cq");
    same_in(same_vec(jo->gq, &def->gq), kind, "gq");
    same_in(jo->ca == def->ca, kind, "ca");
    same_in(jo->em == def->em, kind, "em");
    same_in(jo->mi == def->mi, kind, "mi");
    same_in(jo->has_mi == def->has_mi, kind, "has_mi");
}

static void same_qh_part(const pico8_qh_t *qh, const pico8_jo_def_t *def, const char *kind) {
    same_in(qh->hp == def->hp, kind, "hp");
    same_in(qh->px == def->px, kind, "px");
    same_in(qh->qh == def->qh, kind, "qh");
    same_in(qh->s == def->s, kind, "s");
    same_in(qh->ba == def->ba, kind, "ba");
    same_in(qh->probe->dw == def->probe.dw, kind, "probe.dw");
    same_in(qh->probe->od == def->probe.od, kind, "probe.od");
    same_in(qh->probe->has_od == def->probe.has_od, kind, "probe.has_od");
    same_in(same_text_or_nil(qh->probe->dq, def->probe.dq), kind, "probe.dq");
    same_in(qh->probe->fp == def->probe.fp, kind, "probe.fp");
    same_in(qh->probe->ik == def->probe.ik, kind, "probe.ik");
    same_in(qh->probe->has_ik == def->probe.has_ik, kind, "probe.has_ik");
    assert(qh->probe->et == NULL);
    assert(qh->jo.by == NULL);
}

/* A kind of the qh tree has an `hp`: its `_new` runs `lz`, which fills
 * `kw_own` from that research and points `kw` at it. */
static void assert_lz_ran(const pico8_jo_t *jo) {
    assert(jo->hn != NULL && jo->kw == &jo->kw_own);
}

/* `kw` is the module's shared kw, not the archetype's. */
static void test_jo(void) {
    const pico8_jo_t *jo = pico8_jo_new(NULL);

    same_jo_part(jo, &pico8_jo_def, "jo");
    assert(jo->kw == pico8_jo_shared_kw());
    assert(jo->by == NULL);
}

static void test_qh(void) {
    const pico8_qh_t *qh = pico8_qh_new(NULL);

    same_jo_part(&qh->jo, &pico8_qh_def, "qh");
    same_qh_part(qh, &pico8_qh_def, "qh");
    assert_lz_ran(&qh->jo);
}

static void test_nl(void) {
    const pico8_nl_t *nl = pico8_nl_new(NULL);

    same_jo_part(&nl->qh.jo, &pico8_nl_def, "nl");
    same_qh_part(&nl->qh, &pico8_nl_def, "nl");
    same_in(same_text(nl->rr, pico8_nl_def.rr), "nl", "rr");
    same_in(nl->nl == pico8_nl_def.nl, "nl", "nl");
    same_in(nl->qw == pico8_nl_def.qw, "nl", "qw");
    assert_lz_ran(&nl->qh.jo);
}

static void test_os(void) {
    const pico8_os_t *os = pico8_os_new(NULL);

    same_jo_part(&os->qh.jo, &pico8_os_def, "os");
    same_qh_part(&os->qh, &pico8_os_def, "os");
    assert_lz_ran(&os->qh.jo);
}

/* `qy` is left to each kind's test. */
static void same_emitter_part(const pico8_emitter_t *emitter, const pico8_jo_def_t *def, const char *kind) {
    same_in(emitter->pn == def->emitter.pn, kind, "emitter.pn");
    same_in(emitter->du == def->emitter.du, kind, "emitter.du");
    same_in(emitter->py == def->emitter.py, kind, "emitter.py");
    same_in(emitter->ki == def->emitter.ki, kind, "emitter.ki");
    same_in(emitter->pk == def->emitter.pk, kind, "emitter.pk");
    same_in(emitter->fc == def->emitter.fc, kind, "emitter.fc");
    same_in(emitter->k_ == def->emitter.k_, kind, "emitter.k_");
    same_in(emitter->qo == def->emitter.qo, kind, "emitter.qo");
    same_in(emitter->ro == def->emitter.ro, kind, "emitter.ro");
    same_in(emitter->qg == def->emitter.qg, kind, "emitter.qg");
    same_in(emitter->q_ == def->emitter.q_, kind, "emitter.q_");
}

/* `qy` is the fresh particle list `f_` gives every emitter. */
static void test_hj(void) {
    const pico8_hj_t *hj = pico8_hj_new(NULL);

    same_jo_part(&hj->qh.jo, &pico8_hj_def, "hj");
    same_qh_part(&hj->qh, &pico8_hj_def, "hj");
    same_in(same_text(hj->rr, pico8_hj_def.rr), "hj", "rr");
    same_emitter_part(hj->emitter, &pico8_hj_def, "hj");
    assert(hj->emitter->qy != NULL && hj->emitter->qy->count == 0);
    assert_lz_ran(&hj->qh.jo);
}

static void test_pm(void) {
    const pico8_pm_t *pm = pico8_pm_new(NULL);

    same_jo_part(&pm->hj.qh.jo, &pico8_pm_def, "pm");
    same_qh_part(&pm->hj.qh, &pico8_pm_def, "pm");
    same_in(same_text(pm->hj.rr, pico8_pm_def.rr), "pm", "rr");
    same_in(same_vec(pm->ng, &pico8_pm_def.ng), "pm", "ng");
    same_in(same_text(pm->fz, pico8_pm_def.fz), "pm", "fz");
    same_emitter_part(pm->hj.emitter, &pico8_pm_def, "pm");
    assert(pm->hj.emitter->qy != NULL && pm->hj.emitter->qy->count == 0);
    assert_lz_ran(&pm->hj.qh.jo);
}

static void test_mx(void) {
    const pico8_mx_t *mx = pico8_mx_new(NULL);

    same_jo_part(&mx->hj.qh.jo, &pico8_mx_def, "mx");
    same_qh_part(&mx->hj.qh, &pico8_mx_def, "mx");
    same_in(same_text(mx->hj.rr, pico8_mx_def.rr), "mx", "rr");
    same_in(mx->ht == pico8_mx_def.ht, "mx", "ht");
    same_emitter_part(mx->hj.emitter, &pico8_mx_def, "mx");
    assert(mx->hj.emitter->qy != NULL && mx->hj.emitter->qy->count == 0);
    assert_lz_ran(&mx->hj.qh.jo);
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
    pico8_jl();

    test_bg();
    test_ui();
    test_js();
    test_fq();
    test_qb();
    test_nf();
    test_mh();
    test_iv();
    test_hp();
    test_ie();
    test_pj();
    test_hy();
    test_mz();
    test_jc();
    test_g_();
    test_fv();
    test_jo();
    test_qh();
    test_nl();
    test_os();
    test_hj();
    test_pm();
    test_mx();
    assert(g_compared == 350 && "families a to e1 compare 350 fields: one missed or one too many");

    printf("test_pico8_archetype_chain: all tests passed\n");
    return 0;
}
