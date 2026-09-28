#include "pico8_data_init.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_map_tables.h"
#include "pico8_research.h"

static pico8_ob_table_t *g_nb = NULL;

static pico8_ob_table_t *data_init_row_view(const pico8_jb_row_t *row, int lines);

/* Each view below is a fresh heap table, in the parsed table's entry order,
 * that the caller owns (top table only: nested ones are never freed). */

/* A nested row is a tuple of `lq:mn`, passed as the generated row itself,
 * or with `lines` a line of `jb`, passed as its own view. */
static void data_init_row_items(pico8_ob_table_t *t, const pico8_jb_row_t *row, int lines) {
    int i;

    for (i = 0; i < row->count; i++) {
        const pico8_jb_item_t *item = &row->items[i];
        pico8_value_t v;

        if (item->s != NULL) {
            v = pico8_value_string(item->s);
        } else if (item->row != NULL && lines) {
            v = pico8_value_ref(data_init_row_view(item->row, 1));
        } else if (item->row != NULL) {
            v = pico8_value_ref((void *)item->row);
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

static pico8_ob_table_t *data_init_row_view(const pico8_jb_row_t *row, int lines) {
    pico8_ob_table_t *t = pico8_ob_table_new();

    data_init_row_items(t, row, lines);
    data_init_row_named(t, row);
    return t;
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
        pico8_value_t oy = pico8_value_ref(data_init_row_view(e->oy, 1));

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

void pico8_data_init(void) {
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
