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
    write_file(TST "/c6.c", "int b = field_" "truthy(e, \"x\");\n");
    write_file(TST "/c7.c", "pico8_" "mv(a, b);\n");
    write_file(TST "/c8.c", "int over" "rides;\n");
    write_file(TST "/c9.c", "int k = g_jo_" "spec[0];\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "by-name\t1\t1\n"
                                  "dispatch\t1\t1\n"
                                  "over" "rides\t1\t1\n"
                                  "spec\t1\t1\n"
                                  "residue D: 4 line(s), 4 file(s)\n") == 0);
}

static void test_overlapping_categories_count_each_but_footer_counts_distinct_lines(void) {
    setup();
    write_file(SRC "/e.c", "int b = field_" "truthy(planet, \"x\");\n"
                           "int k = g_planet_" "spec[0];\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "by-name\t1\t1\n"
                                  "spec\t1\t1\n"
                                  "residue D: 2 line(s), 1 file(s)\n") == 0);
    write_file(SRC "/e.c", "int k = g_planet_" "spec[field_" "truthy(planet, \"x\")];\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "by-name\t1\t1\n"
                                  "spec\t1\t1\n"
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
                           "int f = jo_" "default_fixed(x);\n"
                           "hud_" "print_outlined_default(x);\n");
    assert(run_residue(TREE) == 0);
    assert(strcmp(read_file(OUT), "spec\t1\t1\n"
                                  "residue D: 1 line(s), 1 file(s)\n") == 0);
}

static void test_word_boundaries_and_value_truthy(void) {
    setup();
    write_file(SRC "/w.c", "int over" "ridesx;\n"
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

static void test_detail_sorts_by_category_then_file(void) {
    setup();
    write_file(SRC "/b.c", "int k = g_planet_" "spec[1];\n");
    write_file(SRC "/a.c", "int k = g_planet_" "spec[0];\nint b = field_" "truthy(planet, \"x\");\n");
    assert(run_residue("--detail " TREE) == 0);
    const char *expected = "by-name\t" SRC "/a.c\t1\n"
                           "spec\t" SRC "/a.c\t1\n"
                           "spec\t" SRC "/b.c\t1\n"
                           "residue D: 3 line(s), 2 file(s)\n";
    assert(strcmp(read_file(OUT), expected) == 0);
    assert(system("cp " OUT " " DIR "/one.txt") == 0);
    assert(run_residue("--detail " TREE) == 0);
    assert(system("cmp -s " OUT " " DIR "/one.txt") == 0);
}

static void test_check_fails_only_on_a_named_category_present(void) {
    setup();
    write_file(SRC "/r.c", "int a;\nint b = field_" "truthy(planet, \"x\");\n");
    assert(run_residue("--check by-name " TREE) == 1);
    const char *out = read_file(OUT);
    assert(strstr(out, "by-name: " SRC "/r.c:2:int b = field_" "truthy(planet, \"x\");\n") != NULL);
    assert(strstr(out, "guarded residue (by-name): 1 line(s)\n") != NULL);
    assert(run_residue("--check spec " TREE) == 0);
    assert(strcmp(read_file(OUT), "guarded residue (spec): 0 line(s)\n") == 0);
    assert(run_residue("--check spec,by-name " TREE) == 1);
    assert(run_residue("--check nope " TREE) == 2);
    assert(strstr(read_file(ERR), "nope") != NULL);
    assert(run_residue("--check '' " TREE) == 2);
    assert(run_residue("--check spec,,by-name " TREE) == 2);
    assert(run_residue("--check") == 2);
}

static void test_retired_categories_are_unknown(void) {
    static const char *const retired[] = {"value", "ob", "stash", "type-tag", "ref-audit", "list"};
    for (size_t i = 0; i < sizeof retired / sizeof retired[0]; i++) {
        char args[256];
        char message[256];
        setup();
        snprintf(args, sizeof args, "--check %s " TREE, retired[i]);
        snprintf(message, sizeof message, "unknown category: %s\n", retired[i]);
        assert(run_residue(args) == 2);
        assert(strstr(read_file(ERR), message) != NULL);
    }
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

typedef struct {
    const char *category;
    const char *path;
    const char *text;
    const char *expected;
} liveness_row_t;

/* One planted violation per category of DATA_RESIDUE_GUARD, in the C names
 * of today: a renaming that blinds a pattern turns its row red. `expected`
 * is the whole output of `--check <category>`. dispatch, and two branches
 * of by-name, only know `pico8_` tokens: their rows plant them among names
 * of today. */
static const liveness_row_t LIVENESS[] = {
    {"spec", SRC "/planet.c",
     "int k = g_planet_" "spec[0];\n"
     "int f = planet_" "default_fixed(k);\n",
     "spec: " SRC "/planet.c:1:int k = g_planet_" "spec[0];\n"
     "spec: " SRC "/planet.c:2:int f = planet_" "default_fixed(k);\n"
     "guarded residue (spec): 2 line(s)\n"},
    {"over" "rides", SRC "/planet.c", "planet->over" "rides = 0;\n",
     "over" "rides: " SRC "/planet.c:1:planet->over" "rides = 0;\n"
     "guarded residue (over" "rides): 1 line(s)\n"},
    {"dispatch", SRC "/planet.c",
     "int n = pico8_" "mv(planet, slipway);\n"
     "pico8_" "value_function_t draw = planet_draw;\n",
     "dispatch: " SRC "/planet.c:1:int n = pico8_" "mv(planet, slipway);\n"
     "dispatch: " SRC "/planet.c:2:pico8_" "value_function_t draw = planet_draw;\n"
     "guarded residue (dispatch): 2 line(s)\n"},
    {"by-name", SRC "/planet.c",
     "int b = field_" "truthy(planet, \"colonized\");\n"
     "pico8_" "entity_get_field(planet, \"colonized\", &b);\n"
     "pico8_" "planet_set_field(planet, \"colonized\", b);\n",
     "by-name: " SRC "/planet.c:1:int b = field_" "truthy(planet, \"colonized\");\n"
     "by-name: " SRC "/planet.c:2:pico8_" "entity_get_field(planet, \"colonized\", &b);\n"
     "by-name: " SRC "/planet.c:3:pico8_" "planet_set_field(planet, \"colonized\", b);\n"
     "guarded residue (by-name): 3 line(s)\n"},
};

#define LIVENESS_ROWS (sizeof LIVENESS / sizeof LIVENESS[0])

static void test_each_guarded_category_bites_on_its_planted_violation(void) {
    int red = 0;
    for (size_t i = 0; i < LIVENESS_ROWS; i++) {
        const liveness_row_t *row = &LIVENESS[i];
        char args[256];
        snprintf(args, sizeof args, "--check %s " TREE, row->category);
        setup();
        int clean = run_residue(args);
        write_file(row->path, row->text);
        int planted = run_residue(args);
        if (clean != 0 || planted != 1 || strcmp(read_file(OUT), row->expected) != 0) {
            fprintf(stderr, "liveness red: %s\n", row->category);
            red++;
        }
    }
    assert(red == 0 && "a guarded category no longer sees its planted violation");
}

/* Copies into `out` the value of `<variable> := ` in the Makefile, without
 * its newline; fails if the line is missing. */
static void makefile_value(const char *variable, char *out, size_t size) {
    char prefix[128];
    char line[1024];
    snprintf(prefix, sizeof prefix, "%s := ", variable);
    FILE *file = fopen("Makefile", "r");
    assert(file != NULL);
    int found = 0;
    while (!found && fgets(line, sizeof line, file) != NULL) {
        if (strncmp(line, prefix, strlen(prefix)) == 0) {
            snprintf(out, size, "%s", line + strlen(prefix));
            out[strcspn(out, "\n")] = '\0';
            found = 1;
        }
    }
    fclose(file);
    assert(found && "guard variable missing from the Makefile");
}

static int in_list(const char *name, const char *const *list, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (strcmp(name, list[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* A category added to the guard without a liveness row, or a row left
 * behind by a category leaving the guard, turns this red. */
static void test_liveness_rows_are_exactly_the_guarded_categories(void) {
    char guard[1024];
    makefile_value("DATA_RESIDUE_GUARD", guard, sizeof guard);
    const char *rows[LIVENESS_ROWS];
    for (size_t i = 0; i < LIVENESS_ROWS; i++) {
        rows[i] = LIVENESS[i].category;
    }
    size_t guarded = 0;
    for (char *name = strtok(guard, ","); name != NULL; name = strtok(NULL, ",")) {
        if (!in_list(name, rows, LIVENESS_ROWS)) {
            fprintf(stderr, "liveness table: %s\n", name);
        }
        assert(in_list(name, rows, LIVENESS_ROWS) && "guarded category without a liveness row");
        guarded++;
    }
    assert(guarded == LIVENESS_ROWS);
}

int main(void) {
    test_each_guarded_category_bites_on_its_planted_violation();
    test_liveness_rows_are_exactly_the_guarded_categories();
    test_clean_tree_prints_only_the_empty_footer();
    test_one_occurrence_per_category_in_table_order();
    test_overlapping_categories_count_each_but_footer_counts_distinct_lines();
    test_omitted_argument_defaults_are_not_class_specs();
    test_word_boundaries_and_value_truthy();
    test_detail_sorts_by_category_then_file();
    test_check_fails_only_on_a_named_category_present();
    test_retired_categories_are_unknown();
    test_measure_on_nothing_is_an_error_not_an_empty_residue();
    assert(system("rm -rf " DIR) == 0);
    printf("test_data_residue: all tests passed\n");
    return 0;
}
