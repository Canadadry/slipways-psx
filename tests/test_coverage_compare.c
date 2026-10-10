/* tools/coverage_compare.sh, the coverage guardrail of plan 12, on
 * hand-written lines files and zero-context diffs: no replay, no git. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/coverage_compare.sh"
#define DIR "tmp/test_coverage_compare_work"
#define BASELINE DIR "/baseline.txt"
#define CURRENT DIR "/current.txt"
#define DIFF DIR "/diff.txt"
#define OUT DIR "/out.txt"

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
    static char buf[1 << 14];
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

/* 0 when the script exits 0, 1 otherwise; stdout and stderr go to OUT. */
static int compare(const char *baseline, const char *current, const char *diff) {
    write_file(BASELINE, baseline);
    write_file(CURRENT, current);
    write_file(DIFF, diff);
    int status = system(SCRIPT " " BASELINE " " CURRENT " " DIFF " > " OUT " 2>&1");
    return status == 0 ? 0 : 1;
}

/* One tree per run, each file edited one way: a script run costs ~3 ms. */
#define EDITED_BASELINE                                                                    \
    "src/engine/b.c:10 1\n"                                                                \
    "src/engine/b.c:20 1\n"                                                                \
    "src/engine/old.h:2 1\n"                                                               \
    "src/game/a.c:3 1\n"                                                                   \
    "src/game/a.c:4 1\n"                                                                   \
    "src/game/a.c:9 1\n"                                                                   \
    "src/game/c.c:3 1\n"                                                                   \
    "src/game/c.c:4 1\n"                                                                   \
    "src/game/c.c:5 1\n"                                                                   \
    "src/game/c.c:8 1\n"                                                                   \
    "src/game/old.c:5 1\n"                                                                 \
    "src/game/old.c:9 1\n"                                                                 \
    "src/game/u.c:3 1\n"                                                                   \
    "src/game/u.c:4 1\n"                                                                   \
    "src/game/u.c:6 0\n"

#define EDITED_CURRENT                                                                     \
    "src/engine/b.c:10 0\n"                                                                \
    "src/engine/b.c:11 0\n"                                                                \
    "src/engine/b.c:21 1\n"                                                                \
    "src/engine/new.h:2 1\n"                                                               \
    "src/game/a.c:3 1\n"                                                                   \
    "src/game/a.c:4 1\n"                                                                   \
    "src/game/a.c:5 1\n"                                                                   \
    "src/game/a.c:6 0\n"                                                                   \
    "src/game/a.c:11 1\n"                                                                  \
    "src/game/c.c:3 1\n"                                                                   \
    "src/game/c.c:6 1\n"                                                                   \
    "src/game/new.c:5 1\n"                                                                 \
    "src/game/new.c:10 1\n"                                                                \
    "src/game/u.c:3 1\n"                                                                   \
    "src/game/u.c:4 1\n"                                                                   \
    "src/game/u.c:6 0\n"

#define EDITED_DIFF                                                                        \
    "diff --git a/src/engine/b.c b/src/engine/b.c\n"                                       \
    "index 1111111..2222222 100644\n"                                                      \
    "--- a/src/engine/b.c\n"                                                               \
    "+++ b/src/engine/b.c\n"                                                               \
    "@@ -10 +10,2 @@ int b(void) {\n"                                                       \
    "-    return rnd(4);\n"                                                                \
    "+    int roll = visual_rnd(4);\n"                                                     \
    "+    return roll;\n"                                                                  \
    "diff --git a/src/engine/old.h b/src/engine/new.h\n"                                   \
    "similarity index 100%\n"                                                              \
    "rename from src/engine/old.h\n"                                                       \
    "rename to src/engine/new.h\n"                                                         \
    "diff --git a/src/game/a.c b/src/game/a.c\n"                                           \
    "index 1111111..2222222 100644\n"                                                      \
    "--- a/src/game/a.c\n"                                                                 \
    "+++ b/src/game/a.c\n"                                                                 \
    "@@ -4,0 +5,2 @@ void f(void) {\n"                                                      \
    "+    g();\n"                                                                          \
    "+    h();\n"                                                                          \
    "diff --git a/src/game/c.c b/src/game/c.c\n"                                           \
    "index 1111111..2222222 100644\n"                                                      \
    "--- a/src/game/c.c\n"                                                                 \
    "+++ b/src/game/c.c\n"                                                                 \
    "@@ -4,2 +3,0 @@ void f(void) {\n"                                                      \
    "-    g();\n"                                                                          \
    "-    h();\n"                                                                          \
    "diff --git a/src/game/old.c b/src/game/new.c\n"                                       \
    "similarity index 95%\n"                                                               \
    "rename from src/game/old.c\n"                                                         \
    "rename to src/game/new.c\n"                                                           \
    "index 1111111..2222222 100644\n"                                                      \
    "--- a/src/game/old.c\n"                                                               \
    "+++ b/src/game/new.c\n"                                                               \
    "@@ -6,0 +7 @@ void f(void) {\n"                                                        \
    "+    g();\n"

static const char *edited_out;

static void test_untouched_file_keeps_every_hit_line(void) {
    assert(strstr(edited_out, "hit src/game/u.c:3 src/game/u.c:3\n") != NULL);
    assert(strstr(edited_out, "hit src/game/u.c:4 src/game/u.c:4\n") != NULL);
}

static void test_insertion_above_shifts_the_lines_below(void) {
    assert(strstr(edited_out, "hit src/game/a.c:3 src/game/a.c:3\n") != NULL);
    assert(strstr(edited_out, "hit src/game/a.c:4 src/game/a.c:4\n") != NULL && "inserted after it");
    assert(strstr(edited_out, "hit src/game/a.c:9 src/game/a.c:11\n") != NULL);
}

static void test_removed_hunk_counts_its_lines_deleted(void) {
    assert(strstr(edited_out, "hit src/game/c.c:3 src/game/c.c:3\n") != NULL);
    assert(strstr(edited_out, "deleted src/game/c.c:4\n") != NULL);
    assert(strstr(edited_out, "deleted src/game/c.c:5\n") != NULL);
    assert(strstr(edited_out, "hit src/game/c.c:8 src/game/c.c:6\n") != NULL);
}

static void test_rewritten_line_counts_deleted_not_unhit(void) {
    assert(strstr(edited_out, "deleted src/engine/b.c:10\n") != NULL);
    assert(strstr(edited_out, "hit src/engine/b.c:20 src/engine/b.c:21\n") != NULL);
}

static void test_renamed_file_is_followed(void) {
    assert(strstr(edited_out, "hit src/game/old.c:5 src/game/new.c:5\n") != NULL);
    assert(strstr(edited_out, "hit src/game/old.c:9 src/game/new.c:10\n") != NULL);
    assert(strstr(edited_out, "hit src/engine/old.h:2 src/engine/new.h:2\n") != NULL && "no hunk");
}

static void test_baseline_line_not_hit_is_ignored(void) {
    assert(strstr(edited_out, "u.c:6") == NULL);
}

static void test_totals_end_the_output(void) {
    const char *totals = "totals hit 11 unhit 0 deleted 3\n";
    size_t length = strlen(edited_out);
    assert(length >= strlen(totals));
    assert(strcmp(edited_out + length - strlen(totals), totals) == 0);
}

static void test_mapped_line_no_longer_hit_is_unhit_and_fails(void) {
    assert(compare("src/game/a.c:3 1\nsrc/game/a.c:4 1\nsrc/game/a.c:7 1\n",
                   "src/game/a.c:3 0\nsrc/game/a.c:7 1\n", "") == 1);
    const char *out = read_file(OUT);
    assert(strstr(out, "unhit src/game/a.c:3 src/game/a.c:3\n") != NULL);
    assert(strstr(out, "unhit src/game/a.c:4 src/game/a.c:4\n") != NULL && "no longer executable");
    assert(strstr(out, "hit src/game/a.c:7 src/game/a.c:7\n") != NULL);
    assert(strstr(out, "totals hit 1 unhit 2 deleted 0\n") != NULL);
}

int main(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    assert(compare(EDITED_BASELINE, EDITED_CURRENT, EDITED_DIFF) == 0);
    static char out[1 << 14];
    snprintf(out, sizeof out, "%s", read_file(OUT));
    edited_out = out;
    test_untouched_file_keeps_every_hit_line();
    test_insertion_above_shifts_the_lines_below();
    test_removed_hunk_counts_its_lines_deleted();
    test_rewritten_line_counts_deleted_not_unhit();
    test_renamed_file_is_followed();
    test_baseline_line_not_hit_is_ignored();
    test_totals_end_the_output();
    test_mapped_line_no_longer_hit_is_unhit_and_fails();
    assert(system("rm -rf " DIR) == 0);
    printf("test_coverage_compare: all tests passed\n");
    return 0;
}
