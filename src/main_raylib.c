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
#include "pico8_replay_control.h"

#include <assert.h>
#include <stdio.h>

/* Chunks 1 and 2 of docs/prd/triage/raylib-replay-then-resume.md add the
 * one optional positional argument -- the recording this session replays
 * before handing control back to the player -- and then actually replay
 * it. Everything about it that is decidable without a window lives in
 * pico8_replay_source.{c,h} and is unit-tested there; what is left here
 * is the three things only this driver can own: the argv reading, what a
 * failed load does to the process, and the one place per frame where the
 * input comes from.
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
 * its very first frame (PRD, "fidelite du demarrage", user story 11).
 *
 * ONE LOOP, NOT TWO. Chunk 2's whole shape is dictated by the PRD's
 * most emphatic Implementation Decision: "Le rejeu et le jeu partagent
 * la meme boucle... La bascule est un changement de source d'entree, pas
 * un second mode avec sa propre boucle -- un pilote qui dupliquerait la
 * boucle divergerait de celle reellement jouee, ce qui est la faute que
 * ce PRD existe pour eviter." So the loop below is the same single loop
 * as before, with exactly one call site for pico8__update60(),
 * pico8__draw() and present(), and exactly one statement added: where
 * this frame's pico8_input_frame_t came from. A "replay loop" followed
 * by a "play loop" would have been shorter to write and would have been
 * a second, subtly different game loop that no human ever plays --
 * precisely the class of divergence that made the last segfault
 * irreproducible on the first attempt.
 *
 * NO FAST PATH OVER pico8__draw() DURING REPLAY, either -- and this is
 * not a missed optimisation but a measured property of this engine,
 * recorded in the PRD ("Sauter le dessin fait diverger l'etat : c'est
 * une propriete mesuree du moteur, verifiee pendant l'instruction du
 * dernier segfault, ou le chemin fautif disparaissait sans les effets de
 * bord du dessin", user story 15). Drawing has side effects on game
 * state here, so a replay that skipped it would not reach the state the
 * recording describes, which is the only thing the replay is for.
 *
 * CHUNK 4 MAKES THE REPLAY COMFORTABLE WITHOUT TOUCHING ANY OF THAT.
 * The loop below still plays and draws every single recorded frame; the
 * only thing that changes is how long it waits between them. That is
 * the PRD's own hard constraint on this chunk -- "abreger ne doit jamais
 * sauter de frame, seulement reduire l'attente entre frames, sinon
 * l'etat obtenu n'est plus celui de l'enregistrement" -- and it is why
 * the speed control is a frame-rate cap and literally nothing else.
 * Three things are added, each one statement in the loop: a frame-rate
 * cap taken from pico8_replay_control (4x game speed by default, Page
 * Up/Page Down to change it, End to remove it entirely, and back to 60
 * FPS the instant the recording runs out); the reading of those three
 * keys, through pico8_raylib_backend_poll_replay_controls rather than
 * poll_input, for reasons given at the call site; and a progress line
 * passed to present(), which exists only while the recording is
 * driving.
 *
 * WHY THE DEFAULT IS FASTER THAN THE GAME. Replaying a thirty-minute
 * recording at game speed takes thirty minutes, which is the whole
 * benefit gone -- user stories 7 and 10 exist to reach a late game state
 * without living through it again. 4x is the compromise that keeps user
 * story 2 ("reconnaitre le moment ou quelque chose a mal tourne")
 * possible while making the wait a fraction of what it was; the faster
 * levels are for the player who has already seen what they came for.
 * See src/pico8_replay_control.h for the ladder and its reasoning. */

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

    /* Chunk 1's transitional "replay is not wired into the loop yet"
     * stderr line is gone with this chunk, which is what it said would
     * happen: the frames loaded above are now played. What is kept is
     * telling the player how long the replay they are about to watch
     * is, since user story 6 wants them to know the game is not frozen
     * -- a real progress indicator is Chunk 4's. */
    if (pico8_replay_source_frame_count(&replay) > 0) {
        fprintf(stderr,
                "main_raylib: replaying '%s' (%d frames), then handing control back.\n"
                "main_raylib: during the replay -- PAGE UP faster, PAGE DOWN slower, END uncapped.\n",
                replay_path, pico8_replay_source_frame_count(&replay));
    }

    /* The replay's speed, and only its speed. Initialised before the
     * window opens so that the very first replayed frame already runs
     * at the default rather than spending one frame at game speed; it
     * is also read on every live frame afterwards, where
     * pico8_replay_control_target_fps ignores the level entirely and
     * returns 60 (see that header -- the reset the driver cannot
     * forget, because there is nothing here to remember). */
    pico8_replay_control_t replay_control;
    pico8_replay_control_init(&replay_control);

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
        /* THE SWITCH, and the only thing this chunk adds to the loop:
         * the frame comes from the recording while it has one, from the
         * hardware once it does not. pico8_replay_source_next_frame
         * reports exhaustion rather than asserting (see
         * pico8_replay_source.h), and a source opened with no path is
         * exhausted from its first call, so this single line is also the
         * no-argument case -- there is no replay flag and no mode here,
         * which is the point.
         *
         * WHY poll_input IS NOT CALLED WHILE THE FILE DRIVES. Two
         * reasons, and either alone would be enough. First, the replay
         * must be exactly the recorded session: the live mouse and
         * keyboard have no say in a frame that came from the file, and
         * the surest way to guarantee that is not to read them at all,
         * rather than to read them and then hope every future edit keeps
         * discarding them. Second, poll_input is not a pure read --
         * releasing the space bar makes it append the current frame
         * index to this session's marker file (see
         * pico8_raylib_backend.h). Markers are a playtester's bookmark
         * on a frame they are authoring; a tap during a replay would
         * plant one on a frame the player is only watching go by, and
         * marker coherence across a resume is explicitly Chunk 4's
         * subject, not something to prejudge here. Not calling
         * poll_input costs no window responsiveness: raylib pumps its
         * event queue in EndDrawing(), which present() calls every
         * frame, so closing the window or pressing the quit key still
         * ends the loop mid-replay through should_continue(). */
        int from_recording = pico8_replay_source_next_frame(&replay, &frame);
        if (!from_recording) {
            pico8_raylib_backend_poll_input(&backend, &frame);
        } else {
            /* The replay's own narrow input path, and the reason Chunk
             * 4 did not simply let poll_input run with its result
             * discarded. poll_input is not a pure read: releasing the
             * space bar makes it append a frame index to this session's
             * marker file, and a marker is a bookmark on a frame the
             * player is AUTHORING, not one they are watching go by. So
             * the replay reads the three keys it is entitled to and
             * nothing else -- this call is not even given the backend,
             * so it cannot reach the recording or the markers if a
             * later edit wanted it to. The frame for this iteration was
             * decided by the file one line above and is not consulted,
             * let alone modified, by anything here: live hardware has
             * no say whatsoever in a replayed frame. */
            pico8_raylib_backend_poll_replay_controls(&replay_control);
        }
        /* Re-asserted every frame rather than only when it changes.
         * The frame that matters most is the first live one, where the
         * cap must drop back to game speed; making that fall out of an
         * unconditional call means no code path can leave the player's
         * resumed game running at eight times speed. */
        pico8_raylib_backend_set_frame_rate_cap(pico8_replay_control_target_fps(&replay_control, from_recording));
        /* Recorded BEFORE the game steps, so a frame whose
         * pico8__update60() crashes is already on disk -- see
         * pico8_raylib_backend.h's own top comment for the playtest
         * segfault that forced this ordering. Unconditional on purpose:
         * a replayed frame is recorded exactly like a live one, which is
         * what makes the recording of a resumed session contain the
         * replayed frames followed by the new ones and therefore stay
         * replayable in its turn (PRD, "continuite de l'enregistrement",
         * user stories 8 and 9). Chunk 3 verifies that end to end; this
         * chunk's job is to leave the call where it is. */
        pico8_raylib_backend_record_frame(&backend, &frame);
        pico8_input_t input = { &frame, 1, 0 };
        pico8_dispatch_set_input(&input);

        pico8__update60();
        pico8__draw();

        /* User story 6: "savoir ou en est le rejeu pendant qu'il se
         * deroule, afin de ne pas croire que le jeu est fige". Built
         * only while the recording is driving, and passed as NULL
         * otherwise -- so the overlay does not survive the replay by
         * one frame, and live play has no HUD at all. The string is
         * composed by pico8_replay_control_format_status, which is
         * tested; present() only draws whatever it is handed, on top of
         * the already-blitted framebuffer, so neither the PICO-8 pixels
         * nor the recorded frames can be affected by it. */
        const char *replay_status = NULL;
        char replay_status_text[PICO8_REPLAY_CONTROL_STATUS_MAX];
        if (from_recording) {
            pico8_replay_control_format_status(&replay_control,
                                               pico8_replay_source_frames_served(&replay),
                                               pico8_replay_source_frame_count(&replay),
                                               replay_status_text, sizeof replay_status_text);
            replay_status = replay_status_text;
        }
        pico8_raylib_backend_present(&backend, &mem, &frame, replay_status);
    }

    pico8_raylib_backend_shutdown(&backend);
    pico8_replay_source_close(&replay);

    return 0;
}
