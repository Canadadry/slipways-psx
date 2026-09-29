#include "pico8_economy_tiers.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_entity.h"
#include "pico8_game_data.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_planet_trade.h"
#include "pico8_research.h"
#include "pico8_slipway.h"
#include "pico8_trade.h"
#include "pico8_ref_audit.h"

static int economy_truthy(pico8_value_t v) {
    return !(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

static int economy_value_equal_content(pico8_value_t a, pico8_value_t b) {
    if (a.kind == PICO8_VALUE_STRING && b.kind == PICO8_VALUE_STRING) {
        return strcmp(a.as.string, b.as.string) == 0;
    }
    return pico8_value_equal(a, b);
}

static pico8_fixed_t fixed_min(pico8_fixed_t a, pico8_fixed_t b) {
    return (a < b) ? a : b;
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

static void bd_reset_visit(pico8_struct_header_t *pt, void *ctx) {
    pico8_jo_t *jo;
    pico8_list_t *cc;
    (void)ctx;
    if (pt == NULL) {
        return;
    }
    assert(pico8_jo_is_family(pt) && "bd: pt is a planet of cg.jo");
    jo = (pico8_jo_t *)pt;
    if (jo->cc != NULL) {
        pico8_list_free(jo->cc);
        free(jo->cc);
    }
    /* TROU-7 */
    cc = malloc(sizeof *cc);
    pico8_list_init(cc);
    jo->cc = cc;
}

static void bd_route_visit(pico8_struct_header_t *pt, void *ctx) {
    pico8_jo_t *jo;
    pico8_list_t *by;
    pico8_list_iter_t it;
    pico8_value_t e;
    (void)ctx;
    if (pt == NULL) {
        return;
    }

    assert(pico8_jo_is_family(pt) && "ib: pt is a planet of cg.jo");
    jo = (pico8_jo_t *)pt;
    by = pico8_jo_ib(jo);
    jo->by = by;

    if (by == NULL) {
        return;
    }

    it = pico8_list_all(by);
    while (pico8_list_iter_next(&it, &e)) {
        const pico8_trade_t *route;
        const pico8_list_t *rl;
        const pico8_list_t *lp;
        pico8_value_t to_val;
        int i;

        if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
            continue;
        }
        assert(((const pico8_struct_header_t *)e.as.ref)->cls == pico8_trade_struct_class() &&
               "jo.by holds trades only");
        route = e.as.ref;
        rl = route->rl;
        lp = route->lp;

        if (rl != NULL && lp != NULL) {
            for (i = 0; i < rl->count; i++) {
                pico8_value_t w_val = rl->items[i];
                const pico8_jc_t *w;
                pico8_value_t planet_val;
                const pico8_jo_t *planet;

                if (w_val.kind != PICO8_VALUE_REF || w_val.as.ref == NULL) {
                    continue;
                }
                if (i + 1 >= lp->count) {
                    continue;
                }
                planet_val = lp->items[i + 1];
                if (planet_val.kind != PICO8_VALUE_REF || planet_val.as.ref == NULL) {
                    continue;
                }
                assert(pico8_jo_is_family(planet_val.as.ref) && "trade.lp holds planets only");
                planet = planet_val.as.ref;
                if (planet->id == NULL) {
                    continue;
                }
                assert(((const pico8_struct_header_t *)w_val.as.ref)->cls == pico8_jc_struct_class() &&
                       "trade.rl holds slipways only");
                w = w_val.as.ref;
                /* A slipway with no `hg` set is skipped; Lua would error
                 * indexing nil here. */
                if (w->hg == NULL) {
                    continue;
                }
                pico8_jc_hg_add(w->hg, pico8_value_string(planet->id));
            }
        }

        if (pico8_trade_to(route, &to_val) && to_val.kind == PICO8_VALUE_REF &&
            to_val.as.ref != NULL) {
            pico8_jo_t *dest;

            assert(pico8_jo_is_family(to_val.as.ref) && "a trade leads to a planet");
            dest = to_val.as.ref;
            if (dest->cc != NULL) {
                pico8_list_add(dest->cc, e);
            }
        }
    }
}

void pico8_bd(void) {
    for_each_cg("jo", bd_reset_visit, NULL);
    for_each_cg("jo", bd_route_visit, NULL);
}

static void bb_visit(pico8_struct_header_t *pt, void *ctx) {
    int *jg = (int *)ctx;
    pico8_jo_t *jo;
    pico8_fixed_t jw;
    int has_mi;
    pico8_fixed_t mi = pico8_fixed_from_int(0);

    if (pt == NULL) {
        return;
    }
    jw = pico8_dh(pt);
    assert(pico8_jo_is_family(pt) && "bb: pt is a planet of cg.jo");
    jo = (pico8_jo_t *)pt;
    has_mi = jo->has_mi;
    if (has_mi) {
        mi = jo->mi;
    }

    if (!has_mi || jw > mi) {
        if (has_mi && mi == pico8_fixed_from_int(1)) {
            pico8_hp_t *hn = jo->hn;
            if (hn != NULL && hn->has_dm) {
                if (jo->jy != NULL) {
                    pico8_jy_t *entry = pico8_jy_new(&hn->dm);
                    pico8_list_add(jo->jy, pico8_value_ref(entry));
                }
            }
        }
        jo->mi = jw;
        jo->has_mi = 1;
        mi = jw;
        *jg = 1;
    }

    if (mi != pico8_fixed_from_int(0)) {
        pico8_hp_t *hn = jo->hn;
        if (hn != NULL) {
            const pico8_hc_entry_t *po = pico8_hp_po(hn);
            if (po != NULL) {
                int idx = pico8_fixed_flr_int(mi);
                int clamped_idx = (idx < po->count) ? idx : po->count;
                pico8_fixed_t base = pico8_fixed_from_int(0);
                pico8_fixed_t jh = pico8_fixed_from_int(0);

                if (clamped_idx >= 1) {
                    base = pico8_fixed_from_int(po->values[clamped_idx - 1]);
                }
                if (jo->has_jh) {
                    jh = jo->jh;
                }
                if (jo->kw != NULL) {
                    jo->kw->mu = pico8_fixed_add(base, jh);
                }
            }
        }
    }
}

int pico8_bb(void) {
    int jg = 0;
    for_each_cg("jo", bb_visit, &jg);
    return jg;
}

pico8_fixed_t pico8_dh(const pico8_struct_header_t *pt) {
    const pico8_jo_t *jo;
    const pico8_list_t *jy;
    const pico8_list_t *cc;
    pico8_fixed_t jd = pico8_fixed_from_int(1);
    int i;

    if (pt == NULL) {
        return pico8_fixed_from_int(0);
    }
    assert(pico8_jo_is_family(pt) && "dh: pt is a planet of cg.jo");
    jo = (const pico8_jo_t *)pt;
    if (jo->ke.me == NULL || strcmp(jo->ke.me, "ly") != 0) {
        return pico8_fixed_from_int(0);
    }

    jy = jo->jy;
    cc = jo->cc;
    if (jy != NULL) {
        for (i = 0; i < jy->count; i++) {
            pico8_value_t i_val = jy->items[i];
            pico8_jy_t *want;
            pico8_value_t qe_val;
            int gc = 0;
            int j;

            if (i_val.kind != PICO8_VALUE_REF || i_val.as.ref == NULL) {
                continue;
            }
            want = i_val.as.ref;
            qe_val = pico8_jy_qe(want);
            if (cc != NULL) {
                for (j = 0; j < cc->count; j++) {
                    pico8_value_t im_val = cc->items[j];
                    const pico8_trade_t *im;
                    if (im_val.kind != PICO8_VALUE_REF || im_val.as.ref == NULL) {
                        continue;
                    }
                    assert(((const pico8_struct_header_t *)im_val.as.ref)->cls == pico8_trade_struct_class() &&
                           "jo.cc holds trades only");
                    im = im_val.as.ref;
                    /* TROU-3 */
                    if (im->has_qe && economy_value_equal_content(pico8_trade_qe(im), qe_val)) {
                        gc++;
                    }
                }
            }
            want->has_gc = 1;
            want->gc = pico8_fixed_from_int(gc);
            jd = pico8_fixed_mul(jd, pico8_fixed_from_int(gc));
        }
    }

    if (jd == pico8_fixed_from_int(0)) {
        return pico8_fixed_from_int(1);
    }

    if (pico8_nl_of(pt) != NULL) {
        int count = 0;
        if (jy != NULL) {
            for (i = 0; i < jy->count; i++) {
                pico8_value_t i_val = jy->items[i];
                const pico8_jy_t *want;
                if (i_val.kind != PICO8_VALUE_REF || i_val.as.ref == NULL) {
                    continue;
                }
                want = i_val.as.ref;
                if (want->has_gc && want->gc > pico8_fixed_from_int(0)) {
                    count++;
                }
            }
        }
        return fixed_min(pico8_fixed_from_int(count), pico8_fixed_from_int(3));
    }

    if (!economy_truthy(pico8_jo_hf(jo))) {
        if (jy != NULL && jy->count > 0) {
            pico8_value_t last_val = jy->items[jy->count - 1];
            if (last_val.kind == PICO8_VALUE_REF && last_val.as.ref != NULL) {
                const pico8_jy_t *last = last_val.as.ref;
                if (last->has_gc) {
                    return fixed_min(pico8_fixed_add(last->gc, pico8_fixed_from_int(1)), pico8_fixed_from_int(4));
                }
            }
        }
        return pico8_fixed_from_int(2);
    }

    {
        const pico8_list_t *by = jo->by;
        int by_count = (by != NULL) ? by->count : 0;
        int cc_count = (cc != NULL) ? cc->count : 0;
        pico8_fixed_t mid_val =
            pico8_fixed_mid(pico8_fixed_from_int(by_count), pico8_fixed_from_int(cc_count), pico8_fixed_from_int(1));
        return fixed_min(pico8_fixed_from_int(4), pico8_fixed_add(pico8_fixed_from_int(1), mid_val));
    }
}
