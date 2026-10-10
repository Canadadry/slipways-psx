#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_button_guard_work"
#define SRC DIR "/src"
#define LIST DIR "/allowance.txt"
#define OUT DIR "/out.txt"
#define GUARD "sh tools/button_guard.sh " LIST " " SRC

/* Static buffer, NUL-terminated, overwritten by each call. */
static const char *read_file(const char *path) {
    static char buf[1 << 16];
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* The exit code of the script, read back from a file: the raw status of
 * system() cannot be decoded without <sys/wait.h>, outside C99. */
static int run_guard(void) {
    assert(system(GUARD " > " OUT " 2>&1; echo $? > " DIR "/code") == 0);
    return atoi(read_file(DIR "/code"));
}

/* Spawns no process, the directories are made once by main: every guard run
 * costs a few milliseconds. */
static void setup(void) {
    write_file(SRC "/nav.c", "int jump(int p) { return (p & P8_PAD_BUTTON_7) || (p & P8_PAD_BUTTON_8); }\n"
                             "int aim(int p) { return (p & P8_PAD_BUTTON_12) != 0; }\n");
    write_file(SRC "/cheat.h", "#define CHORD (P8_PAD_BUTTON_5 | P8_PAD_BUTTON_6)\n");
    write_file(SRC "/hud.c", "int hud(void) { return 1; }\n");
    write_file(LIST, "# comment\n\n"
                     SRC "/nav.c step-9 2 | jump: enter_aim, open_menu\n"
                     SRC "/nav.c step-10 1 | aim: select\n"
                     SRC "/cheat.h fixed 2 | the cheat chord\n");
}

static void test_tree_matching_the_allowance_passes(void) {
    setup();
    assert(run_guard() == 0);
    assert(strstr(read_file(OUT), "button-guard files 2 reads 5\n") != NULL);
}

static void test_count_changes_fail_naming_the_file_and_both_counts(void) {
    setup();
    write_file(SRC "/nav.c", "int jump(int p) { return (p & P8_PAD_BUTTON_7) || (p & P8_PAD_BUTTON_8); }\n"
                             "int aim(int p) { return (p & P8_PAD_BUTTON_12) || (p & P8_PAD_BUTTON_5); }\n");
    write_file(SRC "/cheat.h", "#define CHORD 0\n");
    write_file(SRC "/hud.c", "int hud(int p) { return p & P8_PAD_BUTTON_11; }\n");
    assert(run_guard() == 1);
    const char *out = read_file(OUT);
    assert(strstr(out, "button_guard: " SRC "/nav.c names 4 button(s), listed 3\n") != NULL);
    assert(strstr(out, "button_guard: " SRC "/hud.c names 1 button(s), not listed\n") != NULL);
    assert(strstr(out, "button_guard: " SRC "/cheat.h names no button, listed 2\n") != NULL);
}

static void test_stale_line_with_a_higher_count_fails(void) {
    setup();
    write_file(SRC "/nav.c", "int jump(int p) { return (p & P8_PAD_BUTTON_7) || (p & P8_PAD_BUTTON_8); }\n");
    assert(run_guard() == 1);
    assert(strstr(read_file(OUT), SRC "/nav.c names 2 button(s), listed 3\n") != NULL);
}

static void test_names_in_comments_literals_and_longer_identifiers_are_not_reads(void) {
    setup();
    write_file(SRC "/hud.c", "/* P8_PAD_BUTTON_1 once\n * P8_PAD_BUTTON_2 twice */\n"
                             "int hud(void) { return 1; } // P8_PAD_BUTTON_3\n"
                             "const char *m = \"P8_PAD_BUTTON_4 \\\" P8_PAD_BUTTON_4\";\n"
                             "char quote = '\"'; /* P8_PAD_BUTTON_5 */ int x; /* \" */\n"
                             "int MY_P8_PAD_BUTTON_5 = 0; char c = 'P';\n");
    assert(run_guard() == 0);
    assert(strstr(read_file(OUT), "button-guard files 2 reads 5\n") != NULL);
}

static void test_invalid_allowance_lines_are_errors(void) {
    setup();
    write_file(LIST, SRC "/nav.c step-9 2\n"
                     SRC "/nav.c step-10 2 |  \n"
                     SRC "/nav.c step-8 1 | before the bindings\n"
                     SRC "/nav.c step-14 1 | after the bindings\n"
                     SRC "/cheat.h fixed 0 | the cheat chord\n"
                     SRC "/hud.c 2 | the hud\n"
                     SRC "/gone.c step-12 2 | deleted\n"
                     SRC "/nav.c step-10 1 | aim, again\n");
    assert(run_guard() == 2);
    const char *out = read_file(OUT);
    assert(strstr(out, "line without a reason: " SRC "/nav.c step-9 2\n") != NULL);
    assert(strstr(out, "empty reason: " SRC "/nav.c\n") != NULL);
    assert(strstr(out, "unknown owner step-8: " SRC "/nav.c\n") != NULL);
    assert(strstr(out, "unknown owner step-14: " SRC "/nav.c\n") != NULL);
    assert(strstr(out, "count not a positive integer: " SRC "/cheat.h fixed 0") != NULL);
    assert(strstr(out, "not `<path> <owner> <count> | <reason>`: " SRC "/hud.c 2") != NULL);
    assert(strstr(out, "listed file missing: " SRC "/gone.c\n") != NULL);
    assert(strstr(out, "duplicate line: " SRC "/nav.c step-10\n") != NULL);
    assert(strstr(out, "button-guard ") == NULL);
}

static void test_empty_allowance_is_an_error_not_a_pass(void) {
    setup();
    write_file(LIST, "# only a comment\n\n");
    assert(run_guard() == 2);
    assert(strstr(read_file(OUT), "empty list\n") != NULL);
}

static void test_missing_directory_is_an_error(void) {
    setup();
    assert(system("sh tools/button_guard.sh " LIST " " DIR "/none > " OUT " 2>&1; echo $? > " DIR "/code") == 0);
    assert(atoi(read_file(DIR "/code")) == 2);
    assert(strstr(read_file(OUT), "directory not found: " DIR "/none\n") != NULL);
}

int main(void) {
    assert(system("mkdir -p " SRC) == 0);
    test_tree_matching_the_allowance_passes();
    test_count_changes_fail_naming_the_file_and_both_counts();
    test_stale_line_with_a_higher_count_fails();
    test_names_in_comments_literals_and_longer_identifiers_are_not_reads();
    test_invalid_allowance_lines_are_errors();
    test_empty_allowance_is_an_error_not_a_pass();
    test_missing_directory_is_an_error();
    assert(system("rm -rf " DIR) == 0);
    printf("test_button_guard: all tests passed\n");
    return 0;
}
