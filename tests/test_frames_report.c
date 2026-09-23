/* tools/frames_report.sh, le rapport de divergence complet et la garde de
 * `make frames-baseline`, sur des fichiers d'empreintes fabriques. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/frames_report.sh"
#define DIR "tmp/test_frames_report_work"
#define OUT DIR "/out.txt"
#define ERR DIR "/err.txt"

#define A_EVENTS "alpha_events.json"
#define A_REF DIR "/alpha.ref"
#define A_CUR DIR "/alpha.cur"
#define B_EVENTS "beta_events.json"
#define B_REF DIR "/beta.ref"
#define B_CUR DIR "/beta.cur"

#define A_TRIPLET " " A_EVENTS " " A_REF " " A_CUR
#define B_TRIPLET " " B_EVENTS " " B_REF " " B_CUR

#define FRAMES 30

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

/* `lines` empreintes ; les frames dont `altered` contient l'indice (liste
 * terminee par -1, NULL pour aucune) recoivent une autre valeur. */
static void write_hashes(const char *path, int lines, const int *altered) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    for (int k = 0; k < lines; k++) {
        unsigned long long value = (unsigned long long)k;
        for (const int *a = altered; a != NULL && *a >= 0; a++) {
            if (*a == k) {
                value += 0xdead0000ULL;
            }
        }
        fprintf(file, "%016llx\n", value);
    }
    fclose(file);
}

static int run(const char *args) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), SCRIPT "%s > " OUT " 2> " ERR, args);
    return system(cmd);
}

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_hashes(A_REF, FRAMES, NULL);
    write_hashes(B_REF, FRAMES, NULL);
}

/* Rapport de beta seul, beta courant a `lines` lignes et `altered`. */
static const char *report_beta(int lines, const int *altered) {
    reset();
    write_hashes(B_CUR, lines, altered);
    assert(run(B_TRIPLET) == 0);
    return read_file(OUT);
}

static void test_conforming_file_is_reported_with_its_frame_count(void) {
    const char *out = report_beta(FRAMES, NULL);
    assert(strstr(out, B_EVENTS " : conforme (30 frames)\n") != NULL);
    assert(strstr(out, "DIVERGENT") == NULL);
}

static void test_isolated_frame_gives_a_single_frame_range(void) {
    const int altered[] = {17, -1};
    const char *out = report_beta(FRAMES, altered);
    assert(strstr(out, B_EVENTS " : DIVERGENT\n") != NULL);
    assert(strstr(out, "premiere frame divergente : 17\n") != NULL);
    assert(strstr(out, "frames divergentes : 1\n") != NULL);
    assert(strstr(out, "plages : 17\n") != NULL);
    assert(strstr(out, "longueur") == NULL);
}

static void test_contiguous_frames_give_one_range(void) {
    const int altered[] = {3, 4, 5, -1};
    const char *out = report_beta(FRAMES, altered);
    assert(strstr(out, "premiere frame divergente : 3\n") != NULL);
    assert(strstr(out, "frames divergentes : 3\n") != NULL);
    assert(strstr(out, "plages : 3-5\n") != NULL);
}

static void test_several_ranges_are_all_listed_in_order(void) {
    const int altered[] = {20, 3, 9, 4, 21, 5, -1};
    const char *out = report_beta(FRAMES, altered);
    assert(strstr(out, "frames divergentes : 6\n") != NULL);
    assert(strstr(out, "plages : 3-5 9 20-21\n") != NULL);
}

/* frames_check.sh s'arrete a 10 : le rapport, jamais. */
static void test_more_than_ten_frames_are_all_listed(void) {
    const int altered[] = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, -1};
    const char *out = report_beta(FRAMES, altered);
    assert(strstr(out, "frames divergentes : 12\n") != NULL);
    assert(strstr(out, "plages : 0 2 4 6 8 10 12 14 16 18 20 22\n") != NULL);
}

static void test_shorter_current_counts_missing_frames_as_divergent(void) {
    const char *out = report_beta(FRAMES - 2, NULL);
    assert(strstr(out, "premiere frame divergente : 28\n") != NULL);
    assert(strstr(out, "frames divergentes : 2\n") != NULL);
    assert(strstr(out, "plages : 28-29\n") != NULL);
    assert(strstr(out, "longueur : 30 en reference, 28 obtenue(s)\n") != NULL);
}

static void test_longer_current_counts_extra_frames_as_divergent(void) {
    const int altered[] = {1, -1};
    const char *out = report_beta(FRAMES + 3, altered);
    assert(strstr(out, "premiere frame divergente : 1\n") != NULL);
    assert(strstr(out, "frames divergentes : 4\n") != NULL);
    assert(strstr(out, "plages : 1 30-32\n") != NULL);
    assert(strstr(out, "longueur : 30 en reference, 33 obtenue(s)\n") != NULL);
}

static void test_missing_reference_is_reported_without_failing(void) {
    reset();
    assert(system("rm -f " A_REF) == 0);
    write_hashes(A_CUR, FRAMES, NULL);
    write_hashes(B_CUR, FRAMES, NULL);
    assert(run(A_TRIPLET B_TRIPLET) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, A_EVENTS " : aucune reference\n") != NULL);
    assert(strstr(out, B_EVENTS " : conforme (30 frames)\n") != NULL);
}

static void test_missing_current_is_an_error(void) {
    reset();
    write_hashes(B_CUR, FRAMES, NULL);
    assert(run(A_TRIPLET B_TRIPLET) != 0);
    assert(strstr(read_file(ERR), A_CUR) != NULL);
}

static void test_output_is_identical_from_run_to_run(void) {
    const int altered[] = {2, 3, 11, -1};
    reset();
    write_hashes(A_CUR, FRAMES, NULL);
    write_hashes(B_CUR, FRAMES + 1, altered);
    assert(run(" --etape C2b" A_TRIPLET B_TRIPLET) == 0);
    assert(system("cp " OUT " " DIR "/first.txt") == 0);
    assert(run(" --etape C2b" A_TRIPLET B_TRIPLET) == 0);
    assert(system("cmp -s " OUT " " DIR "/first.txt") == 0);
}

static void test_named_step_heads_the_report(void) {
    reset();
    write_hashes(A_CUR, FRAMES, NULL);
    assert(run(" --etape D1" A_TRIPLET) == 0);
    assert(strncmp(read_file(OUT), "rapport de divergence, etape D1\n", 32) == 0);
}

static void test_step_that_is_not_a_plain_name_is_refused(void) {
    reset();
    write_hashes(A_CUR, FRAMES, NULL);
    assert(run(" --etape ../x" A_TRIPLET) != 0);
    assert(run(" --etape ''" A_TRIPLET) != 0);
}

/* La garde de frames-baseline : une reference divergente sans etape nommee
 * est un refus, qui nomme le fichier et dit comment nommer l'etape. */
static void test_guard_refuses_a_divergent_reference_without_step(void) {
    const int altered[] = {5, -1};
    reset();
    write_hashes(A_CUR, FRAMES, NULL);
    write_hashes(B_CUR, FRAMES, altered);
    assert(run(" --garde ''" A_TRIPLET B_TRIPLET) != 0);
    const char *err = read_file(ERR);
    assert(strstr(err, B_EVENTS) != NULL);
    assert(strstr(err, A_EVENTS) == NULL);
    assert(strstr(err, "ETAPE=") != NULL);
}

static void test_guard_lets_a_named_step_through(void) {
    const int altered[] = {5, -1};
    reset();
    write_hashes(A_CUR, FRAMES, NULL);
    write_hashes(B_CUR, FRAMES, altered);
    assert(run(" --garde C2b" A_TRIPLET B_TRIPLET) == 0);
}

static void test_guard_lets_conforming_and_first_references_through(void) {
    reset();
    assert(system("rm -f " A_REF) == 0);
    write_hashes(A_CUR, FRAMES, NULL);
    write_hashes(B_CUR, FRAMES, NULL);
    assert(run(" --garde ''" A_TRIPLET B_TRIPLET) == 0);
}

int main(void) {
    test_conforming_file_is_reported_with_its_frame_count();
    test_isolated_frame_gives_a_single_frame_range();
    test_contiguous_frames_give_one_range();
    test_several_ranges_are_all_listed_in_order();
    test_more_than_ten_frames_are_all_listed();
    test_shorter_current_counts_missing_frames_as_divergent();
    test_longer_current_counts_extra_frames_as_divergent();
    test_missing_reference_is_reported_without_failing();
    test_missing_current_is_an_error();
    test_output_is_identical_from_run_to_run();
    test_named_step_heads_the_report();
    test_step_that_is_not_a_plain_name_is_refused();
    test_guard_refuses_a_divergent_reference_without_step();
    test_guard_lets_a_named_step_through();
    test_guard_lets_conforming_and_first_references_through();
    assert(system("rm -rf " DIR) == 0);
    printf("test_frames_report: all tests passed\n");
    return 0;
}
