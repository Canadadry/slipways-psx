/* Replays tests/fixtures/game_end_events.json headless and checks that it
 * plays the game to its end by skipping years, which opens the score panel.
 *
 * The game starts the way src/main.c starts every replay of the net:
 * `eu()` on `bo[2]` before frame 0, no title screen.
 *
 * The fixture's stages and what grounds each check:
 *
 *   1. start (before frame 0). `eu()` creates `fm` with `yr=3401`
 *      (lua.lua:1202).
 *   2. 25 skipped years (presses at frames 14, 16, ... 62). Each click on
 *      `ix[1]` runs `hy:hk()`, `yr+=1` (lua.lua:1213); at 3426 `hl` becomes
 *      "final score" (lua.lua:1235), and `pj:ph` sets `c_` and raises an
 *      `fq()` overlay without `my` (lua.lua:870-880). With `c_` set, `pj:ph`
 *      draws the score rows through `s[st.score]` (lua.lua:882-889).
 *   3. three more clicks on `ix[1]` land on that overlay (fa=14, above the
 *      button's fa=11, lua.lua:324-326 and 258), so `yr` stays at 3426. */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../src/pico8_mem.h"
#include "../src/pico8_assets.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_cheat.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_help.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_class.h"
#include "../src/pico8_value.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_score_panel.h"

#define GAME_END_EVENTS_PATH "tests/fixtures/game_end_events.json"

#define LAST_PLAYING_FRAME 61
#define LAST_YEAR 3426

static int ju_c_(void) {
    pico8_value_t v;
    const void *ju = pico8_ju();
    assert(ju != NULL);
    return pico8_entity_get_field(ju, "c_", &v) && v.kind == PICO8_VALUE_BOOL && v.as.boolean;
}

static int fm_yr_is(int year) {
    pico8_value_t v;
    pico8_instance_t *fm = pico8_fm();
    assert(fm != NULL && pico8_entity_get_field(fm, "yr", &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed == pico8_fixed_from_int(year);
}

static const char *fm_hl(void) {
    pico8_value_t v;
    pico8_instance_t *fm = pico8_fm();
    assert(fm != NULL);
    if (!pico8_entity_get_field(fm, "hl", &v) || v.kind != PICO8_VALUE_STRING) {
        return NULL;
    }
    return v.as.string;
}

int main(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    int frame_count;
    pico8_value_t difficulty_entry;
    int i;
    int end_frame = -1;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, GAME_END_EVENTS_PATH));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref));

    frame_count = input.frame_count;
    assert(frame_count > LAST_PLAYING_FRAME);

    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();

        if (i == LAST_PLAYING_FRAME) {
            assert(fm_hl() == NULL);
            assert(!ju_c_());
            assert(fm_yr_is(LAST_YEAR - 1));
            printf("[years] frame=%d yr=%d, game still on, score panel closed\n", i, LAST_YEAR - 1);
        } else if (i > LAST_PLAYING_FRAME && end_frame < 0 && fm_hl() != NULL) {
            end_frame = i;
            assert(fm_yr_is(LAST_YEAR));
            assert(strcmp(fm_hl(), "final score") == 0);
        }

        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    assert(end_frame >= 0);
    assert(ju_c_());
    assert(strcmp(fm_hl(), "final score") == 0);
    assert(fm_yr_is(LAST_YEAR));
    printf("[game end] frame=%d hl=\"final score\", c_ true, yr stays %d after the extra ix[1] clicks\n", end_frame,
           LAST_YEAR);

    pico8_input_free(&input);

    printf("pico8 game end: 25 ix[1] clicks end the game and open the score panel\n");
    return 0;
}
