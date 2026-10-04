/* make cursor-labels-test: the cursor label of interfaces built by hand,
 * entities of real classes on chosen layers. Temporary (plan 12). */

#include <assert.h>
#include <string.h>

#include "devtools/cursor_label.h"
#include "game/background.h"
#include "game/help.h"
#include "game/menu.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_family_trade.h"
#include "game/structure_icon.h"
#include "game/ui.h"
#include "game/ui_button.h"

enum { MAX_PER_LAYER = 4 };

typedef struct {
    ui_t ui;
    entity_t *items[UI_HOVER_LAYERS][MAX_PER_LAYER];
} hand_ui_t;

static void hand_ui_init(hand_ui_t *h) {
    int layer;
    memset(h, 0, sizeof *h);
    h->ui.hovered_by_layer_own.layer_count = UI_HOVER_LAYERS;
    for (layer = 0; layer < UI_HOVER_LAYERS; layer++) {
        h->ui.hovered_by_layer_own.layers[layer].items = h->items[layer];
        h->ui.hovered_by_layer_own.layers[layer].capacity = MAX_PER_LAYER;
    }
    h->ui.hovered_by_layer = &h->ui.hovered_by_layer_own;
}

static void hand_ui_put(hand_ui_t *h, int layer, entity_t *e) {
    entity_list_t *list = &h->ui.hovered_by_layer_own.layers[layer];
    assert(list->count < MAX_PER_LAYER);
    list->items[list->count++] = e;
}

static entity_t of_class(const entity_class_t *c) {
    entity_t e;
    entity_init_header(&e, c);
    return e;
}

static button_t button_with(button_fn_t on_click) {
    button_t b;
    memset(&b, 0, sizeof b);
    entity_init_header(&b.header, button_class());
    b.on_click = on_click;
    b.has_on_click = on_click != NULL;
    return b;
}

static void expect(const ui_t *ui, const char *label) {
    const char *got = cursor_label_read(ui);
    assert(got != NULL && strcmp(got, label) == 0);
}

typedef struct {
    const entity_class_t *(*class_of)(void);
    const char *label;
} named_class_t;

static const named_class_t NAMED[] = {
    {planet_class, "planet"},       {structure_class, "structure"},
    {trading_hub_class, "trading_hub"}, {processor_class, "processor"},
    {ascension_class, "ascension"}, {protostar_class, "protostar"},
    {synthesizer_class, "synthesizer"}, {burst_class, "burst"},
    {slipgate_class, "slipgate"},   {infraspace_gate_class, "infraspace_gate"},
    {modal_backdrop_class, "modal_backdrop"}, {help_screen_class, "help_screen"},
    {structure_icon_class, "structure_icon"}, {background_class, "carte"},
};

int main(void) {
    hand_ui_t h;
    entity_t background = of_class(background_class());
    entity_t planet = of_class(planet_class());

    expect(NULL, "carte");

    hand_ui_init(&h);
    h.ui.hovered_by_layer = NULL;
    expect(&h.ui, "carte");

    hand_ui_init(&h);
    expect(&h.ui, "carte");

    hand_ui_put(&h, 0, &background);
    expect(&h.ui, "carte");

    hand_ui_put(&h, 3, &planet);
    expect(&h.ui, "planet");

    button_t plain = button_with(NULL);
    hand_ui_put(&h, 5, &plain.header);
    expect(&h.ui, "button");

    hand_ui_init(&h);
    hand_ui_put(&h, 7, &planet);
    hand_ui_put(&h, 7, &plain.header);
    expect(&h.ui, "planet");

    {
        static const char *const galaxy[] = {"button:galaxy_menu_1", "button:galaxy_menu_2", "button:galaxy_menu_3",
                                             "button:galaxy_menu_4"};
        int i;
        for (i = 1; i <= 4; i++) {
            button_t menu = button_with(galaxy_menu_on_click(i));
            hand_ui_init(&h);
            hand_ui_put(&h, 2, &planet);
            hand_ui_put(&h, 9, &menu.header);
            expect(&h.ui, galaxy[i - 1]);
        }
    }

    {
        size_t i;
        for (i = 0; i < sizeof NAMED / sizeof NAMED[0]; i++) {
            entity_t e = of_class(NAMED[i].class_of());
            hand_ui_init(&h);
            hand_ui_put(&h, 1, &background);
            hand_ui_put(&h, 4, &e);
            expect(&h.ui, NAMED[i].label);
        }
    }

    {
        hand_ui_t before;
        button_t menu = button_with(galaxy_menu_on_click(2));
        hand_ui_init(&h);
        hand_ui_put(&h, 0, &background);
        hand_ui_put(&h, 6, &planet);
        hand_ui_put(&h, 6, &menu.header);
        button_t menu_before;
        entity_t planet_before;
        memcpy(&before, &h, sizeof h);
        memcpy(&menu_before, &menu, sizeof menu);
        memcpy(&planet_before, &planet, sizeof planet);
        expect(&h.ui, "planet");
        assert(memcmp(&before, &h, sizeof h) == 0);
        assert(memcmp(&menu_before, &menu, sizeof menu) == 0);
        assert(memcmp(&planet_before, &planet, sizeof planet) == 0);
    }
    return 0;
}
