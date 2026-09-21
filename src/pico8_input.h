#ifndef PICO8_INPUT_H
#define PICO8_INPUT_H

/* One scripted frame from events.json: mouse_x/mouse_y/mouse_buttons feed
 * stat(32)/stat(33)/stat(34) directly, and dpad[0..3] is exactly
 * btn(0)..btn(3) for that frame (0=left, 1=right, 2=up, 3=down), per the
 * PRD's events.json schema. */
typedef struct {
    int mouse_x;
    int mouse_y;
    int mouse_buttons;
    int dpad[4];
} pico8_input_frame_t;

typedef struct {
    pico8_input_frame_t *frames;
    int frame_count;
    int current_frame;
} pico8_input_t;

void pico8_input_init(pico8_input_t *input);

/* Parses `path` as an events.json replay script and loads its frames into
 * `input`, positioned at frame 0. Returns 1 on success, 0 if the file is
 * missing or malformed: a missing "frames" array, a frame object missing
 * one of mouse_x/mouse_y/mouse_buttons, a "dpad" that isn't exactly 4
 * boolean entries, or an empty frames array. Callers must treat 0 as a
 * hard, unrecoverable load error for this phase -- there is no
 * partial/best-effort load. */
int pico8_input_load(pico8_input_t *input, const char *path);

/* Writes `frame_count` frames from `frames` to `path` as a complete
 * events.json-schema file (a full rewrite, never an append), in exactly
 * the shape pico8_input_load parses: {"frames": [ { "mouse_x": ...,
 * "mouse_y": ..., "mouse_buttons": ..., "dpad": [bool, bool, bool,
 * bool] }, ... ]}, with dpad[b] written as JSON true/false (nonzero is
 * true). This is new, C-only infrastructure with no Lua-source
 * equivalent (the events.json writer side of the schema
 * pico8_input_load already reads) -- not a ported function, so it is
 * named plainly rather than mimicking pico8/lua.lua's own naming, per
 * this port's established naming exception for such plumbing (see
 * e.g. pico8_cart_load). Returns 1 on success, 0 on failure: a NULL
 * `frames`, a non-positive `frame_count` (mirroring pico8_input_load's
 * own hard-error treatment of an empty frames array -- this function
 * never writes a file pico8_input_load couldn't then load back), or a
 * file-open/write/close failure (e.g. a bad path or a full disk). No
 * partial file is left in place: nothing is written to `path` at all
 * unless every frame is written and the file closes successfully. */
int pico8_input_save(const pico8_input_frame_t *frames, int frame_count, const char *path);

/* Adds one more frame to the events.json-schema recording at `path`
 * without re-serializing the frames already recorded there, so the cost of
 * recording a frame does not grow with the length of the session (the
 * always-on per-frame recording in pico8_raylib_backend_present() is the
 * one caller this exists for). `*io_tail_offset` is the writer's whole
 * memory of the recording: set it to 0 to start a new one -- `path` is
 * then created or truncated and written with this single frame -- and pass
 * the same variable back, untouched, on every later call for that
 * recording. After every successful call `path` holds a complete, valid
 * events.json that pico8_input_load parses, byte-for-byte identical to
 * what pico8_input_save(frames, n, path) writes for the same n frames;
 * that identity is why the caller never has to keep the frames it has
 * already recorded, and why a session recorded this way replays exactly
 * like one recorded by repeated full rewrites. Returns 1 on success, 0 on
 * failure (a NULL argument, or a file open/seek/write/close failure),
 * leaving `*io_tail_offset` unchanged so the next call writes its frame
 * into the same slot rather than past a frame that never landed. */
int pico8_input_append(const pico8_input_frame_t *frame, const char *path, long *io_tail_offset);

/* Releases the frame array. Safe to call on an input that was only
 * pico8_input_init'd (never loaded), and safe to call again before a
 * fresh pico8_input_load. */
void pico8_input_free(pico8_input_t *input);

/* Advances to the next scripted frame. Calling this while already on the
 * last frame of the loaded script means the replay is running longer
 * than its own script -- a test-authoring bug, not a runtime case this
 * phase handles gracefully (no looping/clamping). Per the PRD's explicit
 * "hard error" decision, this is enforced with assert() and aborts the
 * process; it is not a recoverable error path. */
void pico8_input_step(pico8_input_t *input);

/* b must be 0 (left), 1 (right), 2 (up), or 3 (down) -- PICO-8's own
 * button-index order for player 0 (the only player this phase supports).
 * Any other index returns 0. */
int pico8_btn(const pico8_input_t *input, int b);

/* n must be 32 (mouse x), 33 (mouse y), or 34 (mouse button bitmask,
 * bit 0 = left, bit 1 = right, bit 2 = middle) -- the only stat()
 * indices in scope for this phase. Any other index returns 0. */
int pico8_stat(const pico8_input_t *input, int n);

#endif
