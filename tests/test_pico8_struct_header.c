/* The kernel's component links on entities whose constructor `malloc`s
 * without zeroing. The binary runs itself again with `malloc` filling every
 * block it returns (MallocScribble on macOS; MALLOC_PERTURB_ on glibc, whose
 * tcache hands a freed block back unfilled, hence the tcache turned off), so
 * a link no init set is not NULL. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bg.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_economy.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_msgbox.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_planet_family_trade.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_research.h"
#include "../src/pico8_research_icon.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_trade.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_wormhole.h"

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static void rerun_with_malloc_filling(char **argv) {
    if (getenv("PICO8_MALLOC_FILLS") != NULL) {
        return;
    }
    setenv("MALLOC_PERTURB_", "90", 1);
    setenv("GLIBC_TUNABLES", "glibc.malloc.tcache_count=0", 1);
    setenv("MallocScribble", "1", 1);
    setenv("PICO8_MALLOC_FILLS", "1", 1);
    execv(argv[0], argv);
    perror("execv");
    exit(1);
}

/* Reads a recycled block on purpose: through `volatile`, which the compiler
 * does not flag as uninitialized. */
static void assert_malloc_fills(void) {
    void *block = calloc(1, 64);
    const volatile unsigned char *bytes;
    int i;

    assert(block != NULL);
    free(block);
    block = malloc(64);
    bytes = block;
    assert(block != NULL);
    for (i = 0; i < 64; i++) {
        assert(bytes[i] != 0 && bytes[i] == bytes[0] && "malloc must fill its block, or this test proves nothing");
    }
    free(block);
}

/* Each link of `want`: the one a class carries, or NULL; its `cls`, `me`
 * and `t` are not compared. */
static void assert_links(const pico8_struct_header_t *h, pico8_struct_header_t want) {
    assert(h->fa == want.fa);
    assert(h->ra == want.ra);
    assert(h->p == want.p);
    assert(h->kq == want.kq);
    assert(h->ql == want.ql);
    assert(h->fg == want.fg);
    assert(h->mt == want.mt);
    assert(h->ma == want.ma);
    assert(h->fx == want.fx);
    assert(h->p_ == want.p_);
    assert(h->m_ == want.m_);
    assert(h->emitter == want.emitter);
    assert(h->probe == want.probe);
}

/* Non-carriers, one per constructor that does not go through ke; the mouse
 * point, which `jc:hq` builds, has its own test below. */
static void test_non_carriers_have_null_links(void) {
    pico8_ie_t *ie = pico8_ie_new(NULL);
    pico8_ie_ke_t view = pico8_ie_ke_view(ie);
    pico8_nf_t *nf = pico8_nf_new();
    pico8_lq_t *lq = pico8_lq_new(NULL);

    assert_links(&nf->header, (pico8_struct_header_t){.ra = (const pico8_vec_t *const *)&nf->ra});
    assert_links(&lq->header, (pico8_struct_header_t){.ra = &lq->ra});
    assert_links(&pico8_jb_new(NULL)->header, (pico8_struct_header_t){0});
    assert_links(&pico8_hp_new(NULL)->header, (pico8_struct_header_t){0});
    assert_links(&pico8_hy_new()->header, (pico8_struct_header_t){0});
    assert_links(&pico8_trade_new(NULL)->header, (pico8_struct_header_t){0});
    assert_links(&view.header, (pico8_struct_header_t){0});
}

/* Carriers: the archetype's field for a class that never writes it, the
 * instance's otherwise. */
static void test_carriers_link_their_storage(void) {
    pico8_bg_t *bg = pico8_bg_new();
    pico8_js_t *js = pico8_js_new(NULL);
    pico8_ie_t *ie = pico8_ie_new(NULL);
    pico8_js_keys_t with_fg = {.has_fg = 1};
    pico8_js_t *js_fg = pico8_js_new(&with_fg);
    pico8_fq_t *fq = pico8_fq_new(NULL);
    pico8_ui_t *ui = pico8_ui_new();
    pico8_qb_t *qb = pico8_qb_new();
    pico8_mh_t *mh = pico8_mh_new(NULL);
    pico8_iv_t *iv = pico8_iv_new(NULL);
    pico8_g__t *fv = pico8_fv_new();
    pico8_jo_t *jo = pico8_jo_new(NULL);
    pico8_fr_t *fr = pico8_fr_new(NULL);
    pico8_jo_t *fr_jo = &fr->nr.qh.jo;

    assert_links(&bg->header, (pico8_struct_header_t){.fa = &bg->def->fa, .m_ = &bg->m_});
    assert_links(&js->header,
                 (pico8_struct_header_t){.fa = &js->fa, .ra = &js->ra, .kq = js->header.kq, .m_ = &js->m_});
    assert(js->header.kq != NULL);
    assert_links(&js_fg->header,
                 (pico8_struct_header_t){
                     .fa = &js_fg->fa, .ra = &js_fg->ra, .kq = js_fg->header.kq, .fg = &js_fg->fg, .m_ = &js_fg->m_});
    assert_links(&ie->header,
                 (pico8_struct_header_t){.fa = &ie->def->fa, .ra = &ie->ra, .kq = &ie->def->kq, .m_ = &ie->m_});
    assert_links(&fq->header, (pico8_struct_header_t){.fa = &fq->def->fa, .p_ = &fq->p_, .m_ = &fq->m_});
    assert_links(&ui->header, (pico8_struct_header_t){.fa = &ui->def->fa, .ql = &ui->def->ql});
    assert_links(&qb->header, (pico8_struct_header_t){.ra = &qb->ra, .p = &qb->p, .ma = &qb->ma});
    assert_links(&mh->header, (pico8_struct_header_t){.fa = &mh->def->fa, .p_ = &mh->p_});
    assert_links(&iv->header, (pico8_struct_header_t){.fa = &iv->def->fa, .ra = &iv->ra, .p_ = &iv->p_});
    assert_links(&fv->header, (pico8_struct_header_t){.fa = &fv->def->fa,
                                                         .ra = &fv->ra,
                                                         .p = &fv->p,
                                                         .kq = &fv->def->kq,
                                                         .ql = &fv->def->ql,
                                                         .fg = &fv->def->fg,
                                                         .fx = &fv->def->fx,
                                                         .p_ = &fv->p_,
                                                         .m_ = &fv->m_,
                                                         .emitter = &fv->emitter_own,
                                                         .probe = &fv->probe_own});
    assert(*fv->header.fx == pico8_fixed_from_int(1) &&
           "a probe's fx is its archetype's 1.0, Lua's true: 0 would read as nil");
    assert_links(&jo->header, (pico8_struct_header_t){.fa = &jo->def->fa,
                                                      .ra = (const pico8_vec_t *const *)&jo->ra,
                                                      .p = (const pico8_vec_t *const *)&jo->p,
                                                      .kq = &jo->def->kq,
                                                      .ma = &jo->ma,
                                                      .fx = &jo->fx,
                                                      .p_ = &jo->p_,
                                                      .m_ = &jo->m_});
    assert_links(&fr_jo->header, (pico8_struct_header_t){.fa = &fr_jo->def->fa,
                                                         .ra = (const pico8_vec_t *const *)&fr_jo->ra,
                                                         .p = (const pico8_vec_t *const *)&fr_jo->p,
                                                         .kq = &fr_jo->def->kq,
                                                         .mt = &fr_jo->def->mt,
                                                         .ma = &fr_jo->ma,
                                                         .fx = &fr_jo->fx,
                                                         .p_ = &fr_jo->p_,
                                                         .m_ = &fr_jo->m_,
                                                         .probe = &fr->nr.qh.probe_own});
}

/* A planet's links, but for its emitter and probe. */
static pico8_struct_header_t planet_links(pico8_jo_t *jo) {
    return (pico8_struct_header_t){.fa = &jo->def->fa,
                                   .ra = (const pico8_vec_t *const *)&jo->ra,
                                   .p = (const pico8_vec_t *const *)&jo->p,
                                   .kq = &jo->def->kq,
                                   .ma = &jo->ma,
                                   .fx = &jo->fx,
                                   .p_ = &jo->p_,
                                   .m_ = &jo->m_};
}

static void assert_qh_links(pico8_qh_t *qh, pico8_emitter_t *emitter) {
    pico8_struct_header_t want = planet_links(&qh->jo);

    want.emitter = emitter;
    want.probe = &qh->probe_own;
    assert_links(&qh->jo.header, want);
    assert(qh->probe == qh->jo.header.probe);
}

/* Emitter and probe carriers: each link is the component the instance
 * stores in itself, the one its own member points at. */
static void test_component_carriers_link_their_own(void) {
    pico8_g__t *g_ = pico8_g__new(NULL);
    pico8_nl_t *nl = pico8_nl_new(NULL);
    pico8_os_t *os = pico8_os_new(NULL);
    pico8_hj_t *hj = pico8_hj_new(NULL);
    pico8_pm_t *pm = pico8_pm_new(NULL);
    pico8_mx_t *mx = pico8_mx_new(NULL);
    pico8_nr_t *nr = pico8_nr_new(NULL);
    pico8_ga_t *ga = pico8_ga_new(NULL);
    pico8_mz_t *mz = pico8_mz_new();
    pico8_struct_header_t want = planet_links(&ga->jo);

    assert_links(&g_->header, (pico8_struct_header_t){.fa = &g_->def->fa,
                                                      .ra = &g_->ra,
                                                      .p = &g_->p,
                                                      .fx = &g_->def->fx,
                                                      .p_ = &g_->p_,
                                                      .emitter = &g_->emitter_own,
                                                      .probe = &g_->probe_own});
    assert(g_->emitter == g_->header.emitter && g_->probe == g_->header.probe);
    assert(*g_->header.fx == pico8_fixed_from_int(1) &&
           "a probe's fx is its archetype's 1.0, Lua's true: 0 would read as nil");
    assert_qh_links(pico8_qh_new(NULL), NULL);
    assert_qh_links(&nl->qh, NULL);
    assert_qh_links(&os->qh, NULL);
    assert_qh_links(&hj->qh, &hj->emitter_own);
    assert(hj->emitter == hj->qh.jo.header.emitter);
    assert_qh_links(&pm->hj.qh, &pm->hj.emitter_own);
    assert_qh_links(&mx->hj.qh, &mx->hj.emitter_own);
    assert_qh_links(&nr->qh, NULL);
    want.emitter = &ga->emitter_own;
    assert_links(&ga->jo.header, want);
    assert(ga->emitter == ga->jo.header.emitter);
    assert_links(&mz->header, (pico8_struct_header_t){.fa = &mz->def->fa,
                                                      .ra = &mz->ra,
                                                      .p = &mz->p,
                                                      .p_ = &mz->p_,
                                                      .probe = &mz->probe_own});
    assert(mz->probe == mz->header.probe);
}

static pico8_jc_t *jc_dragged_over_nothing(pico8_struct_header_t *qu) {
    pico8_jc_keys_t keys = {0};
    pico8_vec_t *mp = malloc(sizeof *mp);
    pico8_jc_t *jc;

    keys.qu = qu;
    jc = pico8_jc_new(&keys);
    *mp = pico8_vec_v(pico8_fixed_from_int(-640), pico8_fixed_from_int(-640));
    pico8_ok()->o_ = NULL;
    pico8_jc_struct_class()->hq(jc, mp, NULL);
    return jc;
}

/* The point `jc:hq` builds over nothing carries `ra` and `p` alone; the
 * slipway's `mt` is its start's, else nil. */
static void test_mouse_point_has_null_links(void) {
    pico8_jc_t *jc = jc_dragged_over_nothing(&pico8_jo_new(NULL)->header);
    pico8_jc_mouse_point_t *point = (pico8_jc_mouse_point_t *)jc->re;
    pico8_jc_t *from_fr = jc_dragged_over_nothing(&pico8_fr_new(NULL)->nr.qh.jo.header);

    assert(point != NULL && point->header.cls == pico8_jc_mouse_point_struct_class());
    assert_links(&point->header, (pico8_struct_header_t){.ra = &point->ra, .p = &point->p});
    assert(!jc->has_mt && jc->fa == pico8_fixed_from_int(4));
    assert_links(&jc->header, (pico8_struct_header_t){.fa = &jc->fa, .p_ = &jc->p_, .probe = &jc->probe_own});
    assert(jc->probe == jc->header.probe);
    assert(from_fr->has_mt && from_fr->mt == pico8_fr_def.mt && from_fr->fa == pico8_fixed_from_int(2));
    assert_links(&from_fr->header,
                 (pico8_struct_header_t){
                     .fa = &from_fr->fa, .mt = &from_fr->mt, .p_ = &from_fr->p_, .probe = &from_fr->probe_own});
}

int main(int argc, char **argv) {
    (void)argc;
    rerun_with_malloc_filling(argv);
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

    assert_malloc_fills();
    test_non_carriers_have_null_links();
    test_carriers_link_their_storage();
    test_component_carriers_link_their_own();
    test_mouse_point_has_null_links();

    printf("test_pico8_struct_header: all tests passed\n");
    return 0;
}
