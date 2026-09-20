#include "../src/pico8_dump.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_mem.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DUMP_HEADER "128 128\n"
#define DUMP_ROW_CHARS (PICO8_SCREEN_WIDTH)
#define DUMP_LINE_LEN (DUMP_ROW_CHARS + 1)
#define DUMP_SIZE (sizeof(DUMP_HEADER) - 1 + (size_t)DUMP_LINE_LEN * PICO8_SCREEN_HEIGHT)

static char *dump_to_buffer(const pico8_mem_t *mem, size_t *out_len) {
    FILE *tmp = tmpfile();
    assert(tmp != NULL);

    int ok = pico8_dump_screen(mem, tmp);
    assert(ok);

    long size = ftell(tmp);
    assert(size >= 0);
    rewind(tmp);

    char *buf = malloc((size_t)size + 1);
    assert(buf != NULL);
    size_t got = fread(buf, 1, (size_t)size, tmp);
    fclose(tmp);
    assert(got == (size_t)size);
    buf[got] = '\0';

    *out_len = got;
    return buf;
}

static void set_expected_pixel(char *expected, int x, int y, char hex_digit) {
    size_t line_start = strlen(DUMP_HEADER) + (size_t)y * DUMP_LINE_LEN;
    expected[line_start + (size_t)x] = hex_digit;
}

static void test_dump_of_cleared_screen_is_all_zero_grid(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);
    pico8_cls(&mem);

    size_t actual_len;
    char *actual = dump_to_buffer(&mem, &actual_len);

    char *expected = malloc(DUMP_SIZE + 1);
    assert(expected != NULL);
    memcpy(expected, DUMP_HEADER, strlen(DUMP_HEADER));
    size_t offset = strlen(DUMP_HEADER);
    for (int row = 0; row < PICO8_SCREEN_HEIGHT; row++) {
        memset(expected + offset, '0', DUMP_ROW_CHARS);
        offset += DUMP_ROW_CHARS;
        expected[offset++] = '\n';
    }
    expected[DUMP_SIZE] = '\0';

    assert(actual_len == DUMP_SIZE);
    assert(memcmp(actual, expected, DUMP_SIZE) == 0);

    free(actual);
    free(expected);
}

static void test_dump_of_hand_drawn_pattern_matches_exact_text(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_cls(&mem);
    pico8_pset(&gfx, &mem, 0, 0, 10);
    pico8_pset(&gfx, &mem, 127, 0, 1);
    pico8_pset(&gfx, &mem, 64, 64, 15);
    pico8_pset(&gfx, &mem, 0, 127, 9);

    size_t actual_len;
    char *actual = dump_to_buffer(&mem, &actual_len);

    char *expected = malloc(DUMP_SIZE + 1);
    assert(expected != NULL);
    memcpy(expected, DUMP_HEADER, strlen(DUMP_HEADER));
    size_t offset = strlen(DUMP_HEADER);
    for (int row = 0; row < PICO8_SCREEN_HEIGHT; row++) {
        memset(expected + offset, '0', DUMP_ROW_CHARS);
        offset += DUMP_ROW_CHARS;
        expected[offset++] = '\n';
    }
    expected[DUMP_SIZE] = '\0';

    set_expected_pixel(expected, 0, 0, 'a');
    set_expected_pixel(expected, 127, 0, '1');
    set_expected_pixel(expected, 64, 64, 'f');
    set_expected_pixel(expected, 0, 127, '9');

    assert(actual_len == DUMP_SIZE);
    assert(memcmp(actual, expected, DUMP_SIZE) == 0);

    free(actual);
    free(expected);
}

int main(void) {
    test_dump_of_cleared_screen_is_all_zero_grid();
    test_dump_of_hand_drawn_pattern_matches_exact_text();
    printf("test_pico8_dump: all tests passed\n");
    return 0;
}
