/* Each archetype against the value its class's init chain still leaves in a
 * fresh instance, field by field. Removed with the chains it compares. */

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
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_msgbox.h"
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

static int same_text(const char *got, const char *want) {
    return got != NULL && want != NULL && strcmp(got, want) == 0;
}

static int same_text_or_nil(const char *got, const char *want) {
    return (got == NULL && want == NULL) || same_text(got, want);
}

static int same_vec(const pico8_vec_t *got, const pico8_vec_t *want) {
    return got != NULL && got->x == want->x && got->y == want->y;
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
    assert(g_compared == 76 && "families a and b compare 76 fields: one missed or one too many");

    printf("test_pico8_archetype_chain: all tests passed\n");
    return 0;
}
