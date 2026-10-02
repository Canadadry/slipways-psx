/* tools/float_guard.sh, le corps de `make float-guard`, sur des sources
 * fabriquees : il se teste par ce qu'il doit empecher, un fichier flottant
 * que la garde laisserait passer. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_float_guard_work"
#define FLOATING DIR "/pico8_floating.c"
#define INTEGER DIR "/pico8_integer.c"
#define OUT DIR "/out.txt"
#define GUARD "sh tools/float_guard.sh cc '-std=c99 -Wall -Wextra -Werror' " DIR "/obj"

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

/* Rend le code de sortie du script, relu depuis un fichier : le statut brut
 * de system() n'est pas decodable sans <sys/wait.h>, hors C99. */
static int run_guard(const char *sources) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, GUARD " %s > " OUT " 2>&1; echo $? > " DIR "/code", sources);
    assert(system(cmd) == 0);
    return atoi(read_file(DIR "/code"));
}

static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(FLOATING, "double pico8_half(int x) { return x / 2.0; }\n");
    write_file(INTEGER, "int pico8_half(int x) { return x / 2; }\n");
}

static void test_floating_file_is_listed_and_fails_the_guard(void) {
    setup();
    assert(run_guard(FLOATING " " INTEGER) == 1);
    const char *out = read_file(OUT);
    assert(strstr(out, "failed " FLOATING "\n") != NULL);
    assert(strstr(out, INTEGER) == NULL);
    assert(strstr(out, "float-guard files 2 failing 1\n") != NULL);
}

static void test_integer_file_alone_passes_the_guard(void) {
    setup();
    assert(run_guard(INTEGER) == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "\nfailed ") == NULL);
    assert(strstr(out, "float-guard files 1 failing 0\n") != NULL);
}

static void test_every_failing_file_is_listed_sorted(void) {
    setup();
    write_file(DIR "/pico8_another.c", "float pico8_third(void) { return 3.0f; }\n");
    assert(run_guard(FLOATING " " INTEGER " " DIR "/pico8_another.c") == 1);
    const char *out = read_file(OUT);
    const char *another = strstr(out, "failed " DIR "/pico8_another.c\n");
    const char *floating = strstr(out, "failed " FLOATING "\n");
    assert(another != NULL && floating != NULL && another < floating);
    assert(strstr(out, "float-guard files 3 failing 2\n") != NULL);
}

static void test_no_source_is_an_error_not_a_pass(void) {
    setup();
    assert(run_guard("") == 2);
}

static void test_two_runs_give_identical_reports(void) {
    setup();
    run_guard(FLOATING " " INTEGER);
    assert(system("cp " OUT " " DIR "/one.txt") == 0);
    run_guard(FLOATING " " INTEGER);
    assert(system("cmp -s " OUT " " DIR "/one.txt") == 0);
}

int main(void) {
    test_floating_file_is_listed_and_fails_the_guard();
    test_integer_file_alone_passes_the_guard();
    test_every_failing_file_is_listed_sorted();
    test_no_source_is_an_error_not_a_pass();
    test_two_runs_give_identical_reports();
    assert(system("rm -rf " DIR) == 0);
    printf("test_float_guard: all tests passed\n");
    return 0;
}
