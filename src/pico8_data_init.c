#include "pico8_data_init.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_collect.h"
#include "pico8_research.h"
#include "pico8_stash.h"

static pico8_ob_table_t *g_df = NULL;
static pico8_ob_table_t *g_hc = NULL;
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

void pico8_data_init(pico8_mem_t *mem) {
    pico8_ob_table_t *df_mr;
    pico8_ob_table_t *nb_mr;
    pico8_list_t nb_gp;
    char *decoded;

    pico8_stash_init(mem);

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

    decoded = pico8_unstash(mem, 8383);
    pico8_ob_table_free(g_hc);
    g_hc = pico8_ob(decoded, NULL);
    free(decoded);

    decoded = pico8_unstash(mem, 8494);
    pico8_ob_table_free(g_fd);
    g_fd = pico8_ob(decoded, NULL);
    free(decoded);

    /* hp must be ready before build_gb builds one. */
    pico8_research_init();
    pico8_research_build_gb(g_fd);

    decoded = pico8_unstash(mem, 9807);
    pico8_ob_table_free(g_dr);
    g_dr = pico8_ob(decoded, NULL);
    free(decoded);

    decoded = pico8_unstash(mem, 10457);
    pico8_ob_table_free(g_nb);
    g_nb = pico8_ob(decoded, NULL);
    free(decoded);

    /* Unlike df's pass, `gp={"nk"}` has pico8_ho_apply route position 3
     * through r[] instead of storing it raw. */
    nb_mr = pico8_ob("\"px\",\"lv\",\"nk\",\"od\",\"oy\",", NULL);
    pico8_list_init(&nb_gp);
    pico8_list_add(&nb_gp, pico8_value_string("nk"));
    pico8_data_init_rename_pass(g_nb, nb_mr, &nb_gp);
    pico8_list_free(&nb_gp);
    pico8_ob_table_free(nb_mr);

    pico8_ob_table_free(g_ek);
    g_ek = pico8_ob("2,4,7,12,", NULL);
}

void pico8_data_shutdown(void) {
    pico8_research_shutdown();
    pico8_ob_table_free(g_df);
    g_df = NULL;
    pico8_ob_table_free(g_hc);
    g_hc = NULL;
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

const pico8_ob_table_t *pico8_hc(void) {
    return g_hc;
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
