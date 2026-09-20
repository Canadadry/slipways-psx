#include "../src/pico8_str.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_sub_first_character(void) {
    char *r = pico8_sub("hello", 1, 1);
    assert(strcmp(r, "h") == 0);
    free(r);
}

static void test_sub_last_character(void) {
    char *r = pico8_sub("hello", 5, 5);
    assert(strcmp(r, "o") == 0);
    free(r);
}

static void test_sub_full_string(void) {
    char *r = pico8_sub("hello", 1, 5);
    assert(strcmp(r, "hello") == 0);
    free(r);
}

static void test_sub_middle_range(void) {
    char *r = pico8_sub("hello", 2, 4);
    assert(strcmp(r, "ell") == 0);
    free(r);
}

static void test_sub_empty_range_when_j_less_than_i(void) {
    char *r = pico8_sub("hello", 3, 2);
    assert(strcmp(r, "") == 0);
    free(r);
}

int main(void) {
    test_sub_first_character();
    test_sub_last_character();
    test_sub_full_string();
    test_sub_middle_range();
    test_sub_empty_range_when_j_less_than_i();

    printf("test_pico8_str: all tests passed\n");
    return 0;
}
