#include "devtools/pad_log.h"

#include "p8/input.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { TEXT_CAP = 8192, FILE_CAP = 16384 };

static size_t read_file(const char *path, char *buffer, size_t size) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    size_t length = fread(buffer, 1, size - 1, file);
    assert(length < size - 1);
    buffer[length] = '\0';
    fclose(file);
    return length;
}

static void append(char *text, const char *line) {
    assert(strlen(text) + strlen(line) < TEXT_CAP);
    strcat(text, line);
}

static void test_a_log_of_pad_title_rebuilds_its_recording_byte_for_byte(void) {
    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, "tests/e2e/pad_title.json"));
    static char text[TEXT_CAP];
    text[0] = '\0';
    char line[64];
    for (int frame = 0; frame < input.frame_count; frame++) {
        int pad = input.frames[frame].pad_buttons;
        if (frame == 0 || pad != input.frames[frame - 1].pad_buttons) {
            snprintf(line, sizeof line, "psx-game: pad %d %03x\r\n", frame, pad);
            append(text, line);
        }
    }
    snprintf(line, sizeof line, "psx-replay: done game %d vsyncs 1\r\n", input.frame_count);
    append(text, line);

    pad_log_t log;
    char error[PAD_LOG_ERROR_CAP];
    assert(pad_log_parse(text, &log, error));
    assert(log.frame_count == input.frame_count);
    assert(pad_log_save(&log, "tmp/test_pad_log_pad_title.json"));
    pad_log_free(&log);
    p8_input_free(&input);

    static char expected[FILE_CAP];
    static char written[FILE_CAP];
    size_t expected_length = read_file("tests/e2e/pad_title.json", expected, sizeof expected);
    size_t written_length = read_file("tmp/test_pad_log_pad_title.json", written, sizeof written);
    assert(written_length == expected_length);
    assert(memcmp(written, expected, expected_length) == 0);
}

static void test_values_hold_between_lines_and_foreign_lines_are_ignored(void) {
    const char *text = "psx-game: ready\n"
                       "psx-game: pad 0 000\n"
                       "noise psx-game: pad 1 7ff\n"
                       "psx-game: pad 2 010\n"
                       "psx-replay: frame 3\n"
                       "psx-game: pad 5 002\n"
                       "psx-game: pad 6 002\n"
                       "psx-replay: done game 8 vsyncs 9\n"
                       "psx-state: clock year - month -\n";
    pad_log_t log;
    char error[PAD_LOG_ERROR_CAP];
    assert(pad_log_parse(text, &log, error));
    static const int expected[8] = { 0, 0, 0x10, 0x10, 0x10, 2, 2, 2 };
    assert(log.frame_count == 8);
    for (int frame = 0; frame < 8; frame++) assert(log.pad_buttons[frame] == expected[frame]);
    assert(log.state_count == 3);
    assert(log.states[0] == 0 && log.states[1] == 0x10 && log.states[2] == 2);
    pad_log_free(&log);
}

static void assert_refused(const char *text, const char *expected_error) {
    pad_log_t log;
    char error[PAD_LOG_ERROR_CAP];
    int parsed = pad_log_parse(text, &log, error);
    if (parsed || strcmp(error, expected_error) != 0) {
        fprintf(stderr, "expected refusal \"%s\", got %s \"%s\"\n", expected_error, parsed ? "a log" : "",
                parsed ? "" : error);
    }
    assert(!parsed);
    assert(strcmp(error, expected_error) == 0);
}

static void test_a_broken_log_is_refused_naming_its_line(void) {
    assert_refused("psx-game: pad 1 000\npsx-replay: done game 4 vsyncs 1\n",
                   "line 1: first pad line at frame 1, not 0");
    assert_refused("psx-replay: done game 4 vsyncs 1\n", "no psx-game: pad line");
    assert_refused("psx-game: pad 0 000\npsx-game: pad 2 001\npsx-game: pad 2 000\npsx-replay: done game 4 vsyncs 1\n",
                   "line 3: frame 2 does not increase past 2");
    assert_refused("psx-game: pad 0 00g\npsx-replay: done game 4 vsyncs 1\n", "line 1: malformed pad line");
    assert_refused("psx-game: pad 0\npsx-replay: done game 4 vsyncs 1\n", "line 1: malformed pad line");
    assert_refused("psx-game: pad 0 1000\npsx-replay: done game 4 vsyncs 1\n",
                   "line 1: pad 1000 has bits outside the twelve pad buttons");
    assert_refused("psx-game: pad 0 000\n", "no psx-replay: done line");
    assert_refused("psx-game: pad 0 000\npsx-replay: done game 4 vsyncs 1\npsx-replay: done game 4 vsyncs 1\n",
                   "line 3: second done line (the first is line 2)");
    assert_refused("psx-game: pad 0 000\npsx-replay: done game four vsyncs 1\n", "line 2: malformed done line");
    assert_refused("psx-game: pad 0 000\npsx-game: pad 4 001\npsx-replay: done game 4 vsyncs 1\n",
                   "line 2: pad line at frame 4, at or after the done line's 4 frames");
}

static void test_the_game_script_walks_the_title_screen_states(void) {
    static char text[TEXT_CAP];
    read_file("psx/game_title_script.txt", text, sizeof text);
    pad_script_t script;
    char error[PAD_LOG_ERROR_CAP];
    assert(pad_log_parse_script(text, &script, error));
    static const int expected[] = { 0x000, 0x002, 0x000, 0x002, 0x000, 0x008, 0x000, 0x001, 0x000, 0x010, 0x000 };
    assert(script.count == (int)(sizeof expected / sizeof expected[0]));
    for (int i = 0; i < script.count; i++) assert(script.states[i] == expected[i]);
}

static void test_a_step_holding_only_l2_adds_no_state(void) {
    pad_script_t script;
    char error[PAD_LOG_ERROR_CAP];
    assert(pad_log_parse_script("# l2 is no pad button\n\n10 L2\n20 CROSS L2\n30 -\n", &script, error));
    assert(script.count == 3);
    assert(script.states[0] == 0 && script.states[1] == P8_PAD_BUTTON_5 && script.states[2] == 0);
}

static void assert_script_refused(const char *text, const char *expected_error) {
    pad_script_t script;
    char error[PAD_LOG_ERROR_CAP];
    int parsed = pad_log_parse_script(text, &script, error);
    if (parsed || strcmp(error, expected_error) != 0) {
        fprintf(stderr, "expected refusal \"%s\", got \"%s\"\n", expected_error, parsed ? "a script" : error);
    }
    assert(!parsed);
}

static void test_a_broken_script_is_refused_naming_its_line(void) {
    assert_script_refused("10 UP\nten -\n", "line 2: vsync offset is not an integer: ten");
    assert_script_refused("10 UP\n10 -\n", "line 2: vsync offset 10 does not increase");
    assert_script_refused("10 UP\n20\n", "line 2: no button set (use - for none)");
    assert_script_refused("10 JUMP\n20 -\n", "line 1: unknown button JUMP");
    assert_script_refused("10 up\n20 -\n", "line 1: unknown button up");
    assert_script_refused("10 UP\n20 DOWN\n", "last step must be - (no button held)");
    assert_script_refused("# nothing\n", "no step");
}

static int first_difference(const char *log_text, const char *script_text) {
    pad_log_t log;
    pad_script_t script;
    char error[PAD_LOG_ERROR_CAP];
    assert(pad_log_parse(log_text, &log, error));
    assert(pad_log_parse_script(script_text, &script, error));
    int index = pad_log_first_difference(&log, &script);
    pad_log_free(&log);
    return index;
}

static void test_log_states_are_compared_with_the_script_states(void) {
    const char *script = "10 DOWN\n20 -\n30 CROSS\n40 -\n";
    assert(first_difference("psx-game: pad 0 000\npsx-game: pad 3 002\npsx-game: pad 5 000\n"
                            "psx-game: pad 9 010\npsx-game: pad 12 000\npsx-replay: done game 20 vsyncs 40\n",
                            script)
           == -1);
    assert(first_difference("psx-game: pad 0 000\npsx-game: pad 3 002\npsx-game: pad 5 000\n"
                            "psx-game: pad 9 020\npsx-game: pad 12 000\npsx-replay: done game 20 vsyncs 40\n",
                            script)
           == 3);
    assert(first_difference("psx-game: pad 0 000\npsx-game: pad 3 002\npsx-game: pad 5 000\n"
                            "psx-replay: done game 20 vsyncs 40\n",
                            script)
           == 3);
    assert(first_difference("psx-game: pad 0 000\npsx-game: pad 3 002\npsx-game: pad 5 000\n"
                            "psx-game: pad 9 010\npsx-game: pad 12 000\npsx-game: pad 14 001\n"
                            "psx-replay: done game 20 vsyncs 40\n",
                            script)
           == 5);
}

int main(void) {
    test_a_log_of_pad_title_rebuilds_its_recording_byte_for_byte();
    test_values_hold_between_lines_and_foreign_lines_are_ignored();
    test_a_broken_log_is_refused_naming_its_line();
    test_the_game_script_walks_the_title_screen_states();
    test_a_step_holding_only_l2_adds_no_state();
    test_a_broken_script_is_refused_naming_its_line();
    test_log_states_are_compared_with_the_script_states();
    printf("test_pad_log: ok\n");
    return 0;
}
