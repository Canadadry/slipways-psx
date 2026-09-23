#include "pico8_value.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pico8_value_t pico8_value_fixed(pico8_fixed_t x) {
    pico8_value_t v;
    v.kind = PICO8_VALUE_FIXED;
    v.as.fixed = x;
    return v;
}

pico8_value_t pico8_value_string(const char *s) {
    pico8_value_t v;
    v.kind = PICO8_VALUE_STRING;
    v.as.string = s;
    return v;
}

pico8_value_t pico8_value_bool(int b) {
    pico8_value_t v;
    v.kind = PICO8_VALUE_BOOL;
    v.as.boolean = b ? 1 : 0;
    return v;
}

pico8_value_t pico8_value_ref(void *p) {
    pico8_value_t v;
    v.kind = PICO8_VALUE_REF;
    v.as.ref = p;
    return v;
}

pico8_value_t pico8_value_function(pico8_value_function_t fn) {
    pico8_value_t v;
    v.kind = PICO8_VALUE_FUNCTION;
    v.as.function = fn;
    return v;
}

int pico8_value_equal(pico8_value_t a, pico8_value_t b) {
    if (a.kind != b.kind) {
        return 0;
    }
    switch (a.kind) {
        case PICO8_VALUE_FIXED:
            return a.as.fixed == b.as.fixed;
        case PICO8_VALUE_BOOL:
            return a.as.boolean == b.as.boolean;
        case PICO8_VALUE_STRING:
            return a.as.string == b.as.string;
        case PICO8_VALUE_REF:
            return a.as.ref == b.as.ref;
        case PICO8_VALUE_FUNCTION:
            return a.as.function == b.as.function;
    }
    return 0;
}

/* Not strdup() (POSIX, not C99) -- this codebase's build is -std=c99. */
static char *pico8_strdup_owned(const char *s) {
    size_t n = strlen(s);
    char *out = malloc(n + 1);
    memcpy(out, s, n + 1);
    return out;
}

char *pico8_value_tostr(pico8_value_t v) {
    char buf[64];

    switch (v.kind) {
        case PICO8_VALUE_STRING:
            return pico8_strdup_owned(v.as.string ? v.as.string : "");
        case PICO8_VALUE_BOOL:
            return pico8_strdup_owned(v.as.boolean ? "true" : "false");
        case PICO8_VALUE_FIXED: {
            /* TROU-22 */
            int negative = v.as.fixed < 0;
            uint32_t mag = negative ? 0u - (uint32_t)v.as.fixed
                                    : (uint32_t)v.as.fixed;
            uint32_t ip = mag >> 16;
            uint32_t p = (mag & 0xFFFFu) * 625u;
            uint32_t q = p >> 12;
            uint32_t r = p & 4095u;
            /* Round half to even, as glibc printf does: half-up would be
             * wrong on 8 of the 16 exact ties per integer part, e.g.
             * 0x0000.0800 -> "0.0312". */
            if (r > 2048u || (r == 2048u && (q & 1u))) {
                q++;
            }
            if (q == 10000u) {
                ip++;
                q = 0;
            }
            snprintf(buf, sizeof buf, "%s%lu.%04lu", negative ? "-" : "",
                     (unsigned long)ip, (unsigned long)q);
            char *dot = strchr(buf, '.');
            if (dot) {
                char *end = buf + strlen(buf) - 1;
                while (end > dot && *end == '0') {
                    *end-- = '\0';
                }
                if (end == dot) {
                    *end = '\0';
                }
            }
            return pico8_strdup_owned(buf);
        }
        case PICO8_VALUE_REF:
        case PICO8_VALUE_FUNCTION:
            /* TROU-13 */
            return pico8_strdup_owned("");
    }
    return pico8_strdup_owned("");
}
