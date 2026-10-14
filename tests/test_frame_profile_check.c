/* tools/frame_profile_check.sh on hand-written frames.txt profiles, psx-draw: TTYs and maps: no
 * replay, no emulator. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/frame_profile_check.sh"
#define DIR "tmp/test_frame_profile_check_work"
#define FRAMES_TXT DIR "/frames.txt"
#define TTY DIR "/breakdown.tty.txt"
#define MAP DIR "/map.txt"
#define LOG DIR "/log.txt"

/* Run frames 15 and 16 are full frames 103 and 104; frame 12 is the resume. */
#define RUN_FRAMES 17
#define MAP_RESUME "cut 100 first-game-frame 13 source-first 101 window 103 104\n"
#define MAP_PREFIX "cut none first-game-frame 0 source-first 0 window 12 12\n"

#define TEXT_CAP (1 << 18)

static const char *const FAMILIES[] = {
    "cls", "pset", "line", "rect", "rectfill", "circ", "circfill", "spr", "print", "pattern",
};
enum { FAMILY_COUNT = 10, SPR = 7, PRINT = 8, PATTERN = 9 };

static int family_calls(int frame, int family) {
    switch (family) {
    case 0: return 1;
    case SPR: return 10 + frame;
    case PRINT: return 2 * frame;
    case PATTERN: return frame % 2;
    default: return 0;
    }
}

static void append(char *buf, size_t cap, const char *text) {
    size_t len = strlen(buf);
    assert(len + strlen(text) < cap);
    memcpy(buf + len, text, strlen(text) + 1);
}

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Static buffer, overwritten by each call; empty when the file is missing. */
static const char *read_file(const char *path) {
    static char buf[TEXT_CAP];
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

/* Static buffer, overwritten by each call: <text> with its single <from> replaced by <to>. */
static const char *replaced(const char *text, const char *from, const char *to) {
    static char buf[TEXT_CAP];
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

static char breakdown_profile[TEXT_CAP];
static char breakdown_tty[TEXT_CAP];

/* A frames.txt of RUN_FRAMES frames and the matching breakdown TTY: same calls on every frame. */
static void build_breakdown_inputs(void) {
    char line[512];
    breakdown_profile[0] = '\0';
    append(breakdown_profile, TEXT_CAP, "profile run.json frames 17 hook-pair-ns 4.6 clock monotonic-ns\n");
    for (int frame = 0; frame < RUN_FRAMES; frame++) {
        snprintf(line, sizeof line, "frame %d update 900 draw 5000\nframe %d child draw p8_cls 1 300 290 2\n", frame,
                 frame);
        append(breakdown_profile, TEXT_CAP, line);
        for (int family = 0; family < FAMILY_COUNT; family++) {
            snprintf(line, sizeof line, "frame %d family %s %d %d\n", frame, FAMILIES[family],
                     family_calls(frame, family), 40 * family_calls(frame, family));
            append(breakdown_profile, TEXT_CAP, line);
        }
        snprintf(line, sizeof line, "frame %d in-call 4000\n", frame);
        append(breakdown_profile, TEXT_CAP, line);
    }
    breakdown_tty[0] = '\0';
    append(breakdown_tty, TEXT_CAP, "Hijacked shell, loading psx-gpu-replay-breakdown.exe...\r\n"
                                    "psx-replay: asserts off\r\n"
                                    "psx-draw: clk8-per-64-hblanks 17170\r\n");
    for (int frame = 0; frame < RUN_FRAMES; frame++) {
        snprintf(line, sizeof line, "psx-draw: frame %d", frame);
        append(breakdown_tty, TEXT_CAP, line);
        for (int family = 0; family < FAMILY_COUNT; family++) {
            snprintf(line, sizeof line, " %s %d %d", FAMILIES[family], 52 * family_calls(frame, family),
                     family_calls(frame, family));
            append(breakdown_tty, TEXT_CAP, line);
        }
        append(breakdown_tty, TEXT_CAP, " wrapped 0\r\n");
    }
    append(breakdown_tty, TEXT_CAP, "psx-replay: done run 17 vsyncs 210\r\n");
}

static int run_command(const char *cmd) {
    int status = system(cmd);
    assert(WIFEXITED(status));
    return WEXITSTATUS(status);
}

static void fresh_dir(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
}

/* Returns the script's exit status; its output is in LOG. */
static int run_breakdown(const char *profile, const char *tty, const char *map) {
    fresh_dir();
    write_file(FRAMES_TXT, profile);
    write_file(TTY, tty);
    write_file(MAP, map);
    return run_command(SCRIPT " breakdown " FRAMES_TXT " " TTY " " MAP " > " LOG " 2>&1");
}

static void test_breakdown_match_over_a_resume_window(void) {
    assert(run_breakdown(breakdown_profile, breakdown_tty, MAP_RESUME) == 0);
    const char *log = read_file(LOG);
    assert(strncmp(log, "calls-match ok 2 frames\n", 24) == 0);
    /* Window totals over run frames 15 and 16. */
    assert(strstr(log, "\nfamily cls pc 2 psx 2\n") != NULL);
    assert(strstr(log, "\nfamily spr pc 51 psx 51\n") != NULL);
    assert(strstr(log, "\nfamily print pc 62 psx 62\n") != NULL);
    assert(strstr(log, "\nfamily pattern pc 1 psx 1\n") != NULL);
    assert(strstr(log, "family cls ") < strstr(log, "family pset "));
    assert(strstr(log, "family print ") < strstr(log, "family pattern "));
}

static void test_breakdown_match_over_a_prefix_window(void) {
    assert(run_breakdown(breakdown_profile, breakdown_tty, MAP_PREFIX) == 0);
    const char *log = read_file(LOG);
    assert(strncmp(log, "calls-match ok 1 frames\n", 24) == 0);
    assert(strstr(log, "\nfamily spr pc 22 psx 22\n") != NULL);
}

static void test_breakdown_ignores_frames_outside_the_window(void) {
    const char *tty = replaced(breakdown_tty, "psx-draw: frame 3 cls 52 1 ", "psx-draw: frame 3 cls 52 7 ");
    assert(run_breakdown(breakdown_profile, tty, MAP_RESUME) == 0);
}

static void test_breakdown_reports_the_first_mismatch(void) {
    static char spr[TEXT_CAP];
    strcpy(spr, replaced(breakdown_tty, " spr 1352 26 ", " spr 1352 27 "));
    const char *both = replaced(spr, " print 1560 30 ", " print 1560 31 ");
    assert(run_breakdown(breakdown_profile, both, MAP_RESUME) == 2);
    assert(strcmp(read_file(LOG), "calls-match differs frame 15 family print pc 30 psx 31\n") == 0);
}

static void test_breakdown_missing_frame_line_is_a_mismatch(void) {
    const char *tty = replaced(breakdown_tty, "psx-draw: frame 16 ", "psx-elsewhere: frame 16 ");
    assert(run_breakdown(breakdown_profile, tty, MAP_RESUME) == 2);
    assert(strcmp(read_file(LOG), "calls-match differs frame 16 no psx-draw line\n") == 0);
    const char *profile = replaced(breakdown_profile, "frame 15 family rect 0 0\n", "");
    assert(run_breakdown(profile, breakdown_tty, MAP_RESUME) == 2);
    assert(strcmp(read_file(LOG), "calls-match differs frame 15 no pc family rect line\n") == 0);
}

static void test_breakdown_accepts_lf_line_ends(void) {
    char tty[TEXT_CAP];
    size_t out = 0;
    for (const char *at = breakdown_tty; *at != '\0'; at++) {
        if (*at != '\r') tty[out++] = *at;
    }
    tty[out] = '\0';
    assert(run_breakdown(breakdown_profile, tty, MAP_RESUME) == 0);
}

static void test_breakdown_usage_errors_exit_1(void) {
    fresh_dir();
    assert(run_command(SCRIPT " breakdown " FRAMES_TXT " > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " nonsense a b c > " LOG " 2>&1") == 1);
    assert(run_breakdown(breakdown_profile, breakdown_tty, "cut 100 window 103 104\n") == 1);
    assert(run_command(SCRIPT " breakdown " DIR "/none.txt " TTY " " MAP " > " LOG " 2>&1") == 1);
}

/* Coverage profiles: cart_draw takes 1000 ns on every frame, so a child's ns over 10 is its share. */
#define FULL_TXT DIR "/full.txt"
#define START_TXT DIR "/start.txt"
#define START_MAP DIR "/start.map"
#define DRAW_TXT DIR "/draw-9.txt"
#define DRAW_MAP DIR "/draw-9.map"
#define WINDOWS " start=" START_TXT ":" START_MAP " draw-9=" DRAW_TXT ":" DRAW_MAP
#define FULL_FRAMES 80

typedef int (*child_ns_t)(const char *child, int frame);

static const char *const CHILDREN[] = {"a", "b", "c", "d", "e", "g", "h", "k"};
enum { CHILD_COUNT = 8 };

/* When set, k takes 5% on every fifth frame from 0 to 70: 15 frames, yet no span lifts its mean above 2%. */
static int scattered_child;

static int between(int frame, int first, int last) { return frame >= first && frame <= last; }

/* a always half of draw; b and e 10% over two overlapping 15-frame spans, g 10% over a third; c 5%
 * on only 14 frames; d always 1%; h 3% on the first 20 frames. */
static int full_ns(const char *child, int frame) {
    switch (child[0]) {
    case 'a': return 500;
    case 'b': return between(frame, 30, 44) ? 100 : 0;
    case 'c': return between(frame, 5, 18) ? 50 : 0;
    case 'd': return 10;
    case 'e': return between(frame, 32, 46) ? 100 : 0;
    case 'g': return between(frame, 60, 74) ? 100 : 0;
    case 'h': return between(frame, 0, 19) ? 30 : 0;
    default: return scattered_child && frame % 5 == 0 && frame <= 70 ? 50 : 0;
    }
}

/* Window run frames 2..3: h at 3% inside, b at 10% only outside the window. */
static int start_ns(const char *child, int frame) {
    switch (child[0]) {
    case 'a': return 500;
    case 'b': return between(frame, 2, 3) ? 0 : 100;
    case 'h': return between(frame, 2, 3) ? 30 : 0;
    default: return 0;
    }
}

static int draw_window_ns(const char *child, int frame) {
    (void)frame;
    return child[0] == 'a' ? 500 : 0;
}

static void write_profile(const char *path, int frames, child_ns_t child_ns) {
    static char text[TEXT_CAP];
    char line[256];
    text[0] = '\0';
    snprintf(line, sizeof line, "profile x.json frames %d hook-pair-ns 4.6 clock monotonic-ns\n", frames);
    append(text, TEXT_CAP, line);
    for (int frame = 0; frame < frames; frame++) {
        snprintf(line, sizeof line, "frame %d update 900 draw 1000\nframe %d child update ui_update 1 800 700 9\n",
                 frame, frame);
        append(text, TEXT_CAP, line);
        for (int child = 0; child < CHILD_COUNT; child++) {
            int ns = child_ns(CHILDREN[child], frame);
            snprintf(line, sizeof line, "frame %d child draw %s %d %d %d 3\n", frame, CHILDREN[child], ns > 0, ns,
                     ns / 2);
            append(text, TEXT_CAP, line);
        }
        snprintf(line, sizeof line, "frame %d self draw 10\n", frame);
        append(text, TEXT_CAP, line);
        for (int family = 0; family < FAMILY_COUNT; family++) {
            int ns = family == SPR ? 600 : family == PATTERN ? 100 : 0;
            snprintf(line, sizeof line, "frame %d family %s %d %d\n", frame, FAMILIES[family], ns > 0, ns);
            append(text, TEXT_CAP, line);
        }
        snprintf(line, sizeof line, "frame %d in-call 600\n", frame);
        append(text, TEXT_CAP, line);
    }
    write_file(path, text);
}

static void write_coverage_inputs(void) {
    fresh_dir();
    write_profile(FULL_TXT, FULL_FRAMES, full_ns);
    write_profile(START_TXT, 6, start_ns);
    write_file(START_MAP, "cut none first-game-frame 0 source-first 0 window 2 3\n");
    write_profile(DRAW_TXT, 3, draw_window_ns);
    write_file(DRAW_MAP, "cut 8 first-game-frame 1 source-first 9 window 9 10\n");
}

static int run_coverage(const char *threshold) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " coverage %s " FULL_TXT WINDOWS " > " LOG " 2>&1", threshold);
    return run_command(cmd);
}

/* Static buffer: the output line starting with <prefix> and a space, empty when missing. */
static const char *output_line(const char *prefix) {
    static char line[1024];
    line[0] = '\0';
    const char *text = read_file(LOG);
    size_t len = strlen(prefix);
    for (const char *at = text; *at != '\0';) {
        const char *end = strchr(at, '\n');
        size_t line_len = end != NULL ? (size_t)(end - at) : strlen(at);
        if (line_len > len && strncmp(at, prefix, len) == 0 && at[len] == ' ') {
            assert(line_len < sizeof line);
            memcpy(line, at, line_len);
            line[line_len] = '\0';
            return line;
        }
        at += line_len + (end != NULL ? 1 : 0);
    }
    return line;
}

static int ends_with(const char *text, const char *suffix) {
    size_t len = strlen(text), suffix_len = strlen(suffix);
    return len >= suffix_len && strcmp(text + len - suffix_len, suffix) == 0;
}

static void test_coverage_verdicts(void) {
    write_coverage_inputs();
    assert(run_coverage("2") == 3);
    assert(strcmp(output_line("child a"),
                  "child a above 80 mean 50.0% max 50.0% start 50.0% draw-9 50.0% covered") == 0);
    assert(strcmp(output_line("child h"), "child h above 20 mean 0.8% max 3.0% start 3.0% draw-9 0.0% covered") == 0);
    assert(strcmp(output_line("child d"), "child d above 0 mean 1.0% max 1.0% start 0.0% draw-9 0.0% below") == 0);
    assert(strcmp(output_line("family spr"),
                  "family spr above 80 mean 60.0% max 60.0% start 60.0% draw-9 60.0% covered") == 0);
    assert(ends_with(output_line("family cls"), " below"));
    assert(output_line("family pattern")[0] == '\0');
    assert(output_line("child ui_update")[0] == '\0');
}

/* 15 frames above the threshold is significant, 14 is sporadic. */
static void test_coverage_sporadic_below_fifteen_frames(void) {
    write_coverage_inputs();
    assert(run_coverage("2") == 3);
    assert(strncmp(output_line("child c"), "child c above 14 mean ", 22) == 0);
    assert(ends_with(output_line("child c"), " max 5.0% start 0.0% draw-9 0.0% sporadic frames 5 6 7 8 9 10 11 12 "
                                             "13 14 15 16 17 18"));
    assert(strcmp(output_line("child b"), "child b above 15 mean 1.9% max 10.0% start 0.0% draw-9 0.0% uncovered "
                                          "propose resume 30 44") == 0);
}

static void test_coverage_proposes_the_best_span_and_groups_windows(void) {
    write_coverage_inputs();
    assert(run_coverage("2") == 3);
    assert(ends_with(output_line("child e"), " uncovered propose resume 32 46"));
    assert(ends_with(output_line("child g"), " uncovered propose resume 60 74"));
    const char *log = read_file(LOG);
    assert(strstr(log, "\nwindow draw-30 resume 30 44\nwindow draw-60 resume 60 74\n") != NULL);
    assert(strstr(log, "window draw-32 ") == NULL);
    assert(ends_with(log, "\ncoverage uncovered 3 items, 2 windows\n"));
}

/* An item no proposed span lifts above the threshold still gets its own span, after the others. */
static void test_coverage_item_without_a_covering_span_gets_its_own(void) {
    scattered_child = 1;
    write_coverage_inputs();
    scattered_child = 0;
    assert(run_coverage("2") == 3);
    assert(strcmp(output_line("child k"), "child k above 15 mean 0.9% max 5.0% start 0.0% draw-9 0.0% uncovered "
                                          "propose resume 0 14") == 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "\nwindow draw-30 resume 30 44\nwindow draw-60 resume 60 74\nwindow draw-0 resume 0 14\n") != NULL);
    assert(ends_with(log, "\ncoverage uncovered 4 items, 3 windows\n"));
}

static void test_coverage_threshold_is_a_parameter(void) {
    write_coverage_inputs();
    assert(run_coverage("12") == 0);
    assert(ends_with(output_line("child a"), " covered"));
    assert(ends_with(output_line("child b"), " below"));
    assert(strcmp(output_line("child h"), "child h above 0 mean 0.8% max 3.0% start 3.0% draw-9 0.0% below") == 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "\nwindow ") == NULL);
    assert(ends_with(log, "\ncoverage ok\n"));
    assert(strncmp(log, "coverage threshold 12% ", 23) == 0);
}

static void test_coverage_malformed_input_exits_2(void) {
    write_coverage_inputs();
    write_file(DIR "/broken.txt", replaced(read_file(FULL_TXT), "frame 7 update 900 draw 1000\n", ""));
    assert(run_command(SCRIPT " coverage 2 " DIR "/broken.txt" WINDOWS " > " LOG " 2>&1") == 2);
    assert(strstr(read_file(LOG), "frame 7") != NULL);
    write_file(START_TXT, replaced(read_file(START_TXT), "frame 3 update 900 draw 1000\n", ""));
    assert(run_coverage("2") == 2);
    write_file(DIR "/broken.txt", "nothing here\n");
    assert(run_command(SCRIPT " coverage 2 " DIR "/broken.txt" WINDOWS " > " LOG " 2>&1") == 2);
}

static void test_coverage_usage_errors_exit_1(void) {
    write_coverage_inputs();
    assert(run_coverage("abc") == 1);
    assert(run_coverage("") == 1);
    assert(run_command(SCRIPT " coverage 2 " FULL_TXT " > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " coverage 2 " FULL_TXT " start > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " coverage 2 " FULL_TXT " start=" START_TXT " > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " coverage 2 " FULL_TXT " start=" DIR "/none.txt:" START_MAP " > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " coverage 2 " DIR "/none.txt" WINDOWS " > " LOG " 2>&1") == 1);
}

int main(void) {
    build_breakdown_inputs();
    test_breakdown_match_over_a_resume_window();
    test_breakdown_match_over_a_prefix_window();
    test_breakdown_ignores_frames_outside_the_window();
    test_breakdown_reports_the_first_mismatch();
    test_breakdown_missing_frame_line_is_a_mismatch();
    test_breakdown_accepts_lf_line_ends();
    test_breakdown_usage_errors_exit_1();
    test_coverage_verdicts();
    test_coverage_sporadic_below_fifteen_frames();
    test_coverage_proposes_the_best_span_and_groups_windows();
    test_coverage_item_without_a_covering_span_gets_its_own();
    test_coverage_threshold_is_a_parameter();
    test_coverage_malformed_input_exits_2();
    test_coverage_usage_errors_exit_1();
    assert(system("rm -rf " DIR) == 0);
    printf("test_frame_profile_check: all tests passed\n");
    return 0;
}
