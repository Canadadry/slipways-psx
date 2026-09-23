/* tools/float_coverage.sh, le corps de `make float-coverage`, sur un
 * inventaire et une sortie gcov fabriques : il se teste par ce qu'il doit
 * empecher, un site non atteint qui passerait pour couvert. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/float_coverage.sh"
#define DIR "tmp/test_float_coverage_work"
#define INVENTORY DIR "/inventaire.txt"
#define GCOV DIR "/gcov.txt"
#define OUT DIR "/out.txt"
#define COMPILER "'cc (Fabrique) 1.2.3'"

/* Tampon statique, termine par NUL, ecrase a chaque appel. */
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

static int run_coverage(const char *out) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " " INVENTORY " " COMPILER " " GCOV " > %s 2>&1", out);
    return system(cmd);
}

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
}

#define INVENTORY_TEXT \
    "# en-tete de l'inventaire\n" \
    "src/pico8_a.c | pico8_one | constante | 1 | ligne 2 | - | 0.27\n" \
    "src/pico8_a.c | pico8_two | cast | 1 | ligne 7 | a auditer | pos.x\n" \
    "src/pico8_a.c | pico8_two | cast | 2 | ligne 7 | a auditer | f(pos.y)\n" \
    "src/pico8_a.c | pico8_two | reecrit | 1 | ligne 9 | - | pow\n" \
    "src/pico8_a.c | pico8_two | autre | 1 | ligne 10 | - | double\n" \
    "src/pico8_a.c | pico8_two | autre | 2 | ligne 11 | - | a | b\n" \
    "src/pico8_b.c | pico8_three | constante | 1 | ligne 3 | - | 1.5\n" \
    "src/pico8_c.c | pico8_four | reecrit | 1 | ligne 4 | - | %.4f\n" \
    "#\n" \
    "# Comptes\n" \
    "compte constante 2\n"

/* pico8_a.h s'intercale dans la sortie de pico8_a.c : ses lignes ne sont pas
 * celles de pico8_a.c. pico8_c.c n'a aucune sortie gcov. */
#define GCOV_TEXT \
    "        -:    0:Source:src/pico8_a.c\n" \
    "        -:    0:Graph:tmp/obj-coverage/src/pico8_a.gcno\n" \
    "        -:    0:Runs:1\n" \
    "       12:    2:    return pico8_fixed_from_double(0.27);\n" \
    "        3*:    7:    pico8_spr((int)pico8_fixed_to_double(pos.x), ...);\n" \
    "    #####:    9:    double d = pow(x, 0.75);\n" \
    "        -:   10:    double e;\n" \
    "        -:    0:Source:src/pico8_a.h\n" \
    "      999:   11:    return a | b;\n" \
    "        -:    0:Source:src/pico8_b.c\n" \
    "        -:    0:Runs:1\n" \
    "     1234:    3:    x = pico8_fixed_from_double(1.5);\n"

static void test_reached_site_reports_its_line_count(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(GCOV, GCOV_TEXT);
    assert(run_coverage(OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "src/pico8_a.c | pico8_one | constante | 1 | ligne 2 | passages 12 | - | 0.27\n") != NULL);
    assert(strstr(out, "src/pico8_b.c | pico8_three | constante | 1 | ligne 3 | passages 1234 | - | 1.5\n") != NULL);
}

static void test_two_sites_on_one_line_share_the_line_count(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(GCOV, GCOV_TEXT);
    assert(run_coverage(OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "| pico8_two | cast | 1 | ligne 7 | passages 3 | a auditer | pos.x\n") != NULL);
    assert(strstr(out, "| pico8_two | cast | 2 | ligne 7 | passages 3 | a auditer | f(pos.y)\n") != NULL);
}

static void test_unreached_or_unmeasured_site_is_never_silent(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(GCOV, GCOV_TEXT);
    assert(run_coverage(OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "| pico8_two | reecrit | 1 | ligne 9 | NON COUVERT (#####) | - | pow\n") != NULL);
    assert(strstr(out, "| pico8_two | autre | 1 | ligne 10 | NON COUVERT (ligne sans code ; ligne 9 : #####) | - | double\n") != NULL);
    assert(strstr(out, "| pico8_two | autre | 2 | ligne 11 | NON COUVERT (absente de gcov) | - | a | b\n") != NULL);
    assert(strstr(out, "src/pico8_c.c | pico8_four | reecrit | 1 | ligne 4 | NON COUVERT (absente de gcov) | - | %.4f\n") != NULL);
}

/* Une ligne sans code est le plus souvent la suite d'une instruction ecrite sur
 * plusieurs lignes : le rapport montre la ligne mesuree qui la precede, sans
 * pour autant la compter couverte. */
static void test_line_without_code_shows_the_preceding_measured_line(void) {
    reset();
    write_file(INVENTORY,
               "src/pico8_a.c | pico8_one | constante | 1 | ligne 5 | - | 1.15\n"
               "src/pico8_a.c | pico8_one | constante | 2 | ligne 2 | - | 0.5\n");
    write_file(GCOV,
               "        -:    0:Source:src/pico8_a.c\n"
               "        -:    1:static int x;\n"
               "        -:    2:    pico8_fixed_from_double(0.5);\n"
               "        7:    3:    skill = pico8_ov(a,\n"
               "        -:    4:                     b,\n"
               "        -:    5:                     pico8_fixed_from_double(1.15));\n");
    assert(run_coverage(OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "| ligne 5 | NON COUVERT (ligne sans code ; ligne 3 : 7) | - | 1.15\n") != NULL);
    assert(strstr(out, "| ligne 2 | NON COUVERT (ligne sans code) | - | 0.5\n") != NULL);
    assert(strstr(out, "couverture constante couverts 0 non-couverts 2\n") != NULL);
}

static void test_report_ends_with_covered_and_uncovered_per_category(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(GCOV, GCOV_TEXT);
    assert(run_coverage(OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "couverture constante couverts 2 non-couverts 0\n") != NULL);
    assert(strstr(out, "couverture cast couverts 2 non-couverts 0\n") != NULL);
    assert(strstr(out, "couverture reecrit couverts 0 non-couverts 2\n") != NULL);
    assert(strstr(out, "couverture autre couverts 0 non-couverts 2\n") != NULL);
    assert(strstr(out, "couverture total couverts 4 non-couverts 4\n") != NULL);
}

static void test_header_names_the_compiler_and_the_line_granularity(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(GCOV, GCOV_TEXT);
    assert(run_coverage(OUT) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "# Compilateur : cc (Fabrique) 1.2.3\n") != NULL);
    assert(strstr(out, "granularite") != NULL);
    assert(strstr(out, "compte constante") == NULL);
}

static void test_inventory_without_sites_stops_the_report(void) {
    reset();
    write_file(INVENTORY, "# rien\n");
    write_file(GCOV, GCOV_TEXT);
    assert(run_coverage(OUT) != 0);
    assert(strstr(read_file(OUT), "couverture total") == NULL);
}

static void test_two_runs_give_identical_reports(void) {
    reset();
    write_file(INVENTORY, INVENTORY_TEXT);
    write_file(GCOV, GCOV_TEXT);
    assert(run_coverage(DIR "/one.txt") == 0);
    assert(run_coverage(DIR "/two.txt") == 0);
    assert(system("cmp -s " DIR "/one.txt " DIR "/two.txt") == 0);
}

int main(void) {
    test_reached_site_reports_its_line_count();
    test_two_sites_on_one_line_share_the_line_count();
    test_unreached_or_unmeasured_site_is_never_silent();
    test_line_without_code_shows_the_preceding_measured_line();
    test_report_ends_with_covered_and_uncovered_per_category();
    test_header_names_the_compiler_and_the_line_granularity();
    test_inventory_without_sites_stops_the_report();
    test_two_runs_give_identical_reports();
    assert(system("rm -rf " DIR) == 0);
    printf("test_float_coverage: all tests passed\n");
    return 0;
}
