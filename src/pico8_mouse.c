/* function bu(qn,oa) -- see pico8_mouse.h's own comment on this
 * declaration for the full trace. */

#include "pico8_mouse.h"

void pico8_bu(const pico8_input_t *input, pico8_ob_table_t *qn, int oa) {
    int on = (pico8_stat(input, 34) & oa) > 0;
    pico8_value_t on_prev_val;
    int ia = 0;

    if (pico8_ob_table_get_name(qn, "on", &on_prev_val) && on_prev_val.kind == PICO8_VALUE_BOOL) {
        ia = on_prev_val.as.boolean;
    }

    pico8_ob_table_set_name(qn, "on", pico8_value_bool(on));
    pico8_ob_table_set_name(qn, "jn", pico8_value_bool(on && !ia));
    pico8_ob_table_set_name(qn, "hm", pico8_value_bool(ia && !on));
}
