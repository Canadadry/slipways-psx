#include "pico8_tech_gate.h"

#include <stddef.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_ob.h"
#include "pico8_probe.h"
#include "pico8_ref_audit.h"

/* Lua truthiness -- fixed 0 included, it is truthy -- extended to read
 * this port's absent-value filler as falsy. */
static int pico8_tech_gate_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* Returns a new list the caller owns and must release with
 * pico8_list_free(); a NULL table yields an empty one. */
static pico8_list_t pico8_tech_gate_positional_values(const pico8_ob_table_t *table) {
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

int pico8_oj(pico8_value_t px) {
    pico8_value_t invented;
    const pico8_list_t *invented_list = NULL;

    if (!pico8_tech_gate_truthy(px)) {
        return 1;
    }
    if (pico8_fm() != NULL && pico8_instance_get_field(pico8_fm(), "invented", &invented) &&
        invented.kind == PICO8_VALUE_REF) {
        /* pico8_hy_pq is the only writer of "invented", and it swaps the
         * spec string's `invented=o()` table for a real heap list. */
        invented_list = PICO8_REF_AS_LIST_CONST(invented.as.ref);
    }
    /* TROU-3 */
    return pico8_gu_content(invented_list, px);
}

pico8_value_t pico8_ov(pico8_value_t px, pico8_value_t lk, pico8_value_t mc) {
    if (pico8_oj(px) && pico8_tech_gate_truthy(mc)) {
        return mc;
    }
    return lk;
}

/* `t.nk` keys fm.fu either by index or by name, decided at runtime -- the
 * same split pico8_ft_index/pico8_ft_name already carry. */
pico8_fixed_t pico8_gi(const pico8_ob_table_t *t) {
    pico8_value_t od_val;
    pico8_value_t nk_val;
    pico8_value_t fu_val;
    pico8_fixed_t od = pico8_fixed_from_int(0);
    pico8_fixed_t fu = pico8_fixed_from_int(0);
    pico8_fixed_t threshold = pico8_fixed_from_double(0.55);
    pico8_fixed_t factor;

    if (t != NULL && pico8_ob_table_get_name(t, "od", &od_val) && od_val.kind == PICO8_VALUE_FIXED) {
        od = od_val.as.fixed;
    }

    if (t != NULL && pico8_ob_table_get_name(t, "nk", &nk_val) && pico8_fm() != NULL &&
        pico8_instance_get_field(pico8_fm(), "fu", &fu_val) && fu_val.kind == PICO8_VALUE_REF &&
        fu_val.as.ref != NULL) {
        const pico8_ob_table_t *fu_table = fu_val.as.ref;
        pico8_value_t v;
        int found = 0;

        if (nk_val.kind == PICO8_VALUE_FIXED) {
            found = pico8_ob_table_get_index(fu_table, pico8_fixed_flr_int(nk_val.as.fixed), &v);
        } else if (nk_val.kind == PICO8_VALUE_STRING && nk_val.as.string != NULL) {
            found = pico8_ob_table_get_name(fu_table, nk_val.as.string, &v);
        }
        if (found && v.kind == PICO8_VALUE_FIXED) {
            fu = v.as.fixed;
        }
    }

    factor = (fu > threshold) ? fu : threshold;
    return pico8_fixed_flr(pico8_fixed_mul(od, factor));
}

/* `bi`, the Lua closure's captured upvalue, threaded as an explicit ctx. */
typedef struct {
    pico8_value_t bi;
} pico8_cb_ctx_t;

/* The `e_` write below happens for every `t`, before any early return:
 * hoisting it under a keep/drop branch would change what callers read back
 * off nb. */
static int pico8_cb_predicate(pico8_value_t e, void *ctx_ptr) {
    pico8_cb_ctx_t *ctx = ctx_ptr;
    pico8_ob_table_t *t;
    pico8_value_t jk_val;
    pico8_value_t ew_val;
    pico8_value_t lv_val;
    pico8_value_t px_val;
    pico8_fixed_t jk = pico8_fixed_from_int(0);
    pico8_fixed_t ew = pico8_fixed_from_int(0);
    int affordable;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    t = e.as.ref;

    if (pico8_fm() != NULL && pico8_instance_get_field(pico8_fm(), "jk", &jk_val) &&
        jk_val.kind == PICO8_VALUE_FIXED) {
        jk = jk_val.as.fixed;
    }
    affordable = pico8_gi(t) <= jk;
    pico8_ob_table_set_name(t, "e_", pico8_value_bool(affordable));

    if (pico8_fm() != NULL && pico8_instance_get_field(pico8_fm(), "ew", &ew_val) &&
        ew_val.kind == PICO8_VALUE_FIXED) {
        ew = ew_val.as.fixed;
    }
    if (!(pico8_ob_table_get_name(t, "lv", &lv_val) && lv_val.kind == PICO8_VALUE_FIXED &&
          lv_val.as.fixed == ew)) {
        return 0;
    }

    if (!pico8_ob_table_get_name(t, "px", &px_val)) {
        px_val = pico8_value_ref(NULL);
    }
    if (pico8_oj(px_val)) {
        return 0;
    }

    return affordable || pico8_tech_gate_truthy(ctx->bi);
}

pico8_list_t pico8_cb(pico8_value_t bi) {
    pico8_list_t items = pico8_tech_gate_positional_values(pico8_nb());
    pico8_cb_ctx_t ctx;
    pico8_list_t result;

    ctx.bi = bi;
    result = pico8_jp(&items, pico8_cb_predicate, &ctx);
    pico8_list_free(&items);
    return result;
}
