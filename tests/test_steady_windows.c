/* tools/steady_windows.sh on hand-written frame and situation tables. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/steady_windows.sh"
#define DIR "tmp/test_steady_windows_work"
#define FRAMES DIR "/frames.txt"
#define SITUATIONS DIR "/situations.txt"
#define OUT DIR "/out.txt"

#define FRAMES_HEAD "frame update draw submit total vsyncs\n"
#define SITUATIONS_HEAD "frame situation family\n"

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Static buffer, overwritten by each call; empty when the file is missing. */
static const char *read_file(const char *path) {
    static char buf[1 << 16];
    buf[0] = '\0';
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return buf;
    }
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static int run_command(const char *command) {
    int status = system(command);
    assert(status != -1 && WIFEXITED(status));
    return WEXITSTATUS(status);
}

/* Budget 100, so a frame of total 101 or more is over. */
static int run_args(const char *frames, const char *situations, const char *args) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(FRAMES, frames);
    write_file(SITUATIONS, situations);
    char command[512];
    snprintf(command, sizeof command, SCRIPT " " FRAMES " " SITUATIONS " %s > " OUT " 2>&1", args);
    return run_command(command);
}

static int has_line(const char *line) {
    const char *text = read_file(OUT);
    size_t len = strlen(line);
    for (const char *at = strstr(text, line); at != NULL; at = strstr(at + 1, line)) {
        if ((at == text || at[-1] == '\n') && (at[len] == '\n' || at[len] == '\0')) {
            return 1;
        }
    }
    return 0;
}

static void expect_line(const char *line) {
    if (!has_line(line)) {
        fprintf(stderr, "test_steady_windows: no line '%s' in:\n%s", line, read_file(OUT));
        assert(0);
    }
}

/* Builds both tables from one label and one total per frame; update is 10, draw is total - 10. */
static int run_frames(const char *const *labels, const int *totals, int count, const char *args) {
    static char frames[1 << 14];
    static char situations[1 << 14];
    char *f = frames;
    char *s = situations;
    f += sprintf(f, FRAMES_HEAD);
    s += sprintf(s, SITUATIONS_HEAD);
    for (int frame = 0; frame < count; frame++) {
        const char *family = strcmp(labels[frame], "screen-change") == 0 ? "one-off" : "steady";
        f += sprintf(f, "%d 10 %d 0 %d 1\n", frame, totals[frame] - 10, totals[frame]);
        s += sprintf(s, "%d %s %s\n", frame, labels[frame], family);
    }
    return run_args(frames, situations, args);
}

static void test_the_best_window_of_one_stretch(void) {
    const char *labels[] = {"title", "map-rest", "map-rest", "map-rest", "map-rest", "map-rest", "title"};
    const int totals[] = {50, 40, 150, 150, 50, 50, 50};
    assert(run_frames(labels, totals, 7, "100 3 1") == 0);
    expect_line("candidate map-rest 1 2 4 frames 3 over 2 mean-update 10 mean-draw 106");
    expect_line("situation map-rest frames-over 2 candidates 1");
}

/* The over frames straddle a menu frame: no window holds both sides. */
static void test_a_window_never_crosses_a_label_change(void) {
    const char *labels[] = {"map-rest", "map-rest", "map-rest", "menu", "map-rest", "map-rest", "map-rest"};
    const int totals[] = {50, 50, 150, 150, 150, 50, 50};
    assert(run_frames(labels, totals, 7, "100 3 3") == 0);
    expect_line("candidate map-rest 1 0 2 frames 3 over 1 mean-update 10 mean-draw 73");
    expect_line("candidate map-rest 2 4 6 frames 3 over 1 mean-update 10 mean-draw 73");
    expect_line("situation map-rest frames-over 2 candidates 2");
    expect_line("candidate menu 1 3 3 frames 1 over 1 mean-update 10 mean-draw 140");
}

/* Over first, then summed total, then the earlier first frame. */
static void test_ranking_by_over_then_total_then_first(void) {
    const char *labels[] = {"build", "build", "menu", "build", "build", "menu", "build", "build", "menu",
                            "build", "build"};
    const int totals[] = {101, 101, 20, 200, 50, 20, 300, 50, 20, 200, 50};
    assert(run_frames(labels, totals, 11, "100 2 4") == 0);
    expect_line("candidate build 1 0 1 frames 2 over 2 mean-update 10 mean-draw 91");
    expect_line("candidate build 2 6 7 frames 2 over 1 mean-update 10 mean-draw 165");
    expect_line("candidate build 3 3 4 frames 2 over 1 mean-update 10 mean-draw 115");
    expect_line("candidate build 4 9 10 frames 2 over 1 mean-update 10 mean-draw 115");
}

static void test_candidates_do_not_overlap(void) {
    const char *labels[] = {"map-move", "map-move", "map-move", "map-move", "map-move", "map-move", "map-move"};
    const int totals[] = {50, 150, 150, 150, 50, 50, 150};
    assert(run_frames(labels, totals, 7, "100 3 3") == 0);
    expect_line("candidate map-move 1 1 3 frames 3 over 3 mean-update 10 mean-draw 140");
    expect_line("candidate map-move 2 4 6 frames 3 over 1 mean-update 10 mean-draw 73");
    expect_line("situation map-move frames-over 4 candidates 2");
}

static void test_a_situation_with_no_frame_over_and_one_off_frames_get_no_candidate(void) {
    const char *labels[] = {"title", "title", "screen-change", "screen-change", "map-rest"};
    const int totals[] = {50, 50, 300, 300, 150};
    assert(run_frames(labels, totals, 5, "100 15 3") == 0);
    const char *text = read_file(OUT);
    assert(strstr(text, " title ") == NULL);
    assert(strstr(text, "screen-change") == NULL);
    expect_line("candidate map-rest 1 4 4 frames 1 over 1 mean-update 10 mean-draw 140");
}

static void test_usage_and_join_errors_exit_1(void) {
    assert(run_args(FRAMES_HEAD "0 1 1 1 3 0\n1 1 1 1 3 0\n", SITUATIONS_HEAD "0 title steady\n", "263 15 3") == 1);
    assert(run_args(FRAMES_HEAD "0 1 1 1 3 0\n2 1 1 1 3 0\n", SITUATIONS_HEAD "0 title steady\n1 title steady\n",
                    "263 15 3") == 1);
    assert(run_args("frame update draw\n0 1 1\n", SITUATIONS_HEAD "0 title steady\n", "263 15 3") == 1);
    assert(run_args(FRAMES_HEAD "0 1 1 1 3 0\n", "frame situation\n0 title\n", "263 15 3") == 1);
    assert(run_args(FRAMES_HEAD "0 1 1 1 3 0\n", SITUATIONS_HEAD "0 title steady\n", "263 15") == 1);
    assert(strstr(read_file(OUT), "usage") != NULL);
    assert(run_args(FRAMES_HEAD "0 1 1 1 3 0\n", SITUATIONS_HEAD "0 title steady\n", "263 0 3") == 1);
}

int main(void) {
    test_the_best_window_of_one_stretch();
    test_a_window_never_crosses_a_label_change();
    test_ranking_by_over_then_total_then_first();
    test_candidates_do_not_overlap();
    test_a_situation_with_no_frame_over_and_one_off_frames_get_no_candidate();
    test_usage_and_join_errors_exit_1();
    assert(system("rm -rf " DIR) == 0);
    return 0;
}
