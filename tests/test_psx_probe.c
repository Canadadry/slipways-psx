/* tools/psx_probe.sh, the analysis of one run of `make psx-probe`, on hand-written baseline and probe TTYs and
 * map: no emulator, no git. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/psx_probe.sh"
#define DIR "tmp/test_psx_probe_work"
#define BASELINE DIR "/baseline.tty.txt"
#define PROBE DIR "/probe.tty.txt"
#define MAP DIR "/map.txt"
#define OUT DIR "/out"
#define REPORT OUT "/probe.txt"
#define LOG DIR "/log.txt"

/* Run frames 15 and 16 are the window; frame 12 is the resume. */
#define FRAMES 17
#define MAP_RESUME "cut 100 first-game-frame 13 source-first 101 window 103 104\n"
#define MAP_PREFIX "cut none first-game-frame 0 source-first 0 window 12 12\n"

#define CALIBRATION "psx-probe: clk8-per-64-hblanks 17152\n"
#define STATE_LINES                            \
    "psx-state: clock year 3422 month 1\n"     \
    "psx-state: zone gpu_frame used 100 of 200\n" \
    "psx-state: budget draw 12\n"              \
    "psx-state: cap planets 30\n"              \
    "psx-state: score 1200\n"

typedef struct {
    int update, draw, submit;
} parts_t;

/* Window (15, 16): update 97, draw 3100, submit 17, total 3214. */
static parts_t baseline_parts(int frame) {
    if (frame < 12) return (parts_t){10, 242, 3};
    if (frame == 12) return (parts_t){1413, 752, 8};
    if (frame == 15) return (parts_t){48, 1600, 8};
    if (frame == 16) return (parts_t){49, 1500, 9};
    return (parts_t){49, 300, 8};
}

/* Window: update 103, draw 3410, submit 18, total 3531; the resume frame's big draw stays out of it. */
static parts_t probe_parts(int frame) {
    parts_t parts = baseline_parts(frame);
    if (frame == 12) parts.draw += 5000;
    if (frame == 15) parts.update += 4, parts.draw += 160, parts.submit += 1;
    if (frame == 16) parts.update += 2, parts.draw += 150;
    return parts;
}

/* A delay of 20 hblanks at the entry of update and of draw: every frame grows by 40. */
static parts_t delayed_parts(int frame) {
    parts_t parts = baseline_parts(frame);
    parts.update += 20;
    parts.draw += 20;
    return parts;
}

static void append(char *buf, size_t cap, const char *text) {
    size_t len = strlen(buf);
    assert(len + strlen(text) < cap);
    memcpy(buf + len, text, strlen(text) + 1);
}

#define TTY_CAP (1 << 14)
static char baseline_tty[TTY_CAP];
static char probe_tty[TTY_CAP];
static char delayed_tty[TTY_CAP];

/* <extra> goes between the calibration line and the done line. */
static void build_tty(char *buf, parts_t (*parts_of)(int), const char *extra) {
    char line[256];
    buf[0] = '\0';
    append(buf, TTY_CAP,
           "Hijacked shell, loading psx-gpu-replay.exe...\r\n"
           "psx-replay: asserts off\r\n"
           "psx-replay: backend gpu\r\n"
           "psx-frame: hblanks-per-vsync 263\r\n");
    for (int frame = 0; frame < FRAMES; frame++) {
        parts_t parts = parts_of(frame);
        int total = parts.update + parts.draw + parts.submit;
        snprintf(line, sizeof line, "psx-frame: frame %d update %d draw %d submit %d total %d vsyncs %d\r\n", frame,
                 parts.update, parts.draw, parts.submit, total, total / 263 + 1);
        append(buf, TTY_CAP, line);
    }
    append(buf, TTY_CAP, CALIBRATION);
    append(buf, TTY_CAP, extra);
    snprintf(line, sizeof line, "psx-replay: done run %d vsyncs 210\r\n", FRAMES);
    append(buf, TTY_CAP, line);
    append(buf, TTY_CAP, STATE_LINES);
}

static void build_ttys(void) {
    build_tty(baseline_tty, baseline_parts, "");
    build_tty(probe_tty, probe_parts, "");
    build_tty(delayed_tty, delayed_parts, "psx-probe: delay 20 calls 34\n");
}

/* Static buffer, overwritten by each call: <text> with its single <from> replaced by <to>. */
static const char *replaced(const char *text, const char *from, const char *to) {
    static char buf[TTY_CAP];
    const char *at = strstr(text, from);
    assert(at != NULL);
    assert(strstr(at + 1, from) == NULL);
    size_t head = (size_t)(at - text);
    assert(strlen(text) - strlen(from) + strlen(to) < sizeof buf);
    memcpy(buf, text, head);
    strcpy(buf + head, to);
    strcat(buf, at + strlen(from));
    return buf;
}

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

/* NULL leaves the TTY out. Returns the script's exit status. */
static int run_map(const char *baseline, const char *probe, const char *map) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    if (baseline != NULL) write_file(BASELINE, baseline);
    if (probe != NULL) write_file(PROBE, probe);
    write_file(MAP, map);
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " start " BASELINE " " PROBE " " MAP " %d " OUT " > " LOG " 2>&1", FRAMES);
    int status = system(cmd);
    assert(WIFEXITED(status));
    return WEXITSTATUS(status);
}

static int run(const char *baseline, const char *probe) {
    return run_map(baseline, probe, MAP_RESUME);
}

/* The report is printed as well as written. */
static void assert_report(const char *expected) {
    assert(strcmp(read_file(REPORT), expected) == 0);
    assert(strcmp(read_file(LOG), expected) == 0);
}

static void test_valid_run_prints_window_sums_and_deltas(void) {
    assert(run(baseline_tty, probe_tty) == 0);
    assert_report("start valid window run frames 15..16 (2 frames)\n"
                  "start draw 3100 -> 3410 +310 hblanks +9.65% of window total\n"
                  "start update 97 -> 103 +6 hblanks +0.19%\n"
                  "start submit 17 -> 18 +1 hblanks +0.03%\n"
                  "start total 3214 -> 3531 +317 hblanks +9.86%\n"
                  "start game-state same\n");
}

/* The resume frame 12 is the whole window: its big draw delta is in. */
static void test_prefix_map_is_the_identity(void) {
    assert(run_map(baseline_tty, probe_tty, MAP_PREFIX) == 0);
    assert_report("start valid window run frames 12..12 (1 frames)\n"
                  "start draw 752 -> 5752 +5000 hblanks +230.10% of window total\n"
                  "start update 1413 -> 1413 +0 hblanks +0.00%\n"
                  "start submit 8 -> 8 +0 hblanks +0.00%\n"
                  "start total 2173 -> 7173 +5000 hblanks +230.10%\n"
                  "start game-state same\n");
}

static const char *line_of(const char *text, const char *prefix) {
    static char line[1024];
    line[0] = '\0';
    for (const char *at = text; *at != '\0';) {
        const char *end = strchr(at, '\n');
        size_t len = end != NULL ? (size_t)(end - at) : strlen(at);
        if (strncmp(at, prefix, strlen(prefix)) == 0) {
            assert(len < sizeof line);
            memcpy(line, at, len);
            line[len] = '\0';
            return line;
        }
        at += len + (end != NULL ? 1 : 0);
    }
    return line;
}

static const char *game_state(void) {
    return line_of(read_file(REPORT), "start game-state ");
}

static void test_differing_game_state_prints_the_first_difference(void) {
    char probe[TTY_CAP];
    strcpy(probe, replaced(probe_tty, "psx-state: clock year 3422 month 1\n", "psx-state: clock year 3422 month 2\n"));
    assert(run(baseline_tty, replaced(probe, "psx-state: score 1200\n", "psx-state: score 1300\n")) == 0);
    assert(strcmp(game_state(),
                  "start game-state differs first difference: clock year 3422 month 1 | clock year 3422 month 2")
           == 0);
    assert(strstr(read_file(REPORT), "start total 3214 -> 3531 ") != NULL);
}

static void test_zone_budget_and_cap_lines_do_not_count(void) {
    char probe[TTY_CAP];
    strcpy(probe, replaced(probe_tty, "psx-state: zone gpu_frame used 100 of 200\n",
                           "psx-state: zone gpu_frame used 180 of 400\n"));
    strcpy(probe, replaced(probe, "psx-state: budget draw 12\n", "psx-state: budget draw 24\n"));
    strcpy(probe, replaced(probe, "psx-state: cap planets 30\n", ""));
    assert(run(baseline_tty, probe) == 0);
    assert(strcmp(game_state(), "start game-state same") == 0);
}

static void test_one_side_shorter_differs(void) {
    assert(run(baseline_tty, replaced(probe_tty, "psx-state: score 1200\n", "")) == 0);
    assert(strcmp(game_state(), "start game-state differs first difference: score 1200 | (end of game-state lines)")
           == 0);
    assert(run(replaced(baseline_tty, "psx-state: score 1200\n", ""), probe_tty) == 0);
    assert(strcmp(game_state(), "start game-state differs first difference: (end of game-state lines) | score 1200")
           == 0);
}

/* 40 hblanks more on each of the 17 frames: 680 / 20 = 34 calls over the run, 80 / 20 / 2 = 2 per window frame. */
static void test_delay_line_divides_the_delta_by_the_delay(void) {
    assert(run(baseline_tty, delayed_tty) == 0);
    assert(strcmp(line_of(read_file(REPORT), "start delay "),
                  "start delay 20 hblanks calls 34 delta/delay run 34.00 (+0.0% vs calls) window per frame 2.00")
           == 0);
    assert(strcmp(line_of(read_file(REPORT), "start total "), "start total 3214 -> 3294 +80 hblanks +2.49%") == 0);
    const char *report = read_file(REPORT);
    assert(strstr(report, "start game-state ") < strstr(report, "start delay "));
}

/* 34 measured against 35 counted calls: -2.9%. */
static void test_delay_line_states_the_disagreement_with_the_counted_calls(void) {
    assert(run(baseline_tty, replaced(delayed_tty, "delay 20 calls 34", "delay 20 calls 35")) == 0);
    assert(strcmp(line_of(read_file(REPORT), "start delay "),
                  "start delay 20 hblanks calls 35 delta/delay run 34.00 (-2.9% vs calls) window per frame 2.00")
           == 0);
}

static void test_no_delay_line_without_the_helper(void) {
    assert(run(baseline_tty, probe_tty) == 0);
    assert(strstr(read_file(REPORT), "delay") == NULL);
}

/* A single invalid line, nothing else: no delta of a half run is ever printed. */
static void assert_invalid(const char *reason) {
    char expected[1024];
    snprintf(expected, sizeof expected, "start invalid %s\n", reason);
    assert_report(expected);
}

static void test_probe_stopped_in_p8_fatal_is_invalid_quoting_the_message(void) {
    char probe[TTY_CAP];
    strcpy(probe, replaced(probe_tty, "psx-replay: done run 17 vsyncs 210\r\n",
                           "psx-state: stopped p8_fatal gpu frame buffer overrun 41000 > 40960\r\n"));
    assert(run(baseline_tty, probe) == 2);
    assert_invalid("probe stopped: psx-state: stopped p8_fatal gpu frame buffer overrun 41000 > 40960");
}

static void test_baseline_stopped_is_invalid_naming_the_baseline(void) {
    assert(run(replaced(baseline_tty, "psx-state: score 1200\n", "psx-state: stopped p8_fatal zone full\n"), probe_tty)
           == 2);
    assert_invalid("baseline stopped: psx-state: stopped p8_fatal zone full");
}

static void test_missing_tty_is_invalid(void) {
    assert(run(baseline_tty, NULL) == 2);
    assert_invalid("probe tty missing: " PROBE);
    assert(run(NULL, probe_tty) == 2);
    assert_invalid("baseline tty missing: " BASELINE);
}

static void test_no_done_line_or_wrong_frame_count_is_invalid(void) {
    assert(run(baseline_tty, replaced(probe_tty, "psx-replay: done run 17 vsyncs 210\r\n", "")) == 2);
    assert_invalid("probe expected 17 frames, got: (no done line)");
    assert(run(baseline_tty, replaced(probe_tty, "done run 17 ", "done run 16 ")) == 2);
    assert_invalid("probe expected 17 frames, got: psx-replay: done run 16 vsyncs 210");
}

static void test_asserts_on_or_no_asserts_line_is_invalid(void) {
    assert(run(replaced(baseline_tty, "psx-replay: asserts off", "psx-replay: asserts on"), probe_tty) == 2);
    assert_invalid("baseline says asserts on");
    assert(run(baseline_tty, replaced(probe_tty, "psx-replay: asserts off\r\n", "")) == 2);
    assert_invalid("probe has no asserts line");
}

static void test_missing_or_malformed_window_frame_line_is_invalid(void) {
    assert(run(baseline_tty, replaced(probe_tty, "psx-frame: frame 16 update 51 draw 1650 submit 9 total 1710 vsyncs 7\r\n",
                                      "")) == 2);
    assert_invalid("probe has no psx-frame line for frame 16");
    assert(run(baseline_tty, replaced(probe_tty, "frame 15 update 52 draw 1760 submit 9 ", "frame 15 update 52 draw submit 9 "))
           == 2);
    assert_invalid("probe malformed frame line: frame 15 update 52 draw submit 9 total 1821 vsyncs 7");
    assert(run(replaced(baseline_tty, "frame 15 update 48 draw 1600 ", "frame 15 update 48 draw 16x0 "), probe_tty) == 2);
    assert_invalid("baseline malformed frame line: frame 15 update 48 draw 16x0 submit 8 total 1656 vsyncs 7");
}

/* Outside the window, a frame line only matters to the delay line's run-wide delta. */
static void test_missing_frame_line_outside_the_window_is_valid_without_delay(void) {
    assert(run(baseline_tty, replaced(probe_tty, "psx-frame: frame 3 update 10 draw 242 submit 3 total 255 vsyncs 1\r\n",
                                      "")) == 0);
    assert(run(baseline_tty, replaced(delayed_tty, "psx-frame: frame 3 update 30 draw 262 submit 3 total 295 vsyncs 2\r\n",
                                      "")) == 2);
    assert_invalid("probe has no psx-frame line for frame 3");
}

static void test_delay_without_calibration_or_bad_calibration_is_invalid(void) {
    assert(run(baseline_tty, replaced(delayed_tty, CALIBRATION, "")) == 2);
    assert_invalid("probe has a delay line but no clk8-per-64-hblanks line");
    assert(run(baseline_tty, replaced(delayed_tty, "clk8-per-64-hblanks 17152", "clk8-per-64-hblanks 15499")) == 2);
    assert_invalid("probe clk8-per-64-hblanks 15499 outside 15500..19000");
    assert(run(replaced(baseline_tty, "clk8-per-64-hblanks 17152", "clk8-per-64-hblanks 19001"), probe_tty) == 2);
    assert_invalid("baseline clk8-per-64-hblanks 19001 outside 15500..19000");
    assert(run(baseline_tty, replaced(delayed_tty, "clk8-per-64-hblanks 17152", "clk8-per-64-hblanks 15500")) == 0);
    assert(run(baseline_tty, replaced(delayed_tty, "clk8-per-64-hblanks 17152", "clk8-per-64-hblanks 19000")) == 0);
    assert(run(baseline_tty, replaced(delayed_tty, "delay 20 calls 34", "delay 0 calls 34")) == 2);
    assert_invalid("probe malformed delay line: psx-probe: delay 0 calls 34");
}

/* The baseline is judged first; the first reason found is the one printed. */
static void test_baseline_reason_comes_before_the_probe_reason(void) {
    assert(run(replaced(baseline_tty, "psx-replay: asserts off", "psx-replay: asserts on"), NULL) == 2);
    assert_invalid("baseline says asserts on");
}

static void test_usage_errors_exit_1(void) {
    assert(run(baseline_tty, probe_tty) == 0);
    assert(system(SCRIPT " start " BASELINE " " PROBE " " MAP " 17 > " LOG " 2>&1") != 0);
    assert(strstr(read_file(LOG), "usage") != NULL);
    int status = system(SCRIPT " start " BASELINE " " PROBE " " DIR "/no-map 17 " OUT " > " LOG " 2>&1");
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 1);
    status = system(SCRIPT " start " BASELINE " " PROBE " " MAP " seventeen " OUT " > " LOG " 2>&1");
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 1);
    assert(run_map(baseline_tty, probe_tty, "cut 100 window 103 104\n") == 1);
    assert(strstr(read_file(LOG), "malformed map") != NULL);
    assert(run_map(baseline_tty, probe_tty, "cut 100 first-game-frame 13 source-first 101 window 103 105\n") == 1);
    assert(strstr(read_file(LOG), "window") != NULL);
}

int main(void) {
    build_ttys();
    test_valid_run_prints_window_sums_and_deltas();
    test_prefix_map_is_the_identity();
    test_differing_game_state_prints_the_first_difference();
    test_zone_budget_and_cap_lines_do_not_count();
    test_one_side_shorter_differs();
    test_delay_line_divides_the_delta_by_the_delay();
    test_delay_line_states_the_disagreement_with_the_counted_calls();
    test_no_delay_line_without_the_helper();
    test_probe_stopped_in_p8_fatal_is_invalid_quoting_the_message();
    test_baseline_stopped_is_invalid_naming_the_baseline();
    test_missing_tty_is_invalid();
    test_no_done_line_or_wrong_frame_count_is_invalid();
    test_asserts_on_or_no_asserts_line_is_invalid();
    test_missing_or_malformed_window_frame_line_is_invalid();
    test_missing_frame_line_outside_the_window_is_valid_without_delay();
    test_delay_without_calibration_or_bad_calibration_is_invalid();
    test_baseline_reason_comes_before_the_probe_reason();
    test_usage_errors_exit_1();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_probe: all tests passed\n");
    return 0;
}
