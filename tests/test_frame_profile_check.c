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

/* Static buffer: <text> with " units 3 switches 1" ending every per-frame family line, as
 * tmp/frame_profile writes them. */
static const char *with_units(const char *text) {
    static char buf[TEXT_CAP];
    buf[0] = '\0';
    for (const char *at = text; *at != '\0';) {
        const char *end = strchr(at, '\n');
        assert(end != NULL);
        size_t len = strlen(buf), line_len = (size_t)(end - at);
        assert(len + line_len + 32 < sizeof buf);
        memcpy(buf + len, at, line_len);
        buf[len + line_len] = '\0';
        int family_at = 0;
        sscanf(buf + len, "frame %*d family %n", &family_at);
        if (family_at > 0) strcat(buf, " units 3 switches 1");
        strcat(buf, "\n");
        at = end + 1;
    }
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

static void test_breakdown_reads_family_lines_ending_with_units(void) {
    assert(run_breakdown(breakdown_profile, breakdown_tty, MAP_RESUME) == 0);
    char plain[TEXT_CAP];
    snprintf(plain, sizeof plain, "%s", read_file(LOG));
    assert(run_breakdown(with_units(breakdown_profile), breakdown_tty, MAP_RESUME) == 0);
    assert(strcmp(read_file(LOG), plain) == 0);
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

/* When set, the full profile is a noisy repeat: d spikes to 5% on frames 0..19 and h drops to 0 on
 * frame 0. */
static int noisy_repeat;

/* h's time in the start window, so that several start profiles can differ. */
static int start_h_ns = 30;

static int between(int frame, int first, int last) { return frame >= first && frame <= last; }

/* a always half of draw; b and e 10% over two overlapping 15-frame spans, g 10% over a third; c 5%
 * on only 14 frames; d always 1%; h 3% on the first 20 frames. */
static int full_ns(const char *child, int frame) {
    switch (child[0]) {
    case 'a': return 500;
    case 'b': return between(frame, 30, 44) ? 100 : 0;
    case 'c': return between(frame, 5, 18) ? 50 : 0;
    case 'd': return noisy_repeat && frame < 20 ? 50 : 10;
    case 'e': return between(frame, 32, 46) ? 100 : 0;
    case 'g': return between(frame, 60, 74) ? 100 : 0;
    case 'h': return between(frame, noisy_repeat ? 1 : 0, 19) ? 30 : 0;
    default: return scattered_child && frame % 5 == 0 && frame <= 70 ? 50 : 0;
    }
}

/* Window run frames 2..3: h at 3% inside, b at 10% only outside the window. */
static int start_ns(const char *child, int frame) {
    switch (child[0]) {
    case 'a': return 500;
    case 'b': return between(frame, 2, 3) ? 0 : 100;
    case 'h': return between(frame, 2, 3) ? start_h_ns : 0;
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

static void test_coverage_reads_family_lines_ending_with_units(void) {
    write_coverage_inputs();
    assert(run_coverage("2") == 3);
    char plain[TEXT_CAP];
    snprintf(plain, sizeof plain, "%s", read_file(LOG));
    const char *paths[] = {FULL_TXT, START_TXT, DRAW_TXT};
    for (size_t p = 0; p < sizeof paths / sizeof paths[0]; p++) {
        char text[TEXT_CAP];
        snprintf(text, sizeof text, "%s", read_file(paths[p]));
        write_file(paths[p], with_units(text));
    }
    assert(run_coverage("2") == 3);
    assert(strcmp(read_file(LOG), plain) == 0);
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

#define FULL2_TXT DIR "/full2.txt"
#define START2_TXT DIR "/start2.txt"
#define START3_TXT DIR "/start3.txt"

/* A frame is above the threshold only when every full profile says so: d's spikes in one profile
 * and h's missing frame 0 in the other give d below and h above 19, with the minimum's mean. */
static void test_coverage_takes_the_per_frame_minimum_of_the_full_profiles(void) {
    write_coverage_inputs();
    noisy_repeat = 1;
    write_profile(FULL2_TXT, FULL_FRAMES, full_ns);
    noisy_repeat = 0;
    assert(run_command(SCRIPT " coverage 2 " FULL_TXT "," FULL2_TXT WINDOWS " > " LOG " 2>&1") == 3);
    assert(strcmp(output_line("child d"), "child d above 0 mean 1.0% max 1.0% start 0.0% draw-9 0.0% below") == 0);
    assert(strcmp(output_line("child h"), "child h above 19 mean 0.7% max 3.0% start 3.0% draw-9 0.0% covered") == 0);
    assert(strstr(read_file(LOG), "\ncoverage profiles 2 full, frame share = minimum over the full profiles, "
                                  "window share = median over its profiles\n") != NULL);
}

/* Window shares 1%, 3%, 5%: median 3% with three profiles; 1% and 5%: their mean, 3%, with two. */
static void test_coverage_takes_the_median_of_the_window_profiles(void) {
    write_coverage_inputs();
    start_h_ns = 10;
    write_profile(START2_TXT, 6, start_ns);
    start_h_ns = 50;
    write_profile(START3_TXT, 6, start_ns);
    start_h_ns = 30;
    assert(run_command(SCRIPT " coverage 2 " FULL_TXT " start=" START2_TXT "," START_TXT "," START3_TXT ":" START_MAP
                       " > " LOG " 2>&1") == 3);
    assert(strcmp(output_line("child h"), "child h above 20 mean 0.8% max 3.0% start 3.0% covered") == 0);
    assert(run_command(SCRIPT " coverage 2 " FULL_TXT " start=" START3_TXT "," START2_TXT ":" START_MAP " > " LOG
                       " 2>&1") == 3);
    assert(strcmp(output_line("child h"), "child h above 20 mean 0.8% max 3.0% start 3.0% covered") == 0);
    assert(run_command(SCRIPT " coverage 2 " FULL_TXT " start=" START2_TXT ":" START_MAP " > " LOG " 2>&1") == 3);
    assert(ends_with(output_line("child h"), " start 1.0% uncovered propose resume 0 14"));
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
    write_coverage_inputs();
    write_profile(FULL2_TXT, FULL_FRAMES - 1, full_ns);
    assert(run_command(SCRIPT " coverage 2 " FULL_TXT "," FULL2_TXT WINDOWS " > " LOG " 2>&1") == 2);
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
    assert(run_command(SCRIPT " coverage 2 " FULL_TXT "," DIR "/none.txt" WINDOWS " > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " coverage 2 " FULL_TXT " start=" START_TXT "," DIR "/none.txt:" START_MAP " > " LOG
                       " 2>&1") == 1);
}

/* Subtree profiles rooted on draw_layers, window run frames 2..3: the root takes 1000 then 3000 ns,
 * planet_draw_method half of it, hud_print_outlined#2 160 ns, button_draw_method only outside the
 * window. <scale> multiplies hud's time, so that several profiles of one run can differ. */
#define SUBTREE_START_TXT DIR "/subtree-start.txt"
#define SUBTREE_START_MAP DIR "/subtree-start.map"

static void write_subtree_profile(const char *path, const char *root, int hud_scale) {
    static char text[TEXT_CAP];
    char line[256];
    text[0] = '\0';
    snprintf(line, sizeof line, "profile run.json frames 6 hook-pair-ns 4.6 clock monotonic-ns\nroot draw %s\n", root);
    append(text, TEXT_CAP, line);
    for (int frame = 0; frame < 6; frame++) {
        int window = between(frame, 2, 3);
        int draw = frame == 3 ? 3000 : 1000;
        int planet = draw / 2;
        int hud = (frame == 2 ? 100 : frame == 3 ? 60 : 20) * hud_scale;
        snprintf(line, sizeof line, "frame %d update 900 draw %d\nframe %d child update ui_update 1 800 700 9\n", frame,
                 draw, frame);
        append(text, TEXT_CAP, line);
        snprintf(line, sizeof line, "frame %d child draw planet_draw_method %d %d %d 40\n", frame, 23 + 2 * (frame == 3),
                 planet, planet - 100);
        append(text, TEXT_CAP, line);
        snprintf(line, sizeof line, "frame %d child draw button_draw_method %d %d %d 0\n", frame, !window,
                 window ? 0 : 70, window ? 0 : 70);
        append(text, TEXT_CAP, line);
        snprintf(line, sizeof line, "frame %d child draw hud_print_outlined#2 %d %d %d 9\n", frame,
                 frame == 3 ? 11 : 9, hud, hud);
        append(text, TEXT_CAP, line);
        snprintf(line, sizeof line, "frame %d self draw %d\nframe %d self update 100\n", frame,
                 draw - planet - hud - (window ? 0 : 70), frame);
        append(text, TEXT_CAP, line);
        snprintf(line, sizeof line, "frame %d family print 9 %d\nframe %d in-call 600\n", frame, hud, frame);
        append(text, TEXT_CAP, line);
    }
    write_file(path, text);
}

static void write_subtree_inputs(void) {
    fresh_dir();
    write_subtree_profile(SUBTREE_START_TXT, "draw_layers", 1);
    write_file(SUBTREE_START_MAP, "cut none first-game-frame 0 source-first 0 window 2 3\n");
}

static void test_subtree_lists_the_root_children_over_the_window(void) {
    write_subtree_inputs();
    assert(run_command(SCRIPT " subtree start=" SUBTREE_START_TXT ":" SUBTREE_START_MAP " > " LOG " 2>&1") == 0);
    assert(strcmp(read_file(LOG), "root draw_layers run start window-frames 2 root-ns-per-frame 2000\n"
                                  "child draw_layers planet_draw_method calls 23-25 share 50.0%\n"
                                  "child draw_layers hud_print_outlined#2 calls 9-11 share 4.0%\n"
                                  "self draw_layers share 46.0%\n") == 0);
}

#define SUBTREE_START2_TXT DIR "/subtree-start2.txt"
#define SUBTREE_START3_TXT DIR "/subtree-start3.txt"
#define SUBTREE_DRAW_MAP DIR "/subtree-draw-9.map"
#define SUBTREE_START " start=" SUBTREE_START_TXT ":" SUBTREE_START_MAP

/* hud's shares 4%, 12%, 20%: median 12% with three profiles; 4% and 20%: their mean, 12%, with two. */
static void test_subtree_takes_the_median_of_the_run_profiles(void) {
    write_subtree_inputs();
    write_subtree_profile(SUBTREE_START2_TXT, "draw_layers", 3);
    write_subtree_profile(SUBTREE_START3_TXT, "draw_layers", 5);
    const char *expected = "root draw_layers run start window-frames 2 root-ns-per-frame 2000\n"
                           "child draw_layers planet_draw_method calls 23-25 share 50.0%\n"
                           "child draw_layers hud_print_outlined#2 calls 9-11 share 12.0%\n"
                           "self draw_layers share 38.0%\n";
    assert(run_command(SCRIPT " subtree start=" SUBTREE_START3_TXT "," SUBTREE_START_TXT "," SUBTREE_START2_TXT
                       ":" SUBTREE_START_MAP " > " LOG " 2>&1") == 0);
    assert(strcmp(read_file(LOG), expected) == 0);
    assert(run_command(SCRIPT " subtree start=" SUBTREE_START_TXT "," SUBTREE_START3_TXT ":" SUBTREE_START_MAP " > " LOG
                       " 2>&1") == 0);
    assert(strcmp(read_file(LOG), expected) == 0);
}

/* On run frames 4..5, button_draw_method is called and outweighs hud: children go by decreasing share. */
static void test_subtree_reports_each_run_in_order(void) {
    write_subtree_inputs();
    write_file(SUBTREE_DRAW_MAP, "cut 8 first-game-frame 1 source-first 5 window 8 9\n");
    assert(run_command(SCRIPT " subtree" SUBTREE_START " draw-9=" SUBTREE_START_TXT ":" SUBTREE_DRAW_MAP " > " LOG
                       " 2>&1") == 0);
    assert(strcmp(read_file(LOG), "root draw_layers run start window-frames 2 root-ns-per-frame 2000\n"
                                  "child draw_layers planet_draw_method calls 23-25 share 50.0%\n"
                                  "child draw_layers hud_print_outlined#2 calls 9-11 share 4.0%\n"
                                  "self draw_layers share 46.0%\n"
                                  "root draw_layers run draw-9 window-frames 2 root-ns-per-frame 1000\n"
                                  "child draw_layers planet_draw_method calls 23-23 share 50.0%\n"
                                  "child draw_layers button_draw_method calls 1-1 share 7.0%\n"
                                  "child draw_layers hud_print_outlined#2 calls 9-9 share 2.0%\n"
                                  "self draw_layers share 41.0%\n") == 0);
}

static void test_subtree_malformed_input_exits_2(void) {
    write_subtree_inputs();
    write_subtree_profile(SUBTREE_START2_TXT, "cart_draw", 1);
    assert(run_command(SCRIPT " subtree start=" SUBTREE_START_TXT "," SUBTREE_START2_TXT ":" SUBTREE_START_MAP " > " LOG
                       " 2>&1") == 2);
    assert(strstr(read_file(LOG), "cart_draw") != NULL);
    write_file(SUBTREE_START2_TXT, replaced(read_file(SUBTREE_START_TXT), "root draw draw_layers\n", ""));
    assert(run_command(SCRIPT " subtree start=" SUBTREE_START2_TXT ":" SUBTREE_START_MAP " > " LOG " 2>&1") == 2);
    assert(strstr(read_file(LOG), "no root draw line") != NULL);
    assert(run_command(SCRIPT " subtree start=" SUBTREE_START_TXT "," SUBTREE_START2_TXT ":" SUBTREE_START_MAP " > " LOG
                       " 2>&1") == 2);
    write_file(SUBTREE_START2_TXT, replaced(read_file(SUBTREE_START_TXT), "frame 3 update 900 draw 3000\n", ""));
    assert(run_command(SCRIPT " subtree start=" SUBTREE_START2_TXT ":" SUBTREE_START_MAP " > " LOG " 2>&1") == 2);
    assert(strstr(read_file(LOG), "frame 3") != NULL);
    write_file(SUBTREE_START2_TXT, "nothing here\n");
    assert(run_command(SCRIPT " subtree start=" SUBTREE_START2_TXT ":" SUBTREE_START_MAP " > " LOG " 2>&1") == 2);
}

static void test_subtree_usage_errors_exit_1(void) {
    write_subtree_inputs();
    assert(run_command(SCRIPT " subtree > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " subtree start > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " subtree start=" SUBTREE_START_TXT " > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " subtree start=" DIR "/none.txt:" SUBTREE_START_MAP " > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " subtree start=" SUBTREE_START_TXT ":" DIR "/none.map > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " subtree" SUBTREE_START " start=" SUBTREE_START_TXT ":" DIR "/none.map > " LOG " 2>&1")
           == 1);
    assert(read_file(LOG)[0] != 'r');
}

/* Returns the script's exit status; its output is in LOG. */
static int run_unit_cost(const char *profile, const char *tty, const char *map) {
    fresh_dir();
    write_file(FRAMES_TXT, profile);
    write_file(TTY, tty);
    write_file(MAP, map);
    return run_command(SCRIPT " unit-cost " FRAMES_TXT " " TTY " " MAP " > " LOG " 2>&1");
}

/* Window run frames 15 and 16: spr 51 calls 2652 ticks, print 62 calls 3224 ticks, cls 2 calls 104
 * ticks, every family 3 units and 1 switch per frame; 64 / 17170 hblanks per tick. */
static void test_unit_cost_prices_each_family_over_the_window(void) {
    assert(run_unit_cost(with_units(breakdown_profile), breakdown_tty, MAP_RESUME) == 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "family cls calls 2 units 6 switches 2 hblanks 0.4 per-call 0.194 per-unit 0.065\n") == log);
    assert(strstr(log, "\nfamily pset calls 0 units 6 switches 2 hblanks 0.0 per-call - per-unit 0.000\n") != NULL);
    assert(strstr(log, "\nfamily spr calls 51 units 6 switches 2 hblanks 9.9 per-call 0.194 per-unit 1.648\n") != NULL);
    assert(strstr(log, "\nfamily print calls 62 units 6 switches 2 hblanks 12.0 per-call 0.194 per-unit 2.003\n")
           != NULL);
    assert(strstr(log, "\nfamily pattern calls 1 units 6 switches 2 hblanks 0.2 per-call 0.194 per-unit 0.032\n")
           != NULL);
    assert(strstr(log, "family print ") < strstr(log, "family pattern "));
    assert(strstr(log, "\nin-calls 22.3\nwindow-frames 2\n") != NULL);
    assert(strstr(log, "window-frames 2\n")[strlen("window-frames 2\n")] == '\0');
}

static void test_unit_cost_refuses_a_calls_mismatch_naming_the_first(void) {
    const char *tty = replaced(breakdown_tty, " print 1664 32 ", " print 1664 33 ");
    assert(run_unit_cost(with_units(breakdown_profile), tty, MAP_RESUME) == 2);
    assert(strcmp(read_file(LOG), "calls-match differs frame 16 family print pc 32 psx 33\n") == 0);
    tty = replaced(breakdown_tty, "psx-draw: frame 15 ", "psx-elsewhere: frame 15 ");
    assert(run_unit_cost(with_units(breakdown_profile), tty, MAP_RESUME) == 2);
    assert(strcmp(read_file(LOG), "calls-match differs frame 15 no psx-draw line\n") == 0);
}

static void test_unit_cost_refuses_a_profile_without_units(void) {
    assert(run_unit_cost(breakdown_profile, breakdown_tty, MAP_RESUME) == 2);
    assert(strstr(read_file(LOG), "calls-match differs frame 15 no pc family cls line with units") != NULL);
}

static void test_unit_cost_refuses_a_tty_without_calibration(void) {
    const char *tty = replaced(breakdown_tty, "psx-draw: clk8-per-64-hblanks 17170\r\n", "");
    assert(run_unit_cost(with_units(breakdown_profile), tty, MAP_RESUME) == 2);
    assert(strstr(read_file(LOG), "clk8-per-64-hblanks") != NULL);
}

static void test_unit_cost_over_a_prefix_window(void) {
    assert(run_unit_cost(with_units(breakdown_profile), breakdown_tty, MAP_PREFIX) == 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "\nfamily spr calls 22 units 3 switches 1 hblanks 4.3 per-call 0.194 per-unit 1.421\n") != NULL);
    assert(strstr(log, "\nwindow-frames 1\n") != NULL);
}

static void test_unit_cost_usage_errors_exit_1(void) {
    run_unit_cost(with_units(breakdown_profile), breakdown_tty, MAP_RESUME);
    assert(run_command(SCRIPT " unit-cost " FRAMES_TXT " " TTY " > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " unit-cost " DIR "/none.txt " TTY " " MAP " > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " unit-cost " FRAMES_TXT " " DIR "/none.tty " MAP " > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " unit-cost " FRAMES_TXT " " TTY " " DIR "/none.map > " LOG " 2>&1") == 1);
    assert(strstr(read_file(LOG), "family ") == NULL);
}

#define RESUME_COST DIR "/resume-cost.txt"
#define PREFIX_COST DIR "/prefix-cost.txt"

/* Writes the unit-cost outputs of the resume and prefix windows to RESUME_COST and PREFIX_COST. */
static void write_unit_cost_outputs(void) {
    static char resume[TEXT_CAP];
    assert(run_unit_cost(with_units(breakdown_profile), breakdown_tty, MAP_RESUME) == 0);
    snprintf(resume, sizeof resume, "%s", read_file(LOG));
    assert(run_unit_cost(with_units(breakdown_profile), breakdown_tty, MAP_PREFIX) == 0);
    write_file(PREFIX_COST, read_file(LOG));
    write_file(RESUME_COST, resume);
}

static void test_unit_cost_pooled_sums_the_runs(void) {
    write_unit_cost_outputs();
    assert(run_command(SCRIPT " unit-cost-pooled " RESUME_COST " " PREFIX_COST " > " LOG " 2>&1") == 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "family cls calls 3 units 9 switches 3 hblanks 0.6 per-call 0.200 per-unit 0.067\n") == log);
    assert(strstr(log, "\nfamily pset calls 0 units 9 switches 3 hblanks 0.0 per-call - per-unit 0.000\n") != NULL);
    assert(strstr(log, "\nfamily spr calls 73 units 9 switches 3 hblanks 14.2 per-call 0.195 per-unit 1.578\n") != NULL);
    assert(strstr(log, "family print ") < strstr(log, "family pattern "));
    assert(strstr(log, "\nin-calls 31.4\nwindow-frames 3\n") != NULL);
}

static void test_unit_cost_pooled_malformed_input_exits_2(void) {
    write_unit_cost_outputs();
    write_file(PREFIX_COST, replaced(read_file(PREFIX_COST), "family spr ", "family sprite "));
    assert(run_command(SCRIPT " unit-cost-pooled " RESUME_COST " " PREFIX_COST " > " LOG " 2>&1") == 2);
    write_file(PREFIX_COST, "calls-match differs frame 12 no psx-draw line\n");
    assert(run_command(SCRIPT " unit-cost-pooled " RESUME_COST " " PREFIX_COST " > " LOG " 2>&1") == 2);
    assert(strstr(read_file(LOG), PREFIX_COST) != NULL);
    write_unit_cost_outputs();
    write_file(RESUME_COST, replaced(read_file(RESUME_COST), "window-frames 2\n", ""));
    assert(run_command(SCRIPT " unit-cost-pooled " RESUME_COST " " PREFIX_COST " > " LOG " 2>&1") == 2);
    assert(strstr(read_file(LOG), RESUME_COST ": truncated") != NULL);
}

static void test_unit_cost_pooled_usage_errors_exit_1(void) {
    assert(run_command(SCRIPT " unit-cost-pooled > " LOG " 2>&1") == 1);
    assert(run_command(SCRIPT " unit-cost-pooled " DIR "/none.txt > " LOG " 2>&1") == 1);
}

/* A judge table of SPIKE_FRAMES frames: frame 0 sits on the 263 threshold, frames 1, 2, 4, 5 and 7 are
 * above it, 2 and 7 above 1000. */
#define JUDGE DIR "/judge.txt"
#define BASE_TXT DIR "/base.txt"
#define BASE2_TXT DIR "/base2.txt"
#define UI_TXT DIR "/ui.txt"
#define PLANET_TXT DIR "/planet.txt"
#define SPIKE_FRAMES 8

static void write_judge(void) {
    write_file(JUDGE, "frame update draw submit total vsyncs\n"
                      "0 263 100 2 365 1\n1 264 100 2 366 1\n2 1500 100 2 1602 6\n3 100 100 2 202 0\n"
                      "4 300 100 2 402 1\n5 400 100 2 502 1\n6 50 100 2 152 0\n7 2000 100 2 2102 7\n");
}

/* <children> are "frame <f> child update ..." lines; every frame's update root takes <update_ns>. */
static void write_spike_profile(const char *path, const char *root, int frames, int update_ns, const char *children) {
    static char text[TEXT_CAP];
    char line[256];
    text[0] = '\0';
    snprintf(line, sizeof line,
             "profile full_game.json frames %d hook-pair-ns 4.6 clock monotonic-ns\nroot draw cart_draw\n"
             "root update %s\n",
             frames, root);
    append(text, TEXT_CAP, line);
    for (int frame = 0; frame < frames; frame++) {
        snprintf(line, sizeof line, "frame %d update %d draw 5000\nframe %d child update main_ui 1 50 50 0\n", frame,
                 update_ns, frame);
        append(text, TEXT_CAP, line);
    }
    append(text, TEXT_CAP, children);
    write_file(path, text);
}

static int run_spikes(const char *args) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " spikes %s > " LOG " 2>&1", args);
    return run_command(cmd);
}

static void test_spikes_selects_frames_strictly_above_the_threshold(void) {
    fresh_dir();
    write_judge();
    write_spike_profile(BASE_TXT, "cart_update60", SPIKE_FRAMES, 1000, "");
    assert(run_spikes("263 " JUDGE " " BASE_TXT) == 0);
    assert(strcmp(read_file(LOG),
                  "frame 1 update 264 pc-update-ns 1000 path cart_update60 share 100.0% unsettled\n"
                  "frame 2 update 1500 pc-update-ns 1000 path cart_update60 share 100.0% unsettled\n"
                  "frame 4 update 300 pc-update-ns 1000 path cart_update60 share 100.0% unsettled\n"
                  "frame 5 update 400 pc-update-ns 1000 path cart_update60 share 100.0% unsettled\n"
                  "frame 7 update 2000 pc-update-ns 1000 path cart_update60 share 100.0% unsettled\n"
                  "class cart_update60 frames 5 over-1000 2 update-sum 4464 share 100.0%-100.0%-100.0% runs-on 8 "
                  "spike-runs 5 hblanks-per-pc-us 400.000 list 1,2,4,5,7 unsettled\n"
                  "spike-frames 5 over-1000 2 classes 1 open 0\n") == 0);
    assert(run_spikes("299 " JUDGE " " BASE_TXT) == 0);
    assert(strstr(read_file(LOG), "\nspike-frames 4 over-1000 2 classes 1 open 0\n") != NULL);
}

/* Frame 1: ui_update takes 600 of 1000 ns in BASE_TXT, 450 of 1200 in BASE2_TXT. */
static void test_spikes_takes_the_per_frame_minimum_of_each_set(void) {
    fresh_dir();
    write_judge();
    write_spike_profile(BASE_TXT, "cart_update60", SPIKE_FRAMES, 1000, "frame 1 child update ui_update 1 600 100 9\n");
    write_spike_profile(BASE2_TXT, "cart_update60", SPIKE_FRAMES, 1200, "frame 1 child update ui_update 1 450 100 9\n");
    assert(run_spikes("263 " JUDGE " " BASE_TXT) == 3);
    assert(strcmp(output_line("frame 1"),
                  "frame 1 update 264 pc-update-ns 1000 path cart_update60/ui_update share 60.0% open ui_update") == 0);
    assert(run_spikes("263 " JUDGE " " BASE_TXT "," BASE2_TXT) == 0);
    assert(strcmp(output_line("frame 1"),
                  "frame 1 update 264 pc-update-ns 1000 path cart_update60 share 100.0% unsettled") == 0);
    /* A child missing from one profile of the set counts as 0 there. */
    write_spike_profile(BASE2_TXT, "cart_update60", SPIKE_FRAMES, 1000, "");
    assert(run_spikes("263 " JUDGE " " BASE_TXT "," BASE2_TXT) == 0);
}

/* Exactly half steps in, one ns less stops. */
static void test_spikes_steps_in_from_half_of_the_update(void) {
    fresh_dir();
    write_judge();
    write_spike_profile(BASE_TXT, "cart_update60", SPIKE_FRAMES, 1000,
                        "frame 1 child update ui_update 1 500 100 9\nframe 2 child update ui_update 1 499 100 9\n"
                        "frame 2 child update update_entities 1 300 100 9\n");
    assert(run_spikes("263 " JUDGE " " BASE_TXT) == 3);
    assert(ends_with(output_line("frame 1"), " path cart_update60/ui_update share 50.0% open ui_update"));
    assert(ends_with(output_line("frame 2"), " path cart_update60 share 100.0% unsettled"));
    assert(strcmp(output_line("open ui_update"), "open ui_update frames 1") == 0);
    assert(ends_with(read_file(LOG), "\nspike-frames 5 over-1000 2 classes 1 open 1\n"));
}

/* ui_update holds 800 ns of frames 2 and 5; inside it planet_update holds 700 then 600 ns and stops there,
 * and runs on frames 2, 3 and 5. */
static void write_descent_inputs(void) {
    fresh_dir();
    write_judge();
    write_spike_profile(BASE_TXT, "cart_update60", SPIKE_FRAMES, 1000,
                        "frame 2 child update ui_update 1 800 100 9\nframe 5 child update ui_update 1 800 100 9\n");
    write_spike_profile(UI_TXT, "ui_update", SPIKE_FRAMES, 800,
                        "frame 2 child update planet_update 1 700 100 9\nframe 3 child update planet_update 2 70 10 9\n"
                        "frame 5 child update planet_update 1 600 100 9\nframe 2 child update hud_update 1 50 50 0\n");
    write_spike_profile(PLANET_TXT, "planet_update", SPIKE_FRAMES, 700,
                        "frame 2 child update planet_tick 3 300 100 9\nframe 5 child update planet_tick 3 200 100 9\n");
}

static void test_spikes_descends_through_a_symbol_set(void) {
    write_descent_inputs();
    assert(run_spikes("263 " JUDGE " " BASE_TXT " ui_update=" UI_TXT) == 3);
    assert(strcmp(output_line("frame 2"), "frame 2 update 1500 pc-update-ns 1000 path cart_update60/ui_update/"
                                          "planet_update share 70.0% open planet_update") == 0);
    assert(strcmp(output_line("open planet_update"), "open planet_update frames 2") == 0);
    assert(run_spikes("263 " JUDGE " " BASE_TXT " ui_update=" UI_TXT " planet_update=" PLANET_TXT) == 0);
    assert(strcmp(output_line("frame 2"), "frame 2 update 1500 pc-update-ns 1000 path cart_update60/ui_update/"
                                          "planet_update share 70.0%") == 0);
    assert(strcmp(output_line("frame 5"), "frame 5 update 400 pc-update-ns 1000 path cart_update60/ui_update/"
                                          "planet_update share 60.0%") == 0);
}

/* hblanks per PC us: 1500 and 400 over 1 us, median 950. */
static void test_spikes_class_lines_count_the_runs(void) {
    write_descent_inputs();
    assert(run_spikes("263 " JUDGE " " BASE_TXT " ui_update=" UI_TXT " planet_update=" PLANET_TXT) == 0);
    assert(strcmp(output_line("class cart_update60/ui_update/planet_update"),
                  "class cart_update60/ui_update/planet_update frames 2 over-1000 1 update-sum 1900 "
                  "share 60.0%-65.0%-70.0% runs-on 3 spike-runs 2 hblanks-per-pc-us 950.000 list 2,5") == 0);
    assert(strcmp(output_line("class cart_update60"), "class cart_update60 frames 3 over-1000 1 update-sum 2564 "
                                                      "share 100.0%-100.0%-100.0% runs-on 8 spike-runs 5 "
                                                      "hblanks-per-pc-us 300.000 list 1,4,7 unsettled") == 0);
    assert(strstr(read_file(LOG), "class cart_update60 ") < strstr(read_file(LOG), "class cart_update60/"));
    assert(ends_with(read_file(LOG), "\nspike-frames 5 over-1000 2 classes 2 open 0\n"));
    /* runs-on reads the first profile of the set only. */
    assert(run_spikes("263 " JUDGE " " BASE_TXT "," BASE_TXT " ui_update=" UI_TXT "," UI_TXT " planet_update=" PLANET_TXT
                      "," PLANET_TXT) == 0);
    assert(strstr(read_file(LOG), " runs-on 3 spike-runs 2 ") != NULL);
}

static void test_spikes_marks_a_multi_site_walk(void) {
    write_descent_inputs();
    write_spike_profile(BASE_TXT, "cart_update60", SPIKE_FRAMES, 1000,
                        "frame 2 child update ui_update#2 1 800 100 9\nframe 5 child update ui_update#1 1 800 100 9\n");
    assert(run_spikes("263 " JUDGE " " BASE_TXT) == 3);
    assert(ends_with(output_line("frame 2"), " path cart_update60/ui_update#2 share 80.0% open ui_update"));
    assert(run_spikes("263 " JUDGE " " BASE_TXT " ui_update=" UI_TXT " planet_update=" PLANET_TXT) == 0);
    assert(ends_with(output_line("frame 2"), " path cart_update60/ui_update#2/planet_update share 70.0% multi-site"));
    assert(strcmp(output_line("class cart_update60/ui_update#1/planet_update"),
                  "class cart_update60/ui_update#1/planet_update frames 1 over-1000 0 update-sum 400 "
                  "share 60.0%-60.0%-60.0% runs-on 3 spike-runs 2 hblanks-per-pc-us 400.000 list 5 multi-site") == 0);
}

static void test_spikes_closes_on_an_unresolvable_symbol(void) {
    write_descent_inputs();
    assert(run_spikes("263 " JUDGE " " BASE_TXT " ui_update=" UI_TXT " planet_update=unresolvable") == 0);
    assert(ends_with(output_line("frame 2"), " path cart_update60/ui_update/planet_update share 70.0% unresolvable"));
    assert(ends_with(output_line("class cart_update60/ui_update/planet_update"), " list 2,5 unresolvable"));
}

static void test_spikes_malformed_input_exits_2(void) {
    write_descent_inputs();
    write_spike_profile(BASE2_TXT, "cart_update60", SPIKE_FRAMES - 1, 1000, "");
    assert(run_spikes("263 " JUDGE " " BASE_TXT "," BASE2_TXT) == 2);
    assert(strstr(read_file(LOG), "frames 7, not 8") != NULL);
    write_spike_profile(BASE2_TXT, "ui_update", SPIKE_FRAMES, 1000, "");
    assert(run_spikes("263 " JUDGE " " BASE2_TXT) == 2);
    assert(strstr(read_file(LOG), "root update ui_update, not cart_update60") != NULL);
    assert(run_spikes("263 " JUDGE " " BASE_TXT " planet_update=" UI_TXT) == 2);
    write_file(BASE2_TXT, replaced(read_file(BASE_TXT), "root update cart_update60\n", ""));
    assert(run_spikes("263 " JUDGE " " BASE2_TXT) == 2);
    assert(strstr(read_file(LOG), "no root update line") != NULL);
    assert(run_spikes("263 " JUDGE " " BASE_TXT "," BASE_TXT " ui_update=" UI_TXT) == 2);
    write_file(BASE2_TXT, replaced(read_file(BASE_TXT), "frame 6 update 1000 draw 5000\n", ""));
    assert(run_spikes("263 " JUDGE " " BASE2_TXT) == 2);
    write_file(DIR "/judge2.txt", replaced(read_file(JUDGE), "3 100 100 2 202 0\n", ""));
    assert(run_spikes("263 " DIR "/judge2.txt " BASE_TXT) == 2);
}

static void test_spikes_usage_errors_exit_1(void) {
    write_descent_inputs();
    assert(run_spikes("") == 1);
    assert(run_spikes("263 " JUDGE) == 1);
    assert(run_spikes("abc " JUDGE " " BASE_TXT) == 1);
    assert(run_spikes("263 " DIR "/none.txt " BASE_TXT) == 1);
    assert(run_spikes("263 " JUDGE " " DIR "/none.txt") == 1);
    assert(run_spikes("263 " JUDGE " " BASE_TXT " ui_update") == 1);
    assert(run_spikes("263 " JUDGE " " BASE_TXT " ui_update=" DIR "/none.txt") == 1);
    assert(run_spikes("263 " JUDGE " " BASE_TXT " cart_update60=" UI_TXT) == 1);
    assert(run_spikes("263 " JUDGE " " BASE_TXT " ui_update=" UI_TXT " ui_update=unresolvable") == 1);
}

int main(void) {
    build_breakdown_inputs();
    test_breakdown_match_over_a_resume_window();
    test_breakdown_reads_family_lines_ending_with_units();
    test_breakdown_match_over_a_prefix_window();
    test_breakdown_ignores_frames_outside_the_window();
    test_breakdown_reports_the_first_mismatch();
    test_breakdown_missing_frame_line_is_a_mismatch();
    test_breakdown_accepts_lf_line_ends();
    test_breakdown_usage_errors_exit_1();
    test_coverage_verdicts();
    test_coverage_reads_family_lines_ending_with_units();
    test_coverage_sporadic_below_fifteen_frames();
    test_coverage_proposes_the_best_span_and_groups_windows();
    test_coverage_item_without_a_covering_span_gets_its_own();
    test_coverage_threshold_is_a_parameter();
    test_coverage_takes_the_per_frame_minimum_of_the_full_profiles();
    test_coverage_takes_the_median_of_the_window_profiles();
    test_coverage_malformed_input_exits_2();
    test_coverage_usage_errors_exit_1();
    test_subtree_lists_the_root_children_over_the_window();
    test_subtree_takes_the_median_of_the_run_profiles();
    test_subtree_reports_each_run_in_order();
    test_subtree_malformed_input_exits_2();
    test_subtree_usage_errors_exit_1();
    test_unit_cost_prices_each_family_over_the_window();
    test_unit_cost_refuses_a_calls_mismatch_naming_the_first();
    test_unit_cost_refuses_a_profile_without_units();
    test_unit_cost_refuses_a_tty_without_calibration();
    test_unit_cost_over_a_prefix_window();
    test_unit_cost_usage_errors_exit_1();
    test_unit_cost_pooled_sums_the_runs();
    test_unit_cost_pooled_malformed_input_exits_2();
    test_unit_cost_pooled_usage_errors_exit_1();
    test_spikes_selects_frames_strictly_above_the_threshold();
    test_spikes_takes_the_per_frame_minimum_of_each_set();
    test_spikes_steps_in_from_half_of_the_update();
    test_spikes_descends_through_a_symbol_set();
    test_spikes_class_lines_count_the_runs();
    test_spikes_marks_a_multi_site_walk();
    test_spikes_closes_on_an_unresolvable_symbol();
    test_spikes_malformed_input_exits_2();
    test_spikes_usage_errors_exit_1();
    assert(system("rm -rf " DIR) == 0);
    printf("test_frame_profile_check: all tests passed\n");
    return 0;
}
