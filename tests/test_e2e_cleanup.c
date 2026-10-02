/* tools/e2e_cleanup.sh, la passe de nettoyage finale, sur des ensembles de
 * lignes fabriques. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/e2e_cleanup.sh"
#define DIR "tmp/test_e2e_cleanup_work"
#define OUT DIR "/out.txt"

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Tampon statique, ecrase a chaque appel. */
static const char *read_out(void) {
    FILE *file = fopen(OUT, "rb");
    assert(file != NULL);
    static char buf[1 << 16];
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static int run(const char *args) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " %s > " OUT " 2> " DIR "/log.txt", args);
    return system(cmd);
}

static void test_two_identical_files_leave_only_the_shorter(void) {
    write_file(DIR "/a.lines", "src/a.c:1\nsrc/a.c:2\n");
    write_file(DIR "/b.lines", "src/a.c:1\nsrc/a.c:2\n");
    assert(run("short 12 " DIR "/a.lines long 40 " DIR "/b.lines") == 0);
    assert(strcmp(read_out(), "long\n") == 0);
    assert(run("long 40 " DIR "/b.lines short 12 " DIR "/a.lines") == 0);
    assert(strcmp(read_out(), "long\n") == 0);
}

/* `long` couvre seul src/b.c:9 : il reste, meme le plus long ; `mid`,
 * couvert par `long` et `short`, part. */
static void test_file_alone_to_cover_a_line_is_never_removed(void) {
    write_file(DIR "/long.lines", "src/a.c:1\nsrc/b.c:9\n");
    write_file(DIR "/mid.lines", "src/a.c:1\nsrc/a.c:2\n");
    write_file(DIR "/short.lines", "src/a.c:2\n");
    assert(run("long 90 " DIR "/long.lines mid 50 " DIR "/mid.lines short 10 " DIR "/short.lines") == 0);
    assert(strcmp(read_out(), "mid\n") == 0);
}

/* Du plus long au plus court : c et b sont chacun couverts par l'union des
 * autres au depart, mais retirer le plus long (c) d'abord garde b, qui porte
 * alors seul src/x.c:3. */
static void test_longest_to_shortest_order_is_respected(void) {
    write_file(DIR "/a.lines", "src/x.c:1\nsrc/x.c:2\n");
    write_file(DIR "/b.lines", "src/x.c:2\nsrc/x.c:3\n");
    write_file(DIR "/c.lines", "src/x.c:1\nsrc/x.c:3\n");
    write_file(DIR "/d.lines", "src/x.c:9\n");
    write_file(DIR "/e.lines", "src/x.c:9\n");
    assert(run("a 10 " DIR "/a.lines d 5 " DIR "/d.lines b 20 " DIR "/b.lines c 30 " DIR "/c.lines e 7 "
               DIR "/e.lines") == 0);
    assert(strcmp(read_out(), "c\ne\n") == 0);
}

static void test_missing_set_or_bad_arguments_fail(void) {
    write_file(DIR "/a.lines", "src/a.c:1\n");
    assert(run("a 10 " DIR "/a.lines b 5 " DIR "/absent.lines") != 0);
    assert(run("a 10") != 0);
    assert(run("a ten " DIR "/a.lines") != 0);
}

int main(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    test_two_identical_files_leave_only_the_shorter();
    test_file_alone_to_cover_a_line_is_never_removed();
    test_longest_to_shortest_order_is_respected();
    test_missing_set_or_bad_arguments_fail();
    printf("test_e2e_cleanup: ok\n");
    return 0;
}
