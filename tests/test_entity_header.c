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
#include "engine/entity_id.h"
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

/* Each link of `want`: the one a class carries, or NULL; each value whose
 * bit `want.has` sets; its `class`, `state` and `state_time` are not
 * compared. */
static void assert_links(const entity_t *h, entity_t want) {
    assert(h->has == want.has);
    assert(!entity_has(h, ENTITY_HAS_LAYER) || h->layer == want.layer);
    assert(h->pos == want.pos);
    assert(h->map_pos == want.map_pos);
    assert(!entity_has(h, ENTITY_HAS_HIT_BOX) || (h->hit_box.left == want.hit_box.left && h->hit_box.top == want.hit_box.top &&
                                                  h->hit_box.right == want.hit_box.right && h->hit_box.bottom == want.hit_box.bottom));
    assert(!entity_has(h, ENTITY_HAS_CURSOR_SPRITE) || h->cursor_sprite == want.cursor_sprite);
    assert(!entity_has(h, ENTITY_HAS_INFRASPACE) || h->infraspace == want.infraspace);
    assert(!entity_has(h, ENTITY_HAS_SHAKE) || h->shake == want.shake);
    assert(!entity_has(h, ENTITY_HAS_FOLD_ANCHOR) || h->fold_anchor == want.fold_anchor);
    assert(!entity_has(h, ENTITY_HAS_IS_DEAD) || h->is_dead == want.is_dead);
    assert(!entity_has(h, ENTITY_HAS_IS_HOVERED) || h->is_hovered == want.is_hovered);
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

    assert_links(&background->header, (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_IS_HOVERED, .layer = background->archetype->layer});
    assert_links(&button->header,
                 (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_HIT_BOX | ENTITY_HAS_IS_HOVERED,
                            .layer = button_archetype.layer,
                            .pos = &button->pos,
                            .hit_box = {p8_fixed_from_int(0), p8_fixed_from_int(-1), button->width, p8_fixed_from_int(11)}});
    assert_links(&structure_icon->header,
                 (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_HIT_BOX | ENTITY_HAS_IS_HOVERED, .layer = structure_icon->archetype->layer, .pos = &structure_icon->pos, .hit_box = structure_icon->archetype->hit_box});
    assert_links(&modal_backdrop->header, (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_IS_DEAD | ENTITY_HAS_IS_HOVERED, .layer = modal_backdrop->archetype->layer});
    assert_links(&built_ui->header, (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_CURSOR_SPRITE, .layer = built_ui->archetype->layer, .cursor_sprite = built_ui->archetype->cursor_sprite});
    assert_links(&camera->header, (entity_t){.has = ENTITY_HAS_SHAKE, .pos = &camera->pos, .map_pos = &camera->pos_before_shake, .shake = camera->archetype->shake});
    assert_links(&message_box->header, (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_IS_DEAD, .layer = message_box->archetype->layer});
    assert_links(&toast->header, (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_IS_DEAD, .layer = toast->archetype->layer, .pos = &toast->pos});
    assert_links(&help_screen->header, (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_HIT_BOX | ENTITY_HAS_CURSOR_SPRITE | ENTITY_HAS_FOLD_ANCHOR | ENTITY_HAS_IS_DEAD | ENTITY_HAS_IS_HOVERED,
                                                         .layer = help_screen->archetype->layer,
                                                         .pos = &help_screen->pos,
                                                         .map_pos = &help_screen->map_pos,
                                                         .hit_box = help_screen->archetype->hit_box,
                                                         .cursor_sprite = help_screen->archetype->cursor_sprite,
                                                         .fold_anchor = help_screen->archetype->fold_anchor,
                                                         .emitter = &help_screen->emitter_own,
                                                         .targeting = &help_screen->targeting_own});
    assert(help_screen->header.fold_anchor == p8_fixed_from_int(1) &&
           "a probe's fold_anchor is its archetype's 1.0: 0 would read as absent");
    assert_links(&planet->header, (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_HIT_BOX | ENTITY_HAS_FOLD_ANCHOR | ENTITY_HAS_SHAKE | ENTITY_HAS_IS_DEAD | ENTITY_HAS_IS_HOVERED,
                                                      .layer = planet->archetype->layer,
                                                      .pos = (const vec_t *const *)&planet->pos,
                                                      .map_pos = (const vec_t *const *)&planet->map_pos,
                                                      .hit_box = planet->archetype->hit_box, .shake = planet->archetype->shake,
                                                      .fold_anchor = 0});
    assert(planet->targeting == NULL && planet->emitter == NULL);
    assert_links(&infraspace_gate_planet->header, (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_HIT_BOX | ENTITY_HAS_INFRASPACE | ENTITY_HAS_FOLD_ANCHOR | ENTITY_HAS_SHAKE | ENTITY_HAS_IS_DEAD | ENTITY_HAS_IS_HOVERED,
                                                         .layer = infraspace_gate_planet->archetype->layer,
                                                         .pos = (const vec_t *const *)&infraspace_gate_planet->pos,
                                                         .map_pos = (const vec_t *const *)&infraspace_gate_planet->map_pos,
                                                         .hit_box = infraspace_gate_planet->archetype->hit_box,
                                                         .infraspace = infraspace_gate_planet->archetype->infraspace, .shake = infraspace_gate_planet->archetype->shake,
                                                         .fold_anchor = 0,
                                                         .targeting = &infraspace_gate->targeting_own});
}

/* A planet's links, but for its emitter and probe. */
static entity_t planet_links(planet_t *jo) {
    return (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_HIT_BOX | ENTITY_HAS_FOLD_ANCHOR | ENTITY_HAS_SHAKE | ENTITY_HAS_IS_DEAD | ENTITY_HAS_IS_HOVERED,
                                   .layer = jo->archetype->layer,
                                   .pos = (const vec_t *const *)&jo->pos,
                                   .map_pos = (const vec_t *const *)&jo->map_pos,
                                   .hit_box = jo->archetype->hit_box, .shake = jo->archetype->shake,
                                   .fold_anchor = 0};
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

    assert_links(&g_->header, (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_FOLD_ANCHOR | ENTITY_HAS_IS_DEAD,
                                                      .layer = g_->archetype->layer,
                                                      .pos = &g_->pos,
                                                      .map_pos = &g_->map_pos,
                                                      .fold_anchor = g_->archetype->fold_anchor,
                                                      .emitter = &g_->emitter_own,
                                                      .targeting = &g_->targeting_own});
    assert(g_->emitter == g_->header.emitter && g_->targeting == g_->header.targeting);
    assert(g_->header.fold_anchor == p8_fixed_from_int(1) &&
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
    assert_links(&mz->header, (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_IS_DEAD,
                                                      .layer = mz->archetype->layer,
                                                      .pos = &mz->pos,
                                                      .map_pos = &mz->map_pos,
                                                      .targeting = &mz->targeting_own});
    assert(mz->targeting == mz->header.targeting);
}

static slipway_t *slipway_dragged_over_nothing(entity_t *qu) {
    slipway_keys_t keys = {0};
    vec_t *mp = malloc(sizeof *mp);
    slipway_t *jc;

    keys.start = entity_id_link(qu);
    jc = slipway_new(&keys);
    *mp = vec_make(p8_fixed_from_int(-640), p8_fixed_from_int(-640));
    main_ui()->hovered = entity_id_link(NULL);
    slipway_class()->drag(&jc->header, mp, NULL);
    return jc;
}

/* The point a slipway's `drag` builds over nothing carries `pos` and
 * `map_pos` alone; the slipway's `infraspace` is its start's, else absent. */
static void test_mouse_point_has_null_links(void) {
    slipway_t *jc = slipway_dragged_over_nothing(&planet_new(NULL)->header);
    slipway_mouse_point_t *point = (slipway_mouse_point_t *)slipway_end(jc);
    slipway_t *from_infraspace_gate = slipway_dragged_over_nothing(&infraspace_gate_new(NULL)->header);

    assert(point != NULL && point->header.class == slipway_mouse_point_class());
    assert_links(&point->header, (entity_t){.pos = &point->pos, .map_pos = &point->map_pos});
    assert_links(&jc->header, (entity_t){.has = ENTITY_HAS_LAYER | ENTITY_HAS_IS_DEAD, .layer = p8_fixed_from_int(4), .targeting = &jc->targeting_own});
    assert(jc->targeting == jc->header.targeting);
    assert_links(&from_infraspace_gate->header,
                 (entity_t){
                     .has = ENTITY_HAS_LAYER | ENTITY_HAS_INFRASPACE | ENTITY_HAS_IS_DEAD,
                     .layer = p8_fixed_from_int(2),
                     .infraspace = infraspace_gate_archetype.infraspace, .targeting = &from_infraspace_gate->targeting_own});
}

static int same_vec(vec_t a, vec_t b) {
    return a.x == b.x && a.y == b.y;
}

/* A position reads as the value the class holds, and as the zero vector
 * with its presence test false when the class or the instance has none. */
static void test_position_readers(void) {
    camera_t *camera = camera_new();
    wormhole_t *wormhole = wormhole_new();
    const entity_t *tooltip = &tooltip_new(&heap_allocator, NULL)->header;

    assert(entity_has_pos(&camera->header) && entity_has_map_pos(&camera->header));
    assert(same_vec(entity_pos(&camera->header), *camera->pos));
    assert(same_vec(entity_map_pos(&camera->header), *camera->pos_before_shake));
    assert(!entity_has_map_pos(&wormhole->header));
    assert(same_vec(entity_map_pos(&wormhole->header), vec_make(0, 0)));
    assert(!entity_has_pos(tooltip) && !entity_has_map_pos(tooltip));
    assert(same_vec(entity_pos(tooltip), vec_make(0, 0)) && same_vec(entity_map_pos(tooltip), vec_make(0, 0)));
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
    reset_world(NULL);

    assert_malloc_fills();
    test_non_carriers_have_null_links();
    test_carriers_link_their_storage();
    test_component_carriers_link_their_own();
    test_mouse_point_has_null_links();
    test_position_readers();

    printf("test_entity_header: all tests passed\n");
    return 0;
}
