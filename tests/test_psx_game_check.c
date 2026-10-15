/* tools/psx_game_check.sh (the game EXE's six post-boot conditions) on crafted TTY files built
 * from tests/e2e/pad_title.json and its PC state, with the real tmp/psx_pad_log and tmp/replay:
 * no emulator. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR "tmp/test_psx_game_check_work"
#define OUT DIR "/out"
#define LOG DIR "/log.txt"
#define TTY DIR "/tty.txt"
#define SCRIPT DIR "/script.txt"
#define PC_STATE DIR "/pad_title_state.txt"
#define TOOLS " " SCRIPT " ./tmp/psx_pad_log ./tmp/replay "

/* pad_title.json: one press a frame from f10, X on "reasonable" at f18, 30 frames. */
#define PAD_LINES_TO_UP                                                                                     \
    "psx-game: pad 0 000\r\npsx-game: pad 10 002\r\npsx-game: pad 11 000\r\npsx-game: pad 12 002\r\n"      \
    "psx-game: pad 13 000\r\npsx-game: pad 14 008\r\npsx-game: pad 15 000\r\npsx-game: pad 16 001\r\n"
#define PAD_LINES PAD_LINES_TO_UP "psx-game: pad 17 000\r\npsx-game: pad 18 010\r\npsx-game: pad 19 000\r\n"
#define DONE "psx-replay: done game 30 vsyncs 1\r\n"

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

/* The PSX TTY: header with this asserts line ("" for none), pad lines, done line, then the PC state
 * behind "psx-state: ". */
static void write_tty_asserts(const char *asserts_line, const char *pad_lines, const char *done_line,
                              const char *state_path) {
    static char tty[1 << 16];
    snprintf(tty, sizeof tty, "PS-X Control PAD Driver\r\n%spsx-game: ready\r\n%s%s", asserts_line, pad_lines,
             done_line);
    write_file(TTY, tty);
    char command[512];
    snprintf(command, sizeof command, "sed 's/^/psx-state: /' %s >> " TTY, state_path);
    assert(system(command) == 0);
}

static void write_tty(const char *pad_lines, const char *done_line, const char *state_path) {
    write_tty_asserts("psx-game: asserts off\r\n", pad_lines, done_line, state_path);
}

static void assert_line(const char *line) {
    if (strstr(read_file(LOG), line) == NULL) {
        fprintf(stderr, "missing \"%s\" in:\n%s", line, read_file(LOG));
        assert(0);
    }
}

/* Runs the check with this boot status and difficulty; returns its exit status, output in LOG. */
static int run_check(int boot_status, const char *difficulty) {
    char command[512];
    snprintf(command, sizeof command,
             "sh tools/psx_game_check.sh " TTY " %d 'boot output'" TOOLS "%s " OUT " > " LOG " 2>&1", boot_status,
             difficulty);
    return system(command);
}

static int run_check_expecting(const char *asserts) {
    char command[512];
    snprintf(command, sizeof command,
             "PSX_GAME_ASSERTS=%s sh tools/psx_game_check.sh " TTY " 0 'boot output'" TOOLS "reasonable " OUT " > " LOG
             " 2>&1",
             asserts);
    return system(command);
}

static void assert_other_conditions_pass(void) {
    assert_line("psx-game-check: game-done: ok, 30 frames, 1 vsyncs");
    assert_line("psx-game-check: game-pad-log: ok, 30 frames");
    assert_line("psx-game-check: game-pad-states: ok, 11 states");
    assert_line("psx-game-check: game-state-game-lines: ok");
    assert_line("psx-game-check: game-title-left: ok, difficulty reasonable, clock year 3401 month 1");
}

static void setup(void) {
    assert(system("rm -rf " DIR " && mkdir -p " DIR) == 0);
    write_file(SCRIPT, "10 DOWN\n20 -\n30 DOWN\n40 -\n50 RIGHT\n60 -\n70 UP\n80 -\n90 CROSS\n100 -\n110 -\n");
    assert(system("STATE=" PC_STATE " ./tmp/replay tests/e2e/pad_title.json /dev/null " DIR
                  "/pc.dump.txt > /dev/null") == 0);
    write_tty(PAD_LINES, DONE, PC_STATE);
}

static void test_a_pad_title_run_passes_every_condition(void) {
    setup();
    assert(run_check(0, "reasonable") == 0);
    assert_line("psx-game-check: game-done: ok, 30 frames, 1 vsyncs");
    assert_line("psx-game-check: game-pad-log: ok, 30 frames");
    assert_line("psx-game-check: game-pad-states: ok, 11 states");
    assert_line("psx-game-check: game-state-game-lines: ok");
    assert_line("psx-game-check: game-title-left: ok, difficulty reasonable, clock year 3401 month 1");
    assert_line("psx-game-check: game-asserts: ok, off");
    assert(strstr(read_file(OUT "/game_events.json"), "\"input\": \"pad\"") != NULL);
    assert(strstr(read_file(OUT "/pc_state.txt"), "difficulty reasonable") != NULL);
}

static void test_a_failed_boot_fails_game_done(void) {
    setup();
    assert(run_check(1, "reasonable") != 0);
    assert_line("psx-game-check: game-done: boot output");
    assert_line("psx-game-check: game-title-left: ok");
}

static void test_no_done_line_fails_the_done_and_the_log(void) {
    setup();
    write_tty(PAD_LINES, "", PC_STATE);
    assert(run_check(0, "reasonable") != 0);
    assert_line("psx-game-check: game-done: no done line");
    assert_line("psx-game-check: game-pad-log: psx-pad-log: log: ");
    assert_line("psx-game-check: game-state-game-lines: skipped, game-pad-log failed");
}

static void test_a_stopped_state_fails_game_done(void) {
    setup();
    write_tty(PAD_LINES DONE "psx-state: stopped p8_fatal frame 12\r\n", "", PC_STATE);
    assert(run_check(0, "reasonable") != 0);
    assert_line("psx-game-check: game-done: the game stopped: stopped p8_fatal frame 12");
}

static void test_a_dropped_pad_line_names_the_first_differing_state(void) {
    setup();
    write_tty("psx-game: pad 0 000\r\npsx-game: pad 10 002\r\npsx-game: pad 11 000\r\npsx-game: pad 14 008\r\n"
              "psx-game: pad 15 000\r\npsx-game: pad 16 001\r\npsx-game: pad 17 000\r\npsx-game: pad 18 010\r\n"
              "psx-game: pad 19 000\r\n",
              DONE, PC_STATE);
    assert(run_check(0, "reasonable") != 0);
    assert_line("psx-game-check: game-pad-log: ok, 30 frames");
    assert_line("psx-game-check: game-pad-states: psx-pad-log: states: first differing state, 3: log 008 script 002");
}

static void test_an_altered_state_line_is_named(void) {
    setup();
    assert(system("sed 's/^resources credits 100$/resources credits 101/' " PC_STATE
                  " > " DIR "/altered.txt") == 0);
    write_tty(PAD_LINES, DONE, DIR "/altered.txt");
    assert(run_check(0, "reasonable") != 0);
    assert_line("psx-game-check: game-state-game-lines: psx-state-compare: first differing game line");
    assert_line("credits 101");
}

static void test_a_log_cut_before_cross_never_leaves_the_title(void) {
    setup();
    write_tty(PAD_LINES_TO_UP "psx-game: pad 17 000\r\n", "psx-replay: done game 18 vsyncs 1\r\n", PC_STATE);
    assert(system("./tmp/psx_pad_log " TTY " " SCRIPT " " DIR "/cut.json > /dev/null; STATE=" DIR
                  "/cut_state.txt ./tmp/replay " DIR "/cut.json /dev/null " DIR "/cut.dump.txt > /dev/null") == 0);
    write_tty(PAD_LINES_TO_UP "psx-game: pad 17 000\r\n", "psx-replay: done game 18 vsyncs 1\r\n", DIR "/cut_state.txt");
    assert(run_check(0, "reasonable") != 0);
    assert_line("psx-game-check: game-state-game-lines: ok");
    assert_line("psx-game-check: game-title-left: expected difficulty reasonable: difficulty -, clock year - month -");
}

static void test_another_expected_difficulty_fails_title_left(void) {
    setup();
    assert(run_check(0, "tough") != 0);
    assert_line("psx-game-check: game-title-left: expected difficulty tough: difficulty reasonable, clock year 3401 month 1");
    assert_line("psx-game-check: game-state-game-lines: ok");
}

static void test_an_asserts_on_game_fails_game_asserts(void) {
    setup();
    write_tty_asserts("psx-game: asserts on\r\n", PAD_LINES, DONE, PC_STATE);
    assert(run_check(0, "reasonable") != 0);
    assert_line("psx-game-check: game-asserts: psx-game: asserts on");
    assert_other_conditions_pass();
}

static void test_a_missing_asserts_line_fails_game_asserts(void) {
    setup();
    write_tty_asserts("", PAD_LINES, DONE, PC_STATE);
    assert(run_check(0, "reasonable") != 0);
    assert_line("psx-game-check: game-asserts: psx-game: asserts (no line)");
    assert_other_conditions_pass();
}

static void test_an_asserts_on_expectation_wants_asserts_on(void) {
    setup();
    write_tty_asserts("psx-game: asserts on\r\n", PAD_LINES, DONE, PC_STATE);
    assert(run_check_expecting("on") == 0);
    assert_line("psx-game-check: game-asserts: ok, on");
    assert_other_conditions_pass();
    write_tty(PAD_LINES, DONE, PC_STATE);
    assert(run_check_expecting("on") != 0);
    assert_line("psx-game-check: game-asserts: psx-game: asserts off");
    assert_other_conditions_pass();
}

static void test_an_unknown_expectation_is_refused(void) {
    setup();
    assert(run_check_expecting("maybe") != 0);
    assert_line("PSX_GAME_ASSERTS must be on or off, not 'maybe'");
}

int main(void) {
    test_a_pad_title_run_passes_every_condition();
    test_a_failed_boot_fails_game_done();
    test_no_done_line_fails_the_done_and_the_log();
    test_a_stopped_state_fails_game_done();
    test_a_dropped_pad_line_names_the_first_differing_state();
    test_an_altered_state_line_is_named();
    test_a_log_cut_before_cross_never_leaves_the_title();
    test_another_expected_difficulty_fails_title_left();
    test_an_asserts_on_game_fails_game_asserts();
    test_a_missing_asserts_line_fails_game_asserts();
    test_an_asserts_on_expectation_wants_asserts_on();
    test_an_unknown_expectation_is_refused();
    printf("test_psx_game_check: ok\n");
    return 0;
}
