#ifndef GAME_MOUSE_BUTTON_H
#define GAME_MOUSE_BUTTON_H

#include "p8/input.h"

/* One button's state, `nf.l` or `nf.r` in the cart: until the first `bu`,
 * `has_on` is 0 and `on`/`jn`/`hm` are absent. */
typedef struct {
    int has_held;
    int held;
    int pressed;
    int released;
} mouse_button_t;

/* `qn` is a plain nested record, not an instance: on/jn/hm are read and
 * written on it directly, with no class chain behind them. Both `input`
 * and `qn` are dereferenced, so neither may be NULL, and nothing here is
 * allocated or freed. `oa` is a raw stat(34) bitmask (1 left, 6 right),
 * masked with an ordinary integer `&` since neither side is fractional. */
void mouse_button_update(const p8_input_t *input, mouse_button_t *qn, int oa);

#endif
