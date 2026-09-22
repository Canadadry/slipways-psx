/* Chunks 1 and 2 of docs/prd/triage/raylib-replay-then-resume.md
 * ("Lecture de l'argument et chargement de l'enregistrement dans le
 * pilote interactif, avec l'erreur de lancement explicite", then
 * "Source d'entree commutable dans la boucle existante"): the one thing
 * the interactive driver needs to know at startup -- was this session
 * started from a recording, or not -- and then, frame by frame, the
 * recorded input itself until there is none left.
 *
 * The PRD's own Testing Decisions are what dictate that this lives in
 * its own header/source pair instead of inside src/main_raylib.c or
 * src/pico8_raylib_backend.c: the interactive driver cannot be launched
 * by an agent at all (CLAUDE.md), and raylib is not vendored in every
 * environment, so anything that includes raylib.h is untestable here by
 * construction. src/pico8_raylib_backend_coords.{c,h} is the precedent
 * this follows literally -- raylib-free logic extracted into a module a
 * plain `make unit` test can link on its own, with no window and no
 * libraylib.a. Nothing in this file includes raylib.h, and nothing in it
 * ever will; that is the point of the file.
 *
 * CHUNK 2 ADDS THE OTHER HALF: pico8_replay_source_next_frame, the
 * switchable input source itself -- "les frames viennent du fichier tant
 * qu'il en reste, du matériel ensuite". Chunk 1 deliberately shipped
 * without it, so that the interface would be designed against a real
 * loop rather than guessed at; src/main_raylib.c's loop is that caller,
 * and it wanted exactly one thing per frame: "give me this frame, or
 * tell me you are done". Everything below about that call follows from
 * that single question, and from the PRD's insistence that replay and
 * live play share ONE loop -- a driver that duplicated the loop would
 * diverge from the one actually played, which is the very fault this PRD
 * exists to avoid.
 *
 * WHY EXHAUSTION IS A RETURN VALUE AND NOT AN ASSERT. pico8_input.h's
 * own pico8_input_step() documents the opposite policy: stepping past
 * the last scripted frame is "a test-authoring bug, not a runtime case
 * this phase handles gracefully", enforced with assert() and aborting
 * the process. That contract is right for the headless driver, whose
 * whole run IS the script -- outliving the script there means the
 * fixture and the driver disagree. It is exactly wrong here: running
 * past the last recorded frame is the SUCCESS case of this feature, the
 * moment the player gets the controls back (user story 3). So this
 * module never calls pico8_input_step and never touches the loaded
 * pico8_input_t's own `current_frame` cursor; it keeps its own index and
 * reports exhaustion as an ordinary value the caller acts on every
 * frame. Sharing pico8_input's cursor would have meant either inheriting
 * an abort at frame N+1 or quietly redefining a contract the headless
 * driver still depends on.
 *
 * WHY NO SEPARATE "IS THE REPLAY OVER" PREDICATE. A caller that asked
 * first and fetched second would have two calls that must agree, and a
 * loop that reads the answer twice per frame; one call that either
 * yields a frame or says there is none cannot get out of step with
 * itself. It also makes the no-recording case (PICO8_REPLAY_SOURCE_NONE)
 * fall out with no branch at all in the driver: a source that was never
 * given a path is simply exhausted from its first call, so "the
 * recording ran out" and "there never was one" are the same code path,
 * and the driver has a single input-source switch rather than a mode
 * flag.
 *
 * WHY A THREE-VALUED OUTCOME RATHER THAN A BOOLEAN. "Loaded" and "not
 * loaded" would collapse the two cases the PRD insists on keeping apart:
 * no recording was asked for (an ordinary new game, user story 5), and a
 * recording WAS asked for but cannot be read (user story 12). The PRD's
 * Implementation Decisions call the second one "une erreur de lancement
 * explicite, jamais un repli silencieux sur une partie neuve", the
 * stated reason being that a silent fallback to a new game is
 * indistinguishable, to the player, from having lost their save. A
 * caller holding a boolean has no way to tell those apart and would have
 * to re-stat the path to guess, so the distinction is made here, once,
 * where the decision is actually taken.
 *
 * REJECTED: having this module print the error and exit() itself. It
 * would save the caller three lines, but it would make the module
 * untestable exactly where it matters (the error case would abort the
 * test process), and it would bury a policy decision -- what to print,
 * where, and with which exit status -- inside a module whose whole
 * reason for existing is to be a pure, testable decision. Reporting the
 * outcome and letting src/main_raylib.c own the message and the exit
 * code keeps both halves verifiable.
 *
 * REJECTED: parsing the recording here. src/pico8_input.h already owns
 * the events.json schema, in both directions (pico8_input_load /
 * pico8_input_save / pico8_input_append), and its documented contract is
 * already the hard, no-partial-load error this chunk needs: it returns 0
 * for a missing file, a missing "frames" array, a frame missing one of
 * mouse_x/mouse_y/mouse_buttons, a malformed dpad, or an empty frames
 * array. A second parser would be a second definition of "valid
 * recording", and the recordings this module consumes are written by
 * pico8_input_append -- the two must agree by construction, not by
 * coincidence. */

#ifndef PICO8_REPLAY_SOURCE_H
#define PICO8_REPLAY_SOURCE_H

#include "pico8_input.h"

typedef enum {
    /* No path was given on the command line: an ordinary new game,
     * today's behavior, unchanged. */
    PICO8_REPLAY_SOURCE_NONE = 0,
    /* A path was given and its recording is loaded and non-empty. */
    PICO8_REPLAY_SOURCE_LOADED = 1,
    /* A path was given and could not be loaded as a recording: missing,
     * unreadable, or malformed. The caller must treat this as a launch
     * error -- never as PICO8_REPLAY_SOURCE_NONE. */
    PICO8_REPLAY_SOURCE_LOAD_FAILED = 2
} pico8_replay_source_status_t;

/* `path` is the requested recording, kept only so a caller's error
 * message can name it; the struct does not own the string and never
 * copies it (argv outlives main). `recording` holds the loaded frames,
 * and is meaningful only when status is PICO8_REPLAY_SOURCE_LOADED --
 * in the other two cases it is an initialized, empty pico8_input_t, so
 * that pico8_replay_source_close is valid whatever the outcome was.
 * `next_frame_index` is how many frames have already been served, i.e.
 * the index of the one pico8_replay_source_next_frame will hand out
 * next; it is this module's own cursor and is deliberately NOT
 * `recording.current_frame` (see this header's top comment on
 * pico8_input_step). Read through the accessors below rather than poking
 * the fields. */
typedef struct {
    pico8_replay_source_status_t status;
    const char *path;
    pico8_input_t recording;
    int next_frame_index;
} pico8_replay_source_t;

/* Decides, once, whether this session replays a recording. `path` may be
 * NULL (no positional argument), which is PICO8_REPLAY_SOURCE_NONE and
 * loads nothing. Otherwise the file is loaded through pico8_input_load,
 * giving PICO8_REPLAY_SOURCE_LOADED or PICO8_REPLAY_SOURCE_LOAD_FAILED.
 * The status is both stored in `*source` and returned, so the usual
 * caller can branch on the call itself. `*source` is left safe to close
 * in every case, including the failure one. */
pico8_replay_source_status_t pico8_replay_source_open(pico8_replay_source_t *source, const char *path);

/* How many frames the recording holds: 0 unless the status is
 * PICO8_REPLAY_SOURCE_LOADED. This is the N of "loaded successfully with
 * N frames" -- the part of the outcome a caller can report and a test
 * can assert on -- and not a frame-serving API: it says how long the
 * replay will be, not what any frame contains. */
int pico8_replay_source_frame_count(const pico8_replay_source_t *source);

/* How many frames this source has already served, i.e. how far into the
 * recording the replay has got: 0 before the first
 * pico8_replay_source_next_frame call, N once the recording is
 * exhausted, and never more than N however many times exhaustion is
 * reported afterwards. Added by Chunk 4 for the replay's progress
 * indicator (user story 6), which needs exactly this number and the
 * frame count above to say "frame 1234 of 9589".
 *
 * It is an accessor on the cursor this module already keeps rather than
 * a counter the driver maintains in parallel, because two counters that
 * must agree eventually do not, and the one place that disagreement
 * would show is the line the player reads to decide whether the game
 * has frozen. Note it is a count, not an index: on the frame being
 * played it is one more than that frame's 0-based position, which is
 * the position the marker file writes. The two numbers are different on
 * purpose and neither is derived from the other. */
int pico8_replay_source_frames_served(const pico8_replay_source_t *source);

/* Serves the next recorded frame, or reports that there are none left.
 * Returns 1 and copies the frame into *out_frame when the recording
 * still has one; returns 0 and leaves *out_frame untouched once the
 * recording is exhausted -- which is the normal, expected end of every
 * replay, not an error. Given a recording of N frames this returns 1
 * exactly N times, in recorded order, then 0 on the (N+1)-th call and on
 * every call after it, forever: there is no looping, no clamping to the
 * last frame, and no abort. A source opened with no path
 * (PICO8_REPLAY_SOURCE_NONE) is exhausted from its very first call,
 * which is what lets a driver call this unconditionally and get today's
 * plain new game when no recording was asked for.
 *
 * The frame is COPIED rather than returned by pointer on purpose: the
 * one caller wraps it in a one-frame pico8_input_t of its own and hands
 * that to pico8_dispatch_set_input, exactly as it does for a live-polled
 * frame. Handing out a pointer into the recording would make the replayed
 * and the live frame two different shapes at the call site, and the
 * PRD's central constraint is that those two cases differ in nothing but
 * where the frame came from. */
int pico8_replay_source_next_frame(pico8_replay_source_t *source, pico8_input_frame_t *out_frame);

/* Releases the loaded frames (pico8_input_free) and returns `*source` to
 * the PICO8_REPLAY_SOURCE_NONE state. Safe on a source that loaded
 * nothing, and safe to call twice. */
void pico8_replay_source_close(pico8_replay_source_t *source);

#endif
