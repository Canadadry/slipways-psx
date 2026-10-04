/* tools/oracle_check.sh, the comparison of a normalized state with its
 * plan 12 oracle, on hand-written state files: no replay. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/oracle_check.sh"
#define DIR "tmp/test_oracle_check_work"
#define ORACLE DIR "/oracle.txt"
#define STATE DIR "/state.txt"
#define VERDICT DIR "/verdict.txt"
#define DIFF DIR "/diff.txt"

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Static buffer, overwritten by each call. */
static const char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    static char buf[1 << 16];
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static int run(const char *args) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " %s > " VERDICT " 2>&1", args);
    return system(cmd);
}

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
}

static const char *compare(const char *oracle, const char *state) {
    reset();
    write_file(ORACLE, oracle);
    write_file(STATE, state);
    assert(run("compare " ORACLE " " STATE " " DIFF) == 0);
    return read_file(VERDICT);
}

#define BASE                                     \
    "clock year 3402 month 1\n"                  \
    "count planets 2\n"                          \
    "planet p1 unknown planet - world 1.00 2.00\n"

static void test_identical_state_is_held(void) {
    const char *state = BASE "zone game 10 500 1000\ncap bytes_button - 3000 6240\n";
    assert(strcmp(compare(state, state), "tenu\n") == 0);
}

static void test_one_different_planet_or_count_line_is_one_gap(void) {
    assert(strcmp(compare(BASE, "clock year 3402 month 1\n"
                                "count planets 2\n"
                                "planet p1 colonized planet - world 1.00 2.00\n"),
                  "ecart 1\n") == 0);
    assert(strstr(read_file(DIFF), "planet p1 colonized") != NULL);
    assert(strcmp(compare(BASE, "clock year 3402 month 1\n"
                                "count planets 3\n"
                                "planet p1 unknown planet - world 1.00 2.00\n"),
                  "ecart 1\n") == 0);
}

static void test_zone_and_cap_are_judged_on_peak_against_limit(void) {
    const char *oracle = BASE "zone game 10 500 1000\ncap particles_help_screen 0 1 1\n";
    assert(strcmp(compare(oracle, BASE "zone game 70 900 1000\ncap particles_help_screen 0 0 1\n"),
                  "tenu\n") == 0);
    assert(strcmp(compare(oracle, BASE "zone game 70 1000 1000\ncap particles_help_screen 1 1 1\n"),
                  "tenu\n") == 0);
    assert(strcmp(compare(oracle, BASE "zone game 70 1001 1000\ncap particles_help_screen 0 1 1\n"),
                  "ecart 1\n") == 0);
    assert(strstr(read_file(DIFF), "zone game 70 1001 1000") != NULL);
}

static void test_peak_with_no_current_value_is_read(void) {
    const char *oracle = BASE "cap bytes_particles - 1384 1384\n";
    assert(strcmp(compare(oracle, BASE "cap bytes_particles - 1300 1384\n"), "tenu\n") == 0);
    assert(strcmp(compare(oracle, BASE "cap bytes_particles - 1385 1384\n"), "ecart 1\n") == 0);
}

static void test_zone_or_cap_on_one_side_only_is_a_gap(void) {
    const char *both = BASE "zone game 10 500 1000\ncap toast 0 1 48\n";
    assert(strcmp(compare(both, BASE "zone game 10 500 1000\n"), "ecart 1\n") == 0);
    assert(strstr(read_file(DIFF), "cap toast") != NULL);
    assert(strcmp(compare(BASE "zone game 10 500 1000\n", both), "ecart 1\n") == 0);
}

static void test_changed_limit_is_noted_but_not_a_gap(void) {
    assert(strcmp(compare(BASE "cap toast 0 1 48\n", BASE "cap toast 0 1 64\n"), "tenu\n") == 0);
    assert(strstr(read_file(DIFF), "cap toast") != NULL);
}

static void test_missing_or_stopped_state_is_stopped(void) {
    reset();
    write_file(ORACLE, BASE);
    assert(run("compare " ORACLE " " DIR "/absent.txt " DIFF) == 0);
    assert(strcmp(read_file(VERDICT), "arrete etat absent\n") == 0);
    assert(strcmp(compare(BASE, "stopped p8_fatal frame 42\n" BASE), "arrete stopped p8_fatal frame 42\n") == 0);
}

static const char *limits(const char *state) {
    reset();
    write_file(STATE, state);
    assert(run("limits " STATE " " DIFF) == 0);
    return read_file(VERDICT);
}

static void test_limits_only_judges_stop_and_peaks(void) {
    assert(strcmp(limits("clock year 3414 month 1\nzone game 10 1000 1000\ncap bytes_toast - 1 48\n"),
                  "tenu\n") == 0);
    assert(strcmp(limits("clock year 3414 month 1\nzone game 10 1001 1000\ncap toast 0 49 48\n"),
                  "ecart 2\n") == 0);
    assert(strstr(read_file(DIFF), "cap toast 0 49 48") != NULL);
    assert(strcmp(limits("stopped p8_fatal frame 7\n"), "arrete stopped p8_fatal frame 7\n") == 0);
}

int main(void) {
    test_identical_state_is_held();
    test_one_different_planet_or_count_line_is_one_gap();
    test_zone_and_cap_are_judged_on_peak_against_limit();
    test_peak_with_no_current_value_is_read();
    test_zone_or_cap_on_one_side_only_is_a_gap();
    test_changed_limit_is_noted_but_not_a_gap();
    test_missing_or_stopped_state_is_stopped();
    test_limits_only_judges_stop_and_peaks();
    assert(system("rm -rf " DIR) == 0);
    printf("test_oracle_check: all tests passed\n");
    return 0;
}
