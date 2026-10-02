/* The generated defaults each class outside the `jo` tree starts on, each
 * against its own data: a fresh struct of each module, then its ready cycle.
 * No shared reader, no table of modules. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p8/assets.h"
#include "game/background.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "game/economy.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "p8/gfx.h"
#include "game/help.h"
#include "p8/input.h"
#include "game/layout.h"
#include "p8/mem.h"
#include "game/mouse_button.h"
#include "game/mouse.h"
#include "game/probe.h"
#include "game/research.h"
#include "game/structure_icon.h"
#include "game/score_panel.h"
#include "game/scroll.h"
#include "game/trade.h"
#include "engine/game_zone.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "game/wormhole.h"

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static mouse_button_t *mouse_left_of(const mouse_t *nf) {
    assert(nf != NULL && mouse_left_button() != NULL);
    return (mouse_button_t *)mouse_left_button();
}

/* The second `jl()` (the one `eu` runs) finds `qb`, `nf` and `ui` ready and
 * leaves their state alone: a mark written into `nf`'s `l` and into the
 * shared `le` survives it; `qb` stays ready throughout. */
static void test_second_reset_world_keeps_specs(void) {
    mouse_button_t *mouse_left = mouse_left_of(main_mouse_typed());
    static entity_t mark_cell;
    entity_list_t *ui_ephemerals = ui_ephemerals_shared();
    entity_t *mark = &mark_cell;
    int hm = mouse_left->released;
    int ui_count;

    assert(camera_ready());
    mouse_left->released = 0x7777;
    entity_list_add(ui_ephemerals, mark);
    ui_count = ui_ephemerals->count;

    reset_world();

    assert(camera_ready());
    assert(mouse_left_of(main_mouse_typed()) == mouse_left && mouse_left_of(main_mouse_typed())->released == 0x7777);
    assert(ui_ephemerals_shared() == ui_ephemerals && ui_ephemerals_shared()->count == ui_count);
    assert(entity_list_del(ui_ephemerals, mark));
    mouse_left->released = hm;
}

/* A fresh bg reads its archetype and starts on ke's defaults; ready until
 * its shutdown. */
static void test_background_defaults_and_ready(void) {
    background_t *bg = background_new();

    assert(bg->archetype == &background_archetype && bg->archetype->tile_choices == &background_tile_choices_seq);
    assert(bg->header.state == bg->archetype->state && strcmp(bg->header.state, "normal") == 0 && bg->header.state_time == 0);
    assert(background_ready());
    background_shutdown();
    assert(!background_ready());
}

static void test_button_defaults_and_ready(void) {
    button_t *js = button_new(NULL);

    assert(js->archetype == &button_archetype && js->layer == js->archetype->layer);
    assert(js->header.state == js->archetype->state && strcmp(js->header.state, "normal") == 0 && js->header.state_time == 0);
    assert(button_ready());
    button_shutdown();
    assert(!button_ready());
}

/* `ra` is the module's shared vector, not the archetype's: two fresh
 * readers point at the same one. */
static void test_mouse_pos_is_shared(void) {
    const mouse_t *nf = mouse_new();
    const mouse_t *other = mouse_new();

    assert(nf->archetype == &mouse_archetype);
    assert(nf->pos != NULL && nf->pos == other->pos);
}

/* `v`, `p` and `ra` start on the archetype's vectors. */
static void test_camera_defaults_and_ready(void) {
    camera_t *qb = camera_new();

    assert(qb->archetype == &camera_archetype && qb->archetype->dpad_directions == &camera_dpad_directions_vecs);
    assert(qb->velocity == &qb->archetype->velocity && qb->pos_before_shake == &qb->archetype->pos_before_shake && qb->pos == &qb->archetype->pos && qb->shake == qb->archetype->shake);
    assert(qb->header.state == qb->archetype->state && strcmp(qb->header.state, "normal") == 0 && qb->header.state_time == 0);
    assert(camera_ready());
    camera_shutdown();
    assert(!camera_ready());
}

/* mz's own `me` replaces ke's; `jj` holds the generated `qs`. */
static void test_wormhole_defaults_and_ready(void) {
    wormhole_t *mz = wormhole_new();

    assert(mz->archetype == &wormhole_archetype && mz->targeting->archetype == &wormhole_archetype.targeting);
    assert(mz->header.state == mz->archetype->state && strcmp(mz->header.state, "placing") == 0 && mz->header.state_time == 0);
    assert(mz->radius == mz->archetype->radius);
    assert(mz->targeting->cost == mz->archetype->targeting.cost && mz->targeting->cost == p8_fixed_from_int(3));
    assert(mz->targeting->has_cost == mz->archetype->targeting.has_cost);
    assert(mz->too_far_target == wormhole_too_far_target_wrapper() && mz->too_far_target->status_text == wormhole_too_far_target_status_text);
    assert(wormhole_ready());
    wormhole_shutdown();
    assert(!wormhole_ready());
}

static void test_layout_ready(void) {
    assert(layout_row_ready() && tooltip_ready());
    layout_shutdown();
    assert(!layout_row_ready() && !tooltip_ready());
}

/* A fresh fv: its archetype's defaults copied, ke's under them. */
static void test_help_screen_defaults_and_ready(void) {
    const probe_t *g_ = pico8_fv_new();

    assert(g_->archetype == &help_screen_archetype && g_->emitter->archetype == &help_screen_archetype.emitter &&
           g_->targeting->archetype == &help_screen_archetype.targeting);
    assert(g_->pos == &g_->archetype->pos);
    assert(g_->emitter->trail_colors == g_->archetype->emitter.trail_colors);
    assert(g_->emitter->matrix_xx == g_->archetype->emitter.matrix_xx && g_->emitter->matrix_xy == g_->archetype->emitter.matrix_xy);
    assert(g_->emitter->matrix_yx == g_->archetype->emitter.matrix_yx && g_->emitter->matrix_yy == g_->archetype->emitter.matrix_yy);
    assert(g_->emitter->angle_slices == g_->archetype->emitter.angle_slices && g_->emitter->base_radius == g_->archetype->emitter.base_radius);
    assert(g_->emitter->speed_factor == g_->archetype->emitter.speed_factor);
    assert(g_->targeting->cost == g_->archetype->targeting.cost);
    assert(g_->targeting->has_cost == g_->archetype->targeting.has_cost);
    assert(g_->targeting->has_cost == 1 && g_->targeting->archetype->status_sprite == 0 && g_->targeting->archetype->has_status_sprite == 0);
    assert(g_->header.state == g_->archetype->state && strcmp(g_->header.state, "normal") == 0 && g_->header.state_time == 0);
    assert(pico8_fv_ready());
    pico8_help_shutdown();
    assert(!pico8_fv_ready());
}

/* A fresh probe: its archetype's defaults copied, ke's under them; no `ra`
 * yet. */
static void test_probe_defaults_and_ready(void) {
    probe_t *g_ = probe_new(NULL);

    assert(g_->archetype == &probe_archetype && g_->emitter->archetype == &probe_archetype.emitter &&
           g_->targeting->archetype == &probe_archetype.targeting);
    assert(g_->targeting->cost == g_->archetype->targeting.cost);
    assert(g_->targeting->has_cost == g_->archetype->targeting.has_cost && g_->targeting->has_cost == 1);
    assert(g_->targeting->archetype->status_sprite == 0 && g_->targeting->archetype->has_status_sprite == 0);
    assert(g_->emitter->speed_factor == g_->archetype->emitter.speed_factor);
    assert(g_->emitter->angle_slices == g_->archetype->emitter.angle_slices);
    assert(g_->emitter->trail_colors == g_->archetype->emitter.trail_colors);
    assert(g_->emitter->matrix_xx == g_->archetype->emitter.matrix_xx && g_->emitter->matrix_xy == g_->archetype->emitter.matrix_xy);
    assert(g_->emitter->matrix_yx == g_->archetype->emitter.matrix_yx && g_->emitter->matrix_yy == g_->archetype->emitter.matrix_yy);
    assert(g_->emitter->base_radius == g_->archetype->emitter.base_radius);
    assert(g_->pos == NULL);
    assert(g_->header.state == g_->archetype->state && strcmp(g_->header.state, "normal") == 0 && g_->header.state_time == 0);
    assert(probe_ready());
    probe_shutdown();
    assert(!probe_ready());
}

static void test_research_project_defaults_and_ready(void) {
    research_project_t *hp = research_project_new(NULL);

    assert(hp->archetype == &research_project_archetype && hp->archetype->row_tail == &research_project_row_tail_row);
    assert(research_project_ready());
    research_shutdown();
    assert(!research_project_ready());
}

/* `ra` starts on the archetype's vector. */
static void test_structure_icon_defaults_and_ready(void) {
    structure_icon_t *ie = structure_icon_new(NULL);

    assert(ie->archetype == &structure_icon_archetype && ie->pos == &ie->archetype->pos);
    assert(ie->header.state == ie->archetype->state && strcmp(ie->header.state, "normal") == 0 && ie->header.state_time == 0);
    assert(structure_icon_ready());
    structure_icon_shutdown();
    assert(!structure_icon_ready());
}

/* `cr` holds the spec's record. */
static void test_score_panel_defaults_and_ready(void) {
    pico8_pj_t *pj = pico8_pj_new();
    const pico8_pj_cr_t *cr = pico8_pj_cr();

    assert(pj->def == &score_panel_archetype && pj->fa == pj->def->layer);
    assert(pj->ed == NULL);
    assert(cr->hi->x == 0 && cr->hi->y == p8_fixed_from_int(11));
    assert(cr->gq->x == 0 && cr->gq->y == p8_fixed_from_int(11));
    assert(cr->ca == p8_fixed_from_int(10));
    assert(pj->header.state == pj->def->state && strcmp(pj->header.state, "normal") == 0 && pj->header.state_time == 0);
    assert(pico8_pj_ready());
    pico8_score_panel_shutdown();
    assert(!pico8_pj_ready());
}

/* hy and trade: an empty spec, a root struct. */
static void test_economy_ready(void) {
    economy_t *hy = economy_new();

    assert(hy->header.class == economy_class());
    assert(economy_ready());
    economy_shutdown();
    assert(!economy_ready());
}

static void test_trade_ready(void) {
    trade_route_t *trade = trade_new(game_zone(), NULL);

    assert(trade->header.class == trade_class());
    assert(trade_ready());
    trade_shutdown();
    assert(!trade_ready());
}

int main(void) {
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);

    assert(!trade_ready());
    assert(!economy_ready());
    assert(!research_project_ready());
    assert(!pico8_pj_ready());
    assert(!structure_icon_ready());
    assert(!probe_ready());
    assert(!pico8_fv_ready());
    pico8_cart_load();
    reset_world();

    test_second_reset_world_keeps_specs();
    test_background_defaults_and_ready();
    test_button_defaults_and_ready();
    test_camera_defaults_and_ready();
    test_mouse_pos_is_shared();
    test_wormhole_defaults_and_ready();
    test_layout_ready();
    test_help_screen_defaults_and_ready();
    test_probe_defaults_and_ready();
    test_research_project_defaults_and_ready();
    test_structure_icon_defaults_and_ready();
    test_score_panel_defaults_and_ready();
    test_economy_ready();
    test_trade_ready();

    printf("test_spec_defaults: all tests passed\n");
    return 0;
}
