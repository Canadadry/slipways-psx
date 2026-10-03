/* The kernel's component links on entities whose constructor calls
 * `malloc` without zeroing. The binary runs itself again with `malloc` filling every
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
#include "p8/vram.h"
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

static p8_vram_t memory;
static p8_graphics_t graphics;
static p8_input_t input;

static void rerun_with_malloc_filling(char **argv) {
    if (getenv("MALLOC_FILLS") != NULL) {
        return;
    }
    setenv("MALLOC_PERTURB_", "90", 1);
    setenv("GLIBC_TUNABLES", "glibc.malloc.tcache_count=0", 1);
    setenv("MallocScribble", "1", 1);
    setenv("MALLOC_FILLS", "1", 1);
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

/* Each link of `want`: the one a class carries, or NULL; its `class`,
 * `state` and `state_time` are not compared. */
static void assert_links(const entity_t *h, entity_t want) {
    assert(h->layer == want.layer);
    assert(h->pos == want.pos);
    assert(h->map_pos == want.map_pos);
    assert(h->hit_box == want.hit_box);
    assert(h->cursor_sprite == want.cursor_sprite);
    assert(h->infraspace == want.infraspace);
    assert(h->shake == want.shake);
    assert(h->fold_anchor == want.fold_anchor);
    assert(h->is_dead == want.is_dead);
    assert(h->is_hovered == want.is_hovered);
    assert(h->emitter == want.emitter);
    assert(h->targeting == want.targeting);
}

/* Non-carriers: no link but `pos`, at most. The mouse point a slipway's
 * `drag` builds has its own test below. */
static void test_non_carriers_have_null_links(void) {
    structure_icon_t *ie = structure_icon_new(NULL);
    structure_icon_kind_t view = structure_icon_kind_view(ie);
    mouse_t *nf = mouse_new();
    layout_row_t *lq = layout_row_new(&heap_allocator, NULL);

    assert_links(&nf->header, (entity_t){.pos = (const vec_t *const *)&nf->pos});
    assert_links(&lq->header, (entity_t){.pos = &lq->pos});
    assert_links(&tooltip_new(&heap_allocator, NULL)->header, (entity_t){0});
    assert_links(&research_project_new(NULL)->header, (entity_t){0});
    assert_links(&economy_new()->header, (entity_t){0});
    assert_links(&trade_new(game_zone(), NULL)->header, (entity_t){0});
    assert_links(&view.header, (entity_t){0});
}

/* Carriers: the archetype's field for a class that never writes it, the
 * instance's otherwise. */
static void test_carriers_link_their_storage(void) {
    background_t *background = background_new();
    button_t *button = button_new(NULL);
    structure_icon_t *structure_icon = structure_icon_new(NULL);
    modal_backdrop_t *modal_backdrop = modal_backdrop_new(NULL);
    ui_t *built_ui = ui_new();
    camera_t *camera = camera_new();
    message_box_t *message_box = message_box_new(NULL);
    toast_t *toast = toast_new(NULL);
    probe_t *help_screen = help_screen_new();
    planet_t *planet = planet_new(NULL);
    planet_t *infraspace_gate = infraspace_gate_new(NULL);
    planet_t *infraspace_gate_planet = infraspace_gate;

    assert_links(&background->header, (entity_t){.layer = &background->archetype->layer, .is_hovered = &background->is_hovered});
    assert_links(&button->header,
                 (entity_t){.layer = &button->layer, .pos = &button->pos, .hit_box = button->header.hit_box, .is_hovered = &button->is_hovered});
    assert(button->header.hit_box != NULL);
    assert_links(&structure_icon->header,
                 (entity_t){.layer = &structure_icon->archetype->layer, .pos = &structure_icon->pos, .hit_box = &structure_icon->archetype->hit_box, .is_hovered = &structure_icon->is_hovered});
    assert_links(&modal_backdrop->header, (entity_t){.layer = &modal_backdrop->archetype->layer, .is_dead = &modal_backdrop->is_dead, .is_hovered = &modal_backdrop->is_hovered});
    assert_links(&built_ui->header, (entity_t){.layer = &built_ui->archetype->layer, .cursor_sprite = &built_ui->archetype->cursor_sprite});
    assert_links(&camera->header, (entity_t){.pos = &camera->pos, .map_pos = &camera->pos_before_shake, .shake = &camera->shake});
    assert_links(&message_box->header, (entity_t){.layer = &message_box->archetype->layer, .is_dead = &message_box->is_dead});
    assert_links(&toast->header, (entity_t){.layer = &toast->archetype->layer, .pos = &toast->pos, .is_dead = &toast->is_dead});
    assert_links(&help_screen->header, (entity_t){.layer = &help_screen->archetype->layer,
                                                         .pos = &help_screen->pos,
                                                         .map_pos = &help_screen->map_pos,
                                                         .hit_box = &help_screen->archetype->hit_box,
                                                         .cursor_sprite = &help_screen->archetype->cursor_sprite,
                                                         .fold_anchor = &help_screen->archetype->fold_anchor,
                                                         .is_dead = &help_screen->is_dead,
                                                         .is_hovered = &help_screen->is_hovered,
                                                         .emitter = &help_screen->emitter_own,
                                                         .targeting = &help_screen->targeting_own});
    assert(*help_screen->header.fold_anchor == p8_fixed_from_int(1) &&
           "a probe's fold_anchor is its archetype's 1.0: 0 would read as absent");
    assert_links(&planet->header, (entity_t){.layer = &planet->archetype->layer,
                                                      .pos = (const vec_t *const *)&planet->pos,
                                                      .map_pos = (const vec_t *const *)&planet->map_pos,
                                                      .hit_box = &planet->archetype->hit_box,
                                                      .shake = &planet->shake,
                                                      .fold_anchor = &planet->fold_anchor,
                                                      .is_dead = &planet->is_dead,
                                                      .is_hovered = &planet->is_hovered});
    assert(planet->targeting == NULL && planet->emitter == NULL);
    assert_links(&infraspace_gate_planet->header, (entity_t){.layer = &infraspace_gate_planet->archetype->layer,
                                                         .pos = (const vec_t *const *)&infraspace_gate_planet->pos,
                                                         .map_pos = (const vec_t *const *)&infraspace_gate_planet->map_pos,
                                                         .hit_box = &infraspace_gate_planet->archetype->hit_box,
                                                         .infraspace = &infraspace_gate_planet->archetype->infraspace,
                                                         .shake = &infraspace_gate_planet->shake,
                                                         .fold_anchor = &infraspace_gate_planet->fold_anchor,
                                                         .is_dead = &infraspace_gate_planet->is_dead,
                                                         .is_hovered = &infraspace_gate_planet->is_hovered,
                                                         .targeting = &infraspace_gate->targeting_own});
}

/* A planet's links, but for its emitter and probe. */
static entity_t planet_links(planet_t *jo) {
    return (entity_t){.layer = &jo->archetype->layer,
                                   .pos = (const vec_t *const *)&jo->pos,
                                   .map_pos = (const vec_t *const *)&jo->map_pos,
                                   .hit_box = &jo->archetype->hit_box,
                                   .shake = &jo->shake,
                                   .fold_anchor = &jo->fold_anchor,
                                   .is_dead = &jo->is_dead,
                                   .is_hovered = &jo->is_hovered};
}

static void assert_structure_links(planet_t *qh, emitter_t *emitter) {
    entity_t want = planet_links(qh);

    want.emitter = emitter;
    want.targeting = &qh->targeting_own;
    assert_links(&qh->header, want);
    assert(qh->targeting == qh->header.targeting && qh->emitter == qh->header.emitter);
}

/* Emitter and probe carriers: each link is the component the instance
 * stores in itself, the one its own member points at. */
static void test_component_carriers_link_their_own(void) {
    probe_t *g_ = probe_new(NULL);
    planet_t *nl = trading_hub_new(NULL);
    planet_t *os = processor_new(NULL);
    planet_t *hj = ascension_new(NULL);
    planet_t *pm = protostar_new(NULL);
    planet_t *mx = synthesizer_new(NULL);
    planet_t *nr = slipgate_new(NULL);
    planet_t *ga = burst_new(NULL);
    wormhole_t *mz = wormhole_new();
    entity_t want = planet_links(ga);

    assert_links(&g_->header, (entity_t){.layer = &g_->archetype->layer,
                                                      .pos = &g_->pos,
                                                      .map_pos = &g_->map_pos,
                                                      .fold_anchor = &g_->archetype->fold_anchor,
                                                      .is_dead = &g_->is_dead,
                                                      .emitter = &g_->emitter_own,
                                                      .targeting = &g_->targeting_own});
    assert(g_->emitter == g_->header.emitter && g_->targeting == g_->header.targeting);
    assert(*g_->header.fold_anchor == p8_fixed_from_int(1) &&
           "a probe's fold_anchor is its archetype's 1.0: 0 would read as absent");
    assert_structure_links(structure_new(NULL), NULL);
    assert_structure_links(nl, NULL);
    assert_structure_links(os, NULL);
    assert_structure_links(hj, &hj->emitter_own);
    assert_structure_links(pm, &pm->emitter_own);
    assert_structure_links(mx, &mx->emitter_own);
    assert_structure_links(nr, NULL);
    want.emitter = &ga->emitter_own;
    assert_links(&ga->header, want);
    assert(ga->emitter == ga->header.emitter && ga->targeting == NULL);
    assert_links(&mz->header, (entity_t){.layer = &mz->archetype->layer,
                                                      .pos = &mz->pos,
                                                      .map_pos = &mz->map_pos,
                                                      .is_dead = &mz->is_dead,
                                                      .targeting = &mz->targeting_own});
    assert(mz->targeting == mz->header.targeting);
}

static slipway_t *slipway_dragged_over_nothing(entity_t *qu) {
    slipway_keys_t keys = {0};
    vec_t *mp = malloc(sizeof *mp);
    slipway_t *jc;

    keys.start = qu;
    jc = slipway_new(&keys);
    *mp = vec_make(p8_fixed_from_int(-640), p8_fixed_from_int(-640));
    main_ui()->hovered = NULL;
    slipway_class()->drag(&jc->header, mp, NULL);
    return jc;
}

/* The point a slipway's `drag` builds over nothing carries `pos` and
 * `map_pos` alone; the slipway's `infraspace` is its start's, else absent. */
static void test_mouse_point_has_null_links(void) {
    slipway_t *jc = slipway_dragged_over_nothing(&planet_new(NULL)->header);
    slipway_mouse_point_t *point = (slipway_mouse_point_t *)jc->end;
    slipway_t *from_infraspace_gate = slipway_dragged_over_nothing(&infraspace_gate_new(NULL)->header);

    assert(point != NULL && point->header.class == slipway_mouse_point_class());
    assert_links(&point->header, (entity_t){.pos = &point->pos, .map_pos = &point->map_pos});
    assert(!jc->has_infraspace && jc->layer == p8_fixed_from_int(4));
    assert_links(&jc->header, (entity_t){.layer = &jc->layer, .is_dead = &jc->is_dead, .targeting = &jc->targeting_own});
    assert(jc->targeting == jc->header.targeting);
    assert(from_infraspace_gate->has_infraspace && from_infraspace_gate->infraspace == infraspace_gate_archetype.infraspace && from_infraspace_gate->layer == p8_fixed_from_int(2));
    assert_links(&from_infraspace_gate->header,
                 (entity_t){
                     .layer = &from_infraspace_gate->layer, .infraspace = &from_infraspace_gate->infraspace, .is_dead = &from_infraspace_gate->is_dead, .targeting = &from_infraspace_gate->targeting_own});
}

int main(int argc, char **argv) {
    (void)argc;
    rerun_with_malloc_filling(argv);
    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);
    cart_load();
    reset_world();

    assert_malloc_fills();
    test_non_carriers_have_null_links();
    test_carriers_link_their_storage();
    test_component_carriers_link_their_own();
    test_mouse_point_has_null_links();

    printf("test_entity_header: all tests passed\n");
    return 0;
}
