/* Replays tests/fixtures/cheat_code_events.json headless, the way src/main.c
 * does, with pico8_cheat_step() before every pico8__update60():
 *
 *   1. A A B B A A B B (A = O, B = X), the first A held three frames and a
 *      left-arrow press in the middle: fires on the last B, frame 23, and
 *      leaves fm.jk = 99, fm.nz = 200.
 *   2. A A B A: never fires.
 *   3. the full sequence again: fires on frame 67. fm.jk/fm.nz are zeroed
 *      before segment 2 so that this firing is seen writing them. */

#include <stdio.h>
#include <assert.h>

#include "../src/pico8_mem.h"
#include "../src/pico8_assets.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_class.h"
#include "../src/pico8_value.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_help.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_cheat.h"

#define CHEAT_CODE_EVENTS_PATH "tests/fixtures/cheat_code_events.json"

#define FIRST_FIRING_FRAME 23
#define RESET_FRAME 30
#define SECOND_FIRING_FRAME 67

static pico8_fixed_t fm_fixed(const char *name) {
    pico8_value_t v;
    assert(pico8_fm() != NULL);
    assert(pico8_instance_get_field(pico8_fm(), name, &v));
    assert(v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static void assert_cheat_applied(void) {
    assert(fm_fixed("jk") == pico8_fixed_from_int(99));
    assert(fm_fixed("nz") == pico8_fixed_from_int(200));
}

int main(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    pico8_cheat_t cheat;
    pico8_value_t difficulty_entry;
    int firings = 0;
    int i;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, CHEAT_CODE_EVENTS_PATH));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref));

    assert(fm_fixed("jk") != pico8_fixed_from_int(99));
    assert(fm_fixed("nz") != pico8_fixed_from_int(200));

    pico8_cheat_init(&cheat);
    for (i = 0; i < input.frame_count; i++) {
        pico8_dispatch_set_input(&input);

        if (i == RESET_FRAME) {
            pico8_ob_table_set_name(pico8_fm()->fields, "jk", pico8_value_fixed(pico8_fixed_from_int(0)));
            pico8_ob_table_set_name(pico8_fm()->fields, "nz", pico8_value_fixed(pico8_fixed_from_int(0)));
        }

        int fired = pico8_cheat_step(&cheat, &input);
        assert(fired == (i == FIRST_FIRING_FRAME || i == SECOND_FIRING_FRAME));
        if (fired) {
            assert_cheat_applied();
            firings++;
            printf("[cheat] frame=%d fm.jk = 99, fm.nz = 200\n", i);
        }
        if (i == SECOND_FIRING_FRAME - 1) {
            assert(fm_fixed("jk") != pico8_fixed_from_int(99));
            assert(fm_fixed("nz") != pico8_fixed_from_int(200));
        }

        pico8__update60();
        pico8__draw();

        if (i < input.frame_count - 1) {
            pico8_input_step(&input);
        }
    }
    assert(firings == 2);

    pico8_input_free(&input);

    printf("pico8 cheat code: A A B B A A B B fires twice, A A B A never\n");
    return 0;
}
