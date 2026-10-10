#ifndef GAME_DEMO_DATA_H
#define GAME_DEMO_DATA_H

#include <stdint.h>

/* Defined only by tmp/demo/demo.c, which demo_generate writes from pico8/demo.json. */

#define DEMO_DATA_FRAME_LIMIT 3600
#define DEMO_DATA_CAPTION_TEXT_SIZE 66 /* 2 lines of 32 characters, '\n' between, NUL */

typedef struct {
    int from; /* shown on from <= frame < to */
    int to;
    char text[DEMO_DATA_CAPTION_TEXT_SIZE]; /* lines joined by '\n' */
} demo_data_caption_t;

/* Separate arrays, not a struct of pointers: make pointer-guard counts pointer fields and file-scope pointers of
 * src/game/ and no step may raise its counts. */
extern const uint8_t demo_data_record[]; /* a save record, as save_state_decode reads it */
extern const int demo_data_record_count;
extern const uint16_t demo_data_pad_buttons[]; /* pad masks; [0] is the first frame after the resume */
extern const int demo_data_frame_count;
extern const demo_data_caption_t demo_data_captions[];
extern const int demo_data_caption_count;

#endif
