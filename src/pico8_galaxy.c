#include "pico8_galaxy.h"

#include <stdlib.h>
#include <string.h>

#include "pico8_data_init.h"
#include "pico8_economy_score.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_gen.h"
#include "pico8_planet.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* jo.oc's inner random source, which pico8_gn_free never touches (see
 * pico8_gen.h): owned here, 1:1 with whichever pico8_gn_state_t currently
 * sits in jo.oc, and rebuilt with it on every pico8_bw(). */
static pico8_jf_state_t *g_bw_oc_r_ctx = NULL;

/* Returns a newly heap-allocated string the caller owns and must free(). */
static char *pico8_bw_concat2(const char *a, const char *b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char *out = malloc(la + lb + 1);
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = '\0';
    return out;
}

/* Returns a new list the caller owns and must release with
 * pico8_list_free(); a NULL table yields an empty one. */
static pico8_list_t pico8_bw_positional_values(const pico8_ob_table_t *table) {
    pico8_list_t list;
    int i;

    pico8_list_init(&list);
    if (table == NULL) {
        return list;
    }
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
            pico8_list_add(&list, table->entries[i].value);
        }
    }
    return list;
}

/* Adapts pico8_jf_next to pico8_gn_random_fn's ctx shape: `ctx` is always
 * the pico8_jf_state_t this file passes alongside it. */
static pico8_fixed_t pico8_bw_gn_r(void *ctx) {
    return pico8_jf_next((pico8_jf_state_t *)ctx);
}

void pico8_bw(pico8_fixed_t n) {
    pico8_fixed_t one = pico8_fixed_from_int(1);
    pico8_fixed_t threshold = PICO8_FIXED_LIT(0x00000ccd, 0.05);
    pico8_fixed_t two_seventy_five = PICO8_FIXED_LIT(0x0002c000, 2.75);
    pico8_fixed_t eighth = PICO8_FIXED_LIT(0x00002000, 0.125);
    pico8_fixed_t id;
    const pico8_ob_table_t *df;
    const pico8_ob_table_t *dk;
    pico8_fixed_t dk_en = pico8_fixed_from_int(0);
    pico8_value_t en_val;
    pico8_list_t hr_list;
    pico8_jf_state_t *new_oc_r_ctx;
    pico8_gn_state_t *oc_state;
    pico8_jf_state_t *ir_state;

    for (id = one; id <= n; id = pico8_fixed_add(id, one)) {
        pico8_vec_t p_vec;
        pico8_vec_t *p_heap;
        pico8_vec_t *ra_heap;
        pico8_value_t nearest;
        char *id_num_str;
        char *id_str;
        pico8_ob_table_t *overrides;
        pico8_instance_t *made;

        /* cg.jo is empty only on the very first trial: every accepted
         * planet below self-registers into it, so later candidates really
         * are rejection-sampled against the ones already placed. Not an
         * inert loop over a list that stays empty. */
        do {
            pico8_fixed_t rx = pico8_fixed_add(pico8_fixed_rnd(two_seventy_five), eighth);
            pico8_fixed_t ry = pico8_fixed_add(pico8_fixed_rnd(two_seventy_five), eighth);
            p_vec = pico8_vec_v(rx, ry);
        } while (pico8_ij(p_vec, pico8_cg("jo"), &nearest) <= threshold);

        /* `p` aliases the instance's own field, `p*128` is a separate
         * fresh vector; neither is freed here, both outlive this loop
         * inside the instance built below. */
        p_heap = malloc(sizeof *p_heap);
        *p_heap = p_vec;
        ra_heap = malloc(sizeof *ra_heap);
        *ra_heap = pico8_vec_mul(p_vec, pico8_fixed_from_int(128));

        /* `id_str` is not freed here: the instance's "id" field keeps it
         * as-is. */
        id_num_str = pico8_value_tostr(pico8_value_fixed(id));
        id_str = pico8_bw_concat2("p", id_num_str);
        free(id_num_str);

        overrides = pico8_ob_table_new();
        pico8_ob_table_set_name(overrides, "id", pico8_value_string(id_str));
        pico8_ob_table_set_name(overrides, "p", pico8_value_ref(p_heap));
        pico8_ob_table_set_name(overrides, "ra", pico8_value_ref(ra_heap));

        /* Built for the side effect of self-registering into hv/cv/cg;
         * `overrides` is freed right after, its entries having been
         * shallow-copied into the instance's own fields. */
        /* TROU-7 */
        made = pico8_jo_new(overrides);
        (void)made;
        pico8_ob_table_free(overrides);
    }

    df = pico8_df();

    /* A NULL dk or an absent "en" reads as 0 -- this port's usual
     * absent-field default, not a guess at which difficulty is meant. */
    dk = pico8_dk();
    if (dk != NULL && pico8_ob_table_get_name(dk, "en", &en_val) && en_val.kind == PICO8_VALUE_FIXED) {
        dk_en = en_val.as.fixed;
    }

    hr_list = pico8_bw_positional_values(df);
    new_oc_r_ctx = pico8_jf(PICO8_FIXED_LIT(0x00008000, 0.5), dk_en);
    oc_state = pico8_gn(&hr_list, pico8_fixed_from_int(2), pico8_bw_gn_r, new_oc_r_ctx);
    pico8_list_free(&hr_list);

    /* Freed in lockstep with pico8_jo_set_oc releasing the gn state it was
     * paired with, so a repeated pico8_bw() neither leaks nor double-frees
     * the pair. */
    if (g_bw_oc_r_ctx != NULL) {
        pico8_jf_free(g_bw_oc_r_ctx);
    }
    g_bw_oc_r_ctx = new_oc_r_ctx;
    pico8_jo_set_oc(oc_state);

    /* pico8_jo_set_ir owns this outright: jo.ir has no paired ctx of its
     * own to track. */
    ir_state = pico8_jf(PICO8_FIXED_LIT(0x0000cccd, 0.8), PICO8_FIXED_LIT(0x00008000, 0.5));
    pico8_jo_set_ir(ir_state);
}

void pico8_galaxy_shutdown(void) {
    if (g_bw_oc_r_ctx != NULL) {
        pico8_jf_free(g_bw_oc_r_ctx);
        g_bw_oc_r_ctx = NULL;
    }
}
