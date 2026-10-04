/* tools/line_coverage.sh, the line coverage measure, on built events files
 * and sets of lines. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/line_coverage.sh"
#define COVER SCRIPT " cover ./tmp/replay_gcov tmp/obj-gcov"
#define DIR "tmp/test_line_coverage_work"
#define LOG DIR "/log.txt"
#define IDLE DIR "/idle_events.json"
#define CLICK DIR "/click_events.json"
#define BAD DIR "/bad_events.json"

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
    static char buf[1 << 20];
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static long line_count(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    long lines = 0;
    int c;
    while ((c = fgetc(file)) != EOF) {
        lines += c == '\n';
    }
    fclose(file);
    return lines;
}

static int run(const char *args, const char *out) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, "%s > %s 2> " LOG, args, out);
    return system(cmd);
}

#define FRAME(x, y, b) \
    "{ \"mouse_x\": " #x ", \"mouse_y\": " #y ", \"mouse_buttons\": " #b \
    ", \"dpad\": [false, false, false, false] }"

static void write_fixtures(void) {
    write_file(IDLE, "{ \"frames\": [\n" FRAME(64, 64, 0) ",\n" FRAME(64, 64, 0) ",\n"
                     FRAME(64, 64, 0) ",\n" FRAME(64, 64, 0) "\n] }\n");
    write_file(CLICK, "{ \"frames\": [\n" FRAME(64, 64, 0) ",\n" FRAME(64, 64, 0) ",\n"
                      FRAME(64, 64, 0) ",\n" FRAME(64, 64, 0) ",\n"
                      FRAME(64, 64, 1) ",\n" FRAME(64, 64, 1) ",\n"
                      FRAME(64, 64, 0) ",\n" FRAME(64, 64, 0) "\n] }\n");
    write_file(BAD, "{ \"frames\": [\n{ \"mouse_x\": 64 }\n] }\n");
}

/* Every line covered by the idle file is covered by the clicking one, which
 * covers more. */
static void test_idle_file_covers_strictly_less_than_clicking_file(void) {
    assert(run(COVER " " IDLE, DIR "/idle.lines") == 0);
    assert(run(COVER " " CLICK, DIR "/click.lines") == 0);
    long idle = line_count(DIR "/idle.lines");
    long click = line_count(DIR "/click.lines");
    assert(idle > 0);
    assert(idle < click);
    assert(run(SCRIPT " diff " DIR "/idle.lines " DIR "/click.lines", DIR "/gap.lines") == 0);
    assert(line_count(DIR "/gap.lines") == 0);
    const char *idle_text = read_file(DIR "/idle.lines");
    assert(strstr(idle_text, "src/cmd/replay.c:") != NULL);
}

/* Two files: the coverage is the union. */
static void test_cover_of_a_set_is_the_union(void) {
    assert(run(COVER " " IDLE " " CLICK, DIR "/both.lines") == 0);
    assert(run(SCRIPT " diff " DIR "/both.lines " DIR "/click.lines", DIR "/gap.lines") == 0);
    assert(line_count(DIR "/gap.lines") == 0);
    assert(run(SCRIPT " diff " DIR "/click.lines " DIR "/both.lines", DIR "/gap.lines") == 0);
    assert(line_count(DIR "/gap.lines") == 0);
}

static void test_missing_or_malformed_file_fails(void) {
    assert(run(COVER " " DIR "/absent_events.json", DIR "/x.lines") != 0);
    assert(strstr(read_file(LOG), DIR "/absent_events.json") != NULL);
    assert(run(COVER " " IDLE " " BAD, DIR "/x.lines") != 0);
    assert(strstr(read_file(LOG), BAD) != NULL);
    assert(run(COVER, DIR "/x.lines") != 0);
}

static void test_diff_returns_lines_of_first_set_missing_from_second(void) {
    write_file(DIR "/a.lines", "src/a.c:1\nsrc/a.c:2\nsrc/b.c:10\n");
    write_file(DIR "/b.lines", "src/a.c:2\nsrc/c.c:5\n");
    assert(run(SCRIPT " diff " DIR "/a.lines " DIR "/b.lines", DIR "/out.lines") == 0);
    assert(strcmp(read_file(DIR "/out.lines"), "src/a.c:1\nsrc/b.c:10\n") == 0);
}

/* What one file adds: its lines that none of the others covers. */
static void test_contrib_returns_lines_no_other_set_covers(void) {
    write_file(DIR "/a.lines", "src/a.c:1\nsrc/a.c:2\nsrc/b.c:10\n");
    write_file(DIR "/b.lines", "src/a.c:2\n");
    write_file(DIR "/c.lines", "src/b.c:10\nsrc/c.c:5\n");
    assert(run(SCRIPT " contrib " DIR "/a.lines " DIR "/b.lines " DIR "/c.lines",
               DIR "/out.lines") == 0);
    assert(strcmp(read_file(DIR "/out.lines"), "src/a.c:1\n") == 0);
    assert(run(SCRIPT " contrib " DIR "/a.lines", DIR "/out.lines") == 0);
    assert(strcmp(read_file(DIR "/out.lines"), "src/a.c:1\nsrc/a.c:2\nsrc/b.c:10\n") == 0);
}

static void test_set_operation_on_missing_set_fails(void) {
    assert(run(SCRIPT " diff " DIR "/a.lines " DIR "/absent.lines", DIR "/out.lines") != 0);
    assert(run(SCRIPT " contrib " DIR "/absent.lines " DIR "/a.lines", DIR "/out.lines") != 0);
}

/* The gap: the reference minus the covered lines and minus the unreachable
 * ones, of which only the first field, `file:line`, counts. */
static void test_gap_removes_covered_and_unreachable_lines(void) {
    write_file(DIR "/ref.lines", "src/a.c:1\nsrc/a.c:2\nsrc/b.c:10\nsrc/c.c:5\n");
    write_file(DIR "/covered.lines", "src/a.c:1\nsrc/z.c:9\n");
    write_file(DIR "/unreachable.txt", "src/c.c:5 never in 600 frames\n");
    assert(run(SCRIPT " gap " DIR "/ref.lines " DIR "/unreachable.txt " DIR "/covered.lines",
               DIR "/out.lines") == 0);
    assert(strcmp(read_file(DIR "/out.lines"), "src/a.c:2\nsrc/b.c:10\n") == 0);
}

/* No e2e: the covered set is empty, the gap is the whole reference. */
static void test_gap_of_empty_covered_set_is_whole_reference(void) {
    write_file(DIR "/ref.lines", "src/a.c:1\nsrc/b.c:10\n");
    write_file(DIR "/covered.lines", "");
    write_file(DIR "/unreachable.txt", "");
    assert(run(SCRIPT " gap " DIR "/ref.lines " DIR "/unreachable.txt " DIR "/covered.lines",
               DIR "/out.lines") == 0);
    assert(strcmp(read_file(DIR "/out.lines"), "src/a.c:1\nsrc/b.c:10\n") == 0);
}

static void test_gap_with_missing_set_fails(void) {
    assert(run(SCRIPT " gap " DIR "/ref.lines " DIR "/absent.txt " DIR "/covered.lines",
               DIR "/out.lines") != 0);
    assert(strstr(read_file(LOG), DIR "/absent.txt") != NULL);
}

/* `diff` is the text of a `git diff -U0 --no-renames`, only its headers
 * matter. Returns the translated set; the rewritten lines are in
 * DIR "/rewritten.txt". */
static const char *remap(const char *set, const char *diff) {
    write_file(DIR "/old.lines", set);
    write_file(DIR "/src.diff", diff);
    assert(run(SCRIPT " remap " DIR "/old.lines " DIR "/src.diff " DIR "/rewritten.txt",
               DIR "/out.lines") == 0);
    return read_file(DIR "/out.lines");
}

#define FILE_HEADER(path) \
    "diff --git a/" path " b/" path "\nindex 1..2 100644\n--- a/" path "\n+++ b/" path "\n"

static void test_remap_shifts_lines_after_a_pure_insertion(void) {
    const char *out = remap("src/a.c:3\nsrc/a.c:4\nsrc/a.c:9\nsrc/b.c:4\n",
                            FILE_HEADER("src/a.c") "@@ -4,0 +5,2 @@\n+x\n+y\n");
    assert(strcmp(out, "src/a.c:3\nsrc/a.c:4\nsrc/a.c:11\nsrc/b.c:4\n") == 0);
    assert(strcmp(read_file(DIR "/rewritten.txt"), "") == 0);
}

static void test_remap_insertion_at_start_and_in_the_middle(void) {
    const char *out = remap("src/a.c:1\nsrc/a.c:5\nsrc/a.c:6\n",
                            FILE_HEADER("src/a.c") "@@ -0,0 +1 @@\n+x\n@@ -5,0 +7,3 @@\n");
    assert(strcmp(out, "src/a.c:2\nsrc/a.c:6\nsrc/a.c:10\n") == 0);
}

static void test_remap_pure_deletion_rewrites_its_lines_and_shifts_the_rest(void) {
    const char *out = remap("src/a.c:2\nsrc/a.c:3\nsrc/a.c:4\nsrc/a.c:5\nsrc/a.c:9\n",
                            FILE_HEADER("src/a.c") "@@ -3,2 +2,0 @@\n-x\n-y\n");
    assert(strcmp(out, "src/a.c:2\nsrc/a.c:3\nsrc/a.c:7\n") == 0);
    assert(strcmp(read_file(DIR "/rewritten.txt"), "src/a.c:3 réécrite\nsrc/a.c:4 réécrite\n") == 0);
}

static void test_remap_replacement_of_same_and_other_length(void) {
    const char *out = remap("src/a.c:4\nsrc/a.c:5\nsrc/a.c:6\nsrc/b.c:9\nsrc/b.c:10\nsrc/b.c:11\n",
                            FILE_HEADER("src/a.c") "@@ -5 +5 @@\n-x\n+y\n"
                            FILE_HEADER("src/b.c") "@@ -10 +10,4 @@\n");
    assert(strcmp(out, "src/a.c:4\nsrc/a.c:6\nsrc/b.c:9\nsrc/b.c:14\n") == 0);
    assert(strcmp(read_file(DIR "/rewritten.txt"), "src/a.c:5 réécrite\nsrc/b.c:10 réécrite\n") == 0);
}

static void test_remap_several_blocks_add_their_shifts(void) {
    const char *out = remap("src/a.c:1\nsrc/a.c:10\nsrc/a.c:20\nsrc/a.c:30\n",
                            FILE_HEADER("src/a.c") "@@ -5,0 +6,2 @@\n@@ -12,3 +15 @@\n@@ -25 +26,4 @@\n");
    assert(strcmp(out, "src/a.c:1\nsrc/a.c:12\nsrc/a.c:20\nsrc/a.c:33\n") == 0);
}

static void test_remap_deleted_file_is_all_rewritten_new_file_changes_nothing(void) {
    const char *out = remap("src/a.c:1\nsrc/a.c:7\nsrc/c.c:3\n",
                            "diff --git a/src/a.c b/src/a.c\ndeleted file mode 100644\n"
                            "--- a/src/a.c\n+++ /dev/null\n@@ -1,9 +0,0 @@\n"
                            "diff --git a/src/n.c b/src/n.c\nnew file mode 100644\n"
                            "--- /dev/null\n+++ b/src/n.c\n@@ -0,0 +1,5 @@\n");
    assert(strcmp(out, "src/c.c:3\n") == 0);
    assert(strcmp(read_file(DIR "/rewritten.txt"), "src/a.c:1 réécrite\nsrc/a.c:7 réécrite\n") == 0);
}

static void test_remap_of_empty_set_or_empty_diff(void) {
    assert(strcmp(remap("", FILE_HEADER("src/a.c") "@@ -1 +1 @@\n"), "") == 0);
    assert(strcmp(read_file(DIR "/rewritten.txt"), "") == 0);
    assert(strcmp(remap("src/a.c:2\nsrc/a.c:10\n", ""), "src/a.c:2\nsrc/a.c:10\n") == 0);
}

int main(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_fixtures();
    test_idle_file_covers_strictly_less_than_clicking_file();
    test_cover_of_a_set_is_the_union();
    test_missing_or_malformed_file_fails();
    test_diff_returns_lines_of_first_set_missing_from_second();
    test_contrib_returns_lines_no_other_set_covers();
    test_set_operation_on_missing_set_fails();
    test_gap_removes_covered_and_unreachable_lines();
    test_gap_of_empty_covered_set_is_whole_reference();
    test_gap_with_missing_set_fails();
    test_remap_shifts_lines_after_a_pure_insertion();
    test_remap_insertion_at_start_and_in_the_middle();
    test_remap_pure_deletion_rewrites_its_lines_and_shifts_the_rest();
    test_remap_replacement_of_same_and_other_length();
    test_remap_several_blocks_add_their_shifts();
    test_remap_deleted_file_is_all_rewritten_new_file_changes_nothing();
    test_remap_of_empty_set_or_empty_diff();
    assert(system("rm -rf " DIR) == 0);
    printf("test_line_coverage: all tests passed\n");
    return 0;
}
