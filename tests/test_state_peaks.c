/* tools/state_peaks.sh, the peak table of the make state campaign, on
 * hand-written state files: no replay. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/state_peaks.sh"
#define DIR "tmp/test_state_peaks_work"
#define OUT DIR "/state.txt"
#define LOG DIR "/log.txt"

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

/* `pairs` is a list of `<events> <state>` arguments. */
static int run(const char *pairs) {
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " " OUT " %s > " LOG " 2>&1", pairs);
    return system(cmd);
}

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
}

static void test_highest_peak_is_kept_with_its_replay_and_limit(void) {
    reset();
    write_file(DIR "/a.txt",
               "clock year 3426 month 1\n"
               "zone game 10 500 1000\n"
               "cap message_box 0 0 6\n"
               "cap bytes_button - 3000 6240\n");
    write_file(DIR "/b.txt",
               "zone game 5 300 1000\n"
               "cap message_box 1 2 6\n"
               "cap bytes_button - 3768 6240\n");
    assert(run("a.json " DIR "/a.txt b.json " DIR "/b.txt") == 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "replay a.json clean\n") != NULL);
    assert(strstr(out, "replay b.json clean\n") != NULL);
    assert(strstr(out, "zone game 500 1000 a.json\n") != NULL);
    assert(strstr(out, "cap message_box 2 6 b.json\n") != NULL);
    assert(strstr(out, "cap bytes_button 3768 6240 b.json\n") != NULL);
    assert(strstr(out, "clock") == NULL);
    assert(strstr(out, "replay b.json") < strstr(out, "zone game"));
    assert(strstr(out, "zone game") < strstr(out, "cap message_box"));
}

static void test_tie_keeps_the_first_replay(void) {
    reset();
    write_file(DIR "/a.txt", "cap toast 0 13 128\n");
    write_file(DIR "/b.txt", "cap toast 0 13 128\n");
    assert(run("b.json " DIR "/b.txt a.json " DIR "/a.txt") == 0);
    assert(strstr(read_file(OUT), "cap toast 13 128 b.json\n") != NULL);
}

static void test_stopped_replay_fails_is_named_and_table_still_written(void) {
    reset();
    write_file(DIR "/a.txt", "zone game 1 100 1000\n");
    write_file(DIR "/b.txt",
               "stopped p8_fatal frame 42\n"
               "zone game 1000 1000 1000\n");
    assert(run("a.json " DIR "/a.txt b.json " DIR "/b.txt") != 0);
    const char *out = read_file(OUT);
    assert(strstr(out, "replay a.json clean\n") != NULL);
    assert(strstr(out, "replay b.json stopped p8_fatal frame 42\n") != NULL);
    assert(strstr(out, "zone game 1000 1000 b.json\n") != NULL);
    assert(strstr(read_file(LOG), "state: b.json stopped") != NULL);
    assert(strstr(read_file(LOG), "state: a.json") == NULL);
}

static void test_missing_state_fails(void) {
    reset();
    write_file(DIR "/a.txt", "zone game 1 100 1000\n");
    assert(run("a.json " DIR "/a.txt b.json " DIR "/absent.txt") != 0);
    assert(strstr(read_file(OUT), "replay b.json no state\n") != NULL);
    assert(strstr(read_file(LOG), "state: b.json stopped") != NULL);
}

int main(void) {
    test_highest_peak_is_kept_with_its_replay_and_limit();
    test_tie_keeps_the_first_replay();
    test_stopped_replay_fails_is_named_and_table_still_written();
    test_missing_state_fails();
    assert(system("rm -rf " DIR) == 0);
    printf("test_state_peaks: all tests passed\n");
    return 0;
}
