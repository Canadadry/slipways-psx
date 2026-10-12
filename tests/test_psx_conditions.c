/* check_largest_slots of tools/psx_conditions.sh on fixture TTYs of psx-largest-slots: no emulator
 * is ever booted. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_psx_conditions_work"
#define TTY DIR "/largest.tty.txt"
#define LOG DIR "/log.txt"
#define LINES DIR "/lines.txt"
#define OUT DIR "/out"

#define ASSERTS "psx-largest: asserts on\r\n"
#define BUTTON "psx-largest: bytes_button 5000 6056\r\n"
#define ROW "psx-largest: bytes_row 1100 1184\r\n"
#define DONE "psx-largest: done\r\n"

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

/* Static buffer, overwritten by each call; empty when the file is missing. */
static const char *read_file(const char *path) {
    static char buf[1 << 14];
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

/* Writes `tty`, then runs the condition on it as a boot that ended with `status` and printed
 * `boot`; its output in LOG, its largest_lines in LINES. Returns the shell's exit status, non-zero
 * when the condition failed. */
static int run_condition(const char *tty, int status, const char *boot) {
    char command[2048];
    assert(system("rm -rf " DIR " && mkdir -p " OUT) == 0);
    write_file(TTY, tty);
    snprintf(command, sizeof command,
             "sh -c 'check_name=psx-check out_dir=" OUT " script_dir=tools failed=0; . tools/psx_conditions.sh; "
             "check_largest_slots largest-slots %d \"%s\" " TTY "; printf \"%%s\\n\" \"$largest_lines\" > " LINES
             "; exit $failed' > " LOG " 2>&1",
             status, boot);
    return system(command);
}

static void test_both_slots_within_their_size_pass_and_their_lines_are_kept(void) {
    assert(run_condition(ASSERTS BUTTON ROW DONE, 0, "") == 0);
    assert_has(LOG, "psx-check: largest-slots: ok, bytes_button 5000 of 6056, bytes_row 1100 of 1184\n");
    assert_has(OUT "/conditions.txt", "largest-slots ok\n");
    assert(strcmp(read_file(LINES), "bytes_button 5000 6056\nbytes_row 1100 1184\n") == 0);
}

static void test_a_peak_equal_to_its_size_passes(void) {
    assert(run_condition(ASSERTS "psx-largest: bytes_button 6056 6056\r\n" ROW DONE, 0, "") == 0);
}

static void test_a_missing_row_line_fails_naming_it(void) {
    assert(run_condition(ASSERTS BUTTON DONE, 0, "") != 0);
    assert_has(LOG, "psx-check: largest-slots: no bytes_row line\n");
    assert_has(OUT "/conditions.txt", "largest-slots failed\n");
}

static void test_a_peak_over_its_size_fails_naming_the_slot(void) {
    assert(run_condition(ASSERTS "psx-largest: bytes_button 6064 6056\r\n" ROW DONE, 0, "") != 0);
    assert_has(LOG, "psx-check: largest-slots: bytes_button peak 6064 over its size 6056\n");
}

static void test_a_p8_fatal_fails_with_the_stopped_line(void) {
    assert(run_condition(ASSERTS "psx-state: stopped p8_fatal frame 3\r\n" BUTTON ROW DONE, 0, "") != 0);
    assert_has(LOG, "psx-check: largest-slots: psx-largest-slots stopped: stopped p8_fatal frame 3\n");
}

static void test_a_non_zero_exit_fails_with_the_boot_output(void) {
    assert(run_condition(ASSERTS BUTTON, 1, "psx-boot: psx-largest-slots.exe: program exited with code 4") != 0);
    assert_has(LOG, "psx-check: largest-slots: psx-boot: psx-largest-slots.exe: program exited with code 4\n");
}

static void test_asserts_off_fails(void) {
    assert(run_condition("psx-largest: asserts off\r\n" BUTTON ROW DONE, 0, "") != 0);
    assert_has(LOG, "psx-check: largest-slots: psx-largest: asserts off\n");
}

static void test_a_missing_done_line_fails(void) {
    assert(run_condition(ASSERTS BUTTON ROW, 0, "") != 0);
    assert_has(LOG, "psx-check: largest-slots: no done line\n");
}

int main(void) {
    test_both_slots_within_their_size_pass_and_their_lines_are_kept();
    test_a_peak_equal_to_its_size_passes();
    test_a_missing_row_line_fails_naming_it();
    test_a_peak_over_its_size_fails_naming_the_slot();
    test_a_p8_fatal_fails_with_the_stopped_line();
    test_a_non_zero_exit_fails_with_the_boot_output();
    test_asserts_off_fails();
    test_a_missing_done_line_fails();
    puts("test_psx_conditions: all tests passed");
    return 0;
}
