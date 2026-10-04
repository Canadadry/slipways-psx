/* make replay-convert-test: tools/replay_convert.sh on events files, label
 * files and anchor tables built by hand. Temporary (plan 12). */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/replay_convert.sh"
#define DIR "tmp/test_replay_convert_work"
#define LOG DIR "/log.txt"
#define TABLE DIR "/anchors.txt"
#define EVENTS DIR "/events.json"
#define LABELS DIR "/labels.txt"
#define OUT DIR "/out.json"
#define REPORT DIR "/report.txt"
#define CONVERT SCRIPT " convert " EVENTS " " LABELS " " TABLE " " OUT " " REPORT

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Static buffer, overwritten by each call. */
static const char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    static char buf[1 << 16];
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static int exists(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }
    fclose(file);
    return 1;
}

static int run(const char *args, const char *out) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, "%s > %s 2> " LOG, args, out);
    return system(cmd);
}

static int convert(void) {
    assert(system("rm -f " OUT " " REPORT) == 0);
    return run(CONVERT, DIR "/stdout.txt");
}

#define FRAME(x, y) \
    "    {\"mouse_x\": " #x ", \"mouse_y\": " #y ", \"mouse_buttons\": 0, " \
    "\"dpad\": [false, false, false, false]}"
#define HEAD "{\n  \"frames\": [\n"
#define TAIL "\n  ]\n}\n"

/* The header and the rows of tools/cursor_anchors.txt, one label per
 * anchor. */
static void write_table(void) {
    write_file(TABLE, "# x offset: gauche 0, centre +16, droite +32\n"
                      "carte carte\n"
                      "gh gauche haut\n"
                      "ch centre haut\n"
                      "dh droite haut\n"
                      "gc gauche centre\n"
                      "cc centre centre\n"
                      "dc droite centre\n"
                      "gb gauche bas\n"
                      "cb centre bas\n"
                      "db droite bas\n");
}

static void test_each_anchor_shifts_its_frame(void) {
    write_file(EVENTS, HEAD FRAME(50, 50) ",\n" FRAME(50, 50) ",\n" FRAME(50, 50) ",\n"
                       FRAME(50, 50) ",\n" FRAME(50, 50) ",\n" FRAME(50, 50) ",\n"
                       FRAME(50, 50) ",\n" FRAME(50, 50) ",\n" FRAME(50, 50) ",\n"
                       FRAME(50, 50) TAIL);
    write_file(LABELS, "carte\ngh\nch\ndh\ngc\ncc\ndc\ngb\ncb\ndb\n");
    assert(convert() == 0);
    assert(strcmp(read_file(OUT), HEAD FRAME(66, 46) ",\n" FRAME(50, 50) ",\n"
                                  FRAME(66, 50) ",\n" FRAME(82, 50) ",\n"
                                  FRAME(50, 46) ",\n" FRAME(66, 46) ",\n"
                                  FRAME(82, 46) ",\n" FRAME(50, 42) ",\n"
                                  FRAME(66, 42) ",\n" FRAME(82, 42) TAIL) == 0);
}

/* Back into [0, 159] x [0, 119], as the raylib backend does, and listed. */
static void test_point_out_of_frame_is_clamped_and_reported(void) {
    write_file(EVENTS, HEAD FRAME(0, 0) ",\n" FRAME(150, 124) ",\n" FRAME(10, 20) TAIL);
    write_file(LABELS, "carte\ndb\ngh\n");
    assert(convert() == 0);
    assert(strcmp(read_file(OUT), HEAD FRAME(16, 0) ",\n" FRAME(159, 116) ",\n"
                                  FRAME(10, 20) TAIL) == 0);
    const char *report = read_file(REPORT);
    assert(strstr(report, "recadre 0 0 0 -> 16 0 carte\n") != NULL);
    assert(strstr(report, "recadre 1 150 124 -> 159 116 db\n") != NULL);
    assert(strstr(report, "recadre 2") == NULL);
}

/* The positions a .txt cites: each 128x128 point, its converted point and
 * its frames. */
static void test_report_maps_each_position_to_its_frames(void) {
    write_file(EVENTS, HEAD FRAME(60, 74) ",\n" FRAME(60, 74) ",\n" FRAME(88, 37) ",\n"
                       FRAME(60, 74) ",\n" FRAME(60, 74) TAIL);
    write_file(LABELS, "cc\ncc\ncarte\ncc\ngh\n");
    assert(convert() == 0);
    const char *report = read_file(REPORT);
    assert(strstr(report, "position 60,74 -> 76,70 0-1,3\n") != NULL);
    assert(strstr(report, "position 88,37 -> 104,33 2\n") != NULL);
    assert(strstr(report, "position 60,74 -> 60,74 4\n") != NULL);
}

/* Spaces inside the braces, a dpad of six, negative values: only the two
 * numbers change. */
static void test_other_layouts_change_only_the_two_numbers(void) {
    write_file(EVENTS, "{\"frames\": [\n"
                       "{ \"mouse_x\": -3, \"mouse_y\": -1, \"mouse_buttons\": 2, "
                       "\"dpad\": [false, true, false, false, true, false] },\n"
                       "{ \"mouse_x\":7,\"mouse_y\":  130, \"mouse_buttons\": 1, "
                       "\"dpad\": [true, false, false, false, false, true] }\n"
                       "]}\n");
    write_file(LABELS, "carte\ngh\n");
    assert(convert() == 0);
    assert(strcmp(read_file(OUT), "{\"frames\": [\n"
                                  "{ \"mouse_x\": 13, \"mouse_y\": 0, \"mouse_buttons\": 2, "
                                  "\"dpad\": [false, true, false, false, true, false] },\n"
                                  "{ \"mouse_x\":7,\"mouse_y\":  119, \"mouse_buttons\": 1, "
                                  "\"dpad\": [true, false, false, false, false, true] }\n"
                                  "]}\n") == 0);
    assert(strstr(read_file(REPORT), "recadre 0 -3 -1 -> 13 0 carte\n") != NULL);
}

static void assert_fails_without_output(void) {
    assert(convert() != 0);
    assert(!exists(OUT));
    assert(!exists(REPORT));
}

static void test_frame_and_label_counts_must_match(void) {
    write_file(EVENTS, HEAD FRAME(1, 1) ",\n" FRAME(2, 2) TAIL);
    write_file(LABELS, "carte\n");
    assert_fails_without_output();
    write_file(LABELS, "carte\ncarte\ncarte\n");
    assert_fails_without_output();
}

static void test_label_absent_from_table_fails(void) {
    write_file(EVENTS, HEAD FRAME(1, 1) ",\n" FRAME(2, 2) TAIL);
    write_file(LABELS, "carte\nbutton\n");
    assert_fails_without_output();
    assert(strstr(read_file(LOG), "button") != NULL);
}

static void test_unreadable_frame_line_fails(void) {
    write_file(LABELS, "carte\n");
    write_file(EVENTS, HEAD "    {\"mouse_x\": 4, \"mouse_buttons\": 0}" TAIL);
    assert_fails_without_output();
    write_file(EVENTS, HEAD "    {\"mouse_x\": a, \"mouse_y\": 4}" TAIL);
    assert_fails_without_output();
    write_file(EVENTS, HEAD "    {\"mouse_y\": 4, \"mouse_x\": 4}" TAIL);
    assert_fails_without_output();
    write_file(LABELS, "carte\ncarte\n");
    write_file(EVENTS, HEAD "    {\"mouse_x\": 4, \"mouse_y\": 4}, {\"mouse_x\": 4, \"mouse_y\": 4}" TAIL);
    assert_fails_without_output();
}

static const char *first_divergence(const char *old_labels, const char *new_labels) {
    write_file(DIR "/old.txt", old_labels);
    write_file(DIR "/new.txt", new_labels);
    assert(run(SCRIPT " first-divergence " DIR "/old.txt " DIR "/new.txt", DIR "/div.txt") == 0);
    return read_file(DIR "/div.txt");
}

/* Frames count from 0, as in the replay; a missing frame diverges. */
static void test_first_divergence(void) {
    assert(strcmp(first_divergence("carte\ngh\n", "carte\ngh\n"), "identique\n") == 0);
    assert(strcmp(first_divergence("carte\ngh\ncc\ndb\n", "carte\ngh\ncarte\ngh\n"),
                  "frame 2 cc carte\n") == 0);
    assert(strcmp(first_divergence("carte\ngh\n", "carte\n"), "frame 1 gh absente\n") == 0);
    assert(strcmp(first_divergence("carte\n", "carte\ngh\n"), "frame 1 absente gh\n") == 0);
}

/* A pair of the .txt that is a 128x128 mouse position moved by the
 * conversion is listed; one already converted, or never moved, is not. */
static void test_stale_pairs_lists_old_mouse_positions(void) {
    write_file(DIR "/old.json", HEAD FRAME(60, 74) ",\n" FRAME(0, 0) ",\n" FRAME(5, 5) TAIL);
    write_file(DIR "/new.json", HEAD FRAME(76, 70) ",\n" FRAME(0, 0) ",\n" FRAME(21, 1) TAIL);
    write_file(DIR "/e2e.txt", "- f0 : clic (60,74).\n"
                               "- f1 : (0,0) puis (76,70), (5, 5).\n"
                               "- f2 : (21,1) et (9,9).\n");
    assert(run(SCRIPT " stale-pairs " DIR "/e2e.txt " DIR "/old.json " DIR "/new.json",
               DIR "/stale.txt") == 0);
    assert(strcmp(read_file(DIR "/stale.txt"), "ligne 1 (60,74)\nligne 2 (5, 5)\n") == 0);
}

int main(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_table();
    test_each_anchor_shifts_its_frame();
    test_point_out_of_frame_is_clamped_and_reported();
    test_report_maps_each_position_to_its_frames();
    test_other_layouts_change_only_the_two_numbers();
    test_frame_and_label_counts_must_match();
    test_label_absent_from_table_fails();
    test_unreadable_frame_line_fails();
    test_first_divergence();
    test_stale_pairs_lists_old_mouse_positions();
    assert(system("rm -rf " DIR) == 0);
    printf("test_replay_convert: all tests passed\n");
    return 0;
}
