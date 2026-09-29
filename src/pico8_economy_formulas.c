#include "pico8_economy_formulas.h"

#include <assert.h>

#include "pico8_entity.h"
#include "pico8_game_data.h"
#include "pico8_map_tables.h"
#include "pico8_planet.h"
#include "pico8_research.h"
#include "pico8_slipway.h"
#include "pico8_vec.h"
#include "pico8_ref_audit.h"

static pico8_fixed_t fixed_max(pico8_fixed_t a, pico8_fixed_t b) {
    return (a > b) ? a : b;
}

int pico8_qe_map_get(const pico8_qe_map_t *map, pico8_fixed_t qe, pico8_fixed_t *out) {
    int key = pico8_fixed_flr_int(qe);
    int i;

    for (i = 0; map != NULL && i < map->count; i++) {
        if (map->entries[i].qe == key) {
            *out = map->entries[i].v;
            return 1;
        }
    }
    return 0;
}

void pico8_qe_map_set(pico8_qe_map_t *map, pico8_fixed_t qe, pico8_fixed_t v) {
    int key = pico8_fixed_flr_int(qe);
    int i;

    for (i = 0; i < map->count; i++) {
        if (map->entries[i].qe == key) {
            map->entries[i].v = v;
            return;
        }
    }
    assert(map->count < PICO8_QE_MAP_CAP &&
           "a resource key outside r's 13 and the 3 spares -- stop, do not widen");
    map->entries[map->count].qe = key;
    map->entries[map->count].v = v;
    map->count++;
}

typedef void (*cg_visit_fn)(pico8_struct_header_t *e, void *ctx);

static void for_each_cg(const char *tag, cg_visit_fn fn, void *ctx) {
    const pico8_ptrs_t *list = pico8_cg(tag);
    pico8_ptrs_iter_t it;
    void *e;
    if (list == NULL) {
        return;
    }
    it = pico8_ptrs_all(list);
    while (pico8_ptrs_iter_next(&it, &e)) {
        if (e != NULL) {
            fn(e, ctx);
        }
    }
}

void pico8_cm(pico8_qe_map_t *out, pico8_fixed_t jm) {
    int i;

    out->count = 0;
    for (i = 0; i < PICO8_R_COUNT; i++) {
        pico8_qe_map_set(out, pico8_fixed_from_int(pico8_r_table[i].qe), jm);
    }
}

static void bx_visit(pico8_struct_header_t *pt, void *ctx) {
    pico8_qe_map_t *p = (pico8_qe_map_t *)ctx;
    const pico8_jo_t *jo;
    const pico8_kw_t *kw;
    pico8_fixed_t current;

    assert(pico8_jo_is_family(pt) && "bx: pt is a planet of cg.jo");
    jo = (const pico8_jo_t *)pt;
    if (jo->jy == NULL) {
        return;
    }
    kw = jo->kw;
    if (kw == NULL || !kw->has_qe) {
        return;
    }
    if (!pico8_qe_map_get(p, kw->qe, &current)) {
        current = pico8_fixed_from_int(0);
    }
    pico8_qe_map_set(p, kw->qe, pico8_fixed_add(current, kw->mu));
}

void pico8_bx(pico8_qe_map_t *out) {
    pico8_cm(out, pico8_fixed_from_int(0));
    for_each_cg("jo", bx_visit, out);
}

static void ck_visit(pico8_struct_header_t *l, void *ctx) {
    pico8_qe_map_t *d = (pico8_qe_map_t *)ctx;
    const pico8_list_t *jy;
    const pico8_jy_t *last;
    pico8_value_t last_val;
    pico8_fixed_t current;

    assert(pico8_jo_is_family(l) && "ck: l is a planet of cg.qh");
    jy = ((const pico8_jo_t *)l)->jy;
    if (jy == NULL || jy->count == 0) {
        return;
    }
    last_val = jy->items[jy->count - 1];
    if (last_val.kind != PICO8_VALUE_REF || last_val.as.ref == NULL) {
        return;
    }
    last = last_val.as.ref;
    if (!last->row.has_qe) {
        return;
    }
    /* Fallback 1, not 0: `cm(1)` seeded every key of `r`, so an absent one
     * is a resource `r` never listed, treated as that same seed. */
    if (!pico8_qe_map_get(d, last->row.qe, &current)) {
        current = pico8_fixed_from_int(1);
    }
    pico8_qe_map_set(d, last->row.qe, pico8_fixed_sub(current, PICO8_FIXED_LIT(0x00002666, 0.15)));
}

void pico8_ck(pico8_qe_map_t *out) {
    pico8_cm(out, pico8_fixed_from_int(1));
    for_each_cg("qh", ck_visit, out);
}

pico8_fixed_t pico8_dj(const pico8_struct_header_t *pb, const pico8_struct_header_t *to) {
    pico8_vec_t diff = pico8_vec_sub(pico8_jc_end_p(to), pico8_jc_end_p(pb));
    pico8_fixed_t len_sq = pico8_vec_len_sq(diff);
    pico8_fixed_t x = pico8_fixed_div(len_sq, pico8_bq());
    pico8_fixed_t ec = pico8_fixed_pow075(x);
    pico8_fixed_t clamped = fixed_max(PICO8_FIXED_LIT(0x00008000, 0.5), ec);
    pico8_fixed_t scale = pico8_fixed_mul(pico8_bn(), pico8_bk());
    pico8_fixed_t cost = pico8_fixed_flr(pico8_fixed_mul(scale, clamped));
    return cost;
}

pico8_fixed_t pico8_bk(void) {
    const pico8_ptrs_t *jc = pico8_cg("jc");
    int count = (jc != NULL) ? jc->count : 0;
    pico8_fixed_t sq = pico8_fixed_sqrt(pico8_fixed_from_int(count));
    pico8_fixed_t val = pico8_fixed_sub(pico8_fixed_mul(sq, PICO8_FIXED_LIT(0x0000451e, 0.27)), PICO8_FIXED_LIT(0x00001999, 0.1));
    return fixed_max(pico8_fixed_from_int(1), val);
}
