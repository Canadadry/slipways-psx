#ifndef DEVTOOLS_DRAW_TIMING_H
#define DEVTOOLS_DRAW_TIMING_H

#include <stdint.h>

#include "devtools/draw_filter.h"
#include "devtools/state_format.h"
#include "p8/draw_backend.h"

#define DRAW_TIMING_CAP 1024

/* Both counters free-run over 16 bits. A call whose coarse delta reaches coarse_wrap_limit may have
 * wrapped the fine counter: its frame is flagged wrapped. */
typedef struct {
    uint16_t (*fine)(void);
    uint16_t (*coarse)(void);
    uint16_t coarse_wrap_limit;
} draw_timing_clock_t;

typedef struct {
    uint32_t ticks[DRAW_FILTER_FAMILY_COUNT];
    uint32_t calls[DRAW_FILTER_FAMILY_COUNT];
    /* Every family but DRAW_FILTER_PATTERN, whose time is already under its shape. */
    uint32_t in_call;
    int wrapped;
} draw_timing_frame_t;

/* Wraps the backend active at the call and empties the table; *clock must outlive the install.
 * Returns the wrapped backend: reinstalling it with p8_draw_backend_set removes the wrapper. */
const p8_draw_backend_t *draw_timing_install(const draw_timing_clock_t *clock);

/* Stores the calls timed since the last frame end as the next frame. Recording past
 * DRAW_TIMING_CAP frames is a p8_fatal. A call count above 65,535 is stored as 65,535. */
void draw_timing_frame_end(void);

int draw_timing_count(void);

draw_timing_frame_t draw_timing_get(int frame);

/* One line per frame: "frame <n>", then "<family> <ticks> <calls>" per family, then "wrapped <0|1>". */
void draw_timing_write(const state_sink_t *sink);

void draw_timing_write_calibration(const state_sink_t *sink, uint32_t fine_per_64_coarse);

#endif
