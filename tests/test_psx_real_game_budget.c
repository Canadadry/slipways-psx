/* tools/psx_real_game_budget.sh, the real-game judge of `make psx-full-game-check` and
 * `make psx-real-game-budget`, on hand-written frame and situation tables: no emulator. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define DIR "tmp/test_psx_real_game_budget_work"
#define FRAMES DIR "/frames.txt"
#define SITUATIONS DIR "/situations.txt"
#define LOG DIR "/log.txt"
#define FRAMES_HEADER "frame update draw submit total vsyncs\n"
#define SITUATIONS_HEADER "frame situation family\n"

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
    if (file == NULL) return buf;
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static void assert_has(const char *path, const char *text) {
    if (strstr(read_file(path), text) == NULL) {
        fprintf(stderr, "missing \"%s\" in %s:\n%s", text, path, read_file(path));
        assert(0);
    }
}

/* Writes both tables (NULL leaves that table missing) and runs the script with `args` after the two
 * table paths; output in LOG, returns the exit status. */
static int run_args(const char *frames, const char *situations, const char *args) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    if (frames != NULL) write_file(FRAMES, frames);
    if (situations != NULL) write_file(SITUATIONS, situations);
    char command[1024];
    snprintf(command, sizeof command, "sh tools/psx_real_game_budget.sh %s > " LOG " 2>&1", args);
    int status = system(command);
    return WEXITSTATUS(status);
}

/* Budget 263, one-off allowance `allowance`. */
static int run(const char *frames, const char *situations, const char *allowance) {
    char args[256];
    snprintf(args, sizeof args, FRAMES " " SITUATIONS " 263 %s", allowance);
    return run_args(frames, situations, args);
}

/* Frame 1 steady at exactly 263 (not over); frame 2 one-off at 264 (2 vsyncs, its column says 1);
 * frame 3 one-off at 3992 (16 vsyncs). */
#define GREEN_FRAMES FRAMES_HEADER "0 7 41 3 51 0\n1 10 250 3 263 1\n2 10 251 3 264 1\n3 3900 89 3 3992 15\n4 7 30 3 40 0\n"
#define GREEN_SITUATIONS \
    SITUATIONS_HEADER "0 title steady\n1 map steady\n2 build one-off\n3 galaxy-generation one-off\n4 map steady\n"

static void test_a_table_within_both_limits_is_ok_with_both_figures(void) {
    assert(run(GREEN_FRAMES, GREEN_SITUATIONS, "18") == 0);
    assert(strcmp(read_file(LOG), "real-game steady-over 0 frames, worst steady frame -\n"
                                  "real-game one-off-over 2 frames, vsyncs 18 of 18\n"
                                  "real-game-budget ok steady-over 0, one-off vsyncs 18 of 18\n") == 0);
}

static void test_a_table_without_any_frame_over_reads_zero_in_every_figure(void) {
    assert(run(FRAMES_HEADER "0 7 41 3 51 0\n1 7 41 3 51 0\n", SITUATIONS_HEADER "0 title steady\n1 build one-off\n",
               "10800") == 0);
    assert(strcmp(read_file(LOG), "real-game steady-over 0 frames, worst steady frame -\n"
                                  "real-game one-off-over 0 frames, vsyncs 0 of 10800\n"
                                  "real-game-budget ok steady-over 0, one-off vsyncs 0 of 10800\n") == 0);
}

static void test_one_steady_frame_over_fails_naming_the_worst_steady_frame(void) {
    assert(run(FRAMES_HEADER "0 7 41 3 51 0\n1 10 251 3 264 1\n2 10 280 3 293 1\n3 10 300 3 313 1\n",
               SITUATIONS_HEADER "0 title steady\n1 map steady\n2 build one-off\n3 map steady\n", "10800") == 0);
    assert_has(LOG, "real-game steady-over 2 frames, worst steady frame 3 total 313 situation map\n");
    assert_has(LOG, "real-game one-off-over 1 frames, vsyncs 2 of 10800\n");
    assert_has(LOG, "real-game-budget failed 2 steady frames over 263, one-off vsyncs 2 of 10800\n");
}

static void test_a_one_off_sum_one_above_the_allowance_fails(void) {
    assert(run(GREEN_FRAMES, GREEN_SITUATIONS, "17") == 0);
    assert_has(LOG, "real-game one-off-over 2 frames, vsyncs 18 of 17\n");
    assert_has(LOG, "real-game-budget failed one-off vsyncs 18 over 17\n");
}

static void test_both_failing_give_both_reasons_in_one_line(void) {
    assert(run(GREEN_FRAMES, SITUATIONS_HEADER "0 title steady\n1 map steady\n2 build steady\n"
                                               "3 galaxy-generation one-off\n4 map steady\n",
               "15") == 0);
    assert_has(LOG, "real-game steady-over 1 frames, worst steady frame 2 total 264 situation build\n");
    assert_has(LOG, "real-game-budget failed 1 steady frames over 263, one-off vsyncs 16 of 15; "
                    "one-off vsyncs 16 over 15\n");
}

/* A join failure prints `-` for both figures, names the reason and still exits 0. */
static void assert_join_failed(int status, const char *reason) {
    assert(status == 0);
    char expected[512];
    snprintf(expected, sizeof expected,
             "real-game steady-over -\nreal-game one-off-over -\nreal-game-budget failed %s\n", reason);
    if (strcmp(read_file(LOG), expected) != 0) {
        fprintf(stderr, "expected:\n%sgot:\n%s", expected, read_file(LOG));
        assert(0);
    }
}

static void test_join_failures_are_failed_verdicts_naming_the_reason(void) {
    assert_join_failed(run(GREEN_FRAMES, SITUATIONS_HEADER "0 title steady\n1 map steady\n", "18"),
                       "frame count 5 vs 2");
    assert_join_failed(run(FRAMES_HEADER "0 7 41 3 51 0\n2 7 41 3 51 0\n",
                           SITUATIONS_HEADER "0 title steady\n1 map steady\n", "18"),
                       "frame 1: frame table has 2, situation table has 1");
    assert_join_failed(run(FRAMES_HEADER "0 7 41 3 51 0\n1 7 41 3 51 0\n",
                           SITUATIONS_HEADER "0 title steady\n1 map rare\n", "18"),
                       "frame 1: family rare is neither steady nor one-off");
    assert_join_failed(run("frame update draw total\n0 7 41 51\n", GREEN_SITUATIONS, "18"),
                       "frame table header is not \"frame update draw submit total vsyncs\": frame update draw total");
    assert_join_failed(run(GREEN_FRAMES, "frame family\n0 steady\n", "18"),
                       "situation table header is not \"frame situation family\": frame family");
    assert_join_failed(run(GREEN_FRAMES, NULL, "18"), "missing situation table: " SITUATIONS);
    assert_join_failed(run(NULL, GREEN_SITUATIONS, "18"), "missing frame table: " FRAMES);
    assert_join_failed(run(FRAMES_HEADER, SITUATIONS_HEADER, "18"), "no frame line");
}

static void test_usage_errors_exit_1(void) {
    assert(run_args(GREEN_FRAMES, GREEN_SITUATIONS, FRAMES " " SITUATIONS " 263") == 1);
    assert(run_args(GREEN_FRAMES, GREEN_SITUATIONS, FRAMES " " SITUATIONS " 263 lots") == 1);
    assert(run_args(GREEN_FRAMES, GREEN_SITUATIONS, FRAMES " " SITUATIONS " x 18") == 1);
}

int main(void) {
    test_a_table_within_both_limits_is_ok_with_both_figures();
    test_a_table_without_any_frame_over_reads_zero_in_every_figure();
    test_one_steady_frame_over_fails_naming_the_worst_steady_frame();
    test_a_one_off_sum_one_above_the_allowance_fails();
    test_both_failing_give_both_reasons_in_one_line();
    test_join_failures_are_failed_verdicts_naming_the_reason();
    test_usage_errors_exit_1();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_real_game_budget: all tests passed\n");
    return 0;
}
