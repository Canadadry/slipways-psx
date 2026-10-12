#include "devtools/draw_timing.h"

#include "p8/fatal.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint32_t ticks[DRAW_FILTER_FAMILY_COUNT];
    uint16_t calls[DRAW_FILTER_FAMILY_COUNT];
    uint8_t wrapped;
} frame_record_t;

static const p8_draw_backend_t *wrapped;
static const draw_timing_clock_t *timing_clock;
static draw_timing_frame_t current;
static frame_record_t table[DRAW_TIMING_CAP];
static int count;

typedef struct {
    uint16_t fine;
    uint16_t coarse;
} reading_t;

static reading_t call_begin(void) {
    reading_t start = { timing_clock->fine(), timing_clock->coarse() };
    return start;
}

static uint32_t call_end(draw_filter_family_t family, reading_t start) {
    uint16_t fine = timing_clock->fine();
    uint16_t coarse = timing_clock->coarse();
    uint32_t ticks = (uint16_t)(fine - start.fine);
    if ((uint16_t)(coarse - start.coarse) >= timing_clock->coarse_wrap_limit) current.wrapped = 1;
    current.ticks[family] += ticks;
    current.calls[family]++;
    return ticks;
}

static void pattern_end(const p8_graphics_t *gfx, uint32_t ticks) {
    if ((gfx->fillp_pattern & 0xFFFFu) == 0) return;
    current.ticks[DRAW_FILTER_PATTERN] += ticks;
    current.calls[DRAW_FILTER_PATTERN]++;
}

static void timing_cls(p8_vram_t *mem) {
    reading_t start = call_begin();
    wrapped->cls(mem);
    call_end(DRAW_FILTER_CLS, start);
}

static void timing_pset(const p8_graphics_t *gfx, p8_vram_t *mem, int x, int y, int col) {
    reading_t start = call_begin();
    wrapped->pset(gfx, mem, x, y, col);
    call_end(DRAW_FILTER_PSET, start);
}

static void timing_line(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    reading_t start = call_begin();
    wrapped->line(gfx, mem, x1, y1, x2, y2, col);
    call_end(DRAW_FILTER_LINE, start);
}

static void timing_rect(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    reading_t start = call_begin();
    wrapped->rect(gfx, mem, x1, y1, x2, y2, col);
    call_end(DRAW_FILTER_RECT, start);
}

static void timing_rectfill(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    reading_t start = call_begin();
    wrapped->rectfill(gfx, mem, x1, y1, x2, y2, col);
    pattern_end(gfx, call_end(DRAW_FILTER_RECTFILL, start));
}

static void timing_circ(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    reading_t start = call_begin();
    wrapped->circ(gfx, mem, cx, cy, r, col);
    call_end(DRAW_FILTER_CIRC, start);
}

static void timing_circfill(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    reading_t start = call_begin();
    wrapped->circfill(gfx, mem, cx, cy, r, col);
    pattern_end(gfx, call_end(DRAW_FILTER_CIRCFILL, start));
}

static void timing_spr(const p8_graphics_t *gfx, p8_vram_t *mem, int n, int x, int y, int w, int h) {
    reading_t start = call_begin();
    wrapped->spr(gfx, mem, n, x, y, w, h);
    call_end(DRAW_FILTER_SPR, start);
}

static void timing_print(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col) {
    reading_t start = call_begin();
    wrapped->print(gfx, mem, str, x, y, col);
    call_end(DRAW_FILTER_PRINT, start);
}

static const p8_draw_backend_t TIMING = {
    timing_cls, timing_pset, timing_line, timing_rect, timing_rectfill,
    timing_circ, timing_circfill, timing_spr, timing_print,
};

const p8_draw_backend_t *draw_timing_install(const draw_timing_clock_t *new_clock) {
    const p8_draw_backend_t *active = p8_draw_backend_get();
    assert(active != &TIMING && "the timing wrapper would wrap itself");
    assert(new_clock != NULL && new_clock->fine != NULL && new_clock->coarse != NULL);
    wrapped = active;
    timing_clock = new_clock;
    memset(&current, 0, sizeof current);
    count = 0;
    p8_draw_backend_set(&TIMING);
    return wrapped;
}

void draw_timing_frame_end(void) {
    if (count == DRAW_TIMING_CAP) {
        char message[64];
        snprintf(message, sizeof message, "draw timing table full at frame %d, capacity %d", count, DRAW_TIMING_CAP);
        p8_fatal(message);
    }
    frame_record_t *record = &table[count++];
    for (int family = 0; family < DRAW_FILTER_FAMILY_COUNT; family++) {
        uint32_t calls = current.calls[family];
        record->ticks[family] = current.ticks[family];
        record->calls[family] = calls > UINT16_MAX ? UINT16_MAX : (uint16_t)calls;
    }
    record->wrapped = (uint8_t)current.wrapped;
    memset(&current, 0, sizeof current);
}

int draw_timing_count(void) {
    return count;
}

draw_timing_frame_t draw_timing_get(int frame) {
    assert(frame >= 0 && frame < count && "draw_timing_get past the recorded frames");
    const frame_record_t *record = &table[frame];
    draw_timing_frame_t result;
    memset(&result, 0, sizeof result);
    for (int family = 0; family < DRAW_FILTER_FAMILY_COUNT; family++) {
        result.ticks[family] = record->ticks[family];
        result.calls[family] = record->calls[family];
        if (family != DRAW_FILTER_PATTERN) result.in_call += record->ticks[family];
    }
    result.wrapped = record->wrapped;
    return result;
}

static const char *const FAMILY_NAMES[DRAW_FILTER_FAMILY_COUNT] = {
    "cls", "pset", "line", "rect", "rectfill", "circ", "circfill", "spr", "print", "pattern",
};

void draw_timing_write(const state_sink_t *sink) {
    char line[320];
    for (int frame = 0; frame < count; frame++) {
        draw_timing_frame_t timing = draw_timing_get(frame);
        int used = snprintf(line, sizeof line, "frame %d", frame);
        for (int family = 0; family < DRAW_FILTER_FAMILY_COUNT; family++) {
            used += snprintf(line + used, sizeof line - (size_t)used, " %s %lu %lu", FAMILY_NAMES[family],
                             (unsigned long)timing.ticks[family], (unsigned long)timing.calls[family]);
        }
        used += snprintf(line + used, sizeof line - (size_t)used, " wrapped %d\n", timing.wrapped);
        assert(used < (int)sizeof line && "draw timing line truncated");
        sink->write_line(sink->context, line);
    }
}

void draw_timing_write_calibration(const state_sink_t *sink, uint32_t fine_per_64_coarse) {
    char line[48];
    snprintf(line, sizeof line, "clk8-per-64-hblanks %lu\n", (unsigned long)fine_per_64_coarse);
    sink->write_line(sink->context, line);
}
