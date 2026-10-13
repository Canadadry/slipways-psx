#ifndef DEVTOOLS_FRAME_TIMES_H
#define DEVTOOLS_FRAME_TIMES_H

#include <stdint.h>

#include "devtools/state_format.h"

/* full_game's 6,059 frames plus 1/8, rounded up to a multiple of 64. */
#define FRAME_TIMES_CAP 6848

#define FRAME_TIMES_MARKS 4

typedef struct {
    uint32_t update;
    uint32_t draw;
    uint32_t submit;
    uint32_t total;
    uint32_t vsyncs;
} frame_time_t;

void frame_times_init(void);

/* marks are 16-bit hblank counter readings: before the input step, after the
   update, after the draw, after the submit; each part is the difference of
   two of them modulo 65,536. A vsync count above 65,535 is stored as 65,535.
   Recording past FRAME_TIMES_CAP frames is a p8_fatal. */
void frame_times_record(const uint16_t marks[FRAME_TIMES_MARKS], uint32_t vsyncs);

int frame_times_count(void);

frame_time_t frame_times_get(int frame);

void frame_times_write(const state_sink_t *sink);

void frame_times_write_calibration(const state_sink_t *sink, uint32_t hblanks_per_vsync);

#endif
