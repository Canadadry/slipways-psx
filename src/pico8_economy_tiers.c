#include "pico8_economy_tiers.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_entity.h"
#include "pico8_game_data.h"
#include "pico8_planet.h"
#include "pico8_planet_trade.h"
#include "pico8_research.h"
#include "pico8_slipway.h"
#include "pico8_trade.h"

static pico8_fixed_t fixed_min(pico8_fixed_t a, pico8_fixed_t b) {
    return (a < b) ? a : b;
}

typedef void (*cg_visit_fn)(pico8_struct_header_t *e, void *ctx);

static void for_each_cg(const char *tag, cg_visit_fn fn, void *ctx) {
    const pico8_ents_t *list = pico8_cg(tag);
    pico8_ents_iter_t it;
    pico8_struct_header_t *e;
    if (list == NULL) {
        return;
    }
    it = pico8_ents_all(list);
    while (pico8_ents_iter_next(&it, &e)) {
        if (e != NULL) {
            fn(e, ctx);
        }
    }
}

static void bd_reset_visit(pico8_struct_header_t *pt, void *ctx) {
    pico8_jo_t *jo;
    pico8_ents_t *cc;
    (void)ctx;
    if (pt == NULL) {
        return;
    }
    assert(pico8_jo_is_family(pt) && "bd: pt is a planet of cg.jo");
    jo = (pico8_jo_t *)pt;
    if (jo->cc != NULL) {
        pico8_ents_free(jo->cc);
        free(jo->cc);
    }
    /* TROU-7 */
    cc = malloc(sizeof *cc);
    pico8_ents_init(cc);
    jo->cc = cc;
}

static void bd_route_visit(pico8_struct_header_t *pt, void *ctx) {
    pico8_jo_t *jo;
    pico8_ents_t *by;
    pico8_ents_iter_t it;
    pico8_struct_header_t *e;
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

    it = pico8_ents_all(by);
    while (pico8_ents_iter_next(&it, &e)) {
        const pico8_trade_t *route;
        const pico8_ents_t *rl;
        const pico8_ents_t *lp;
        pico8_struct_header_t *to_val;
        int i;

        if (e == NULL) {
            continue;
        }
        assert(e->cls == pico8_trade_struct_class() && "jo.by holds trades only");
        route = (const pico8_trade_t *)e;
        rl = route->rl;
        lp = route->lp;

        if (rl != NULL && lp != NULL) {
            for (i = 0; i < rl->count; i++) {
                const pico8_struct_header_t *w_val = rl->items[i];
                const pico8_jc_t *w;
                const pico8_struct_header_t *planet_val;
                const pico8_jo_t *planet;

                if (w_val == NULL) {
                    continue;
                }
                if (i + 1 >= lp->count) {
                    continue;
                }
                planet_val = lp->items[i + 1];
                if (planet_val == NULL) {
                    continue;
                }
                assert(pico8_jo_is_family(planet_val) && "trade.lp holds planets only");
                planet = (const pico8_jo_t *)planet_val;
                if (planet->id == NULL) {
                    continue;
                }
                assert(w_val->cls == pico8_jc_struct_class() && "trade.rl holds slipways only");
                w = (const pico8_jc_t *)w_val;
                /* A slipway with no `hg` set is skipped; Lua would error
                 * indexing nil here. */
                if (w->hg == NULL) {
                    continue;
                }
                pico8_jc_hg_add(w->hg, planet->id);
            }
        }

        if (pico8_trade_to(route, &to_val) && to_val != NULL) {
            pico8_jo_t *dest;

            assert(pico8_jo_is_family(to_val) && "a trade leads to a planet");
            dest = (pico8_jo_t *)to_val;
            if (dest->cc != NULL) {
                pico8_ents_add(dest->cc, e);
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
                    pico8_ptrs_add(jo->jy, entry);
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
    const pico8_ptrs_t *jy;
    const pico8_ents_t *cc;
    pico8_fixed_t jd = pico8_fixed_from_int(1);
    int i;

    if (pt == NULL) {
        return pico8_fixed_from_int(0);
    }
    assert(pico8_jo_is_family(pt) && "dh: pt is a planet of cg.jo");
    jo = (const pico8_jo_t *)pt;
    if (jo->header.me == NULL || strcmp(jo->header.me, "ly") != 0) {
        return pico8_fixed_from_int(0);
    }

    jy = jo->jy;
    cc = jo->cc;
    if (jy != NULL) {
        for (i = 0; i < jy->count; i++) {
            void *i_val = jy->items[i];
            pico8_jy_t *want;
            pico8_qe_t qe_val;
            int gc = 0;
            int j;

            if (i_val == NULL) {
                continue;
            }
            want = i_val;
            qe_val = pico8_jy_qe(want);
            if (cc != NULL) {
                for (j = 0; j < cc->count; j++) {
                    const pico8_struct_header_t *im_val = cc->items[j];
                    const pico8_trade_t *im;
                    if (im_val == NULL) {
                        continue;
                    }
                    assert(im_val->cls == pico8_trade_struct_class() && "jo.cc holds trades only");
                    im = (const pico8_trade_t *)im_val;
                    /* TROU-24 */
                    if (im->has_qe && pico8_qe_equal(pico8_trade_qe(im), qe_val)) {
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

    if (pt->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_NL)) {
        int count = 0;
        if (jy != NULL) {
            for (i = 0; i < jy->count; i++) {
                void *i_val = jy->items[i];
                const pico8_jy_t *want;
                if (i_val == NULL) {
                    continue;
                }
                want = i_val;
                if (want->has_gc && want->gc > pico8_fixed_from_int(0)) {
                    count++;
                }
            }
        }
        return fixed_min(pico8_fixed_from_int(count), pico8_fixed_from_int(3));
    }

    if (!jo->def->hf) {
        if (jy != NULL && jy->count > 0) {
            void *last_val = jy->items[jy->count - 1];
            if (last_val != NULL) {
                const pico8_jy_t *last = last_val;
                if (last->has_gc) {
                    return fixed_min(pico8_fixed_add(last->gc, pico8_fixed_from_int(1)), pico8_fixed_from_int(4));
                }
            }
        }
        return pico8_fixed_from_int(2);
    }

    {
        const pico8_ents_t *by = jo->by;
        int by_count = (by != NULL) ? by->count : 0;
        int cc_count = (cc != NULL) ? cc->count : 0;
        pico8_fixed_t mid_val =
            pico8_fixed_mid(pico8_fixed_from_int(by_count), pico8_fixed_from_int(cc_count), pico8_fixed_from_int(1));
        return fixed_min(pico8_fixed_from_int(4), pico8_fixed_add(pico8_fixed_from_int(1), mid_val));
    }
}
