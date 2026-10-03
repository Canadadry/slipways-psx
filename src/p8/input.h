#ifndef P8_INPUT_H
#define P8_INPUT_H

/* One scripted frame: mouse_x/mouse_y/mouse_buttons feed
 * stat(32)/stat(33)/stat(34), and dpad[0..5] is btn(0)..btn(5) for that
 * frame (0=left, 1=right, 2=up, 3=down, 4=O, 5=X) -- O and X included
 * despite the name, to keep the events.json key. */
typedef struct {
    int mouse_x;
    int mouse_y;
    int mouse_buttons;
    int dpad[6];
} p8_input_frame_t;

typedef struct {
    p8_input_frame_t *frames;
    int frame_count;
    int current_frame;
} p8_input_t;

void p8_input_init(p8_input_t *input);

/* Parses `path` as an events.json replay script and loads its frames into
 * `input`, positioned at frame 0. Returns 1 on success, 0 on a missing or
 * malformed file: no "frames" array, a frame object missing one of
 * mouse_x/mouse_y/mouse_buttons, a "dpad" that is neither 4 nor 6 JSON
 * booleans, or an empty frames array. A 4-boolean dpad loads with O and X
 * released. There is no partial load: on 0,
 * `input` is left untouched and the caller has nothing to replay. */
int p8_input_load(p8_input_t *input, const char *path);

/* Writes `frame_count` frames to `path` as a complete events.json file --
 * always a full rewrite -- in the exact shape p8_input_load parses:
 * {"frames": [ { "mouse_x": ..., "mouse_y": ..., "mouse_buttons": ...,
 * "dpad": [6 bools] }, ... ]}, nonzero dpad written as
 * true. Returns 1 on success, 0 on a NULL `frames`, a non-positive
 * `frame_count` (it never writes a file p8_input_load would reject), or
 * a file error. No partial file is left behind: `path` is replaced only
 * once every frame is written and the file has closed. */
int p8_input_save(const p8_input_frame_t *frames, int frame_count, const char *path);

/* Adds one frame to the recording at `path` without re-serializing the
 * frames already there, so recording one frame costs the same whatever the
 * session's length. `*tail_offset` is the writer's whole memory of the
 * recording: set it to 0 to start one -- `path` is then created or
 * truncated -- and pass the same variable back untouched on every later
 * call.
 *
 * After each successful call `path` holds a complete events.json that is
 * byte-for-byte what p8_input_save writes for the same frames. That
 * identity is the contract the whole replay toolchain rests
 * on: break it and a recorded session no longer replays as one that was
 * saved. It is why the caller need not keep the frames it has recorded.
 *
 * Returns 1 on success, 0 on a NULL argument or a file error, leaving
 * `*tail_offset` unchanged so the next call rewrites the same slot
 * rather than writing past a frame that never landed. */
int p8_input_append(const p8_input_frame_t *frame, const char *path, long *tail_offset);

/* Releases the frame array. Safe on an input that only went through
 * p8_input_init, and safe to call again before a fresh load. */
void p8_input_free(p8_input_t *input);

/* Advances to the next scripted frame. Stepping off the last frame of the
 * script is a hard error: it means the replay is running longer than the
 * script it was given. No looping, no clamping. */
void p8_input_step(p8_input_t *input);

/* The button index is 0 (left), 1 (right), 2 (up), 3 (down), 4 (O) or 5
 * (X); any other index returns 0. */
int p8_btn(const p8_input_t *input, int b);

/* The index is 32 (mouse x), 33 (mouse y) or 34 (mouse button bitmask,
 * bit 0 = left, bit 1 = right, bit 2 = middle); any other index returns 0. */
int p8_stat(const p8_input_t *input, int n);

#endif
