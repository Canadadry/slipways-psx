/* tools/e2e_limits.sh, the guard of new e2e files (companion files, 180
 * frames, 600 for a listed exception, keyboard_ files without the mouse, pad files
 * without any mouse-mode field), on built events files. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/e2e_limits.sh"
#define DIR "tmp/test_e2e_limits_work"
#define LOG DIR "/log.txt"
#define EXCEPTIONS DIR "/exceptions.txt"

#define FRAME "{ \"mouse_x\": %d, \"mouse_y\": %d, \"mouse_buttons\": %d, " \
              "\"dpad\": [false, false, false, false, false, false] }"

typedef struct {
    int x, y, buttons;
} mouse_t;

static const mouse_t still = {64, 64, 0};

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Static buffer, overwritten by each call. */
static const char *read_log(void) {
    FILE *file = fopen(LOG, "rb");
    assert(file != NULL);
    static char buf[1 << 16];
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

/* A complete e2e: <name>.json of `frames` frames, <name>.txt and
 * <name>.json.sha256. Frame 1 has the mouse `odd`, every other one `still`;
 * `declared` writes the keyboard_cursor member before "frames". */
static void write_e2e_mouse(const char *name, int frames, int declared, mouse_t odd) {
    char path[256];
    snprintf(path, sizeof path, DIR "/%s.json", name);
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(declared ? "{\n  \"keyboard_cursor\": true,\n  \"frames\": [\n"
                   : "{\n  \"frames\": [\n", file);
    for (int i = 0; i < frames; i++) {
        mouse_t mouse = i == 1 ? odd : still;
        fputs("    ", file);
        fprintf(file, FRAME, mouse.x, mouse.y, mouse.buttons);
        fputs(i + 1 < frames ? ",\n" : "\n", file);
    }
    fputs("  ]\n}\n", file);
    fclose(file);
    snprintf(path, sizeof path, DIR "/%s.txt", name);
    write_file(path, "what the replay does\n");
    snprintf(path, sizeof path, DIR "/%s.json.sha256", name);
    write_file(path, "0000\n");
}

static void write_e2e(const char *name, int frames) {
    write_e2e_mouse(name, frames, 0, still);
}

/* A complete pad e2e of `frames` idle frames; frame 1 is `odd` when not NULL. */
static void write_e2e_pad(const char *name, int frames, const char *odd) {
    char path[256];
    snprintf(path, sizeof path, DIR "/%s.json", name);
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs("{\n  \"input\": \"pad\",\n  \"frames\": [\n", file);
    for (int i = 0; i < frames; i++) {
        fputs("    ", file);
        fputs(i == 1 && odd != NULL ? odd : "{ \"pad\": [] }", file);
        fputs(i + 1 < frames ? ",\n" : "\n", file);
    }
    fputs("  ]\n}\n", file);
    fclose(file);
    snprintf(path, sizeof path, DIR "/%s.txt", name);
    write_file(path, "what the replay does\n");
    snprintf(path, sizeof path, DIR "/%s.json.sha256", name);
    write_file(path, "0000\n");
}

static int run(const char *events) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " " EXCEPTIONS " %s > " LOG " 2>&1", events);
    return system(cmd);
}

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(EXCEPTIONS, "");
}

static void test_no_e2e_passes(void) {
    reset();
    assert(run("") == 0);
}

static void test_e2e_of_180_frames_passes(void) {
    reset();
    write_e2e("short", 180);
    assert(run(DIR "/short.json") == 0);
}

static void test_e2e_over_180_frames_fails_and_is_named(void) {
    reset();
    write_e2e("short", 10);
    write_e2e("long", 181);
    assert(run(DIR "/short.json " DIR "/long.json") != 0);
    assert(strstr(read_log(), DIR "/long.json") != NULL);
    assert(strstr(read_log(), "181") != NULL);
    assert(strstr(read_log(), DIR "/short.json:") == NULL);
}

static void test_listed_exception_is_allowed_up_to_600_frames(void) {
    reset();
    write_e2e("long", 600);
    write_file(EXCEPTIONS, DIR "/long.json the probe takes 9 s to come back\n");
    assert(run(DIR "/long.json") == 0);
    write_e2e("long", 601);
    assert(run(DIR "/long.json") != 0);
    assert(strstr(read_log(), DIR "/long.json") != NULL);
}

static void test_missing_companion_file_fails(void) {
    reset();
    write_e2e("no_txt", 10);
    assert(system("rm " DIR "/no_txt.txt") == 0);
    assert(run(DIR "/no_txt.json") != 0);
    assert(strstr(read_log(), DIR "/no_txt.txt") != NULL);
    write_e2e("no_sha", 10);
    assert(system("rm " DIR "/no_sha.json.sha256") == 0);
    assert(run(DIR "/no_sha.json") != 0);
    assert(strstr(read_log(), DIR "/no_sha.json.sha256") != NULL);
    assert(run(DIR "/absent.json") != 0);
    assert(strstr(read_log(), DIR "/absent.json") != NULL);
}

static void test_missing_exceptions_list_fails(void) {
    reset();
    assert(system("rm " EXCEPTIONS) == 0);
    assert(run("") != 0);
}

static void test_keyboard_e2e_declared_with_still_mouse_passes(void) {
    reset();
    write_e2e_mouse("keyboard_ok", 10, 1, still);
    assert(run(DIR "/keyboard_ok.json") == 0);
}

static void test_declared_e2e_not_named_keyboard_may_move_the_mouse(void) {
    reset();
    write_e2e_mouse("takeover", 10, 1, (mouse_t){30, 20, 1});
    assert(run(DIR "/takeover.json") == 0);
}

static void test_keyboard_e2e_with_moved_mouse_fails_and_is_named(void) {
    reset();
    write_e2e_mouse("keyboard_x", 10, 1, (mouse_t){65, 64, 0});
    assert(run(DIR "/keyboard_x.json") != 0);
    assert(strstr(read_log(), DIR "/keyboard_x.json") != NULL);
    write_e2e_mouse("keyboard_y", 10, 1, (mouse_t){64, 63, 0});
    assert(run(DIR "/keyboard_y.json") != 0);
    assert(strstr(read_log(), DIR "/keyboard_y.json") != NULL);
}

static void test_keyboard_e2e_with_mouse_button_fails_and_is_named(void) {
    reset();
    write_e2e_mouse("keyboard_click", 10, 1, (mouse_t){64, 64, 1});
    assert(run(DIR "/keyboard_click.json") != 0);
    assert(strstr(read_log(), DIR "/keyboard_click.json") != NULL);
}

static void test_keyboard_e2e_without_declaration_fails_and_is_named(void) {
    reset();
    write_e2e_mouse("keyboard_undeclared", 10, 0, still);
    assert(run(DIR "/keyboard_undeclared.json") != 0);
    assert(strstr(read_log(), DIR "/keyboard_undeclared.json") != NULL);
}

static void test_pad_e2e_of_180_frames_passes(void) {
    reset();
    write_e2e_pad("pad_short", 180, NULL);
    assert(run(DIR "/pad_short.json") == 0);
}

static void test_pad_e2e_over_180_frames_fails_and_is_named(void) {
    reset();
    write_e2e_pad("pad_long", 181, NULL);
    assert(run(DIR "/pad_long.json") != 0);
    assert(strstr(read_log(), DIR "/pad_long.json") != NULL);
    assert(strstr(read_log(), "181") != NULL);
}

static void test_listed_pad_exception_is_allowed_up_to_600_frames(void) {
    reset();
    write_e2e_pad("pad_long", 600, NULL);
    write_file(EXCEPTIONS, DIR "/pad_long.json the game takes long to end\n");
    assert(run(DIR "/pad_long.json") == 0);
    write_e2e_pad("pad_long", 601, NULL);
    assert(run(DIR "/pad_long.json") != 0);
    assert(strstr(read_log(), DIR "/pad_long.json") != NULL);
}

static void test_pad_e2e_with_a_mouse_mode_field_fails_and_is_named(void) {
    reset();
    write_e2e_pad("pad_mouse_x", 10, "{ \"pad\": [], \"mouse_x\": 64 }");
    assert(run(DIR "/pad_mouse_x.json") != 0);
    assert(strstr(read_log(), DIR "/pad_mouse_x.json") != NULL);
    write_e2e_pad("pad_dpad", 10, "{ \"pad\": [], \"dpad\": [false, false, false, false] }");
    assert(run(DIR "/pad_dpad.json") != 0);
    assert(strstr(read_log(), DIR "/pad_dpad.json") != NULL);
    write_e2e_pad("pad_keyboard", 10, "{ \"pad\": [], \"keyboard_cursor\": true }");
    assert(run(DIR "/pad_keyboard.json") != 0);
    assert(strstr(read_log(), DIR "/pad_keyboard.json") != NULL);
}

static void test_pad_e2e_missing_companion_file_fails(void) {
    reset();
    write_e2e_pad("pad_no_txt", 10, NULL);
    assert(system("rm " DIR "/pad_no_txt.txt") == 0);
    assert(run(DIR "/pad_no_txt.json") != 0);
    assert(strstr(read_log(), DIR "/pad_no_txt.txt") != NULL);
}

int main(void) {
    test_no_e2e_passes();
    test_e2e_of_180_frames_passes();
    test_e2e_over_180_frames_fails_and_is_named();
    test_listed_exception_is_allowed_up_to_600_frames();
    test_missing_companion_file_fails();
    test_missing_exceptions_list_fails();
    test_keyboard_e2e_declared_with_still_mouse_passes();
    test_declared_e2e_not_named_keyboard_may_move_the_mouse();
    test_keyboard_e2e_with_moved_mouse_fails_and_is_named();
    test_keyboard_e2e_with_mouse_button_fails_and_is_named();
    test_keyboard_e2e_without_declaration_fails_and_is_named();
    test_pad_e2e_of_180_frames_passes();
    test_pad_e2e_over_180_frames_fails_and_is_named();
    test_listed_pad_exception_is_allowed_up_to_600_frames();
    test_pad_e2e_with_a_mouse_mode_field_fails_and_is_named();
    test_pad_e2e_missing_companion_file_fails();
    assert(system("rm -rf " DIR) == 0);
    printf("test_e2e_limits: all tests passed\n");
    return 0;
}
