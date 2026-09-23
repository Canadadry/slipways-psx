/* tools/frames_check.sh, le corps de `make frames-check`, sur des fichiers
 * d'empreintes fabriques : il se teste par ce qu'il doit empecher. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/frames_check.sh ./tmp/pico8_frames"
#define DIR "tmp/test_frames_check_work"
#define LOG DIR "/log.txt"

#define A_EVENTS "alpha_events.json"
#define A_REF DIR "/alpha.ref"
#define A_CUR DIR "/alpha.cur"
#define B_EVENTS "beta_events.json"
#define B_REF DIR "/beta.ref"
#define B_CUR DIR "/beta.cur"

#define ARGS " " A_EVENTS " " A_REF " " A_CUR " " DIR "/alpha.bin" \
             " " B_EVENTS " " B_REF " " B_CUR " " DIR "/beta.bin"

#define FRAMES 30

/* Tampon alloue, termine par NUL : l'appelant le libere. */
static char *read_log(void) {
    FILE *file = fopen(LOG, "rb");
    assert(file != NULL);
    static char buf[1 << 16];
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    char *copy = malloc(len + 1);
    assert(copy != NULL);
    memcpy(copy, buf, len + 1);
    return copy;
}

/* Une empreinte par ligne ; `salt` distingue les deux rejeux, et la frame
 * `altered` (ou aucune si -1) recoit une autre valeur. `lines` fixe le
 * nombre de lignes. */
static void write_hashes(const char *path, int salt, int altered, int lines) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    for (int k = 0; k < lines; k++) {
        unsigned long long value = (unsigned long long)(salt * 1000 + k);
        if (k == altered) {
            value += 0xdead0000ULL;
        }
        fprintf(file, "%016llx\n", value);
    }
    fclose(file);
}

static int run_check(void) {
    return system(SCRIPT ARGS " > " LOG " 2>&1");
}

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_hashes(A_CUR, 1, -1, FRAMES);
    write_hashes(B_CUR, 2, -1, FRAMES);
}

static void test_up_to_date_references_pass(void) {
    reset();
    write_hashes(A_REF, 1, -1, FRAMES);
    write_hashes(B_REF, 2, -1, FRAMES);
    assert(run_check() == 0);
    char *log = read_log();
    assert(strstr(log, A_EVENTS) != NULL);
    assert(strstr(log, B_EVENTS) != NULL);
    assert(strstr(log, "DIVERGENT") == NULL);
    free(log);
}

static void test_one_altered_line_fails_naming_only_that_replay_and_frame(void) {
    reset();
    write_hashes(A_REF, 1, -1, FRAMES);
    write_hashes(B_REF, 2, 17, FRAMES);
    assert(run_check() != 0);
    char *log = read_log();
    assert(strstr(log, "DIVERGENT " B_EVENTS) != NULL);
    assert(strstr(log, "DIVERGENT " A_EVENTS) == NULL);
    assert(strstr(log, "1 frame(s) divergente(s)") != NULL);
    assert(strstr(log, ": 17") != NULL);
    assert(strstr(log, "pico8_frames frame " DIR "/beta.bin 17") != NULL);
    free(log);
}

/* Le premier rejeu divergent n'arrete pas l'examen : le second est
 * rapporte aussi. */
static void test_every_divergent_replay_is_reported(void) {
    reset();
    write_hashes(A_REF, 1, 0, FRAMES);
    write_hashes(B_REF, 2, FRAMES - 1, FRAMES);
    assert(run_check() != 0);
    char *log = read_log();
    assert(strstr(log, "DIVERGENT " A_EVENTS) != NULL);
    assert(strstr(log, "DIVERGENT " B_EVENTS) != NULL);
    free(log);
}

static void test_line_count_difference_is_reported(void) {
    reset();
    write_hashes(A_REF, 1, -1, FRAMES + 2);
    write_hashes(B_REF, 2, -1, FRAMES);
    assert(run_check() != 0);
    char *log = read_log();
    assert(strstr(log, "DIVERGENT " A_EVENTS) != NULL);
    assert(strstr(log, "32 ligne(s) en reference, 30 obtenue(s)") != NULL);
    free(log);
}

static void test_missing_reference_does_not_fail_and_says_how_to_set_it(void) {
    reset();
    write_hashes(B_REF, 2, -1, FRAMES);
    assert(run_check() == 0);
    char *log = read_log();
    assert(strstr(log, A_REF) != NULL);
    assert(strstr(log, "make frames-baseline") != NULL);
    free(log);

    write_hashes(B_REF, 2, 3, FRAMES);
    assert(run_check() != 0);
}

int main(void) {
    test_up_to_date_references_pass();
    test_one_altered_line_fails_naming_only_that_replay_and_frame();
    test_every_divergent_replay_is_reported();
    test_line_count_difference_is_reported();
    test_missing_reference_does_not_fail_and_says_how_to_set_it();
    assert(system("rm -rf " DIR) == 0);
    printf("test_frames_check: all tests passed\n");
    return 0;
}
