#include "pico8_tech_gate.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_economy_formulas.h"
#include "pico8_entity.h"
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

/* `e_` of each entry of `pico8_nb_table`, by rank. */
static int g_nb_e_[PICO8_NB_COUNT];

int pico8_oj(pico8_value_t px) {
    const pico8_hy_t *fm = pico8_fm_hy();
    const pico8_ptrs_t *invented_list = NULL;

    if (!pico8_tech_gate_truthy(px)) {
        return 1;
    }
    if (px.kind != PICO8_VALUE_STRING) {
        return 0;
    }
    if (fm != NULL) {
        invented_list = fm->invented;
    }
    /* TROU-3 */
    return pico8_gu_content_ptrs(invented_list, px.as.string);
}

pico8_value_t pico8_ov(pico8_value_t px, pico8_value_t lk, pico8_value_t mc) {
    if (pico8_oj(px) && pico8_tech_gate_truthy(mc)) {
        return mc;
    }
    return lk;
}

/* `flr(od*max(fm.fu[nk], 0.55))`, an absent `fu[nk]` reading as 0. */
static pico8_fixed_t pico8_gi_cost(pico8_fixed_t od, int has_nk, pico8_fixed_t nk) {
    const pico8_hy_t *fm = pico8_fm_hy();
    pico8_fixed_t fu = pico8_fixed_from_int(0);
    pico8_fixed_t threshold = PICO8_FIXED_LIT(0x00008ccc, 0.55);
    pico8_fixed_t factor;

    if (has_nk && fm != NULL && fm->fu != NULL) {
        pico8_qe_map_get(fm->fu, nk, &fu);
    }

    factor = (fu > threshold) ? fu : threshold;
    return pico8_fixed_flr(pico8_fixed_mul(od, factor));
}

pico8_fixed_t pico8_gi_nb(const pico8_nb_entry_t *t) {
    if (t == NULL) {
        return pico8_gi_cost(pico8_fixed_from_int(0), 0, 0);
    }
    return pico8_gi_cost(pico8_fixed_from_int(t->od), 1, pico8_fixed_from_int(t->nk));
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
    const pico8_nb_entry_t *t;
    const pico8_hy_t *fm = pico8_fm_hy();
    const pico8_fm_placeholder_t *provisional = pico8_fm_placeholder();
    pico8_fixed_t jk = pico8_fixed_from_int(0);
    pico8_fixed_t ew = pico8_fixed_from_int(0);
    int affordable;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    t = e.as.ref;

    if (fm != NULL) {
        jk = fm->jk;
    } else if (provisional != NULL && provisional->has_jk) {
        jk = provisional->jk;
    }
    affordable = pico8_gi_nb(t) <= jk;
    g_nb_e_[t - pico8_nb_table] = affordable;

    if (fm != NULL) {
        ew = fm->ew;
    }
    if (pico8_fixed_from_int(t->lv) != ew) {
        return 0;
    }

    if (pico8_oj(pico8_value_string(t->px))) {
        return 0;
    }

    return affordable || pico8_tech_gate_truthy(ctx->bi);
}

pico8_list_t pico8_cb(pico8_value_t bi) {
    pico8_list_t items;
    pico8_cb_ctx_t ctx;
    pico8_list_t result;
    int i;

    pico8_list_init(&items);
    for (i = 0; i < PICO8_NB_COUNT; i++) {
        pico8_list_add(&items, pico8_value_ref((void *)&pico8_nb_table[i]));
    }
    ctx.bi = bi;
    result = pico8_jp(&items, pico8_cb_predicate, &ctx);
    pico8_list_free(&items);
    return result;
}

int pico8_nb_e_(const pico8_nb_entry_t *t) {
    assert(t >= pico8_nb_table && t < pico8_nb_table + PICO8_NB_COUNT && "e_ is kept for entries of nb only");
    return g_nb_e_[t - pico8_nb_table];
}

void pico8_nb_e_reset(void) {
    memset(g_nb_e_, 0, sizeof g_nb_e_);
}
