/* tools/class_coverage.sh, le corps de `make class-coverage`, sur un dossier
 * fabrique : un rejeu court valide et un fichier malforme. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/class_coverage.sh ./tmp/pico8_class_coverage"
#define DIR "tmp/test_class_coverage_work"
#define LOG DIR "/log.txt"
#define EXEMPT DIR "/exempt.txt"

static char *read_log(void) {
    FILE *file = fopen(LOG, "rb");
    assert(file != NULL);
    static char buf[1 << 16];
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
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

static int run(const char *exempt) {
    char cmd[512];
    snprintf(cmd, sizeof cmd, SCRIPT " " DIR "/replays %s > " LOG " 2>&1", exempt);
    return system(cmd);
}

static void test_short_replay_and_malformed_file(void) {
    assert(run("") != 0);
    char *log = read_log();
    assert(strstr(log, "illisible: " DIR "/replays/bad_events.json") != NULL);
    assert(count_of(log, "jo") > 0);
    assert(count_of(log, "ui") > 0);
    assert(count_of(log, "nl") == 0);
    assert(strstr(log, "\nnl  0  0/1") != NULL);
}

/* Une exemption pour chaque classe a 0 : le verdict passe. */
static void test_exempting_every_zero_class_passes(void) {
    assert(system("awk '$2 == 0 && $1 != \"inconnue\" { print $1, \"test\" }' " LOG
                  " > " EXEMPT) == 0);
    assert(run(EXEMPT) == 0);
    char *log = read_log();
    assert(strstr(log, "exemptée") != NULL);
}

int main(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR "/replays") == 0);
    assert(system("cp tests/fixtures/screencast_capture_events.json " DIR "/replays/short_events.json") == 0);
    assert(system("cp tests/fixtures/events_malformed_missing_field.json " DIR "/replays/bad_events.json") == 0);
    test_short_replay_and_malformed_file();
    test_exempting_every_zero_class_passes();
    assert(system("rm -rf " DIR) == 0);
    printf("test_class_coverage: all tests passed\n");
    return 0;
}
