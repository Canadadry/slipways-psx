/* tools/data_residue.sh, the body of `make data-residue` and of the category
 * check of `make data-guard`, on a small built tree.
 * The script reads tests/: each fixture token is split into two literals,
 * or this file would enter the residue it measures. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_data_residue_work"
#define SRC DIR "/src"
#define TST DIR "/tests"
#define OUT DIR "/out.txt"
#define ERR DIR "/err.txt"
#define TREE SRC " " TST
#define EMPTY_FOOTER "residue D: 0 line(s), 0 file(s)\n"

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
 * system() cannot be decoded without <sys/wait.h>, outside C99. The standard
 * output goes to OUT, the error to ERR. */
static int run_residue(const char *args) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, "sh tools/data_residue.sh %s > " OUT " 2> " ERR "; echo $? > " DIR "/code", args);
    assert(system(cmd) == 0);
    return atoi(read_file(DIR "/code"));
}

static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " SRC " " TST) == 0);
    write_file(SRC "/clean.c", "int main(void) { return 0; }\n");
    write_file(TST "/clean_test.c", "int helper(int x) { return x + 1; }\n");
}

static void test_clean_tree_prints_only_the_empty_footer(void) {
    setup();
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), EMPTY_FOOTER) == 0);
}

static void test_one_occurrence_per_category_in_table_order(void) {
    setup();
    write_file(SRC "/c1.c", "pico8_" "value_t v;\n");
    write_file(SRC "/c2.c", "pico8_" "ob_table_t *t;\n");
    write_file(SRC "/c3.c", "char *s = pico8_" "unstash(x);\n");
    write_file(SRC "/c4.h", "pico8_" "type_tag_t tag;\n");
    write_file(SRC "/c5.c", "#include \"pico8_" "ref_audit.h\"\n");
    write_file(TST "/c6.c", "int b = field_" "truthy(e, \"x\");\n");
    write_file(TST "/c7.c", "pico8_" "mv(a, b);\n");
    write_file(TST "/c8.c", "int over" "rides;\n");
    write_file(TST "/c9.c", "int k = g_jo_" "spec[0];\n");
    write_file(TST "/c10.c", "pico8_" "list_t l;\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "value\t1\t1\n"
                                  "ob\t1\t1\n"
                                  "stash\t1\t1\n"
                                  "type-tag\t1\t1\n"
                                  "ref-audit\t1\t1\n"
                                  "by-name\t1\t1\n"
                                  "dispatch\t1\t1\n"
                                  "over" "rides\t1\t1\n"
                                  "spec\t1\t1\n"
                                  "list\t1\t1\n"
                                  "residue D: 10 line(s), 10 file(s)\n") == 0);
}

static void test_overlapping_categories_count_each_but_footer_counts_distinct_lines(void) {
    setup();
    write_file(SRC "/e.c", "pico8_" "entity_get_field(e, \"x\", &v);\n"
                           "pico8_" "value_t v;\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "value\t1\t1\n"
                                  "by-name\t1\t1\n"
                                  "residue D: 2 line(s), 1 file(s)\n") == 0);
    write_file(SRC "/e.c", "pico8_" "value_t v = pico8_" "entity_get_field(e, \"x\");\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "value\t1\t1\n"
                                  "by-name\t1\t1\n"
                                  "residue D: 1 line(s), 1 file(s)\n") == 0);
}

static void test_omitted_argument_defaults_are_not_class_specs(void) {
    setup();
    write_file(SRC "/g.c", "p8_" "spr_default(gfx, mem, 1, 0, 0);\n"
                           "hud_" "print_outlined_default(x);\n"
                           "hud_" "draw_bar_default(gfx, mem, y1, y2);\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), EMPTY_FOOTER) == 0);
    write_file(SRC "/g.c", "p8_" "spr_default(gfx, mem, 1, 0, 0);\n"
                           "pico8_" "jo_default(\"nu\", &k);\n"
                           "int f = jo_" "default_fixed(x);\n"
                           "hud_" "print_outlined_default(x);\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "spec\t2\t1\n"
                                  "residue D: 2 line(s), 1 file(s)\n") == 0);
}

static void test_word_boundaries_and_value_truthy(void) {
    setup();
    write_file(SRC "/w.c", "int mypico8_" "value_t;\n"
                           "int over" "ridesx;\n"
                           "int fields;\n"
                           "int t = pico8_" "jo_truthy(v);\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), EMPTY_FOOTER) == 0);
    write_file(SRC "/w.c", "self->" "field(self, n);\n"
                           "static k_t kind = { ." "set_field = f };\n"
                           "int b = field_" "truthy(e, \"x\");\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "by-name\t3\t1\n"
                                  "residue D: 3 line(s), 1 file(s)\n") == 0);
}

static void test_list_word_boundaries(void) {
    setup();
    write_file(SRC "/l.c", "int mypico8_" "list_t;\n"
                           "int pico8_" "listing;\n"
                           "pico8_" "ptrs_t p;\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), EMPTY_FOOTER) == 0);
    write_file(SRC "/l.c", "#include \"pico8_" "list.h\"\n"
                           "int n = PICO8_" "REF_AS_LIST_CONST(v)->count;\n"
                           "int t = PICO8_" "TYPE_TAG_LIST;\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "type-tag\t1\t1\n"
                                  "ref-audit\t1\t1\n"
                                  "list\t3\t1\n"
                                  "residue D: 3 line(s), 1 file(s)\n") == 0);
}

static void test_detail_sorts_by_category_then_file(void) {
    setup();
    write_file(SRC "/b.c", "pico8_" "value_t b;\n");
    write_file(SRC "/a.c", "pico8_" "value_t a;\npico8_" "ob_table_t *t;\n");
    assert(run_residue("--detail " TREE) == 0);
    const char *expected = "value\t" SRC "/a.c\t1\n"
                           "value\t" SRC "/b.c\t1\n"
                           "ob\t" SRC "/a.c\t1\n"
                           "residue D: 3 line(s), 2 file(s)\n";
    assert(strcmp(read_file(OUT), expected) == 0);
    assert(system("cp " OUT " " DIR "/one.txt") == 0);
    assert(run_residue("--detail " TREE) == 0);
    assert(system("cmp -s " OUT " " DIR "/one.txt") == 0);
}

static void test_check_fails_only_on_a_named_category_present(void) {
    setup();
    write_file(SRC "/r.c", "int a;\n#include \"pico8_" "ref_audit.h\"\n");
    assert(run_residue("--check ref-audit " TREE) == 1);
    const char *out = read_file(OUT);
    assert(strstr(out, "ref-audit: " SRC "/r.c:2:#include \"pico8_" "ref_audit.h\"\n") != NULL);
    assert(strstr(out, "guarded residue (ref-audit): 1 line(s)\n") != NULL);
    assert(run_residue("--check spec " TREE) == 0);
    assert(strcmp(read_file(OUT), "guarded residue (spec): 0 line(s)\n") == 0);
    assert(run_residue("--check spec,ref-audit " TREE) == 1);
    assert(run_residue("--check nope " TREE) == 2);
    assert(strstr(read_file(ERR), "nope") != NULL);
    assert(run_residue("--check '' " TREE) == 2);
    assert(run_residue("--check spec,,ref-audit " TREE) == 2);
    assert(run_residue("--check") == 2);
}

static void test_measure_on_nothing_is_an_error_not_an_empty_residue(void) {
    setup();
    assert(run_residue(SRC " " DIR "/absent") == 2);
    assert(strstr(read_file(OUT), "residue D: 0") == NULL);
    assert(system("mkdir -p " DIR "/bare && echo x > " DIR "/bare/notes.txt") == 0);
    assert(run_residue(DIR "/bare") == 2);
    assert(strstr(read_file(OUT), "residue D: 0") == NULL);
    assert(run_residue("--check spec " DIR "/bare") == 2);
    assert(run_residue("--bogus " TREE) == 2);
}

int main(void) {
    test_clean_tree_prints_only_the_empty_footer();
    test_one_occurrence_per_category_in_table_order();
    test_overlapping_categories_count_each_but_footer_counts_distinct_lines();
    test_omitted_argument_defaults_are_not_class_specs();
    test_word_boundaries_and_value_truthy();
    test_list_word_boundaries();
    test_detail_sorts_by_category_then_file();
    test_check_fails_only_on_a_named_category_present();
    test_measure_on_nothing_is_an_error_not_an_empty_residue();
    assert(system("rm -rf " DIR) == 0);
    printf("test_data_residue: all tests passed\n");
    return 0;
}
