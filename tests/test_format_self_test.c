#include "devtools/format_self_test.h"

#include <assert.h>
#include <string.h>

static void test_every_conversion_formats_as_on_pc(void) {
    char text[FORMAT_SELF_TEST_CAP];
    format_self_test(text);
    assert(strcmp(text, "0|-123456|2147483647||planet|0|-45|07|123|0|4294967295|0007|12345") == 0);
}

int main(void) {
    test_every_conversion_formats_as_on_pc();
    return 0;
}
