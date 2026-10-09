/* tools/psx_digest_compare.sh, the sprite-sheet/screen/map conditions of
 * `make psx-check`, on hand-written TTY and digest files: no emulator. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/psx_digest_compare.sh"
#define DIR "tmp/test_psx_digest_compare_work"
#define TTY DIR "/tty.txt"
#define PC DIR "/pc_digest.txt"
#define OUT DIR "/out"
#define LOG DIR "/log.txt"

#define PC_DIGEST \
    "sprite-sheet 0a1b2c3d\n" \
    "screen 0 11111111\n" \
    "screen 32 22222222\n" \
    "screen 64 33333333\n" \
    "screen 70 44444444\n" \
    "map 0000beef\n"

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Static buffer, overwritten by each call; empty when the file is missing. */
static const char *read_file(const char *path) {
    static char buf[1 << 16];
    buf[0] = '\0';
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return buf;
    }
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

#define PSX_DIGEST \
    "psx-digest: sprite-sheet 0a1b2c3d\n" \
    "psx-digest: screen 0 11111111\n" \
    "psx-digest: screen 32 22222222\n" \
    "psx-digest: screen 64 33333333\n" \
    "psx-digest: screen 70 44444444\n" \
    "psx-digest: map 0000beef\n"

static int run_against(const char *tty, const char *pc_digest) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(PC, pc_digest);
    write_file(TTY, tty);
    return system(SCRIPT " " TTY " " PC " " OUT " > " LOG " 2>&1");
}

static int run(const char *tty) {
    return run_against(tty, PC_DIGEST);
}

static void test_identical_lines_in_interleaved_tty_pass(void) {
    const char *tty =
        "PCSX-Redux booting\r\n"
        "psx-replay: asserts on\r\n"
        "psx-digest: sprite-sheet 0a1b2c3d\r\n"
        "psx-digest: screen 0 11111111\r\n"
        "[GPU] vsync 120\n"
        "psx-digest: screen 32 22222222\n"
        "psx-digest: screen 64 33333333\n"
        "SPU: dma idle\n"
        "psx-digest: screen 70 44444444\n"
        "psx-digest: map 0000beef\r\n"
        "psx-state: clock year 3400 month 2\n"
        "psx-replay: done pad_slipway 71 vsyncs 900\n";
    assert(run(tty) == 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "sprite-sheet: ok") != NULL);
    assert(strstr(log, "screen: ok") != NULL);
    assert(strstr(log, "map: ok") != NULL);
    assert(strcmp(read_file(OUT "/psx_digest.txt"), PC_DIGEST) == 0);
}

static void test_differing_sprite_sheet_fails_that_result_only(void) {
    const char *tty =
        "psx-digest: sprite-sheet 0a1b2c3e\n"
        "psx-digest: screen 0 11111111\n"
        "psx-digest: screen 32 22222222\n"
        "psx-digest: screen 64 33333333\n"
        "psx-digest: screen 70 44444444\n"
        "psx-digest: map 0000beef\n";
    assert(run(tty) != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "sprite-sheet: first differing sprite-sheet line, 1:") != NULL);
    assert(strstr(log, "psx: sprite-sheet 0a1b2c3e") != NULL);
    assert(strstr(log, "pc:  sprite-sheet 0a1b2c3d") != NULL);
    assert(strstr(log, "screen: ok") != NULL);
    assert(strstr(log, "map: ok") != NULL);
}

static void test_differing_screen_names_its_frame_from_both_sides(void) {
    const char *tty =
        "psx-digest: sprite-sheet 0a1b2c3d\n"
        "psx-digest: screen 0 11111111\n"
        "psx-digest: screen 32 2222222f\n"
        "psx-digest: screen 64 3333333f\n"
        "psx-digest: screen 70 44444444\n"
        "psx-digest: map 0000beef\n";
    assert(run(tty) != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "screen: first differing screen line, 2:") != NULL);
    assert(strstr(log, "psx: screen 32 2222222f") != NULL);
    assert(strstr(log, "pc:  screen 32 22222222") != NULL);
    assert(strstr(log, "screen 64") == NULL);
    assert(strstr(log, "sprite-sheet: ok") != NULL);
    assert(strstr(log, "map: ok") != NULL);
}

static void test_differing_map_fails_that_result_only(void) {
    const char *tty =
        "psx-digest: sprite-sheet 0a1b2c3d\n"
        "psx-digest: screen 0 11111111\n"
        "psx-digest: screen 32 22222222\n"
        "psx-digest: screen 64 33333333\n"
        "psx-digest: screen 70 44444444\n"
        "psx-digest: map 0000beee\n";
    assert(run(tty) != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "map: first differing map line, 1:") != NULL);
    assert(strstr(log, "psx: map 0000beee") != NULL);
    assert(strstr(log, "pc:  map 0000beef") != NULL);
    assert(strstr(log, "sprite-sheet: ok") != NULL);
    assert(strstr(log, "screen: ok") != NULL);
}

/* A truncated TTY is a difference, never a shorter match. */
static void test_missing_last_screen_fails(void) {
    const char *tty =
        "psx-digest: sprite-sheet 0a1b2c3d\n"
        "psx-digest: screen 0 11111111\n"
        "psx-digest: screen 32 22222222\n"
        "psx-digest: screen 64 33333333\n"
        "psx-digest: map 0000beef\n";
    assert(run(tty) != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "screen: first differing screen line, 4:") != NULL);
    assert(strstr(log, "psx: (none)") != NULL);
    assert(strstr(log, "pc:  screen 70 44444444") != NULL);
}

static void test_extra_screen_fails(void) {
    assert(run(PSX_DIGEST "psx-digest: screen 96 55555555\n") != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "psx: screen 96 55555555") != NULL);
    assert(strstr(log, "pc:  (none)") != NULL);
}

static void test_empty_tty_fails_all_three(void) {
    assert(run("") != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "sprite-sheet: first differing") != NULL);
    assert(strstr(log, "screen: first differing") != NULL);
    assert(strstr(log, "map: first differing") != NULL);
    assert(strstr(log, ": ok") == NULL);
}

/* Both sides empty would otherwise compare equal: a PC run without DIGEST=. */
static void test_empty_pc_digest_fails(void) {
    assert(run_against("", "") != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "sprite-sheet: no sprite-sheet line in the pc digest") != NULL);
    assert(strstr(log, "screen: no screen line in the pc digest") != NULL);
    assert(strstr(log, "map: no map line in the pc digest") != NULL);
}

int main(void) {
    test_identical_lines_in_interleaved_tty_pass();
    test_differing_sprite_sheet_fails_that_result_only();
    test_differing_screen_names_its_frame_from_both_sides();
    test_differing_map_fails_that_result_only();
    test_missing_last_screen_fails();
    test_extra_screen_fails();
    test_empty_tty_fails_all_three();
    test_empty_pc_digest_fails();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_digest_compare: all tests passed\n");
    return 0;
}
