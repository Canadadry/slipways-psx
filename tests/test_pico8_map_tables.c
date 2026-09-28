#include "../src/pico8_map_tables.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_collect.h"
#include "../src/pico8_data_init.h"
#include "../src/pico8_economy_formulas.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_game_data.h"
#include "../src/pico8_gen.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_pi.h"
#include "../src/pico8_stash.h"
#include "../src/pico8_vec.h"

/* The table ob() parses from the string the cart parks at map address pd:
 * the reference every generated table is compared against. */
static pico8_ob_table_t *decode(const pico8_mem_t *mem, int pd) {
    char *s = pico8_unstash(mem, pd);
    pico8_ob_table_t *t = pico8_ob(s, NULL);

    free(s);
    return t;
}

static pico8_ob_table_t *parse(const char *s) {
    return pico8_ob(s, NULL);
}

static int tables_equal(const pico8_ob_table_t *a, const pico8_ob_table_t *b, int count, const char *path);

/* v(x, y) stores a heap vector, not a table, under these names only. */
static int is_vec_name(const char *name) {
    return strcmp(name, "ra") == 0 || strcmp(name, "hi") == 0 || strcmp(name, "gq") == 0;
}

static int vecs_equal(pico8_value_t a, pico8_value_t b, const char *path) {
    const pico8_vec_t *va;
    const pico8_vec_t *vb;

    if (a.kind != PICO8_VALUE_REF || b.kind != PICO8_VALUE_REF || a.as.ref == NULL || b.as.ref == NULL) {
        fprintf(stderr, "%s: not two vectors\n", path);
        return 0;
    }
    va = a.as.ref;
    vb = b.as.ref;
    if (va->x != vb->x || va->y != vb->y) {
        fprintf(stderr, "%s: vector (0x%x, 0x%x) != (0x%x, 0x%x)\n", path, (unsigned)va->x, (unsigned)va->y,
                (unsigned)vb->x, (unsigned)vb->y);
        return 0;
    }
    return 1;
}

static int values_equal(pico8_value_t a, pico8_value_t b, const char *path) {
    if (a.kind != b.kind) {
        fprintf(stderr, "%s: kind %d != %d\n", path, a.kind, b.kind);
        return 0;
    }
    switch (a.kind) {
    case PICO8_VALUE_FIXED:
        if (a.as.fixed != b.as.fixed) {
            fprintf(stderr, "%s: fixed 0x%x != 0x%x\n", path, (unsigned)a.as.fixed, (unsigned)b.as.fixed);
            return 0;
        }
        return 1;
    case PICO8_VALUE_STRING:
        if (strcmp(a.as.string, b.as.string) != 0) {
            fprintf(stderr, "%s: \"%s\" != \"%s\"\n", path, a.as.string, b.as.string);
            return 0;
        }
        return 1;
    case PICO8_VALUE_BOOL:
        if (a.as.boolean != b.as.boolean) {
            fprintf(stderr, "%s: bool %d != %d\n", path, a.as.boolean, b.as.boolean);
            return 0;
        }
        return 1;
    case PICO8_VALUE_REF:
        if (a.as.ref == NULL || b.as.ref == NULL) {
            return a.as.ref == b.as.ref;
        }
        if (((const pico8_ob_table_t *)a.as.ref)->count != ((const pico8_ob_table_t *)b.as.ref)->count) {
            fprintf(stderr, "%s: table sizes differ\n", path);
            return 0;
        }
        return tables_equal(a.as.ref, b.as.ref, ((const pico8_ob_table_t *)b.as.ref)->count, path);
    default:
        fprintf(stderr, "%s: unexpected kind %d\n", path, a.kind);
        return 0;
    }
}

/* Compares a's first `count` entries with b's (b having exactly `count`, a
 * at least as many), entry by entry and in order: key kind, index or name,
 * then value. */
static int tables_equal(const pico8_ob_table_t *a, const pico8_ob_table_t *b, int count, const char *path) {
    int i;

    if (b->count != count || a->count < count) {
        fprintf(stderr, "%s: count %d != %d\n", path, a->count, b->count);
        return 0;
    }
    for (i = 0; i < count; i++) {
        const pico8_ob_key_t *ka = &a->entries[i].key;
        const pico8_ob_key_t *kb = &b->entries[i].key;
        char sub[256];

        snprintf(sub, sizeof sub, "%s[%d]", path, i + 1);
        if (ka->kind != kb->kind || (ka->kind == PICO8_OB_KEY_INDEX && ka->index != kb->index) ||
            (ka->kind == PICO8_OB_KEY_NAME && strcmp(ka->name, kb->name) != 0)) {
            fprintf(stderr, "%s: key differs\n", sub);
            return 0;
        }
        if (ka->kind == PICO8_OB_KEY_NAME && is_vec_name(ka->name)) {
            if (!vecs_equal(a->entries[i].value, b->entries[i].value, sub)) {
                return 0;
            }
            continue;
        }
        if (!values_equal(a->entries[i].value, b->entries[i].value, sub)) {
            return 0;
        }
    }
    return 1;
}

static void test_r_matches_the_decoded_table_in_order(const pico8_mem_t *mem) {
    pico8_ob_table_t *ref = decode(mem, 8192);
    int qe = -1;
    int i;

    assert(ref->count == PICO8_R_COUNT);
    for (i = 0; i < PICO8_R_COUNT; i++) {
        const pico8_ob_entry_t *e = &ref->entries[i];
        int found = -1;

        assert(e->key.kind == PICO8_OB_KEY_NAME);
        assert(strcmp(e->key.name, pico8_r_table[i].key) == 0);
        assert(e->value.kind == PICO8_VALUE_FIXED);
        assert(e->value.as.fixed == pico8_fixed_from_int(pico8_r_table[i].qe));
        assert(pico8_r_lookup(e->key.name, &found));
        assert(found == pico8_r_table[i].qe);
    }
    assert(!pico8_r_lookup("zz", &qe));
    assert(qe == -1);
}

static void test_iz_matches_the_decoded_table_in_order(const pico8_mem_t *mem) {
    pico8_ob_table_t *ref = decode(mem, 8259);
    int i;

    assert(ref->count == PICO8_IZ_COUNT);
    for (i = 0; i < PICO8_IZ_COUNT; i++) {
        const pico8_ob_entry_t *e = &ref->entries[i];

        assert(e->key.kind == PICO8_OB_KEY_NAME);
        assert(strcmp(e->key.name, pico8_iz_table[i].key) == 0);
        assert(e->value.kind == PICO8_VALUE_STRING);
        assert(strcmp(e->value.as.string, pico8_iz_table[i].name) == 0);
        assert(pico8_iz_lookup(e->key.name) == pico8_iz_table[i].name);
    }
    assert(pico8_iz_lookup("zz") == NULL);
}

static void test_hc_matches_the_decoded_table_in_order(const pico8_mem_t *mem) {
    pico8_ob_table_t *ref = decode(mem, 8383);
    int i;

    assert(ref->count == PICO8_HC_COUNT);
    for (i = 0; i < PICO8_HC_COUNT; i++) {
        const pico8_hc_entry_t *hc = &pico8_hc_table[i];
        const pico8_ob_entry_t *e = &ref->entries[i];
        const pico8_ob_table_t *values;
        int j;

        assert(e->key.kind == PICO8_OB_KEY_NAME);
        assert(strcmp(e->key.name, hc->key) == 0);
        assert(e->value.kind == PICO8_VALUE_REF);
        values = e->value.as.ref;
        assert(values->count == hc->count + (hc->mc != NULL));
        for (j = 0; j < hc->count; j++) {
            assert(values->entries[j].key.kind == PICO8_OB_KEY_INDEX);
            assert(values->entries[j].key.index == j + 1);
            assert(values->entries[j].value.kind == PICO8_VALUE_FIXED);
            assert(values->entries[j].value.as.fixed == pico8_fixed_from_int(hc->values[j]));
        }
        if (hc->mc != NULL) {
            assert(values->entries[hc->count].key.kind == PICO8_OB_KEY_NAME);
            assert(strcmp(values->entries[hc->count].key.name, "mc") == 0);
            assert(values->entries[hc->count].value.kind == PICO8_VALUE_STRING);
            assert(strcmp(values->entries[hc->count].value.as.string, hc->mc) == 0);
        }
        assert(pico8_hc_lookup(hc->key) == hc);
    }
    assert(pico8_hc_lookup("zz") == NULL);
}

static void test_dr_view_matches_the_decoded_table(const pico8_mem_t *mem) {
    pico8_ob_table_t *ref = decode(mem, 9807);

    assert(ref->count == PICO8_DR_COUNT);
    assert(pico8_dr()->count == ref->count);
    assert(tables_equal(pico8_dr(), ref, ref->count, "dr"));
}

/* The cart's nb_mr pass, redone on the decoded table: each record gains px,
 * lv, nk (routed through r), od and oy by name, and nb gains px -> record. */
static pico8_ob_table_t *nb_reference(const pico8_mem_t *mem) {
    static const char *const mr_items[] = {"px", "lv", "nk", "od", "oy"};
    static const char *const gp_items[] = {"nk"};
    const pico8_str_seq_t mr = {5, mr_items};
    const pico8_str_seq_t gp = {1, gp_items};
    pico8_ob_table_t *nb = decode(mem, 10457);
    pico8_ob_table_t *records[PICO8_NB_COUNT];
    int i;

    assert(nb->count == PICO8_NB_COUNT);
    for (i = 0; i < PICO8_NB_COUNT; i++) {
        assert(nb->entries[i].value.kind == PICO8_VALUE_REF);
        records[i] = nb->entries[i].value.as.ref;
    }
    for (i = 0; i < PICO8_NB_COUNT; i++) {
        pico8_ho_apply(nb, &mr, &gp, records[i]);
    }
    return nb;
}

static void test_nb_view_matches_the_decoded_table_after_its_pass(const pico8_mem_t *mem) {
    pico8_ob_table_t *ref = nb_reference(mem);
    const pico8_ob_table_t *nb = pico8_nb();
    int i;

    assert(ref->count == 2 * PICO8_NB_COUNT);
    assert(nb->count == ref->count);
    assert(tables_equal(nb, ref, ref->count, "nb"));
    for (i = 0; i < PICO8_NB_COUNT; i++) {
        const pico8_ob_table_t *record = nb->entries[i].value.as.ref;

        assert(nb->entries[PICO8_NB_COUNT + i].value.as.ref == record);
        assert(record->count == 10);
        assert(strcmp(record->entries[9].key.name, "oy") == 0);
        assert(record->entries[9].value.as.ref == record->entries[4].value.as.ref);
    }
}

static void test_fd_view_matches_the_decoded_table_plus_rr(const pico8_mem_t *mem) {
    pico8_ob_table_t *ref = decode(mem, 8494);
    const pico8_ob_table_t *fd = pico8_fd();
    const char *rr = NULL;
    int records = 0;
    int i;

    assert(ref->count == PICO8_FD_COUNT);
    assert(fd->count == ref->count);
    for (i = 0; i < ref->count; i++) {
        pico8_value_t want = ref->entries[i].value;
        pico8_value_t got = fd->entries[i].value;
        char path[32];

        assert(fd->entries[i].key.kind == PICO8_OB_KEY_INDEX && fd->entries[i].key.index == i + 1);
        snprintf(path, sizeof path, "fd[%d]", i + 1);
        if (want.kind == PICO8_VALUE_STRING) {
            assert(values_equal(got, want, path));
            rr = got.as.string;
        } else {
            const pico8_ob_table_t *view;
            const pico8_ob_entry_t *last;

            assert(want.kind == PICO8_VALUE_REF && got.kind == PICO8_VALUE_REF);
            view = got.as.ref;
            assert(view->count == ((const pico8_ob_table_t *)want.as.ref)->count + 1);
            assert(tables_equal(view, want.as.ref, view->count - 1, path));
            last = &view->entries[view->count - 1];
            assert(last->key.kind == PICO8_OB_KEY_NAME && strcmp(last->key.name, "rr") == 0);
            if (rr == NULL) {
                assert(last->value.kind == PICO8_VALUE_REF && last->value.as.ref == NULL);
            } else {
                assert(last->value.kind == PICO8_VALUE_STRING && last->value.as.string == rr);
            }
            records++;
        }
    }
    assert(records == PICO8_FD_COUNT - 8);
}

/* The start tree's literals, byte for byte: game_data.c (es, ct, b_, co, fb),
 * data_init.c (df and its rename pass, ek), entity.c (ci), menu.c (ix, ha),
 * help.c (bo), score_panel.c (ef, bz). */
static const char *const ES_SRC = "1,0,1,2,";
static const char *const CT_SRC = "-0.333,0,0.25,0.5,";
static const char *const B__SRC = "0,100,200,400,";
static const char *const CO_SRC = "-5,0,1,1,";
static const char *const FB_SRC = "2,2,5,5,7,7,10,10,14,14,14,19,19,19,";
static const char *const EK_SRC = "2,4,7,12,";
static const char *const CI_SRC = "0.5,1,1,1,1,1,1,1,1,0,0,0,0,0,0,";
static const char *const EF_SRC =
    "\"JAN\",\"FEB\",\"MAR\",\"APR\",\"MAY\",\"JUN\",\"JUL\",\"AUG\",\"SEP\",\"OCT\",\"NOV\",\"DEC\",\"UND\","
    "\"DUO\",\"TER\",";
static const char *const DF_SRC = "\n"
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
                                  "o(\"g\",96,4,\"gas giant\"),\n";
static const char *const IX_SRC = "\n"
                                  "o(\n"
                                  "ra=v(118,0),\n"
                                  "lq=o(o(15,7)),\n"
                                  "),o(\n"
                                  "ra=v(0,0),\n"
                                  "lq=o(o(37,7)),\n"
                                  "),o(\n"
                                  "ra=v(47,0),\n"
                                  "lq=o(o(25,7)),\n"
                                  "),o(\n"
                                  "ra=v(0,117),\n"
                                  "lq=o(o(8,7)),\n"
                                  "),\n";
static const char *const HA_SRC = "\n"
                                  "o(lq=o(31,\"restart     \")),\n"
                                  "o(lq=o(42,\"toggle music\")),\n"
                                  "o(lq=o(40,\"toggle sfx  \")),\n"
                                  "o(lq=o(26,\"how to play?\"),fg=5,),\n"
                                  "hi=v(0,107),gq=v(0,-11),\n"
                                  "ca=14,\n";
static const char *const BO_SRC = "\no(lq=o(49,\"forgiving   \"),px=\"forgiving\",\ntrade=7,en=-0.99),\n"
                                  "o(lq=o(50,\"reasonable  \"),px=\"reasonable\",\ntrade=6,en=-0.6),\n"
                                  "o(lq=o(51,\"challenging \"),px=\"challenging\",\ntrade=5,en=-0.4),\n"
                                  "o(lq=o(52,\"tough       \"),px=\"tough\",\ntrade=4,en=-0.2),\n";
static const char *const BZ_SRC = "\n"
                                  "o(26,86,0,fn=\"br\"),\n"
                                  "o(51,59,1,fn=\"br\"),\n"
                                  "o(76,77,1,fn=\"br\"),\n"
                                  "o(24,25,1,fn=\"br\"),\n"
                                  "o(33,41,1,fn=\"br\"),\n"
                                  "o(\"\",score=0,ra=v(64,21),c=5),\n"
                                  "o(55,\"planets:   \",\n"
                                  "score=1,ra=v(64,34)),\n"
                                  "o(19,\"population:\",\n"
                                  "score=2,ra=v(64,43)),\n"
                                  "o(24,\"technology:\",\n"
                                  "score=3,ra=v(64,52)),\n"
                                  "o(o(27,5),o(f,0),f,o(25,5),\"happiness\",\n"
                                  "score=4,ra=v(64,61)),\n"
                                  "o(\"total:\",\"\",\n"
                                  "score=5,ra=v(64,74)),\n"
                                  "o(score=6,ra=v(64,82)),\n";

/* Entry i of the parsed reference is positional i + 1 and its number. */
static pico8_fixed_t positional_fixed(const pico8_ob_table_t *ref, int i) {
    assert(ref->entries[i].key.kind == PICO8_OB_KEY_INDEX);
    assert(ref->entries[i].key.index == i + 1);
    assert(ref->entries[i].value.kind == PICO8_VALUE_FIXED);
    return ref->entries[i].value.as.fixed;
}

static void check_int_seq(const char *src, const int *seq, int count) {
    pico8_ob_table_t *ref = parse(src);
    int out = -12345;
    int i;

    assert(ref->count == count);
    for (i = 0; i < count; i++) {
        assert(positional_fixed(ref, i) == pico8_fixed_from_int(seq[i]));
        assert(pico8_int_seq_at(seq, count, i + 1, &out));
        assert(out == seq[i]);
    }
    out = -12345;
    assert(!pico8_int_seq_at(seq, count, 0, &out));
    assert(!pico8_int_seq_at(seq, count, count + 1, &out));
    assert(out == -12345);
}

static void check_fixed_seq(const char *src, const pico8_fixed_t *seq, int count) {
    pico8_ob_table_t *ref = parse(src);
    pico8_fixed_t out = -12345;
    int i;

    assert(ref->count == count);
    for (i = 0; i < count; i++) {
        assert(positional_fixed(ref, i) == seq[i]);
        assert(pico8_fixed_seq_at(seq, count, i + 1, &out));
        assert(out == seq[i]);
    }
    out = -12345;
    assert(!pico8_fixed_seq_at(seq, count, 0, &out));
    assert(!pico8_fixed_seq_at(seq, count, count + 1, &out));
    assert(out == -12345);
}

static void check_int_seq_at_bounds(const pico8_int_seq_t *seq) {
    int out = -12345;
    int i;

    for (i = 0; i < seq->count; i++) {
        assert(pico8_int_seq_at(seq->items, seq->count, i + 1, &out) && out == seq->items[i]);
    }
    out = -12345;
    assert(!pico8_int_seq_at(seq->items, seq->count, 0, &out));
    assert(!pico8_int_seq_at(seq->items, seq->count, seq->count + 1, &out));
    assert(out == -12345);
}

static void test_leaf_tables_match_their_parsed_literal_in_order(void) {
    pico8_ob_table_t *ef = parse(EF_SRC);
    int i;

    check_int_seq(ES_SRC, pico8_es_table, PICO8_ES_COUNT);
    check_fixed_seq(CT_SRC, pico8_ct_table, PICO8_CT_COUNT);
    check_int_seq(B__SRC, pico8_b__table, PICO8_B__COUNT);
    check_int_seq(CO_SRC, pico8_co_table, PICO8_CO_COUNT);
    check_int_seq(FB_SRC, pico8_fb_table, PICO8_FB_COUNT);
    check_int_seq(EK_SRC, pico8_ek_table, PICO8_EK_COUNT);
    check_fixed_seq(CI_SRC, pico8_ci_table, PICO8_CI_COUNT);

    assert(ef->count == PICO8_EF_COUNT);
    for (i = 0; i < PICO8_EF_COUNT; i++) {
        assert(ef->entries[i].key.kind == PICO8_OB_KEY_INDEX && ef->entries[i].key.index == i + 1);
        assert(ef->entries[i].value.kind == PICO8_VALUE_STRING);
        assert(strcmp(ef->entries[i].value.as.string, pico8_ef_table[i]) == 0);
    }
}

/* The cart's px/s/r/d rename pass, redone on the parsed df: each record
 * gains px, s, r and d (when present) by name, and df gains px -> record. */
static pico8_ob_table_t *df_reference(void) {
    static const char *const mr_items[] = {"px", "s", "r", "d"};
    const pico8_str_seq_t mr = {4, mr_items};
    pico8_ob_table_t *df = parse(DF_SRC);
    pico8_ob_table_t *records[PICO8_DF_COUNT];
    int i;

    assert(df->count == PICO8_DF_COUNT);
    for (i = 0; i < PICO8_DF_COUNT; i++) {
        assert(df->entries[i].value.kind == PICO8_VALUE_REF);
        records[i] = df->entries[i].value.as.ref;
    }
    for (i = 0; i < PICO8_DF_COUNT; i++) {
        pico8_ho_apply(df, &mr, NULL, records[i]);
    }
    return df;
}

/* A df entry laid out as the rename pass leaves a record: positionals px,
 * s, r (, d), then me when present, then px, s, r (, d) by name. */
static pico8_ob_table_t *df_entry_view(const pico8_df_entry_t *e) {
    pico8_ob_table_t *ri = pico8_ob_table_new();

    pico8_ob_table_set_index(ri, 1, pico8_value_string(e->px));
    pico8_ob_table_set_index(ri, 2, pico8_value_fixed(pico8_fixed_from_int(e->s)));
    pico8_ob_table_set_index(ri, 3, pico8_value_fixed(e->r));
    if (e->d != NULL) {
        pico8_ob_table_set_index(ri, 4, pico8_value_string(e->d));
    }
    if (e->me != NULL) {
        pico8_ob_table_set_name(ri, "me", pico8_value_string(e->me));
    }
    pico8_ob_table_set_name(ri, "px", pico8_value_string(e->px));
    pico8_ob_table_set_name(ri, "s", pico8_value_fixed(pico8_fixed_from_int(e->s)));
    pico8_ob_table_set_name(ri, "r", pico8_value_fixed(e->r));
    if (e->d != NULL) {
        pico8_ob_table_set_name(ri, "d", pico8_value_string(e->d));
    }
    return ri;
}

static void test_df_table_matches_the_parsed_table_after_its_pass(void) {
    pico8_ob_table_t *ref = df_reference();
    int i;

    assert(ref->count == 2 * PICO8_DF_COUNT);
    for (i = 0; i < PICO8_DF_COUNT; i++) {
        const pico8_df_entry_t *e = &pico8_df_table[i];
        const pico8_ob_table_t *record = ref->entries[i].value.as.ref;
        pico8_ob_table_t *view = df_entry_view(e);

        assert(ref->entries[PICO8_DF_COUNT + i].key.kind == PICO8_OB_KEY_NAME);
        assert(strcmp(ref->entries[PICO8_DF_COUNT + i].key.name, e->px) == 0);
        assert(ref->entries[PICO8_DF_COUNT + i].value.as.ref == record);
        assert(view->count == record->count);
        assert(tables_equal(view, record, record->count, "df"));
        pico8_ob_table_free(view);
        assert(pico8_df_lookup(e->px) == e);
    }
    assert(pico8_df_lookup("zz") == NULL);
    assert(pico8_df_lookup(NULL) == NULL);
}

static pico8_fixed_t never_drawn(void *ctx) {
    (void)ctx;
    assert(0 && "the pool is built without a draw");
    return 0;
}

/* bw hands gn every df entry, in order, at og = 2. */
static void test_gn_pools_df_entries_by_their_r(void) {
    pico8_list_t hr;
    pico8_gn_state_t *state;
    int i;

    pico8_list_init(&hr);
    for (i = 0; i < PICO8_DF_COUNT; i++) {
        pico8_list_add(&hr, pico8_value_ref((void *)&pico8_df_table[i]));
    }
    state = pico8_gn(&hr, pico8_fixed_from_int(2), never_drawn, NULL);
    assert(state->oc.count == 66);
    for (i = 0; i < PICO8_DF_COUNT; i++) {
        int want = pico8_fixed_flr_int(pico8_fixed_mul(pico8_df_table[i].r, pico8_fixed_from_int(2)));
        int got = 0;
        int j;

        for (j = 0; j < state->oc.count; j++) {
            got += state->oc.items[j].as.ref == &pico8_df_table[i];
        }
        assert(got == want);
    }
    pico8_gn_free(state);
    pico8_list_free(&hr);
}

static void check_view(pico8_ob_table_t *view, const char *src, const char *name) {
    pico8_ob_table_t *ref = parse(src);

    assert(view->count == ref->count);
    assert(tables_equal(view, ref, ref->count, name));
    pico8_ob_table_free(view);
}

static void test_ix_bo_bz_views_match_their_parsed_literal(void) {
    check_view(pico8_ix_view(), IX_SRC, "ix");
    check_view(pico8_bo_view(), BO_SRC, "bo");
    check_view(pico8_bz_view(), BZ_SRC, "bz");
}

static void test_ha_view_matches_its_parsed_literal_with_two_vectors(void) {
    pico8_ob_table_t *ha = pico8_ha_view();
    pico8_value_t hi;
    pico8_value_t gq;

    assert(pico8_ob_table_get_name(ha, "hi", &hi) && pico8_ob_table_get_name(ha, "gq", &gq));
    assert(hi.as.ref != gq.as.ref);
    check_view(ha, HA_SRC, "ha");
}

/* The start tree's class literals, byte for byte: planet.c (jo),
 * planet_family.c (qh, nl, os, hj, pm, ga, then the keys pm:pq hands its ga),
 * planet_family_trade.c (nr, fr, mx). */
static const char *const JO_SRC = "\n"
    "nu=o(\"jo\",\"np\",\"prober\"),\n"
    "me=\"unknown\",\n"
    "hf=1,hs=1,\n"
    "fa=6,\n"
    "kq=b(-7,-7,7,7),\n"
    "ma=0,\n"
    "ka=o(2,3,9,12),\n"
    "dz=o(\n"
    "o(\"unhappy\",c=8),\n"
    "o(\"content\",c=3),\n"
    "o(\"prosperous\",c=9),\n"
    "o(\"rich\",c=12),\n"
    "),\n"
    "cq=o(12,36,24,25),\n"
    "kw=o(mu=0),\n"
    "gq=v(0,11),ca=9,\n"
    "em=o(f,3,0,-1),\n";
static const char *const QH_SRC = "\n"
    "nu=o(\"qh\",\"jo\",\"np\",\"prober\"),\n"
    "hp=1,px=o(\"laboratory\",c=13),\n"
    "me=\"kz\",dq=\"ly\",\n"
    "hf=f,qh=1,\n"
    "od=15,fp=2,\n"
    "dw=0.025,\n"
    "mi=1,\n"
    "s=174,\n"
    "ba=o(28,18,20,21,22,48),\n";
static const char *const NL_SRC = "\n"
    "nu=o(\"jo\",\"np\",\"prober\"),\n"
    "rr=\"trade league\",qh=f,nl=1,\n"
    "hp=6,s=68,px=o(\"trading hub\",c=13),\n"
    "od=20,qw=15,\n"
    "ba=o(28,48,18,20,21,22),\n";
static const char *const OS_SRC = "\n"
    "nu=o(\"jo\",\"np\",\"prober\"),\n"
    "hp=4,px=o(\"processor\",c=13),\n"
    "qh=f,s=100,\n"
    "od=15,\n"
    "hs=0.5,\n";
static const char *const HJ_SRC = "\n"
    "nu=o(\"hj\",\"jo\",\"np\",\"prober\"),\n"
    "rr=\"ascension\",\n"
    "hp=2,qh=f,\n"
    "px=o(\"ascension nr\",c=13),\n"
    "od=60,\n"
    "s=166,\n"
    "py=0.12,ki=9,\n"
    "pn=5,pk=1,\n"
    "fc=0.5,\n"
    "k_=o(1,13,12,12,12,13,13,5,1,1,1),\n"
    "du=7.5,\n"
    "qo=0.98,ro=-0.07,\n"
    "qg=0.07,q_=0.98,\n";
static const char *const PM_SRC = "\n"
    "fz=\"pm\",\n"
    "nu=o(\"jo\",\"np\",\"prober\"),\n"
    "hp=3,px=o(\"protostar\",c=13),\n"
    "me=\"ly\",s=98,\n"
    "py=0.06,ki=9,\n"
    "pn=8,pk=f,\n"
    "ng=v(1,0),\n"
    "du=2,\n"
    "fc=1,\n"
    "k_=o(1,2,4,9,10,7),\n"
    "qo=1.02,ro=-0.0525,\n"
    "qg=0.0525,q_=1.02,\n";
static const char *const GA_SRC = "\n"
    "fz=\"ga\",\n"
    "nu=o(),\n"
    "hf=f,\n"
    "by=o(),\n"
    "kq=b(0,0,-1,-1),\n"
    "fa=3,\n"
    "fc=0,\n"
    "qo=0.9,ro=0,\n"
    "qg=0,q_=0.9,\n"
    "k_=o(1,2,4,9,9,10,10,7,7,7,7,7),\n"
    "iu=-0.06,py=6,ms=3,\n";
static const char *const NR_SRC = "\n"
    "nu=o(\"nr\",\"np\",\"prober\"),\n"
    "od=10,fp=1,\n"
    "kq=b(-4,-4,5,5),\n"
    "s=170,\n"
    "hz=o(170,170,170,168),\n"
    "dw=0.01,\n"
    "rr=\"slipgates\",\n";
static const char *const FR_SRC = "\n"
    "mt=1,\n"
    "rr=\"infraspace\",\n"
    "od=25,\n"
    "s=78,hz=o(78,78,78,76),\n";
static const char *const MX_SRC = "\n"
    "nu=o(\"jo\",\"np\",\"prober\"),\n"
    "rr=\"void synthesis\",\n"
    "hp=5,s=66,px=o(\"synthesizer\",c=13),\n"
    "od=40,\n"
    "ht=o(28,20,21,22),\n"
    "py=0.12,ki=9,\n"
    "pn=5,pk=1,\n"
    "fc=0.3,\n"
    "k_=o(1,1,5,5,3,3,3,5,5,1),\n"
    "du=5,\n"
    "qo=0.98,ro=-0.07,\n"
    "qg=0.07,q_=0.98,\n";
static const char *const PM_PQ_KEYS_SRC = "\n"
    "qo=0.97,ro=-0.1255,\n"
    "qg=0.1255,q_=0.97,\n"
    "k_=o(1,2,8,14,14,14,14,8,8,2,2,1,1),\n"
    "iu=-0.009,py=9,ms=1.5,\n";

static const char *const G__SRC =
    "\n"
    "nu=o(\"g_\",\"prober\"),\n"
    "fx=1,\n"
    "dq=\"ly\",\n"
    "od=0,fp=f,\n"
    "fa=2,\n"
    "dw=0.025,\n"
    "py=0.06,ki=9,\n"
    "pn=3,pk=1,\n"
    "fc=1,\n"
    "k_=o(1,1,5,13,6),\n"
    "qo=1.02,ro=-0.0525,\n"
    "qg=0.0525,q_=1.02,\n"
    "du=2,\n";
static const char *const FV_SRC =
    "\n"
    "fa=10,ra=v(64,28),\n"
    "hi=v(34,56),\n"
    "ca=11,\n"
    "gq=v(0,11),\n"
    "eo=-1,\n"
    "ff=1,\n"
    "fc=5,\n"
    "k_=o(0,1,1,2,4,9,10,9,4,2,1,1,1),\n"
    "qo=1.02,ro=-0.007,\n"
    "qg=0.007,q_=1.02,\n"
    "pn=5,du=15,\n"
    "py=0.0067,pk=1,\n"
    "ki=17,\n"
    "kq=b(-40,78,40,98),\n"
    "ql=11,fg=5,\n"
    "rb=o(\n"
    "o(192,0,11,16,4,fn=\"s\"),\n"
    "o(172,20,107,2,2,fn=\"s\"),\n"
    "o(\"need help?\",40,110,13,fn=\"qr\"),\n"
    "o(\"slipways.net/help\",40,116,5,fn=\"qr\"),\n"
    "),\n";
/* The class strings ke and the interface modules parsed at init, byte for
 * byte. */
static const char *const KE_SRC = "\nme=\"ph\",t=0,\n";
static const char *const BG_SRC = "\nfa=1,hd=1,pl=o(0,0,0,72,73,74,75,88,89,90,91,88,91,89,91),\n";
static const char *const UI_SRC = "\nql=58,fa=15,le=o(),hw=b(10,10,118,118),\n";
static const char *const JS_SRC = "\nfa=11,\n";
static const char *const FQ_SRC = "\nfa=14,hd=1,\n";
static const char *const LQ_SRC = "";
static const char *const JB_SRC = "";
static const char *const QB_SRC = "\nv=v(0,0),ma=0,p=v(192,192),ra=v(192,192),"
                                  "pc=o(v(-1,0),v(1,0),v(0,-1),v(0,1)),\n";
static const char *const NF_SRC = "\nl=o(),r=o(),ra=v(0,0),\n";
static const char *const MH_SRC = "\nnu=o(\"mh\"),\nfa=14,\nrh=5,\n";
static const char *const IV_SRC = "\nfa=9,\nl=0,v=v(0,-0.34),\n";
static const char *const MZ_SRC = "\nnu=o(\"mz\"),\nme=\"kz\",dw=0,\ndq=\"pr\",\nspawn_snd=52,ik=49,\noi=0,"
                                  "\nod=3,fp=1,\nfa=5,\nkk=o(7,7,6,13,5,1,1,1,0),\njj=o(qs=\"-too far-\"),\n";
static const char *const JC_SRC = "\n"
                                  "nu=o(\"jc\"),\n"
                                  "me=\"kz\",\n"
                                  "fa=2,\n"
                                  "fp=f,ik=48,\n"
                                  "dw=0.004,\n"
                                  "k_=o(0,0,7,7,10,10,9,9,9,9,9,9,9,9,4,4,4,2,2,1),\n"
                                  "nh=20,\n";

static const char *const HP_SRC =
    "\n"
    "mr=o(\"px\",\"lr\",\"fj\",\"ja\",\"i\",\"o\",\"il\",\"dm\"),\n"
    "gp=o(\"o\",\"dm\"),\n"
    "fk=o(f,2),\n";
static const char *const PJ_SRC =
    "\n"
    "fa=10,\n"
    "ea=o(\n"
    "o(42,43,1,fn=\"br\"),\n"
    "o(\"\",64,40,6,0.5,fn=\"qr\"),\n"
    "),\n"
    "gs=o(\n"
    "o(\"last year!\",64,48,9,0.5,fn=\"qr\"),\n"
    "o(\"2 years remain\",64,48,2,0.5,fn=\"qr\"),\n"
    "f,f,\n"
    "o(\"5 years remain\",64,48,2,0.5,fn=\"qr\"),\n"
    "),\n"
    "cr=o(\n"
    "hi=v(0,11),gq=v(0,11),ca=10,\n"
    "),\n"
    "i_=o(\n"
    "o(126,127,1,fn=\"br\"),\n"
    "o(0,1,1,fn=\"br\"),\n"
    "o(8,118,34,fn=\"rf\"),\n"
    "o(0,-3,26,fn=\"rf\"),\n"
    "o(81,-3,117,fn=\"rf\"),\n"
    "),\n"
    "ou=o(\n"
    "o(f,13,121,9,fn=\"qr\"),\n"
    "o(f,37,121,2,fn=\"qr\"),\n"
    "o(f,13,2,9,fn=\"qr\"),\n"
    "o(f,29,2,2,fn=\"qr\"),\n"
    "o(f,59,2,2,fn=\"qr\"),\n"
    "o(f,85,2,13,fn=\"qr\"),\n"
    "o(f,99,2,13,fn=\"qr\"),\n"
    "),\n"
    "ed=1,\n";
/* ie's class string, then the second string its module parsed for the
 * objects it lends every icon; hy and trade parsed an empty string. */
static const char *const IE_SRC = "\nkq=b(0,0,9,8),\nfa=12,\nid=0,\nra=v(999,999),\neh=v(118,120),\n";
static const char *const IE_OBJECTS_SRC = "\nkq=b(0,0,9,8),\nra=v(999,999),\neh=v(118,120),\n";
static const char *const HY_SRC = "";
static const char *const TRADE_SRC = "";

/* The table `ref` holds under `key`. */
static const pico8_ob_table_t *member_of(const pico8_ob_table_t *ref, const char *key) {
    pico8_value_t v;

    assert(pico8_ob_table_get_name(ref, key, &v));
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

static void check_seq(const pico8_ob_table_t *t, const pico8_int_seq_t *seq) {
    int i;

    assert(t->count == seq->count);
    for (i = 0; i < seq->count; i++) {
        assert(positional_fixed(t, i) == pico8_fixed_from_int(seq->items[i]));
    }
    check_int_seq_at_bounds(seq);
}

static void check_member_seq(const pico8_ob_table_t *ref, const char *key, const pico8_int_seq_t *seq) {
    check_seq(member_of(ref, key), seq);
}

static void check_names(const pico8_ob_table_t *t, const pico8_str_seq_t *tags) {
    int i;

    assert(t->count == tags->count);
    for (i = 0; i < tags->count; i++) {
        assert(t->entries[i].key.kind == PICO8_OB_KEY_INDEX && t->entries[i].key.index == i + 1);
        assert(t->entries[i].value.kind == PICO8_VALUE_STRING);
        assert(strcmp(t->entries[i].value.as.string, tags->items[i]) == 0);
    }
}

static void check_member_tags(const pico8_ob_table_t *ref, const pico8_str_seq_t *tags) {
    check_names(member_of(ref, "nu"), tags);
}

static void check_row(const pico8_ob_table_t *t, const pico8_jb_row_t *row, const char *path) {
    pico8_ob_table_t *view = pico8_jb_row_view(row);

    assert(view->count == t->count);
    assert(tables_equal(view, t, t->count, path));
    pico8_ob_table_free(view);
}

static void test_jo_tree_data_matches_the_parsed_class_literals(void) {
    pico8_ob_table_t *jo = parse(JO_SRC);
    pico8_ob_table_t *qh = parse(QH_SRC);
    pico8_ob_table_t *nl = parse(NL_SRC);
    pico8_ob_table_t *os = parse(OS_SRC);
    pico8_ob_table_t *hj = parse(HJ_SRC);
    pico8_ob_table_t *pm = parse(PM_SRC);
    pico8_ob_table_t *ga = parse(GA_SRC);
    pico8_ob_table_t *nr = parse(NR_SRC);
    pico8_ob_table_t *fr = parse(FR_SRC);
    pico8_ob_table_t *mx = parse(MX_SRC);
    pico8_ob_table_t *pm_pq = parse(PM_PQ_KEYS_SRC);
    const pico8_ob_table_t *dz = member_of(jo, "dz");
    const pico8_ob_table_t *kw = member_of(jo, "kw");
    pico8_value_t v;
    int i;

    check_member_tags(jo, &pico8_jo_nu_tags);
    check_member_tags(qh, &pico8_qh_nu_tags);
    check_member_tags(nl, &pico8_nl_nu_tags);
    check_member_tags(os, &pico8_os_nu_tags);
    check_member_tags(hj, &pico8_hj_nu_tags);
    check_member_tags(pm, &pico8_pm_nu_tags);
    check_member_tags(ga, &pico8_ga_nu_tags);
    check_member_tags(nr, &pico8_nr_nu_tags);
    check_member_tags(mx, &pico8_mx_nu_tags);
    assert(pico8_ga_nu_tags.count == 0 && pico8_ga_nu_tags.items == NULL);
    assert(!pico8_ob_table_get_name(fr, "nu", &v));

    check_member_seq(jo, "ka", &pico8_jo_ka_seq);
    check_member_seq(jo, "cq", &pico8_jo_cq_seq);
    assert(dz->count == PICO8_JO_DZ_COUNT);
    for (i = 0; i < PICO8_JO_DZ_COUNT; i++) {
        assert(dz->entries[i].key.kind == PICO8_OB_KEY_INDEX && dz->entries[i].key.index == i + 1);
        check_row(dz->entries[i].value.as.ref, &pico8_jo_dz_rows[i], "jo.dz");
    }
    assert(kw->count == 1 && pico8_ob_table_get_name(kw, "mu", &v));
    assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == pico8_jo_kw_start.mu && pico8_jo_kw_start.mu == 0);
    assert(!pico8_jo_kw_start.has_qe);
    check_row(member_of(jo, "em"), &pico8_jo_em_row, "jo.em");

    check_row(member_of(qh, "px"), &pico8_qh_px_row, "qh.px");
    check_row(member_of(nl, "px"), &pico8_nl_px_row, "nl.px");
    check_row(member_of(os, "px"), &pico8_os_px_row, "os.px");
    check_row(member_of(hj, "px"), &pico8_hj_px_row, "hj.px");
    check_row(member_of(pm, "px"), &pico8_pm_px_row, "pm.px");
    check_row(member_of(mx, "px"), &pico8_mx_px_row, "mx.px");

    check_member_seq(qh, "ba", &pico8_qh_ba_seq);
    check_member_seq(nl, "ba", &pico8_nl_ba_seq);
    check_member_seq(hj, "k_", &pico8_hj_k__seq);
    check_member_seq(pm, "k_", &pico8_pm_k__seq);
    check_member_seq(mx, "k_", &pico8_mx_k__seq);
    check_member_seq(ga, "k_", &pico8_ga_k__seq);
    check_member_seq(pm_pq, "k_", &pico8_pm_ga_k__seq);
    check_member_seq(nr, "hz", &pico8_nr_hz_seq);
    check_member_seq(fr, "hz", &pico8_fr_hz_seq);
    check_member_seq(mx, "ht", &pico8_mx_ht_seq);
}

static void test_other_classes_data_matches_the_parsed_literals(void) {
    pico8_ob_table_t *g_ = parse(G__SRC);
    pico8_ob_table_t *fv = parse(FV_SRC);
    pico8_ob_table_t *bg = parse(BG_SRC);
    const pico8_ob_table_t *qb_pc = member_of(parse(QB_SRC), "pc");
    pico8_ob_table_t *mh = parse(MH_SRC);
    pico8_ob_table_t *jc = parse(JC_SRC);
    pico8_ob_table_t *mz = parse(MZ_SRC);
    pico8_value_t v;
    int i;

    check_member_tags(mh, &pico8_mh_nu_tags);
    check_member_tags(g_, &pico8_g__nu_tags);
    assert(!pico8_ob_table_get_name(fv, "nu", &v));
    check_member_tags(jc, &pico8_jc_nu_tags);
    check_member_tags(mz, &pico8_mz_nu_tags);

    check_member_seq(g_, "k_", &pico8_g__k__seq);
    check_member_seq(fv, "k_", &pico8_fv_k__seq);
    check_member_seq(jc, "k_", &pico8_jc_k__seq);
    check_member_seq(mz, "kk", &pico8_mz_kk_seq);
    check_member_seq(bg, "pl", &pico8_bg_pl_seq);

    assert(qb_pc->count == pico8_qb_pc_vecs.count && pico8_qb_pc_vecs.count == 4);
    for (i = 0; i < pico8_qb_pc_vecs.count; i++) {
        const pico8_vec_t *parsed;

        assert(qb_pc->entries[i].key.kind == PICO8_OB_KEY_INDEX && qb_pc->entries[i].key.index == i + 1);
        assert(qb_pc->entries[i].value.kind == PICO8_VALUE_REF && qb_pc->entries[i].value.as.ref != NULL);
        parsed = qb_pc->entries[i].value.as.ref;
        assert(parsed->x == pico8_qb_pc_vecs.items[i].x && parsed->y == pico8_qb_pc_vecs.items[i].y);
    }
}

/* The parsed value under `key`, of the kind asked for. */
static pico8_fixed_t number_at(const pico8_ob_table_t *t, const char *key) {
    pico8_value_t v;

    assert(pico8_ob_table_get_name(t, key, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static const char *string_at(const pico8_ob_table_t *t, const char *key) {
    pico8_value_t v;

    assert(pico8_ob_table_get_name(t, key, &v) && v.kind == PICO8_VALUE_STRING);
    return v.as.string;
}

/* v(...) and b(...) both parse to an untagged REF. */
static const pico8_vec_t *vec_at(const pico8_ob_table_t *t, const char *key) {
    pico8_value_t v;

    assert(pico8_ob_table_get_name(t, key, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

static const pico8_qz_box_t *box_at(const pico8_ob_table_t *t, const char *key) {
    pico8_value_t v;

    assert(pico8_ob_table_get_name(t, key, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

static void check_vec(const pico8_vec_t *parsed, const pico8_vec_t *start) {
    assert(parsed->x == start->x && parsed->y == start->y);
}

/* Both ways: every key the string parses to is one of the class's generated
 * fields (or one of the empty tables its module holds), equal to it, and the
 * key count is the number of fields checked. */
static void test_ke_and_interface_starts_match_the_parsed_class_strings(void) {
    pico8_ob_table_t *ke = parse(KE_SRC);
    pico8_ob_table_t *bg = parse(BG_SRC);
    pico8_ob_table_t *ui = parse(UI_SRC);
    pico8_ob_table_t *js = parse(JS_SRC);
    pico8_ob_table_t *fq = parse(FQ_SRC);
    pico8_ob_table_t *qb = parse(QB_SRC);
    pico8_ob_table_t *nf = parse(NF_SRC);
    pico8_ob_table_t *mh = parse(MH_SRC);
    pico8_ob_table_t *iv = parse(IV_SRC);
    pico8_ob_table_t *mz = parse(MZ_SRC);
    pico8_ob_table_t *jc = parse(JC_SRC);
    const pico8_qz_box_t *hw;
    const pico8_ob_table_t *jj;
    pico8_value_t v;

    assert(ke->count == 2);
    assert(strcmp(string_at(ke, "me"), pico8_ke_start.me) == 0 && number_at(ke, "t") == pico8_ke_start.t);

    assert(bg->count == 3);
    assert(number_at(bg, "fa") == pico8_bg_start.fa && number_at(bg, "hd") == pico8_bg_start.hd);
    assert(pico8_bg_start.pl == &pico8_bg_pl_seq);
    check_member_seq(bg, "pl", pico8_bg_start.pl);

    assert(ui->count == 4);
    assert(number_at(ui, "ql") == pico8_ui_start.ql && number_at(ui, "fa") == pico8_ui_start.fa);
    assert(member_of(ui, "le")->count == 0);
    hw = box_at(ui, "hw");
    assert(hw->xl == pico8_ui_start.hw.xl && hw->yt == pico8_ui_start.hw.yt);
    assert(hw->xr == pico8_ui_start.hw.xr && hw->yb == pico8_ui_start.hw.yb);

    assert(js->count == 1 && number_at(js, "fa") == pico8_js_start.fa);

    assert(fq->count == 2);
    assert(number_at(fq, "fa") == pico8_fq_start.fa && number_at(fq, "hd") == pico8_fq_start.hd);

    assert(parse(LQ_SRC)->count == 0 && parse(JB_SRC)->count == 0);

    assert(qb->count == 5);
    check_vec(vec_at(qb, "v"), &pico8_qb_start.v);
    assert(number_at(qb, "ma") == pico8_qb_start.ma);
    check_vec(vec_at(qb, "p"), &pico8_qb_start.p);
    check_vec(vec_at(qb, "ra"), &pico8_qb_start.ra);
    assert(pico8_qb_start.pc == &pico8_qb_pc_vecs && member_of(qb, "pc")->count == pico8_qb_start.pc->count);

    assert(nf->count == 3);
    assert(member_of(nf, "l")->count == 0 && member_of(nf, "r")->count == 0);
    check_vec(vec_at(nf, "ra"), &pico8_nf_start.ra);

    assert(mh->count == 3);
    assert(pico8_mh_start.nu == &pico8_mh_nu_tags);
    check_member_tags(mh, pico8_mh_start.nu);
    assert(number_at(mh, "fa") == pico8_mh_start.fa && number_at(mh, "rh") == pico8_mh_start.rh);

    assert(iv->count == 3);
    assert(number_at(iv, "fa") == pico8_iv_start.fa && number_at(iv, "l") == pico8_iv_start.l);
    check_vec(vec_at(iv, "v"), &pico8_iv_start.v);

    assert(mz->count == 12);
    assert(pico8_mz_start.nu == &pico8_mz_nu_tags);
    check_member_tags(mz, pico8_mz_start.nu);
    assert(strcmp(string_at(mz, "me"), pico8_mz_start.me) == 0);
    assert(number_at(mz, "dw") == pico8_mz_start.dw);
    assert(strcmp(string_at(mz, "dq"), pico8_mz_start.dq) == 0);
    assert(number_at(mz, "spawn_snd") == pico8_mz_start.spawn_snd && number_at(mz, "ik") == pico8_mz_start.ik);
    assert(number_at(mz, "oi") == pico8_mz_start.oi && number_at(mz, "od") == pico8_mz_start.od);
    assert(number_at(mz, "fp") == pico8_mz_start.fp && number_at(mz, "fa") == pico8_mz_start.fa);
    assert(pico8_mz_start.kk == &pico8_mz_kk_seq);
    check_member_seq(mz, "kk", pico8_mz_start.kk);
    jj = member_of(mz, "jj");
    assert(jj->count == 1 && strcmp(string_at(jj, "qs"), pico8_mz_jj_qs) == 0);

    assert(jc->count == 8);
    assert(pico8_jc_start.nu == &pico8_jc_nu_tags);
    check_member_tags(jc, pico8_jc_start.nu);
    assert(strcmp(string_at(jc, "me"), pico8_jc_start.me) == 0);
    assert(number_at(jc, "fa") == pico8_jc_start.fa);
    assert(pico8_ob_table_get_name(jc, "fp", &v) && v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
    assert(pico8_jc_start.fp == 0);
    assert(number_at(jc, "ik") == pico8_jc_start.ik && number_at(jc, "dw") == pico8_jc_start.dw);
    assert(pico8_jc_start.k_ == &pico8_jc_k__seq);
    check_member_seq(jc, "k_", pico8_jc_start.k_);
    assert(number_at(jc, "nh") == pico8_jc_start.nh);
}

static void check_box(const pico8_qz_box_t *parsed, const pico8_qz_box_t *start) {
    assert(parsed->xl == start->xl && parsed->yt == start->yt);
    assert(parsed->xr == start->xr && parsed->yb == start->yb);
}

/* Both ways, as above, for g_, fv, hp, ie (and the objects its module
 * lends), pj, hy and trade. */
static void test_probe_help_research_panel_starts_match_the_parsed_class_strings(void) {
    pico8_ob_table_t *g_ = parse(G__SRC);
    pico8_ob_table_t *fv = parse(FV_SRC);
    pico8_ob_table_t *hp = parse(HP_SRC);
    pico8_ob_table_t *ie = parse(IE_SRC);
    pico8_ob_table_t *ie_objects = parse(IE_OBJECTS_SRC);
    pico8_ob_table_t *pj = parse(PJ_SRC);
    const pico8_ob_table_t *cr;
    pico8_value_t v;

    assert(g_->count == 18);
    assert(pico8_g__start.nu == &pico8_g__nu_tags);
    check_member_tags(g_, pico8_g__start.nu);
    assert(number_at(g_, "fx") == pico8_g__start.fx);
    assert(strcmp(string_at(g_, "dq"), pico8_g__start.dq) == 0);
    assert(number_at(g_, "od") == pico8_g__start.od);
    assert(pico8_ob_table_get_name(g_, "fp", &v) && v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
    assert(pico8_g__start.fp == 0);
    assert(number_at(g_, "fa") == pico8_g__start.fa);
    assert(number_at(g_, "dw") == pico8_g__start.dw);
    assert(number_at(g_, "py") == pico8_g__start.py);
    assert(number_at(g_, "ki") == pico8_g__start.ki);
    assert(number_at(g_, "pn") == pico8_g__start.pn);
    assert(number_at(g_, "pk") == pico8_g__start.pk);
    assert(number_at(g_, "fc") == pico8_g__start.fc);
    assert(pico8_g__start.k_ == &pico8_g__k__seq);
    check_member_seq(g_, "k_", pico8_g__start.k_);
    assert(number_at(g_, "qo") == pico8_g__start.qo);
    assert(number_at(g_, "ro") == pico8_g__start.ro);
    assert(number_at(g_, "qg") == pico8_g__start.qg);
    assert(number_at(g_, "q_") == pico8_g__start.q_);
    assert(number_at(g_, "du") == pico8_g__start.du);

    assert(fv->count == 22);
    assert(number_at(fv, "fa") == pico8_fv_start.fa);
    check_vec(vec_at(fv, "ra"), &pico8_fv_start.ra);
    check_vec(vec_at(fv, "hi"), &pico8_fv_start.hi);
    assert(number_at(fv, "ca") == pico8_fv_start.ca);
    check_vec(vec_at(fv, "gq"), &pico8_fv_start.gq);
    assert(number_at(fv, "eo") == pico8_fv_start.eo);
    assert(number_at(fv, "ff") == pico8_fv_start.ff);
    assert(number_at(fv, "fc") == pico8_fv_start.fc);
    assert(pico8_fv_start.k_ == &pico8_fv_k__seq);
    check_member_seq(fv, "k_", pico8_fv_start.k_);
    assert(number_at(fv, "qo") == pico8_fv_start.qo);
    assert(number_at(fv, "ro") == pico8_fv_start.ro);
    assert(number_at(fv, "qg") == pico8_fv_start.qg);
    assert(number_at(fv, "q_") == pico8_fv_start.q_);
    assert(number_at(fv, "pn") == pico8_fv_start.pn);
    assert(number_at(fv, "du") == pico8_fv_start.du);
    assert(number_at(fv, "py") == pico8_fv_start.py);
    assert(number_at(fv, "pk") == pico8_fv_start.pk);
    assert(number_at(fv, "ki") == pico8_fv_start.ki);
    check_box(box_at(fv, "kq"), &pico8_fv_start.kq);
    assert(number_at(fv, "ql") == pico8_fv_start.ql);
    assert(number_at(fv, "fg") == pico8_fv_start.fg);
    assert(pico8_fv_start.rb == &pico8_fv_rb_row);
    check_row(member_of(fv, "rb"), pico8_fv_start.rb, "fv.rb");

    assert(hp->count == 3);
    assert(pico8_hp_start.mr == &pico8_hp_mr_names);
    check_names(member_of(hp, "mr"), pico8_hp_start.mr);
    assert(pico8_hp_start.gp == &pico8_hp_gp_names);
    check_names(member_of(hp, "gp"), pico8_hp_start.gp);
    assert(pico8_hp_start.fk == &pico8_hp_fk_row);
    check_row(member_of(hp, "fk"), pico8_hp_start.fk, "hp.fk");

    assert(ie->count == 5);
    check_box(box_at(ie, "kq"), &pico8_ie_start.kq);
    assert(number_at(ie, "fa") == pico8_ie_start.fa);
    assert(number_at(ie, "id") == pico8_ie_start.id);
    check_vec(vec_at(ie, "ra"), &pico8_ie_start.ra);
    check_vec(vec_at(ie, "eh"), &pico8_ie_start.eh);
    assert(ie_objects->count == 3);
    check_box(box_at(ie_objects, "kq"), &pico8_ie_start.kq);
    check_vec(vec_at(ie_objects, "ra"), &pico8_ie_start.ra);
    check_vec(vec_at(ie_objects, "eh"), &pico8_ie_start.eh);

    assert(pj->count == 7);
    assert(number_at(pj, "fa") == pico8_pj_start.fa);
    assert(pico8_pj_start.ea == &pico8_pj_ea_row);
    check_row(member_of(pj, "ea"), pico8_pj_start.ea, "pj.ea");
    assert(pico8_pj_start.gs == &pico8_pj_gs_row);
    check_row(member_of(pj, "gs"), pico8_pj_start.gs, "pj.gs");
    assert(pico8_pj_start.cr == &pico8_pj_cr_start);
    cr = member_of(pj, "cr");
    assert(cr->count == 3);
    check_vec(vec_at(cr, "hi"), &pico8_pj_start.cr->hi);
    check_vec(vec_at(cr, "gq"), &pico8_pj_start.cr->gq);
    assert(number_at(cr, "ca") == pico8_pj_start.cr->ca);
    assert(pico8_pj_start.i_ == &pico8_pj_i__row);
    check_row(member_of(pj, "i_"), pico8_pj_start.i_, "pj.i_");
    assert(pico8_pj_start.ou == &pico8_pj_ou_row);
    check_row(member_of(pj, "ou"), pico8_pj_start.ou, "pj.ou");
    assert(number_at(pj, "ed") == pico8_pj_start.ed);

    assert(parse(HY_SRC)->count == 0 && parse(TRADE_SRC)->count == 0);
}

static void test_rows_match_the_parsed_literals(void) {
    pico8_ob_table_t *fv = parse(FV_SRC);
    pico8_ob_table_t *hp = parse(HP_SRC);
    pico8_ob_table_t *pj = parse(PJ_SRC);

    check_row(member_of(fv, "rb"), &pico8_fv_rb_row, "fv.rb");
    check_names(member_of(hp, "mr"), &pico8_hp_mr_names);
    check_names(member_of(hp, "gp"), &pico8_hp_gp_names);
    check_row(member_of(hp, "fk"), &pico8_hp_fk_row, "hp.fk");
    check_row(member_of(pj, "gs"), &pico8_pj_gs_row, "pj.gs");
    check_row(member_of(pj, "i_"), &pico8_pj_i__row, "pj.i_");
}

/* A non-integer item keeps its bits; fn comes after c. */
static void test_row_view_writes_fixed_items_then_c_then_fn(void) {
    static const pico8_jb_item_t items[] = {{.is_fixed = 1, .fixed = 0x8000}, {.n = 2}};
    static const pico8_jb_row_t row = {.count = 2, .items = items, .has_c = 1, .c = 7, .fn = "qr"};
    pico8_ob_table_t *view = pico8_jb_row_view(&row);
    pico8_value_t v;

    assert(view->count == 4);
    assert(positional_fixed(view, 0) == 0x8000 && positional_fixed(view, 1) == pico8_fixed_from_int(2));
    assert(view->entries[2].key.kind == PICO8_OB_KEY_NAME && strcmp(view->entries[2].key.name, "c") == 0);
    assert(view->entries[3].key.kind == PICO8_OB_KEY_NAME && strcmp(view->entries[3].key.name, "fn") == 0);
    assert(pico8_ob_table_get_name(view, "c", &v) && v.kind == PICO8_VALUE_FIXED && v.as.fixed == 0x70000);
    assert(pico8_ob_table_get_name(view, "fn", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, "qr") == 0);
    pico8_ob_table_free(view);
}

static void test_nb_lookup_finds_each_px(void) {
    int i;

    for (i = 0; i < PICO8_NB_COUNT; i++) {
        assert(pico8_nb_lookup(pico8_nb_table[i].px) == &pico8_nb_table[i]);
    }
    assert(pico8_nb_lookup("zz") == NULL && pico8_nb_lookup(NULL) == NULL);
}

/* Keys floored as Lua indexes a table, one entry per key, insertion order. */
static void test_qe_map_keeps_one_entry_per_key_in_order(void) {
    pico8_qe_map_t map = {0, {{0, 0}}};
    pico8_fixed_t v = 0x1234;

    assert(!pico8_qe_map_get(&map, pico8_fixed_from_int(21), &v) && v == 0x1234);
    pico8_qe_map_set(&map, pico8_fixed_from_int(21), pico8_fixed_from_int(3));
    pico8_qe_map_set(&map, pico8_fixed_from_int(12), pico8_fixed_from_int(5));
    pico8_qe_map_set(&map, pico8_fixed_from_int(21), pico8_fixed_from_int(7));
    assert(map.count == 2 && map.entries[0].qe == 21 && map.entries[1].qe == 12);
    assert(pico8_qe_map_get(&map, pico8_fixed_from_int(21), &v) && v == pico8_fixed_from_int(7));
    assert(pico8_qe_map_get(&map, pico8_fixed_from_int(12) + 0x8000, &v) && v == pico8_fixed_from_int(5));
    assert(!pico8_qe_map_get(&map, pico8_fixed_from_int(13), &v) && v == pico8_fixed_from_int(5));
    pico8_qe_map_set(&map, pico8_fixed_from_int(99) + 0x4000, pico8_fixed_from_int(1));
    assert(map.count == 3 && map.entries[2].qe == 99);
}

static void test_int_seq_has_fixed_keeps_gu_on_integers(void) {
    static const pico8_int_seq_t empty = {0, NULL};
    const pico8_int_seq_t *seq = &pico8_jo_cq_seq;
    int i;

    for (i = 0; i < seq->count; i++) {
        assert(pico8_int_seq_has_fixed(seq, pico8_fixed_from_int(seq->items[i])));
        assert(!pico8_int_seq_has_fixed(seq, pico8_fixed_from_int(seq->items[i]) + 0x8000));
    }
    assert(!pico8_int_seq_has_fixed(seq, pico8_fixed_from_int(99)));
    assert(!pico8_int_seq_has_fixed(seq, (pico8_fixed_t)seq->items[0]));
    assert(!pico8_int_seq_has_fixed(&empty, pico8_fixed_from_int(0)));
}

int main(void) {
    pico8_mem_t mem;

    pico8_mem_init(&mem);
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    test_r_matches_the_decoded_table_in_order(&mem);
    test_iz_matches_the_decoded_table_in_order(&mem);
    test_hc_matches_the_decoded_table_in_order(&mem);

    pico8_pi_init();
    test_leaf_tables_match_their_parsed_literal_in_order();
    test_ix_bo_bz_views_match_their_parsed_literal();
    test_ha_view_matches_its_parsed_literal_with_two_vectors();
    test_df_table_matches_the_parsed_table_after_its_pass();
    test_gn_pools_df_entries_by_their_r();
    test_int_seq_has_fixed_keeps_gu_on_integers();
    test_jo_tree_data_matches_the_parsed_class_literals();
    test_other_classes_data_matches_the_parsed_literals();
    test_ke_and_interface_starts_match_the_parsed_class_strings();
    test_probe_help_research_panel_starts_match_the_parsed_class_strings();
    test_rows_match_the_parsed_literals();
    test_row_view_writes_fixed_items_then_c_then_fn();
    test_nb_lookup_finds_each_px();
    test_qe_map_keeps_one_entry_per_key_in_order();

    pico8_data_init(&mem);
    test_dr_view_matches_the_decoded_table(&mem);
    test_nb_view_matches_the_decoded_table_after_its_pass(&mem);
    test_fd_view_matches_the_decoded_table_plus_rr(&mem);
    pico8_data_shutdown();

    printf("test_pico8_map_tables: all tests passed\n");
    return 0;
}
