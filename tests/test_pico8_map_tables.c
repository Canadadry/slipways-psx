#include "../src/pico8_map_tables.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_collect.h"
#include "../src/pico8_data_init.h"
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
    pico8_ob_table_t *nb = decode(mem, 10457);
    pico8_ob_table_t *mr = pico8_ob("\"px\",\"lv\",\"nk\",\"od\",\"oy\",", NULL);
    pico8_ob_table_t *records[PICO8_NB_COUNT];
    pico8_list_t gp;
    int i;

    assert(nb->count == PICO8_NB_COUNT);
    for (i = 0; i < PICO8_NB_COUNT; i++) {
        assert(nb->entries[i].value.kind == PICO8_VALUE_REF);
        records[i] = nb->entries[i].value.as.ref;
    }
    pico8_list_init(&gp);
    pico8_list_add(&gp, pico8_value_string("nk"));
    for (i = 0; i < PICO8_NB_COUNT; i++) {
        pico8_ho_apply(nb, mr, &gp, records[i]);
    }
    pico8_list_free(&gp);
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
static const char *const DF_PASS_SRC = "\n\"px\",\"s\",\"r\",\"d\",\n";
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
    pico8_ob_table_t *df = parse(DF_SRC);
    pico8_ob_table_t *mr = parse(DF_PASS_SRC);
    pico8_ob_table_t *records[PICO8_DF_COUNT];
    int i;

    assert(df->count == PICO8_DF_COUNT);
    for (i = 0; i < PICO8_DF_COUNT; i++) {
        assert(df->entries[i].value.kind == PICO8_VALUE_REF);
        records[i] = df->entries[i].value.as.ref;
    }
    for (i = 0; i < PICO8_DF_COUNT; i++) {
        pico8_ho_apply(df, mr, NULL, records[i]);
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

/* The table `ref` holds under `key`. */
static const pico8_ob_table_t *member_of(const pico8_ob_table_t *ref, const char *key) {
    pico8_value_t v;

    assert(pico8_ob_table_get_name(ref, key, &v));
    assert(v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    return v.as.ref;
}

static void check_member_seq(const pico8_ob_table_t *ref, const char *key, const pico8_int_seq_t *seq) {
    const pico8_ob_table_t *t = member_of(ref, key);
    int i;

    assert(t->count == seq->count);
    for (i = 0; i < seq->count; i++) {
        assert(positional_fixed(t, i) == pico8_fixed_from_int(seq->items[i]));
    }
    check_int_seq_at_bounds(seq);
}

static void check_member_tags(const pico8_ob_table_t *ref, const pico8_str_seq_t *tags) {
    const pico8_ob_table_t *t = member_of(ref, "nu");
    int i;

    assert(t->count == tags->count);
    for (i = 0; i < tags->count; i++) {
        assert(t->entries[i].key.kind == PICO8_OB_KEY_INDEX && t->entries[i].key.index == i + 1);
        assert(t->entries[i].value.kind == PICO8_VALUE_STRING);
        assert(strcmp(t->entries[i].value.as.string, tags->items[i]) == 0);
    }
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

    pico8_data_init(&mem);
    test_dr_view_matches_the_decoded_table(&mem);
    test_nb_view_matches_the_decoded_table_after_its_pass(&mem);
    test_fd_view_matches_the_decoded_table_plus_rr(&mem);
    pico8_data_shutdown();

    printf("test_pico8_map_tables: all tests passed\n");
    return 0;
}
