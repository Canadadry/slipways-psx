#include "pico8_str.h"

#include <stdlib.h>
#include <string.h>

char *pico8_sub(const char *s, int i, int j) {
    int len = (int)strlen(s);

    /* Clamp out-of-range indices the way PICO-8's own sub() does, rather
     * than reading out of bounds or erroring. */
    if (i < 1) {
        i = 1;
    }
    if (j > len) {
        j = len;
    }

    if (j < i) {
        /* Empty range (e.g. an empty quoted-string token in ob()'s
         * grammar): still a valid, independent, heap-owned string. */
        char *out = malloc(1);
        out[0] = '\0';
        return out;
    }

    int n = j - i + 1;
    char *out = malloc((size_t)n + 1);
    memcpy(out, s + (i - 1), (size_t)n);
    out[n] = '\0';
    return out;
}
