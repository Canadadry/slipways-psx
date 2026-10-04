#include "game/cart.h"

#include <assert.h>
#include <stddef.h>

#include "game/background.h"
#include "engine/bootstrap.h"
#include "game/main_economy.h"
#include "engine/entity_class.h"
#include "game/data_init.h"
#include "engine/dispatch_context.h"
#include "game/economy.h"
#include "game/economy_score.h"
#include "engine/entity_registry.h"
#include "engine/exit_queue.h"
#include "p8/fixed.h"
#include "game/galaxy.h"
#include "p8/gfx.h"
#include "game/help.h"
#include "game/hud.h"
#include "engine/entity.h"
#include "game/layout.h"
#include "p8/vram.h"
#include "game/menu.h"
#include "game/mouse.h"
#include "game/message_box.h"
#include "game/grab_arena.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/probe.h"
#include "game/structure_icon.h"
#include "game/score_panel.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "game/trade.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "game/wormhole.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

static void quit_game_zone(void) {
    entity_registry_abandon_live_entities();
    cart_flush_exit_queue();
    game_zone_reset();
    entity_registry_forget_registries();
    grab_arena_forget_free();
}

/* Do not list the inits data_init() and score_panel_init() run themselves: they would run twice. */
void cart_load(void) {
    p8_vram_t *memory;

    quit_game_zone();
    entity_init();
    assert(entity_ready() && "cart_load: entity_init() must run before every class that extends entity_t");
    camera_init();
    mouse_init();
    ui_init();

    layout_init();
    trade_init();

    button_init();
    modal_backdrop_init();
    background_init();
    message_box_init();
    toast_init();
    probe_init();
    economy_init();

    memory = dispatch_context_mem();
    if (memory != NULL) {
        data_init();
    }

    planet_init();
    slipway_init();
    planet_family_init();
    assert(structure_ready() && burst_ready() && planet_ready() &&
           "cart_load: structure and burst extend planet: planet_init() must run first");
    planet_family_trade_init();
    assert(slipgate_ready() && synthesizer_ready() && structure_ready() && ascension_ready() &&
           "cart_load: slipgate extends structure and synthesizer extends ascension: planet_family_init() must run first");
    wormhole_init();

    help_init();
    assert(help_screen_ready() && probe_ready() && "cart_load: the help screen extends probe: probe_init() must run first");
    score_panel_init();
    structure_icon_init();
    menu_init();

    help_set_start_game(cart_start_game);
    score_panel_set_init(cart_init_method);
}

/* A released slot can come back at the same address, so stale pointers must be cleared here; ASan does not see the comparison. */
static void forget(const entity_t *e) {
    ui_t *ok = main_ui();
    score_panel_t *ju = main_score_panel();

    if (ok != NULL && ok->hovered == e) {
        ok->hovered = NULL;
    }
    if (ok != NULL && ok->previous_hovered == e) {
        ok->previous_hovered = NULL;
    }
    if (ok != NULL && ok->left_held != NULL && ok->left_held->class == slipway_class() &&
        ((slipway_t *)ok->left_held)->end == e) {
        ((slipway_t *)ok->left_held)->end = NULL;
    }
    if ((const entity_t *)shown_message_box() == e) {
        message_box_set_shown(NULL);
    }
    if (ju != NULL && (const entity_t *)ju->last_box == e) {
        ju->last_box = NULL;
    }
    economy_score_difficulty_quits(e);
    planet_forget_targeted_by(e);
}

static void forget_game_zone(void) {
    grab_arena_forget_free();
}

void cart_flush_exit_queue(void) {
    exit_queue_flush(forget);
    if (game_zone_pending()) {
        forget_game_zone();
        game_zone_release();
    }
}

void cart_shutdown(void) {
    quit_game_zone();
    menu_shutdown();
    structure_icon_shutdown();
    score_panel_shutdown();
    help_shutdown();

    wormhole_shutdown();
    planet_family_trade_shutdown();
    planet_family_shutdown();
    slipway_shutdown();
    planet_shutdown();

    data_init_shutdown();

    economy_shutdown();
    probe_shutdown();
    toast_shutdown();
    message_box_shutdown();
    background_shutdown();
    modal_backdrop_shutdown();
    button_shutdown();

    trade_shutdown();
    layout_shutdown();

    ui_shutdown();
    mouse_shutdown();
    camera_shutdown();
    entity_shutdown();
}

void cart_init(void) {
    assert(structure_icon_ready() && "cart_init: cart_load() must run first");
    assert(help_start_game_field_value() == cart_start_game &&
           score_panel_init_field_value() == cart_init_method &&
           "cart_init: help_init() and score_panel_init() reset these bridges, so cart_load() must "
           "install them afterwards: every button copies the value when it is built");

    /* An empty generation needs no flip, and flipping it would trip flip_zone_flip's assert. */
    if (!game_zone_pending() && game_zone_used() > 0) game_zone_flip();
    reset_world(NULL);

    structure_icon_set_cursor(structure_icon_rest_cursor());
    message_box_set_shown(NULL);

    if (main_economy_typed() != NULL) probe_set_main_economy(NULL);
    score_panel_set_main(NULL);

    (void)help_screen_new();
}

void cart_init_method(button_t *self) {
    (void)self;
    cart_init();
}

const entity_registry_reserve_t cart_game_registry_reserve = {
    329,
    {
        [ENTITY_MESSAGE_DRAW] = 322,
        [ENTITY_MESSAGE_DRAW_OVERLAY] = 265,
        [ENTITY_MESSAGE_HIT_TEST] = 125,
        [ENTITY_MESSAGE_NEW_YEAR] = 111,
        [ENTITY_MESSAGE_REFRESH] = 120,
    },
    {
        [TAG_STRUCTURE] = 8,
        [TAG_ASCENSION] = 3,
        [TAG_SLIPGATE] = 14,
        [TAG_PROBE] = 2,
        [TAG_MESSAGE_BOX] = 4,
        [TAG_SLIPWAY] = 216,
        [TAG_WORMHOLE] = 3,
        [TAG_PLANET] = 110,
        [TAG_NODE] = 110,
        [TAG_TARGETING] = 110,
    },
};

void cart_start_game(button_t *chosen_option) {
    const entity_kind_t *kind_table[7];
    int index;

    assert(economy_ready() && structure_ready() && "cart_start_game: cart_load() must run first");

    economy_score_set_difficulty(chosen_option);
    game_zone_flip();

    /* Not redundant with cart_init()'s call: the registries, camera, mouse and UI must be rebuilt. */
    reset_world(&cart_game_registry_reserve);
    (void)background_new();
    galaxy_populate(p8_fixed_from_int(90));

    probe_set_main_economy((entity_t *)economy_new());
    score_panel_set_main(score_panel_new());

    if (pause_menu_open_box() != NULL) {
        int j;

        for (j = 1; j <= GALAXY_MENU_COUNT; j++) {
            (void)galaxy_menu_button(j);
        }
    }

    kind_table[0] = structure_kind();
    kind_table[1] = processor_kind();
    kind_table[2] = slipgate_kind();
    kind_table[3] = infraspace_gate_kind();
    kind_table[4] = trading_hub_kind();
    kind_table[5] = synthesizer_kind();
    kind_table[6] = ascension_kind();

    for (index = 0; index < 7; index++) {
        structure_icon_keys_t keys = {0};

        keys.kind = kind_table[index];
        keys.tooltip = tooltip_new(game_zone(), &icon_tooltips_table[index]);
        (void)structure_icon_new(&keys);
    }

    assert(main_economy_typed() != NULL && "cart_start_game: the main economy must be this game's economy before economy_recompute()");
    economy_recompute(main_economy_typed());
}

/* Here, not in a driver: every driver goes through cart_update60(), and cart_draw() reads what it wrote in the frame zone. */
void cart_update60(void) {
    frame_zone_reset();
    cart_flush_exit_queue();
    update_entities();
    ui_update(dispatch_context_mem(), main_ui());
}

void cart_draw(void) {
    p8_vram_t *mem = dispatch_context_mem();

    p8_cls(mem);
    hud_select_palette_block_default(mem);
    draw_layers(ENTITY_MESSAGE_DRAW);
    draw_layers(ENTITY_MESSAGE_DRAW_OVERLAY);
}
