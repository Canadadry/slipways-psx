#include "pico8_galaxy.h"

#include <string.h>

#include "pico8_economy_score.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "pico8_game_data.h"
#include "pico8_gen.h"
#include "pico8_planet.h"
#include "p8/str.h"
#include "engine/vec.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

/* Adapts pico8_jf_next to pico8_gn_random_fn's ctx shape: `ctx` is always
 * the pico8_jf_state_t this file passes alongside it. */
static pico8_fixed_t pico8_bw_gn_r(void *ctx) {
    return pico8_jf_next((pico8_jf_state_t *)ctx);
}

void pico8_bw(pico8_fixed_t n) {
    pico8_fixed_t one = pico8_fixed_from_int(1);
    pico8_fixed_t threshold = PICO8_FIXED_LIT(0x00000ccc, 0.05);
    pico8_fixed_t two_seventy_five = PICO8_FIXED_LIT(0x0002c000, 2.75);
    pico8_fixed_t eighth = PICO8_FIXED_LIT(0x00002000, 0.125);
    pico8_fixed_t id;
    const pico8_js_t *dk;
    pico8_fixed_t dk_en = pico8_fixed_from_int(0);
    pico8_jf_state_t *oc_r_ctx;
    pico8_gn_state_t *oc_state;
    pico8_jf_state_t *ir_state;

    for (id = one; id <= n; id = pico8_fixed_add(id, one)) {
        pico8_vec_t p_vec;
        pico8_vec_t *p_box;
        pico8_vec_t *ra_box;
        pico8_struct_header_t *nearest;
        char *id_num_str;
        char *id_str;
        pico8_jo_keys_t keys;
        pico8_jo_t *made;

        /* cg.jo is empty only on the very first trial: every accepted
         * planet below self-registers into it, so later candidates really
         * are rejection-sampled against the ones already placed. Not an
         * inert loop over a list that stays empty. */
        do {
            pico8_fixed_t rx = pico8_fixed_add(pico8_fixed_rnd(two_seventy_five), eighth);
            pico8_fixed_t ry = pico8_fixed_add(pico8_fixed_rnd(two_seventy_five), eighth);
            p_vec = pico8_vec_v(rx, ry);
        } while (pico8_ij_ents(p_vec, pico8_cg("jo"), &nearest) <= threshold);

        p_box = (pico8_vec_t *)pico8_zone_alloc(pico8_zone_partie(), sizeof *p_box);
        *p_box = p_vec;
        ra_box = (pico8_vec_t *)pico8_zone_alloc(pico8_zone_partie(), sizeof *ra_box);
        *ra_box = pico8_vec_mul(p_vec, pico8_fixed_from_int(128));

        id_num_str = pico8_tostr_fixed(pico8_zone_frame(), id);
        id_str = pico8_concat2(pico8_zone_partie(), "p", id_num_str);

        keys.id = id_str;
        keys.p = p_box;
        keys.ra = ra_box;

        /* Built for the side effect of self-registering into hv/cv/cg. */
        made = pico8_jo_new(&keys);
        (void)made;
    }

    /* A NULL dk or an absent "en" reads as 0 -- this port's usual
     * absent-field default, not a guess at which difficulty is meant. */
    dk = pico8_dk();
    if (dk != NULL && dk->has_en) {
        dk_en = dk->en;
    }

    oc_r_ctx = pico8_jf(pico8_zone_partie(), PICO8_FIXED_LIT(0x00008000, 0.5), dk_en);
    oc_state = pico8_gn(pico8_zone_partie(), pico8_df_table, PICO8_DF_COUNT, pico8_fixed_from_int(2), pico8_bw_gn_r,
                        oc_r_ctx);
    pico8_jo_set_oc(oc_state);

    ir_state = pico8_jf(pico8_zone_partie(), PICO8_FIXED_LIT(0x0000cccc, 0.8), PICO8_FIXED_LIT(0x00008000, 0.5));
    pico8_jo_set_ir(ir_state);
}
