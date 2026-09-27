#include "pico8_cart.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_audio.h"
#include "pico8_bg.h"
#include "pico8_bootstrap.h"
#include "pico8_struct.h"
#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_dispatch_context.h"
#include "pico8_economy.h"
#include "pico8_economy_score.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_fixed.h"
#include "pico8_galaxy.h"
#include "pico8_game_data.h"
#include "pico8_gfx.h"
#include "pico8_help.h"
#include "pico8_hud.h"
#include "pico8_ke.h"
#include "pico8_layout.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_menu.h"
#include "pico8_mouse_reader.h"
#include "pico8_msgbox.h"
#include "pico8_ob.h"
#include "pico8_pi.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_planet_family_trade.h"
#include "pico8_probe.h"
#include "pico8_research.h"
#include "pico8_research_icon.h"
#include "pico8_score_panel.h"
#include "pico8_scroll.h"
#include "pico8_slipway.h"
#include "pico8_trade.h"
#include "pico8_ui.h"
#include "pico8_ui_button.h"
#include "pico8_value.h"
#include "pico8_vec.h"
#include "pico8_wormhole.h"

/* pico8_data_init() and pico8_score_panel_init() each run further module
 * inits of their own (stash/research, score-panel display); listing those
 * here would run them twice. */
void pico8_cart_load(void) {
    pico8_mem_t *mem;

    pico8_ke_init();
    assert(pico8_ke_ready() && "ke must be declared before every ke-descended class");
    pico8_qb_init();
    pico8_nf_init();
    pico8_ui_init();

    pico8_layout_init();
    pico8_pi_init();
    pico8_trade_init();
    pico8_game_data_init();

    pico8_js_init();
    pico8_fq_init();
    pico8_bg_init();
    pico8_mh_init();
    pico8_iv_init();
    pico8_probe_init();
    pico8_economy_init();

    mem = pico8_dispatch_mem();
    if (mem != NULL) {
        pico8_data_init(mem);
    }

    pico8_jo_init();
    pico8_jc_init();
    pico8_planet_family_init();
    assert(pico8_qh_ready() && pico8_ga_ready() && pico8_jo_ready() &&
           "qh and ga extend jo: pico8_jo_init() must run first");
    pico8_planet_family_trade_init();
    assert(pico8_nr_ready() && pico8_mx_ready() && pico8_qh_ready() && pico8_hj_ready() &&
           "nr extends qh and mx extends hj: pico8_planet_family_init() must run first");
    pico8_wormhole_init();

    pico8_help_init();
    assert(pico8_fv_ready() && pico8_g__ready() && "fv extends g_: pico8_probe_init() must run first");
    pico8_score_panel_init();
    pico8_research_icon_init();
    pico8_menu_init();

    pico8_help_set_eu(pico8_eu_dispatch);
    pico8_score_panel_set_init(pico8_init_dispatch);
}

void pico8_cart_shutdown(void) {
    pico8_menu_shutdown();
    pico8_research_icon_shutdown();
    pico8_score_panel_shutdown();
    pico8_help_shutdown();

    pico8_wormhole_shutdown();
    pico8_planet_family_trade_shutdown();
    pico8_planet_family_shutdown();
    pico8_jc_shutdown();
    pico8_jo_shutdown();

    pico8_data_shutdown();

    pico8_economy_shutdown();
    pico8_probe_shutdown();
    pico8_iv_shutdown();
    pico8_mh_shutdown();
    pico8_bg_shutdown();
    pico8_fq_shutdown();
    pico8_js_shutdown();

    pico8_game_data_shutdown();
    pico8_trade_shutdown();
    pico8_layout_shutdown();

    pico8_ui_shutdown();
    pico8_nf_shutdown();
    pico8_qb_shutdown();
    pico8_ke_shutdown();
}

void pico8__init(void) {
    pico8_mem_t *mem = pico8_dispatch_mem();

    assert(pico8_ie_ready() && "pico8_cart_load() must run before _init()");
    assert(pico8_eu_field_value().as.function == pico8_eu_dispatch &&
           pico8_init_field_value().as.function == pico8_init_dispatch &&
           "pico8_help_init()/pico8_score_panel_init() reset these bridges, so pico8_cart_load() must "
           "install them afterwards: every button copies the value when it is built");

    pico8_cu(mem);
    pico8_jl();

    /* `ie.gy,kc=ie.eh`: Lua pads the extra name with nil, so `gy` takes `eh`
     * and `kc` is cleared. `gy` aliases `eh`'s heap vector, it is not a copy. */
    pico8_ie_set_gy(pico8_ie_eh());
    pico8_msgbox_set_kc(NULL);

    (void)pico8_fv_new(NULL);
    /* TROU-10 */
    pico8_music(0);
}

pico8_value_t pico8_init_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8__init();
    return pico8_value_ref(NULL);
}

/* `nv(ix,js)`'s per-element adapter: an entry that is not a REF to a table
 * contributes nothing. */
static int pico8_eu_ix_map(pico8_value_t e, void *ctx, pico8_value_t *out_value) {
    const pico8_ob_table_t *row;
    pico8_js_t *made;

    (void)ctx;
    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    row = (const pico8_ob_table_t *)e.as.ref;
    made = pico8_js_new(row);
    *out_value = pico8_value_ref(made);
    return 1;
}

void pico8_eu(pico8_js_t *nj) {
    const pico8_entity_kind_t *gx[7];
    int i;

    assert(pico8_hy_ready() && pico8_qh_ready() && "pico8_cart_load() must run before eu()");

    pico8_economy_set_dk(nj);

    /* `jl()` runs a second time here, as in the cart: it unconditionally
     * rebuilds hv/cv/cg and the mo/je/ok singletons. */
    pico8_jl();
    (void)pico8_bg_new(NULL);
    pico8_bw(pico8_fixed_from_int(90));

    /* TROU-7 */
    pico8_probe_set_fm((pico8_struct_header_t *)pico8_hy_new(NULL));
    pico8_pj_set_ju(pico8_pj_new(NULL));

    if (pico8_ix() != NULL) {
        pico8_list_t ix_items;
        pico8_list_t mapped;
        int j;

        pico8_list_init(&ix_items);
        for (j = 0; j < pico8_ix()->count; j++) {
            if (pico8_ix()->entries[j].key.kind == PICO8_OB_KEY_INDEX) {
                pico8_list_add(&ix_items, pico8_ix()->entries[j].value);
            }
        }
        mapped = pico8_nv(&ix_items, pico8_eu_ix_map, NULL);
        pico8_list_free(&mapped);
        pico8_list_free(&ix_items);
    }

    gx[0] = pico8_qh_kind();
    gx[1] = pico8_os_kind();
    gx[2] = pico8_nr_kind();
    gx[3] = pico8_fr_kind();
    gx[4] = pico8_nl_kind();
    gx[5] = pico8_mx_kind();
    gx[6] = pico8_hj_kind();

    for (i = 0; i < 7; i++) {
        pico8_value_t dr_val;
        const pico8_ob_table_t *dr_entry = NULL;
        pico8_ob_table_t *overrides;
        pico8_jb_t *jb;

        if (pico8_dr() != NULL && pico8_ob_table_get_index(pico8_dr(), i + 1, &dr_val) &&
            dr_val.kind == PICO8_VALUE_REF && dr_val.as.ref != NULL) {
            dr_entry = (const pico8_ob_table_t *)dr_val.as.ref;
        }
        jb = pico8_jb_new(dr_entry);

        overrides = pico8_ob_table_new();
        pico8_ob_table_set_name(overrides, "ke", pico8_value_ref((void *)gx[i]));
        pico8_ob_table_set_name(overrides, "jb", pico8_value_ref(jb));
        (void)pico8_ie_new(overrides);
        pico8_ob_table_free(overrides);
    }

    assert(pico8_fm_hy() != NULL && "fm must be the hy of this game before fm:lj()");
    pico8_hy_lj(pico8_fm_hy());
}

pico8_value_t pico8_eu_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_eu((pico8_js_t *)self);
    return pico8_value_ref(NULL);
}

void pico8__update60(void) {
    pico8_dg();
    pico8_ui_lj(pico8_dispatch_mem(), pico8_ok());
    PICO8_MEMBER_CENSUS_WALK(pico8_hv());
}

void pico8__draw(void) {
    pico8_mem_t *mem = pico8_dispatch_mem();

    pico8_cls(mem);
    pico8_dv_default(mem);
    pico8_dc("kj");
    pico8_dc("ev");
    PICO8_MEMBER_CENSUS_WALK(pico8_hv());
}
