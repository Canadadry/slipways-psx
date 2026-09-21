/* Chunk 1 of docs/prd/triage/raylib-replay-then-resume.md ("Lecture de
 * l'argument et chargement de l'enregistrement dans le pilote
 * interactif, avec l'erreur de lancement explicite"): the one thing the
 * interactive driver needs to know at startup -- was this session
 * started from a recording, or not -- and, if it was, the recorded
 * frames themselves.
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
 * WHAT THIS CHUNK DELIBERATELY DOES NOT DO. The PRD's Implementation
 * Chunk 2 is the switchable input source -- "les frames viennent du
 * fichier tant qu'il en reste, du matériel ensuite" -- and it is that
 * switch, not this open, that carries the feature's real behavior. No
 * frame-serving call exists here yet: adding one now, with no caller and
 * no loop to exercise it, would be a guess at an interface Chunk 2 is
 * about to design against a real loop. What is here is only the opening
 * half: turn an optional command-line path into a decision, holding the
 * loaded frames for the next chunk to consume.
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
 * Read through the accessors below rather than poking the fields: Chunk
 * 2 owns how frames are handed out. */
typedef struct {
    pico8_replay_source_status_t status;
    const char *path;
    pico8_input_t recording;
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

/* Releases the loaded frames (pico8_input_free) and returns `*source` to
 * the PICO8_REPLAY_SOURCE_NONE state. Safe on a source that loaded
 * nothing, and safe to call twice. */
void pico8_replay_source_close(pico8_replay_source_t *source);

#endif
