#include "game/tech_gate.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/collect.h"
#include "game/economy_formulas.h"
#include "game/main_economy.h"

/* `e_` of each entry of `pico8_nb_table`, by rank. */
static int g_nb_e_[TECHNOLOGIES_COUNT];

int pico8_oj(const char *px) {
    const pico8_hy_t *fm = main_economy_typed();
    const pointer_list_t *invented_list = NULL;

    if (px == NULL) {
        return 1;
    }
    if (fm != NULL) {
        invented_list = fm->invented;
    }
    /* TROU-3 */
    return collect_contains_string(invented_list, px);
}

p8_fixed_t pico8_ov(const char *px, p8_fixed_t lk, p8_fixed_t mc) {
    if (pico8_oj(px)) {
        return mc;
    }
    return lk;
}

/* `flr(od*max(fm.fu[nk], 0.55))`, an absent `fu[nk]` reading as 0. */
static p8_fixed_t pico8_gi_cost(p8_fixed_t od, int has_nk, p8_fixed_t nk) {
    const pico8_hy_t *fm = main_economy_typed();
    p8_fixed_t fu = p8_fixed_from_int(0);
    p8_fixed_t threshold = P8_FIXED_LIT(0x00008ccc, 0.55);
    p8_fixed_t factor;

    if (has_nk && fm != NULL && fm->fu != NULL) {
        pico8_qe_map_get(fm->fu, nk, &fu);
    }

    factor = (fu > threshold) ? fu : threshold;
    return p8_fixed_flr(p8_fixed_mul(od, factor));
}

p8_fixed_t pico8_gi_nb(const technologies_entry_t *t) {
    if (t == NULL) {
        return pico8_gi_cost(p8_fixed_from_int(0), 0, 0);
    }
    return pico8_gi_cost(p8_fixed_from_int(t->cost), 1, p8_fixed_from_int(t->icon));
}

/* The `e_` write below happens for every `t`, before any early return:
 * hoisting it under a keep/drop branch would change what callers read back
 * off nb. */
static int pico8_cb_predicate(const technologies_entry_t *t, int bi) {
    const pico8_hy_t *fm = main_economy_typed();
    const pico8_fm_placeholder_t *provisional = main_economy_placeholder();
    p8_fixed_t jk = p8_fixed_from_int(0);
    p8_fixed_t ew = p8_fixed_from_int(0);
    int affordable;

    if (fm != NULL) {
        jk = fm->jk;
    } else if (provisional != NULL && provisional->has_jk) {
        jk = provisional->jk;
    }
    affordable = pico8_gi_nb(t) <= jk;
    g_nb_e_[t - technologies_table] = affordable;

    if (fm != NULL) {
        ew = fm->ew;
    }
    if (p8_fixed_from_int(t->era) != ew) {
        return 0;
    }

    if (pico8_oj(t->name)) {
        return 0;
    }

    return affordable || bi;
}

pico8_cb_kept_t pico8_cb(int bi) {
    pico8_cb_kept_t result;
    int i;

    result.count = 0;
    for (i = 0; i < TECHNOLOGIES_COUNT; i++) {
        if (pico8_cb_predicate(&technologies_table[i], bi)) {
            assert(result.count < TECHNOLOGIES_COUNT && "cb keeps entries of nb only: #nb bounds the result");
            result.items[result.count++] = &technologies_table[i];
        }
    }
    return result;
}

int pico8_nb_e_(const technologies_entry_t *t) {
    assert(t >= technologies_table && t < technologies_table + TECHNOLOGIES_COUNT && "e_ is kept for entries of nb only");
    return g_nb_e_[t - technologies_table];
}

void pico8_nb_e_reset(void) {
    memset(g_nb_e_, 0, sizeof g_nb_e_);
}
