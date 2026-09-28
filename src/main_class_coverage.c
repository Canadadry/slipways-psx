/* Rejoue argv[1] comme src/main.c, depuis l'écran titre, sans capture ni
 * dump, puis imprime une ligne `<classe> <instanciations>` par nom de
 * KNOWN[]. Rend 2 si le fichier ne se charge pas : tools/class_coverage.sh
 * le signale alors « illisible ». Rend 3, le nom fautif sur stderr, si un
 * nom compté manque à KNOWN[]. */

#include "pico8_mem.h"
#include "pico8_assets.h"
#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_cheat.h"
#include "pico8_dispatch_context.h"
#include "pico8_cart.h"
#include "pico8_coverage.h"
#include "pico8_help.h"
#include "pico8_ui_button.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *const KNOWN[] = {
    "bg", "fq", "fr", "fv", "g_", "ga", "hj", "hp", "hy", "ie", "iv", "jb", "jc", "jo", "js",
    "ke", "lq", "mh", "mx", "mz", "nf", "nl", "nr", "os", "pj", "pm", "qb", "qh", "trade", "ui",
};

#define KNOWN_COUNT ((int)(sizeof KNOWN / sizeof KNOWN[0]))

int main(int argc, char **argv) {
    assert(argc == 2 && "usage: pico8_class_coverage <events.json>");

    pico8_mem_t mem;
    pico8_mem_init(&mem);
    pico8_gfx_t gfx;
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_t input;
    pico8_input_init(&input);
    if (!pico8_input_load(&input, argv[1])) {
        fprintf(stderr, "pico8_class_coverage: %s ne se charge pas\n", argv[1]);
        return 2;
    }

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (int i = 0; i < input.frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();
        if (i < input.frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    for (int k = 0; k < KNOWN_COUNT; k++) {
        for (int j = 0; j < k; j++) {
            assert(strcmp(KNOWN[j], KNOWN[k]) != 0 && "two KNOWN[] entries share a name: its line would be doubled");
        }
    }
    const char *unknown = pico8_class_coverage_first_unknown(KNOWN, KNOWN_COUNT);
    if (unknown != NULL) {
        if (unknown[0] == '\0') {
            fprintf(stderr, "pico8_class_coverage: une classe instanciée n'a pas de nom de couverture (nom \"\")\n");
        } else {
            fprintf(stderr, "pico8_class_coverage: le nom %s n'est pas dans KNOWN[]\n", unknown);
        }
        pico8_input_free(&input);
        return 3;
    }
    for (int k = 0; k < KNOWN_COUNT; k++) {
        printf("%s %ld\n", KNOWN[k], pico8_class_coverage_count(KNOWN[k]));
    }

    pico8_input_free(&input);
    return 0;
}
