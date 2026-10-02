#include "devtools/fixed_test_double.h"

p8_fixed_t pico8_fixed_from_double(double d) {
    double scaled = d * 65536.0;
    int64_t raw = (int64_t)(scaled >= 0 ? scaled + 0.5 : scaled - 0.5);
    return (p8_fixed_t)(uint32_t)raw;
}

double pico8_fixed_to_double(p8_fixed_t x) {
    return (double)x / 65536.0;
}
