#ifndef P8_INPUT_H
#define P8_INPUT_H

typedef struct {
    int mouse_x;
    int mouse_y;
    int mouse_buttons;
    int pad_buttons;
} p8_input_frame_t;

#define P8_PAD_UP 1
#define P8_PAD_DOWN 2
#define P8_PAD_LEFT 4
#define P8_PAD_RIGHT 8
#define P8_PAD_CROSS 16
#define P8_PAD_CIRCLE 32
#define P8_PAD_SQUARE 64
#define P8_PAD_TRIANGLE 128
#define P8_PAD_L1 256
#define P8_PAD_R1 512
#define P8_PAD_START 1024

#define P8_MOUSE_LEFT 1

typedef struct {
    int x;
    int y;
    int buttons;
} p8_mouse_t;

typedef struct {
    p8_input_frame_t *frames;
    int frame_count;
    int current_frame;
} p8_input_t;

void p8_input_init(p8_input_t *input);

int p8_input_load(p8_input_t *input, const char *path);

int p8_input_save(const p8_input_frame_t *frames, int frame_count, const char *path);

int p8_input_append(const p8_input_frame_t *frame, const char *path, long *tail_offset);

void p8_input_free(p8_input_t *input);

void p8_input_step(p8_input_t *input);

p8_mouse_t p8_input_read_mouse(const p8_input_t *input);

#endif
