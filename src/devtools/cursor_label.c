#include "devtools/cursor_label.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "game/background.h"
#include "game/cart.h"
#include "game/help.h"
#include "game/menu.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/score_panel.h"
#include "game/structure_icon.h"
#include "game/ui_button.h"

static const entity_t *first_to_receive(const ui_t *ui) {
    int layer;
    if (ui == NULL || ui->hovered_by_layer == NULL) {
        return NULL;
    }
    for (layer = UI_HOVER_LAYERS - 1; layer >= 0; layer--) {
        const entity_list_t *bucket = &ui->hovered_by_layer->layers[layer];
        if (bucket->count > 0) {
            return bucket->items[0];
        }
    }
    return NULL;
}

static int on_tech_tree_screen(const button_t *b) {
    const score_panel_layout_block_t *block = score_panel_layout_block();
    return b->technology != NULL || (block != NULL && b->layer == block->back_color);
}

static const char *button_label(const button_t *b) {
    static const char *const galaxy_menu[GALAXY_MENU_COUNT] = {"button:galaxy_menu_1", "button:galaxy_menu_2",
                                                               "button:galaxy_menu_3", "button:galaxy_menu_4"};
    const p8_fixed_t pause_menu_layer = p8_fixed_from_int(pause_menu_tail.back_color);
    int i;
    if (b->layer < p8_fixed_from_int(10)) return "button:monde";
    if (on_tech_tree_screen(b)) return "button:technologies";
    for (i = 1; i <= GALAXY_MENU_COUNT; i++) {
        if (b->on_click != NULL && b->on_click == galaxy_menu_on_click(i)) {
            return galaxy_menu[i - 1];
        }
    }
    if (b->on_click != NULL && b->on_click == help_start_game_field_value()) return "button:difficulte";
    if (b->layer == pause_menu_layer) return "button:pause";
    if (b->layer > pause_menu_layer) return "button:fin_de_partie";
    return "button";
}

const char *cursor_label_read(const ui_t *ui) {
    const entity_t *e = first_to_receive(ui);
    const entity_class_t *c;
    if (e == NULL) {
        return "carte";
    }
    c = e->class;
    if (c == background_class()) return "carte";
    if (c == button_class()) return button_label((const button_t *)e);
    if (c == planet_class()) return "planet";
    if (c == structure_class()) return "structure";
    if (c == trading_hub_class()) return "trading_hub";
    if (c == processor_class()) return "processor";
    if (c == ascension_class()) return "ascension";
    if (c == protostar_class()) return "protostar";
    if (c == synthesizer_class()) return "synthesizer";
    if (c == burst_class()) return "burst";
    if (c == slipgate_class()) return "slipgate";
    if (c == infraspace_gate_class()) return "infraspace_gate";
    if (c == modal_backdrop_class()) return "modal_backdrop";
    if (c == help_screen_class()) return "help_screen";
    if (c == structure_icon_class()) return "structure_icon";
    assert(0 && "cursor_label_read: a class the hit test can find has no label; name it here and in the anchor table");
    return NULL;
}

int cursor_label_camera_clamped(vec_t top_left) {
    return top_left.x <= p8_fixed_from_int(-48) || top_left.y >= p8_fixed_from_int(316);
}

static int camera_now_clamped(void) {
    const entity_t *camera = main_camera();
    const vec_t *pos = camera != NULL && camera->pos != NULL ? *camera->pos : NULL;
    return pos != NULL && cursor_label_camera_clamped(*pos);
}

static int write_camera_clamp(int frame) {
    static FILE *out;
    const char *path = getenv("CAMERA_CLAMP");
    if (path == NULL) {
        return 1;
    }
    if (out == NULL && (out = fopen(path, "w")) == NULL) {
        fprintf(stderr, "replay: cannot open the camera clamp file '%s'\n", path);
        return 0;
    }
    if (camera_now_clamped() && fprintf(out, "%d\n", frame) < 0) {
        fprintf(stderr, "replay: cannot write to the camera clamp file '%s'\n", path);
        return 0;
    }
    return 1;
}

int cursor_label_update60(const char *path) {
    static FILE *out;
    static int frame;
    const ui_t *before = main_ui();
    const ui_t *after;
    if (out == NULL && (out = fopen(path, "w")) == NULL) {
        fprintf(stderr, "replay: cannot open the labels file '%s'\n", path);
        return 0;
    }
    cart_update60();
    after = main_ui();
    if (fprintf(out, "%s\n", cursor_label_read(after != NULL && after->hovered_by_layer != NULL ? after : before)) < 0) {
        fprintf(stderr, "replay: cannot write to the labels file '%s'\n", path);
        return 0;
    }
    return write_camera_clamp(frame++);
}
