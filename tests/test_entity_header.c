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

#include "p8/assets.h"
#include "game/background.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "game/economy.h"
#include "p8/gfx.h"
#include "game/help.h"
#include "p8/input.h"
#include "game/layout.h"
#include "p8/mem.h"
#include "game/mouse.h"
#include "game/message_box.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/probe.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "game/trade.h"
#include "engine/game_zone.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "game/wormhole.h"
#include "heap_allocator.h"

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

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
static void assert_links(const entity_t *h, entity_t want) {
    assert(h->layer == want.layer);
    assert(h->pos == want.pos);
    assert(h->map_pos == want.map_pos);
    assert(h->hit_box == want.hit_box);
    assert(h->cursor_sprite == want.cursor_sprite);
    assert(h->gpio_signal == want.gpio_signal);
    assert(h->infraspace == want.infraspace);
    assert(h->shake == want.shake);
    assert(h->fold_anchor == want.fold_anchor);
    assert(h->is_dead == want.is_dead);
    assert(h->is_hovered == want.is_hovered);
    assert(h->emitter == want.emitter);
    assert(h->targeting == want.targeting);
}

/* Non-carriers, one per constructor that does not go through ke; the mouse
 * point, which `jc:hq` builds, has its own test below. */
static void test_non_carriers_have_null_links(void) {
    pico8_ie_t *ie = pico8_ie_new(NULL);
    pico8_ie_ke_t view = pico8_ie_ke_view(ie);
    pico8_nf_t *nf = pico8_nf_new();
    pico8_lq_t *lq = pico8_lq_new(&heap_allocator, NULL);

    assert_links(&nf->header, (entity_t){.pos = (const vec_t *const *)&nf->ra});
    assert_links(&lq->header, (entity_t){.pos = &lq->ra});
    assert_links(&pico8_jb_new(&heap_allocator, NULL)->header, (entity_t){0});
    assert_links(&pico8_hp_new(NULL)->header, (entity_t){0});
    assert_links(&pico8_hy_new()->header, (entity_t){0});
    assert_links(&pico8_trade_new(game_zone(), NULL)->header, (entity_t){0});
    assert_links(&view.header, (entity_t){0});
}

/* Carriers: the archetype's field for a class that never writes it, the
 * instance's otherwise. */
static void test_carriers_link_their_storage(void) {
    pico8_bg_t *background = pico8_bg_new();
    pico8_js_t *button = pico8_js_new(NULL);
    pico8_ie_t *structure_icon = pico8_ie_new(NULL);
    pico8_js_keys_t with_gpio_signal = {.has_fg = 1};
    pico8_js_t *button_gpio_signal = pico8_js_new(&with_gpio_signal);
    pico8_fq_t *modal_backdrop = pico8_fq_new(NULL);
    pico8_ui_t *built_ui = pico8_ui_new();
    pico8_qb_t *camera = pico8_qb_new();
    pico8_mh_t *message_box = pico8_mh_new(NULL);
    pico8_iv_t *toast = pico8_iv_new(NULL);
    pico8_g__t *help_screen = pico8_fv_new();
    pico8_jo_t *planet = pico8_jo_new(NULL);
    pico8_jo_t *infraspace_gate = pico8_fr_new(NULL);
    pico8_jo_t *infraspace_gate_planet = infraspace_gate;

    assert_links(&background->header, (entity_t){.layer = &background->def->fa, .is_hovered = &background->m_});
    assert_links(&button->header,
                 (entity_t){.layer = &button->fa, .pos = &button->ra, .hit_box = button->header.hit_box, .is_hovered = &button->m_});
    assert(button->header.hit_box != NULL);
    assert_links(&button_gpio_signal->header,
                 (entity_t){
                     .layer = &button_gpio_signal->fa, .pos = &button_gpio_signal->ra, .hit_box = button_gpio_signal->header.hit_box, .gpio_signal = &button_gpio_signal->fg, .is_hovered = &button_gpio_signal->m_});
    assert_links(&structure_icon->header,
                 (entity_t){.layer = &structure_icon->def->fa, .pos = &structure_icon->ra, .hit_box = &structure_icon->def->kq, .is_hovered = &structure_icon->m_});
    assert_links(&modal_backdrop->header, (entity_t){.layer = &modal_backdrop->def->fa, .is_dead = &modal_backdrop->p_, .is_hovered = &modal_backdrop->m_});
    assert_links(&built_ui->header, (entity_t){.layer = &built_ui->def->fa, .cursor_sprite = &built_ui->def->ql});
    assert_links(&camera->header, (entity_t){.pos = &camera->ra, .map_pos = &camera->p, .shake = &camera->ma});
    assert_links(&message_box->header, (entity_t){.layer = &message_box->def->fa, .is_dead = &message_box->p_});
    assert_links(&toast->header, (entity_t){.layer = &toast->def->fa, .pos = &toast->ra, .is_dead = &toast->p_});
    assert_links(&help_screen->header, (entity_t){.layer = &help_screen->def->fa,
                                                         .pos = &help_screen->ra,
                                                         .map_pos = &help_screen->p,
                                                         .hit_box = &help_screen->def->kq,
                                                         .cursor_sprite = &help_screen->def->ql,
                                                         .gpio_signal = &help_screen->def->fg,
                                                         .fold_anchor = &help_screen->def->fx,
                                                         .is_dead = &help_screen->p_,
                                                         .is_hovered = &help_screen->m_,
                                                         .emitter = &help_screen->emitter_own,
                                                         .targeting = &help_screen->probe_own});
    assert(*help_screen->header.fold_anchor == p8_fixed_from_int(1) &&
           "a probe's fx is its archetype's 1.0, Lua's true: 0 would read as nil");
    assert_links(&planet->header, (entity_t){.layer = &planet->def->fa,
                                                      .pos = (const vec_t *const *)&planet->ra,
                                                      .map_pos = (const vec_t *const *)&planet->p,
                                                      .hit_box = &planet->def->kq,
                                                      .shake = &planet->ma,
                                                      .fold_anchor = &planet->fx,
                                                      .is_dead = &planet->p_,
                                                      .is_hovered = &planet->m_});
    assert(planet->probe == NULL && planet->emitter == NULL);
    assert_links(&infraspace_gate_planet->header, (entity_t){.layer = &infraspace_gate_planet->def->fa,
                                                         .pos = (const vec_t *const *)&infraspace_gate_planet->ra,
                                                         .map_pos = (const vec_t *const *)&infraspace_gate_planet->p,
                                                         .hit_box = &infraspace_gate_planet->def->kq,
                                                         .infraspace = &infraspace_gate_planet->def->mt,
                                                         .shake = &infraspace_gate_planet->ma,
                                                         .fold_anchor = &infraspace_gate_planet->fx,
                                                         .is_dead = &infraspace_gate_planet->p_,
                                                         .is_hovered = &infraspace_gate_planet->m_,
                                                         .targeting = &infraspace_gate->probe_own});
}

/* A planet's links, but for its emitter and probe. */
static entity_t planet_links(pico8_jo_t *jo) {
    return (entity_t){.layer = &jo->def->fa,
                                   .pos = (const vec_t *const *)&jo->ra,
                                   .map_pos = (const vec_t *const *)&jo->p,
                                   .hit_box = &jo->def->kq,
                                   .shake = &jo->ma,
                                   .fold_anchor = &jo->fx,
                                   .is_dead = &jo->p_,
                                   .is_hovered = &jo->m_};
}

static void assert_structure_links(pico8_jo_t *qh, pico8_emitter_t *emitter) {
    entity_t want = planet_links(qh);

    want.emitter = emitter;
    want.targeting = &qh->probe_own;
    assert_links(&qh->header, want);
    assert(qh->probe == qh->header.targeting && qh->emitter == qh->header.emitter);
}

/* Emitter and probe carriers: each link is the component the instance
 * stores in itself, the one its own member points at. */
static void test_component_carriers_link_their_own(void) {
    pico8_g__t *g_ = pico8_g__new(NULL);
    pico8_jo_t *nl = pico8_nl_new(NULL);
    pico8_jo_t *os = pico8_os_new(NULL);
    pico8_jo_t *hj = pico8_hj_new(NULL);
    pico8_jo_t *pm = pico8_pm_new(NULL);
    pico8_jo_t *mx = pico8_mx_new(NULL);
    pico8_jo_t *nr = pico8_nr_new(NULL);
    pico8_jo_t *ga = pico8_ga_new(NULL);
    pico8_mz_t *mz = pico8_mz_new();
    entity_t want = planet_links(ga);

    assert_links(&g_->header, (entity_t){.layer = &g_->def->fa,
                                                      .pos = &g_->ra,
                                                      .map_pos = &g_->p,
                                                      .fold_anchor = &g_->def->fx,
                                                      .is_dead = &g_->p_,
                                                      .emitter = &g_->emitter_own,
                                                      .targeting = &g_->probe_own});
    assert(g_->emitter == g_->header.emitter && g_->probe == g_->header.targeting);
    assert(*g_->header.fold_anchor == p8_fixed_from_int(1) &&
           "a probe's fx is its archetype's 1.0, Lua's true: 0 would read as nil");
    assert_structure_links(pico8_qh_new(NULL), NULL);
    assert_structure_links(nl, NULL);
    assert_structure_links(os, NULL);
    assert_structure_links(hj, &hj->emitter_own);
    assert_structure_links(pm, &pm->emitter_own);
    assert_structure_links(mx, &mx->emitter_own);
    assert_structure_links(nr, NULL);
    want.emitter = &ga->emitter_own;
    assert_links(&ga->header, want);
    assert(ga->emitter == ga->header.emitter && ga->probe == NULL);
    assert_links(&mz->header, (entity_t){.layer = &mz->def->fa,
                                                      .pos = &mz->ra,
                                                      .map_pos = &mz->p,
                                                      .is_dead = &mz->p_,
                                                      .targeting = &mz->probe_own});
    assert(mz->probe == mz->header.targeting);
}

static pico8_jc_t *slipway_dragged_over_nothing(entity_t *qu) {
    pico8_jc_keys_t keys = {0};
    vec_t *mp = malloc(sizeof *mp);
    pico8_jc_t *jc;

    keys.qu = qu;
    jc = pico8_jc_new(&keys);
    *mp = vec_make(p8_fixed_from_int(-640), p8_fixed_from_int(-640));
    pico8_ok()->o_ = NULL;
    pico8_jc_struct_class()->drag(&jc->header, mp, NULL);
    return jc;
}

/* The point `jc:hq` builds over nothing carries `ra` and `p` alone; the
 * slipway's `mt` is its start's, else nil. */
static void test_mouse_point_has_null_links(void) {
    pico8_jc_t *jc = slipway_dragged_over_nothing(&pico8_jo_new(NULL)->header);
    pico8_jc_mouse_point_t *point = (pico8_jc_mouse_point_t *)jc->re;
    pico8_jc_t *from_infraspace_gate = slipway_dragged_over_nothing(&pico8_fr_new(NULL)->header);

    assert(point != NULL && point->header.class == pico8_jc_mouse_point_struct_class());
    assert_links(&point->header, (entity_t){.pos = &point->ra, .map_pos = &point->p});
    assert(!jc->has_mt && jc->fa == p8_fixed_from_int(4));
    assert_links(&jc->header, (entity_t){.layer = &jc->fa, .is_dead = &jc->p_, .targeting = &jc->probe_own});
    assert(jc->probe == jc->header.targeting);
    assert(from_infraspace_gate->has_mt && from_infraspace_gate->mt == pico8_fr_def.mt && from_infraspace_gate->fa == p8_fixed_from_int(2));
    assert_links(&from_infraspace_gate->header,
                 (entity_t){
                     .layer = &from_infraspace_gate->fa, .infraspace = &from_infraspace_gate->mt, .is_dead = &from_infraspace_gate->p_, .targeting = &from_infraspace_gate->probe_own});
}

int main(int argc, char **argv) {
    (void)argc;
    rerun_with_malloc_filling(argv);
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);
    pico8_cart_load();
    reset_world();

    assert_malloc_fills();
    test_non_carriers_have_null_links();
    test_carriers_link_their_storage();
    test_component_carriers_link_their_own();
    test_mouse_point_has_null_links();

    printf("test_entity_header: all tests passed\n");
    return 0;
}
