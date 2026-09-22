#include "../src/pico8_ob.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_fixed.h"
#include "../src/pico8_value.h"

/* The cart's own `bm` spec, in its single-line form. */
static void test_purely_positional_list(void) {
    pico8_ob_table_t *t = pico8_ob("\"kj\",\"ev\",\"hd\",\"hk\",\"hb\",", NULL);
    pico8_value_t v;

    assert(pico8_ob_table_get_index(t, 1, &v));
    assert(v.kind == PICO8_VALUE_STRING);
    assert(strcmp(v.as.string, "kj") == 0);
    free((void *)v.as.string);

    assert(pico8_ob_table_get_index(t, 2, &v));
    assert(strcmp(v.as.string, "ev") == 0);
    free((void *)v.as.string);

    assert(pico8_ob_table_get_index(t, 3, &v));
    assert(strcmp(v.as.string, "hd") == 0);
    free((void *)v.as.string);

    assert(pico8_ob_table_get_index(t, 4, &v));
    assert(strcmp(v.as.string, "hk") == 0);
    free((void *)v.as.string);

    assert(pico8_ob_table_get_index(t, 5, &v));
    assert(strcmp(v.as.string, "hb") == 0);
    free((void *)v.as.string);

    assert(!pico8_ob_table_get_index(t, 6, &v));

    pico8_ob_table_free(t);
}

/* The same literal verbatim, embedded newlines and all: it exercises the
 * asymmetry between a newline, always skipped, and a space, skipped only
 * outside quotes. */
static void test_purely_positional_list_real_bm_spec_with_embedded_newlines(void) {
    const char *bm_spec =
        "\"kj\",\"ev\",\n"
        "\"hd\",\n"
        "\"hk\",\"hb\",\n";
    pico8_ob_table_t *t = pico8_ob(bm_spec, NULL);
    pico8_value_t v;

    assert(pico8_ob_table_get_index(t, 1, &v) && strcmp(v.as.string, "kj") == 0);
    free((void *)v.as.string);
    assert(pico8_ob_table_get_index(t, 2, &v) && strcmp(v.as.string, "ev") == 0);
    free((void *)v.as.string);
    assert(pico8_ob_table_get_index(t, 3, &v) && strcmp(v.as.string, "hd") == 0);
    free((void *)v.as.string);
    assert(pico8_ob_table_get_index(t, 4, &v) && strcmp(v.as.string, "hk") == 0);
    free((void *)v.as.string);
    assert(pico8_ob_table_get_index(t, 5, &v) && strcmp(v.as.string, "hb") == 0);
    free((void *)v.as.string);
    assert(!pico8_ob_table_get_index(t, 6, &v));

    pico8_ob_table_free(t);
}

/* A purely named-field list, the shape of an `o(x=...,y=...,c=...)` spec's
 * inner content. */
static void test_purely_named_field_list(void) {
    pico8_ob_table_t *t = pico8_ob("x=-1,y=-1,c=0,", NULL);
    pico8_value_t v;

    assert(pico8_ob_table_get_name(t, "x", &v));
    assert(v.kind == PICO8_VALUE_FIXED);
    assert(v.as.fixed == pico8_fixed_from_int(-1));

    assert(pico8_ob_table_get_name(t, "y", &v));
    assert(v.as.fixed == pico8_fixed_from_int(-1));

    assert(pico8_ob_table_get_name(t, "c", &v));
    assert(v.as.fixed == pico8_fixed_from_int(0));

    assert(!pico8_ob_table_get_name(t, "z", &v));

    pico8_ob_table_free(t);
}

/* No registered constructor -- the common case, `pi` has no 'o' entry
 * either: the recursive parse's own table becomes the value. */
static void test_nested_value_with_no_registered_constructor(void) {
    pico8_ob_clear_constructors();

    pico8_ob_table_t *t = pico8_ob("o(x=1,y=2),", NULL);
    pico8_value_t v;

    assert(pico8_ob_table_get_index(t, 1, &v));
    assert(v.kind == PICO8_VALUE_REF);

    pico8_ob_table_t *nested = v.as.ref;
    pico8_value_t nx, ny;
    assert(pico8_ob_table_get_name(nested, "x", &nx));
    assert(nx.as.fixed == pico8_fixed_from_int(1));
    assert(pico8_ob_table_get_name(nested, "y", &ny));
    assert(ny.as.fixed == pico8_fixed_from_int(2));

    pico8_ob_table_free(nested);
    pico8_ob_table_free(t);
}

/* A test-only constructor: sums its first two positional arguments, just
 * enough to prove `oo()` really unpacks positional fields into a
 * registered function's arguments. */
static pico8_value_t test_sum_constructor(pico8_value_t a1, pico8_value_t a2, pico8_value_t a3,
                                           pico8_value_t a4, pico8_value_t a5) {
    (void)a3;
    (void)a4;
    (void)a5;
    return pico8_value_fixed(pico8_fixed_add(a1.as.fixed, a2.as.fixed));
}

/* A nested value whose identifier IS registered in the constructor
 * table: the constructor's result (not the parsed sub-table) becomes the
 * value. */
static void test_nested_value_with_registered_constructor(void) {
    pico8_ob_clear_constructors();
    pico8_ob_register_constructor('z', test_sum_constructor);

    pico8_ob_table_t *t = pico8_ob("z(2,3),", NULL);
    pico8_value_t v;

    assert(pico8_ob_table_get_index(t, 1, &v));
    assert(v.kind == PICO8_VALUE_FIXED);
    assert(v.as.fixed == pico8_fixed_from_int(5));

    pico8_ob_table_free(t);
    pico8_ob_clear_constructors();
}

/* A quoted-string value. */
static void test_quoted_string_value(void) {
    pico8_ob_table_t *t = pico8_ob("\"hello\",", NULL);
    pico8_value_t v;

    assert(pico8_ob_table_get_index(t, 1, &v));
    assert(v.kind == PICO8_VALUE_STRING);
    assert(strcmp(v.as.string, "hello") == 0);
    free((void *)v.as.string);

    pico8_ob_table_free(t);
}

/* A `false` value -- detected purely by the token's first character
 * being 'f', never by comparing the full token against the string
 * "false". */
static void test_false_value(void) {
    pico8_ob_table_t *t = pico8_ob("false,", NULL);
    pico8_value_t v;

    assert(pico8_ob_table_get_index(t, 1, &v));
    assert(v.kind == PICO8_VALUE_BOOL);
    assert(v.as.boolean == 0);

    pico8_ob_table_free(t);
}

/* "0.00002" parses to raw bit 1 before masking, since 2*65536/100000
 * truncates to 1, so a skipped `band(...,0xffff.fffe)` would show up here
 * as 1 instead of 0. */
static void test_numeric_value_is_masked_with_0xffff_fffe(void) {
    pico8_ob_table_t *t = pico8_ob("n=0.00002,", NULL);
    pico8_value_t v;

    assert(pico8_ob_table_get_name(t, "n", &v));
    assert(v.kind == PICO8_VALUE_FIXED);
    assert(v.as.fixed == 0);

    pico8_ob_table_free(t);
}

/* `qj(kn,mr)`'s merge direction: `mr`'s entries are copied into `kn` and
 * win on conflict, so a key in both the spec and `defaults` keeps the
 * DEFAULTS value. */
static void test_defaults_argument_merge(void) {
    pico8_ob_table_t *defaults = pico8_ob_table_new();
    pico8_ob_table_set_name(defaults, "x", pico8_value_fixed(pico8_fixed_from_int(42)));
    pico8_ob_table_set_name(defaults, "y", pico8_value_fixed(pico8_fixed_from_int(99)));

    pico8_ob_table_t *t = pico8_ob("x=1,z=2,", defaults);
    pico8_value_t v;

    assert(pico8_ob_table_get_name(t, "x", &v));
    assert(v.as.fixed == pico8_fixed_from_int(42));

    assert(pico8_ob_table_get_name(t, "y", &v));
    assert(v.as.fixed == pico8_fixed_from_int(99));

    assert(pico8_ob_table_get_name(t, "z", &v));
    assert(v.as.fixed == pico8_fixed_from_int(2));

    pico8_ob_table_free(t);
    pico8_ob_table_free(defaults);
}

int main(void) {
    test_purely_positional_list();
    test_purely_positional_list_real_bm_spec_with_embedded_newlines();
    test_purely_named_field_list();
    test_nested_value_with_no_registered_constructor();
    test_nested_value_with_registered_constructor();
    test_quoted_string_value();
    test_false_value();
    test_numeric_value_is_masked_with_0xffff_fffe();
    test_defaults_argument_merge();

    printf("test_pico8_ob: all tests passed\n");
    return 0;
}
