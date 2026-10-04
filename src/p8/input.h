#ifndef P8_INPUT_H
#define P8_INPUT_H

typedef struct {
    int mouse_x;
    int mouse_y;
    int mouse_buttons;
    int dpad[6];
} p8_input_frame_t;

#define P8_MOUSE_LEFT 1
#define P8_MOUSE_RIGHT 2
#define P8_MOUSE_MIDDLE 4

typedef struct {
    int x;
    int y;
    int buttons;
} p8_mouse_t;

typedef struct {
    p8_input_frame_t *frames;
    int frame_count;
    int current_frame;
    int keyboard_cursor;
} p8_input_t;

void p8_input_init(p8_input_t *input);

int p8_input_load(p8_input_t *input, const char *path);

int p8_input_save(const p8_input_frame_t *frames, int frame_count, const char *path, int keyboard_cursor);

int p8_input_append(const p8_input_frame_t *frame, const char *path, long *tail_offset, int keyboard_cursor);

void p8_input_free(p8_input_t *input);

void p8_input_step(p8_input_t *input);

int p8_btn(const p8_input_t *input, int b);

p8_mouse_t p8_input_read_mouse(const p8_input_t *input);

#endif
