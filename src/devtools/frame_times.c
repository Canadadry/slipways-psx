#include "devtools/frame_times.h"

#include <assert.h>
#include <stdio.h>

#include "p8/fatal.h"

typedef struct {
    uint16_t update;
    uint16_t draw;
    uint16_t submit;
    uint16_t vsyncs;
} frame_record_t;

static frame_record_t g_frames[FRAME_TIMES_CAP];
static int g_count;

void frame_times_init(void) {
    g_count = 0;
}

static uint16_t mark_delta(uint16_t from, uint16_t to) {
    return (uint16_t)(to - from);
}

void frame_times_record(const uint16_t marks[FRAME_TIMES_MARKS], uint32_t vsyncs) {
    if (g_count == FRAME_TIMES_CAP) {
        char message[64];
        snprintf(message, sizeof message, "frame table full at frame %d, capacity %d", g_count, FRAME_TIMES_CAP);
        p8_fatal(message);
    }
    frame_record_t *frame = &g_frames[g_count++];
    frame->update = mark_delta(marks[0], marks[1]);
    frame->draw = mark_delta(marks[1], marks[2]);
    frame->submit = mark_delta(marks[2], marks[3]);
    frame->vsyncs = vsyncs > UINT16_MAX ? UINT16_MAX : (uint16_t)vsyncs;
}

int frame_times_count(void) {
    return g_count;
}

frame_time_t frame_times_get(int frame) {
    assert(frame >= 0 && frame < g_count && "frame_times_get past the recorded frames");
    const frame_record_t *record = &g_frames[frame];
    frame_time_t time = {
        .update = record->update,
        .draw = record->draw,
        .submit = record->submit,
        .total = (uint32_t)record->update + record->draw + record->submit,
        .vsyncs = record->vsyncs,
    };
    return time;
}

void frame_times_write(const state_sink_t *sink) {
    char line[128];
    for (int i = 0; i < g_count; i++) {
        frame_time_t time = frame_times_get(i);
        snprintf(line, sizeof line, "frame %d update %lu draw %lu submit %lu total %lu vsyncs %lu\n", i,
                 (unsigned long)time.update, (unsigned long)time.draw, (unsigned long)time.submit,
                 (unsigned long)time.total, (unsigned long)time.vsyncs);
        sink->write_line(sink->context, line);
    }
}

void frame_times_write_calibration(const state_sink_t *sink, uint32_t hblanks_per_vsync) {
    char line[48];
    snprintf(line, sizeof line, "hblanks-per-vsync %lu\n", (unsigned long)hblanks_per_vsync);
    sink->write_line(sink->context, line);
}
