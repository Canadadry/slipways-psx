#include "../src/pico8_ob.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_fixed.h"
#include "../src/pico8_value.h"

/* Chunk 8's own scope note (see the PRD and pico8_ob.h/.c): ob()/fi() get
 * per-parsing-rule unit tests here (unlike kv:jr/hv/cv/cg/dc/mv/lt/he,
 * which the PRD explicitly defers to source/port comparison plus a later
 * smoke test) -- this parser is dense, stateful, character-by-character
 * logic, exactly the kind of thing a hand-computed-value test catches a
 * regression in immediately. */

/* A purely positional list, mirroring pico8/lua.lua:169-172's own
 * bm=ob([["kj","ev","hd","hk","hb",]]) spec string (single-line form). */
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

/* Bonus, beyond the PRD's required test list: the REAL bm=ob([[...]])
 * literal verbatim, embedded newlines and all (pico8/lua.lua:169-172) --
 * a direct check against the real cart source, exercising the
 * newline-always-skips / space-only-skips-outside-quotes asymmetry this
 * module's top comment documents (see pico8_ob.c). */
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

/* A purely named-field list, mirroring an o(x=...,y=...,c=...)-style
 * spec (e.g. pico8/lua.lua:143-152's er=ob([[o(x=-1,y=-1,c=0),...]])
 * entries' own inner content). */
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

/* A nested o(...) value with no registered constructor -- the common
 * case (pi has no 'o' entry in the real source either): the recursive
 * parse's own result table is used directly as the value. */
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

/* Trivial test-only "constructor" (see pico8_ob.h's comment on why this
 * chunk registers no real ones): sums its first two positional args,
 * just enough to prove oo()'s "unpack positional fields as arguments"
 * behavior actually reaches a registered function. */
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

/* A numeric value confirming the 0xffff.fffe masking is actually
 * applied: "0.00002" parses to raw fixed-point bit 1 BEFORE masking
 * (2*65536/100000 truncates to 1), so if band(...,0xffff.fffe) -- which
 * clears only the least-significant bit -- were skipped, this would
 * observe 1, not 0. */
static void test_numeric_value_is_masked_with_0xffff_fffe(void) {
    pico8_ob_table_t *t = pico8_ob("n=0.00002,", NULL);
    pico8_value_t v;

    assert(pico8_ob_table_get_name(t, "n", &v));
    assert(v.kind == PICO8_VALUE_FIXED);
    assert(v.as.fixed == 0);

    pico8_ob_table_free(t);
}

/* A defaults-argument merge case -- and, deliberately, the case that
 * pins down qj()'s REAL merge direction (see pico8_ob.c's own comment on
 * this in pico8_ob()): a key present in BOTH the parsed spec and
 * `defaults` ends up holding the DEFAULTS value ("x" here), not the
 * parsed one, because qj(kn,mr) copies mr's entries INTO kn, overwriting
 * on conflict. A key only in `defaults` is simply added ("y"), and a key
 * only in the parsed spec is left untouched ("z"). */
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
