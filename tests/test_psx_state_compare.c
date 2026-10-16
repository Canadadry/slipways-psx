/* tools/psx_state_compare.sh, the game-line comparison of `make psx-check`,
 * on hand-written TTY and state files: no emulator. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/psx_state_compare.sh"
#define DIR "tmp/test_psx_state_compare_work"
#define TTY DIR "/tty.txt"
#define PC DIR "/pc_state.txt"
#define OUT DIR "/out"
#define MEASURE OUT "/measurements.txt"
#define LOG DIR "/log.txt"
#define FRAMES "401"

#define PC_STATE \
    "clock year 3400 month 2\n" \
    "difficulty reasonable\n" \
    "planet p1 earthlike - world 1.00 2.00 screen 3.00 4.00 visible 1\n" \
    "planet p2 unknown planet - world 5.00 6.00 screen 7.00 8.00 visible 0\n" \
    "zone game 112336 112336 468664\n" \
    "budget zones 795243 796632\n" \
    "cap button 6 11 15\n" \
    "count planets 2\n"

#define PSX_STATE_LINES \
    "psx-state: clock year 3400 month 2\n" \
    "psx-state: difficulty reasonable\n" \
    "psx-state: planet p1 earthlike - world 1.00 2.00 screen 3.00 4.00 visible 1\n" \
    "psx-state: planet p2 unknown planet - world 5.00 6.00 screen 7.00 8.00 visible 0\n" \
    "psx-state: zone game 81200 81200 468664\n" \
    "psx-state: budget zones 600000 796632\n" \
    "psx-state: cap button 5 9 15\n" \
    "psx-state: count planets 2\n"

#define PSX_HEADER \
    "psx-replay: asserts on\n" \
    "psx-replay: frame 256\n"

#define PSX_DONE "psx-replay: done pad_slipway " FRAMES " vsyncs 9000\n"

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

static int run(const char *tty, const char *frames) {
    write_file(TTY, tty);
    char cmd[1024];
    snprintf(cmd, sizeof cmd, SCRIPT " " TTY " " PC " " OUT " %s > " LOG " 2>&1", frames);
    return system(cmd);
}

static void reset(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(PC, PC_STATE);
}

/* The MIPS zone/budget/cap lines differ from the PC's by pointer size: they
 * are measured, not compared. */
static void test_match_with_different_zone_lines_passes_and_writes_measurements(void) {
    reset();
    assert(run(PSX_HEADER PSX_DONE PSX_STATE_LINES, FRAMES) == 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "game lines match") != NULL);
    const char *measure = read_file(MEASURE);
    assert(strstr(measure, "zone game 81200 81200 468664\n") != NULL);
    assert(strstr(measure, "budget zones 600000 796632\n") != NULL);
    assert(strstr(measure, "cap button 5 9 15\n") != NULL);
    assert(strstr(measure, "vsyncs 9000\n") != NULL);
    assert(strstr(measure, "112336") == NULL);
    assert(strstr(measure, "planet") == NULL);
}

static void test_differing_planet_line_fails_naming_both_lines(void) {
    reset();
    const char *tty = PSX_HEADER PSX_DONE
        "psx-state: clock year 3400 month 2\n"
        "psx-state: difficulty reasonable\n"
        "psx-state: planet p1 earthlike - world 1.00 2.00 screen 3.00 4.00 visible 1\n"
        "psx-state: planet p2 unknown planet - world 5.00 6.01 screen 7.00 8.00 visible 0\n"
        "psx-state: count planets 2\n";
    assert(run(tty, FRAMES) != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "psx: planet p2 unknown planet - world 5.00 6.01") != NULL);
    assert(strstr(log, "pc:  planet p2 unknown planet - world 5.00 6.00") != NULL);
    assert(strstr(log, "game lines match") == NULL);
}

/* A truncated MIPS state is a difference too, not a shorter match. */
static void test_missing_trailing_line_fails(void) {
    reset();
    const char *tty = PSX_HEADER PSX_DONE
        "psx-state: clock year 3400 month 2\n"
        "psx-state: difficulty reasonable\n"
        "psx-state: planet p1 earthlike - world 1.00 2.00 screen 3.00 4.00 visible 1\n"
        "psx-state: planet p2 unknown planet - world 5.00 6.00 screen 7.00 8.00 visible 0\n";
    assert(run(tty, FRAMES) != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "pc:  count planets 2") != NULL);
    assert(strstr(log, "psx: (end of state)") != NULL);
}

static void test_tty_interleaved_with_emulator_log_lines_passes(void) {
    reset();
    const char *tty =
        "PCSX-Redux booting\r\n"
        "BIOS: OpenBIOS\r\n"
        PSX_HEADER
        "[GPU] vsync 120\n"
        PSX_DONE
        "psx-state: clock year 3400 month 2\r\n"
        "psx-state: difficulty reasonable\n"
        "SPU: dma idle\n"
        "psx-state: planet p1 earthlike - world 1.00 2.00 screen 3.00 4.00 visible 1\n"
        "psx-state: planet p2 unknown planet - world 5.00 6.00 screen 7.00 8.00 visible 0\n"
        "psx-state: zone game 81200 81200 468664\n"
        "psx-state: budget zones 600000 796632\n"
        "psx-state: cap button 5 9 15\n"
        "emulator: exit code 0\n"
        "psx-state: count planets 2\n"
        "emulator: quitting\n";
    assert(run(tty, FRAMES) == 0);
    assert(strstr(read_file(LOG), "game lines match") != NULL);
}

/* A replay that hung or was cut short never looks like a match. */
static void test_missing_done_line_fails(void) {
    reset();
    assert(run(PSX_HEADER PSX_STATE_LINES, FRAMES) != 0);
    assert(strstr(read_file(LOG), "no done line") != NULL);
}

static void test_wrong_frame_count_fails_naming_both_counts(void) {
    reset();
    assert(run(PSX_HEADER PSX_DONE PSX_STATE_LINES, "402") != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "401") != NULL);
    assert(strstr(log, "402") != NULL);
    assert(strstr(log, "game lines match") != NULL);
}

static void test_stopped_p8_fatal_fails(void) {
    reset();
    const char *tty = PSX_HEADER
        "p8_fatal: zone game full\n"
        "psx-state: stopped p8_fatal frame 300\n"
        PSX_DONE PSX_STATE_LINES;
    assert(run(tty, FRAMES) != 0);
    assert(strstr(read_file(LOG), "stopped p8_fatal frame 300") != NULL);
}

static void test_tty_without_state_lines_fails(void) {
    reset();
    assert(run(PSX_HEADER PSX_DONE, FRAMES) != 0);
    assert(strstr(read_file(LOG), "no psx-state") != NULL);
}


/* The meter lines differ when the MIPS replay runs without the meter: measured, not compared. */
static void test_gpu_meter_lines_are_measured_not_compared(void) {
    reset();
    write_file(PC, PC_STATE "zone gpu_frame 4000 9000 10240\nzone gpu_build 0 800 9216\ncap gpu_cluts - 5 6\n"
                            "cap gpu_patterns - 3 4\n");
    const char *tty = PSX_HEADER PSX_DONE PSX_STATE_LINES
        "psx-state: zone gpu_frame 0 0 10240\n"
        "psx-state: zone gpu_build 0 0 9216\n"
        "psx-state: cap gpu_cluts - 0 6\n"
        "psx-state: cap gpu_patterns - 0 4\n";
    assert(run(tty, FRAMES) == 0);
    assert(strstr(read_file(LOG), "game lines match") != NULL);
    const char *measure = read_file(MEASURE);
    assert(strstr(measure, "zone gpu_frame 0 0 10240\n") != NULL);
    assert(strstr(measure, "zone gpu_build 0 0 9216\n") != NULL);
    assert(strstr(measure, "cap gpu_cluts - 0 6\n") != NULL);
    assert(strstr(measure, "cap gpu_patterns - 0 4\n") != NULL);
}

int main(void) {
    test_match_with_different_zone_lines_passes_and_writes_measurements();
    test_differing_planet_line_fails_naming_both_lines();
    test_missing_trailing_line_fails();
    test_tty_interleaved_with_emulator_log_lines_passes();
    test_missing_done_line_fails();
    test_wrong_frame_count_fails_naming_both_counts();
    test_stopped_p8_fatal_fails();
    test_tty_without_state_lines_fails();
    test_gpu_meter_lines_are_measured_not_compared();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_state_compare: all tests passed\n");
    return 0;
}
