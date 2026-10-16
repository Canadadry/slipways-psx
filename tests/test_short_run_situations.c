/* tools/short_run_situations.sh on hand-written run and full situation tables and maps. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define SCRIPT "sh tools/short_run_situations.sh"
#define DIR "tmp/test_short_run_situations_work"
#define RUN_DIR DIR "/map-rest-5"
#define RUN RUN_DIR "/situations.txt"
#define MAP RUN_DIR "/map.txt"
#define FULL DIR "/full.txt"
#define OUT DIR "/out.txt"

#define HEAD "frame situation family\n"

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

static int run(const char *run_table, const char *map, const char *full_table) {
    assert(system("rm -rf " DIR " && mkdir -p " RUN_DIR) == 0);
    write_file(RUN, run_table);
    write_file(MAP, map);
    write_file(FULL, full_table);
    return run_command(SCRIPT " " RUN " " MAP " " FULL " > " OUT " 2>&1");
}

static void expect_output(const char *text) {
    if (strcmp(read_file(OUT), text) != 0) {
        fprintf(stderr, "test_short_run_situations: expected '%s', got:\n%s", text, read_file(OUT));
        assert(0);
    }
}

/* Full game: frames 0-4 title, 5-8 map-rest, 9 menu. */
#define FULL_TEN HEAD \
    "0 title steady\n1 title steady\n2 title steady\n3 title steady\n4 title steady\n" \
    "5 map-rest steady\n6 map-rest steady\n7 map-rest steady\n8 map-rest steady\n9 menu steady\n"

/* A prefix run plays the full game's frames as they are. */
#define PREFIX_MAP "cut none first-game-frame 0 source-first 0 window 5 7\n"

static void test_a_prefix_run_whose_window_matches_is_same(void) {
    assert(run(HEAD "0 title steady\n1 title steady\n2 title steady\n3 title steady\n4 title steady\n"
                    "5 map-rest steady\n6 map-rest steady\n7 map-rest steady\n",
               PREFIX_MAP, FULL_TEN) == 0);
    expect_output("short-run-situation map-rest-5 same map-rest\n");
}

/* Resumed at full frame 4: run frames 0-12 are the title resume, run frame 13 plays full frame 4. */
#define RESUME_MAP "cut 3 first-game-frame 13 source-first 4 window 5 7\n"

static const char *resume_run(const char *frame_14, const char *frame_15, const char *frame_16) {
    static char text[1 << 12];
    char *at = text + sprintf(text, HEAD);
    for (int frame = 0; frame < 13; frame++) {
        at += sprintf(at, "%d galaxy-generation one-off\n", frame);
    }
    sprintf(at, "13 title steady\n14 %s\n15 %s\n16 %s\n", frame_14, frame_15, frame_16);
    return text;
}

static void test_a_resume_run_never_compares_its_title_resume_frames(void) {
    assert(run(resume_run("map-rest steady", "map-rest steady", "map-rest steady"), RESUME_MAP, FULL_TEN) == 0);
    expect_output("short-run-situation map-rest-5 same map-rest\n");
}

static void test_differs_names_the_first_differing_full_frame_and_both_labels(void) {
    assert(run(resume_run("map-rest steady", "map-move steady", "menu steady"), RESUME_MAP, FULL_TEN) == 0);
    expect_output("short-run-situation map-rest-5 differs frame 6 run map-move full map-rest\n");
}

static void test_a_run_table_ending_before_the_window_differs_as_missing(void) {
    assert(run(HEAD "0 title steady\n1 title steady\n2 title steady\n3 title steady\n4 title steady\n"
                    "5 map-rest steady\n",
               PREFIX_MAP, FULL_TEN) == 0);
    expect_output("short-run-situation map-rest-5 differs frame 6 run missing full map-rest\n");
}

static void test_a_full_window_of_several_labels_is_mixed(void) {
    assert(run(resume_run("map-rest steady", "map-rest steady", "map-rest steady"),
               "cut 3 first-game-frame 13 source-first 4 window 7 9\n", FULL_TEN) == 0);
    expect_output("short-run-situation map-rest-5 mixed map-rest menu\n");
}

static void test_a_one_off_full_window_is_not_steady(void) {
    assert(run(HEAD "0 colonise one-off\n", "cut none first-game-frame 0 source-first 0 window 0 0\n",
               HEAD "0 colonise one-off\n") == 0);
    expect_output("short-run-situation map-rest-5 not-steady colonise\n");
}

static void test_usage_and_parse_errors_exit_1(void) {
    const char *table = resume_run("map-rest steady", "map-rest steady", "map-rest steady");
    assert(run(table, "cut 3 first-game-frame 13 window 5 7\n", FULL_TEN) == 1);
    assert(run(table, "cut x first-game-frame 13 source-first 4 window 5 7\n", FULL_TEN) == 1);
    assert(run(table, "cut 3 first-game-frame 13 source-first 4 window 7 5\n", FULL_TEN) == 1);
    assert(run(table, "", FULL_TEN) == 1);
    assert(run(table, "cut 3 first-game-frame 13 source-first 4 window 5 12\n", FULL_TEN) == 1);
    assert(run("frame situation\n0 title\n", RESUME_MAP, FULL_TEN) == 1);
    assert(run(table, RESUME_MAP, HEAD "0 title steady\n2 title steady\n") == 1);
    assert(run(table, RESUME_MAP, FULL_TEN) == 0);
    assert(run_command(SCRIPT " " RUN " " MAP " > " OUT " 2>&1") == 1);
    assert(strstr(read_file(OUT), "usage") != NULL);
}

int main(void) {
    test_a_prefix_run_whose_window_matches_is_same();
    test_a_resume_run_never_compares_its_title_resume_frames();
    test_differs_names_the_first_differing_full_frame_and_both_labels();
    test_a_run_table_ending_before_the_window_differs_as_missing();
    test_a_full_window_of_several_labels_is_mixed();
    test_a_one_off_full_window_is_not_steady();
    test_usage_and_parse_errors_exit_1();
    assert(system("rm -rf " DIR) == 0);
    return 0;
}
