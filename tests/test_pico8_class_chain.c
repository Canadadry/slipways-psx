/* `classe.nom` read off a class value: its own defaults, then each parent's. */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/pico8_class.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_value.h"

static void test_reads_up_the_chain(void) {
    pico8_class_t *root;
    pico8_class_t *overrides;
    pico8_class_t *inherits;
    pico8_value_t v;
    pico8_value_t untouched = pico8_value_fixed(pico8_fixed_from_int(77));

    pico8_class_registry_clear();
    root = pico8_class_declare(NULL, "\nme=\"kz\",\ns=174,\nod=15,\n");
    overrides = pico8_class_declare(root, "\ns=170,\nrr=\"slipgates\",\n");
    inherits = pico8_class_declare(overrides, "\nod=10,\n");

    assert(pico8_class_get_field(root, "s", &v) && v.kind == PICO8_VALUE_FIXED &&
           v.as.fixed == pico8_fixed_from_int(174));
    assert(pico8_class_get_field(overrides, "s", &v) && v.as.fixed == pico8_fixed_from_int(170));
    assert(pico8_class_get_field(inherits, "s", &v) && v.as.fixed == pico8_fixed_from_int(170));
    assert(pico8_class_get_field(inherits, "od", &v) && v.as.fixed == pico8_fixed_from_int(10));
    assert(pico8_class_get_field(overrides, "od", &v) && v.as.fixed == pico8_fixed_from_int(15));
    assert(pico8_class_get_field(inherits, "me", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, "kz") == 0);
    assert(pico8_class_get_field(inherits, "rr", &v) && v.kind == PICO8_VALUE_STRING &&
           strcmp(v.as.string, "slipgates") == 0);

    v = untouched;
    assert(!pico8_class_get_field(root, "rr", &v));
    assert(!pico8_class_get_field(inherits, "et", &v));
    assert(v.kind == PICO8_VALUE_FIXED && v.as.fixed == untouched.as.fixed);

    pico8_class_free(inherits);
    pico8_class_free(overrides);
    pico8_class_free(root);
    pico8_class_registry_clear();
}

int main(void) {
    test_reads_up_the_chain();
    printf("test_pico8_class_chain: all tests passed\n");
    return 0;
}
