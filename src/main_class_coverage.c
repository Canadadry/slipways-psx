/* Rejoue argv[1] comme src/main.c, sans capture ni dump, puis imprime une
 * ligne `<classe> <instanciations>` par classe connue et une ligne
 * `inconnue <n>`. Rend 2 si le fichier ne se charge pas : tools/class_coverage.sh
 * le signale alors « illisible ». */

#include "pico8_mem.h"
#include "pico8_assets.h"
#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_cheat.h"
#include "pico8_dispatch_context.h"
#include "pico8_cart.h"
#include "pico8_class.h"
#include "pico8_bg.h"
#include "pico8_economy.h"
#include "pico8_help.h"
#include "pico8_ke.h"
#include "pico8_layout.h"
#include "pico8_mouse_reader.h"
#include "pico8_msgbox.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_planet_family_trade.h"
#include "pico8_probe.h"
#include "pico8_research.h"
#include "pico8_research_icon.h"
#include "pico8_score_panel.h"
#include "pico8_scroll.h"
#include "pico8_slipway.h"
#include "pico8_trade.h"
#include "pico8_ui.h"
#include "pico8_ui_button.h"
#include "pico8_wormhole.h"

#include <assert.h>
#include <stdio.h>

typedef struct {
    const char *name;
    pico8_class_t *(*accessor)(void);
} known_class_t;

static const known_class_t KNOWN[] = {
    {"bg", pico8_bg_class},   {"fq", pico8_fq_class},       {"fr", pico8_fr_class},
    {"fv", pico8_fv_class},   {"g_", pico8_g__class},       {"ga", pico8_ga_class},
    {"hj", pico8_hj_class},   {"hp", pico8_hp_class},       {"hy", pico8_hy_class},
    {"ie", pico8_ie_class},   {"iv", pico8_iv_class},       {"jb", pico8_jb_class},
    {"jc", pico8_jc_class},   {"jo", pico8_jo_class},       {"js", pico8_js_class},
    {"ke", pico8_ke_class},   {"lq", pico8_lq_class},       {"mh", pico8_mh_class},
    {"mx", pico8_mx_class},   {"mz", pico8_mz_class},       {"nf", pico8_nf_class},
    {"nl", pico8_nl_class},   {"nr", pico8_nr_class},       {"os", pico8_os_class},
    {"pj", pico8_pj_class},   {"pm", pico8_pm_class},       {"qb", pico8_qb_class},
    {"qh", pico8_qh_class},   {"trade", pico8_trade_class}, {"ui", pico8_ui_class},
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

    pico8_value_t difficulty_entry;
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref));

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

    long known_total = 0;
    for (int k = 0; k < KNOWN_COUNT; k++) {
        const pico8_class_t *cls = KNOWN[k].accessor();
        assert(cls != NULL && "a class accessor returned NULL once the cart is loaded");
        for (int j = 0; j < k; j++) {
            assert(KNOWN[j].accessor() != cls && "two accessors share a class: its count would be doubled");
        }
        long count = pico8_class_coverage_count(cls);
        known_total += count;
        printf("%s %ld\n", KNOWN[k].name, count);
    }
    printf("inconnue %ld\n", pico8_class_coverage_total() - known_total);

    pico8_input_free(&input);
    return 0;
}
