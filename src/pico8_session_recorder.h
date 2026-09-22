/* Chunk 3 of docs/prd/triage/raylib-replay-then-resume.md ("Continuite
 * de l'enregistrement : la session reprise ecrit un enregistrement
 * complet, contenant les frames rejouees puis les nouvelles, verifie
 * rejouable"): the whole memory of the session currently being recorded
 * -- where the recording lives, how many frames it already holds, and
 * where in the file the next one goes -- and the one operation that
 * moves it forward, "this frame was played, put it in the recording".
 *
 * This is bookkeeping that already existed and already worked: it was
 * three fields and four lines inside pico8_raylib_backend.c. It is a
 * module of its own now for one reason, and it is a testing reason, not
 * a design one. pico8_raylib_backend.c includes raylib.h, the
 * interactive driver may never be launched by an agent at this phase
 * (CLAUDE.md), and raylib is not vendored in every environment, so
 * nothing that lives in that file can be exercised by `make unit` at
 * all. The PRD's Testing Decisions say plainly what to do about that:
 * "La logique testable doit donc etre placee hors du module raylib,
 * comme l'a deja ete la conversion de coordonnees -- un module sans
 * dependance raylib, teste directement, est le precedent a suivre."
 * pico8_raylib_backend_coords.{c,h} did it first, pico8_replay_source.
 * {c,h} did it second; this is the third and it follows them literally.
 * Nothing here includes raylib.h, and nothing here ever will.
 *
 * WHAT THE EXTRACTION IS FOR, CONCRETELY. The property this chunk owes
 * the player is that resuming a game keeps recording it: the resulting
 * file must hold the replayed frames followed by the new ones, and be
 * replayable in its turn (user stories 8 and 9), "sinon reprendre une
 * partie casse la chaine". src/main_raylib.c's loop already satisfies
 * that by recording every frame it is about to play, replayed or live,
 * with one unconditional call. But a mechanism that is merely present
 * is not a mechanism that is proven, and the only pieces of that loop
 * that were testable were the input source and the append writer, each
 * on its own -- never the pair, which is where the continuity actually
 * lives. With the recorder out here, tests/test_pico8_recording_
 * continuity.c can drive the real pico8_replay_source and the real
 * recorder exactly as the loop drives them, and assert on the file that
 * comes out.
 *
 * WHY open() TAKES AN EXPLICIT PATH INSTEAD OF MINTING A TIMESTAMPED
 * ONE. The timestamp stays in pico8_raylib_backend_init(), which needs
 * it anyway: the marker file it writes (see pico8_raylib_backend.h)
 * must carry the SAME <timestamp> as the recording so a human can pair
 * the two by name, and that pairing is only expressible where both
 * names are built, side by side, from one time(NULL). Minting the path
 * here would have split that single fact across two modules and left
 * the backend asking the recorder what name it had chosen. It would
 * also have made this module depend on the clock, so that every test of
 * it would write to a path it could not predict -- the opposite of what
 * the extraction is for.
 *
 * WHY A DROPPED FRAME IS A RETURN VALUE AND NOT A CRASH. A failed
 * append returns 0 and the recording simply does not grow; the session
 * keeps running. That is the behavior pico8_raylib_backend_record_frame
 * already had and it is deliberate: this recorder runs inside a live
 * playtest, where losing a frame of a debugging aid is a far smaller
 * harm than killing the game a human is playing. The frame counter is
 * left alone on failure on purpose too -- it is defined as the index
 * the NEXT frame will occupy in the file, which is what the backend's
 * marker file writes out, so counting a frame that never landed would
 * make every later marker point one frame too far.
 *
 * REJECTED: keeping the frames in memory and rewriting the file each
 * time. pico8_input_save would make this module a two-liner, at the
 * cost of re-serialising the whole session every frame -- an O(n^2)
 * session that visibly slowed down a real playtest, which is the
 * reason pico8_input_append exists at all. Its documented contract is
 * also exactly what this chunk needs and what makes the resumed file
 * replayable: after every appended frame the file on disk is
 * byte-for-byte what pico8_input_save would have written for the same
 * frames, which tests/test_pico8_input.c pins down frame by frame
 * (test_append_recording_is_byte_identical_to_save_every_frame).
 *
 * REJECTED: having this module decide what to record -- e.g. skipping
 * frames that came from a replay. That would be the one change capable
 * of breaking this chunk's entire point: a resumed recording that
 * contained only the new frames would describe half a game and would
 * not replay to the state it claims. A replayed frame and a live frame
 * are the same thing to this module by construction, because the PRD
 * requires the resulting file to be "un enregistrement complet et
 * autonome". */

#ifndef PICO8_SESSION_RECORDER_H
#define PICO8_SESSION_RECORDER_H

#include "pico8_input.h"

/* Room for the backend's own tmp/events_recorded_<timestamp>.json and
 * then some. Not a limit any caller is expected to reach: the one real
 * caller builds its path itself from a fixed prefix and a Unix
 * timestamp. */
#define PICO8_SESSION_RECORDER_PATH_MAX 128

/* The session's recording, as seen by the loop that is filling it.
 * `path` is the file being written (copied in, so the caller need not
 * keep its buffer alive), `frame_count` is how many frames have
 * successfully landed in it -- equivalently, the 0-based index the next
 * one will occupy in the file's own "frames" array -- and `tail_offset`
 * is pico8_input_append's private resume point, this module's only
 * reason to exist as state rather than as a function. Read `frame_count`
 * through the accessor below; no field is meant to be written
 * directly. */
typedef struct {
    char path[PICO8_SESSION_RECORDER_PATH_MAX];
    int frame_count;
    long tail_offset;
} pico8_session_recorder_t;

/* Starts a new, empty recording at `path`. The file itself is not
 * created here but by the first recorded frame -- a session that plays
 * no frame at all leaves no file, exactly as before this module
 * existed, and pico8_input_load would reject an empty recording anyway.
 * An existing file at `path` is truncated by that first frame (append's
 * "tail offset 0 means start over" contract), never appended to: two
 * sessions' frames in one file would be a recording of a game nobody
 * played. `path` must be shorter than PICO8_SESSION_RECORDER_PATH_MAX;
 * a longer one is a caller bug and aborts rather than silently
 * recording to a truncated, wrong path. */
void pico8_session_recorder_open(pico8_session_recorder_t *recorder, const char *path);

/* Adds one played frame to the recording, replayed or live alike --
 * this module never knows or asks which, and that indifference is the
 * whole of the continuity this chunk owes (see the header comment).
 * Returns 1 if the frame landed, 0 if the append failed, in which case
 * the recording and the frame count are both left exactly as they were
 * and the session carries on. After a successful call the file on disk
 * is a complete, valid events.json that pico8_input_load parses -- not
 * a file that only becomes valid when the session ends -- which is what
 * makes a recording of a crashed session usable, and what lets a test
 * reload the file at any point. */
int pico8_session_recorder_record_frame(pico8_session_recorder_t *recorder, const pico8_input_frame_t *frame);

/* How many frames this session has recorded so far, i.e. the 0-based
 * index the next recorded frame will have in the file. The backend's
 * marker file writes this out as "the frame I want to talk about", so
 * it is the recording's own numbering, not a count of loop iterations:
 * a frame whose append failed is not in the file and is not counted. */
int pico8_session_recorder_frame_count(const pico8_session_recorder_t *recorder);

/* Ends the session's recording and returns the struct to its
 * pre-open state. Purely in-memory: the file on disk is already
 * complete and is deliberately left in place -- it is the artifact the
 * whole feature produces. Nothing here can fail, and calling it is
 * optional for correctness; it exists so a caller that reuses a
 * recorder cannot inherit the previous session's tail offset. */
void pico8_session_recorder_close(pico8_session_recorder_t *recorder);

#endif
