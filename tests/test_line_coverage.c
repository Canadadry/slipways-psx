/* tools/line_coverage.sh, la mesure de couverture de lignes, sur des fichiers
 * d'evenements et des ensembles de lignes fabriques. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/line_coverage.sh"
#define COVER SCRIPT " cover ./tmp/pico8_replay_gcov tmp/obj-gcov"
#define DIR "tmp/test_line_coverage_work"
#define LOG DIR "/log.txt"
#define IDLE DIR "/idle_events.json"
#define CLICK DIR "/click_events.json"
#define BAD DIR "/bad_events.json"

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Tampon statique, ecrase a chaque appel. */
static const char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    static char buf[1 << 20];
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static long line_count(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    long lines = 0;
    int c;
    while ((c = fgetc(file)) != EOF) {
        lines += c == '\n';
    }
    fclose(file);
    return lines;
}

static int run(const char *args, const char *out) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, "%s > %s 2> " LOG, args, out);
    return system(cmd);
}

#define FRAME(x, y, b) \
    "{ \"mouse_x\": " #x ", \"mouse_y\": " #y ", \"mouse_buttons\": " #b \
    ", \"dpad\": [false, false, false, false] }"

static void write_fixtures(void) {
    write_file(IDLE, "{ \"frames\": [\n" FRAME(64, 64, 0) ",\n" FRAME(64, 64, 0) ",\n"
                     FRAME(64, 64, 0) ",\n" FRAME(64, 64, 0) "\n] }\n");
    write_file(CLICK, "{ \"frames\": [\n" FRAME(64, 64, 0) ",\n" FRAME(64, 64, 0) ",\n"
                      FRAME(64, 64, 0) ",\n" FRAME(64, 64, 0) ",\n"
                      FRAME(64, 64, 1) ",\n" FRAME(64, 64, 1) ",\n"
                      FRAME(64, 64, 0) ",\n" FRAME(64, 64, 0) "\n] }\n");
    write_file(BAD, "{ \"frames\": [\n{ \"mouse_x\": 64 }\n] }\n");
}

/* Chaque ligne de `sub` est dans `super`, et `super` en a davantage. */
static void test_idle_file_covers_strictly_less_than_clicking_file(void) {
    assert(run(COVER " " IDLE, DIR "/idle.lines") == 0);
    assert(run(COVER " " CLICK, DIR "/click.lines") == 0);
    long idle = line_count(DIR "/idle.lines");
    long click = line_count(DIR "/click.lines");
    assert(idle > 0);
    assert(idle < click);
    assert(run(SCRIPT " diff " DIR "/idle.lines " DIR "/click.lines", DIR "/gap.lines") == 0);
    assert(line_count(DIR "/gap.lines") == 0);
    const char *idle_text = read_file(DIR "/idle.lines");
    assert(strstr(idle_text, "src/main.c:") != NULL);
}

/* Deux fichiers : la couverture est l'union. */
static void test_cover_of_a_set_is_the_union(void) {
    assert(run(COVER " " IDLE " " CLICK, DIR "/both.lines") == 0);
    assert(run(SCRIPT " diff " DIR "/both.lines " DIR "/click.lines", DIR "/gap.lines") == 0);
    assert(line_count(DIR "/gap.lines") == 0);
    assert(run(SCRIPT " diff " DIR "/click.lines " DIR "/both.lines", DIR "/gap.lines") == 0);
    assert(line_count(DIR "/gap.lines") == 0);
}

static void test_missing_or_malformed_file_fails(void) {
    assert(run(COVER " " DIR "/absent_events.json", DIR "/x.lines") != 0);
    assert(strstr(read_file(LOG), DIR "/absent_events.json") != NULL);
    assert(run(COVER " " IDLE " " BAD, DIR "/x.lines") != 0);
    assert(strstr(read_file(LOG), BAD) != NULL);
    assert(run(COVER, DIR "/x.lines") != 0);
}

static void test_diff_returns_lines_of_first_set_missing_from_second(void) {
    write_file(DIR "/a.lines", "src/a.c:1\nsrc/a.c:2\nsrc/b.c:10\n");
    write_file(DIR "/b.lines", "src/a.c:2\nsrc/c.c:5\n");
    assert(run(SCRIPT " diff " DIR "/a.lines " DIR "/b.lines", DIR "/out.lines") == 0);
    assert(strcmp(read_file(DIR "/out.lines"), "src/a.c:1\nsrc/b.c:10\n") == 0);
}

/* L'apport d'un fichier : ses lignes qu'aucun des autres ne couvre. */
static void test_contrib_returns_lines_no_other_set_covers(void) {
    write_file(DIR "/a.lines", "src/a.c:1\nsrc/a.c:2\nsrc/b.c:10\n");
    write_file(DIR "/b.lines", "src/a.c:2\n");
    write_file(DIR "/c.lines", "src/b.c:10\nsrc/c.c:5\n");
    assert(run(SCRIPT " contrib " DIR "/a.lines " DIR "/b.lines " DIR "/c.lines",
               DIR "/out.lines") == 0);
    assert(strcmp(read_file(DIR "/out.lines"), "src/a.c:1\n") == 0);
    assert(run(SCRIPT " contrib " DIR "/a.lines", DIR "/out.lines") == 0);
    assert(strcmp(read_file(DIR "/out.lines"), "src/a.c:1\nsrc/a.c:2\nsrc/b.c:10\n") == 0);
}

static void test_set_operation_on_missing_set_fails(void) {
    assert(run(SCRIPT " diff " DIR "/a.lines " DIR "/absent.lines", DIR "/out.lines") != 0);
    assert(run(SCRIPT " contrib " DIR "/absent.lines " DIR "/a.lines", DIR "/out.lines") != 0);
}

/* L'ecart : la reference moins les lignes couvertes et moins les inatteignables,
 * dont seul le premier champ `fichier:ligne` compte. */
static void test_gap_removes_covered_and_unreachable_lines(void) {
    write_file(DIR "/ref.lines", "src/a.c:1\nsrc/a.c:2\nsrc/b.c:10\nsrc/c.c:5\n");
    write_file(DIR "/covered.lines", "src/a.c:1\nsrc/z.c:9\n");
    write_file(DIR "/unreachable.txt", "src/c.c:5 jamais en 600 frames\n");
    assert(run(SCRIPT " gap " DIR "/ref.lines " DIR "/unreachable.txt " DIR "/covered.lines",
               DIR "/out.lines") == 0);
    assert(strcmp(read_file(DIR "/out.lines"), "src/a.c:2\nsrc/b.c:10\n") == 0);
}

/* Aucun e2e : l'ensemble couvert est vide, l'ecart est toute la reference. */
static void test_gap_of_empty_covered_set_is_whole_reference(void) {
    write_file(DIR "/ref.lines", "src/a.c:1\nsrc/b.c:10\n");
    write_file(DIR "/covered.lines", "");
    write_file(DIR "/unreachable.txt", "");
    assert(run(SCRIPT " gap " DIR "/ref.lines " DIR "/unreachable.txt " DIR "/covered.lines",
               DIR "/out.lines") == 0);
    assert(strcmp(read_file(DIR "/out.lines"), "src/a.c:1\nsrc/b.c:10\n") == 0);
}

static void test_gap_with_missing_set_fails(void) {
    assert(run(SCRIPT " gap " DIR "/ref.lines " DIR "/absent.txt " DIR "/covered.lines",
               DIR "/out.lines") != 0);
    assert(strstr(read_file(LOG), DIR "/absent.txt") != NULL);
}

int main(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_fixtures();
    test_idle_file_covers_strictly_less_than_clicking_file();
    test_cover_of_a_set_is_the_union();
    test_missing_or_malformed_file_fails();
    test_diff_returns_lines_of_first_set_missing_from_second();
    test_contrib_returns_lines_no_other_set_covers();
    test_set_operation_on_missing_set_fails();
    test_gap_removes_covered_and_unreachable_lines();
    test_gap_of_empty_covered_set_is_whole_reference();
    test_gap_with_missing_set_fails();
    assert(system("rm -rf " DIR) == 0);
    printf("test_line_coverage: all tests passed\n");
    return 0;
}
