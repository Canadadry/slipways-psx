#include "pico8_trade.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_list.h"
#include "pico8_ref_audit.h"
#include "pico8_type_tag.h"

static pico8_ob_table_t *g_trade_spec = NULL;

static pico8_value_t *trade_member(pico8_trade_t *self, const char *name) {
    if (strcmp(name, "lp") == 0) {
        return &self->lp;
    }
    if (strcmp(name, "rl") == 0) {
        return &self->rl;
    }
    if (strcmp(name, "qe") == 0) {
        return &self->qe;
    }
    return NULL;
}

static int pico8_trade_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_value_t *member = trade_member((pico8_trade_t *)self, name);

    if (member == NULL || (member->kind == PICO8_VALUE_REF && member->as.ref == NULL)) {
        return 0;
    }
    *out_value = *member;
    return 1;
}

static const pico8_struct_class_t g_trade_struct_class = {
    {NULL},
    pico8_trade_field,
    NULL,
};

static void *pico8_trade_struct_new(const pico8_ob_table_t *overrides) {
    pico8_trade_t *self = malloc(sizeof *self);
    int i;

#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("trade");
#endif
    self->header.tag = PICO8_TYPE_TAG_STRUCT;
    self->header.cls = &g_trade_struct_class;
    self->lp = pico8_value_ref(NULL);
    self->rl = pico8_value_ref(NULL);
    self->qe = pico8_value_ref(NULL);
    for (i = 0; overrides != NULL && i < overrides->count; i++) {
        pico8_value_t *member = NULL;
        if (overrides->entries[i].key.kind == PICO8_OB_KEY_NAME) {
            member = trade_member(self, overrides->entries[i].key.name);
        }
        assert(member != NULL && "trade: every measured construction passes only lp, rl, qe; another key "
                                 "would be a field under no rule of plan-3 section 5 -- stop, do not add a member");
        *member = overrides->entries[i].value;
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
    if (self == NULL || self->lp.kind != PICO8_VALUE_REF || self->lp.as.ref == NULL) {
        return 0;
    }
    lp = PICO8_REF_AS_LIST_CONST(self->lp.as.ref);
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
    lp = (self->lp.kind == PICO8_VALUE_REF && self->lp.as.ref != NULL) ? PICO8_REF_AS_LIST_CONST(self->lp.as.ref)
                                                                         : &empty_lp;
    rl = (self->rl.kind == PICO8_VALUE_REF && self->rl.as.ref != NULL) ? PICO8_REF_AS_LIST_CONST(self->rl.as.ref)
                                                                         : &empty_rl;
    /* TROU-24 */
    qe_val = value_is_nil(self->qe) ? pico8_value_bool(0) : self->qe;

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
    if (l->lp.kind != PICO8_VALUE_REF || l->lp.as.ref == NULL || r->lp.kind != PICO8_VALUE_REF ||
        r->lp.as.ref == NULL) {
        return 0;
    }
    l_lp = PICO8_REF_AS_LIST_CONST(l->lp.as.ref);
    r_lp = PICO8_REF_AS_LIST_CONST(r->lp.as.ref);
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
