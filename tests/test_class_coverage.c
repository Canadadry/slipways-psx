/* tools/class_coverage.sh, le corps de `make class-coverage`, sur un dossier
 * fabrique : un rejeu court depuis l'ecran titre et un fichier malforme. Puis
 * le compteur de couverture, seul. */

#include "devtools/class_coverage.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_class_coverage_work"
#define LOG DIR "/log.txt"
#define EXEMPT DIR "/exempt.txt"
#define EXPECTED DIR "/expected.txt"

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    static char buf[1 << 16];
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static char *read_log(void) {
    return read_file(LOG);
}

/* Rend le compte total imprime pour `name`, -1 si la ligne manque. */
static long count_of(const char *log, const char *name) {
    char prefix[32];
    snprintf(prefix, sizeof prefix, "\n%s ", name);
    const char *line = strstr(log, prefix);
    if (line == NULL) {
        return -1;
    }
    return strtol(line + strlen(prefix), NULL, 10);
}

static int run_with(const char *options, const char *exempt) {
    char cmd[512];
    snprintf(cmd, sizeof cmd, "sh tools/class_coverage.sh %s ./tmp/pico8_class_coverage " DIR "/replays %s > " LOG
             " 2>&1", options, exempt);
    return system(cmd);
}

static int run(const char *exempt) {
    return run_with("", exempt);
}

static void test_short_replay_and_malformed_file(void) {
    assert(run("") != 0);
    char *log = read_log();
    assert(strstr(log, "illisible: " DIR "/replays/bad_events.json") != NULL);
    assert(count_of(log, "jo") > 0);
    assert(count_of(log, "ui") > 0);
    assert(count_of(log, "nl") == 0);
    assert(strstr(log, "\nnl  0  0/1") != NULL);
    assert(strstr(log, "inconnue") == NULL);
}

/* Une exemption pour chaque classe a 0 : le verdict passe. */
static void test_exempting_every_zero_class_passes(void) {
    assert(system("awk '$2 == 0 { print $1, \"test\" }' " LOG " > " EXEMPT) == 0);
    assert(run(EXEMPT) == 0);
    char *log = read_log();
    assert(strstr(log, "exemptée") != NULL);
}

static void test_expect_equal_output_passes_unchanged(void) {
    assert(run(EXEMPT) == 0);
    assert(system("cp " LOG " " EXPECTED) == 0);
    assert(run_with("--expect " EXPECTED, EXEMPT) == 0);
    assert(system("cmp -s " LOG " " EXPECTED) == 0);
}

static void test_expect_one_count_off_fails(void) {
    assert(system("sed 's/^jo  \\([0-9]*\\)/jo  1\\1/' " LOG " > " EXPECTED) == 0);
    assert(system("cmp -s " LOG " " EXPECTED) != 0);
    assert(run_with("--expect " EXPECTED, EXEMPT) != 0);
    assert(strstr(read_log(), "sortie différente") != NULL);
}

static void test_expect_missing_file_fails(void) {
    assert(run_with("--expect " DIR "/absent.txt", EXEMPT) != 0);
}

/* Les cas s'enchainent dans un seul processus : les comptes s'accumulent.
 * Le nom "" vient en dernier, restant compte ensuite. */
static void test_coverage_hook_first_unknown_name(void) {
    static const char *const known_aa[] = {"aa"};

    pico8_class_coverage_record("aa");
    pico8_class_coverage_record("aa");
    assert(pico8_class_coverage_count("aa") == 2);
    assert(pico8_class_coverage_count("zz") == 0);
    assert(pico8_class_coverage_first_unknown(known_aa, 1) == NULL);
    assert(strcmp(pico8_class_coverage_first_unknown(NULL, 0), "aa") == 0);

    pico8_class_coverage_record("");
    assert(strcmp(pico8_class_coverage_first_unknown(known_aa, 1), "") == 0);
}

int main(void) {
    test_coverage_hook_first_unknown_name();

    assert(system("rm -rf " DIR " && mkdir -p " DIR "/replays") == 0);
    assert(system("cp tests/fixtures/title_click_forgiving.json " DIR "/replays/short_events.json") == 0);
    assert(system("cp tests/fixtures/events_malformed_missing_field.json " DIR "/replays/bad_events.json") == 0);
    test_short_replay_and_malformed_file();
    test_exempting_every_zero_class_passes();
    test_expect_equal_output_passes_unchanged();
    test_expect_one_count_off_fails();
    test_expect_missing_file_fails();
    assert(system("rm -rf " DIR) == 0);
    printf("test_class_coverage: all tests passed\n");
    return 0;
}
