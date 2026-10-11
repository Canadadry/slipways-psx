#include "p8/format.h"

#include <stdarg.h>
#include <stddef.h>

/* Linked with -Wl,--wrap=snprintf,--wrap=vsnprintf: the SDK's formatter
   builds powers of ten in 32 bits, so every value from 276447232 up prints
   wrong digits (2147483647 reads 100200000025264127). */
int __wrap_vsnprintf(char *buffer, size_t size, const char *format, va_list args);
int __wrap_snprintf(char *buffer, size_t size, const char *format, ...);

int __wrap_vsnprintf(char *buffer, size_t size, const char *format, va_list args) {
    return p8_vsnprintf(buffer, size, format, args);
}

int __wrap_snprintf(char *buffer, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int length = p8_vsnprintf(buffer, size, format, args);
    va_end(args);
    return length;
}
