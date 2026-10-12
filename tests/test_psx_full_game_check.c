/* tools/psx_full_game_check.sh on a stub install whose pcsx-redux prints a crafted TTY, the
 * software replay's or the GPU replay's by the PS-EXE it is given: no emulator is ever booted. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_psx_full_game_check_work"
#define INSTALL DIR "/install"
#define OUT DIR "/out"
#define LOG DIR "/log.txt"
#define PC_STATE DIR "/pc_state.txt"
#define PC_DIGEST DIR "/pc_digest.txt"

#define STATE_LINES "zone gpu_frame 100 200\ncap gpu_cluts 3 8\ncap gpu_patterns 2 8\nscore 40\nframe 30 ok\n"
#define DIGEST_LINES "sprite-sheet 0a1b2c3d\nscreen 0 11111111\nmap 0000beef\n"
#define DONE "psx-replay: done full_game 30 vsyncs 250\r\n"
#define GPU_DONE "psx-replay: done full_game 30 vsyncs 300\r\n"

/* Picks the TTY by the PS-EXE's name and records each boot's ceiling, timeout and capture script
 * size; STUB_ABORT=1 makes the GPU boot die on SIGABRT. */
#define STUB                                                                                             \
    "case \"$*\" in *psx-gpu-replay.exe*) which=gpu ;; *) which=replay ;; esac; "                           \
    "bytes=none; [ -z \"$PSX_CAPTURE_SCRIPT\" ] || bytes=$(wc -c < \"$PSX_CAPTURE_SCRIPT\" | tr -d \" \"); " \
    "echo \"$which $PSX_CEILING_FRAMES $PSX_BOOT_TIMEOUT capture $bytes\" >> \"$STUB_DIR/boots.txt\"; "    \
    "cat \"$STUB_DIR/$which.tty\"; "                                                                       \
    "[ \"$which-$STUB_ABORT\" != gpu-1 ] || kill -ABRT $$; "                                               \
    "echo \"exit 250\" > \"$PSX_RESULT_FILE\""

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
    if (file == NULL) return buf;
    size_t len = fread(buf, 1, sizeof(buf) - 1, file);
    fclose(file);
    buf[len] = '\0';
    return buf;
}

static void assert_has(const char *path, const char *text) {
    if (strstr(read_file(path), text) == NULL) {
        fprintf(stderr, "missing \"%s\" in %s:\n%s", text, path, read_file(path));
        assert(0);
    }
}

/* A replay TTY: progress, the state lines behind "psx-state: ", then `digests` and `done`. */
static void write_tty(const char *path, const char *state, const char *digests, const char *done) {
    static char tty[1 << 14];
    int len = snprintf(tty, sizeof tty, "psx-replay: asserts on\r\npsx-replay: frame 256\r\n");
    for (const char *line = state; *line != '\0';) {
        const char *end = strchr(line, '\n');
        len += snprintf(tty + len, sizeof tty - (size_t)len, "psx-state: %.*s\r\n", (int)(end - line), line);
        line = end + 1;
    }
    snprintf(tty + len, sizeof tty - (size_t)len, "%s%s", digests, done);
    write_file(path, tty);
}

#define PSX_DIGESTS "psx-digest: sprite-sheet 0a1b2c3d\r\npsx-digest: screen 0 11111111\r\npsx-digest: map 0000beef\r\n"

static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " INSTALL "/src/mips/openbios " DIR "/build") == 0);
    write_file(INSTALL "/psx-emulator.stamp", "stub\n");
    write_file(INSTALL "/src/mips/openbios/openbios.bin", "bios\n");
    write_file(INSTALL "/pcsx-redux", "#!/bin/sh\neval \"$PSX_STUB\"\n");
    write_file(DIR "/size", "#!/bin/sh\necho '   text    data     bss     dec     hex filename'\n"
                            "echo \"   1000     200     300    1500     5dc $1\"\n");
    assert(system("chmod +x " INSTALL "/pcsx-redux " DIR "/size") == 0);
    write_file(DIR "/build/psx-replay.exe", "exe\n");
    write_file(DIR "/build/psx-gpu-replay.exe", "exe\n");
    write_file(PC_STATE, STATE_LINES);
    write_file(PC_DIGEST, DIGEST_LINES);
    write_tty(DIR "/replay.tty", STATE_LINES, PSX_DIGESTS, DONE);
    write_tty(DIR "/gpu.tty", STATE_LINES, "", GPU_DONE);
}

/* Runs the check (ceilings 900 and 950, timeouts 40 and 45); output in LOG. */
static int run_check(const char *stub_abort) {
    char command[2048];
    snprintf(command, sizeof command,
             "STUB_DIR=\"$PWD/" DIR "\" STUB_ABORT=%s PSX_STUB='" STUB "' sh tools/psx_full_game_check.sh " INSTALL
             " " DIR "/build/psx-replay.exe " DIR "/build/psx-gpu-replay.exe " PC_STATE " " PC_DIGEST
             " 30 900 40 950 45 " DIR "/size " OUT " > " LOG " 2>&1",
             stub_abort);
    return system(command);
}

static void test_both_replays_reaching_the_pc_frame_count_pass_the_seven_conditions(void) {
    setup();
    assert(run_check("0") == 0);
    assert_has(LOG, "psx-full-game-check: replay-done: ok\n");
    assert_has(LOG, "psx-full-game-check: state-game-lines: ok\n");
    assert_has(LOG, "psx-full-game-check: sprite-sheet: ok\n");
    assert_has(LOG, "psx-full-game-check: screen: ok\n");
    assert_has(LOG, "psx-full-game-check: map: ok\n");
    assert_has(LOG, "psx-full-game-check: gpu-replay-done: ok\n");
    assert_has(LOG, "psx-full-game-check: gpu-state-game-lines: ok\n");
    assert_has(LOG, "psx-full-game-check: all seven conditions hold\n");
    assert_has(OUT "/conditions.txt", "gpu-replay-done ok\n");
}

static void test_the_gpu_boot_gets_an_empty_capture_script_and_its_own_ceiling_and_timeout(void) {
    setup();
    assert(run_check("0") == 0);
    assert(strcmp(read_file(DIR "/boots.txt"), "replay 900 40 capture none\ngpu 950 45 capture 0\n") == 0);
}

static void test_measurements_carry_the_gpu_replay_vsyncs_meter_lines_and_size_behind_gpu(void) {
    setup();
    assert(run_check("0") == 0);
    assert_has(OUT "/measurements.txt", "vsyncs 250\n");
    assert_has(OUT "/measurements.txt", "gpu vsyncs 300\n");
    assert_has(OUT "/measurements.txt",
               "gpu zone gpu_frame 100 200\ngpu cap gpu_cluts 3 8\ngpu cap gpu_patterns 2 8\n");
    assert_has(OUT "/measurements.txt",
               "gpu    1000     200     300    1500     5dc " DIR "/build/psx-gpu-replay.elf\n");
}

/* `condition` failed with `reason`, and the software replay's conditions still passed. */
static void assert_only_gpu_failed(const char *condition, const char *reason) {
    char line[512];
    snprintf(line, sizeof line, "psx-full-game-check: %s: %s", condition, reason);
    assert_has(LOG, line);
    snprintf(line, sizeof line, "%s failed\n", condition);
    assert_has(OUT "/conditions.txt", line);
    assert_has(LOG, "psx-full-game-check: replay-done: ok\n");
    assert_has(LOG, "psx-full-game-check: map: ok\n");
    assert(strstr(read_file(LOG), "all seven conditions hold") == NULL);
}

static void test_a_missing_gpu_done_line_fails_gpu_replay_done(void) {
    setup();
    write_tty(DIR "/gpu.tty", STATE_LINES, "", "");
    assert(run_check("0") != 0);
    assert_only_gpu_failed("gpu-replay-done", "expected 30 frames, got: (no done line)\n");
}

static void test_a_wrong_gpu_frame_count_fails_gpu_replay_done(void) {
    setup();
    write_tty(DIR "/gpu.tty", STATE_LINES, "", "psx-replay: done full_game 29 vsyncs 290\r\n");
    assert(run_check("0") != 0);
    assert_only_gpu_failed("gpu-replay-done",
                           "expected 30 frames, got: psx-replay: done full_game 29 vsyncs 290\n");
}

static void test_a_gpu_p8_fatal_fails_gpu_replay_done(void) {
    setup();
    write_tty(DIR "/gpu.tty", STATE_LINES "stopped p8_fatal zone full\n", "", GPU_DONE);
    assert(run_check("0") != 0);
    assert_only_gpu_failed("gpu-replay-done", "the PSX replay stopped: stopped p8_fatal zone full\n");
}

static void test_a_gpu_emulator_abort_fails_gpu_replay_done_naming_the_signal(void) {
    setup();
    write_tty(DIR "/gpu.tty", STATE_LINES, "", "");
    assert(run_check("1") != 0);
    assert_only_gpu_failed("gpu-replay-done", "psx-boot: ");
    assert_has(LOG, "psx-gpu-replay.exe: emulator aborted with status 134 (signal SIGABRT); last progress line: "
                    "psx-replay: frame 256;");
}

static void test_a_differing_gpu_game_line_fails_gpu_state_game_lines(void) {
    setup();
    write_tty(DIR "/gpu.tty", "score 41\nframe 30 ok\n", "", GPU_DONE);
    assert(run_check("0") != 0);
    assert_only_gpu_failed("gpu-state-game-lines",
                           "psx-state-compare: first differing game line, 1:\n  psx: score 41\n  pc:  score 40\n");
}

static void test_a_differing_screen_digest_fails_screen(void) {
    setup();
    write_tty(DIR "/replay.tty", STATE_LINES,
              "psx-digest: sprite-sheet 0a1b2c3d\r\npsx-digest: screen 0 99999999\r\npsx-digest: map 0000beef\r\n",
              DONE);
    assert(run_check("0") != 0);
    assert_has(LOG, "psx-full-game-check: screen: first differing screen line, 1:\n  psx: screen 0 99999999\n"
                    "  pc:  screen 0 11111111\n");
    assert_has(LOG, "psx-full-game-check: map: ok\n");
    assert_has(LOG, "psx-full-game-check: gpu-replay-done: ok\n");
}

int main(void) {
    test_both_replays_reaching_the_pc_frame_count_pass_the_seven_conditions();
    test_the_gpu_boot_gets_an_empty_capture_script_and_its_own_ceiling_and_timeout();
    test_measurements_carry_the_gpu_replay_vsyncs_meter_lines_and_size_behind_gpu();
    test_a_missing_gpu_done_line_fails_gpu_replay_done();
    test_a_wrong_gpu_frame_count_fails_gpu_replay_done();
    test_a_gpu_p8_fatal_fails_gpu_replay_done();
    test_a_gpu_emulator_abort_fails_gpu_replay_done_naming_the_signal();
    test_a_differing_gpu_game_line_fails_gpu_state_game_lines();
    test_a_differing_screen_digest_fails_screen();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_full_game_check: all tests passed\n");
    return 0;
}
