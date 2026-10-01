#include "engine/bootstrap.h"

#include <assert.h>

#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "engine/entity.h"
#include "game/mouse.h"
#include "game/scroll.h"
#include "game/ui.h"

void pico8_jl(void) {
    pico8_qb_t *real_mo;
    pico8_nf_t *real_je;
    pico8_ui_t *real_ok;

    pico8_entity_abandonner_hv();
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
    const pico8_ents_t *hv = pico8_hv();
    int i;

    /* TROU-5 */
    for (i = 0; i < hv->count; i++) {
        pico8_struct_header_t *qc = hv->items[i];

        /* The only caller of the state methods (`ph`, `ly`, `ll`, ...): the
         * name comes from `me` at run time, so no site ever writes `:ly(`
         * and a grep reads them as dead code. */
        if (qc->me != NULL) {
            int state = pico8_state_of(qc->me);

            if (state >= 0 && qc->cls->states[state] != NULL) {
                qc->cls->states[state](qc, qc->t);
            }
        }

        if (qc->p_ != NULL && *qc->p_) {
            pico8_he(qc);
        }

        qc->t = pico8_fixed_add(qc->t, pico8_fixed_from_int(1));
    }
}
