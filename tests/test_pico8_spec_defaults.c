/* The generated defaults each class outside the `jo` tree starts on, each
 * against its own data: a fresh struct of each module, then its ready cycle.
 * No shared reader, no table of modules. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bg.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_mouse.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_trade.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_wormhole.h"

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static pico8_nf_button_t *nf_l_of(const pico8_nf_t *nf) {
    assert(nf != NULL && pico8_nf_l() != NULL);
    return (pico8_nf_button_t *)pico8_nf_l();
}

/* The second `jl()` (the one `eu` runs) finds `qb`, `nf` and `ui` ready and
 * leaves their state alone: a mark written into `nf`'s `l` and into the
 * shared `le` survives it; `qb` stays ready throughout. */
static void test_second_jl_keeps_specs(void) {
    pico8_nf_button_t *nf_l = nf_l_of(pico8_je_nf());
    pico8_list_t *ui_le = pico8_ui_le_shared();
    pico8_value_t mark = pico8_value_fixed(0x7777);
    int hm = nf_l->hm;
    int ui_count;

    assert(pico8_qb_ready());
    nf_l->hm = 0x7777;
    pico8_list_add(ui_le, mark);
    ui_count = ui_le->count;

    pico8_jl();

    assert(pico8_qb_ready());
    assert(nf_l_of(pico8_je_nf()) == nf_l && nf_l_of(pico8_je_nf())->hm == 0x7777);
    assert(pico8_ui_le_shared() == ui_le && pico8_ui_le_shared()->count == ui_count);
    assert(pico8_list_del(ui_le, mark));
    nf_l->hm = hm;
}

/* A fresh bg starts on bg's generated defaults and ke's; ready until its
 * shutdown. */
static void test_bg_defaults_and_ready(void) {
    pico8_bg_t *bg = pico8_bg_new();

    assert(bg->fa == pico8_bg_start.fa && bg->hd == pico8_bg_start.hd);
    assert(bg->pl == pico8_bg_start.pl && bg->pl == &pico8_bg_pl_seq);
    assert(bg->ke.me == pico8_ke_start.me && bg->ke.t == pico8_ke_start.t);
    assert(pico8_bg_ready());
    pico8_bg_shutdown();
    assert(!pico8_bg_ready());
}

static void test_js_defaults_and_ready(void) {
    pico8_js_t *js = pico8_js_new(NULL);

    assert(js->fa == pico8_js_start.fa);
    assert(js->ke.me == pico8_ke_start.me && js->ke.t == pico8_ke_start.t);
    assert(pico8_js_ready());
    pico8_js_shutdown();
    assert(!pico8_js_ready());
}

/* `v`, `p` and `ra` are the generated vectors themselves. */
static void test_qb_defaults_and_ready(void) {
    pico8_qb_t *qb = pico8_qb_new();

    assert(qb->v == &pico8_qb_start.v && qb->p == &pico8_qb_start.p && qb->ra == &pico8_qb_start.ra);
    assert(qb->ma == pico8_qb_start.ma && qb->pc == pico8_qb_start.pc && qb->pc == &pico8_qb_pc_vecs);
    assert(qb->ke.me == pico8_ke_start.me && qb->ke.t == pico8_ke_start.t);
    assert(pico8_qb_ready());
    pico8_qb_shutdown();
    assert(!pico8_qb_ready());
}

/* mz's own `me` replaces ke's; `jj` holds the generated `qs`. */
static void test_mz_defaults_and_ready(void) {
    pico8_mz_t *mz = pico8_mz_new();

    assert(mz->ke.me == pico8_mz_start.me && strcmp(mz->ke.me, "kz") == 0 && mz->ke.t == pico8_ke_start.t);
    assert(mz->nu == pico8_mz_start.nu && mz->kk == pico8_mz_start.kk);
    assert(mz->probe->dw == pico8_mz_start.dw && mz->probe->dq == pico8_mz_start.dq);
    assert(mz->spawn_snd == pico8_mz_start.spawn_snd && mz->probe->ik == pico8_mz_start.ik);
    assert(mz->oi == pico8_mz_start.oi);
    assert(mz->probe->od == pico8_mz_start.od && mz->probe->od == pico8_fixed_from_int(3));
    assert(mz->probe->fp == pico8_mz_start.fp && mz->fa == pico8_mz_start.fa);
    assert(mz->jj == pico8_mz_jj_wrapper() && mz->jj->qs == pico8_mz_jj_qs);
    assert(pico8_mz_ready());
    pico8_wormhole_shutdown();
    assert(!pico8_mz_ready());
}

static void test_layout_ready(void) {
    assert(pico8_lq_ready() && pico8_jb_ready());
    pico8_layout_shutdown();
    assert(!pico8_lq_ready() && !pico8_jb_ready());
}

/* A fresh fv: fv's own generated defaults over g_'s, ke's under both. */
static void test_fv_defaults_and_ready(void) {
    pico8_fv_t *fv = pico8_fv_new();

    assert(fv->g_.fa == pico8_fv_start.fa);
    assert(fv->g_.ra->x == pico8_fv_start.ra.x && fv->g_.ra->y == pico8_fv_start.ra.y);
    assert(fv->g_.emitter->fc == pico8_fv_start.fc);
    assert(fv->g_.emitter->k_ == pico8_fv_start.k_);
    assert(fv->g_.emitter->qo == pico8_fv_start.qo);
    assert(fv->g_.emitter->ro == pico8_fv_start.ro);
    assert(fv->g_.emitter->qg == pico8_fv_start.qg);
    assert(fv->g_.emitter->q_ == pico8_fv_start.q_);
    assert(fv->g_.emitter->pn == pico8_fv_start.pn);
    assert(fv->g_.emitter->du == pico8_fv_start.du);
    assert(fv->g_.emitter->py == pico8_fv_start.py);
    assert(fv->g_.emitter->pk == pico8_fv_start.pk);
    assert(fv->g_.emitter->ki == pico8_fv_start.ki);
    assert(fv->g_.nu == pico8_g__start.nu);
    assert(fv->g_.fx == pico8_g__start.fx);
    assert(strcmp(fv->g_.probe->dq, pico8_g__start.dq) == 0);
    assert(fv->g_.probe->od == pico8_g__start.od);
    assert(fv->g_.probe->fp == pico8_g__start.fp);
    assert(fv->g_.probe->dw == pico8_g__start.dw);
    assert(fv->hi->x == pico8_fv_start.hi.x && fv->hi->y == pico8_fv_start.hi.y);
    assert(fv->ca == pico8_fv_start.ca);
    assert(fv->gq->x == pico8_fv_start.gq.x && fv->gq->y == pico8_fv_start.gq.y);
    assert(fv->eo == pico8_fv_start.eo);
    assert(fv->ff == pico8_fv_start.ff);
    assert(fv->kq->xl == pico8_fv_start.kq.xl && fv->kq->yt == pico8_fv_start.kq.yt);
    assert(fv->kq->xr == pico8_fv_start.kq.xr && fv->kq->yb == pico8_fv_start.kq.yb);
    assert(fv->ql == pico8_fv_start.ql);
    assert(fv->fg == pico8_fv_start.fg);
    assert(fv->rb == pico8_fv_start.rb);
    assert(fv->g_.ke.me == pico8_ke_start.me && fv->g_.ke.t == pico8_ke_start.t);
    assert(pico8_fv_ready());
    pico8_help_shutdown();
    assert(!pico8_fv_ready());
}

/* A fresh probe: g_'s generated defaults and ke's; no `ra` yet. */
static void test_g__defaults_and_ready(void) {
    pico8_g__t *g_ = pico8_g__new(NULL);

    assert(g_->nu == pico8_g__start.nu);
    assert(g_->fx == pico8_g__start.fx);
    assert(strcmp(g_->probe->dq, pico8_g__start.dq) == 0);
    assert(g_->probe->od == pico8_g__start.od);
    assert(g_->probe->fp == pico8_g__start.fp);
    assert(g_->fa == pico8_g__start.fa);
    assert(g_->probe->dw == pico8_g__start.dw);
    assert(g_->emitter->py == pico8_g__start.py);
    assert(g_->emitter->ki == pico8_g__start.ki);
    assert(g_->emitter->pn == pico8_g__start.pn);
    assert(g_->emitter->pk == pico8_g__start.pk);
    assert(g_->emitter->fc == pico8_g__start.fc);
    assert(g_->emitter->k_ == pico8_g__start.k_);
    assert(g_->emitter->qo == pico8_g__start.qo);
    assert(g_->emitter->ro == pico8_g__start.ro);
    assert(g_->emitter->qg == pico8_g__start.qg);
    assert(g_->emitter->q_ == pico8_g__start.q_);
    assert(g_->emitter->du == pico8_g__start.du);
    assert(g_->ra == NULL);
    assert(g_->ke.me == pico8_ke_start.me && g_->ke.t == pico8_ke_start.t);
    assert(pico8_g__ready());
    pico8_probe_shutdown();
    assert(!pico8_g__ready());
}

static void test_hp_defaults_and_ready(void) {
    pico8_hp_t *hp = pico8_hp_new(NULL);

    assert(hp->mr == pico8_hp_start.mr);
    assert(hp->gp == pico8_hp_start.gp);
    assert(hp->fk == pico8_hp_start.fk);
    assert(pico8_hp_ready());
    pico8_research_shutdown();
    assert(!pico8_hp_ready());
}

/* `kq`, `ra`, `eh` hold ie's generated box and vectors. */
static void test_ie_defaults_and_ready(void) {
    pico8_ie_t *ie = pico8_ie_new(NULL);

    assert(ie->kq->xl == pico8_ie_start.kq.xl && ie->kq->yt == pico8_ie_start.kq.yt);
    assert(ie->kq->xr == pico8_ie_start.kq.xr && ie->kq->yb == pico8_ie_start.kq.yb);
    assert(ie->fa == pico8_ie_start.fa);
    assert(ie->ra->x == pico8_ie_start.ra.x && ie->ra->y == pico8_ie_start.ra.y);
    assert(ie->eh->x == pico8_ie_start.eh.x && ie->eh->y == pico8_ie_start.eh.y);
    assert(ie->ke_base.me == pico8_ke_start.me && ie->ke_base.t == pico8_ke_start.t);
    assert(pico8_ie_ready());
    pico8_research_icon_shutdown();
    assert(!pico8_ie_ready());
}

/* `ed` absent reads as pj's generated `ed`; `cr` holds pj's generated
 * record. */
static void test_pj_defaults_and_ready(void) {
    pico8_pj_t *pj = pico8_pj_new();
    const pico8_pj_cr_t *cr = pico8_pj_cr();

    assert(pj->fa == pico8_pj_start.fa);
    assert(pj->gs == pico8_pj_start.gs);
    assert(pj->i_ == pico8_pj_start.i_);
    assert(pj->ed == NULL && pico8_pj_ed(pj).as.fixed == pico8_pj_start.ed);
    assert(cr->hi->x == pico8_pj_start.cr->hi.x && cr->hi->y == pico8_pj_start.cr->hi.y);
    assert(cr->gq->x == pico8_pj_start.cr->gq.x && cr->gq->y == pico8_pj_start.cr->gq.y);
    assert(cr->ca == pico8_pj_start.cr->ca);
    assert(pj->ke.me == pico8_ke_start.me && pj->ke.t == pico8_ke_start.t);
    assert(pico8_pj_ready());
    pico8_score_panel_shutdown();
    assert(!pico8_pj_ready());
}

/* hy and trade: an empty spec, a root struct. */
static void test_hy_ready(void) {
    pico8_hy_t *hy = pico8_hy_new();

    assert(hy->header.cls == pico8_hy_struct_class());
    assert(pico8_hy_ready());
    pico8_economy_shutdown();
    assert(!pico8_hy_ready());
}

static void test_trade_ready(void) {
    pico8_trade_t *trade = pico8_trade_new(NULL);

    assert(trade->header.cls == pico8_trade_struct_class());
    assert(pico8_trade_ready());
    pico8_trade_shutdown();
    assert(!pico8_trade_ready());
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

    assert(!pico8_trade_ready());
    assert(!pico8_hy_ready());
    assert(!pico8_hp_ready());
    assert(!pico8_pj_ready());
    assert(!pico8_ie_ready());
    assert(!pico8_g__ready());
    assert(!pico8_fv_ready());
    pico8_cart_load();
    pico8_jl();

    test_second_jl_keeps_specs();
    test_bg_defaults_and_ready();
    test_js_defaults_and_ready();
    test_qb_defaults_and_ready();
    test_mz_defaults_and_ready();
    test_layout_ready();
    test_fv_defaults_and_ready();
    test_g__defaults_and_ready();
    test_hp_defaults_and_ready();
    test_ie_defaults_and_ready();
    test_pj_defaults_and_ready();
    test_hy_ready();
    test_trade_ready();

    printf("test_pico8_spec_defaults: all tests passed\n");
    return 0;
}
