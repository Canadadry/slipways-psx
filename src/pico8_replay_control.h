/* Chunk 4 of docs/prd/triage/raylib-replay-then-resume.md ("Confort de
 * rejeu : reglage de vitesse, abregement, indication de progression,
 * coherence du marqueur de frame apres reprise"): how fast a replay
 * runs, and what it tells the player while it does.
 *
 * Fourth application of the same precedent as pico8_raylib_backend_
 * coords.{c,h}, pico8_replay_source.{c,h} and pico8_session_recorder.
 * {c,h}, for the same reason and no other: src/pico8_raylib_backend.c
 * includes raylib.h, the interactive driver may not be launched by an
 * agent at this phase (CLAUDE.md), and raylib is not vendored in every
 * environment, so anything living in that file is untestable here by
 * construction. The PRD's Testing Decisions say what to do about that
 * in so many words -- "la logique testable doit donc etre placee hors du
 * module raylib" -- so the two decisions this chunk actually makes, the
 * speed level -> frame-rate cap mapping and the text of the progress
 * line, live out here where tests/test_pico8_replay_control.c pins them
 * to exact values. Nothing in this file includes raylib.h, and nothing
 * in it ever will.
 *
 * WHY A FRAME-RATE CAP IS THE ONLY MECHANISM ON OFFER. The PRD's hard
 * constraint on this chunk is that "abreger ne doit jamais sauter de
 * frame, seulement reduire l'attente entre frames, sinon l'etat obtenu
 * n'est plus celui de l'enregistrement". Every recorded frame is still
 * fed to pico8__update60() and still drawn -- drawing has measured side
 * effects on game state in this engine (user story 15), so a replay that
 * skipped either would not arrive at the state the recording describes,
 * which is the only thing the replay is for. This module is therefore
 * deliberately incapable of expressing a skip: it never sees a frame,
 * never sees the recording, and its entire output is a number for
 * raylib's SetTargetFPS. A speed control that cannot represent the
 * forbidden operation cannot drift into performing it.
 *
 * WHY THE DEFAULT IS 4x AND NOT 1x. The arbitration for this chunk was
 * explicit: a thirty-minute recording replayed at game speed takes
 * thirty minutes, which destroys the main benefit of the feature (user
 * stories 7 and 10 -- reaching a deep game state without replaying the
 * afternoon that produced it). 4x brings that half-hour back in under
 * eight minutes while still running the game at a rate a human can
 * watch and recognise something going wrong in (user story 2), which
 * 8x does not. The levels above it exist for the player who has
 * recognised nothing yet and wants the rest over with.
 *
 * WHY THE LEVELS ARE MULTIPLES OF 60 AND NOT ARBITRARY FPS NUMBERS. The
 * game's own target is 60 FPS, so a level expressed as a multiple of it
 * is a statement about the game -- "two recorded frames per real
 * sixtieth of a second" -- rather than about the display, and it stays
 * true if the window ever runs on a 144 Hz monitor. It also makes the
 * progress line able to say "x4" instead of "240 fps", which is the
 * number the player actually cares about. Note that a cap is a ceiling,
 * not a promise: on a machine that cannot step and draw this cart 480
 * times a second, level 8x simply runs as fast as the machine manages.
 * That is why the uncapped level is a real level and not a synonym for
 * the top of the ladder -- it is the one that stops asking raylib to
 * wait at all.
 *
 * WHY replay_active IS AN ARGUMENT TO target_fps RATHER THAN THE
 * CALLER'S BUSINESS. The moment the recording is exhausted the player
 * is playing, and a game still running at 8x -- or uncapped -- is not a
 * fast game, it is a broken one, and it would look like a bug in the
 * port rather than a leftover setting. Making that a property of this
 * function rather than a reset the driver has to remember means the
 * driver cannot forget it: there is no code path through
 * pico8_replay_control_target_fps that returns anything but
 * PICO8_REPLAY_CONTROL_LIVE_FPS once the replay is over, whatever the
 * level is, and tests/test_pico8_replay_control.c asserts exactly that.
 * The level itself is deliberately NOT reset at that point, so that a
 * caller which replays again in the same process (a test) sees the same
 * mapping it set up.
 *
 * REJECTED: having this module own the keys. IsKeyPressed lives in
 * raylib, so a module that mapped keys to speed changes would include
 * raylib.h and would be untestable -- the whole point of the file lost
 * for the sake of three if statements. The keys are read in
 * pico8_raylib_backend_poll_replay_controls(), which does nothing but
 * turn three key presses into three calls below.
 *
 * REJECTED: expressing the speed as a float multiplier or as a delay in
 * milliseconds. Both make "uncapped" an awkward special value (0.0? a
 * negative delay?) and both invite arithmetic that can land between two
 * levels. An integer level indexing a fixed ladder cannot. */

#ifndef PICO8_REPLAY_CONTROL_H
#define PICO8_REPLAY_CONTROL_H

#include <stddef.h>

/* The game's own frame rate, and the one the player gets back the
 * instant the recording runs out. */
#define PICO8_REPLAY_CONTROL_LIVE_FPS 60

/* Big enough for the longest line pico8_replay_control_format_status
 * can produce from any plausible recording; the function truncates
 * rather than overflowing whatever it is given, so this is a
 * convenience for callers, not a contract they can violate. */
#define PICO8_REPLAY_CONTROL_STATUS_MAX 64

/* The replay's speed setting, and nothing else: one index into a fixed
 * ladder of levels. Not a mode, not a state machine -- whether a replay
 * is running at all is the driver's question, answered per frame by
 * pico8_replay_source_next_frame, and passed in below rather than
 * mirrored here, because a copy of that fact kept in this struct could
 * disagree with the source that owns it. Read through the accessors;
 * `level` is not meant to be written directly. */
typedef struct {
    int level;
} pico8_replay_control_t;

/* Starts at the default speed: faster than the game, slow enough to
 * watch. See this header's top comment for why that is 4x and not 1x. */
void pico8_replay_control_init(pico8_replay_control_t *control);

/* One step up / one step down the ladder, clamped at both ends rather
 * than wrapping: one extra tap on "faster" must never drop a replay
 * back to the slowest setting, which is the worst thing a speed control
 * can do to someone holding the key down to get back to their game. The
 * slow end stops at game speed on purpose -- this chunk's mechanism is
 * shortening the wait between frames, never lengthening it. */
void pico8_replay_control_faster(pico8_replay_control_t *control);
void pico8_replay_control_slower(pico8_replay_control_t *control);

/* Jumps straight to the uncapped level: "get me back to my game now",
 * user story 7. One key rather than a held "faster", because the player
 * who wants it wants it at once and from whatever level they are on.
 * Not a one-way door -- slower() steps back down from it. */
void pico8_replay_control_uncap(pico8_replay_control_t *control);

/* The current level as a multiple of the game's 60 FPS -- 1, 2, 4 or 8
 * -- or 0 for the uncapped level, which is not a multiple of anything.
 * Exposed for the progress line and for tests; the driver wants
 * target_fps below. */
int pico8_replay_control_speed_multiplier(const pico8_replay_control_t *control);

/* The number to hand raylib's SetTargetFPS right now. With
 * `replay_active` zero -- the recording is exhausted and the player has
 * the controls -- this is always PICO8_REPLAY_CONTROL_LIVE_FPS,
 * whatever speed the replay was left on (see this header's top
 * comment). With it non-zero, it is the current level's cap, or 0 for
 * the uncapped level, 0 being raylib's own documented "do not wait
 * between frames at all" rather than a sentinel this port invented --
 * so the value returned here is passed through untranslated. */
int pico8_replay_control_target_fps(const pico8_replay_control_t *control, int replay_active);

/* Writes the replay's progress line into `out` (always NUL-terminated,
 * truncated rather than overflowed if `out_size` is too small):
 * "replay 1234/9589 (12%) speed x4", or "... speed max" at the uncapped
 * level, which prints a word because "x0" would read as "stopped".
 * `frames_played` is a 1-based count of frames served, so the line reads
 * 1/N on the first frame and N/N on the last -- the numbering a player
 * counts in, deliberately NOT the 0-based index the marker file writes
 * (see pico8_raylib_backend_poll_input). A `frames_total` of zero
 * prints 0% instead of dividing by it: this line is a comfort feature
 * and must never be the thing that kills the session it exists to
 * reassure the player about.
 *
 * The string is built here, rather than formatted at the DrawText call
 * site, so that "what the overlay says" has a test. Where it is drawn
 * and whether it is legible cannot have one -- those need a real window
 * and a human -- but that a replay stopped at frame 1234 of 9589 says
 * 1234 and not 1233 is an ordinary fact about a function. */
void pico8_replay_control_format_status(const pico8_replay_control_t *control, int frames_played, int frames_total, char *out, size_t out_size);

#endif
