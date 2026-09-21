/* Phase 5, Etape 1, Chunk 4 (docs/prd/triage/phase5-etape1-raylib-
 * interactive-backend.md): the interactive PC driver. Duplicates
 * src/main.c's own asset-loading/dispatch-wiring/cart-boot sequence
 * verbatim (see that file's own header comment for the rationale on why
 * it is a second file rather than a refactor of main.c) but sources its
 * per-frame input from a live raylib window via pico8_raylib_backend
 * instead of a parsed events.json, and presents the framebuffer by
 * uploading it as a texture instead of dumping text. No
 * pico8_dump_screen call and no sfx/music anywhere -- both out of scope
 * per the PRD. src/main.c itself is never modified by this file. */

#include "pico8_mem.h"
#include "pico8_assets.h"
#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_dispatch_context.h"
#include "pico8_cart.h"
#include "pico8_help.h"
#include "pico8_ui_button.h"
#include "pico8_raylib_backend.h"

#include <assert.h>

int main(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    pico8_gfx_t gfx;
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);

    pico8_raylib_backend_t backend;
    pico8_raylib_backend_init(&backend);

    pico8_cart_load();
    pico8__init();

    pico8_value_t difficulty_entry;
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    pico8_instance_t *difficulty_button =
        pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref);
    pico8_eu(difficulty_button);

    while (pico8_raylib_backend_should_continue(&backend)) {
        pico8_input_frame_t frame;
        pico8_raylib_backend_poll_input(&backend, &frame);
        pico8_input_t input = { &frame, 1, 0 };
        pico8_dispatch_set_input(&input);

        pico8__update60();
        pico8__draw();

        pico8_raylib_backend_present(&backend, &mem, &frame);
    }

    pico8_raylib_backend_shutdown(&backend);

    return 0;
}
