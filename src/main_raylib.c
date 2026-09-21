/* Phase 5, Etape 1, Chunk 4 (docs/prd/triage/phase5-etape1-raylib-
 * interactive-backend.md): the interactive PC driver. Duplicates
 * src/main.c's own asset-loading/dispatch-wiring/cart-boot sequence up
 * through pico8_cart_load()/pico8__init() (see that file's own header
 * comment for the rationale on why this is a second file rather than a
 * refactor of main.c), but sources its per-frame input from a live
 * raylib window via pico8_raylib_backend instead of a parsed
 * events.json, and presents the framebuffer by uploading it as a
 * texture instead of dumping text. No pico8_dump_screen call and no
 * sfx/music anywhere -- both out of scope per the PRD. src/main.c
 * itself is never modified by this file.
 *
 * UNLIKE main.c, this file deliberately does NOT call pico8_eu() itself
 * after pico8__init(). main.c's own difficulty_button/pico8_eu() dance
 * simulates "the player already clicked a difficulty button" purely so
 * its scripted events.json replay can start testing actual gameplay
 * immediately, without needing to script a fragile click on the exact
 * screen coordinates of a title-screen button -- a shortcut that only
 * makes sense for Backend 1's own scripted, non-interactive replay.
 * pico8__init() alone (pico8_cart.c) already calls pico8_fv_new(NULL),
 * which is the real cart's own title/difficulty-select screen (`fv`,
 * Phase 3 Chunk 17, pico8_help.c) -- exactly what a human playing this
 * backend should see first, matching the real PICO-8 cartridge. Each of
 * `bo`'s four difficulty buttons has its own `my` field wired to `eu`
 * (pico8_fv_pq, pico8_help.c) through the SAME generic js-button click
 * dispatch every other button in this port already uses, so a real
 * mouse click on one of them calls pico8_eu(that_button) naturally via
 * pico8__update60()'s own dg()/ok:lj() dispatch, with no help from this
 * driver. Verified via a disposable headless program (never executed
 * interactively): pico8__init() alone, followed by 30 frames of neutral-
 * input pico8__update60()/pico8__draw(), produces zero crashes/ASan
 * violations, pico8_cg("jo") stays unregistered (the galaxy is not
 * populated -- bw(90) only happens inside pico8_eu()'s own body,
 * pico8_cart.c), and the dumped framebuffer is ~33% non-black pixels
 * (a real screen is drawn, not a blank canvas) -- consistent with the
 * title screen, not an already-started game. */

#include "pico8_mem.h"
#include "pico8_assets.h"
#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_dispatch_context.h"
#include "pico8_cart.h"
#include "pico8_raylib_backend.h"
#include "pico8_replay_source.h"

#include <assert.h>
#include <stdio.h>

/* Chunk 1 of docs/prd/triage/raylib-replay-then-resume.md adds the one
 * optional positional argument: the recording this session replays
 * before handing control back to the player. Everything about it that
 * is decidable without a window lives in pico8_replay_source.{c,h} and
 * is unit-tested there; what is left here is the two things only this
 * driver can own -- the argv reading, and what a failed load does to the
 * process.
 *
 * The launch error is emitted and the process exits BEFORE
 * pico8_raylib_backend_init opens the window, deliberately. The PRD's
 * Implementation Decisions forbid falling back to a new game on a bad
 * path ("un repli silencieux... ferait croire a une perte de
 * sauvegarde"), and a window that opens on the title screen IS that
 * fallback as far as the player can tell, whatever was printed to a
 * terminal they may not be looking at. Failing before anything is
 * displayed makes the failure impossible to mistake for a lost save.
 *
 * With no argument, this file behaves exactly as it did before: same
 * boot sequence, same loop, same title screen. In particular the long
 * comment above still holds in full -- no simulated difficulty click is
 * added here, and none may be. It is load-bearing for this PRD and not
 * merely for the last one: a recording made by this backend starts at
 * the title screen and contains its own difficulty click, so a driver
 * that pre-clicked would desynchronise every recording it replays from
 * its very first frame (PRD, "fidelite du demarrage", user story 11). */

int main(int argc, char **argv) {
    /* One optional positional argument, nothing else: no flags, no
     * recording-picker UI (explicitly out of scope in the PRD). */
    const char *replay_path = (argc > 1) ? argv[1] : NULL;

    pico8_replay_source_t replay;
    if (pico8_replay_source_open(&replay, replay_path) == PICO8_REPLAY_SOURCE_LOAD_FAILED) {
        fprintf(stderr,
                "main_raylib: '%s' could not be loaded as a recording "
                "(missing, unreadable, or not a valid events.json). "
                "Refusing to start a new game instead.\n",
                replay_path);
        pico8_replay_source_close(&replay);
        return 1;
    }

    /* HONEST STATEMENT OF WHAT THIS CHUNK DOES NOT DO: when a recording
     * loaded, its frames are held and then freed below without ever
     * being played. The loop still takes every frame from the live
     * window, so `./main_raylib some_recording.json` currently starts an
     * ordinary new game after successfully loading the file. Feeding
     * those frames to the loop is Chunk 2 ("source d'entree
     * commutable"), which is where the feature's actual behavior lands;
     * this chunk is only the argument, the load, and the launch error.
     *
     * Until then, saying so on stderr is not optional politeness: a
     * successful load that silently starts a new game is, on screen,
     * the exact thing the PRD forbids the FAILED case from doing. The
     * line below is what keeps "loaded, not replayed yet" from looking
     * like "your recording was ignored". It goes away with Chunk 2. */
    if (pico8_replay_source_frame_count(&replay) > 0) {
        fprintf(stderr,
                "main_raylib: loaded '%s' (%d frames). Replay is not wired into "
                "the loop yet; starting a new game.\n",
                replay_path, pico8_replay_source_frame_count(&replay));
    }

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

    while (pico8_raylib_backend_should_continue(&backend)) {
        pico8_input_frame_t frame;
        pico8_raylib_backend_poll_input(&backend, &frame);
        /* Recorded BEFORE the game steps, so a frame whose
         * pico8__update60() crashes is already on disk -- see
         * pico8_raylib_backend.h's own top comment for the playtest
         * segfault that forced this ordering. */
        pico8_raylib_backend_record_frame(&backend, &frame);
        pico8_input_t input = { &frame, 1, 0 };
        pico8_dispatch_set_input(&input);

        pico8__update60();
        pico8__draw();

        pico8_raylib_backend_present(&backend, &mem, &frame);
    }

    pico8_raylib_backend_shutdown(&backend);
    pico8_replay_source_close(&replay);

    return 0;
}
