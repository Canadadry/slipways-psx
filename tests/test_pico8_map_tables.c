#include "../src/pico8_map_tables.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_collect.h"
#include "../src/pico8_data_init.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_stash.h"

/* The table ob() parses from the string the cart parks at map address pd:
 * the reference every generated table is compared against. */
static pico8_ob_table_t *decode(const pico8_mem_t *mem, int pd) {
    char *s = pico8_unstash(mem, pd);
    pico8_ob_table_t *t = pico8_ob(s, NULL);

    free(s);
    return t;
}

static int tables_equal(const pico8_ob_table_t *a, const pico8_ob_table_t *b, int count, const char *path);

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
        return a.as.boolean == b.as.boolean;
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

int main(void) {
    pico8_mem_t mem;

    pico8_mem_init(&mem);
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    test_r_matches_the_decoded_table_in_order(&mem);
    test_iz_matches_the_decoded_table_in_order(&mem);
    test_hc_matches_the_decoded_table_in_order(&mem);

    pico8_data_init(&mem);
    test_dr_view_matches_the_decoded_table(&mem);
    test_nb_view_matches_the_decoded_table_after_its_pass(&mem);
    test_fd_view_matches_the_decoded_table_plus_rr(&mem);
    pico8_data_shutdown();

    printf("test_pico8_map_tables: all tests passed\n");
    return 0;
}
