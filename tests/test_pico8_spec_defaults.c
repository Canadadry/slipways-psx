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
#include "../src/pico8_mem.h"
#include "../src/pico8_mouse.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_ptrs.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_score_panel.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_trade.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
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
    static pico8_struct_header_t mark_cell;
    pico8_ents_t *ui_le = pico8_ui_le_shared();
    pico8_struct_header_t *mark = &mark_cell;
    int hm = nf_l->hm;
    int ui_count;

    assert(pico8_qb_ready());
    nf_l->hm = 0x7777;
    pico8_ents_add(ui_le, mark);
    ui_count = ui_le->count;

    pico8_jl();

    assert(pico8_qb_ready());
    assert(nf_l_of(pico8_je_nf()) == nf_l && nf_l_of(pico8_je_nf())->hm == 0x7777);
    assert(pico8_ui_le_shared() == ui_le && pico8_ui_le_shared()->count == ui_count);
    assert(pico8_ents_del(ui_le, mark));
    nf_l->hm = hm;
}

/* A fresh bg reads its archetype and starts on ke's defaults; ready until
 * its shutdown. */
static void test_bg_defaults_and_ready(void) {
    pico8_bg_t *bg = pico8_bg_new();

    assert(bg->def == &pico8_bg_def && bg->def->pl == &pico8_bg_pl_seq);
    assert(bg->header.me == pico8_ke_start.me && bg->header.t == pico8_ke_start.t);
    assert(pico8_bg_ready());
    pico8_bg_shutdown();
    assert(!pico8_bg_ready());
}

static void test_js_defaults_and_ready(void) {
    pico8_js_t *js = pico8_js_new(NULL);

    assert(js->def == &pico8_js_def && js->fa == js->def->fa);
    assert(js->header.me == pico8_ke_start.me && js->header.t == pico8_ke_start.t);
    assert(pico8_js_ready());
    pico8_js_shutdown();
    assert(!pico8_js_ready());
}

/* `v`, `p` and `ra` start on the archetype's vectors. */
static void test_qb_defaults_and_ready(void) {
    pico8_qb_t *qb = pico8_qb_new();

    assert(qb->def == &pico8_qb_def && qb->def->pc == &pico8_qb_pc_vecs);
    assert(qb->v == &qb->def->v && qb->p == &qb->def->p && qb->ra == &qb->def->ra && qb->ma == qb->def->ma);
    assert(qb->header.me == pico8_ke_start.me && qb->header.t == pico8_ke_start.t);
    assert(pico8_qb_ready());
    pico8_qb_shutdown();
    assert(!pico8_qb_ready());
}

/* mz's own `me` replaces ke's; `jj` holds the generated `qs`. */
static void test_mz_defaults_and_ready(void) {
    pico8_mz_t *mz = pico8_mz_new();

    assert(mz->def == &pico8_mz_def && mz->probe->def == &pico8_mz_def.probe);
    assert(mz->header.me == mz->def->me && strcmp(mz->header.me, "kz") == 0 && mz->header.t == pico8_ke_start.t);
    assert(mz->probe->dw == mz->def->probe.dw && mz->probe->dq == mz->def->probe.dq);
    assert(mz->probe->ik == mz->def->probe.ik && mz->probe->has_ik == mz->def->probe.has_ik);
    assert(mz->oi == mz->def->oi);
    assert(mz->probe->od == mz->def->probe.od && mz->probe->od == pico8_fixed_from_int(3));
    assert(mz->probe->has_od == mz->def->probe.has_od && mz->probe->fp == mz->def->probe.fp);
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

/* A fresh fv: its archetype's defaults copied, ke's under them. */
static void test_fv_defaults_and_ready(void) {
    pico8_fv_t *fv = pico8_fv_new();
    const pico8_g__t *g_ = &fv->g_;

    assert(g_->def == &pico8_fv_def && g_->emitter->def == &pico8_fv_def.emitter &&
           g_->probe->def == &pico8_fv_def.probe);
    assert(g_->ra == &g_->def->ra);
    assert(g_->emitter->fc == g_->def->emitter.fc && g_->emitter->k_ == g_->def->emitter.k_);
    assert(g_->emitter->qo == g_->def->emitter.qo && g_->emitter->ro == g_->def->emitter.ro);
    assert(g_->emitter->qg == g_->def->emitter.qg && g_->emitter->q_ == g_->def->emitter.q_);
    assert(g_->emitter->pn == g_->def->emitter.pn && g_->emitter->du == g_->def->emitter.du);
    assert(g_->emitter->py == g_->def->emitter.py && g_->emitter->pk == g_->def->emitter.pk);
    assert(g_->emitter->ki == g_->def->emitter.ki);
    assert(g_->probe->dq == g_->def->probe.dq && g_->probe->od == g_->def->probe.od);
    assert(g_->probe->has_od == g_->def->probe.has_od && g_->probe->fp == g_->def->probe.fp);
    assert(g_->probe->ik == g_->def->probe.ik && g_->probe->has_ik == g_->def->probe.has_ik);
    assert(g_->probe->dw == g_->def->probe.dw);
    assert(g_->probe->has_od == 1 && g_->probe->ik == 0 && g_->probe->has_ik == 0);
    assert(g_->header.me == pico8_ke_start.me && g_->header.t == pico8_ke_start.t);
    assert(pico8_fv_ready());
    pico8_help_shutdown();
    assert(!pico8_fv_ready());
}

/* A fresh probe: its archetype's defaults copied, ke's under them; no `ra`
 * yet. */
static void test_g__defaults_and_ready(void) {
    pico8_g__t *g_ = pico8_g__new(NULL);

    assert(g_->def == &pico8_g__def && g_->emitter->def == &pico8_g__def.emitter &&
           g_->probe->def == &pico8_g__def.probe);
    assert(g_->probe->dq == g_->def->probe.dq && g_->probe->od == g_->def->probe.od);
    assert(g_->probe->has_od == g_->def->probe.has_od && g_->probe->has_od == 1);
    assert(g_->probe->fp == g_->def->probe.fp && g_->probe->dw == g_->def->probe.dw);
    assert(g_->probe->ik == g_->def->probe.ik && g_->probe->has_ik == g_->def->probe.has_ik);
    assert(g_->probe->ik == 0 && g_->probe->has_ik == 0);
    assert(g_->emitter->py == g_->def->emitter.py && g_->emitter->ki == g_->def->emitter.ki);
    assert(g_->emitter->pn == g_->def->emitter.pn && g_->emitter->pk == g_->def->emitter.pk);
    assert(g_->emitter->fc == g_->def->emitter.fc && g_->emitter->k_ == g_->def->emitter.k_);
    assert(g_->emitter->qo == g_->def->emitter.qo && g_->emitter->ro == g_->def->emitter.ro);
    assert(g_->emitter->qg == g_->def->emitter.qg && g_->emitter->q_ == g_->def->emitter.q_);
    assert(g_->emitter->du == g_->def->emitter.du);
    assert(g_->ra == NULL);
    assert(g_->header.me == pico8_ke_start.me && g_->header.t == pico8_ke_start.t);
    assert(pico8_g__ready());
    pico8_probe_shutdown();
    assert(!pico8_g__ready());
}

static void test_hp_defaults_and_ready(void) {
    pico8_hp_t *hp = pico8_hp_new(NULL);

    assert(hp->def == &pico8_hp_def && hp->def->fk == &pico8_hp_fk_row);
    assert(pico8_hp_ready());
    pico8_research_shutdown();
    assert(!pico8_hp_ready());
}

/* `ra` starts on the archetype's vector. */
static void test_ie_defaults_and_ready(void) {
    pico8_ie_t *ie = pico8_ie_new(NULL);

    assert(ie->def == &pico8_ie_def && ie->ra == &ie->def->ra);
    assert(ie->header.me == pico8_ke_start.me && ie->header.t == pico8_ke_start.t);
    assert(pico8_ie_ready());
    pico8_research_icon_shutdown();
    assert(!pico8_ie_ready());
}

/* `cr` holds the spec's record. */
static void test_pj_defaults_and_ready(void) {
    pico8_pj_t *pj = pico8_pj_new();
    const pico8_pj_cr_t *cr = pico8_pj_cr();

    assert(pj->def == &pico8_pj_def && pj->fa == pj->def->fa);
    assert(pj->ed == NULL);
    assert(cr->hi->x == 0 && cr->hi->y == pico8_fixed_from_int(11));
    assert(cr->gq->x == 0 && cr->gq->y == pico8_fixed_from_int(11));
    assert(cr->ca == pico8_fixed_from_int(10));
    assert(pj->header.me == pico8_ke_start.me && pj->header.t == pico8_ke_start.t);
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
