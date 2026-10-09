#include "platform/psx_pad_map.h"

#include "platform/keymap.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static uint16_t hardware_bit_of_keymap_button(int index) {
    char name[16];
    const char *lower = keymap_button_name(index);
    size_t length = strlen(lower);
    assert(length < sizeof name);
    for (size_t i = 0; i <= length; i++) name[i] = (char)toupper((unsigned char)lower[i]);
    return psx_pad_map_hardware_bit(name);
}

static void test_each_keymap_button_maps_to_its_pad_bit(void) {
    for (int i = 0; i < KEYMAP_BUTTON_COUNT; i++) {
        uint16_t hardware = hardware_bit_of_keymap_button(i);
        assert(hardware != 0);
        assert(psx_pad_map_buttons(hardware) == 1 << i);
    }
}

static void test_buttons_keymap_does_not_name_map_to_nothing(void) {
    static const char *const ignored[] = { "SELECT", "L3", "R3", "L2", "R2" };
    for (size_t i = 0; i < sizeof ignored / sizeof ignored[0]; i++) {
        uint16_t hardware = psx_pad_map_hardware_bit(ignored[i]);
        assert(hardware != 0);
        assert(psx_pad_map_buttons(hardware) == 0);
    }
}

static void test_a_combination_maps_to_the_or_of_its_parts(void) {
    uint16_t cross = psx_pad_map_hardware_bit("CROSS");
    uint16_t up = psx_pad_map_hardware_bit("UP");
    uint16_t select = psx_pad_map_hardware_bit("SELECT");
    assert(psx_pad_map_buttons((uint16_t)(cross | up | select))
           == (psx_pad_map_buttons(cross) | psx_pad_map_buttons(up)));
    assert(psx_pad_map_buttons(0) == 0);
    assert(psx_pad_map_buttons(0xFFFF) == (1 << KEYMAP_BUTTON_COUNT) - 1);
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
    test_each_keymap_button_maps_to_its_pad_bit();
    test_buttons_keymap_does_not_name_map_to_nothing();
    test_a_combination_maps_to_the_or_of_its_parts();
    test_hardware_bits_follow_the_sdk_order();
    test_unknown_or_lowercase_names_have_no_bit();
    printf("test_psx_pad_map: ok\n");
    return 0;
}
