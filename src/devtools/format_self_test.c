#include "devtools/format_self_test.h"

#include <stdio.h>

void format_self_test(char text[FORMAT_SELF_TEST_CAP]) {
    snprintf(text, FORMAT_SELF_TEST_CAP, "%d|%d|%d|%s|%s|%ld|%ld|%02ld|%02ld|%lu|%lu|%04lu|%04lu",
             0, -123456, 2147483647, "", "planet",
             0L, -45L, 7L, 123L,
             0UL, 4294967295UL, 7UL, 12345UL);
}
