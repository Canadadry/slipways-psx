#include "pico8_trade.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


static int g_trade_up = 0;

pico8_qe_t pico8_trade_qe(const pico8_trade_t *self) {
    if (self == NULL || !self->has_qe) {
        return (pico8_qe_t){.kind = PICO8_QE_NIL};
    }
    assert(!(self->qe_is_bool && self->qe != 0) && "no route of src/ holds true: `pico8_qe_t` has no kind for it");
    return self->qe_is_bool ? (pico8_qe_t){.kind = PICO8_QE_FALSE}
                            : (pico8_qe_t){.kind = PICO8_QE_FIXED, .n = self->qe};
}

int pico8_qe_equal(pico8_qe_t a, pico8_qe_t b) {
    return a.kind == b.kind && (a.kind != PICO8_QE_FIXED || a.n == b.n);
}

static const pico8_struct_class_t g_trade_struct_class = {0};

static pico8_trade_t *pico8_trade_struct_new(const pico8_trade_keys_t *keys) {
    pico8_trade_t *self = malloc(sizeof *self);

#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("trade");
#endif
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
    assert((self == NULL ||
            self->header.cls == &g_trade_struct_class) &&
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

int pico8_trade_to(const pico8_trade_t *self, pico8_struct_header_t **out) {
    const pico8_ents_t *lp;

    assert_trade(self);
    if (self == NULL || self->lp == NULL) {
        return 0;
    }
    lp = self->lp;
    if (lp->count == 0) {
        return 0;
    }
    *out = lp->items[lp->count - 1];
    return 1;
}

/* `{np}`/`{jc}` are one-element stack containers, freed once pico8_ents_cat
 * has copied their element into the heap lp/rl it returns; a Lua `{x}`
 * holds zero elements when x is nil, so a NULL contributes nothing. */
pico8_trade_t *pico8_trade_kg(const pico8_trade_t *self, pico8_struct_header_t *np, pico8_struct_header_t *jc) {
    pico8_trade_keys_t keys = {0};
    pico8_ents_t empty_lp;
    pico8_ents_t empty_rl;
    const pico8_ents_t *lp;
    const pico8_ents_t *rl;
    pico8_ents_t np_list;
    pico8_ents_t jc_list;
    pico8_ents_t *new_lp;
    pico8_ents_t *new_rl;

    assert_trade(self);
    if (self == NULL) {
        return NULL;
    }

    pico8_ents_init(&empty_lp);
    pico8_ents_init(&empty_rl);
    lp = self->lp != NULL ? self->lp : &empty_lp;
    rl = self->rl != NULL ? self->rl : &empty_rl;
    /* TROU-24 */
    keys.has_qe = 1;
    keys.qe_is_bool = self->has_qe ? self->qe_is_bool : 1;
    keys.qe = self->has_qe ? self->qe : 0;

    pico8_ents_init(&np_list);
    if (np != NULL) {
        pico8_ents_add(&np_list, np);
    }
    pico8_ents_init(&jc_list);
    if (jc != NULL) {
        pico8_ents_add(&jc_list, jc);
    }

    /* TROU-7 */
    new_lp = malloc(sizeof *new_lp);
    *new_lp = pico8_ents_cat(lp, &np_list);
    new_rl = malloc(sizeof *new_rl);
    *new_rl = pico8_ents_cat(rl, &jc_list);

    pico8_ents_free(&np_list);
    pico8_ents_free(&jc_list);

    keys.lp = new_lp;
    keys.rl = new_rl;
    return pico8_trade_new(&keys);
}

int pico8_trade_eq(const pico8_trade_t *l, const pico8_trade_t *r) {
    const pico8_ents_t *l_lp, *r_lp;
    const pico8_struct_header_t *l_first, *r_first;
    pico8_struct_header_t *l_last, *r_last;

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

    if (l_first != r_first) {
        return 0;
    }
    if (!pico8_trade_to(l, &l_last) || !pico8_trade_to(r, &r_last)) {
        return 0;
    }
    return l_last == r_last;
}
