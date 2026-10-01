#ifndef PICO8_MOUSE_H
#define PICO8_MOUSE_H

#include "pico8_input.h"

/* One button's state, `nf.l` or `nf.r` in the cart: until the first `bu`,
 * `has_on` is 0 and `on`/`jn`/`hm` are absent. */
typedef struct {
    int has_on;
    int on;
    int jn;
    int hm;
} pico8_nf_button_t;

/* `qn` is a plain nested record, not an instance: on/jn/hm are read and
 * written on it directly, with no class chain behind them. Both `input`
 * and `qn` are dereferenced, so neither may be NULL, and nothing here is
 * allocated or freed. `oa` is a raw stat(34) bitmask (1 left, 6 right),
 * masked with an ordinary integer `&` since neither side is fractional. */
void pico8_bu(const pico8_input_t *input, pico8_nf_button_t *qn, int oa);

#endif
