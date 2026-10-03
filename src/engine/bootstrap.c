#include "engine/bootstrap.h"

#include <assert.h>

#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "engine/entity.h"
#include "game/mouse.h"
#include "game/scroll.h"
#include "game/ui.h"

void reset_world(void) {
    camera_t *real_main_camera;
    mouse_t *real_main_mouse;
    ui_t *real_main_ui;

    entity_registry_abandon_live_entities();
    entity_registry_init();

    /* Build once, never rebuild. The entity module must be ready before the
     * camera and mouse modules, whose inits assert it. */
    if (!entity_ready()) {
        entity_init();
    }
    if (!camera_ready()) {
        camera_init();
    }
    if (!mouse_ready()) {
        mouse_init();
    }
    if (!ui_ready()) {
        ui_init();
    }

    real_main_camera = camera_new();
    real_main_mouse = mouse_new();
    real_main_ui = ui_new();

    entity_registry_set_main_camera((entity_t *)real_main_camera);
    ui_set_main_mouse((entity_t *)real_main_mouse);
    ui_set_main_ui(real_main_ui);
}

void bootstrap_shutdown(void) {
    ui_shutdown();
    mouse_shutdown();
    camera_shutdown();
    entity_shutdown();
    entity_registry_shutdown();
}

void update_entities(void) {
    const entity_list_t *hv = live_entities();
    int i;

    /* GAP-5 */
    for (i = 0; i < hv->count; i++) {
        entity_t *qc = hv->items[i];

        /* The only caller of the state methods: the entity's state picks
         * one at run time, so no site calls one by name and a grep reads
         * them as dead code. */
        if (qc->state != ENTITY_STATE_NONE && qc->class->states[qc->state] != NULL) {
            qc->class->states[qc->state](qc, qc->state_time);
        }

        if (qc->is_dead != NULL && *qc->is_dead) {
            entity_unregister(qc);
        }

        qc->state_time = p8_fixed_add(qc->state_time, p8_fixed_from_int(1));
    }
}
