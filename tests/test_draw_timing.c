#include "devtools/draw_timing.h"

#include "p8/draw_backend.h"
#include "p8/gfx.h"
#include "p8/image.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define STDERR_LOG "tmp/test_draw_timing_stderr.txt"

static uint16_t fine_now;
static uint16_t coarse_now;
static uint16_t fine_step = 10;
static uint16_t coarse_step = 1;

static uint16_t fake_fine(void) {
    return fine_now;
}

static uint16_t fake_coarse(void) {
    return coarse_now;
}

static const draw_timing_clock_t CLOCK = { fake_fine, fake_coarse, 200 };

static void elapse(void) {
    fine_now = (uint16_t)(fine_now + fine_step);
    coarse_now = (uint16_t)(coarse_now + coarse_step);
}

enum { KINDS = 9, KIND_PRINT_OUTLINED = KINDS, KIND_LINES, KIND_PSETS, RECEIVED_KINDS };

typedef struct {
    int calls;
    const p8_graphics_t *gfx;
    p8_vram_t *mem;
    int args[6];
    const char *str;
} received_t;

static received_t received[RECEIVED_KINDS];

static void seen(int kind, const p8_graphics_t *gfx, p8_vram_t *mem, int a, int b, int c, int d, int e, int f) {
    received_t *r = &received[kind];
    r->calls++;
    r->gfx = gfx;
    r->mem = mem;
    int args[6] = { a, b, c, d, e, f };
    memcpy(r->args, args, sizeof args);
    elapse();
}

static void slow_cls(p8_vram_t *mem) {
    seen(0, NULL, mem, 0, 0, 0, 0, 0, 0);
}

static void slow_pset(const p8_graphics_t *gfx, p8_vram_t *mem, int x, int y, int col) {
    seen(1, gfx, mem, x, y, col, 0, 0, 0);
}

static void slow_line(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    seen(2, gfx, mem, x1, y1, x2, y2, col, 0);
}

static void slow_rect(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    seen(3, gfx, mem, x1, y1, x2, y2, col, 0);
}

static void slow_rectfill(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    seen(4, gfx, mem, x1, y1, x2, y2, col, 0);
}

static void slow_circ(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    seen(5, gfx, mem, cx, cy, r, col, 0, 0);
}

static void slow_circfill(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    seen(6, gfx, mem, cx, cy, r, col, 0, 0);
}

static void slow_spr(const p8_graphics_t *gfx, p8_vram_t *mem, int n, int x, int y, int w, int h) {
    seen(7, gfx, mem, n, x, y, w, h, 0);
}

static void slow_print(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col) {
    seen(8, gfx, mem, x, y, col, 0, 0, 0);
    received[8].str = str;
}

static void slow_print_outlined(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col) {
    seen(KIND_PRINT_OUTLINED, gfx, mem, x, y, col, 0, 0, 0);
    received[KIND_PRINT_OUTLINED].str = str;
}

static int image_calls;

static void slow_image_begin(int slot, int w, int h) {
    (void)slot, (void)w, (void)h;
    image_calls++;
    elapse();
}

static void slow_image_window(int slot, int x, int y) {
    (void)slot, (void)x, (void)y;
    image_calls++;
    elapse();
}

static void slow_image_end(int slot) {
    (void)slot;
    image_calls++;
    elapse();
}

static void slow_image_draw(const p8_graphics_t *gfx, p8_vram_t *mem, int slot, int x, int y) {
    (void)gfx, (void)mem, (void)slot, (void)x, (void)y;
    image_calls++;
    elapse();
}

static void slow_lines(const p8_graphics_t *gfx, p8_vram_t *mem, const p8_segment_t *segments, int count, int col) {
    seen(KIND_LINES, gfx, mem, count, col, segments[0].x1, segments[0].y2, 0, 0);
}

static void slow_psets(const p8_graphics_t *gfx, p8_vram_t *mem, const p8_point_t *points, int count) {
    seen(KIND_PSETS, gfx, mem, count, points[0].x, points[0].y, points[0].col, 0, 0);
}

static const p8_draw_backend_t SLOW = {
    slow_cls, slow_pset, slow_line, slow_rect, slow_rectfill,
    slow_circ, slow_circfill, slow_spr, slow_print, slow_print_outlined,
    slow_image_begin, slow_image_window, slow_image_end, slow_image_draw,
    slow_lines, slow_psets,
};

static p8_vram_t mem;
static p8_graphics_t gfx;

static void timing_over(const p8_draw_backend_t *backend) {
    memset(received, 0, sizeof received);
    p8_vram_init(&mem);
    p8_gfx_init(&gfx);
    fine_now = 0;
    coarse_now = 0;
    fine_step = 10;
    coarse_step = 1;
    p8_draw_backend_set(backend);
    const p8_draw_backend_t *wrapped = draw_timing_install(&CLOCK);
    assert(wrapped == backend);
    assert(p8_draw_backend_get() != backend);
}

static void timing_removed(void) {
    p8_draw_backend_set(NULL);
}

static void test_a_call_adds_its_fine_ticks_and_one_call_to_its_family(void) {
    timing_over(&SLOW);
    fine_step = 7;
    p8_pset(&gfx, &mem, 1, 2, 3);
    p8_pset(&gfx, &mem, 1, 2, 3);
    draw_timing_frame_end();
    assert(draw_timing_count() == 1);
    draw_timing_frame_t frame = draw_timing_get(0);
    assert(frame.ticks[DRAW_FILTER_PSET] == 14);
    assert(frame.calls[DRAW_FILTER_PSET] == 2);
    assert(frame.in_call == 14);
    assert(!frame.wrapped);
    timing_removed();
}

static const char TEXT[] = "ab";

static void draw_one_of_each(void) {
    p8_cls(&mem);
    p8_pset(&gfx, &mem, 1, 2, 3);
    p8_line(&gfx, &mem, 1, 2, 3, 4, 5);
    p8_rect(&gfx, &mem, 1, 2, 3, 4, 6);
    p8_rectfill(&gfx, &mem, 1, 2, 3, 4, 7);
    p8_circ(&gfx, &mem, 10, 11, 3, 8);
    p8_circfill(&gfx, &mem, 12, 13, 4, 9);
    p8_spr(&gfx, &mem, 5, 6, 7, 1, 2);
    p8_print(&gfx, &mem, TEXT, 8, 9, 10);
}

static const draw_filter_family_t KIND_FAMILY[KINDS] = {
    DRAW_FILTER_CLS, DRAW_FILTER_PSET, DRAW_FILTER_LINE, DRAW_FILTER_RECT, DRAW_FILTER_RECTFILL,
    DRAW_FILTER_CIRC, DRAW_FILTER_CIRCFILL, DRAW_FILTER_SPR, DRAW_FILTER_PRINT,
};

static void test_every_call_reaches_the_backend_with_its_arguments(void) {
    timing_over(&SLOW);
    draw_one_of_each();
    for (int kind = 0; kind < KINDS; kind++) {
        assert(received[kind].calls == 1);
        assert(received[kind].mem == &mem);
        assert(kind == 0 || received[kind].gfx == &gfx);
    }
    assert(received[1].args[0] == 1 && received[1].args[1] == 2 && received[1].args[2] == 3);
    assert(received[2].args[0] == 1 && received[2].args[3] == 4 && received[2].args[4] == 5);
    assert(received[3].args[4] == 6);
    assert(received[4].args[0] == 1 && received[4].args[3] == 4 && received[4].args[4] == 7);
    assert(received[5].args[0] == 10 && received[5].args[2] == 3 && received[5].args[3] == 8);
    assert(received[6].args[0] == 12 && received[6].args[2] == 4 && received[6].args[3] == 9);
    assert(received[7].args[0] == 5 && received[7].args[3] == 1 && received[7].args[4] == 2);
    assert(received[8].str == TEXT && received[8].args[0] == 8 && received[8].args[2] == 10);
    timing_removed();
}

static void test_each_call_counts_under_its_own_family(void) {
    timing_over(&SLOW);
    draw_one_of_each();
    draw_timing_frame_end();
    draw_timing_frame_t frame = draw_timing_get(0);
    for (int kind = 0; kind < KINDS; kind++) {
        assert(frame.ticks[KIND_FAMILY[kind]] == 10);
        assert(frame.calls[KIND_FAMILY[kind]] == 1);
    }
    assert(frame.ticks[DRAW_FILTER_PATTERN] == 0 && frame.calls[DRAW_FILTER_PATTERN] == 0);
    assert(frame.in_call == 90);
    timing_removed();
}

static void test_an_outlined_print_reaches_the_backend_and_counts_once_under_print(void) {
    timing_over(&SLOW);
    fine_step = 12;
    p8_print_outlined(&gfx, &mem, TEXT, 3, 4, 7);
    draw_timing_frame_end();
    assert(received[KIND_PRINT_OUTLINED].calls == 1 && received[KIND_PRINT_OUTLINED].str == TEXT);
    assert(received[KIND_PRINT_OUTLINED].args[0] == 3 && received[KIND_PRINT_OUTLINED].args[2] == 7);
    draw_timing_frame_t frame = draw_timing_get(0);
    assert(frame.ticks[DRAW_FILTER_PRINT] == 12 && frame.calls[DRAW_FILTER_PRINT] == 1);
    assert(frame.in_call == 12);
    timing_removed();
}

static void test_a_batch_of_lines_is_timed_once_and_counts_one_line_call_per_segment(void) {
    static const p8_segment_t segments[] = { { 1, 2, 3, 4 }, { 5, 6, 7, 8 }, { 9, 10, 11, 12 }, { 13, 14, 15, 16 } };
    timing_over(&SLOW);
    fine_step = 9;
    p8_lines(&gfx, &mem, segments, 4, 7);
    p8_line(&gfx, &mem, 1, 2, 3, 4, 7);
    draw_timing_frame_end();
    assert(received[KIND_LINES].calls == 1 && received[KIND_LINES].args[0] == 4 && received[KIND_LINES].args[1] == 7);
    assert(received[KIND_LINES].args[2] == 1 && received[KIND_LINES].args[3] == 4);
    draw_timing_frame_t frame = draw_timing_get(0);
    assert(frame.ticks[DRAW_FILTER_LINE] == 18 && frame.calls[DRAW_FILTER_LINE] == 5);
    assert(frame.in_call == 18);
    timing_removed();
}

static void test_a_batch_of_points_is_timed_once_and_counts_one_pset_call_per_point(void) {
    static const p8_point_t points[] = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
    timing_over(&SLOW);
    fine_step = 13;
    p8_psets(&gfx, &mem, points, 3);
    p8_pset(&gfx, &mem, 1, 2, 3);
    draw_timing_frame_end();
    assert(received[KIND_PSETS].calls == 1 && received[KIND_PSETS].args[0] == 3 && received[KIND_PSETS].args[3] == 3);
    draw_timing_frame_t frame = draw_timing_get(0);
    assert(frame.ticks[DRAW_FILTER_PSET] == 26 && frame.calls[DRAW_FILTER_PSET] == 4);
    assert(frame.in_call == 26);
    timing_removed();
}

static void test_the_fine_delta_is_taken_modulo_65536(void) {
    timing_over(&SLOW);
    fine_now = 65530;
    fine_step = 20;
    p8_line(&gfx, &mem, 0, 0, 5, 5, 7);
    draw_timing_frame_end();
    assert(draw_timing_get(0).ticks[DRAW_FILTER_LINE] == 20);
    assert(!draw_timing_get(0).wrapped);
    timing_removed();
}

static void test_a_sum_past_16_bits_is_kept(void) {
    timing_over(&SLOW);
    fine_step = 60000;
    for (int i = 0; i < 3; i++) p8_spr(&gfx, &mem, 1, 0, 0, 1, 1);
    draw_timing_frame_end();
    assert(draw_timing_get(0).ticks[DRAW_FILTER_SPR] == 180000);
    assert(draw_timing_get(0).in_call == 180000);
    timing_removed();
}

static void test_a_coarse_delta_at_the_limit_flags_the_frame_wrapped(void) {
    timing_over(&SLOW);
    coarse_step = 199;
    p8_circ(&gfx, &mem, 5, 5, 2, 7);
    draw_timing_frame_end();
    assert(!draw_timing_get(0).wrapped && "199 coarse ticks is under the limit");

    coarse_now = 65500;
    coarse_step = 200;
    p8_circ(&gfx, &mem, 5, 5, 2, 7);
    coarse_step = 1;
    p8_circ(&gfx, &mem, 5, 5, 2, 7);
    draw_timing_frame_end();
    assert(draw_timing_get(1).wrapped && "the flag holds for the frame, across the counter wrap");

    p8_circ(&gfx, &mem, 5, 5, 2, 7);
    draw_timing_frame_end();
    assert(!draw_timing_get(2).wrapped && "the next frame starts unflagged");
    timing_removed();
}

static void test_a_patterned_fill_counts_under_its_shape_and_pattern_outside_the_total(void) {
    timing_over(&SLOW);
    p8_fillp(&gfx, 0xA5A5);
    fine_step = 30;
    p8_rectfill(&gfx, &mem, 0, 0, 5, 5, 7);
    fine_step = 40;
    p8_circfill(&gfx, &mem, 10, 10, 3, 7);
    fine_step = 5;
    p8_rect(&gfx, &mem, 0, 0, 5, 5, 7);
    draw_timing_frame_end();
    draw_timing_frame_t frame = draw_timing_get(0);
    assert(frame.ticks[DRAW_FILTER_RECTFILL] == 30 && frame.calls[DRAW_FILTER_RECTFILL] == 1);
    assert(frame.ticks[DRAW_FILTER_CIRCFILL] == 40 && frame.calls[DRAW_FILTER_CIRCFILL] == 1);
    assert(frame.ticks[DRAW_FILTER_PATTERN] == 70 && frame.calls[DRAW_FILTER_PATTERN] == 2);
    assert(frame.ticks[DRAW_FILTER_RECT] == 5 && "only fills count as pattern");
    assert(frame.in_call == 75);
    timing_removed();
}

static void one_line(p8_graphics_t *build_gfx, p8_vram_t *build_mem, void *context) {
    (void)context;
    p8_line(build_gfx, build_mem, 0, 0, 5, 5, 7);
}

static void test_build_calls_count_under_their_families_and_a_copy_under_spr(void) {
    timing_over(&SLOW);
    image_calls = 0;
    fine_step = 3;
    p8_image_build(&gfx, &mem, 0, 160, 240, one_line, NULL);
    p8_image_draw(&gfx, &mem, 0, 0, 0);
    draw_timing_frame_end();
    assert(image_calls == 5 && "begin, two windows, end and the copy reached the backend");
    draw_timing_frame_t frame = draw_timing_get(0);
    assert(frame.calls[DRAW_FILTER_LINE] == 2 && frame.ticks[DRAW_FILTER_LINE] == 6);
    assert(frame.calls[DRAW_FILTER_SPR] == 1 && frame.ticks[DRAW_FILTER_SPR] == 3);
    assert(frame.in_call == 9 && "the build's own switches are not a family's");
    timing_removed();
}

static void test_frame_end_starts_an_empty_frame(void) {
    timing_over(&SLOW);
    draw_one_of_each();
    draw_timing_frame_end();
    draw_timing_frame_end();
    assert(draw_timing_count() == 2);
    draw_timing_frame_t frame = draw_timing_get(1);
    for (int family = 0; family < DRAW_FILTER_FAMILY_COUNT; family++) {
        assert(frame.ticks[family] == 0 && frame.calls[family] == 0);
    }
    assert(frame.in_call == 0 && !frame.wrapped);
    assert(draw_timing_get(0).in_call == 90);
    timing_removed();
}

static void test_install_empties_the_table(void) {
    timing_over(&SLOW);
    p8_pset(&gfx, &mem, 1, 2, 3);
    draw_timing_frame_end();
    p8_pset(&gfx, &mem, 1, 2, 3);
    timing_removed();
    timing_over(&SLOW);
    assert(draw_timing_count() == 0);
    draw_timing_frame_end();
    assert(draw_timing_get(0).calls[DRAW_FILTER_PSET] == 0 && "a call before the reinstall is dropped");
    timing_removed();
}

static void test_reinstalling_the_returned_backend_removes_the_wrapper(void) {
    timing_over(&SLOW);
    p8_draw_backend_set(&SLOW);
    p8_pset(&gfx, &mem, 1, 2, 3);
    assert(received[1].calls == 1);
    draw_timing_frame_end();
    assert(draw_timing_get(0).calls[DRAW_FILTER_PSET] == 0);
    timing_removed();
}

static void test_a_call_count_past_16_bits_is_stored_as_65535(void) {
    timing_over(&SLOW);
    fine_step = 0;
    for (int i = 0; i < 65537; i++) p8_cls(&mem);
    draw_timing_frame_end();
    assert(draw_timing_get(0).calls[DRAW_FILTER_CLS] == 65535);
    timing_removed();
}

typedef struct {
    char text[4096];
} lines_t;

static void append_line(void *context, const char *line) {
    lines_t *lines = context;
    size_t used = strlen(lines->text);
    assert(used + strlen(line) < sizeof lines->text);
    strcpy(lines->text + used, line);
}

static void test_frame_lines_through_the_sink(void) {
    timing_over(&SLOW);
    draw_one_of_each();
    draw_timing_frame_end();
    p8_fillp(&gfx, 0xA5A5);
    fine_step = 300;
    coarse_step = 250;
    p8_rectfill(&gfx, &mem, 0, 0, 5, 5, 7);
    draw_timing_frame_end();
    lines_t lines = {""};
    state_sink_t sink = { append_line, &lines };
    draw_timing_write(&sink);
    assert(strcmp(lines.text,
                  "frame 0 cls 10 1 pset 10 1 line 10 1 rect 10 1 rectfill 10 1 circ 10 1 circfill 10 1"
                  " spr 10 1 print 10 1 pattern 0 0 wrapped 0\n"
                  "frame 1 cls 0 0 pset 0 0 line 0 0 rect 0 0 rectfill 300 1 circ 0 0 circfill 0 0"
                  " spr 0 0 print 0 0 pattern 300 1 wrapped 1\n") == 0);
    timing_removed();
}

static void test_no_frame_writes_no_line(void) {
    timing_over(&SLOW);
    lines_t lines = {""};
    state_sink_t sink = { append_line, &lines };
    draw_timing_write(&sink);
    assert(lines.text[0] == '\0');
    timing_removed();
}

static void test_calibration_line_through_the_sink(void) {
    lines_t lines = {""};
    state_sink_t sink = { append_line, &lines };
    draw_timing_write_calibration(&sink, 17216);
    assert(strcmp(lines.text, "clk8-per-64-hblanks 17216\n") == 0);
}

static int case_full(void) {
    timing_over(&SLOW);
    for (int i = 0; i < DRAW_TIMING_CAP; i++) draw_timing_frame_end();
    assert(draw_timing_count() == DRAW_TIMING_CAP);
    draw_timing_frame_end();
    return 0;
}

static char *read_whole_file(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    char *text = calloc(1, 4096);
    assert(text != NULL);
    size_t read = fread(text, 1, 4095, file);
    (void)read;
    fclose(file);
    return text;
}

static void expect_fatal(const char *self_path, const char *name, const char *message) {
    char cmd[512];
    remove(STDERR_LOG);
    snprintf(cmd, sizeof cmd, "%s %s > /dev/null 2> " STDERR_LOG, self_path, name);
    int status = system(cmd);
    assert(status != -1 && WIFEXITED(status));
    char *err = read_whole_file(STDERR_LOG);
    if (WEXITSTATUS(status) != 3 || strstr(err, message) == NULL) {
        fprintf(stderr, "%s: expected exit 3 and \"%s\", exit %d\n%s", name, message, WEXITSTATUS(status), err);
        assert(0 && "recording past the capacity must call p8_fatal");
    }
    free(err);
    remove(STDERR_LOG);
}

int main(int argc, char **argv) {
    if (argc == 2) {
        if (strcmp(argv[1], "full") == 0) return case_full();
        fprintf(stderr, "test_draw_timing: unknown case %s\n", argv[1]);
        return 2;
    }
    test_a_call_adds_its_fine_ticks_and_one_call_to_its_family();
    test_every_call_reaches_the_backend_with_its_arguments();
    test_each_call_counts_under_its_own_family();
    test_an_outlined_print_reaches_the_backend_and_counts_once_under_print();
    test_a_batch_of_lines_is_timed_once_and_counts_one_line_call_per_segment();
    test_a_batch_of_points_is_timed_once_and_counts_one_pset_call_per_point();
    test_the_fine_delta_is_taken_modulo_65536();
    test_a_sum_past_16_bits_is_kept();
    test_a_coarse_delta_at_the_limit_flags_the_frame_wrapped();
    test_a_patterned_fill_counts_under_its_shape_and_pattern_outside_the_total();
    test_build_calls_count_under_their_families_and_a_copy_under_spr();
    test_frame_end_starts_an_empty_frame();
    test_install_empties_the_table();
    test_reinstalling_the_returned_backend_removes_the_wrapper();
    test_a_call_count_past_16_bits_is_stored_as_65535();
    test_frame_lines_through_the_sink();
    test_no_frame_writes_no_line();
    test_calibration_line_through_the_sink();
    expect_fatal(argv[0], "full", "draw timing table full at frame 1024, capacity 1024");
    printf("test_draw_timing: ok\n");
    return 0;
}
