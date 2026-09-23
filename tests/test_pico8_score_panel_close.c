/* Replays tests/fixtures/score_panel_close_events.json headless: the score
 * panel opened by `ix[3]` closes on the next click.
 *
 * The game starts the way src/main.c starts every replay of the net:
 * `eu()` on `bo[2]` before frame 0, no title screen.
 *
 *   1. open (press at frame 10). `ix[3]` at (50,3) sets `ju.c_` and puts
 *      the `fq` overlay up.
 *   2. close (press at frame 20). Any click lands on the `fq`: `fq:ldown`
 *      stores `de.my()` in `p_` (lua.lua:327), which must be `true`
 *      (lua.lua:901-903) for `dg` to remove the overlay; `c_` toggles back. */

#include <stdio.h>
#include <assert.h>

#include "../src/pico8_mem.h"
#include "../src/pico8_assets.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_class.h"
#include "../src/pico8_value.h"
#include "../src/pico8_list.h"
#include "../src/pico8_help.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_score_panel.h"

#define SCORE_PANEL_CLOSE_EVENTS_PATH "tests/fixtures/score_panel_close_events.json"

#define OPENED_FRAME 12
#define CLOSED_FRAME 22

static int ju_c_(void) {
    pico8_value_t v;
    pico8_instance_t *ju = pico8_ju();
    assert(ju != NULL);
    if (!pico8_instance_get_field(ju, "c_", &v)) {
        return 0;
    }
    return v.kind == PICO8_VALUE_BOOL && v.as.boolean;
}

static int fq_alive(void) {
    const pico8_list_t *hv = pico8_hv();
    pico8_class_t *fq_class = pico8_fq_class();
    int i;
    assert(fq_class != NULL);
    for (i = 0; i < hv->count; i++) {
        pico8_instance_t *e = (pico8_instance_t *)hv->items[i].as.ref;
        if (e != NULL && e->cls == fq_class) {
            return 1;
        }
    }
    return 0;
}

int main(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    int frame_count;
    int i;
    pico8_value_t difficulty_entry;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, SCORE_PANEL_CLOSE_EVENTS_PATH));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref));

    frame_count = input.frame_count;
    assert(frame_count > CLOSED_FRAME + 5);

    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8__update60();
        pico8__draw();

        if (i == OPENED_FRAME) {
            assert(ju_c_());
            assert(fq_alive());
            printf("[open] frame=%d c_ is true, the fq overlay is up\n", i);
        } else if (i == CLOSED_FRAME) {
            assert(!ju_c_());
            assert(!fq_alive());
            printf("[close] frame=%d c_ is false, the fq overlay is gone\n", i);
        }

        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    pico8_input_free(&input);

    printf("pico8 score panel close: a click closes the panel ix[3] opened\n");
    return 0;
}
