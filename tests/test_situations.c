/* tmp/situations run on short e2e replays and on a resume-start recording built from the
 * committed demo record, its table read back. The expected frames come from each e2e's .txt. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TOOL "./tmp/situations"
#define OUT "tmp/test-situations"
#define RECORD "pico8/demo_record.hex"
#define RESUME_EVENTS OUT "/resume.json"
#define STDERR_LOG OUT "/stderr.txt"

enum { FRAMES_MAX = 600, NAME_CAP = 32, RESUME_FRAMES = 20 };

typedef struct {
    int count;
    char situation[FRAMES_MAX][NAME_CAP];
    char family[FRAMES_MAX][NAME_CAP];
} table_t;

/* Allocated buffer, NUL-terminated, freed by the caller. */
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

static int run(const char *command) {
    int status = system(command);
    assert(status != -1 && WIFEXITED(status));
    return WEXITSTATUS(status);
}

static int frame_count_of(const char *events) {
    char *text = read_whole_file(events);
    int count = 0;
    for (const char *at = strstr(text, "\"pad\""); at != NULL; at = strstr(at + 1, "\"pad\"")) count++;
    free(text);
    return count - 1;
}

static void label(const char *events, table_t *table) {
    char command[512];
    snprintf(command, sizeof command, "mkdir -p " OUT " && " TOOL " %s " OUT "/table.txt > /dev/null", events);
    assert(run(command) == 0);
    FILE *in = fopen(OUT "/table.txt", "r");
    assert(in != NULL);
    char line[128];
    assert(fgets(line, sizeof line, in) != NULL);
    assert(strcmp(line, "frame situation family\n") == 0);
    table->count = 0;
    while (fgets(line, sizeof line, in) != NULL) {
        assert(table->count < FRAMES_MAX);
        int frame;
        assert(sscanf(line, "%d %31s %31s", &frame, table->situation[table->count], table->family[table->count]) == 3);
        assert(frame == table->count);
        table->count++;
    }
    fclose(in);
}

static void expect(const table_t *table, int frame, const char *situation, const char *family) {
    assert(frame < table->count);
    if (strcmp(table->situation[frame], situation) != 0 || strcmp(table->family[frame], family) != 0) {
        fprintf(stderr, "test_situations: frame %d is %s %s, want %s %s\n", frame, table->situation[frame],
                table->family[frame], situation, family);
        assert(0);
    }
}

static table_t table;

static void test_the_score_e2e_opens_its_panels_after_the_galaxy(void) {
    label("tests/e2e/pad_score.json", &table);
    assert(table.count == frame_count_of("tests/e2e/pad_score.json"));
    expect(&table, 0, "title", "steady");
    expect(&table, 11, "title", "steady");
    expect(&table, 12, "title", "steady");
    expect(&table, 13, "title", "steady");
    expect(&table, 14, "galaxy-generation", "one-off");
    expect(&table, 20, "map-rest", "steady");
    expect(&table, 22, "screen-change", "one-off");
    expect(&table, 26, "screen-change", "one-off");
    expect(&table, 27, "screen-change", "one-off");
    expect(&table, 32, "score-panel", "steady");
    expect(&table, 39, "screen-change", "one-off");
    expect(&table, 45, "map-rest", "steady");
    expect(&table, 60, "build", "steady");
}

static void test_the_probe_e2e_moves_the_camera_then_launches_a_probe(void) {
    label("tests/e2e/pad_probe.json", &table);
    expect(&table, 24, "map-move", "steady");
    expect(&table, 27, "map-rest", "steady");
    expect(&table, 28, "probe", "one-off");
}

static void test_the_game_end_e2e_ends_the_game_then_returns_to_the_title(void) {
    label("tests/e2e/pad_game_end.json", &table);
    expect(&table, 45, "new-year", "one-off");
    expect(&table, 46, "map-rest", "steady");
    expect(&table, 93, "game-end", "one-off");
    expect(&table, 102, "menu", "steady");
    expect(&table, 108, "return-to-title", "one-off");
}

static void write_resume_events(void) {
    char *hex = read_whole_file(RECORD);
    hex[strcspn(hex, "\n")] = '\0';
    FILE *out = fopen(RESUME_EVENTS, "w");
    assert(out != NULL);
    fprintf(out, "{\n  \"input\": \"pad\",\n  \"start\": \"resume\",\n  \"initial_save\": \"%s\",\n  \"frames\": [\n", hex);
    for (int i = 0; i < RESUME_FRAMES; i++) fprintf(out, "    { \"pad\": [] }%s\n", i < RESUME_FRAMES - 1 ? "," : "");
    fputs("  ]\n}\n", out);
    fclose(out);
    free(hex);
}

static void test_a_resumed_game_starts_on_the_map_with_no_galaxy_generation(void) {
    assert(run("mkdir -p " OUT) == 0);
    write_resume_events();
    label(RESUME_EVENTS, &table);
    assert(table.count == RESUME_FRAMES);
    for (int frame = 0; frame < table.count; frame++) {
        assert(strcmp(table.situation[frame], "galaxy-generation") != 0);
        assert(strcmp(table.situation[frame], "screen-change") != 0);
    }
    expect(&table, RESUME_FRAMES - 1, "map-rest", "steady");
}

static void test_a_missing_events_file_is_refused(void) {
    assert(run("mkdir -p " OUT " && " TOOL " " OUT "/missing.json " OUT "/table.txt 2> " STDERR_LOG) == 1);
    char *err = read_whole_file(STDERR_LOG);
    assert(strstr(err, "situations: ") != NULL);
    free(err);
    assert(run(TOOL " 2> " STDERR_LOG) == 1);
}

int main(void) {
    test_the_score_e2e_opens_its_panels_after_the_galaxy();
    test_the_probe_e2e_moves_the_camera_then_launches_a_probe();
    test_the_game_end_e2e_ends_the_game_then_returns_to_the_title();
    test_a_resumed_game_starts_on_the_map_with_no_galaxy_generation();
    test_a_missing_events_file_is_refused();
    return 0;
}
