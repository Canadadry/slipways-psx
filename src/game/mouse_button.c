#include "game/mouse_button.h"

#include <assert.h>
#include <stddef.h>

void mouse_button_update(const p8_input_t *input, mouse_button_t *qn, int oa) {
    int on;
    int ia = 0;

    assert(input != NULL && qn != NULL && "bu() dereferences both");

    on = (p8_stat(input, 34) & oa) > 0;
    if (qn->has_held) {
        ia = qn->held;
    }

    qn->has_held = 1;
    qn->held = on;
    qn->pressed = on && !ia;
    qn->released = ia && !on;
}
