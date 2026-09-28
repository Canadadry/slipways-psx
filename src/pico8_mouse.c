#include "pico8_mouse.h"

#include <assert.h>
#include <stddef.h>

void pico8_bu(const pico8_input_t *input, pico8_nf_button_t *qn, int oa) {
    int on;
    int ia = 0;

    assert(input != NULL && qn != NULL && "bu() dereferences both");

    on = (pico8_stat(input, 34) & oa) > 0;
    if (qn->has_on) {
        ia = qn->on;
    }

    qn->has_on = 1;
    qn->on = on;
    qn->jn = on && !ia;
    qn->hm = ia && !on;
}
