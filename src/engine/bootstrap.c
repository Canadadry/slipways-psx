#include "engine/bootstrap.h"

#include <assert.h>

#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "engine/entity.h"
#include "game/mouse.h"
#include "game/scroll.h"
#include "game/ui.h"

void reset_world(void) {
    pico8_qb_t *real_main_camera;
    pico8_nf_t *real_main_mouse;
    pico8_ui_t *real_main_ui;

    entity_registry_abandon_live_entities();
    entity_registry_init();

    /* Build once, never rebuild. `ke` must exist before `qb`/`nf`/`ui`,
     * whose inits assert it. */
    if (!entity_ready()) {
        entity_init();
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

    real_main_camera = pico8_qb_new();
    real_main_mouse = pico8_nf_new();
    real_main_ui = pico8_ui_new();

    entity_registry_set_main_camera((entity_t *)real_main_camera);
    pico8_ui_set_je((entity_t *)real_main_mouse);
    pico8_ui_set_ok(real_main_ui);
}

void bootstrap_shutdown(void) {
    pico8_ui_shutdown();
    pico8_nf_shutdown();
    pico8_qb_shutdown();
    entity_shutdown();
    entity_registry_shutdown();
}

void update_entities(void) {
    const entity_list_t *hv = live_entities();
    int i;

    /* TROU-5 */
    for (i = 0; i < hv->count; i++) {
        entity_t *qc = hv->items[i];

        /* The only caller of the state methods (`ph`, `ly`, `ll`, ...): the
         * name comes from `me` at run time, so no site ever writes `:ly(`
         * and a grep reads them as dead code. */
        if (qc->state != NULL) {
            int state = entity_state_of(qc->state);

            if (state >= 0 && qc->class->states[state] != NULL) {
                qc->class->states[state](qc, qc->state_time);
            }
        }

        if (qc->is_dead != NULL && *qc->is_dead) {
            entity_unregister(qc);
        }

        qc->state_time = p8_fixed_add(qc->state_time, p8_fixed_from_int(1));
    }
}
