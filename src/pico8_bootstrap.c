#include "pico8_bootstrap.h"

#include <assert.h>

#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_mouse_reader.h"
#include "pico8_ob.h"
#include "pico8_scroll.h"
#include "pico8_type_tag.h"
#include "pico8_ui.h"
#include "pico8_value.h"

/* Lua truthiness, sans la clause REF-NULL que portent les autres helpers du
 * port : aucun appelant n'a été montré l'atteignant avec un nil. L'assert
 * énonce cette hypothèse au lieu de lui faire confiance — s'il part, c'est
 * qu'un nil aurait été rendu vrai. */
static int pico8_dg_truthy(pico8_value_t v) {
    assert(!(v.kind == PICO8_VALUE_REF && v.as.ref == NULL));
    return !(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

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

    real_mo = pico8_qb_new(NULL);
    real_je = pico8_nf_new(NULL);
    real_ok = pico8_ui_new(NULL);

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
        pico8_value_t me_val;
        pico8_value_t p_val;
        pico8_value_t t_val;

        assert((qc == NULL || pico8_type_tag_of(qc) == PICO8_TYPE_TAG_STRUCT) &&
               "hv holds structs only; no class instance descends from ke any more");

        /* The only caller of the state methods (`ph`, `ly`, `ll`, ...): the
         * name comes from `me` at run time, so no site ever writes `:ly(`
         * and a grep reads them as dead code. */
        if (pico8_entity_get_field(qc, "me", &me_val) && me_val.kind == PICO8_VALUE_STRING) {
            pico8_value_t t_arg;
            pico8_value_t discard;
            if (!pico8_entity_get_field(qc, "t", &t_arg)) {
                t_arg = pico8_value_ref(NULL);
            }
            pico8_mv(qc, me_val.as.string, 1, t_arg, pico8_value_ref(NULL), &discard);
        }

        if (pico8_entity_get_field(qc, "p_", &p_val) && pico8_dg_truthy(p_val)) {
            pico8_he(qc);
        }

        if (pico8_entity_get_field(qc, "t", &t_val) && t_val.kind == PICO8_VALUE_FIXED) {
            ((pico8_ke_t *)qc)->t = pico8_fixed_add(t_val.as.fixed, pico8_fixed_from_int(1));
        }
    }
}
