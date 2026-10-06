/* tools/e2e_limits.sh, the guard of new e2e files (companion files, 180
 * frames, 600 for a listed exception, the pad header, no mouse field), on
 * built events files. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/e2e_limits.sh"
#define DIR "tmp/test_e2e_limits_work"
#define LOG DIR "/log.txt"
#define EXCEPTIONS DIR "/exceptions.txt"

#define OLD_MOUSE_FRAME "{ \"mouse_x\": 64, \"mouse_y\": 64, \"mouse_buttons\": 0, " \
                        "\"dpad\": [false, false, false, false, false, false] }"

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

/* A complete e2e: <name>.json opening with `header`, then `frames` copies
 * of `frame` except frame 1, which is `odd` when not NULL; <name>.txt and
 * <name>.json.sha256. */
static void write_e2e_with(const char *name, const char *header, int frames, const char *frame, const char *odd) {
    char path[256];
    snprintf(path, sizeof path, DIR "/%s.json", name);
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(header, file);
    for (int i = 0; i < frames; i++) {
        fputs("    ", file);
        fputs(i == 1 && odd != NULL ? odd : frame, file);
        fputs(i + 1 < frames ? ",\n" : "\n", file);
    }
    fputs("  ]\n}\n", file);
    fclose(file);
    snprintf(path, sizeof path, DIR "/%s.txt", name);
    write_file(path, "what the replay does\n");
    snprintf(path, sizeof path, DIR "/%s.json.sha256", name);
    write_file(path, "0000\n");
}

#define PAD_HEADER "{\n  \"input\": \"pad\",\n  \"frames\": [\n"

static void write_e2e(const char *name, int frames, const char *odd) {
    write_e2e_with(name, PAD_HEADER, frames, "{ \"pad\": [] }", odd);
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
    write_e2e("short", 180, NULL);
    assert(run(DIR "/short.json") == 0);
}

static void test_e2e_over_180_frames_fails_and_is_named(void) {
    reset();
    write_e2e("short", 10, NULL);
    write_e2e("long", 181, NULL);
    assert(run(DIR "/short.json " DIR "/long.json") != 0);
    assert(strstr(read_log(), DIR "/long.json") != NULL);
    assert(strstr(read_log(), "181") != NULL);
    assert(strstr(read_log(), DIR "/short.json:") == NULL);
}

static void test_frames_are_counted_by_their_pad_key(void) {
    reset();
    write_e2e_with("pressed", PAD_HEADER, 181, "{ \"pad\": [\"cross\", \"up\"] }", NULL);
    assert(run(DIR "/pressed.json") != 0);
    assert(strstr(read_log(), "181 frames") != NULL);
}

static void test_listed_exception_is_allowed_up_to_600_frames(void) {
    reset();
    write_e2e("long", 600, NULL);
    write_file(EXCEPTIONS, DIR "/long.json the game takes long to end\n");
    assert(run(DIR "/long.json") == 0);
    write_e2e("long", 601, NULL);
    assert(run(DIR "/long.json") != 0);
    assert(strstr(read_log(), DIR "/long.json") != NULL);
}

static void test_missing_companion_file_fails(void) {
    reset();
    write_e2e("no_txt", 10, NULL);
    assert(system("rm " DIR "/no_txt.txt") == 0);
    assert(run(DIR "/no_txt.json") != 0);
    assert(strstr(read_log(), DIR "/no_txt.txt") != NULL);
    write_e2e("no_sha", 10, NULL);
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

static void test_e2e_with_a_mouse_field_fails_and_is_named(void) {
    reset();
    write_e2e("mouse_x", 10, "{ \"pad\": [], \"mouse_x\": 64 }");
    assert(run(DIR "/mouse_x.json") != 0);
    assert(strstr(read_log(), DIR "/mouse_x.json") != NULL);
    write_e2e("dpad", 10, "{ \"pad\": [], \"dpad\": [false, false, false, false] }");
    assert(run(DIR "/dpad.json") != 0);
    assert(strstr(read_log(), DIR "/dpad.json") != NULL);
}

static void test_e2e_without_the_pad_header_fails_and_is_named(void) {
    reset();
    write_e2e_with("no_header", "{\n  \"frames\": [\n", 10, "{ \"pad\": [] }", NULL);
    assert(run(DIR "/no_header.json") != 0);
    assert(strstr(read_log(), DIR "/no_header.json") != NULL);
    write_e2e_with("old_mouse", "{\n  \"frames\": [\n", 10, OLD_MOUSE_FRAME, NULL);
    assert(run(DIR "/old_mouse.json") != 0);
    assert(strstr(read_log(), DIR "/old_mouse.json") != NULL);
    write_e2e_with("mouse_header", "{\n  \"input\": \"mouse\",\n  \"frames\": [\n", 10, "{ \"pad\": [] }", NULL);
    assert(run(DIR "/mouse_header.json") != 0);
    assert(strstr(read_log(), DIR "/mouse_header.json") != NULL);
}

int main(void) {
    test_no_e2e_passes();
    test_e2e_of_180_frames_passes();
    test_e2e_over_180_frames_fails_and_is_named();
    test_frames_are_counted_by_their_pad_key();
    test_listed_exception_is_allowed_up_to_600_frames();
    test_missing_companion_file_fails();
    test_missing_exceptions_list_fails();
    test_e2e_with_a_mouse_field_fails_and_is_named();
    test_e2e_without_the_pad_header_fails_and_is_named();
    assert(system("rm -rf " DIR) == 0);
    printf("test_e2e_limits: all tests passed\n");
    return 0;
}
