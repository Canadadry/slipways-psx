#include "pico8_data_init.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_map_tables.h"
#include "pico8_research.h"
#include "pico8_vec.h"

static pico8_ob_table_t *g_nb = NULL;

/* Each view below is a fresh heap table, in the parsed table's entry order,
 * that the caller owns (top table only: nested ones are never freed). */

static void data_init_row_items(pico8_ob_table_t *t, const pico8_jb_row_t *row) {
    int i;

    for (i = 0; i < row->count; i++) {
        const pico8_jb_item_t *item = &row->items[i];
        pico8_value_t v;

        if (item->s != NULL) {
            v = pico8_value_string(item->s);
        } else if (item->row != NULL) {
            v = pico8_value_ref(pico8_jb_row_view(item->row));
        } else if (item->is_false) {
            v = pico8_value_bool(0);
        } else if (item->is_fixed) {
            v = pico8_value_fixed(item->fixed);
        } else {
            v = pico8_value_fixed(pico8_fixed_from_int(item->n));
        }
        pico8_ob_table_set_index(t, i + 1, v);
    }
}

static void data_init_row_named(pico8_ob_table_t *t, const pico8_jb_row_t *row) {
    if (row->has_c) {
        pico8_ob_table_set_name(t, "c", pico8_value_fixed(pico8_fixed_from_int(row->c)));
    }
    if (row->fn != NULL) {
        pico8_ob_table_set_name(t, "fn", pico8_value_string(row->fn));
    }
}

pico8_ob_table_t *pico8_jb_row_view(const pico8_jb_row_t *row) {
    pico8_ob_table_t *t = pico8_ob_table_new();

    data_init_row_items(t, row);
    data_init_row_named(t, row);
    return t;
}

/* A fresh heap vector per call, as the v() constructor allocated one per
 * parse; never freed. */
static pico8_value_t data_init_vec(int x, int y) {
    pico8_vec_t *v = malloc(sizeof *v);

    *v = pico8_vec_v(pico8_fixed_from_int(x), pico8_fixed_from_int(y));
    return pico8_value_ref(v);
}

/* A record sits both at its position and under its px name, one shared
 * table, as nb_mr leaves it: tech_gate's write to e_ shows through both. */
static pico8_ob_table_t *data_init_nb_view(void) {
    pico8_ob_table_t *nb = pico8_ob_table_new();
    pico8_ob_table_t *records[PICO8_NB_COUNT];
    int i;

    for (i = 0; i < PICO8_NB_COUNT; i++) {
        const pico8_nb_entry_t *e = &pico8_nb_table[i];
        pico8_ob_table_t *ri = pico8_ob_table_new();
        pico8_value_t oy = pico8_value_ref(pico8_jb_row_view(e->oy));

        pico8_ob_table_set_index(ri, 1, pico8_value_string(e->px));
        pico8_ob_table_set_index(ri, 2, pico8_value_fixed(pico8_fixed_from_int(e->lv)));
        pico8_ob_table_set_index(ri, 3, pico8_value_string(e->nk_key));
        pico8_ob_table_set_index(ri, 4, pico8_value_fixed(pico8_fixed_from_int(e->od)));
        pico8_ob_table_set_index(ri, 5, oy);
        pico8_ob_table_set_name(ri, "px", pico8_value_string(e->px));
        pico8_ob_table_set_name(ri, "lv", pico8_value_fixed(pico8_fixed_from_int(e->lv)));
        pico8_ob_table_set_name(ri, "nk", pico8_value_fixed(pico8_fixed_from_int(e->nk)));
        pico8_ob_table_set_name(ri, "od", pico8_value_fixed(pico8_fixed_from_int(e->od)));
        pico8_ob_table_set_name(ri, "oy", oy);
        records[i] = ri;
        pico8_ob_table_set_index(nb, i + 1, pico8_value_ref(ri));
    }
    for (i = 0; i < PICO8_NB_COUNT; i++) {
        pico8_ob_table_set_name(nb, pico8_nb_table[i].px, pico8_value_ref(records[i]));
    }
    return nb;
}

pico8_ob_table_t *pico8_ix_view(void) {
    pico8_ob_table_t *ix = pico8_ob_table_new();
    int i;

    for (i = 0; i < PICO8_IX_COUNT; i++) {
        const pico8_ix_entry_t *e = &pico8_ix_table[i];
        pico8_ob_table_t *ri = pico8_ob_table_new();

        pico8_ob_table_set_name(ri, "ra", data_init_vec(e->ra_x, e->ra_y));
        pico8_ob_table_set_name(ri, "lq", pico8_value_ref(pico8_jb_row_view(e->lq)));
        pico8_ob_table_set_index(ix, i + 1, pico8_value_ref(ri));
    }
    return ix;
}

pico8_ob_table_t *pico8_ha_view(void) {
    pico8_ob_table_t *ha = pico8_ob_table_new();
    int i;

    for (i = 0; i < PICO8_HA_COUNT; i++) {
        const pico8_ha_entry_t *e = &pico8_ha_table[i];
        pico8_ob_table_t *ri = pico8_ob_table_new();

        pico8_ob_table_set_name(ri, "lq", pico8_value_ref(pico8_jb_row_view(e->lq)));
        if (e->has_fg) {
            pico8_ob_table_set_name(ri, "fg", pico8_value_fixed(pico8_fixed_from_int(e->fg)));
        }
        pico8_ob_table_set_index(ha, i + 1, pico8_value_ref(ri));
    }
    pico8_ob_table_set_name(ha, "hi", data_init_vec(pico8_ha_tail.hi_x, pico8_ha_tail.hi_y));
    pico8_ob_table_set_name(ha, "gq", data_init_vec(pico8_ha_tail.gq_x, pico8_ha_tail.gq_y));
    pico8_ob_table_set_name(ha, "ca", pico8_value_fixed(pico8_fixed_from_int(pico8_ha_tail.ca)));
    return ha;
}

pico8_ob_table_t *pico8_bo_view(void) {
    pico8_ob_table_t *bo = pico8_ob_table_new();
    int i;

    for (i = 0; i < PICO8_BO_COUNT; i++) {
        const pico8_bo_entry_t *e = &pico8_bo_table[i];
        pico8_ob_table_t *ri = pico8_ob_table_new();

        pico8_ob_table_set_name(ri, "lq", pico8_value_ref(pico8_jb_row_view(e->lq)));
        pico8_ob_table_set_name(ri, "px", pico8_value_string(e->px));
        pico8_ob_table_set_name(ri, "trade", pico8_value_fixed(pico8_fixed_from_int(e->trade)));
        pico8_ob_table_set_name(ri, "en", pico8_value_fixed(e->en));
        pico8_ob_table_set_index(bo, i + 1, pico8_value_ref(ri));
    }
    return bo;
}

pico8_ob_table_t *pico8_bz_view(void) {
    pico8_ob_table_t *bz = pico8_ob_table_new();
    int i;

    for (i = 0; i < PICO8_BZ_COUNT; i++) {
        const pico8_bz_entry_t *e = &pico8_bz_table[i];
        pico8_ob_table_t *ri = pico8_ob_table_new();

        data_init_row_items(ri, &e->row);
        if (e->fn != NULL) {
            pico8_ob_table_set_name(ri, "fn", pico8_value_string(e->fn));
        }
        if (e->has_score) {
            pico8_ob_table_set_name(ri, "score", pico8_value_fixed(pico8_fixed_from_int(e->score)));
        }
        if (e->has_ra) {
            pico8_ob_table_set_name(ri, "ra", data_init_vec(e->ra_x, e->ra_y));
        }
        data_init_row_named(ri, &e->row);
        pico8_ob_table_set_index(bz, i + 1, pico8_value_ref(ri));
    }
    return bz;
}

void pico8_data_init(pico8_mem_t *mem) {
    (void)mem;

    /* hp must be ready before build_gb builds one. */
    pico8_research_init();
    pico8_research_build_gb();

    pico8_ob_table_free(g_nb);
    g_nb = data_init_nb_view();
}

void pico8_data_shutdown(void) {
    pico8_research_shutdown();
    pico8_ob_table_free(g_nb);
    g_nb = NULL;
}

const pico8_ob_table_t *pico8_nb(void) {
    return g_nb;
}
