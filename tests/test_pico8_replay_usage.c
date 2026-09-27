/* Le pilote de rejeu n'a pas de fichier d'evenements par defaut : sans
 * lui, il affiche son usage et echoue au lieu de rejouer quelque chose
 * en silence. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define DRIVER "./tmp/pico8_replay"
#define STDERR_LOG "tmp/test_replay_usage_stderr.txt"

/* Tampon alloue, termine par un nul, que l'appelant libere. */
static char *read_whole_file(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    assert(fseek(file, 0, SEEK_END) == 0);
    long size = ftell(file);
    assert(size >= 0);
    rewind(file);
    char *buf = malloc((size_t)size + 1);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)size, file) == (size_t)size);
    buf[size] = '\0';
    fclose(file);
    return buf;
}

static void assert_usage_and_failure(const char *args) {
    char cmd[256];
    remove(STDERR_LOG);
    snprintf(cmd, sizeof cmd, DRIVER "%s > /dev/null 2> " STDERR_LOG, args);
    int status = system(cmd);
    assert(status != -1 && WIFEXITED(status));
    assert(WEXITSTATUS(status) != 0 && "no events file must be a failure");
    char *err = read_whole_file(STDERR_LOG);
    assert(strstr(err, "usage") != NULL && "the failure must print the usage");
    free(err);
}

static void test_no_argument_prints_usage_and_fails(void) {
    assert_usage_and_failure("");
}

static void test_flag_alone_prints_usage_and_fails(void) {
    assert_usage_and_failure(" --from-title");
}

int main(void) {
    test_no_argument_prints_usage_and_fails();
    test_flag_alone_prints_usage_and_fails();
    remove(STDERR_LOG);
    printf("test_pico8_replay_usage: all tests passed\n");
    return 0;
}
