/* tools/psx_frame_budget.sh, the frame analysis of `make psx-full-game-check`,
 * on hand-written TTY and state files: no emulator. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/psx_frame_budget.sh"
#define DIR "tmp/test_psx_frame_budget_work"
#define TTY DIR "/gpu.tty.txt"
#define STATE DIR "/psx_state.txt"
#define OUT DIR "/out"
#define FRAMES_TXT OUT "/frames.txt"
#define SUMMARY OUT "/summary.txt"
#define VERDICTS OUT "/verdicts.txt"
#define LOG DIR "/log.txt"
#define FRAMES "4"
#define BUDGET "263"

#define TTY_HEAD \
    "Hijacked shell, loading psx-gpu-replay.exe...\r\n" \
    "psx-replay: asserts on\r\n" \
    "psx-replay: backend gpu\n"

#define CALIBRATION "psx-frame: hblanks-per-vsync 263\n"

#define FRAME_0 "psx-frame: frame 0 update 10 draw 205 submit 3 total 218 vsyncs 0\n"
#define FRAME_1 "psx-frame: frame 1 update 9 draw 47 submit 2 total 58 vsyncs 1\n"
#define FRAME_2 "psx-frame: frame 2 update 9 draw 210 submit 3 total 222 vsyncs 0\n"
#define FRAME_3 "psx-frame: frame 3 update 10 draw 213 submit 2 total 225 vsyncs 1\n"

#define TTY_TAIL \
    "psx-replay: done full_game " FRAMES " vsyncs 813\n" \
    "psx-state: clock year 3401 month 10\n"

#define STATE_GAME "clock year 3401 month 10\ndifficulty reasonable\n"
#define STATE_ZONES \
    "zone game 52096 52912 466616\n" \
    "zone ui 27936 52854 208080\n" \
    "budget zones 793195 794584\n" \
    "zone gpu_frame 10388 25328 73728\n" \
    "zone gpu_build 0 8180 9216\n"
#define STATE_CAPS \
    "cap button 4 8 15\n" \
    "cap modal_backdrop 0 0 2\n" \
    "cap entity_table 107 114 512\n" \
    "cap particles_help_screen 0 1 1\n" \
    "cap bytes_row - 336 1184\n" \
    "cap gpu_cluts - 4 6\n"
#define STATE_ALL STATE_GAME STATE_ZONES STATE_CAPS "cheat activations 0\n"

#define TTY_OK TTY_HEAD CALIBRATION FRAME_0 FRAME_1 FRAME_2 FRAME_3 TTY_TAIL

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

static int run(const char *tty, const char *state, const char *frames) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(TTY, tty);
    write_file(STATE, state);
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " " TTY " " STATE " %s " BUDGET " " OUT " > " LOG " 2>&1", frames);
    return system(cmd);
}

/* Static buffer: the verdicts.txt line of <condition>, empty when missing. */
static const char *verdict(const char *condition) {
    static char line[1024];
    line[0] = '\0';
    const char *text = read_file(VERDICTS);
    size_t len = strlen(condition);
    for (const char *at = text; *at != '\0';) {
        const char *end = strchr(at, '\n');
        size_t line_len = end != NULL ? (size_t)(end - at) : strlen(at);
        if (line_len > len && strncmp(at, condition, len) == 0 && at[len] == ' ') {
            assert(line_len < sizeof line);
            memcpy(line, at, line_len);
            line[line_len] = '\0';
            return line;
        }
        at += line_len + (end != NULL ? 1 : 0);
    }
    return line;
}

static int verdict_ok(const char *condition) {
    const char *line = verdict(condition);
    size_t len = strlen(condition);
    return strncmp(line + len, " ok", 3) == 0 && (line[len + 3] == '\0' || line[len + 3] == ' ');
}

static int verdict_failed(const char *condition, const char *reason_part) {
    const char *line = verdict(condition);
    size_t len = strlen(condition);
    return strncmp(line + len, " failed ", 8) == 0 && strstr(line, reason_part) != NULL;
}

static void test_every_verdict_ok_on_a_clean_run(void) {
    assert(run(TTY_OK, STATE_ALL, FRAMES) == 0);
    assert(verdict_ok("timer"));
    assert(verdict_ok("frame-budget"));
    assert(verdict_ok("zone-sizes"));
    assert(verdict_ok("cap-sizes"));
    const char *text = read_file(VERDICTS);
    assert(strncmp(text, "timer ", 6) == 0);
    assert(strstr(text, "frame-budget ") < strstr(text, "zone-sizes "));
    assert(strstr(text, "zone-sizes ") < strstr(text, "cap-sizes "));
}


/* Every other condition is still judged when one fails. */
static void assert_only_failed(const char *condition) {
    const char *conditions[] = {"timer", "frame-budget", "zone-sizes", "cap-sizes"};
    for (size_t i = 0; i < sizeof conditions / sizeof conditions[0]; i++) {
        if (strcmp(conditions[i], condition) != 0) {
            assert(verdict_ok(conditions[i]));
        }
    }
}

static void test_missing_calibration_fails_timer(void) {
    assert(run(TTY_HEAD FRAME_0 FRAME_1 FRAME_2 FRAME_3 TTY_TAIL, STATE_ALL, FRAMES) == 0);
    assert(verdict_failed("timer", "no hblanks-per-vsync line"));
    assert_only_failed("timer");
}

/* A timer clocked by the wrong source counts plausible figures, never 255..271 a vsync. */
static void test_calibration_out_of_range_fails_timer_naming_it(void) {
    const char *tty = TTY_HEAD "psx-frame: hblanks-per-vsync 272\n" FRAME_0 FRAME_1 FRAME_2 FRAME_3 TTY_TAIL;
    assert(run(tty, STATE_ALL, FRAMES) == 0);
    assert(verdict_failed("timer", "hblanks-per-vsync 272"));
    assert_only_failed("timer");
    assert(run(TTY_HEAD "psx-frame: hblanks-per-vsync 254\n" FRAME_0 FRAME_1 FRAME_2 FRAME_3 TTY_TAIL,
               STATE_ALL, FRAMES) == 0);
    assert(verdict_failed("timer", "hblanks-per-vsync 254"));
    assert(run(TTY_HEAD "psx-frame: hblanks-per-vsync 255\n" FRAME_0 FRAME_1 FRAME_2 FRAME_3 TTY_TAIL,
               STATE_ALL, FRAMES) == 0);
    assert(verdict_ok("timer"));
}

static void test_missing_frame_line_fails_timer_naming_it(void) {
    assert(run(TTY_HEAD CALIBRATION FRAME_0 FRAME_2 FRAME_3 TTY_TAIL, STATE_ALL, FRAMES) == 0);
    assert(verdict_failed("timer", "expected frame 1, got frame 2"));
    assert_only_failed("timer");
}

static void test_frames_out_of_order_fail_timer(void) {
    assert(run(TTY_HEAD CALIBRATION FRAME_0 FRAME_2 FRAME_1 FRAME_3 TTY_TAIL, STATE_ALL, FRAMES) == 0);
    assert(verdict_failed("timer", "expected frame 1, got frame 2"));
}

/* A replay cut short writes fewer frame lines than the recording has frames. */
static void test_missing_trailing_frames_fail_timer(void) {
    assert(run(TTY_HEAD CALIBRATION FRAME_0 FRAME_1 TTY_TAIL, STATE_ALL, FRAMES) == 0);
    assert(verdict_failed("timer", "expected frame 2, got end of frames"));
}

static void test_extra_frame_fails_timer(void) {
    assert(run(TTY_OK, STATE_ALL, "3") == 0);
    assert(verdict_failed("timer", "expected 3 frames, got frame 3"));
}

/* 249 vsyncs is past the 16-bit timer's ~4.2 s: the parts may have wrapped. */
static void test_frame_spanning_249_vsyncs_fails_timer(void) {
    const char *tty = TTY_HEAD CALIBRATION FRAME_0 FRAME_1
        "psx-frame: frame 2 update 9 draw 110 submit 3 total 122 vsyncs 249\n" FRAME_3 TTY_TAIL;
    assert(run(tty, STATE_ALL, FRAMES) == 0);
    assert(verdict_failed("timer", "frame 2 spans 249 vsyncs"));
    const char *long_frame = TTY_HEAD CALIBRATION FRAME_0 FRAME_1
        "psx-frame: frame 2 update 9 draw 110 submit 3 total 122 vsyncs 248\n" FRAME_3 TTY_TAIL;
    assert(run(long_frame, STATE_ALL, FRAMES) == 0);
    assert(verdict_ok("timer"));
}

static void test_one_frame_over_budget_fails_frame_budget_naming_the_worst(void) {
    const char *tty = TTY_HEAD CALIBRATION FRAME_0 FRAME_1
        "psx-frame: frame 2 update 9 draw 250 submit 5 total 264 vsyncs 1\n" FRAME_3 TTY_TAIL;
    assert(run(tty, STATE_ALL, FRAMES) == 0);
    assert(verdict_failed("frame-budget", "1 frames over 263 hblanks, worst frame 2 total 264"));
    assert_only_failed("frame-budget");
    const char *at_budget = TTY_HEAD CALIBRATION FRAME_0 FRAME_1
        "psx-frame: frame 2 update 9 draw 249 submit 5 total 263 vsyncs 1\n" FRAME_3 TTY_TAIL;
    assert(run(at_budget, STATE_ALL, FRAMES) == 0);
    assert(verdict_ok("frame-budget"));
}

static void test_zone_over_its_size_fails_zone_sizes_naming_it(void) {
    const char *state = STATE_GAME
        "zone game 52096 52912 466616\n"
        "zone ui 27936 208081 208080\n"
        "budget zones 793195 794584\n"
        "zone gpu_frame 10388 25328 73728\n" STATE_CAPS;
    assert(run(TTY_OK, state, FRAMES) == 0);
    assert(verdict_failed("zone-sizes", "zone ui peak 208081 limit 208080"));
    assert(strstr(verdict("zone-sizes"), "zone game") == NULL);
    assert_only_failed("zone-sizes");
}

/* zone gpu_frame is the GPU primitive buffer against GPU_FRAME_BUFFER_BYTES. */
static void test_primitive_buffer_over_its_size_fails_zone_sizes(void) {
    const char *state = STATE_GAME
        "zone game 52096 52912 466616\n"
        "budget zones 793195 794584\n"
        "zone gpu_frame 10388 73729 73728\n" STATE_CAPS;
    assert(run(TTY_OK, state, FRAMES) == 0);
    assert(verdict_failed("zone-sizes", "zone gpu_frame peak 73729 limit 73728"));
}

static void test_build_buffer_over_its_size_fails_zone_sizes(void) {
    const char *state = STATE_GAME
        "zone game 52096 52912 466616\n"
        "budget zones 793195 794584\n"
        "zone gpu_frame 10388 25328 73728\n"
        "zone gpu_build 0 9217 9216\n" STATE_CAPS;
    assert(run(TTY_OK, state, FRAMES) == 0);
    assert(verdict_failed("zone-sizes", "zone gpu_build peak 9217 limit 9216"));
}

static void test_budget_over_its_total_fails_zone_sizes(void) {
    const char *state = STATE_GAME
        "zone game 52096 52912 466616\n"
        "budget zones 794585 794584\n" STATE_CAPS;
    assert(run(TTY_OK, state, FRAMES) == 0);
    assert(verdict_failed("zone-sizes", "budget zones used 794585 total 794584"));
    assert_only_failed("zone-sizes");
}

static void test_state_without_budget_line_fails_zone_sizes(void) {
    const char *state = STATE_GAME "zone game 52096 52912 466616\n" STATE_CAPS;
    assert(run(TTY_OK, state, FRAMES) == 0);
    assert(verdict_failed("zone-sizes", "no budget zones line"));
}

static void test_cap_over_its_size_fails_cap_sizes_naming_it(void) {
    const char *state = STATE_GAME STATE_ZONES
        "cap button 4 16 15\n"
        "cap entity_table 107 114 512\n"
        "cap bytes_row - 1185 1184\n";
    assert(run(TTY_OK, state, FRAMES) == 0);
    assert(verdict_failed("cap-sizes", "cap button peak 16 limit 15"));
    assert(verdict_failed("cap-sizes", "cap bytes_row peak 1185 limit 1184"));
    assert(strstr(verdict("cap-sizes"), "entity_table") == NULL);
    assert_only_failed("cap-sizes");
}

/* A state without its measured lines is not a state that fits. */
static void test_state_without_zone_or_cap_lines_fails_both(void) {
    assert(run(TTY_OK, STATE_GAME, FRAMES) == 0);
    assert(verdict_failed("zone-sizes", "no zone line"));
    assert(verdict_failed("cap-sizes", "no cap line"));
}

#define FRAME_OVER_2 "psx-frame: frame 2 update 71 draw 455 submit 8 total 534 vsyncs 2\n"
#define FRAME_OVER_3 "psx-frame: frame 3 update 20 draw 250 submit 3 total 273 vsyncs 1\n"
#define FRAME_OVER_4 "psx-frame: frame 4 update 30 draw 300 submit 10 total 340 vsyncs 1\n"
#define FRAME_OVER_5 "psx-frame: frame 5 update 50 draw 350 submit 50 total 450 vsyncs 2\n"
#define FRAME_OVER_6 "psx-frame: frame 6 update 100 draw 900 submit 20 total 1020 vsyncs 4\n"
#define FRAME_7 "psx-frame: frame 7 update 5 draw 100 submit 1 total 106 vsyncs 0\n"

/* Overruns 271, 10, 77, 187 and 757: one in each bucket. */
static void test_summary_figures_buckets_and_dominant_part(void) {
    const char *tty = TTY_HEAD CALIBRATION FRAME_0 FRAME_1 FRAME_OVER_2 FRAME_OVER_3 FRAME_OVER_4
        FRAME_OVER_5 FRAME_OVER_6 FRAME_7 TTY_TAIL;
    assert(run(tty, STATE_ALL, "8") == 0);
    assert(verdict_failed("frame-budget", "5 frames over 263 hblanks, worst frame 6 total 1020"));
    const char *summary = read_file(SUMMARY);
    assert(strstr(summary, "frames 8 budget 263 hblanks\n") != NULL);
    assert(strstr(summary, "over 5 frames (625‰), worst frame 6 total 1020 (+757) update 100 draw 900 submit 20\n") != NULL);
    assert(strstr(summary, "overrun mean 260 hblanks over the over-budget frames\n") != NULL);
    assert(strstr(summary, "overrun distribution <=1/4 vsync 1 <=1/2 1 <=1 1 <=2 1 >2 1\n") != NULL);
    assert(strstr(summary, "parts over-budget frames update 271 draw 2255 submit 91 dominant draw\n") != NULL);
    assert(strstr(summary, "parts all frames update 295 draw 2607 submit 97 mean total 374\n") != NULL);
}

/* An overrun of exactly one or two vsyncs falls in the lower bucket. */
static void test_overrun_bucket_boundaries(void) {
    const char *tty = TTY_HEAD CALIBRATION
        "psx-frame: frame 0 update 10 draw 506 submit 10 total 526 vsyncs 2\n"
        "psx-frame: frame 1 update 10 draw 769 submit 10 total 789 vsyncs 3\n"
        "psx-frame: frame 2 update 10 draw 770 submit 10 total 790 vsyncs 3\n" TTY_TAIL;
    assert(run(tty, STATE_ALL, "3") == 0);
    assert(strstr(read_file(SUMMARY), "overrun distribution <=1/4 vsync 0 <=1/2 0 <=1 1 <=2 1 >2 1\n") != NULL);
}

static void test_summary_names_submit_when_it_dominates(void) {
    const char *tty = TTY_HEAD CALIBRATION FRAME_0
        "psx-frame: frame 1 update 9 draw 47 submit 400 total 456 vsyncs 2\n" TTY_TAIL;
    assert(run(tty, STATE_ALL, "2") == 0);
    assert(strstr(read_file(SUMMARY), "dominant submit\n") != NULL);
}

static void test_summary_without_over_budget_frame(void) {
    assert(run(TTY_OK, STATE_ALL, FRAMES) == 0);
    const char *summary = read_file(SUMMARY);
    assert(strstr(summary, "over 0 frames (0‰), worst frame 3 total 225 (-38) update 10 draw 213 submit 2\n") != NULL);
    assert(strstr(summary, "overrun mean 0 hblanks") != NULL);
    assert(strstr(summary, "overrun distribution <=1/4 vsync 0 <=1/2 0 <=1 0 <=2 0 >2 0\n") != NULL);
    assert(strstr(summary, "parts over-budget frames update 0 draw 0 submit 0 dominant -\n") != NULL);
}

/* Margins in eighths are information for re-sizing, not a verdict. */
static void test_summary_lists_every_zone_budget_and_cap_with_margin(void) {
    const char *state = STATE_GAME
        "zone game 52096 52912 466616\n"
        "zone ui 27936 208081 208080\n"
        "budget zones 793195 794584\n"
        "cap toast 0 3 48\n"
        "cap button 4 16 15\n";
    assert(run(TTY_OK, state, FRAMES) == 0);
    const char *summary = read_file(SUMMARY);
    assert(strstr(summary, "zone game peak 52912 limit 466616 ok margin 7/8\n") != NULL);
    assert(strstr(summary, "zone ui peak 208081 limit 208080 over by 1\n") != NULL);
    assert(strstr(summary, "budget zones used 793195 total 794584 ok margin 0/8\n") != NULL);
    assert(strstr(summary, "cap toast peak 3 limit 48 ok margin 7/8\n") != NULL);
    assert(strstr(summary, "cap button peak 16 limit 15 over by 1\n") != NULL);
}

static void test_frames_file_has_a_header_then_one_line_a_frame(void) {
    assert(run(TTY_OK, STATE_ALL, FRAMES) == 0);
    assert(strcmp(read_file(FRAMES_TXT),
                  "frame update draw submit total vsyncs\n"
                  "0 10 205 3 218 0\n"
                  "1 9 47 2 58 1\n"
                  "2 9 210 3 222 0\n"
                  "3 10 213 2 225 1\n") == 0);
}

static void test_usage_errors_exit_1(void) {
    assert(run(TTY_OK, STATE_ALL, FRAMES) == 0);
    assert(system(SCRIPT " " TTY " " STATE " 4 263 > " LOG " 2>&1") != 0);
    assert(strstr(read_file(LOG), "usage") != NULL);
    assert(system(SCRIPT " " DIR "/missing " STATE " 4 263 " OUT " > " LOG " 2>&1") != 0);
    assert(system(SCRIPT " " TTY " " STATE " four 263 " OUT " > " LOG " 2>&1") != 0);
    assert(system(SCRIPT " " TTY " " STATE " 4 -1 " OUT " > " LOG " 2>&1") != 0);
}

int main(void) {
    test_every_verdict_ok_on_a_clean_run();
    test_missing_calibration_fails_timer();
    test_calibration_out_of_range_fails_timer_naming_it();
    test_missing_frame_line_fails_timer_naming_it();
    test_frames_out_of_order_fail_timer();
    test_missing_trailing_frames_fail_timer();
    test_extra_frame_fails_timer();
    test_frame_spanning_249_vsyncs_fails_timer();
    test_one_frame_over_budget_fails_frame_budget_naming_the_worst();
    test_zone_over_its_size_fails_zone_sizes_naming_it();
    test_primitive_buffer_over_its_size_fails_zone_sizes();
    test_build_buffer_over_its_size_fails_zone_sizes();
    test_budget_over_its_total_fails_zone_sizes();
    test_state_without_budget_line_fails_zone_sizes();
    test_cap_over_its_size_fails_cap_sizes_naming_it();
    test_state_without_zone_or_cap_lines_fails_both();
    test_summary_figures_buckets_and_dominant_part();
    test_overrun_bucket_boundaries();
    test_summary_names_submit_when_it_dominates();
    test_summary_without_over_budget_frame();
    test_summary_lists_every_zone_budget_and_cap_with_margin();
    test_frames_file_has_a_header_then_one_line_a_frame();
    test_usage_errors_exit_1();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_frame_budget: all tests passed\n");
    return 0;
}
