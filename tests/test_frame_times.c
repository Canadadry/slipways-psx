/* The refusal past capacity runs in a child process: this binary reruns
 * itself with the case name, and the parent reads its exit status and its
 * stderr. */

#include "devtools/frame_times.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define STDERR_LOG "tmp/test_frame_times_stderr.txt"

typedef struct {
    char text[4096];
} lines_t;

static void append_line(void *context, const char *line) {
    lines_t *lines = context;
    size_t used = strlen(lines->text);
    assert(used + strlen(line) < sizeof lines->text);
    strcpy(lines->text + used, line);
}

static void record(uint16_t m0, uint16_t m1, uint16_t m2, uint16_t m3, uint32_t vsyncs) {
    const uint16_t marks[FRAME_TIMES_MARKS] = {m0, m1, m2, m3};
    frame_times_record(marks, vsyncs);
}

static void test_parts_are_mark_deltas_across_the_counter_wrap(void) {
    frame_times_init();
    record(65530, 4, 20, 50, 0);
    frame_time_t frame = frame_times_get(0);
    assert(frame.update == 10);
    assert(frame.draw == 16);
    assert(frame.submit == 30);
    assert(frame.total == 56);

    record(100, 65000, 1000, 1100, 1);
    frame = frame_times_get(1);
    assert(frame.update == 64900);
    assert(frame.draw == 1536);
    assert(frame.submit == 100);
    assert(frame_times_count() == 2);
}

static void test_a_total_past_16_bits_is_the_sum_of_its_parts(void) {
    frame_times_init();
    record(0, 60000, 54464, 48928, 249);
    frame_time_t frame = frame_times_get(0);
    assert(frame.update == 60000);
    assert(frame.draw == 60000);
    assert(frame.submit == 60000);
    assert(frame.total == 180000);
}

static void test_the_vsync_count_is_carried_through(void) {
    frame_times_init();
    record(0, 1, 2, 3, 0);
    record(0, 1, 2, 3, 3);
    record(0, 1, 2, 3, 65535);
    record(0, 1, 2, 3, 70000);
    assert(frame_times_get(0).vsyncs == 0);
    assert(frame_times_get(1).vsyncs == 3);
    assert(frame_times_get(2).vsyncs == 65535);
    assert(frame_times_get(3).vsyncs == 65535);
}

static void test_init_empties_the_table(void) {
    frame_times_init();
    record(0, 1, 2, 3, 0);
    frame_times_init();
    assert(frame_times_count() == 0);
}

static void test_frame_lines_through_the_sink(void) {
    frame_times_init();
    record(65530, 4, 20, 50, 1);
    record(0, 60000, 54464, 48928, 249);
    lines_t lines = {""};
    state_sink_t sink = {append_line, &lines};
    frame_times_write(&sink);
    assert(strcmp(lines.text,
                  "frame 0 update 10 draw 16 submit 30 total 56 vsyncs 1\n"
                  "frame 1 update 60000 draw 60000 submit 60000 total 180000 vsyncs 249\n") == 0);
}

static void test_no_frame_writes_no_line(void) {
    frame_times_init();
    lines_t lines = {""};
    state_sink_t sink = {append_line, &lines};
    frame_times_write(&sink);
    assert(lines.text[0] == '\0');
}

static void test_calibration_line_through_the_sink(void) {
    lines_t lines = {""};
    state_sink_t sink = {append_line, &lines};
    frame_times_write_calibration(&sink, 263);
    assert(strcmp(lines.text, "hblanks-per-vsync 263\n") == 0);
}

static int case_full(void) {
    frame_times_init();
    for (int i = 0; i < FRAME_TIMES_CAP; i++) record(0, 1, 2, 3, 0);
    assert(frame_times_count() == FRAME_TIMES_CAP);
    record(0, 1, 2, 3, 0);
    return 0;
}

static char *read_whole_file(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    char *text = calloc(1, 4096);
    assert(text != NULL);
    fread(text, 1, 4095, file);
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
        fprintf(stderr, "test_frame_times: unknown case %s\n", argv[1]);
        return 2;
    }
    test_parts_are_mark_deltas_across_the_counter_wrap();
    test_a_total_past_16_bits_is_the_sum_of_its_parts();
    test_the_vsync_count_is_carried_through();
    test_init_empties_the_table();
    test_frame_lines_through_the_sink();
    test_no_frame_writes_no_line();
    test_calibration_line_through_the_sink();
    expect_fatal(argv[0], "full", "frame table full at frame 6848, capacity 6848");
    printf("test_frame_times: all tests passed\n");
    return 0;
}
