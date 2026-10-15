/* tools/psx_short_run.sh, the analysis of one run of `make psx-short-runs`, on hand-written TTYs,
 * map and reference frame table: no emulator. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/psx_short_run.sh"
#define DIR "tmp/test_psx_short_run_work"
#define PLAIN_1 DIR "/plain-1.tty.txt"
#define PLAIN_2 DIR "/plain-2.tty.txt"
#define BREAKDOWN DIR "/breakdown.tty.txt"
#define MAP DIR "/map.txt"
#define REFERENCE DIR "/frames.txt"
#define OUT DIR "/out"
#define VERDICTS OUT "/verdicts.txt"
#define SUMMARY OUT "/summary.txt"
#define WINDOW OUT "/window.txt"
#define LOG DIR "/log.txt"
#define BUDGET "263"

/* Run frames 15 and 16 are full frames 103 and 104; frame 12 is the resume. */
#define FRAMES 17
#define MAP_RESUME "cut 100 first-game-frame 13 source-first 101 window 103 104\n"

#define TTY_HEAD \
    "Hijacked shell, loading psx-gpu-replay.exe...\r\n" \
    "psx-replay: asserts on\r\n" \
    "psx-replay: backend gpu\n" \
    "psx-frame: hblanks-per-vsync 263\n"

/* 268 fine ticks an hblank: every tick figure below is a whole number of hblanks. */
#define CALIBRATION_OK "psx-draw: clk8-per-64-hblanks 17152\n"
#define TICKS(hblanks) ((hblanks) * 268)

#define REFERENCE_OK \
    "frame update draw submit total vsyncs\n" \
    "102 1057 1215 7 2279 9\n" \
    "103 48 1600 8 1656 6\n" \
    "104 49 1500 9 1558 6\n" \
    "105 55 1212 7 1274 5\n"

typedef struct {
    int update, draw, submit;
} parts_t;

static parts_t plain_parts(int frame) {
    if (frame < 12) return (parts_t){10, 242, 3};
    if (frame == 12) return (parts_t){1413, 752, 8};
    if (frame == 15) return (parts_t){48, 1600, 8};
    if (frame == 16) return (parts_t){49, 1500, 9};
    return (parts_t){49, 300, 8};
}

/* The breakdown boot's own draw part: the plain one plus the timing overhead. */
static int breakdown_draw(int frame) {
    if (frame == 15) return 1640;
    if (frame == 16) return 1520;
    return plain_parts(frame).draw + 10;
}

static void append(char *buf, size_t cap, const char *text) {
    size_t len = strlen(buf);
    assert(len + strlen(text) < cap);
    memcpy(buf + len, text, strlen(text) + 1);
}

static void append_frame_lines(char *buf, size_t cap, int breakdown) {
    char line[256];
    for (int frame = 0; frame < FRAMES; frame++) {
        parts_t parts = plain_parts(frame);
        if (breakdown) parts.draw = breakdown_draw(frame);
        int total = parts.update + parts.draw + parts.submit;
        snprintf(line, sizeof line, "psx-frame: frame %d update %d draw %d submit %d total %d vsyncs %d\n", frame,
                 parts.update, parts.draw, parts.submit, total, total / 263 + 1);
        append(buf, cap, line);
    }
}

/* In calls, cls (52 ticks) and spr together: frame 15 spr 750, print 300, line 180, rectfill 20
 * (all patterned), 1250 hblanks; frame 16 spr 500, print 260, 760 hblanks; elsewhere spr 100. */
static void append_draw_lines(char *buf, size_t cap) {
    char line[512];
    for (int frame = 0; frame < FRAMES; frame++) {
        int spr = 100, print = 0, line_ticks = 0, rectfill = 0, pattern = 0;
        if (frame == 15) spr = 750, print = 300, line_ticks = 180, rectfill = 20, pattern = 20;
        if (frame == 16) spr = 500, print = 260;
        snprintf(line, sizeof line,
                 "psx-draw: frame %d cls 52 1 pset 0 0 line %d %d rect 0 0 rectfill %d %d circ 0 0 "
                 "circfill 0 0 spr %d %d print %d %d pattern %d %d wrapped 0\n",
                 frame, TICKS(line_ticks), line_ticks > 0 ? 120 : 0, TICKS(rectfill), rectfill > 0 ? 2 : 0,
                 TICKS(spr) - 52, 481, TICKS(print), print > 0 ? 63 : 0, TICKS(pattern), pattern > 0 ? 2 : 0);
        append(buf, cap, line);
    }
}

#define TTY_CAP (1 << 14)
static char plain_tty[TTY_CAP];
static char breakdown_tty[TTY_CAP];

static void build_ttys(void) {
    char done[64];
    snprintf(done, sizeof done, "psx-replay: done run %d vsyncs 210\n", FRAMES);
    plain_tty[0] = '\0';
    append(plain_tty, TTY_CAP, TTY_HEAD);
    append_frame_lines(plain_tty, TTY_CAP, 0);
    append(plain_tty, TTY_CAP, done);
    append(plain_tty, TTY_CAP, "psx-state: clock year 3422 month 1\n");
    append(plain_tty, TTY_CAP, "psx-state: cheat activations 0\n");
    breakdown_tty[0] = '\0';
    append(breakdown_tty, TTY_CAP, TTY_HEAD);
    append_frame_lines(breakdown_tty, TTY_CAP, 1);
    append(breakdown_tty, TTY_CAP, CALIBRATION_OK);
    append_draw_lines(breakdown_tty, TTY_CAP);
    append(breakdown_tty, TTY_CAP, done);
    append(breakdown_tty, TTY_CAP, "psx-state: clock year 3422 month 1\n");
    append(breakdown_tty, TTY_CAP, "psx-state: cheat activations 0\n");
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

/* NULL leaves the file out. <reference_arg> and <asserts> are passed as is, quoted. Returns the script's exit
 * status. */
static int run_asserts(const char *plain_1, const char *plain_2, const char *breakdown, const char *map,
                       const char *reference_arg, const char *reference, const char *asserts) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    if (plain_1 != NULL) write_file(PLAIN_1, plain_1);
    if (plain_2 != NULL) write_file(PLAIN_2, plain_2);
    if (breakdown != NULL) write_file(BREAKDOWN, breakdown);
    if (reference != NULL) write_file(REFERENCE, reference);
    write_file(MAP, map);
    char cmd[1024];
    snprintf(cmd, sizeof cmd,
             SCRIPT " " PLAIN_1 " " PLAIN_2 " " BREAKDOWN " " MAP " %d " BUDGET " '%s' " OUT " '%s' > " LOG " 2>&1",
             FRAMES, reference_arg, asserts);
    int status = system(cmd);
    assert(WIFEXITED(status));
    return WEXITSTATUS(status);
}

static int run_with(const char *plain_1, const char *plain_2, const char *breakdown, const char *map,
                    const char *reference_arg, const char *reference) {
    return run_asserts(plain_1, plain_2, breakdown, map, reference_arg, reference, "on");
}

static int run_reference(const char *plain_1, const char *plain_2, const char *breakdown, const char *reference) {
    return run_with(plain_1, plain_2, breakdown, MAP_RESUME, REFERENCE, reference);
}

static int run(const char *plain_1, const char *plain_2, const char *breakdown) {
    return run_reference(plain_1, plain_2, breakdown, REFERENCE_OK);
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

static const char *const conditions[] = {"done", "deterministic", "representative", "breakdown", "asserts"};

/* Every other condition is still judged when one fails. */
static void assert_only_failed(const char *condition) {
    for (size_t i = 0; i < sizeof conditions / sizeof conditions[0]; i++) {
        if (strcmp(conditions[i], condition) != 0) {
            assert(verdict_ok(conditions[i]));
        }
    }
}

static void test_every_verdict_ok_on_a_clean_run(void) {
    assert(run(plain_tty, plain_tty, breakdown_tty) == 0);
    for (size_t i = 0; i < sizeof conditions / sizeof conditions[0]; i++) {
        assert(verdict_ok(conditions[i]));
    }
    const char *text = read_file(VERDICTS);
    assert(strncmp(text, "done ", 5) == 0);
    assert(strstr(text, "deterministic ") < strstr(text, "representative "));
    assert(strstr(text, "representative ") < strstr(text, "breakdown "));
    assert(strstr(text, "breakdown ") < strstr(text, "asserts "));
    assert(strcmp(verdict("asserts"), "asserts ok on in 3 boots") == 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "done ok") != NULL);
    assert(strstr(log, "breakdown ok") != NULL);
}

static void test_missing_done_line_fails_done_naming_the_boot(void) {
    const char *plain_2 = replaced(plain_tty, "psx-replay: done run 17 vsyncs 210\n", "");
    assert(run(plain_tty, plain_2, breakdown_tty) == 2);
    assert(verdict_failed("done", "plain-2 expected 17 frames, got: (no done line)"));
    assert(strstr(verdict("done"), "plain-1") == NULL);
    assert_only_failed("done");
}

static void test_wrong_frame_count_or_fatal_fails_done(void) {
    const char *short_run = replaced(breakdown_tty, "done run 17 ", "done run 16 ");
    assert(run(plain_tty, plain_tty, short_run) == 2);
    assert(verdict_failed("done", "breakdown expected 17 frames, got: psx-replay: done run 16 vsyncs 210"));
    const char *stopped = replaced(plain_tty, "psx-state: clock year 3422 month 1\n",
                                   "psx-state: stopped p8_fatal zone full\n");
    assert(run(stopped, plain_tty, breakdown_tty) == 2);
    assert(verdict_failed("done", "plain-1 stopped: stopped p8_fatal zone full"));
}

static void test_a_state_dump_cut_before_its_last_line_fails_done_naming_the_boot(void) {
    const char *cut = replaced(breakdown_tty, "psx-state: cheat activations 0\n", "");
    assert(run(plain_tty, plain_tty, cut) == 2);
    assert(strcmp(verdict("done"),
                  "done failed breakdown state dump incomplete (no cheat activations line after the done line)") == 0);
    assert_only_failed("done");
}

static void test_a_cheat_activations_line_before_the_done_line_does_not_complete_the_dump(void) {
    char early[TTY_CAP];
    snprintf(early, sizeof early, "psx-state: cheat activations 0\n%s",
             replaced(plain_tty, "psx-state: cheat activations 0\n", ""));
    assert(run(plain_tty, early, breakdown_tty) == 2);
    assert(verdict_failed("done", "plain-2 state dump incomplete"));
}

/* With one boot only, the missing ones are named: a single TTY never passes for two. */
static void test_missing_tty_fails_done_and_deterministic(void) {
    assert(run(plain_tty, NULL, breakdown_tty) == 2);
    assert(verdict_failed("done", "plain-2 tty missing"));
    assert(verdict_failed("deterministic", "plain-2 tty missing"));
    assert(verdict_ok("breakdown"));
    assert(verdict_ok("representative"));
}

static void test_one_differing_frame_line_fails_deterministic_naming_it(void) {
    const char *plain_2 = replaced(plain_tty, "frame 5 update 10 draw 242 ", "frame 5 update 10 draw 243 ");
    assert(run(plain_tty, plain_2, breakdown_tty) == 2);
    assert(verdict_failed("deterministic", "first difference: frame 5 update 10 draw 242"));
    assert(verdict_failed("deterministic", "frame 5 update 10 draw 243"));
    assert_only_failed("deterministic");
    const char *calibration = replaced(plain_tty, "hblanks-per-vsync 263", "hblanks-per-vsync 262");
    assert(run(plain_tty, calibration, breakdown_tty) == 2);
    assert(verdict_failed("deterministic", "hblanks-per-vsync 262"));
}

static void test_a_missing_frame_line_fails_deterministic(void) {
    const char *plain_2 = replaced(plain_tty,
                                   "psx-frame: frame 16 update 49 draw 1500 submit 9 total 1558 vsyncs 6\n", "");
    assert(run(plain_tty, plain_2, breakdown_tty) == 2);
    assert(verdict_failed("deterministic", "(end of frame lines)"));
}

/* Plain window draw 1600 + 1500 = 3100, update 48 + 49 = 97, against the reference's sums. */
static void test_representative_within_ten_percent_of_the_reference_sums(void) {
    assert(run(plain_tty, plain_tty, breakdown_tty) == 0);
    assert(strstr(verdict("representative"), "draw 3100 vs 3100 (+0.0%)") != NULL);
    assert(strstr(verdict("representative"), "update 97 vs 97 (+0.0%)") != NULL);
    const char *inside_low = replaced(REFERENCE_OK, "103 48 1600 ", "103 48 1319 ");
    assert(run_reference(plain_tty, plain_tty, breakdown_tty, inside_low) == 0);
    assert(verdict_ok("representative"));
    const char *outside_low = replaced(REFERENCE_OK, "103 48 1600 ", "103 48 1318 ");
    assert(run_reference(plain_tty, plain_tty, breakdown_tty, outside_low) == 2);
    assert(verdict_failed("representative", "draw 3100 vs 2818 (+10.0%)"));
    assert_only_failed("representative");
    const char *inside_high = replaced(REFERENCE_OK, "104 49 1500 ", "104 49 1844 ");
    assert(run_reference(plain_tty, plain_tty, breakdown_tty, inside_high) == 0);
    const char *outside_high = replaced(REFERENCE_OK, "104 49 1500 ", "104 49 1845 ");
    assert(run_reference(plain_tty, plain_tty, breakdown_tty, outside_high) == 2);
    assert(verdict_failed("representative", "draw 3100 vs 3445 (-10.0%)"));
}

static void test_update_outside_ten_percent_fails_representative(void) {
    const char *reference = replaced(REFERENCE_OK, "104 49 1500 ", "104 60 1500 ");
    assert(run_reference(plain_tty, plain_tty, breakdown_tty, reference) == 2);
    assert(verdict_failed("representative", "update 97 vs 108 (-10.2%)"));
    assert(strstr(verdict("representative"), "draw 3100") == NULL);
}

static void test_window_frames_side_by_side(void) {
    assert(run(plain_tty, plain_tty, breakdown_tty) == 0);
    assert(strcmp(read_file(WINDOW),
                  "run full update reference-update draw reference-draw\n"
                  "15 103 48 48 1600 1600\n"
                  "16 104 49 49 1500 1500\n") == 0);
}

static void test_window_frame_missing_from_the_reference_fails_representative(void) {
    const char *reference = replaced(REFERENCE_OK, "104 49 1500 9 1558 6\n", "");
    assert(run_reference(plain_tty, plain_tty, breakdown_tty, reference) == 2);
    assert(verdict_failed("representative", "no reference frame 104"));
}

/* Later sub-steps measure a changed program: no reference, no representativeness. */
static void test_empty_reference_skips_representative(void) {
    assert(run_with(plain_tty, plain_tty, breakdown_tty, MAP_RESUME, "", NULL) == 0);
    assert(strcmp(verdict("representative"), "representative skipped no reference frame table") == 0);
    assert(verdict_ok("breakdown"));
    assert(read_file(WINDOW)[0] == '\0');
}

static void test_missing_reference_fails_representative_naming_the_check(void) {
    assert(run_with(plain_tty, plain_tty, breakdown_tty, MAP_RESUME, DIR "/no-frames.txt", NULL) == 2);
    assert(verdict_failed("representative", DIR "/no-frames.txt"));
    assert(verdict_failed("representative", "make psx-full-game-check"));
    assert(verdict_ok("breakdown"));
}

static void test_bad_calibration_fails_breakdown(void) {
    const char *low = replaced(breakdown_tty, CALIBRATION_OK, "psx-draw: clk8-per-64-hblanks 15499\n");
    assert(run(plain_tty, plain_tty, low) == 2);
    assert(verdict_failed("breakdown", "clk8-per-64-hblanks 15499 outside 15500..19000"));
    assert_only_failed("breakdown");
    const char *high = replaced(breakdown_tty, CALIBRATION_OK, "psx-draw: clk8-per-64-hblanks 19001\n");
    assert(run(plain_tty, plain_tty, high) == 2);
    assert(verdict_failed("breakdown", "19001"));
    const char *none = replaced(breakdown_tty, CALIBRATION_OK, "");
    assert(run(plain_tty, plain_tty, none) == 2);
    assert(verdict_failed("breakdown", "no clk8-per-64-hblanks line"));
    assert(strstr(read_file(SUMMARY), "summary unavailable") != NULL);
}

static void test_wrapped_frame_fails_breakdown(void) {
    const char *at = strstr(breakdown_tty, "psx-draw: frame 9 ");
    assert(at != NULL);
    const char *wrapped_end = strstr(at, "wrapped 0\n");
    char from[512];
    size_t len = (size_t)(wrapped_end - at) + strlen("wrapped 0\n");
    memcpy(from, at, len);
    from[len] = '\0';
    char to[512];
    strcpy(to, from);
    strcpy(strstr(to, "wrapped 0"), "wrapped 1\n");
    assert(run(plain_tty, plain_tty, replaced(breakdown_tty, from, to)) == 2);
    assert(verdict_failed("breakdown", "frame 9 wrapped"));
    assert_only_failed("breakdown");
}

static void test_missing_breakdown_frame_fails_breakdown(void) {
    const char *at = strstr(breakdown_tty, "psx-draw: frame 9 ");
    char from[512];
    size_t len = (size_t)(strchr(at, '\n') - at) + 1;
    memcpy(from, at, len);
    from[len] = '\0';
    assert(run(plain_tty, plain_tty, replaced(breakdown_tty, from, "")) == 2);
    assert(verdict_failed("breakdown", "expected draw record 9, got frame 10"));
    const char *last = strstr(breakdown_tty, "psx-draw: frame 16 ");
    len = (size_t)(strchr(last, '\n') - last) + 1;
    memcpy(from, last, len);
    from[len] = '\0';
    assert(run(plain_tty, plain_tty, replaced(breakdown_tty, from, "")) == 2);
    assert(verdict_failed("breakdown", "expected draw record 16, got end of records"));
}

static void test_malformed_draw_record_fails_breakdown(void) {
    assert(run(plain_tty, plain_tty, replaced(breakdown_tty, "psx-draw: frame 3 cls 52 1 pset", "psx-draw: frame 3 cls 52 pset"))
           == 2);
    assert(verdict_failed("breakdown", "malformed draw record: frame 3 cls 52 pset"));
}

/* Frame 16 is 760 hblanks in calls; its breakdown draw is 1520. */
static void test_calls_longer_than_the_draw_part_fail_breakdown(void) {
    const char *at = "psx-frame: frame 16 update 49 draw 1520 ";
    assert(run(plain_tty, plain_tty, replaced(breakdown_tty, at, "psx-frame: frame 16 update 49 draw 759 ")) == 2);
    assert(verdict_failed("breakdown", "frame 16 in calls 760.0 hblanks above its draw 759"));
    assert(run(plain_tty, plain_tty, replaced(breakdown_tty, at, "psx-frame: frame 16 update 49 draw 760 ")) == 0);
    assert(verdict_ok("breakdown"));
}

/* Window (run frames 15-16): breakdown draw 1640 + 1520 = 3160, in calls 1250 + 760 = 2010.
 * Frame 15 share 1250/1640, frame 16 share 760/1520 = 0.5, with the plain parts:
 *   15: 56 + 1600 x 390/1640 = 436.5, 56 + 1600 x 1250/1640 = 1275.5, 56
 *   16: 58 + 750 = 808.0 both ways, 58 */
static void test_summary_split_overhead_and_best_cases(void) {
    assert(run(plain_tty, plain_tty, breakdown_tty) == 0);
    const char *summary = read_file(SUMMARY);
    assert(strstr(summary, "window run frames 15..16, full frames 103..104, 2 frames\n") != NULL);
    assert(strstr(summary, "draw breakdown boot 3160.0 hblanks\n") != NULL);
    assert(strstr(summary, "family spr 1249.6 hblanks 962 calls 39.5%\n") != NULL);
    assert(strstr(summary, "family print 560.0 hblanks 126 calls 17.7%\n") != NULL);
    assert(strstr(summary, "family line 180.0 hblanks 120 calls 5.7%\n") != NULL);
    assert(strstr(summary, "family rectfill 20.0 hblanks 2 calls 0.6%\n") != NULL);
    assert(strstr(summary, "family cls 0.4 hblanks 2 calls 0.0%\n") != NULL);
    assert(strstr(summary, "family circ 0.0 hblanks 0 calls 0.0%\n") != NULL);
    assert(strstr(summary, "pattern 20.0 hblanks 2 calls 0.6% (inside rectfill and circfill, not added)\n") != NULL);
    assert(strstr(summary, "in-calls 2010.0 hblanks 63.6%\n") != NULL);
    assert(strstr(summary, "outside-calls 1150.0 hblanks 36.4%\n") != NULL);
    assert(strstr(summary, "overhead 60 hblanks (breakdown draw 3160 - plain draw 3100)\n") != NULL);
    assert(strstr(summary, "frame 15 full 103 backend-free 436.5 game-draw-free 1275.5 draw-free 56.0\n") != NULL);
    assert(strstr(summary, "frame 16 full 104 backend-free 808.0 game-draw-free 808.0 draw-free 58.0\n") != NULL);
    assert(strstr(summary, "best backend-free max 808.0 frame 16, 2 frames over 263\n") != NULL);
    assert(strstr(summary, "best game-draw-free max 1275.5 frame 15, 2 frames over 263\n") != NULL);
    assert(strstr(summary, "best draw-free max 58.0 frame 16, 0 frames over 263\n") != NULL);
    assert(strstr(read_file(LOG), "best draw-free max 58.0") != NULL);
}

/* Plain window frames 15 and 16: update 48 + 49, draw 1600 + 1500, submit 8 + 9; no reference needed. */
static void test_window_plain_sums_follow_the_window_line(void) {
    assert(run_with(plain_tty, plain_tty, breakdown_tty, MAP_RESUME, "", NULL) == 0);
    assert(strncmp(read_file(SUMMARY),
                   "window run frames 15..16, full frames 103..104, 2 frames\n"
                   "window plain update 97 draw 3100 submit 17\n",
                   strlen("window run frames 15..16, full frames 103..104, 2 frames\n"
                          "window plain update 97 draw 3100 submit 17\n")) == 0);
    assert(strstr(read_file(LOG), "window plain update 97 draw 3100 submit 17") != NULL);
}

/* The plain numbers stay visible when the breakdown boot is what failed. */
static void test_window_plain_sums_survive_a_missing_breakdown_tty(void) {
    assert(run_with(plain_tty, plain_tty, NULL, MAP_RESUME, "", NULL) == 2);
    const char *summary = read_file(SUMMARY);
    assert(strstr(summary, "window plain update 97 draw 3100 submit 17\n") != NULL);
    assert(strstr(summary, "summary unavailable") != NULL);
}

static void test_window_plain_sums_unavailable_without_a_plain_window_frame(void) {
    const char *plain_1 = replaced(plain_tty,
                                   "psx-frame: frame 16 update 49 draw 1500 submit 9 total 1558 vsyncs 6\n", "");
    char plain_1_copy[TTY_CAP];
    strcpy(plain_1_copy, plain_1);
    assert(run_with(plain_1_copy, plain_tty, breakdown_tty, MAP_RESUME, "", NULL) == 2);
    const char *summary = read_file(SUMMARY);
    assert(strstr(summary, "window plain unavailable: plain-1 has no frame 16\n") != NULL);
    assert(strstr(summary, "window plain update") == NULL);
}

static void test_prefix_map_is_the_identity(void) {
    assert(run_with(plain_tty, plain_tty, breakdown_tty, "cut none first-game-frame 0 source-first 0 window 12 12\n",
                    "", NULL) == 0);
    const char *summary = read_file(SUMMARY);
    assert(strstr(summary, "window run frames 12..12, full frames 12..12, 1 frames\n") != NULL);
    assert(strstr(summary, "overhead 10 hblanks (breakdown draw 762 - plain draw 752)\n") != NULL);
}

static void test_asserts_off_in_every_boot_passes_asserts_off(void) {
    const char *plain = replaced(plain_tty, "psx-replay: asserts on", "psx-replay: asserts off");
    char plain_off[TTY_CAP];
    strcpy(plain_off, plain);
    const char *breakdown_off = replaced(breakdown_tty, "psx-replay: asserts on", "psx-replay: asserts off");
    assert(run_asserts(plain_off, plain_off, breakdown_off, MAP_RESUME, REFERENCE, REFERENCE_OK, "off") == 0);
    assert(strcmp(verdict("asserts"), "asserts ok off in 3 boots") == 0);
}

/* A stale build directory must not pass for the requested mode. */
static void test_a_boot_in_the_other_mode_fails_asserts_naming_it(void) {
    const char *plain = replaced(plain_tty, "psx-replay: asserts on", "psx-replay: asserts off");
    char plain_off[TTY_CAP];
    strcpy(plain_off, plain);
    assert(run_asserts(plain_off, plain_off, breakdown_tty, MAP_RESUME, REFERENCE, REFERENCE_OK, "off") == 2);
    assert(strcmp(verdict("asserts"), "asserts failed breakdown says on") == 0);
    assert_only_failed("asserts");
}

static void test_a_boot_without_the_asserts_line_fails_asserts(void) {
    const char *plain_2 = replaced(plain_tty, "psx-replay: asserts on\r\n", "");
    assert(run(plain_tty, plain_2, breakdown_tty) == 2);
    assert(strcmp(verdict("asserts"), "asserts failed plain-2 has no asserts line") == 0);
    assert_only_failed("asserts");
}

static void test_a_missing_tty_fails_asserts_naming_it(void) {
    assert(run(plain_tty, plain_tty, NULL) == 2);
    assert(verdict_failed("asserts", "breakdown tty missing"));
    assert(strstr(verdict("asserts"), "plain-1") == NULL);
}

static void test_usage_errors_exit_1(void) {
    assert(run_with(plain_tty, plain_tty, breakdown_tty, MAP_RESUME, "", NULL) == 0);
    assert(system(SCRIPT " " PLAIN_1 " " PLAIN_2 " " BREAKDOWN " " MAP " 17 263 '' > " LOG " 2>&1") != 0);
    assert(strstr(read_file(LOG), "usage") != NULL);
    assert(system(SCRIPT " " PLAIN_1 " " PLAIN_2 " " BREAKDOWN " " DIR "/no-map 17 263 '' " OUT " on > " LOG " 2>&1")
           != 0);
    assert(system(SCRIPT " " PLAIN_1 " " PLAIN_2 " " BREAKDOWN " " MAP " seventeen 263 '' " OUT " on > " LOG " 2>&1")
           != 0);
    assert(system(SCRIPT " " PLAIN_1 " " PLAIN_2 " " BREAKDOWN " " MAP " 17 -1 '' " OUT " on > " LOG " 2>&1") != 0);
    assert(run_asserts(plain_tty, plain_tty, breakdown_tty, MAP_RESUME, "", NULL, "yes") == 1);
    assert(strstr(read_file(LOG), "asserts is not on or off: yes") != NULL);
    assert(system(SCRIPT " " PLAIN_1 " " PLAIN_2 " " BREAKDOWN " " MAP " 17 263 '' " OUT " > " LOG " 2>&1") != 0);
    assert(strstr(read_file(LOG), "usage") != NULL);
    assert(run_with(plain_tty, plain_tty, breakdown_tty, "cut 100 window 103 104\n", "", NULL) == 1);
    assert(strstr(read_file(LOG), "malformed map") != NULL);
    /* Window ends past the run's last frame 16. */
    assert(run_with(plain_tty, plain_tty, breakdown_tty, "cut 100 first-game-frame 13 source-first 101 window 103 105\n",
                    "", NULL) == 1);
    assert(strstr(read_file(LOG), "window") != NULL);
}

int main(void) {
    build_ttys();
    test_every_verdict_ok_on_a_clean_run();
    test_missing_done_line_fails_done_naming_the_boot();
    test_wrong_frame_count_or_fatal_fails_done();
    test_a_state_dump_cut_before_its_last_line_fails_done_naming_the_boot();
    test_a_cheat_activations_line_before_the_done_line_does_not_complete_the_dump();
    test_missing_tty_fails_done_and_deterministic();
    test_one_differing_frame_line_fails_deterministic_naming_it();
    test_a_missing_frame_line_fails_deterministic();
    test_representative_within_ten_percent_of_the_reference_sums();
    test_update_outside_ten_percent_fails_representative();
    test_window_frames_side_by_side();
    test_window_frame_missing_from_the_reference_fails_representative();
    test_empty_reference_skips_representative();
    test_missing_reference_fails_representative_naming_the_check();
    test_bad_calibration_fails_breakdown();
    test_wrapped_frame_fails_breakdown();
    test_missing_breakdown_frame_fails_breakdown();
    test_malformed_draw_record_fails_breakdown();
    test_calls_longer_than_the_draw_part_fail_breakdown();
    test_summary_split_overhead_and_best_cases();
    test_window_plain_sums_follow_the_window_line();
    test_window_plain_sums_survive_a_missing_breakdown_tty();
    test_window_plain_sums_unavailable_without_a_plain_window_frame();
    test_prefix_map_is_the_identity();
    test_asserts_off_in_every_boot_passes_asserts_off();
    test_a_boot_in_the_other_mode_fails_asserts_naming_it();
    test_a_boot_without_the_asserts_line_fails_asserts();
    test_a_missing_tty_fails_asserts_naming_it();
    test_usage_errors_exit_1();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_short_run: all tests passed\n");
    return 0;
}
