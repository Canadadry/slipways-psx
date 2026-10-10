#include "platform/keymap.h"

#include "p8/input.h"
#include "platform/keymap_names.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void read_file(const char *path, char *buffer, size_t size) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    size_t length = fread(buffer, 1, size - 1, file);
    assert(length < size - 1);
    buffer[length] = '\0';
    fclose(file);
}

static void parse_default_file(keymap_t *out) {
    char text[8192];
    read_file("keymap.default.cfg", text, sizeof text);
    keymap_error_t error;
    int parsed = keymap_parse(text, out, &error);
    if (!parsed) {
        fprintf(stderr, "keymap.default.cfg:%d: %s\n", error.line, error.message);
        assert(0);
    }
}

static void test_the_compiled_default_keymap_is_the_parse_of_the_default_file(void) {
    keymap_t parsed;
    parse_default_file(&parsed);
    assert(memcmp(&keymap_compiled, &parsed, sizeof parsed) == 0);
}

static void test_defaults_bind_button_5_to_x_and_right_face_down(void) {
    keymap_t keymap;
    parse_default_file(&keymap);
    const keymap_button_t *button_5 = &keymap.buttons[4];
    assert(button_5->count == 2);
    assert(button_5->bindings[0].device == KEYMAP_KEYBOARD);
    assert(button_5->bindings[0].code == 88);
    assert(button_5->bindings[1].device == KEYMAP_GAMEPAD);
    assert(button_5->bindings[1].code == 7);
}

static keymap_binding_t binding_named(const char *name) {
    keymap_binding_t binding;
    int found = keymap_find_binding(name, &binding);
    assert(found);
    return binding;
}

static void assert_binding(const keymap_binding_t *binding, const char *name) {
    keymap_binding_t expected = binding_named(name);
    assert(binding->device == expected.device);
    assert(binding->code == expected.code);
}

static void test_defaults_hold_the_plan_table_in_pad_bit_order(void) {
    static const char *const expected[P8_PAD_BUTTON_COUNT][2] = {
        { "UP", "LEFT_FACE_UP" }, { "DOWN", "LEFT_FACE_DOWN" },
        { "LEFT", "LEFT_FACE_LEFT" }, { "RIGHT", "LEFT_FACE_RIGHT" },
        { "X", "RIGHT_FACE_DOWN" }, { "C", "RIGHT_FACE_RIGHT" },
        { "Z", "RIGHT_FACE_LEFT" }, { "V", "RIGHT_FACE_UP" },
        { "A", "LEFT_TRIGGER_1" }, { "S", "RIGHT_TRIGGER_1" },
        { "ENTER", "MIDDLE_RIGHT" }, { "RIGHT_SHIFT", "MIDDLE_LEFT" },
    };
    keymap_t keymap;
    parse_default_file(&keymap);
    for (int i = 0; i < P8_PAD_BUTTON_COUNT; i++) {
        assert(keymap.buttons[i].count == 2);
        assert(keymap.buttons[i].bindings[0].device == KEYMAP_KEYBOARD);
        assert(keymap.buttons[i].bindings[1].device == KEYMAP_GAMEPAD);
        assert_binding(&keymap.buttons[i].bindings[0], expected[i][0]);
        assert_binding(&keymap.buttons[i].bindings[1], expected[i][1]);
    }
}

#define PREAMBLE "# a comment\n\n1 = UP, LEFT_FACE_UP\n"

static void assert_parse_error(const char *text, int line, const char *token) {
    keymap_t keymap;
    keymap_error_t error;
    assert(!keymap_parse(text, &keymap, &error));
    if (error.line != line || strstr(error.message, token) == NULL) {
        fprintf(stderr, "expected line %d naming '%s', got line %d: %s\n", line, token, error.line, error.message);
        assert(0);
    }
}

static void test_line_without_equal_is_reported_with_its_number(void) {
    assert_parse_error(PREAMBLE "2 DOWN\n", 4, "2 DOWN");
}

static void test_bad_button_is_reported_with_its_number(void) {
    static const char *const tokens[] = { "13", "0", "02", "+2", "2x", "2 x", "down" };
    for (int i = 0; i < 7; i++) {
        char text[256];
        char expected[64];
        snprintf(text, sizeof text, PREAMBLE "%s = DOWN\n", tokens[i]);
        snprintf(expected, sizeof expected, "bad button \"%s\": expected a number from 1 to 12", tokens[i]);
        assert_parse_error(text, 4, expected);
    }
}

static void test_empty_item_is_reported_with_its_number(void) {
    assert_parse_error(PREAMBLE "2 =\n", 4, "button 2");
    assert_parse_error(PREAMBLE "2 = DOWN,,LEFT_FACE_DOWN\n", 4, "button 2");
    assert_parse_error(PREAMBLE "2 = DOWN, \n", 4, "button 2");
}

static void test_unknown_key_name_is_reported_with_its_number(void) {
    assert_parse_error(PREAMBLE "2 = DOWN, LEFT_FACE_DWN\n", 4, "LEFT_FACE_DWN");
    assert_parse_error(PREAMBLE "2 = down\n", 4, "down");
    assert_parse_error(PREAMBLE "2 = KEY_DOWN\n", 4, "KEY_DOWN");
    assert_parse_error(PREAMBLE "2 = NULL\n", 4, "NULL");
    assert_parse_error(PREAMBLE "2 = UNKNOWN\n", 4, "UNKNOWN");
}

static void test_button_given_twice_is_reported_at_the_second_line(void) {
    assert_parse_error(PREAMBLE "# again\n1 = W\n", 5, "button 1 already given");
}

static void test_binding_used_twice_is_reported_at_the_second_use(void) {
    assert_parse_error(PREAMBLE "2 = DOWN, UP\n", 4, "UP");
    assert_parse_error(PREAMBLE "2 = LEFT_FACE_UP\n", 4, "LEFT_FACE_UP");
    assert_parse_error(PREAMBLE "2 = DOWN, W, DOWN\n", 4, "DOWN");
}

static void test_reserved_key_is_reported_with_its_number(void) {
    static const char *const texts[] = {
        PREAMBLE "2 = DOWN, SPACE\n", PREAMBLE "2 = PAGE_UP\n", PREAMBLE "2 = PAGE_DOWN\n",
        PREAMBLE "2 = END\n", PREAMBLE "2 = ESCAPE\n",
    };
    static const char *const names[] = { "SPACE", "PAGE_UP", "PAGE_DOWN", "END", "ESCAPE" };
    for (int i = 0; i < 5; i++) {
        assert_parse_error(texts[i], 4, names[i]);
    }
}

static void test_more_than_four_bindings_is_reported_with_its_number(void) {
    keymap_t keymap;
    keymap_error_t error;
    assert(keymap_parse(PREAMBLE "2 = DOWN, S, W, LEFT_FACE_DOWN\n", &keymap, &error) || error.line != 4);
    assert_parse_error(PREAMBLE "2 = DOWN, S, W, LEFT_FACE_DOWN, J\n", 4, "J");
}

static void test_line_over_127_characters_is_reported_with_its_number(void) {
    char text[512];
    char line[KEYMAP_MAX_LINE_LENGTH + 2];
    memset(line, '#', sizeof line - 1);
    line[sizeof line - 1] = '\0';
    snprintf(text, sizeof text, PREAMBLE "%s\n", line);
    assert_parse_error(text, 4, "127");

    line[sizeof line - 2] = '\0';
    keymap_t keymap;
    keymap_error_t error;
    snprintf(text, sizeof text, PREAMBLE "%s\n", line);
    assert(keymap_parse(text, &keymap, &error) || error.line != 4);
}

static void test_button_without_a_line_is_reported_with_line_zero(void) {
    char file_text[8192];
    read_file("keymap.default.cfg", file_text, sizeof file_text);
    char text[8192];
    snprintf(text, sizeof text, "%s", file_text);
    char *button_8 = strstr(text, "\n8 = ");
    assert(button_8 != NULL);
    button_8[1] = '#';
    assert_parse_error(text, 0, "no line for button 8");

    snprintf(text, sizeof text, "%s", file_text);
    char *button_12 = strstr(text, "\n12 = ");
    assert(button_12 != NULL);
    button_12[1] = '#';
    assert_parse_error(text, 0, "no line for button 12");
}

static const char plain_text[] =
    "1 = UP, LEFT_FACE_UP\n"
    "2 = DOWN, LEFT_FACE_DOWN\n"
    "3 = LEFT, LEFT_FACE_LEFT\n"
    "4 = RIGHT, LEFT_FACE_RIGHT\n"
    "5 = X, RIGHT_FACE_DOWN\n"
    "6 = C, RIGHT_FACE_RIGHT\n"
    "7 = Z, RIGHT_FACE_LEFT\n"
    "8 = V, RIGHT_FACE_UP\n"
    "9 = A, LEFT_TRIGGER_1\n"
    "10 = S, RIGHT_TRIGGER_1\n"
    "11 = ENTER, MIDDLE_RIGHT\n"
    "12 = RIGHT_SHIFT, MIDDLE_LEFT\n";

static void assert_parses_as_the_plain_form(const char *text) {
    keymap_t keymap;
    keymap_t plain;
    keymap_error_t error;
    int parsed = keymap_parse(text, &keymap, &error);
    if (!parsed) {
        fprintf(stderr, "line %d: %s\n", error.line, error.message);
        assert(0);
    }
    assert(keymap_parse(plain_text, &plain, &error));
    assert(memcmp(&keymap, &plain, sizeof keymap) == 0);
}

static void test_blanks_comments_crlf_and_last_line_without_newline_parse(void) {
    assert_parses_as_the_plain_form(
        "\t 1\t=\tUP ,\tLEFT_FACE_UP \t# the D-pad\n"
        "2=DOWN,LEFT_FACE_DOWN\n"
        "   \t\n"
        "  # only a comment\n"
        "3   =   LEFT   ,   LEFT_FACE_LEFT\n"
        "4 = RIGHT, LEFT_FACE_RIGHT\r\n"
        "5 = X, RIGHT_FACE_DOWN # bottom\r\n"
        "\r\n"
        "6 = C, RIGHT_FACE_RIGHT\n"
        "7 = Z, RIGHT_FACE_LEFT\n"
        "8 = V, RIGHT_FACE_UP\n"
        "9 = A, LEFT_TRIGGER_1\n"
        "10 = S, RIGHT_TRIGGER_1\n"
        "11 = ENTER, MIDDLE_RIGHT\n"
        "12 = RIGHT_SHIFT, MIDDLE_LEFT");
}

#define NAME_STRING(name, code) #name,

static const char *const all_names[] = {
    KEYMAP_KEYBOARD_NAMES(NAME_STRING)
    KEYMAP_GAMEPAD_NAMES(NAME_STRING)
};

static void test_name_table_is_unique_and_holds_raylib_codes(void) {
    int count = (int)(sizeof all_names / sizeof all_names[0]);
    assert(count == 109 + 17);
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            assert(strcmp(all_names[i], all_names[j]) != 0);
        }
    }
    keymap_binding_t binding;
    assert(!keymap_find_binding("NULL", &binding));
    assert(!keymap_find_binding("UNKNOWN", &binding));
    binding = binding_named("A");
    assert(binding.device == KEYMAP_KEYBOARD && binding.code == 65);
    binding = binding_named("ENTER");
    assert(binding.device == KEYMAP_KEYBOARD && binding.code == 257);
    binding = binding_named("UP");
    assert(binding.device == KEYMAP_KEYBOARD && binding.code == 265);
    binding = binding_named("RIGHT_FACE_DOWN");
    assert(binding.device == KEYMAP_GAMEPAD && binding.code == 7);
}

typedef struct {
    keymap_binding_t held[32];
    int count;
} held_set_t;

static int fake_is_down(keymap_device_t device, int code, void *context) {
    const held_set_t *held = context;
    for (int i = 0; i < held->count; i++) {
        if (held->held[i].device == device && held->held[i].code == code) {
            return 1;
        }
    }
    return 0;
}

static void hold(held_set_t *held, const char *name) {
    held->held[held->count++] = binding_named(name);
}

static void test_holding_button_n_sets_bit_n_minus_1(void) {
    assert(P8_PAD_BUTTON_12 == 1 << (P8_PAD_BUTTON_COUNT - 1));
    keymap_t keymap;
    parse_default_file(&keymap);
    for (int number = 1; number <= P8_PAD_BUTTON_COUNT; number++) {
        held_set_t held = { .count = 1 };
        held.held[0] = keymap.buttons[number - 1].bindings[0];
        assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == 1 << (number - 1));
    }
}

static void test_lines_in_reverse_order_parse_as_the_plain_form(void) {
    char reversed[sizeof plain_text];
    size_t length = strlen(plain_text);
    size_t written = 0;
    size_t line_end = length;
    while (line_end > 0) {
        size_t line_start = line_end - 1;
        while (line_start > 0 && plain_text[line_start - 1] != '\n') {
            line_start--;
        }
        memcpy(reversed + written, plain_text + line_start, line_end - line_start);
        written += line_end - line_start;
        line_end = line_start;
    }
    reversed[written] = '\0';
    assert(written == length && strncmp(reversed, "12 = ", 5) == 0);
    assert_parses_as_the_plain_form(reversed);
}

static void test_pad_buttons_or_the_held_bindings(void) {
    keymap_t keymap;
    parse_default_file(&keymap);
    held_set_t held = { .count = 0 };
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == 0);

    hold(&held, "X");
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == P8_PAD_BUTTON_5);

    held.count = 0;
    hold(&held, "RIGHT_FACE_DOWN");
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == P8_PAD_BUTTON_5);

    hold(&held, "X");
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == P8_PAD_BUTTON_5);

    held.count = 0;
    hold(&held, "RIGHT_SHIFT");
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == P8_PAD_BUTTON_12);

    held.count = 0;
    hold(&held, "MIDDLE_LEFT");
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == P8_PAD_BUTTON_12);

    held.count = 0;
    for (int i = 0; i < P8_PAD_BUTTON_COUNT; i++) {
        for (int j = 0; j < keymap.buttons[i].count; j++) {
            held.held[held.count++] = keymap.buttons[i].bindings[j];
        }
    }
    assert(held.count == 24);
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == (1 << P8_PAD_BUTTON_COUNT) - 1);
}

int main(void) {
    test_the_compiled_default_keymap_is_the_parse_of_the_default_file();
    test_defaults_bind_button_5_to_x_and_right_face_down();
    test_defaults_hold_the_plan_table_in_pad_bit_order();
    test_line_without_equal_is_reported_with_its_number();
    test_bad_button_is_reported_with_its_number();
    test_empty_item_is_reported_with_its_number();
    test_unknown_key_name_is_reported_with_its_number();
    test_button_given_twice_is_reported_at_the_second_line();
    test_binding_used_twice_is_reported_at_the_second_use();
    test_reserved_key_is_reported_with_its_number();
    test_more_than_four_bindings_is_reported_with_its_number();
    test_line_over_127_characters_is_reported_with_its_number();
    test_button_without_a_line_is_reported_with_line_zero();
    test_blanks_comments_crlf_and_last_line_without_newline_parse();
    test_name_table_is_unique_and_holds_raylib_codes();
    test_holding_button_n_sets_bit_n_minus_1();
    test_lines_in_reverse_order_parse_as_the_plain_form();
    test_pad_buttons_or_the_held_bindings();
    printf("test_keymap: ok\n");
    return 0;
}
