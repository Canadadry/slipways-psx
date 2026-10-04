#include "devtools/cursor_label.h"

#include <assert.h>
#include <stddef.h>

#include "game/background.h"
#include "game/help.h"
#include "game/menu.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
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

static const char *button_label(const button_t *b) {
    static const char *const galaxy_menu[GALAXY_MENU_COUNT] = {"button:galaxy_menu_1", "button:galaxy_menu_2",
                                                               "button:galaxy_menu_3", "button:galaxy_menu_4"};
    int i;
    for (i = 1; i <= GALAXY_MENU_COUNT; i++) {
        if (b->on_click != NULL && b->on_click == galaxy_menu_on_click(i)) {
            return galaxy_menu[i - 1];
        }
    }
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
