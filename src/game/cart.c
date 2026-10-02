#include "game/cart.h"

#include <assert.h>
#include <stddef.h>

#include "p8/audio.h"
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
#include "p8/mem.h"
#include "game/menu.h"
#include "game/mouse.h"
#include "game/message_box.h"
#include "game/draw_functions.h"
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

static void quitter_partie(void) {
    entity_registry_abandon_live_entities();
    pico8_cart_vider_file_sortie();
    game_zone_reset();
    entity_registry_forget_registries();
    planet_forget_grab_arenas();
}

/* pico8_data_init() and pico8_score_panel_init() each run further module
 * inits of their own (research, score-panel display); listing those
 * here would run them twice. */
void pico8_cart_load(void) {
    p8_memory_t *mem;

    quitter_partie();
    entity_init();
    assert(entity_ready() && "ke must be declared before every ke-descended class");
    camera_init();
    mouse_init();
    ui_init();

    layout_init();
    draw_functions_init();
    trade_init();

    button_init();
    modal_backdrop_init();
    background_init();
    pico8_mh_init();
    pico8_iv_init();
    probe_init();
    economy_init();

    mem = dispatch_context_mem();
    if (mem != NULL) {
        data_init();
    }

    planet_init();
    slipway_init();
    planet_family_init();
    assert(structure_ready() && burst_ready() && planet_ready() &&
           "qh and ga extend jo: pico8_jo_init() must run first");
    planet_family_trade_init();
    assert(slipgate_ready() && synthesizer_ready() && structure_ready() && ascension_ready() &&
           "nr extends qh and mx extends hj: pico8_planet_family_init() must run first");
    wormhole_init();

    pico8_help_init();
    assert(pico8_fv_ready() && probe_ready() && "fv extends g_: pico8_probe_init() must run first");
    pico8_score_panel_init();
    structure_icon_init();
    pico8_menu_init();

    pico8_help_set_eu(pico8_eu);
    pico8_score_panel_set_init(pico8_init_dispatch);
}

/* Une place rendue peut revenir a la meme adresse : `kc == ju.ed` serait
 * vrai pour une boite neuve, et `gj.m_` ecrirait dans un autre objet.
 * ASan ne voit pas la comparaison. */
static void oublier(const entity_t *e) {
    ui_t *ok = main_ui();
    pico8_pj_t *ju = pico8_ju();

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
    if ((const entity_t *)pico8_kc() == e) {
        pico8_msgbox_set_kc(NULL);
    }
    if (ju != NULL && (const entity_t *)ju->ed == e) {
        ju->ed = NULL;
    }
    economy_score_difficulty_quits(e);
    planet_forget_targeted_by(e);
}

/* Point d'oubli de partie : coupe les references a la generation que
 * pico8_zone_partie_rendre va rendre. */
static void oublier_partie(void) {
    planet_forget_grab_arenas();
}

void pico8_cart_vider_file_sortie(void) {
    exit_queue_flush(oublier);
    if (game_zone_pending()) {
        oublier_partie();
        game_zone_release();
    }
}

void pico8_cart_shutdown(void) {
    quitter_partie();
    pico8_menu_shutdown();
    structure_icon_shutdown();
    pico8_score_panel_shutdown();
    pico8_help_shutdown();

    wormhole_shutdown();
    planet_family_trade_shutdown();
    planet_family_shutdown();
    slipway_shutdown();
    planet_shutdown();

    data_init_data_shutdown();

    economy_shutdown();
    probe_shutdown();
    pico8_iv_shutdown();
    pico8_mh_shutdown();
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

void pico8__init(void) {
    p8_memory_t *mem = dispatch_context_mem();

    assert(structure_icon_ready() && "pico8_cart_load() must run before _init()");
    assert(pico8_eu_field_value() == pico8_eu &&
           pico8_init_field_value() == pico8_init_dispatch &&
           "pico8_help_init()/pico8_score_panel_init() reset these bridges, so pico8_cart_load() must "
           "install them afterwards: every button copies the value when it is built");

    hud_build_palette_table(mem);
    reset_world();

    /* `ie.gy,kc=ie.eh`: Lua pads the extra name with nil, so `gy` takes `eh`
     * and `kc` is cleared. `gy` aliases `eh`'s vector, it is not a copy. */
    structure_icon_set_cursor(structure_icon_rest_cursor());
    pico8_msgbox_set_kc(NULL);

    (void)pico8_fv_new();
    /* TROU-10 */
    p8_music(0);
}

void pico8_init_dispatch(button_t *self) {
    (void)self;
    pico8__init();
}

void pico8_eu(button_t *nj) {
    const entity_kind_t *gx[7];
    int i;

    assert(economy_ready() && structure_ready() && "pico8_cart_load() must run before eu()");

    economy_score_set_difficulty(nj);
    game_zone_flip();

    /* `jl()` runs a second time here, as in the cart: it unconditionally
     * rebuilds hv/cv/cg and the mo/je/ok singletons. */
    reset_world();
    (void)background_new();
    galaxy_populate(p8_fixed_from_int(90));

    probe_set_main_economy((entity_t *)economy_new());
    pico8_pj_set_ju(pico8_pj_new());

    if (pico8_ha_pz() != NULL) {
        int j;

        for (j = 1; j <= GALAXY_MENU_COUNT; j++) {
            (void)pico8_ix_button(j);
        }
    }

    gx[0] = structure_kind();
    gx[1] = processor_kind();
    gx[2] = slipgate_kind();
    gx[3] = infraspace_gate_kind();
    gx[4] = trading_hub_kind();
    gx[5] = synthesizer_kind();
    gx[6] = ascension_kind();

    for (i = 0; i < 7; i++) {
        structure_icon_keys_t keys = {0};

        keys.kind = gx[i];
        keys.tooltip = tooltip_new(game_zone(), &icon_tooltips_table[i]);
        (void)structure_icon_new(&keys);
    }

    assert(main_economy_typed() != NULL && "fm must be the hy of this game before fm:lj()");
    economy_recompute(main_economy_typed());
}

/* Ici et pas dans un pilote : rejeu, raylib, couverture et TU passent tous
 * par update60, et draw, qui suit dans la meme frame, relit ce qu'update a
 * fait en zone frame. */
void pico8__update60(void) {
    frame_zone_reset();
    pico8_cart_vider_file_sortie();
    update_entities();
    ui_update(dispatch_context_mem(), main_ui());
}

void pico8__draw(void) {
    p8_memory_t *mem = dispatch_context_mem();

    p8_cls(mem);
    hud_select_palette_block_default(mem);
    draw_layers("draw");
    draw_layers("draw_overlay");
}
