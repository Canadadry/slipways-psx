/* Rejoue argv[1] comme src/cmd/replay.c, depuis l'écran titre, sans capture ni
 * dump, puis imprime une ligne `<classe> <instanciations>` par nom de
 * KNOWN[]. Rend 2 si le fichier ne se charge pas : tools/class_coverage.sh
 * le signale alors « illisible ». Rend 3, le nom fautif sur stderr, si un
 * nom compté manque à KNOWN[]. */

#include "p8/mem.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/cheat.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "devtools/class_coverage.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *const KNOWN[] = {
    "background", "modal_backdrop", "infraspace_gate", "help_screen", "probe", "burst", "ascension", "research_project", "economy", "structure_icon", "toast", "tooltip", "slipway", "planet", "button",
    "entity", "layout_row", "message_box", "synthesizer", "wormhole", "mouse", "trading_hub", "slipgate", "processor", "score_panel", "protostar", "camera", "structure", "trade", "ui",
};

#define KNOWN_COUNT ((int)(sizeof KNOWN / sizeof KNOWN[0]))

int main(int argc, char **argv) {
    assert(argc == 2 && "usage: class_coverage <events.json>");

    p8_memory_t mem;
    p8_mem_init(&mem);
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));

    p8_input_t input;
    p8_input_init(&input);
    if (!p8_input_load(&input, argv[1])) {
        fprintf(stderr, "class_coverage: %s does not load\n", argv[1]);
        return 2;
    }

    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);

    cart_load();
    cart_init();

    cheat_t cheat;
    cheat_init(&cheat);
    for (int i = 0; i < input.frame_count; i++) {
        dispatch_context_set_input(&input);
        cheat_step(&cheat, &input);
        cart_update60();
        cart_draw();
        if (i < input.frame_count - 1) {
            p8_input_step(&input);
        }
    }

    for (int k = 0; k < KNOWN_COUNT; k++) {
        for (int j = 0; j < k; j++) {
            assert(strcmp(KNOWN[j], KNOWN[k]) != 0 && "two KNOWN[] entries share a name: its line would be doubled");
        }
    }
    const char *unknown = class_coverage_first_unknown(KNOWN, KNOWN_COUNT);
    if (unknown != NULL) {
        if (unknown[0] == '\0') {
            fprintf(stderr, "class_coverage: an instantiated class has no coverage name (name \"\")\n");
        } else {
            fprintf(stderr, "class_coverage: the name %s is not in KNOWN[]\n", unknown);
        }
        p8_input_free(&input);
        return 3;
    }
    for (int k = 0; k < KNOWN_COUNT; k++) {
        printf("%s %ld\n", KNOWN[k], class_coverage_count(KNOWN[k]));
    }

    p8_input_free(&input);
    return 0;
}
