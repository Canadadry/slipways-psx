#include "pico8_bootstrap.h"

#include <assert.h>

#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_mouse_reader.h"
#include "pico8_scroll.h"
#include "pico8_type_tag.h"
#include "pico8_ui.h"
#include "pico8_value.h"

void pico8_jl(void) {
    pico8_qb_t *real_mo;
    pico8_nf_t *real_je;
    pico8_ui_t *real_ok;

    pico8_entity_init();

    /* Build once, never rebuild. `ke` must exist before `qb`/`nf`/`ui`,
     * whose inits assert it. */
    if (!pico8_ke_ready()) {
        pico8_ke_init();
    }
    if (!pico8_qb_ready()) {
        pico8_qb_init();
    }
    if (!pico8_nf_ready()) {
        pico8_nf_init();
    }
    if (!pico8_ui_ready()) {
        pico8_ui_init();
    }

    real_mo = pico8_qb_new();
    real_je = pico8_nf_new();
    real_ok = pico8_ui_new();

    pico8_entity_set_mo((pico8_struct_header_t *)real_mo);
    pico8_ui_set_je((pico8_struct_header_t *)real_je);
    pico8_ui_set_ok(real_ok);
}

void pico8_bootstrap_shutdown(void) {
    pico8_ui_shutdown();
    pico8_nf_shutdown();
    pico8_qb_shutdown();
    pico8_ke_shutdown();
    pico8_entity_shutdown();
}

void pico8_dg(void) {
    const pico8_list_t *hv = pico8_hv();
    int i;

    /* TROU-5 */
    for (i = 0; i < hv->count; i++) {
        pico8_struct_header_t *qc = hv->items[i].as.ref;
        pico8_ke_t *ke = hv->items[i].as.ref;

        assert((qc == NULL || pico8_type_tag_of(qc) == PICO8_TYPE_TAG_STRUCT) &&
               "hv holds structs only; no class instance descends from ke any more");

        /* The only caller of the state methods (`ph`, `ly`, `ll`, ...): the
         * name comes from `me` at run time, so no site ever writes `:ly(`
         * and a grep reads them as dead code. */
        if (ke->me != NULL) {
            int state = pico8_state_of(ke->me);

            if (state >= 0 && qc->cls->states[state] != NULL) {
                qc->cls->states[state](qc, ke->t);
            }
        }

        if (qc->cls->get_p_ != NULL && qc->cls->get_p_(qc)) {
            pico8_he(qc);
        }

        ke->t = pico8_fixed_add(ke->t, pico8_fixed_from_int(1));
    }
}
