/* Replays tests/fixtures/score_panel_close_events.json headless up to the
 * frame before the closing click, with `fm.happiness` forced so that
 * `ky()[6]` is the star count given on the command line (1 or 2), then reads
 * the star row off the framebuffer.
 *
 * The cart's star row `o(score=6,ra=v(64,82))` (lua.lua:814) gets `st[3]`
 * before `st[1..5]` (lua.lua:884-889); `lq:pq` walks it through `all()`,
 * index 1..n, so the lit stars (sprite 56) must be the leftmost slots. */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../src/pico8_mem.h"
#include "../src/pico8_assets.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_cheat.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_value.h"
#include "../src/pico8_list.h"
#include "../src/pico8_help.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_economy_score.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_score_panel.h"

#define SCORE_PANEL_EVENTS_PATH "tests/fixtures/score_panel_close_events.json"

#define FORCE_FRAME 9
#define LOOK_FRAME 19

/* Slot k (0..4) of the star row: sprite at x=84+8k, y=82. Row y=85 reads
 * 7777aaa0 for sprite 56 and 22221210 for sprite 57, so its first pixel
 * tells them apart. */
#define STAR_X0 84
#define STAR_Y 85
#define LIT_COLOR 7
#define UNLIT_COLOR 2

static int happiness_for_stars(int stars) {
    switch (stars) {
    case 1:
        return 150;
    case 2:
        return 300;
    default:
        assert(0 && "only 1 and 2 stars are covered");
        return 0;
    }
}

static int ky_stars(void) {
    pico8_ob_table_t *s = pico8_ky();
    pico8_value_t v;
    int stars;
    assert(pico8_ob_table_get_index(s, 6, &v) && v.kind == PICO8_VALUE_FIXED);
    stars = pico8_fixed_flr_int(v.as.fixed);
    pico8_ob_table_free(s);
    return stars;
}

int main(int argc, char **argv) {
    static pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    pico8_cheat_t cheat;
    pico8_value_t difficulty_entry;
    int stars;
    int happiness;
    int i;
    int k;

    assert(argc == 2 && "usage: test_pico8_score_panel_stars <1|2>");
    stars = atoi(argv[1]);
    happiness = happiness_for_stars(stars);

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, SCORE_PANEL_EVENTS_PATH));
    assert(input.frame_count > LOOK_FRAME);

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref));

    pico8_cheat_init(&cheat);
    for (i = 0; i <= LOOK_FRAME; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        if (i == FORCE_FRAME) {
            /* With ten invented entries, happiness 150 and 300 measure as
             * ky()[6] = 1 and 2; the assert below keeps that honest. */
            pico8_list_t *invented = malloc(sizeof *invented);
            pico8_list_init(invented);
            for (k = 0; k < 10; k++) {
                pico8_list_add(invented, pico8_value_fixed(pico8_fixed_from_int(0)));
            }
            pico8_fm_hy()->invented = invented;
        }
        pico8_fm_hy()->happiness = pico8_fixed_from_int(happiness);
        pico8_fm_hy()->has_happiness = 1;
        if (i == FORCE_FRAME) {
            assert(ky_stars() == stars);
        }
        pico8__update60();
        pico8__draw();
        if (i < LOOK_FRAME) {
            pico8_input_step(&input);
        }
    }

    printf("[stars=%d] row:", stars);
    for (k = 0; k < 5; k++) {
        int c = pico8_peek(&mem, 0xA000 + STAR_Y * 128 + STAR_X0 + 8 * k) & 15;
        printf(" %c", c == LIT_COLOR ? 'x' : (c == UNLIT_COLOR ? 'o' : '?'));
    }
    printf("\n");
    fflush(stdout);
    for (k = 0; k < 5; k++) {
        int c = pico8_peek(&mem, 0xA000 + STAR_Y * 128 + STAR_X0 + 8 * k) & 15;
        assert(c == (k < stars ? LIT_COLOR : UNLIT_COLOR) && "lit stars must fill the leftmost slots");
    }

    pico8_input_free(&input);

    printf("pico8 score panel stars: %d lit star(s) drawn in the leftmost slot(s)\n", stars);
    return 0;
}
