#include "devtools/format_self_test.h"

#include <stdio.h>

int main(void) {
    char text[FORMAT_SELF_TEST_CAP];
    format_self_test(text);
    printf("format %s\n", text);
    return 0;
}
