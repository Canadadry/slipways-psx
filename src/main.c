/* Phase 4, Chunk 1 (docs/prd/triage/phase4-etape1-headless-replay-wiring-
 * and-playability-verification.md): the project's one real headless game
 * driver -- loads cart assets, wires the dispatch context, boots the real
 * cart (pico8_cart_load/pico8__init/pico8_eu), steps it frame-by-frame
 * from a scripted events.json, and dumps the final framebuffer as text.
 * No game-specific assertions live here (that's tests/test_pico8_
 * playability.c's job, Chunk 2/3) -- this is a thin, general-purpose
 * shell, reusable as-is by Phase 5's raylib backend. Phase 1's own demo
 * assertions, formerly this file's entire content, now live verbatim in
 * tests/test_pico8_demo.c. */

#include "pico8_mem.h"
#include "pico8_assets.h"
#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_dump.h"
#include "pico8_dispatch_context.h"
#include "pico8_cart.h"
#include "pico8_help.h"
#include "pico8_ui_button.h"

#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv) {
    const char *events_path = (argc > 1) ? argv[1] : "events.json";

    pico8_mem_t mem;
    pico8_mem_init(&mem);

    pico8_gfx_t gfx;
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, events_path));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    pico8_value_t difficulty_entry;
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    pico8_instance_t *difficulty_button =
        pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref);
    pico8_eu(difficulty_button);

    int frame_count = input.frame_count;
    for (int i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8__update60();
        pico8__draw();
        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    FILE *dump_out = fopen("tmp/pico8_replay_dump.txt", "w");
    assert(dump_out != NULL);
    int dump_ok = pico8_dump_screen(&mem, dump_out);
    fclose(dump_out);
    assert(dump_ok);

    pico8_input_free(&input);

    printf("pico8 replay: %d frame(s) run against %s; framebuffer dumped to tmp/pico8_replay_dump.txt\n",
           frame_count, events_path);
    return 0;
}
