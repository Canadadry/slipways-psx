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
#define SITUATIONS DIR "/situations.txt"

#define STATE_LINES                                                                                 \
    "zone game 10 20 100\nbudget zones 300 400\nzone gpu_frame 100 150 200\nzone gpu_build 0 70 90\n" \
    "cap gpu_cluts - 3 8\ncap gpu_patterns - 2 8\nscore 40\nframe 30 ok\n"
/* The GPU boot's MIPS state: no meter is installed, so its gpu_* lines are zeros. */
#define GPU_STATE_LINES                                                                           \
    "zone game 10 20 100\nbudget zones 300 400\nzone gpu_frame 0 0 200\nzone gpu_build 0 0 90\n" \
    "cap gpu_cluts - 0 8\ncap gpu_patterns - 0 8\nscore 40\nframe 30 ok\n"
#define DIGEST_LINES "sprite-sheet 0a1b2c3d\nscreen 0 11111111\nmap 0000beef\n"
#define DONE "psx-replay: done full_game 30 vsyncs 250\r\n"
#define GPU_DONE "psx-replay: done full_game 30 vsyncs 300\r\n"
#define FILTER_OFF \
    "psx-replay: backend gpu\r\npsx-replay: draw-filter off\r\npsx-replay: draw-meter off\r\n"

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

/* The GPU replay's frame lines: calibration, then frames 0 to 29 at 180 hblanks, `slow_frame` (-1
 * for none) at 300. */
static const char *frame_lines(int slow_frame) {
    static char lines[1 << 12];
    int len = snprintf(lines, sizeof lines, "psx-frame: hblanks-per-vsync 263\r\n");
    for (int frame = 0; frame < 30; frame++) {
        int submit = frame == slow_frame ? 190 : 70;
        len += snprintf(lines + len, sizeof lines - (size_t)len,
                        "psx-frame: frame %d update 50 draw 60 submit %d total %d vsyncs 1\r\n", frame, submit,
                        110 + submit);
    }
    return lines;
}

/* A replay TTY: the asserts line, `modes`, progress, `frames`, the state lines behind "psx-state: ",
 * then `digests` and `done`. */
static void write_tty(const char *path, const char *modes, const char *frames, const char *state,
                      const char *digests, const char *done) {
    static char tty[1 << 14];
    int len = snprintf(tty, sizeof tty, "psx-replay: asserts on\r\n%spsx-replay: frame 256\r\n%s", modes, frames);
    for (const char *line = state; *line != '\0';) {
        const char *end = strchr(line, '\n');
        len += snprintf(tty + len, sizeof tty - (size_t)len, "psx-state: %.*s\r\n", (int)(end - line), line);
        line = end + 1;
    }
    snprintf(tty + len, sizeof tty - (size_t)len, "%s%s", digests, done);
    write_file(path, tty);
}

/* The situation table of the stub's 30 frames: every frame `steady` but `one_off_frame` (-1 for none). */
static void write_situations(int one_off_frame) {
    static char table[1 << 12];
    int len = snprintf(table, sizeof table, "frame situation family\n");
    for (int frame = 0; frame < 30; frame++) {
        len += snprintf(table + len, sizeof table - (size_t)len, "%d %s\n", frame,
                        frame == one_off_frame ? "build one-off" : "map steady");
    }
    write_file(SITUATIONS, table);
}

#define PSX_DIGESTS "psx-digest: sprite-sheet 0a1b2c3d\r\npsx-digest: screen 0 11111111\r\npsx-digest: map 0000beef\r\n"

static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " INSTALL "/src/mips/openbios " DIR "/build") == 0);
    write_file(INSTALL "/psx-emulator.stamp", "stub\n");
    write_file(INSTALL "/src/mips/openbios/openbios.bin", "bios\n");
    write_file(INSTALL "/pcsx-redux", "#!/bin/sh\neval \"$PSX_STUB\"\n");
    write_file(DIR "/size", "#!/bin/sh\necho '   text    data     bss     dec     hex filename'\n"
                            "echo \"   1000     200     ${STUB_BSS:-300}    1500     5dc $1\"\n");
    assert(system("chmod +x " INSTALL "/pcsx-redux " DIR "/size") == 0);
    write_file(DIR "/build/psx-replay.exe", "exe\n");
    write_file(DIR "/build/psx-gpu-replay.exe", "exe\n");
    write_file(PC_STATE, STATE_LINES);
    write_file(PC_DIGEST, DIGEST_LINES);
    write_tty(DIR "/replay.tty", "", "", STATE_LINES, PSX_DIGESTS, DONE);
    write_tty(DIR "/gpu.tty", FILTER_OFF, frame_lines(-1), GPU_STATE_LINES, "", GPU_DONE);
    write_situations(7);
}

/* Runs the check (ceilings 900 and 950, timeouts 40 and 45, budget 263 hblanks, SITUATIONS, one-off
 * allowance 10800 vsyncs, 64 KiB stack reserve)
 * with the size tool printing `bss` as every ELF's bss; output in LOG. */
static int run_check_bss(const char *stub_abort, const char *bss) {
    char command[2048];
    snprintf(command, sizeof command,
             "STUB_DIR=\"$PWD/" DIR "\" STUB_ABORT=%s STUB_BSS=%s PSX_STUB='" STUB
             "' sh tools/psx_full_game_check.sh " INSTALL " " DIR "/build/psx-replay.exe " DIR
             "/build/psx-gpu-replay.exe " PC_STATE " " PC_DIGEST " 30 900 40 950 45 263 " SITUATIONS
             " 10800 " DIR "/size 65536 " OUT " > " LOG " 2>&1",
             stub_abort, bss);
    return system(command);
}

static int run_check(const char *stub_abort) { return run_check_bss(stub_abort, "300"); }

static int line_count(const char *path) {
    int lines = 0;
    for (const char *c = read_file(path); *c != '\0'; c++) {
        if (*c == '\n') lines++;
    }
    return lines;
}

static void test_both_replays_reaching_the_pc_frame_count_within_budget_pass_every_condition(void) {
    setup();
    assert(run_check("0") == 0);
    assert_has(LOG, "psx-full-game-check: replay-done: ok\n");
    assert_has(LOG, "psx-full-game-check: state-game-lines: ok\n");
    assert_has(LOG, "psx-full-game-check: sprite-sheet: ok\n");
    assert_has(LOG, "psx-full-game-check: screen: ok\n");
    assert_has(LOG, "psx-full-game-check: map: ok\n");
    assert_has(LOG, "psx-full-game-check: gpu-replay-done: ok\n");
    assert_has(LOG, "psx-full-game-check: gpu-state-game-lines: ok\n");
    assert_has(LOG, "psx-full-game-check: gpu-draw-filter: ok\n");
    assert_has(LOG, "psx-full-game-check: gpu-draw-meter: ok\n");
    assert_has(LOG, "psx-full-game-check: timer: ok, hblanks-per-vsync 263, 30 frames\n");
    assert_has(LOG, "psx-full-game-check: frame-budget: ok, worst frame 0 total 180\n");
    assert_has(LOG, "psx-full-game-check: zone-sizes: ok, 3 zones and the budget fit\n");
    assert_has(LOG, "psx-full-game-check: cap-sizes: ok, 2 caps fit\n");
    assert_has(LOG, "psx-full-game-check: real-game steady-over 0 frames, worst steady frame -\n"
                    "psx-full-game-check: real-game one-off-over 0 frames, vsyncs 0 of 10800\n");
    assert_has(LOG, "psx-full-game-check: real-game-budget: ok, steady-over 0, one-off vsyncs 0 of 10800\n");
    assert_has(LOG, "psx-full-game-check: gpu-ram-room: ok, 1964580 bytes left in 2097152 (kernel 65536 + ELF + "
                    "stack 65536 = 132572)\n");
    assert_has(OUT "/conditions.txt", "gpu-replay-done ok\ngpu-state-game-lines ok\ngpu-draw-filter ok\ngpu-draw-meter ok\n"
                                      "timer ok\nframe-budget ok\nzone-sizes ok\ncap-sizes ok\nreal-game-budget ok\ngpu-ram-room ok\n");
    char line[128];
    snprintf(line, sizeof line, "psx-full-game-check: all %d conditions hold\n", line_count(OUT "/conditions.txt"));
    assert_has(LOG, line);
}

static void test_the_gpu_boot_gets_an_empty_capture_script_and_its_own_ceiling_and_timeout(void) {
    setup();
    assert(run_check("0") == 0);
    assert(strcmp(read_file(DIR "/boots.txt"), "replay 900 40 capture none\ngpu 950 45 capture 0\n") == 0);
}

static void test_measurements_carry_the_gpu_replay_vsyncs_the_pc_meter_lines_and_size_behind_gpu(void) {
    setup();
    assert(run_check("0") == 0);
    assert_has(OUT "/measurements.txt", "vsyncs 250\n");
    assert_has(OUT "/measurements.txt", "gpu vsyncs 300\n");
    assert_has(OUT "/measurements.txt",
               "gpu zone gpu_frame 100 150 200\ngpu zone gpu_build 0 70 90\ngpu cap gpu_cluts - 3 8\n"
               "gpu cap gpu_patterns - 2 8\n");
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
    assert(strstr(read_file(LOG), "conditions hold") == NULL);
}

static void test_a_missing_gpu_done_line_fails_gpu_replay_done(void) {
    setup();
    write_tty(DIR "/gpu.tty", FILTER_OFF, frame_lines(-1), GPU_STATE_LINES, "", "");
    assert(run_check("0") != 0);
    assert_only_gpu_failed("gpu-replay-done", "expected 30 frames, got: (no done line)\n");
}

static void test_a_wrong_gpu_frame_count_fails_gpu_replay_done(void) {
    setup();
    write_tty(DIR "/gpu.tty", FILTER_OFF, frame_lines(-1), GPU_STATE_LINES, "",
              "psx-replay: done full_game 29 vsyncs 290\r\n");
    assert(run_check("0") != 0);
    assert_only_gpu_failed("gpu-replay-done",
                           "expected 30 frames, got: psx-replay: done full_game 29 vsyncs 290\n");
}

static void test_a_gpu_p8_fatal_fails_gpu_replay_done(void) {
    setup();
    write_tty(DIR "/gpu.tty", FILTER_OFF, frame_lines(-1), GPU_STATE_LINES "stopped p8_fatal zone full\n", "", GPU_DONE);
    assert(run_check("0") != 0);
    assert_only_gpu_failed("gpu-replay-done", "the PSX replay stopped: stopped p8_fatal zone full\n");
}

static void test_a_gpu_emulator_abort_fails_gpu_replay_done_naming_the_signal(void) {
    setup();
    write_tty(DIR "/gpu.tty", FILTER_OFF, frame_lines(-1), GPU_STATE_LINES, "", "");
    assert(run_check("1") != 0);
    assert_only_gpu_failed("gpu-replay-done", "psx-boot: ");
    assert_has(LOG, "psx-gpu-replay.exe: emulator aborted with status 134 (signal SIGABRT); last progress line: "
                    "psx-replay: frame 256;");
}

static void test_a_differing_gpu_game_line_fails_gpu_state_game_lines(void) {
    setup();
    write_tty(DIR "/gpu.tty", FILTER_OFF, frame_lines(-1), "score 41\nframe 30 ok\n", "", GPU_DONE);
    assert(run_check("0") != 0);
    assert_only_gpu_failed("gpu-state-game-lines",
                           "psx-state-compare: first differing game line, 1:\n  psx: score 41\n  pc:  score 40\n");
}

static void assert_only_draw_filter_failed(const char *reason) {
    assert_only_gpu_failed("gpu-draw-filter", reason);
    assert_has(OUT "/conditions.txt", "gpu-replay-done ok\ngpu-state-game-lines ok\ngpu-draw-filter failed\ngpu-draw-meter ok\n"
                                      "timer ok\n"
                                      "frame-budget ok\nzone-sizes ok\ncap-sizes ok\nreal-game-budget ok\ngpu-ram-room ok\n");
}

static void test_a_gpu_boot_built_with_the_draw_filter_fails_gpu_draw_filter(void) {
    setup();
    write_tty(DIR "/gpu.tty", "psx-replay: backend gpu\r\npsx-replay: draw-filter on\r\npsx-replay: draw-meter off\r\n",
              frame_lines(-1), GPU_STATE_LINES,
              "", GPU_DONE);
    assert(run_check("0") != 0);
    assert_only_draw_filter_failed("expected psx-replay: draw-filter off, got: psx-replay: draw-filter on\n");
}

static void test_a_gpu_boot_without_a_draw_filter_line_fails_gpu_draw_filter(void) {
    setup();
    write_tty(DIR "/gpu.tty", "psx-replay: backend gpu\r\npsx-replay: draw-meter off\r\n", frame_lines(-1), GPU_STATE_LINES, "", GPU_DONE);
    assert(run_check("0") != 0);
    assert_only_draw_filter_failed("expected psx-replay: draw-filter off, got: (no line)\n");
}

static void assert_only_draw_meter_failed(const char *reason) {
    assert_only_gpu_failed("gpu-draw-meter", reason);
    assert_has(OUT "/conditions.txt", "gpu-replay-done ok\ngpu-state-game-lines ok\ngpu-draw-filter ok\ngpu-draw-meter failed\n"
                                      "timer ok\nframe-budget ok\nzone-sizes ok\ncap-sizes ok\nreal-game-budget ok\ngpu-ram-room ok\n");
}

static void test_a_gpu_boot_built_with_the_draw_meter_fails_gpu_draw_meter(void) {
    setup();
    write_tty(DIR "/gpu.tty", "psx-replay: backend gpu\r\npsx-replay: draw-filter off\r\npsx-replay: draw-meter on\r\n",
              frame_lines(-1), GPU_STATE_LINES, "", GPU_DONE);
    assert(run_check("0") != 0);
    assert_only_draw_meter_failed("expected psx-replay: draw-meter off, got: psx-replay: draw-meter on\n");
}

static void test_a_gpu_boot_without_a_draw_meter_line_fails_gpu_draw_meter(void) {
    setup();
    write_tty(DIR "/gpu.tty", "psx-replay: backend gpu\r\npsx-replay: draw-filter off\r\n", frame_lines(-1),
              GPU_STATE_LINES, "", GPU_DONE);
    assert(run_check("0") != 0);
    assert_only_draw_meter_failed("expected psx-replay: draw-meter off, got: (no line)\n");
}

static void test_a_pc_gpu_frame_peak_above_its_limit_fails_zone_sizes(void) {
    setup();
    write_file(PC_STATE, "zone game 10 20 100\nbudget zones 300 400\nzone gpu_frame 100 250 200\n"
                         "cap gpu_cluts - 3 8\ncap gpu_patterns - 2 8\nscore 40\nframe 30 ok\n");
    assert(run_check("0") != 0);
    assert_has(LOG, "psx-full-game-check: zone-sizes: ");
    assert_has(LOG, "zone gpu_frame peak 250 limit 200");
    assert_has(OUT "/conditions.txt", "zone-sizes failed\n");
    assert_has(OUT "/measurements.txt", "gpu zone gpu_frame 100 250 200\n");
}

static void test_a_pc_gpu_build_peak_above_its_limit_fails_zone_sizes(void) {
    setup();
    write_file(PC_STATE, "zone game 10 20 100\nbudget zones 300 400\nzone gpu_frame 100 150 200\n"
                         "zone gpu_build 0 95 90\ncap gpu_cluts - 3 8\ncap gpu_patterns - 2 8\nscore 40\nframe 30 ok\n");
    assert(run_check("0") != 0);
    assert_has(LOG, "zone gpu_build peak 95 limit 90");
    assert_has(OUT "/conditions.txt", "zone-sizes failed\n");
    assert_has(OUT "/measurements.txt", "gpu zone gpu_build 0 95 90\n");
}

static void test_the_gpu_replay_backend_peaks_are_measured_behind_gpu_gpu_peak(void) {
    setup();
    write_tty(DIR "/gpu.tty", FILTER_OFF, frame_lines(-1), GPU_STATE_LINES, "",
              "psx-replay: gpu-peak bytes 120 cluts 2 patterns 1 build 96\r\n" GPU_DONE);
    assert(run_check("0") == 0);
    assert_has(OUT "/measurements.txt", "gpu gpu-peak bytes 120 cluts 2 patterns 1 build 96\n");
}

static void test_a_differing_screen_digest_fails_screen(void) {
    setup();
    write_tty(DIR "/replay.tty", "", "", STATE_LINES,
              "psx-digest: sprite-sheet 0a1b2c3d\r\npsx-digest: screen 0 99999999\r\npsx-digest: map 0000beef\r\n",
              DONE);
    assert(run_check("0") != 0);
    assert_has(LOG, "psx-full-game-check: screen: first differing screen line, 1:\n  psx: screen 0 99999999\n"
                    "  pc:  screen 0 11111111\n");
    assert_has(LOG, "psx-full-game-check: map: ok\n");
    assert_has(LOG, "psx-full-game-check: gpu-replay-done: ok\n");
}

static void test_an_over_budget_frame_fails_only_frame_budget_and_every_condition_is_still_printed(void) {
    setup();
    write_tty(DIR "/gpu.tty", FILTER_OFF, frame_lines(7), GPU_STATE_LINES, "", GPU_DONE);
    assert(run_check("0") != 0);
    assert_has(LOG, "psx-full-game-check: frame-budget: 1 frames over 263 hblanks, worst frame 7 total 300\n");
    assert_has(OUT "/conditions.txt", "replay-done ok\nstate-game-lines ok\nsprite-sheet ok\nscreen ok\nmap ok\n"
                                      "gpu-replay-done ok\ngpu-state-game-lines ok\ngpu-draw-filter ok\ngpu-draw-meter ok\n"
                                      "timer ok\nframe-budget failed\nzone-sizes ok\ncap-sizes ok\nreal-game-budget ok\ngpu-ram-room ok\n");
    assert(strstr(read_file(LOG), "conditions hold") == NULL);
}

static void test_an_over_budget_steady_frame_also_fails_real_game_budget_and_every_condition_is_still_printed(void) {
    setup();
    write_tty(DIR "/gpu.tty", FILTER_OFF, frame_lines(7), GPU_STATE_LINES, "", GPU_DONE);
    write_situations(-1);
    assert(run_check("0") != 0);
    assert_has(LOG, "psx-full-game-check: real-game steady-over 1 frames, worst steady frame 7 total 300 situation map\n");
    assert_has(LOG, "psx-full-game-check: real-game-budget: 1 steady frames over 263, one-off vsyncs 0 of 10800\n");
    assert_has(OUT "/conditions.txt", "replay-done ok\nstate-game-lines ok\nsprite-sheet ok\nscreen ok\nmap ok\n"
                                      "gpu-replay-done ok\ngpu-state-game-lines ok\ngpu-draw-filter ok\ngpu-draw-meter ok\n"
                                      "timer ok\nframe-budget failed\nzone-sizes ok\ncap-sizes ok\nreal-game-budget failed\n"
                                      "gpu-ram-room ok\n");
    assert(strstr(read_file(LOG), "conditions hold") == NULL);
}

static void test_a_situation_table_of_another_recording_fails_real_game_budget(void) {
    setup();
    write_file(SITUATIONS, "frame situation family\n0 title steady\n");
    assert(run_check("0") != 0);
    assert_has(LOG, "psx-full-game-check: real-game-budget: frame count 30 vs 1\n");
    assert_has(OUT "/conditions.txt", "real-game-budget failed\n");
}

static void test_the_real_game_figures_are_measured_behind_gpu(void) {
    setup();
    write_tty(DIR "/gpu.tty", FILTER_OFF, frame_lines(7), GPU_STATE_LINES, "", GPU_DONE);
    run_check("0");
    assert_has(OUT "/measurements.txt", "gpu real-game steady-over 0 frames, worst steady frame -\n"
                                        "gpu real-game one-off-over 1 frames, vsyncs 2 of 10800\n");
}

static void test_a_gpu_elf_too_big_for_ram_fails_gpu_ram_room(void) {
    setup();
    assert(run_check_bss("0", "2000000") != 0);
    assert_has(LOG, "psx-full-game-check: gpu-ram-room: kernel 65536 + ELF + stack 65536 = 2132272 bytes, over "
                    "2097152 by 35120\n");
    assert_has(OUT "/conditions.txt", "gpu-ram-room failed\n");
    assert_has(OUT "/measurements.txt", "gpu ram-room 2132272 2097152 -35120\n");
}

static void test_the_frame_lines_and_summary_are_printed_and_the_summary_measured_behind_gpu_frame(void) {
    setup();
    write_tty(DIR "/gpu.tty", FILTER_OFF, frame_lines(7), GPU_STATE_LINES, "", GPU_DONE);
    run_check("0");
    assert_has(LOG, "psx-full-game-check: frame update draw submit total vsyncs\n"
                    "psx-full-game-check: frame 0 50 60 70 180 1\n");
    assert_has(LOG, "psx-full-game-check: frame 7 50 60 190 300 1\n");
    assert_has(LOG, "psx-full-game-check: frame 29 50 60 70 180 1\n");
    assert_has(LOG, "psx-full-game-check: frames 30 budget 263 hblanks\n");
    assert_has(OUT "/measurements.txt", "gpu ram-room 132572 2097152 1964580\n");
    assert_has(OUT "/measurements.txt", "gpu frame frames 30 budget 263 hblanks\n");
    assert_has(OUT "/measurements.txt", "gpu frame parts over-budget frames update 50 draw 60 submit 190 dominant submit\n");
    assert_has(OUT "/measurements.txt", "gpu frame cap gpu_patterns peak 2 limit 8 ok margin 6/8\n");
}

int main(void) {
    test_both_replays_reaching_the_pc_frame_count_within_budget_pass_every_condition();
    test_the_gpu_boot_gets_an_empty_capture_script_and_its_own_ceiling_and_timeout();
    test_measurements_carry_the_gpu_replay_vsyncs_the_pc_meter_lines_and_size_behind_gpu();
    test_a_missing_gpu_done_line_fails_gpu_replay_done();
    test_a_wrong_gpu_frame_count_fails_gpu_replay_done();
    test_a_gpu_p8_fatal_fails_gpu_replay_done();
    test_a_gpu_emulator_abort_fails_gpu_replay_done_naming_the_signal();
    test_a_differing_gpu_game_line_fails_gpu_state_game_lines();
    test_a_gpu_boot_built_with_the_draw_filter_fails_gpu_draw_filter();
    test_a_gpu_boot_without_a_draw_filter_line_fails_gpu_draw_filter();
    test_a_gpu_boot_built_with_the_draw_meter_fails_gpu_draw_meter();
    test_a_gpu_boot_without_a_draw_meter_line_fails_gpu_draw_meter();
    test_a_pc_gpu_frame_peak_above_its_limit_fails_zone_sizes();
    test_a_pc_gpu_build_peak_above_its_limit_fails_zone_sizes();
    test_the_gpu_replay_backend_peaks_are_measured_behind_gpu_gpu_peak();
    test_a_differing_screen_digest_fails_screen();
    test_an_over_budget_frame_fails_only_frame_budget_and_every_condition_is_still_printed();
    test_an_over_budget_steady_frame_also_fails_real_game_budget_and_every_condition_is_still_printed();
    test_a_situation_table_of_another_recording_fails_real_game_budget();
    test_the_real_game_figures_are_measured_behind_gpu();
    test_a_gpu_elf_too_big_for_ram_fails_gpu_ram_room();
    test_the_frame_lines_and_summary_are_printed_and_the_summary_measured_behind_gpu_frame();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_full_game_check: all tests passed\n");
    return 0;
}
