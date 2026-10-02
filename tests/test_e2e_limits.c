/* tools/e2e_limits.sh, la garde des nouveaux e2e (fichiers compagnons, 180
 * frames, 600 en exception listee), sur des fichiers d'evenements fabriques. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/e2e_limits.sh"
#define DIR "tmp/test_e2e_limits_work"
#define LOG DIR "/log.txt"
#define EXCEPTIONS DIR "/exceptions.txt"

#define FRAME "{ \"mouse_x\": 64, \"mouse_y\": 64, \"mouse_buttons\": 0, " \
              "\"dpad\": [false, false, false, false, false, false] }"

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Tampon statique, ecrase a chaque appel. */
static const char *read_log(void) {
    FILE *file = fopen(LOG, "rb");
    assert(file != NULL);
    static char buf[1 << 16];
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

/* Un e2e complet : <nom>.json de `frames` frames, <nom>.txt et <nom>.json.sha256. */
static void write_e2e(const char *name, int frames) {
    char path[256];
    snprintf(path, sizeof path, DIR "/%s.json", name);
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs("{\n  \"frames\": [\n", file);
    for (int i = 0; i < frames; i++) {
        fprintf(file, "    %s%s\n", FRAME, i + 1 < frames ? "," : "");
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

int main(void) {
    test_no_e2e_passes();
    test_e2e_of_180_frames_passes();
    test_e2e_over_180_frames_fails_and_is_named();
    test_listed_exception_is_allowed_up_to_600_frames();
    test_missing_companion_file_fails();
    test_missing_exceptions_list_fails();
    assert(system("rm -rf " DIR) == 0);
    printf("test_e2e_limits: all tests passed\n");
    return 0;
}
