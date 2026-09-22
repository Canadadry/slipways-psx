#ifndef PICO8_INPUT_H
#define PICO8_INPUT_H

/* One scripted frame: mouse_x/mouse_y/mouse_buttons feed
 * stat(32)/stat(33)/stat(34), and dpad[0..3] is btn(0)..btn(3) for that
 * frame (0=left, 1=right, 2=up, 3=down). */
/* TROU-2 */
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
 * `input`, positioned at frame 0. Returns 1 on success, 0 on a missing or
 * malformed file: no "frames" array, a frame object missing one of
 * mouse_x/mouse_y/mouse_buttons, a "dpad" that is not exactly 4 JSON
 * booleans, or an empty frames array. There is no partial load: on 0,
 * `input` is left untouched and the caller has nothing to replay. */
int pico8_input_load(pico8_input_t *input, const char *path);

/* Writes `frame_count` frames to `path` as a complete events.json file --
 * always a full rewrite -- in the exact shape pico8_input_load parses:
 * {"frames": [ { "mouse_x": ..., "mouse_y": ..., "mouse_buttons": ...,
 * "dpad": [bool, bool, bool, bool] }, ... ]}, nonzero dpad written as
 * true. Returns 1 on success, 0 on a NULL `frames`, a non-positive
 * `frame_count` (it never writes a file pico8_input_load would reject), or
 * a file error. No partial file is left behind: `path` is replaced only
 * once every frame is written and the file has closed. */
int pico8_input_save(const pico8_input_frame_t *frames, int frame_count, const char *path);

/* Adds one frame to the recording at `path` without re-serializing the
 * frames already there, so recording one frame costs the same whatever the
 * session's length. `*io_tail_offset` is the writer's whole memory of the
 * recording: set it to 0 to start one -- `path` is then created or
 * truncated -- and pass the same variable back untouched on every later
 * call.
 *
 * After each successful call `path` holds a complete events.json that is
 * byte-for-byte what pico8_input_save(frames, n, path) writes for the same
 * n frames. That identity is the contract the whole replay toolchain rests
 * on: break it and a recorded session no longer replays as one that was
 * saved. It is why the caller need not keep the frames it has recorded.
 *
 * Returns 1 on success, 0 on a NULL argument or a file error, leaving
 * `*io_tail_offset` unchanged so the next call rewrites the same slot
 * rather than writing past a frame that never landed. */
int pico8_input_append(const pico8_input_frame_t *frame, const char *path, long *io_tail_offset);

/* Releases the frame array. Safe on an input that was only
 * pico8_input_init'd, and safe to call again before a fresh load. */
void pico8_input_free(pico8_input_t *input);

/* Advances to the next scripted frame. Stepping off the last frame of the
 * script is a hard error: it means the replay is running longer than the
 * script it was given. No looping, no clamping. */
void pico8_input_step(pico8_input_t *input);

/* `b` is 0 (left), 1 (right), 2 (up) or 3 (down); any other index
 * returns 0. */
int pico8_btn(const pico8_input_t *input, int b);

/* `n` is 32 (mouse x), 33 (mouse y) or 34 (mouse button bitmask, bit 0 =
 * left, bit 1 = right, bit 2 = middle); any other index returns 0. */
int pico8_stat(const pico8_input_t *input, int n);

#endif
