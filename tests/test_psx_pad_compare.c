/* tools/psx_pad_compare.sh, the pad-script condition of `make psx-check`, on
 * hand-written pad scripts and TTY files: no emulator. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT "sh tools/psx_pad_compare.sh"
#define DIR "tmp/test_psx_pad_compare_work"
#define PAD DIR "/pad.txt"
#define TTY DIR "/tty.txt"
#define LOG DIR "/log.txt"

#define PAD_SCRIPT \
    "# a comment\n" \
    "\n" \
    "20 CROSS\n" \
    "40 CROSS UP\n" \
    "60 -\n" \
    "80 START SELECT\n" \
    "100 -\n"

#define PAD_STATES \
    "psx-pad: ready\n" \
    "psx-pad: 4000\n" \
    "psx-pad: 4010\n" \
    "psx-pad: 0000\n" \
    "psx-pad: 0009\n" \
    "psx-pad: 0000\n"

#define PAD_DONE "psx-pad: done\n"

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

static int run(const char *pad_script, const char *tty) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(PAD, pad_script);
    write_file(TTY, tty);
    return system(SCRIPT " " PAD " " TTY " > " LOG " 2>&1");
}

static void test_matching_states_pass(void) {
    assert(run(PAD_SCRIPT, PAD_STATES PAD_DONE) == 0);
    assert(strstr(read_file(LOG), "pad states match (5 states)") != NULL);
}

static void test_tty_interleaved_with_emulator_log_lines_passes(void) {
    const char *tty =
        "PCSX-Redux booting\r\n"
        "psx-pad: ready\r\n"
        "psx-pad: 4000\r\n"
        "SIO0: pad poll\n"
        "psx-pad: 4010\n"
        "psx-pad: 0000\n"
        "[GPU] vsync 120\n"
        "psx-pad: 0009\n"
        "psx-pad: 0000\n"
        "psx-pad: done\r\n"
        "emulator: quitting\n";
    assert(run(PAD_SCRIPT, tty) == 0);
}

/* A step that holds the same buttons as the one before changes nothing the
 * probe can see. */
static void test_repeated_step_expects_no_new_state(void) {
    const char *pad_script = "20 CROSS\n40 CROSS\n60 -\n";
    assert(run(pad_script, "psx-pad: 4000\npsx-pad: 0000\n" PAD_DONE) == 0);
    assert(strstr(read_file(LOG), "pad states match (2 states)") != NULL);
}

/* Every button name in hardware bit order, 1 = pressed. */
static void test_every_button_maps_to_its_hardware_bit(void) {
    const char *pad_script =
        "10 SELECT START\n"
        "20 UP RIGHT DOWN LEFT\n"
        "30 L2 R2 L1 R1\n"
        "40 TRIANGLE CIRCLE CROSS SQUARE\n"
        "50 -\n";
    const char *tty = "psx-pad: 0009\npsx-pad: 00f0\npsx-pad: 0f00\npsx-pad: f000\npsx-pad: 0000\n" PAD_DONE;
    assert(run(pad_script, tty) == 0);
}

static void test_ignored_pad_fails_naming_the_first_expected_state(void) {
    assert(run(PAD_SCRIPT, "psx-pad: ready\n") != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "first differing pad state, 1:") != NULL);
    assert(strstr(log, "probe:  (none)") != NULL);
    assert(strstr(log, "script: 4000") != NULL);
    assert(strstr(log, "pad states match") == NULL);
}

static void test_misordered_states_fail_naming_both(void) {
    const char *tty =
        "psx-pad: 4000\npsx-pad: 0000\npsx-pad: 4010\npsx-pad: 0009\npsx-pad: 0000\n" PAD_DONE;
    assert(run(PAD_SCRIPT, tty) != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "first differing pad state, 2:") != NULL);
    assert(strstr(log, "probe:  0000") != NULL);
    assert(strstr(log, "script: 4010") != NULL);
}

static void test_extra_state_fails(void) {
    assert(run(PAD_SCRIPT, PAD_STATES "psx-pad: 0001\n" PAD_DONE) != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "first differing pad state, 6:") != NULL);
    assert(strstr(log, "script: (none)") != NULL);
}

/* The probe's done line says it saw the end of the script with no button
 * held; without it a stuck pad could pass on a truncated TTY. */
static void test_missing_done_line_fails(void) {
    assert(run(PAD_SCRIPT, PAD_STATES) != 0);
    assert(strstr(read_file(LOG), "no done line") != NULL);
}

static void test_unknown_button_fails_naming_it(void) {
    assert(run("20 CROSS\n40 JUMP\n60 -\n", PAD_STATES PAD_DONE) != 0);
    const char *log = read_file(LOG);
    assert(strstr(log, "line 2") != NULL);
    assert(strstr(log, "unknown button JUMP") != NULL);
    /* The emulator's digital pad has no L3/R3 override names. */
    assert(run("20 L3\n40 -\n", PAD_STATES PAD_DONE) != 0);
    assert(strstr(read_file(LOG), "unknown button L3") != NULL);
}

static void test_offsets_must_increase(void) {
    assert(run("20 CROSS\n20 UP\n60 -\n", PAD_STATES PAD_DONE) != 0);
    assert(strstr(read_file(LOG), "line 2") != NULL);
    assert(run("20 CROSS\nsoon UP\n60 -\n", PAD_STATES PAD_DONE) != 0);
    assert(strstr(read_file(LOG), "line 2") != NULL);
}

/* The probe ends on "no button held after the last step". */
static void test_last_step_must_release_every_button(void) {
    assert(run("20 CROSS\n", "psx-pad: 4000\n" PAD_DONE) != 0);
    assert(strstr(read_file(LOG), "last step must be -") != NULL);
}

static void test_empty_script_fails(void) {
    assert(run("# nothing\n", PAD_DONE) != 0);
    assert(strstr(read_file(LOG), "no step") != NULL);
}

int main(void) {
    test_matching_states_pass();
    test_tty_interleaved_with_emulator_log_lines_passes();
    test_repeated_step_expects_no_new_state();
    test_every_button_maps_to_its_hardware_bit();
    test_ignored_pad_fails_naming_the_first_expected_state();
    test_misordered_states_fail_naming_both();
    test_extra_state_fails();
    test_missing_done_line_fails();
    test_unknown_button_fails_naming_it();
    test_offsets_must_increase();
    test_last_step_must_release_every_button();
    test_empty_script_fails();
    assert(system("rm -rf " DIR) == 0);
    printf("test_psx_pad_compare: all tests passed\n");
    return 0;
}
