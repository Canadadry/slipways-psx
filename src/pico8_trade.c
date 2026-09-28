#include "pico8_trade.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_list.h"
#include "pico8_type_tag.h"

static pico8_ob_table_t *g_trade_spec = NULL;

pico8_value_t pico8_trade_qe(const pico8_trade_t *self) {
    if (self == NULL || !self->has_qe) {
        return pico8_value_ref(NULL);
    }
    return self->qe_is_bool ? pico8_value_bool(self->qe) : pico8_value_fixed(self->qe);
}

static int pico8_trade_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_trade_t *t = self;

    if (strcmp(name, "lp") == 0) {
        if (t->lp == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(t->lp);
        return 1;
    }
    if (strcmp(name, "rl") == 0) {
        if (t->rl == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(t->rl);
        return 1;
    }
    if (strcmp(name, "qe") == 0) {
        if (!t->has_qe) {
            return 0;
        }
        *out_value = pico8_trade_qe(t);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_trade_struct_class = {
    {NULL},
    pico8_trade_field,
    NULL,
};

static pico8_list_t *trade_list_key(pico8_value_t value) {
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return NULL;
    }
    assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_LIST &&
           "trade.lp/rl: a key neither nil nor a list does not fit the member's type -- stop, do not widen the member");
    return value.as.ref;
}

static void *pico8_trade_struct_new(const pico8_ob_table_t *overrides) {
    pico8_trade_t *self = malloc(sizeof *self);
    int i;

#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("trade");
#endif
    self->header.tag = PICO8_TYPE_TAG_STRUCT;
    self->header.cls = &g_trade_struct_class;
    self->lp = NULL;
    self->rl = NULL;
    self->qe = 0;
    self->has_qe = 0;
    self->qe_is_bool = 0;
    for (i = 0; overrides != NULL && i < overrides->count; i++) {
        pico8_ob_key_t key = overrides->entries[i].key;
        pico8_value_t value = overrides->entries[i].value;

        if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "lp") == 0) {
            self->lp = trade_list_key(value);
        } else if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "rl") == 0) {
            self->rl = trade_list_key(value);
        } else if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "qe") == 0) {
            if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
                self->qe = 0;
                self->has_qe = 0;
                self->qe_is_bool = 0;
            } else if (value.kind == PICO8_VALUE_FIXED) {
                self->qe = value.as.fixed;
                self->has_qe = 1;
                self->qe_is_bool = 0;
            } else {
                assert(value.kind == PICO8_VALUE_BOOL &&
                       "trade.qe: a key neither nil, fixed nor boolean does not fit the member's type "
                       "-- stop, do not widen the member");
                self->qe = value.as.boolean;
                self->has_qe = 1;
                self->qe_is_bool = 1;
            }
        } else {
            assert(0 && "trade: every construction passes only lp, rl, qe; another key has no member "
                        "-- stop, do not add one");
        }
    }
    return self;
}

static void assert_trade(const pico8_trade_t *self) {
    assert((self == NULL || (pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT &&
                             self->header.cls == &g_trade_struct_class)) &&
           "trade: every by/cc element and every proposal's t is a route; anything else would be read "
           "through the wrong layout");
}

void pico8_trade_init(void) {
    pico8_trade_shutdown();
    g_trade_spec = pico8_ob("", NULL);
}

void pico8_trade_shutdown(void) {
    pico8_ob_table_free(g_trade_spec);
    g_trade_spec = NULL;
}

int pico8_trade_ready(void) {
    return g_trade_spec != NULL;
}

int pico8_trade_default(const char *name, pico8_value_t *out_value) {
    return g_trade_spec != NULL && pico8_ob_table_get_name(g_trade_spec, name, out_value);
}

const pico8_struct_class_t *pico8_trade_struct_class(void) {
    return &g_trade_struct_class;
}

pico8_trade_t *pico8_trade_new(const pico8_ob_table_t *overrides) {
    return pico8_trade_struct_new(overrides);
}

int pico8_trade_to(const pico8_trade_t *self, pico8_value_t *out_value) {
    const pico8_list_t *lp;

    assert_trade(self);
    if (self == NULL || self->lp == NULL) {
        return 0;
    }
    lp = self->lp;
    if (lp->count == 0) {
        return 0;
    }
    *out_value = lp->items[lp->count - 1];
    return 1;
}

/* A Lua `{x}` holds zero elements when x is nil, so `{jc}` must contribute
 * nothing to `rl` when trade:kg is called with no slipway -- as
 * pico8_planet_trade.c does. */
static int value_is_nil(pico8_value_t v) {
    return v.kind == PICO8_VALUE_REF && v.as.ref == NULL;
}

/* `{np}`/`{jc}` are built as one-element stack lists, freed once pico8_la has
 * copied their element into the heap lp/rl lists it returns. */
pico8_trade_t *pico8_trade_kg(const pico8_trade_t *self, pico8_value_t np, pico8_value_t jc) {
    pico8_value_t qe_val;
    pico8_list_t empty_lp;
    pico8_list_t empty_rl;
    const pico8_list_t *lp;
    const pico8_list_t *rl;
    pico8_list_t np_list;
    pico8_list_t jc_list;
    pico8_list_t *new_lp;
    pico8_list_t *new_rl;
    pico8_ob_table_t *overrides;
    pico8_trade_t *result;

    assert_trade(self);
    if (self == NULL) {
        return NULL;
    }

    pico8_list_init(&empty_lp);
    pico8_list_init(&empty_rl);
    lp = self->lp != NULL ? self->lp : &empty_lp;
    rl = self->rl != NULL ? self->rl : &empty_rl;
    /* TROU-24 */
    qe_val = self->has_qe ? pico8_trade_qe(self) : pico8_value_bool(0);

    pico8_list_init(&np_list);
    if (!value_is_nil(np)) {
        pico8_list_add(&np_list, np);
    }
    pico8_list_init(&jc_list);
    if (!value_is_nil(jc)) {
        pico8_list_add(&jc_list, jc);
    }

    /* TROU-7 */
    new_lp = malloc(sizeof *new_lp);
    *new_lp = pico8_la(lp, &np_list);
    new_rl = malloc(sizeof *new_rl);
    *new_rl = pico8_la(rl, &jc_list);

    pico8_list_free(&np_list);
    pico8_list_free(&jc_list);

    overrides = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides, "lp", pico8_value_ref(new_lp));
    pico8_ob_table_set_name(overrides, "rl", pico8_value_ref(new_rl));
    pico8_ob_table_set_name(overrides, "qe", qe_val);

    result = pico8_trade_new(overrides);
    pico8_ob_table_free(overrides);
    return result;
}

int pico8_trade_eq(const pico8_trade_t *l, const pico8_trade_t *r) {
    const pico8_list_t *l_lp, *r_lp;
    pico8_value_t l_first, r_first;
    pico8_value_t l_last, r_last;

    assert_trade(l);
    assert_trade(r);
    if (l == NULL || r == NULL) {
        return 0;
    }
    if (l->lp == NULL || r->lp == NULL) {
        return 0;
    }
    l_lp = l->lp;
    r_lp = r->lp;
    if (l_lp->count == 0 || r_lp->count == 0) {
        return 0;
    }
    l_first = l_lp->items[0];
    r_first = r_lp->items[0];

    if (!pico8_value_equal(l_first, r_first)) {
        return 0;
    }
    if (!pico8_trade_to(l, &l_last) || !pico8_trade_to(r, &r_last)) {
        return 0;
    }
    return pico8_value_equal(l_last, r_last);
}
