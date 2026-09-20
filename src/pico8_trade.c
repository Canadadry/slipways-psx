#include "pico8_trade.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_collect.h"
#include "pico8_list.h"

/* See pico8_trade.h's own top comment for trade's full real Lua source
 * and design trace. */
static pico8_class_t *g_trade_class = NULL;

/* trade=kv:jr() -- root class, no spec string, no `pq` (never assigned
 * anywhere in the real source, unlike ke's `ke.pq=lt`). */
void pico8_trade_init(void) {
    pico8_trade_shutdown();
    g_trade_class = pico8_class_declare(NULL, NULL);
}

void pico8_trade_shutdown(void) {
    if (g_trade_class != NULL) {
        pico8_class_free(g_trade_class);
        g_trade_class = NULL;
    }
}

pico8_class_t *pico8_trade_class(void) {
    return g_trade_class;
}

pico8_instance_t *pico8_trade_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_trade_class, overrides);
}

/* See pico8_trade.h's own comment on this declaration. */
int pico8_trade_to(const pico8_instance_t *self, pico8_value_t *out_value) {
    pico8_value_t lp_val;
    const pico8_list_t *lp;

    if (self == NULL || !pico8_instance_get_field(self, "lp", &lp_val) || lp_val.kind != PICO8_VALUE_REF ||
        lp_val.as.ref == NULL) {
        return 0;
    }
    lp = (const pico8_list_t *)lp_val.as.ref;
    if (lp->count == 0) {
        return 0;
    }
    *out_value = lp->items[lp->count - 1];
    return 1;
}

/* See pico8_trade.h's own comment on this declaration. `{np}`/`{jc}` (the
 * real Lua's own single-element list literals) are built here as
 * one-element pico8_list_t stack values (np_list/jc_list) fed to pico8_la
 * (pico8_collect.h), then freed once pico8_la has copied their single
 * element into the new heap-allocated lp/rl lists it returns. */
pico8_instance_t *pico8_trade_kg(const pico8_instance_t *self, pico8_value_t np, pico8_value_t jc) {
    pico8_value_t lp_val, rl_val, qe_val;
    pico8_list_t empty_lp;
    pico8_list_t empty_rl;
    const pico8_list_t *lp;
    const pico8_list_t *rl;
    pico8_list_t np_list;
    pico8_list_t jc_list;
    pico8_list_t *new_lp;
    pico8_list_t *new_rl;
    pico8_ob_table_t *overrides;
    pico8_instance_t *result;

    if (self == NULL) {
        return NULL;
    }

    pico8_list_init(&empty_lp);
    pico8_list_init(&empty_rl);
    lp = (pico8_instance_get_field(self, "lp", &lp_val) && lp_val.kind == PICO8_VALUE_REF && lp_val.as.ref != NULL)
             ? (const pico8_list_t *)lp_val.as.ref
             : &empty_lp;
    rl = (pico8_instance_get_field(self, "rl", &rl_val) && rl_val.kind == PICO8_VALUE_REF && rl_val.as.ref != NULL)
             ? (const pico8_list_t *)rl_val.as.ref
             : &empty_rl;
    if (!pico8_instance_get_field(self, "qe", &qe_val)) {
        qe_val = pico8_value_bool(0);
    }

    pico8_list_init(&np_list);
    pico8_list_add(&np_list, np);
    pico8_list_init(&jc_list);
    pico8_list_add(&jc_list, jc);

    new_lp = malloc(sizeof *new_lp);
    *new_lp = pico8_la(lp, &np_list);
    new_rl = malloc(sizeof *new_rl);
    *new_rl = pico8_la(rl, &jc_list);

    pico8_list_free(&np_list);
    pico8_list_free(&jc_list);

    overrides = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides, "lp", pico8_value_ref(new_lp));
    pico8_ob_table_set_name(overrides, "rl", pico8_value_ref(new_rl));
    pico8_ob_table_set_name(overrides, "qe", qe_val);

    result = pico8_trade_new(overrides);
    pico8_ob_table_free(overrides);
    return result;
}

/* See pico8_trade.h's own comment on this declaration. */
int pico8_trade_eq(const pico8_instance_t *l, const pico8_instance_t *r) {
    pico8_value_t l_lp_val, r_lp_val;
    const pico8_list_t *l_lp, *r_lp;
    pico8_value_t l_first, r_first;
    pico8_value_t l_last, r_last;

    if (l == NULL || r == NULL) {
        return 0;
    }
    if (!pico8_instance_get_field(l, "lp", &l_lp_val) || l_lp_val.kind != PICO8_VALUE_REF ||
        l_lp_val.as.ref == NULL || !pico8_instance_get_field(r, "lp", &r_lp_val) ||
        r_lp_val.kind != PICO8_VALUE_REF || r_lp_val.as.ref == NULL) {
        return 0;
    }
    l_lp = (const pico8_list_t *)l_lp_val.as.ref;
    r_lp = (const pico8_list_t *)r_lp_val.as.ref;
    if (l_lp->count == 0 || r_lp->count == 0) {
        return 0;
    }
    l_first = l_lp->items[0];
    r_first = r_lp->items[0];

    if (!pico8_value_equal(l_first, r_first)) {
        return 0;
    }
    if (!pico8_trade_to(l, &l_last) || !pico8_trade_to(r, &r_last)) {
        return 0;
    }
    return pico8_value_equal(l_last, r_last);
}
