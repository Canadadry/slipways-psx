/* Replays tests/fixtures/pause_menu_music_events.json headless and checks
 * that it opens the pause menu and clicks its "toggle music" row.
 *
 * The fixture's stages and what grounds each check:
 *
 *   1. difficulty (frame 10). A click on the title menu's second row
 *      ("reasonable") starts the game.
 *   2. pause menu (press at frame 14 on `ix[4]`, ra=v(0,117)). Its `my`
 *      installs the four `ha[i].my` callbacks, then `pz(ha,ha)` lays the
 *      rows out upward from hi=v(0,107), gq=v(0,-11) (lua.lua:986-997).
 *      Before that press `ha[2]` has no `my`.
 *   3. "toggle music" (press at frame 20 at (10,99), inside the second row).
 *      The pressed target is `ok.o_`, the button `ui:lj` found under the
 *      mouse; its `my` is `ha[2].my`, the `music(-sgn(stat(20)))` closure
 *      (lua.lua:993). `music()` is inert in the port (TROU-10), so nothing
 *      after the click is observable: reaching the call is the whole point. */

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
#include "../src/pico8_menu.h"
#include "../src/pico8_ui.h"

#define PAUSE_MENU_MUSIC_EVENTS_PATH "tests/fixtures/pause_menu_music_events.json"

#define IX4_PRESS_FRAME 14
#define MUSIC_ROW_PRESS_FRAME 20
#define MUSIC_ROW 2

/* Zeroed kind-FIXED value when `ha[2]` has no `my` yet. */
static pico8_value_t ha_row_my(int row) {
    pico8_value_t row_val;
    pico8_value_t my_val = pico8_value_fixed(0);
    const pico8_ob_table_t *ha = pico8_ha();
    assert(ha != NULL);
    assert(pico8_ob_table_get_index(ha, row, &row_val) && row_val.kind == PICO8_VALUE_REF && row_val.as.ref != NULL);
    pico8_ob_table_get_name((const pico8_ob_table_t *)row_val.as.ref, "my", &my_val);
    return my_val;
}

static pico8_instance_t *ok_hovered(void) {
    pico8_value_t v;
    pico8_instance_t *ok = pico8_ok();
    assert(ok != NULL);
    if (!pico8_instance_get_field(ok, "o_", &v) || v.kind != PICO8_VALUE_REF) {
        return NULL;
    }
    return (pico8_instance_t *)v.as.ref;
}

int main(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    int frame_count;
    int i;
    int music_row_pressed = 0;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, PAUSE_MENU_MUSIC_EVENTS_PATH));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    frame_count = input.frame_count;
    assert(frame_count > MUSIC_ROW_PRESS_FRAME);

    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8__update60();
        pico8__draw();

        if (i == IX4_PRESS_FRAME - 1) {
            assert(ha_row_my(MUSIC_ROW).kind != PICO8_VALUE_FUNCTION);
        } else if (i == MUSIC_ROW_PRESS_FRAME) {
            pico8_value_t music_my = ha_row_my(MUSIC_ROW);
            pico8_instance_t *hovered = ok_hovered();
            pico8_value_t hovered_my;
            assert(music_my.kind == PICO8_VALUE_FUNCTION);
            assert(hovered != NULL);
            assert(pico8_instance_get_field(hovered, "my", &hovered_my) && hovered_my.kind == PICO8_VALUE_FUNCTION);
            assert(hovered_my.as.function == music_my.as.function);
            music_row_pressed = 1;
            printf("[pause menu] frame=%d the pressed button is ha[2], \"toggle music\"\n", i);
        }

        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    assert(music_row_pressed);
    pico8_input_free(&input);

    printf("pico8 pause menu: ix[4] opens the menu and its \"toggle music\" row is clicked\n");
    return 0;
}
