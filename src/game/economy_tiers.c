#include "game/economy_tiers.h"

#include <assert.h>
#include <string.h>

#include "engine/entity_registry.h"
#include "game/game_data.h"
#include "game/planet.h"
#include "game/planet_trade.h"
#include "game/research.h"
#include "game/slipway.h"
#include "game/trade.h"
#include "engine/game_zone.h"

static p8_fixed_t fixed_min(p8_fixed_t a, p8_fixed_t b) {
    return (a < b) ? a : b;
}

typedef void (*cg_visit_fn)(entity_t *e, void *ctx);

static void for_each_cg(const char *tag, cg_visit_fn fn, void *ctx) {
    const entity_list_t *list = entities_by_tag(tag);
    entity_list_iter_t it;
    entity_t *e;
    if (list == NULL) {
        return;
    }
    it = entity_list_all(list);
    while (entity_list_iter_next(&it, &e)) {
        if (e != NULL) {
            fn(e, ctx);
        }
    }
}

static void bd_reset_visit(entity_t *pt, void *ctx) {
    pico8_jo_t *jo;
    entity_list_t *cc;
    (void)ctx;
    if (pt == NULL) {
        return;
    }
    assert(pico8_jo_is_family(pt) && "bd: pt is a planet of cg.jo");
    jo = (pico8_jo_t *)pt;
    if (jo->cc != NULL) {
        entity_list_clear(jo->cc);
        return;
    }
    cc = (entity_list_t *)allocator_alloc(game_zone(), sizeof *cc);
    entity_list_init(cc, game_zone());
    jo->cc = cc;
}

static void bd_route_visit(entity_t *pt, void *ctx) {
    pico8_jo_t *jo;
    entity_list_t *by;
    entity_list_iter_t it;
    entity_t *e;
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

    it = entity_list_all(by);
    while (entity_list_iter_next(&it, &e)) {
        const pico8_trade_t *route;
        const entity_list_t *rl;
        const entity_list_t *lp;
        entity_t *to_val;
        int i;

        if (e == NULL) {
            continue;
        }
        assert(e->class == pico8_trade_struct_class() && "jo.by holds trades only");
        route = (const pico8_trade_t *)e;
        rl = route->rl;
        lp = route->lp;

        if (rl != NULL && lp != NULL) {
            for (i = 0; i < rl->count; i++) {
                const entity_t *w_val = rl->items[i];
                const pico8_jc_t *w;
                const entity_t *planet_val;
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
                assert(w_val->class == pico8_jc_struct_class() && "trade.rl holds slipways only");
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
                entity_list_add(dest->cc, e);
            }
        }
    }
}

void pico8_bd(void) {
    for_each_cg("planet", bd_reset_visit, NULL);
    for_each_cg("planet", bd_route_visit, NULL);
}

static void bb_visit(entity_t *pt, void *ctx) {
    int *jg = (int *)ctx;
    pico8_jo_t *jo;
    p8_fixed_t jw;
    int has_mi;
    p8_fixed_t mi = p8_fixed_from_int(0);

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
        if (has_mi && mi == p8_fixed_from_int(1)) {
            pico8_hp_t *hn = jo->hn;
            if (hn != NULL && hn->has_dm) {
                if (jo->jy != NULL) {
                    pico8_jy_t *entry = pico8_jy_new(game_zone(), &hn->dm);
                    pointer_list_add(jo->jy, entry);
                }
            }
        }
        jo->mi = jw;
        jo->has_mi = 1;
        mi = jw;
        *jg = 1;
    }

    if (mi != p8_fixed_from_int(0)) {
        pico8_hp_t *hn = jo->hn;
        if (hn != NULL) {
            const pico8_hc_entry_t *po = pico8_hp_po(hn);
            if (po != NULL) {
                int idx = p8_fixed_flr_int(mi);
                int clamped_idx = (idx < po->count) ? idx : po->count;
                p8_fixed_t base = p8_fixed_from_int(0);
                p8_fixed_t jh = p8_fixed_from_int(0);

                if (clamped_idx >= 1) {
                    base = p8_fixed_from_int(po->values[clamped_idx - 1]);
                }
                if (jo->has_jh) {
                    jh = jo->jh;
                }
                if (jo->kw != NULL) {
                    jo->kw->mu = p8_fixed_add(base, jh);
                }
            }
        }
    }
}

int pico8_bb(void) {
    int jg = 0;
    for_each_cg("planet", bb_visit, &jg);
    return jg;
}

p8_fixed_t pico8_dh(const entity_t *pt) {
    const pico8_jo_t *jo;
    const pointer_list_t *jy;
    const entity_list_t *cc;
    p8_fixed_t jd = p8_fixed_from_int(1);
    int i;

    if (pt == NULL) {
        return p8_fixed_from_int(0);
    }
    assert(pico8_jo_is_family(pt) && "dh: pt is a planet of cg.jo");
    jo = (const pico8_jo_t *)pt;
    if (jo->header.state == NULL || strcmp(jo->header.state, "settled") != 0) {
        return p8_fixed_from_int(0);
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
                    const entity_t *im_val = cc->items[j];
                    const pico8_trade_t *im;
                    if (im_val == NULL) {
                        continue;
                    }
                    assert(im_val->class == pico8_trade_struct_class() && "jo.cc holds trades only");
                    im = (const pico8_trade_t *)im_val;
                    /* TROU-24 */
                    if (im->has_qe && pico8_qe_equal(pico8_trade_qe(im), qe_val)) {
                        gc++;
                    }
                }
            }
            want->has_gc = 1;
            want->gc = p8_fixed_from_int(gc);
            jd = p8_fixed_mul(jd, p8_fixed_from_int(gc));
        }
    }

    if (jd == p8_fixed_from_int(0)) {
        return p8_fixed_from_int(1);
    }

    if (pt->class->lineage & LINEAGE(LINEAGE_TRADING_HUB)) {
        int count = 0;
        if (jy != NULL) {
            for (i = 0; i < jy->count; i++) {
                void *i_val = jy->items[i];
                const pico8_jy_t *want;
                if (i_val == NULL) {
                    continue;
                }
                want = i_val;
                if (want->has_gc && want->gc > p8_fixed_from_int(0)) {
                    count++;
                }
            }
        }
        return fixed_min(p8_fixed_from_int(count), p8_fixed_from_int(3));
    }

    if (!jo->def->hf) {
        if (jy != NULL && jy->count > 0) {
            void *last_val = jy->items[jy->count - 1];
            if (last_val != NULL) {
                const pico8_jy_t *last = last_val;
                if (last->has_gc) {
                    return fixed_min(p8_fixed_add(last->gc, p8_fixed_from_int(1)), p8_fixed_from_int(4));
                }
            }
        }
        return p8_fixed_from_int(2);
    }

    {
        const entity_list_t *by = jo->by;
        int by_count = (by != NULL) ? by->count : 0;
        int cc_count = (cc != NULL) ? cc->count : 0;
        p8_fixed_t mid_val =
            p8_fixed_mid(p8_fixed_from_int(by_count), p8_fixed_from_int(cc_count), p8_fixed_from_int(1));
        return fixed_min(p8_fixed_from_int(4), p8_fixed_add(p8_fixed_from_int(1), mid_val));
    }
}
