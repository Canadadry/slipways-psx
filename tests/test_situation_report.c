/* tools/situation_report.sh on hand-written frame and situation tables. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/situation_report.sh"
#define DIR "tmp/test_situation_report_work"
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

static int run(const char *frames, const char *situations) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(FRAMES, frames);
    write_file(SITUATIONS, situations);
    return run_command(SCRIPT " " FRAMES " " SITUATIONS " > " OUT " 2>&1");
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
        fprintf(stderr, "test_situation_report: no line '%s' in:\n%s", line, read_file(OUT));
        assert(0);
    }
}

/* Totals 300 and 264 are over 263; 263 is not. */
#define FRAMES_SIX FRAMES_HEAD \
    "0 10 100 3 113 0\n" \
    "1 20 200 3 223 0\n" \
    "2 40 3000 30 3070 11\n" \
    "3 10 253 0 263 1\n" \
    "4 11 251 2 264 1\n" \
    "5 5 290 5 300 1\n"

#define SITUATIONS_SIX SITUATIONS_HEAD \
    "0 title steady\n" \
    "1 title steady\n" \
    "2 galaxy-generation one-off\n" \
    "3 map-rest steady\n" \
    "4 screen-change one-off\n" \
    "5 map-rest steady\n"

static void test_per_situation_counts_over_and_integer_means(void) {
    assert(run(FRAMES_SIX, SITUATIONS_SIX) == 0);
    expect_line("situation title steady frames 2 over 0 update-mean 15 draw-mean 150");
    expect_line("situation map-rest steady frames 2 over 1 update-mean 7 draw-mean 271");
    expect_line("situation galaxy-generation one-off frames 1 over 1 update-mean 40 draw-mean 3000");
    expect_line("situation screen-change one-off frames 1 over 1 update-mean 11 draw-mean 251");
    expect_line("frame-count ok 6 frames");
    expect_line("families ok");
    expect_line("one-off-runs ok");
}

/* Steady first then one-off, each in the rule's order, whatever order the table shows them in. */
static void test_situations_print_in_the_rules_order(void) {
    assert(run(FRAMES_SIX, SITUATIONS_SIX) == 0);
    const char *text = read_file(OUT);
    const char *title = strstr(text, "situation title ");
    const char *rest = strstr(text, "situation map-rest ");
    const char *galaxy = strstr(text, "situation galaxy-generation ");
    const char *change = strstr(text, "situation screen-change ");
    const char *one_off = strstr(text, "one-off galaxy-generation ");
    const char *verdict = strstr(text, "frame-count ");
    assert(title != NULL && rest != NULL && galaxy != NULL && change != NULL && one_off != NULL && verdict != NULL);
    assert(title < rest && rest < galaxy && galaxy < change && change < one_off && one_off < verdict);
    assert(strstr(text, "situation pause ") == NULL);
}

static void test_occurrences_and_longest_run_of_each_one_off(void) {
    assert(run(FRAMES_HEAD
               "0 1 1 1 3 0\n1 1 1 1 3 0\n2 1 1 1 3 0\n3 1 1 1 3 0\n4 1 1 1 3 0\n"
               "5 1 1 1 3 0\n6 1 1 1 3 0\n7 1 1 1 3 0\n",
               SITUATIONS_HEAD
               "0 screen-change one-off\n"
               "1 screen-change one-off\n"
               "2 map-rest steady\n"
               "3 screen-change one-off\n"
               "4 screen-change one-off\n"
               "5 screen-change one-off\n"
               "6 colonise one-off\n"
               "7 screen-change one-off\n") == 0);
    expect_line("one-off screen-change occurrences 3 longest 3");
    expect_line("one-off colonise occurrences 1 longest 1");
    assert(strstr(read_file(OUT), "one-off new-year ") == NULL);
}

static void write_run(const char *label, int length) {
    static char frames[1 << 14];
    static char situations[1 << 14];
    char *f = frames;
    char *s = situations;
    f += sprintf(f, FRAMES_HEAD);
    s += sprintf(s, SITUATIONS_HEAD);
    for (int frame = 0; frame < length; frame++) {
        f += sprintf(f, "%d 1 1 1 3 0\n", frame);
        s += sprintf(s, "%d %s one-off\n", frame, label);
    }
    assert(run(frames, situations) == 0);
}

/* 31 frames: the shake of 3 decaying 0.1 a frame, plus the event frame. */
static void test_a_run_past_31_frames_is_long_and_fails_the_runs(void) {
    write_run("screen-change", 31);
    expect_line("one-off screen-change occurrences 1 longest 31");
    expect_line("one-off-runs ok");
    write_run("screen-change", 32);
    expect_line("one-off screen-change occurrences 1 longest 32 long");
    expect_line("one-off-runs failed screen-change");
}

static void test_a_frame_count_mismatch_fails_frame_count(void) {
    assert(run(FRAMES_SIX "6 1 1 1 3 0\n", SITUATIONS_SIX) == 0);
    expect_line("frame-count failed frame table 7 frames, situation table 6");
}

static void test_a_numbering_gap_fails_frame_count(void) {
    assert(run(FRAMES_SIX, SITUATIONS_HEAD
               "0 title steady\n"
               "1 title steady\n"
               "3 galaxy-generation one-off\n"
               "4 map-rest steady\n"
               "5 screen-change one-off\n"
               "6 map-rest steady\n") == 0);
    expect_line("frame-count failed situation table line 3 is frame 3, expected 2");
    assert(run(FRAMES_HEAD "0 1 1 1 3 0\n2 1 1 1 3 0\n", SITUATIONS_HEAD "0 title steady\n1 title steady\n") == 0);
    expect_line("frame-count failed frame table line 2 is frame 2, expected 1");
}

static void test_an_unknown_family_fails_families(void) {
    assert(run(FRAMES_HEAD "0 1 1 1 3 0\n", SITUATIONS_HEAD "0 title rare\n") == 0);
    expect_line("families failed title has unknown family rare");
}

static void test_a_name_with_two_families_fails_families(void) {
    assert(run(FRAMES_HEAD "0 1 1 1 3 0\n1 1 1 1 3 0\n",
               SITUATIONS_HEAD "0 menu steady\n1 menu one-off\n") == 0);
    expect_line("families failed menu is both steady and one-off");
}

static void test_usage_and_parse_errors_exit_1(void) {
    assert(run(FRAMES_SIX, SITUATIONS_SIX) == 0);
    assert(run_command(SCRIPT " " FRAMES " > " OUT " 2>&1") == 1);
    assert(strstr(read_file(OUT), "usage") != NULL);
    assert(run_command(SCRIPT " " DIR "/missing.txt " SITUATIONS " > " OUT " 2>&1") == 1);
    assert(run("frame update draw\n0 1 1\n", SITUATIONS_SIX) == 1);
    assert(run(FRAMES_SIX, "frame situation\n0 title\n") == 1);
    assert(run(FRAMES_HEAD "0 1 x 1 3 0\n", SITUATIONS_HEAD "0 title steady\n") == 1);
    assert(run(FRAMES_HEAD "0 1 1 1 3 0\n", SITUATIONS_HEAD "0 title\n") == 1);
}

int main(void) {
    test_per_situation_counts_over_and_integer_means();
    test_situations_print_in_the_rules_order();
    test_occurrences_and_longest_run_of_each_one_off();
    test_a_run_past_31_frames_is_long_and_fails_the_runs();
    test_a_frame_count_mismatch_fails_frame_count();
    test_a_numbering_gap_fails_frame_count();
    test_an_unknown_family_fails_families();
    test_a_name_with_two_families_fails_families();
    test_usage_and_parse_errors_exit_1();
    assert(system("rm -rf " DIR) == 0);
    return 0;
}
