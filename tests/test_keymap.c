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

static void test_defaults_bind_cross_to_x_and_right_face_down(void) {
    keymap_t keymap;
    keymap_defaults(&keymap);
    const keymap_button_t *cross = &keymap.buttons[4];
    assert(cross->count == 2);
    assert(cross->bindings[0].device == KEYMAP_KEYBOARD);
    assert(cross->bindings[0].code == 88);
    assert(cross->bindings[1].device == KEYMAP_GAMEPAD);
    assert(cross->bindings[1].code == 7);
}

static void test_versioned_file_is_the_default_text_and_loads_as_the_defaults(void) {
    char text[8192];
    read_file("keymap.cfg", text, sizeof text);
    assert(strcmp(text, keymap_default_text()) == 0);

    keymap_t loaded;
    keymap_t defaults;
    keymap_error_t error;
    assert(keymap_load("keymap.cfg", &loaded, &error) == KEYMAP_LOADED);
    keymap_defaults(&defaults);
    assert(memcmp(&loaded, &defaults, sizeof loaded) == 0);
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
    static const char *const expected[KEYMAP_BUTTON_COUNT][2] = {
        { "UP", "LEFT_FACE_UP" }, { "DOWN", "LEFT_FACE_DOWN" },
        { "LEFT", "LEFT_FACE_LEFT" }, { "RIGHT", "LEFT_FACE_RIGHT" },
        { "X", "RIGHT_FACE_DOWN" }, { "C", "RIGHT_FACE_RIGHT" },
        { "Z", "RIGHT_FACE_LEFT" }, { "V", "RIGHT_FACE_UP" },
        { "A", "LEFT_TRIGGER_1" }, { "S", "RIGHT_TRIGGER_1" },
        { "ENTER", "MIDDLE_RIGHT" }, { "RIGHT_SHIFT", "MIDDLE_LEFT" },
    };
    keymap_t keymap;
    keymap_defaults(&keymap);
    for (int i = 0; i < KEYMAP_BUTTON_COUNT; i++) {
        assert(keymap.buttons[i].count == 2);
        assert(keymap.buttons[i].bindings[0].device == KEYMAP_KEYBOARD);
        assert(keymap.buttons[i].bindings[1].device == KEYMAP_GAMEPAD);
        assert_binding(&keymap.buttons[i].bindings[0], expected[i][0]);
        assert_binding(&keymap.buttons[i].bindings[1], expected[i][1]);
    }
}

static void test_missing_file_loads_the_defaults(void) {
    keymap_t loaded;
    keymap_t defaults;
    keymap_error_t error;
    remove("tmp/test_keymap_missing.cfg");
    assert(keymap_load("tmp/test_keymap_missing.cfg", &loaded, &error) == KEYMAP_MISSING);
    keymap_defaults(&defaults);
    assert(memcmp(&loaded, &defaults, sizeof loaded) == 0);
    const keymap_button_t *start = &loaded.buttons[10];
    assert(start->bindings[0].device == KEYMAP_KEYBOARD && start->bindings[0].code == 257);
    assert(start->bindings[1].device == KEYMAP_GAMEPAD && start->bindings[1].code == 15);
}

static void test_unreadable_path_is_invalid_not_the_defaults(void) {
    keymap_t keymap;
    keymap_error_t error;
    assert(keymap_load("tmp", &keymap, &error) == KEYMAP_INVALID);
    assert(error.line == 0);
    assert(strstr(error.message, "read") != NULL);
}

static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "wb");
    assert(file != NULL);
    fputs(text, file);
    fclose(file);
}

static void test_file_over_four_kib_is_invalid(void) {
    char text[4200];
    memset(text, '#', sizeof text - 1);
    text[sizeof text - 1] = '\0';
    write_file("tmp/test_keymap_large.cfg", text);
    keymap_t keymap;
    keymap_error_t error;
    assert(keymap_load("tmp/test_keymap_large.cfg", &keymap, &error) == KEYMAP_INVALID);
    assert(error.line == 0);
    assert(strstr(error.message, "4096") != NULL);
}

#define PREAMBLE "# a comment\n\nup = UP, LEFT_FACE_UP\n"

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
    assert_parse_error(PREAMBLE "down DOWN\n", 4, "down DOWN");
}

static void test_unknown_psx_button_is_reported_with_its_number(void) {
    assert_parse_error(PREAMBLE "l2 = DOWN\n", 4, "l2");
    assert_parse_error(PREAMBLE "Down = DOWN\n", 4, "Down");
}

static void test_empty_item_is_reported_with_its_number(void) {
    assert_parse_error(PREAMBLE "down =\n", 4, "down");
    assert_parse_error(PREAMBLE "down = DOWN,,LEFT_FACE_DOWN\n", 4, "down");
    assert_parse_error(PREAMBLE "down = DOWN, \n", 4, "down");
}

static void test_unknown_key_name_is_reported_with_its_number(void) {
    assert_parse_error(PREAMBLE "down = DOWN, LEFT_FACE_DWN\n", 4, "LEFT_FACE_DWN");
    assert_parse_error(PREAMBLE "down = down\n", 4, "down");
    assert_parse_error(PREAMBLE "down = KEY_DOWN\n", 4, "KEY_DOWN");
    assert_parse_error(PREAMBLE "down = NULL\n", 4, "NULL");
    assert_parse_error(PREAMBLE "down = UNKNOWN\n", 4, "UNKNOWN");
}

static void test_psx_button_given_twice_is_reported_at_the_second_line(void) {
    assert_parse_error(PREAMBLE "# again\nup = W\n", 5, "up");
}

static void test_binding_used_twice_is_reported_at_the_second_use(void) {
    assert_parse_error(PREAMBLE "down = DOWN, UP\n", 4, "UP");
    assert_parse_error(PREAMBLE "down = LEFT_FACE_UP\n", 4, "LEFT_FACE_UP");
    assert_parse_error(PREAMBLE "down = DOWN, W, DOWN\n", 4, "DOWN");
}

static void test_reserved_key_is_reported_with_its_number(void) {
    static const char *const texts[] = {
        PREAMBLE "down = DOWN, SPACE\n", PREAMBLE "down = PAGE_UP\n", PREAMBLE "down = PAGE_DOWN\n",
        PREAMBLE "down = END\n", PREAMBLE "down = ESCAPE\n",
    };
    static const char *const names[] = { "SPACE", "PAGE_UP", "PAGE_DOWN", "END", "ESCAPE" };
    for (int i = 0; i < 5; i++) {
        assert_parse_error(texts[i], 4, names[i]);
    }
}

static void test_more_than_four_bindings_is_reported_with_its_number(void) {
    keymap_t keymap;
    keymap_error_t error;
    assert(keymap_parse(PREAMBLE "down = DOWN, S, W, LEFT_FACE_DOWN\n", &keymap, &error) || error.line != 4);
    assert_parse_error(PREAMBLE "down = DOWN, S, W, LEFT_FACE_DOWN, J\n", 4, "J");
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

static void test_psx_button_without_a_line_is_reported_with_line_zero(void) {
    char text[4096];
    snprintf(text, sizeof text, "%s", keymap_default_text());
    char *triangle = strstr(text, "triangle = ");
    assert(triangle != NULL);
    triangle[0] = '#';
    assert_parse_error(text, 0, "triangle");

    snprintf(text, sizeof text, "%s", keymap_default_text());
    char *select = strstr(text, "select = ");
    assert(select != NULL);
    select[0] = '#';
    assert_parse_error(text, 0, "select");
}

static void test_invalid_file_reports_the_line_through_load(void) {
    write_file("tmp/test_keymap_invalid.cfg", PREAMBLE "down = DOWN\nleft = LEFT_KEY\n");
    keymap_t keymap;
    keymap_error_t error;
    assert(keymap_load("tmp/test_keymap_invalid.cfg", &keymap, &error) == KEYMAP_INVALID);
    assert(error.line == 5);
    assert(strstr(error.message, "LEFT_KEY") != NULL);
}

static const char plain_text[] =
    "up = UP, LEFT_FACE_UP\n"
    "down = DOWN, LEFT_FACE_DOWN\n"
    "left = LEFT, LEFT_FACE_LEFT\n"
    "right = RIGHT, LEFT_FACE_RIGHT\n"
    "cross = X, RIGHT_FACE_DOWN\n"
    "circle = C, RIGHT_FACE_RIGHT\n"
    "square = Z, RIGHT_FACE_LEFT\n"
    "triangle = V, RIGHT_FACE_UP\n"
    "l1 = A, LEFT_TRIGGER_1\n"
    "r1 = S, RIGHT_TRIGGER_1\n"
    "start = ENTER, MIDDLE_RIGHT\n"
    "select = RIGHT_SHIFT, MIDDLE_LEFT\n";

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
        "\t up\t=\tUP ,\tLEFT_FACE_UP \t# the D-pad\n"
        "down=DOWN,LEFT_FACE_DOWN\n"
        "   \t\n"
        "  # only a comment\n"
        "left   =   LEFT   ,   LEFT_FACE_LEFT\n"
        "right = RIGHT, LEFT_FACE_RIGHT\r\n"
        "cross = X, RIGHT_FACE_DOWN # bottom\r\n"
        "\r\n"
        "circle = C, RIGHT_FACE_RIGHT\n"
        "square = Z, RIGHT_FACE_LEFT\n"
        "triangle = V, RIGHT_FACE_UP\n"
        "l1 = A, LEFT_TRIGGER_1\n"
        "r1 = S, RIGHT_TRIGGER_1\n"
        "start = ENTER, MIDDLE_RIGHT\n"
        "select = RIGHT_SHIFT, MIDDLE_LEFT");
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

static void test_psx_button_names_follow_the_pad_bits(void) {
    static const char *const names[KEYMAP_BUTTON_COUNT] = {
        "up", "down", "left", "right", "cross", "circle", "square", "triangle", "l1", "r1", "start", "select"
    };
    static const int bits[KEYMAP_BUTTON_COUNT] = {
        P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT, P8_PAD_CROSS, P8_PAD_CIRCLE,
        P8_PAD_SQUARE, P8_PAD_TRIANGLE, P8_PAD_L1, P8_PAD_R1, P8_PAD_START, P8_PAD_SELECT
    };
    assert(KEYMAP_BUTTON_COUNT == 12);
    assert(strcmp(keymap_button_name(11), "select") == 0 && P8_PAD_SELECT == 2048);
    for (int i = 0; i < KEYMAP_BUTTON_COUNT; i++) {
        assert(strcmp(keymap_button_name(i), names[i]) == 0);
        assert(bits[i] == 1 << i);
    }
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

static void test_pad_buttons_or_the_held_bindings(void) {
    keymap_t keymap;
    keymap_defaults(&keymap);
    held_set_t held = { .count = 0 };
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == 0);

    hold(&held, "X");
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == P8_PAD_CROSS);

    held.count = 0;
    hold(&held, "RIGHT_FACE_DOWN");
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == P8_PAD_CROSS);

    hold(&held, "X");
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == P8_PAD_CROSS);

    held.count = 0;
    hold(&held, "RIGHT_SHIFT");
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == P8_PAD_SELECT);

    held.count = 0;
    hold(&held, "MIDDLE_LEFT");
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == P8_PAD_SELECT);

    held.count = 0;
    for (int i = 0; i < KEYMAP_BUTTON_COUNT; i++) {
        for (int j = 0; j < keymap.buttons[i].count; j++) {
            held.held[held.count++] = keymap.buttons[i].bindings[j];
        }
    }
    assert(held.count == 24);
    assert(keymap_pad_buttons(&keymap, fake_is_down, &held) == (1 << KEYMAP_BUTTON_COUNT) - 1);
}

int main(void) {
    test_defaults_bind_cross_to_x_and_right_face_down();
    test_versioned_file_is_the_default_text_and_loads_as_the_defaults();
    test_defaults_hold_the_plan_table_in_pad_bit_order();
    test_missing_file_loads_the_defaults();
    test_unreadable_path_is_invalid_not_the_defaults();
    test_file_over_four_kib_is_invalid();
    test_line_without_equal_is_reported_with_its_number();
    test_unknown_psx_button_is_reported_with_its_number();
    test_empty_item_is_reported_with_its_number();
    test_unknown_key_name_is_reported_with_its_number();
    test_psx_button_given_twice_is_reported_at_the_second_line();
    test_binding_used_twice_is_reported_at_the_second_use();
    test_reserved_key_is_reported_with_its_number();
    test_more_than_four_bindings_is_reported_with_its_number();
    test_line_over_127_characters_is_reported_with_its_number();
    test_psx_button_without_a_line_is_reported_with_line_zero();
    test_invalid_file_reports_the_line_through_load();
    test_blanks_comments_crlf_and_last_line_without_newline_parse();
    test_name_table_is_unique_and_holds_raylib_codes();
    test_psx_button_names_follow_the_pad_bits();
    test_pad_buttons_or_the_held_bindings();
    printf("test_keymap: ok\n");
    return 0;
}
