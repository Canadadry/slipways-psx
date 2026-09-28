#include "pico8_trade.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_list.h"
#include "pico8_type_tag.h"

static int g_trade_up = 0;

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

static void *pico8_trade_struct_new(const pico8_trade_keys_t *keys) {
    pico8_trade_t *self = malloc(sizeof *self);

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
    if (keys != NULL) {
        assert((!keys->qe_is_bool || keys->qe == 0 || keys->qe == 1) &&
               "trade.qe: a boolean key holds 0 or 1; `pico8_trade_qe` would read any other as a fixed");
        assert((keys->has_qe || (!keys->qe_is_bool && keys->qe == 0)) &&
               "trade.qe: a nil key carries neither a kind nor a value; the member reads 0 for nil");
        self->lp = keys->lp;
        self->rl = keys->rl;
        self->qe = keys->qe;
        self->has_qe = keys->has_qe;
        self->qe_is_bool = keys->qe_is_bool;
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
    g_trade_up = 1;
}

void pico8_trade_shutdown(void) {
    g_trade_up = 0;
}

int pico8_trade_ready(void) {
    return g_trade_up;
}

const pico8_struct_class_t *pico8_trade_struct_class(void) {
    return &g_trade_struct_class;
}

pico8_trade_t *pico8_trade_new(const pico8_trade_keys_t *keys) {
    return pico8_trade_struct_new(keys);
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
    pico8_trade_keys_t keys = {0};
    pico8_list_t empty_lp;
    pico8_list_t empty_rl;
    const pico8_list_t *lp;
    const pico8_list_t *rl;
    pico8_list_t np_list;
    pico8_list_t jc_list;
    pico8_list_t *new_lp;
    pico8_list_t *new_rl;

    assert_trade(self);
    if (self == NULL) {
        return NULL;
    }

    pico8_list_init(&empty_lp);
    pico8_list_init(&empty_rl);
    lp = self->lp != NULL ? self->lp : &empty_lp;
    rl = self->rl != NULL ? self->rl : &empty_rl;
    /* TROU-24 */
    keys.has_qe = 1;
    keys.qe_is_bool = self->has_qe ? self->qe_is_bool : 1;
    keys.qe = self->has_qe ? self->qe : 0;

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

    keys.lp = new_lp;
    keys.rl = new_rl;
    return pico8_trade_new(&keys);
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
