#include "pico8_data_init.h"

#include <stddef.h>

#include "pico8_collect.h"
#include "pico8_fixed.h"
#include "pico8_map_tables.h"
#include "pico8_research.h"

static pico8_ob_table_t *g_df = NULL;
static pico8_ob_table_t *g_fd = NULL;
static pico8_ob_table_t *g_dr = NULL;
static pico8_ob_table_t *g_nb = NULL;
static pico8_ob_table_t *g_ek = NULL;

/* Returns a new list the caller owns and must release with
 * pico8_list_free(); a NULL table yields an empty one. */
static pico8_list_t pico8_data_init_positional_values(const pico8_ob_table_t *table) {
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

/* `rc` is both the table walked and the table renamed into: pico8_ho_apply
 * appends a named entry per record as it goes. Hence the snapshot of the
 * positional entries first -- iterating `rc` directly would walk entries
 * the pass itself is adding. */
static void pico8_data_init_rename_pass(pico8_ob_table_t *rc, const pico8_ob_table_t *mr, const pico8_list_t *gp) {
    pico8_list_t items = pico8_data_init_positional_values(rc);
    pico8_list_iter_t it = pico8_list_all(&items);
    pico8_value_t entry;

    while (pico8_list_iter_next(&it, &entry)) {
        if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
            pico8_ho_apply(rc, mr, gp, (pico8_ob_table_t *)entry.as.ref);
        }
    }
    pico8_list_free(&items);
}

/* Each view below is a fresh heap table, in the parsed table's entry order,
 * that the caller owns (top table only: nested ones are never freed). */
static pico8_ob_table_t *data_init_row_view(const pico8_jb_row_t *row) {
    pico8_ob_table_t *t = pico8_ob_table_new();
    int i;

    for (i = 0; i < row->count; i++) {
        const pico8_jb_item_t *item = &row->items[i];
        pico8_value_t v;

        if (item->s != NULL) {
            v = pico8_value_string(item->s);
        } else if (item->row != NULL) {
            v = pico8_value_ref(data_init_row_view(item->row));
        } else {
            v = pico8_value_fixed(pico8_fixed_from_int(item->n));
        }
        pico8_ob_table_set_index(t, i + 1, v);
    }
    if (row->has_c) {
        pico8_ob_table_set_name(t, "c", pico8_value_fixed(pico8_fixed_from_int(row->c)));
    }
    return t;
}

static pico8_ob_table_t *data_init_fd_view(void) {
    pico8_ob_table_t *fd = pico8_ob_table_new();
    int i;

    for (i = 0; i < PICO8_FD_COUNT; i++) {
        const pico8_fd_entry_t *e = &pico8_fd_table[i];
        const char *const strings[8] = {e->px, e->lr, NULL, e->ja, e->i, e->o, e->il, e->dm};
        pico8_ob_table_t *ri;
        int p;

        if (e->header != NULL) {
            pico8_ob_table_set_index(fd, i + 1, pico8_value_string(e->header));
            continue;
        }
        ri = pico8_ob_table_new();
        for (p = 0; p < e->npos; p++) {
            pico8_value_t v;

            if (p == 2) {
                v = pico8_value_fixed(pico8_fixed_from_int(e->fj));
            } else if (p == 6 && e->il_false) {
                v = pico8_value_bool(0);
            } else {
                v = pico8_value_string(strings[p]);
            }
            pico8_ob_table_set_index(ri, p + 1, v);
        }
        if (e->has_ii) {
            pico8_ob_table_set_name(ri, "ii", pico8_value_fixed(pico8_fixed_from_int(e->ii)));
        }
        if (e->go != NULL) {
            pico8_ob_table_set_name(ri, "go", pico8_value_string(e->go));
        }
        if (e->cw != NULL) {
            pico8_ob_table_set_name(ri, "cw", pico8_value_string(e->cw));
        }
        if (e->has_rn) {
            pico8_ob_table_set_name(ri, "rn", pico8_value_fixed(pico8_fixed_from_int(e->rn)));
        }
        pico8_ob_table_set_index(fd, i + 1, pico8_value_ref(ri));
    }
    return fd;
}

static pico8_ob_table_t *data_init_dr_view(void) {
    pico8_ob_table_t *dr = pico8_ob_table_new();
    int i;

    for (i = 0; i < PICO8_DR_COUNT; i++) {
        pico8_ob_table_set_index(dr, i + 1, pico8_value_ref(data_init_row_view(&pico8_dr_table[i])));
    }
    return dr;
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
        pico8_value_t oy = pico8_value_ref(data_init_row_view(e->oy));

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

void pico8_data_init(pico8_mem_t *mem) {
    pico8_ob_table_t *df_mr;

    (void)mem;

    pico8_ob_table_free(g_df);
    /* Every spec string here closes on a comma (trailing newline aside):
     * pico8_ob only ends a field on one, so dropping the last comma loses
     * the last record without a word. */
    g_df = pico8_ob("\n"
                     "o(\"ll\",45,0,me=\"ll\"),\n"
                     "o(\"e\",160,4.5,\"earth-like\"),\n"
                     "o(\"f\",128,6,\"forgeworld\"),\n"
                     "o(\"m\",134,4,\"mineral\"),\n"
                     "o(\"o\",130,3,\"ocean\"),\n"
                     "o(\"r\",140,2.5,\"remnant\"),\n"
                     "o(\"x\",162,3,\"xeno\"),\n"
                     "o(\"j\",136,2,\"jungle\"),\n"
                     "o(\"i\",138,2,\"iceball\"),\n"
                     "o(\"s\",142,2,\"barren\"),\n"
                     "o(\"g\",96,4,\"gas giant\"),\n",
                     NULL);
    /* No `gp`: none of df's fields is routed through r[]. */
    df_mr = pico8_ob("\n\"px\",\"s\",\"r\",\"d\",\n", NULL);
    pico8_data_init_rename_pass(g_df, df_mr, NULL);
    pico8_ob_table_free(df_mr);

    pico8_ob_table_free(g_fd);
    g_fd = data_init_fd_view();

    /* hp must be ready before build_gb builds one. */
    pico8_research_init();
    pico8_research_build_gb(g_fd);

    pico8_ob_table_free(g_dr);
    g_dr = data_init_dr_view();

    pico8_ob_table_free(g_nb);
    g_nb = data_init_nb_view();

    pico8_ob_table_free(g_ek);
    g_ek = pico8_ob("2,4,7,12,", NULL);
}

void pico8_data_shutdown(void) {
    pico8_research_shutdown();
    pico8_ob_table_free(g_df);
    g_df = NULL;
    pico8_ob_table_free(g_fd);
    g_fd = NULL;
    pico8_ob_table_free(g_dr);
    g_dr = NULL;
    pico8_ob_table_free(g_nb);
    g_nb = NULL;
    pico8_ob_table_free(g_ek);
    g_ek = NULL;
}

const pico8_ob_table_t *pico8_df(void) {
    return g_df;
}

const pico8_ob_table_t *pico8_fd(void) {
    return g_fd;
}

const pico8_ob_table_t *pico8_dr(void) {
    return g_dr;
}

const pico8_ob_table_t *pico8_nb(void) {
    return g_nb;
}

const pico8_ob_table_t *pico8_ek(void) {
    return g_ek;
}
