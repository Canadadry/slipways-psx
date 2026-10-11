#include "platform/psx_pad_map.h"

#include "p8/input.h"
#include "platform/keymap.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *psx;
    const char *gamepad;
} button_pair_t;

static const button_pair_t pairs[16] = {
    { "SELECT", "MIDDLE_LEFT" },        { "L3", "LEFT_THUMB" },           { "R3", "RIGHT_THUMB" },
    { "START", "MIDDLE_RIGHT" },        { "UP", "LEFT_FACE_UP" },         { "RIGHT", "LEFT_FACE_RIGHT" },
    { "DOWN", "LEFT_FACE_DOWN" },       { "LEFT", "LEFT_FACE_LEFT" },     { "L2", "LEFT_TRIGGER_2" },
    { "R2", "RIGHT_TRIGGER_2" },        { "L1", "LEFT_TRIGGER_1" },       { "R1", "RIGHT_TRIGGER_1" },
    { "TRIANGLE", "RIGHT_FACE_UP" },    { "CIRCLE", "RIGHT_FACE_RIGHT" }, { "CROSS", "RIGHT_FACE_DOWN" },
    { "SQUARE", "RIGHT_FACE_LEFT" },
};

static void parse_default_keymap(keymap_t *out) {
    static char text[8192];
    FILE *file = fopen("keymap.default.cfg", "rb");
    assert(file != NULL);
    size_t length = fread(text, 1, sizeof text - 1, file);
    assert(length < sizeof text - 1);
    text[length] = '\0';
    fclose(file);
    keymap_error_t error;
    int parsed = keymap_parse(text, out, &error);
    assert(parsed);
}

static int only_this_gamepad_button_is_down(keymap_device_t device, int code, void *context) {
    const keymap_binding_t *down = context;
    return device == KEYMAP_GAMEPAD && code == down->code;
}

static void test_each_hardware_button_presses_what_the_default_keymap_binds_to_its_gamepad_button(void) {
    keymap_t keymap;
    parse_default_keymap(&keymap);
    int mapped = 0;
    for (int i = 0; i < 16; i++) {
        keymap_binding_t gamepad;
        int found = keymap_find_binding(pairs[i].gamepad, &gamepad);
        assert(found);
        int expected = keymap_pad_buttons(&keymap, only_this_gamepad_button_is_down, &gamepad);
        assert(psx_pad_map_buttons(psx_pad_map_hardware_bit(pairs[i].psx)) == expected);
        mapped |= expected;
    }
    assert(mapped == (1 << P8_PAD_BUTTON_COUNT) - 1 && "every button has a hardware button on the PSX pad");
}

static void test_buttons_keymap_does_not_name_map_to_nothing(void) {
    static const char *const ignored[] = { "L3", "R3", "L2", "R2" };
    for (size_t i = 0; i < sizeof ignored / sizeof ignored[0]; i++) {
        uint16_t hardware = psx_pad_map_hardware_bit(ignored[i]);
        assert(hardware != 0);
        assert(psx_pad_map_buttons(hardware) == 0);
    }
}

static void test_a_combination_maps_to_the_or_of_its_parts(void) {
    uint16_t cross = psx_pad_map_hardware_bit("CROSS");
    uint16_t up = psx_pad_map_hardware_bit("UP");
    uint16_t l2 = psx_pad_map_hardware_bit("L2");
    assert(psx_pad_map_buttons((uint16_t)(cross | up | l2))
           == (psx_pad_map_buttons(cross) | psx_pad_map_buttons(up)));
    assert(psx_pad_map_buttons(0) == 0);
    assert(psx_pad_map_buttons(0xFFFF) == (1 << P8_PAD_BUTTON_COUNT) - 1);
}

static void test_hardware_bits_follow_the_sdk_order(void) {
    static const char *const order[16] = { "SELECT", "L3",       "R3",     "START", "UP",    "RIGHT",
                                           "DOWN",   "LEFT",     "L2",     "R2",    "L1",    "R1",
                                           "TRIANGLE", "CIRCLE", "CROSS", "SQUARE" };
    for (int bit = 0; bit < 16; bit++) assert(psx_pad_map_hardware_bit(order[bit]) == 1u << bit);
}

static void test_unknown_or_lowercase_names_have_no_bit(void) {
    assert(psx_pad_map_hardware_bit("cross") == 0);
    assert(psx_pad_map_hardware_bit("X") == 0);
    assert(psx_pad_map_hardware_bit("") == 0);
    assert(psx_pad_map_hardware_bit("-") == 0);
}

int main(void) {
    test_each_hardware_button_presses_what_the_default_keymap_binds_to_its_gamepad_button();
    test_buttons_keymap_does_not_name_map_to_nothing();
    test_a_combination_maps_to_the_or_of_its_parts();
    test_hardware_bits_follow_the_sdk_order();
    test_unknown_or_lowercase_names_have_no_bit();
    printf("test_psx_pad_map: ok\n");
    return 0;
}
