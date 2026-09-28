#include "pico8_economy_tiers.h"

#include <stdlib.h>
#include <string.h>

#include "pico8_entity.h"
#include "pico8_game_data.h"
#include "pico8_research.h"
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

static int field_truthy(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst == NULL || !pico8_entity_get_field(inst, name, &v)) {
        return 0;
    }
    return economy_truthy(v);
}

static const pico8_list_t *field_list(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return PICO8_REF_AS_LIST_CONST(v.as.ref);
    }
    return NULL;
}

static pico8_struct_header_t *field_ref_instance(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (pico8_struct_header_t *)v.as.ref;
    }
    return NULL;
}

static void set_by_value(pico8_ob_table_t *table, pico8_value_t key, pico8_value_t value) {
    if (table == NULL) {
        return;
    }
    if (key.kind == PICO8_VALUE_FIXED) {
        pico8_ob_table_set_index(table, pico8_fixed_flr_int(key.as.fixed), value);
    } else if (key.kind == PICO8_VALUE_STRING && key.as.string != NULL) {
        pico8_ob_table_set_name(table, key.as.string, value);
    }
}

typedef void (*cg_visit_fn)(pico8_struct_header_t *e, void *ctx);

static void for_each_cg(const char *tag, cg_visit_fn fn, void *ctx) {
    const pico8_list_t *list = pico8_cg(tag);
    pico8_list_iter_t it;
    pico8_value_t e;
    if (list == NULL) {
        return;
    }
    it = pico8_list_all(list);
    while (pico8_list_iter_next(&it, &e)) {
        if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
            fn((pico8_struct_header_t *)e.as.ref, ctx);
        }
    }
}

static void bd_reset_visit(pico8_struct_header_t *pt, void *ctx) {
    pico8_value_t old;
    pico8_list_t *cc;
    (void)ctx;
    if (pt == NULL) {
        return;
    }
    if (pico8_entity_get_field(pt, "cc", &old) && old.kind == PICO8_VALUE_REF && old.as.ref != NULL) {
        pico8_list_free(PICO8_REF_AS_LIST(old.as.ref));
        free(old.as.ref);
    }
    /* TROU-7 */
    cc = malloc(sizeof *cc);
    pico8_list_init(cc);
    pico8_entity_set_field(pt, "cc", pico8_value_ref(cc));
}

static void bd_route_visit(pico8_struct_header_t *pt, void *ctx) {
    pico8_value_t by_val;
    pico8_list_iter_t it;
    pico8_value_t e;
    (void)ctx;
    if (pt == NULL) {
        return;
    }

    if (!pico8_mv(pt, "ib", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &by_val)) {
        by_val = pico8_value_ref(NULL);
    }
    pico8_entity_set_field(pt, "by", by_val);

    if (by_val.kind != PICO8_VALUE_REF || by_val.as.ref == NULL) {
        return;
    }

    it = pico8_list_all(PICO8_REF_AS_LIST_CONST(by_val.as.ref));
    while (pico8_list_iter_next(&it, &e)) {
        pico8_struct_header_t *route;
        const pico8_list_t *rl;
        const pico8_list_t *lp;
        pico8_value_t to_val;
        int i;

        if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
            continue;
        }
        route = (pico8_struct_header_t *)e.as.ref;
        rl = field_list(route, "rl");
        lp = field_list(route, "lp");

        if (rl != NULL && lp != NULL) {
            for (i = 0; i < rl->count; i++) {
                pico8_value_t w_val = rl->items[i];
                pico8_struct_header_t *w;
                pico8_value_t hg_val;
                pico8_value_t planet_val;
                pico8_struct_header_t *planet;
                pico8_value_t id_val;

                if (w_val.kind != PICO8_VALUE_REF || w_val.as.ref == NULL) {
                    continue;
                }
                w = (pico8_struct_header_t *)w_val.as.ref;
                if (i + 1 >= lp->count) {
                    continue;
                }
                planet_val = lp->items[i + 1];
                if (planet_val.kind != PICO8_VALUE_REF || planet_val.as.ref == NULL) {
                    continue;
                }
                planet = (pico8_struct_header_t *)planet_val.as.ref;
                if (!pico8_entity_get_field(planet, "id", &id_val)) {
                    continue;
                }
                /* A slipway with no `hg` set is skipped; Lua would error
                 * indexing nil here. */
                if (!pico8_entity_get_field(w, "hg", &hg_val) || hg_val.kind != PICO8_VALUE_REF ||
                    hg_val.as.ref == NULL) {
                    continue;
                }
                set_by_value((pico8_ob_table_t *)hg_val.as.ref, id_val, pico8_value_bool(1));
            }
        }

        if (pico8_trade_to((const pico8_trade_t *)route, &to_val) && to_val.kind == PICO8_VALUE_REF &&
            to_val.as.ref != NULL) {
            pico8_struct_header_t *dest = (pico8_struct_header_t *)to_val.as.ref;
            pico8_value_t cc_val;
            if (pico8_entity_get_field(dest, "cc", &cc_val) && cc_val.kind == PICO8_VALUE_REF &&
                cc_val.as.ref != NULL) {
                pico8_list_add(PICO8_REF_AS_LIST(cc_val.as.ref), e);
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
    pico8_fixed_t jw;
    pico8_value_t mi_val;
    int has_mi;
    pico8_fixed_t mi = pico8_fixed_from_int(0);

    if (pt == NULL) {
        return;
    }
    jw = pico8_dh(pt);
    has_mi = pico8_entity_get_field(pt, "mi", &mi_val) && mi_val.kind == PICO8_VALUE_FIXED;
    if (has_mi) {
        mi = mi_val.as.fixed;
    }

    if (!has_mi || jw > mi) {
        if (has_mi && mi == pico8_fixed_from_int(1)) {
            pico8_hp_t *hn = (pico8_hp_t *)field_ref_instance(pt, "hn");
            if (hn != NULL && hn->has_dm) {
                pico8_value_t jy_val;
                if (pico8_entity_get_field(pt, "jy", &jy_val) && jy_val.kind == PICO8_VALUE_REF &&
                    jy_val.as.ref != NULL) {
                    pico8_ob_table_t *entry = pico8_ob_table_new();
                    pico8_ob_table_set_name(entry, "qe", pico8_value_fixed(hn->dm));
                    pico8_list_add(PICO8_REF_AS_LIST(jy_val.as.ref), pico8_value_ref(entry));
                }
            }
        }
        pico8_entity_set_field(pt, "mi", pico8_value_fixed(jw));
        mi = jw;
        *jg = 1;
    }

    if (mi != pico8_fixed_from_int(0)) {
        pico8_hp_t *hn = (pico8_hp_t *)field_ref_instance(pt, "hn");
        if (hn != NULL) {
            const pico8_hc_entry_t *po = pico8_hp_po(hn);
            if (po != NULL) {
                int idx = pico8_fixed_flr_int(mi);
                int clamped_idx = (idx < po->count) ? idx : po->count;
                pico8_fixed_t base = pico8_fixed_from_int(0);
                pico8_fixed_t jh = pico8_fixed_from_int(0);
                pico8_value_t jh_val;
                pico8_value_t kw_val;

                if (clamped_idx >= 1) {
                    base = pico8_fixed_from_int(po->values[clamped_idx - 1]);
                }
                if (pico8_entity_get_field(pt, "jh", &jh_val) && jh_val.kind == PICO8_VALUE_FIXED) {
                    jh = jh_val.as.fixed;
                }
                if (pico8_entity_get_field(pt, "kw", &kw_val) && kw_val.kind == PICO8_VALUE_REF &&
                    kw_val.as.ref != NULL) {
                    ((pico8_kw_t *)kw_val.as.ref)->mu = pico8_fixed_add(base, jh);
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
    pico8_value_t me_val;
    const pico8_list_t *jy;
    const pico8_list_t *cc;
    pico8_fixed_t jd = pico8_fixed_from_int(1);
    int i;

    if (pt == NULL) {
        return pico8_fixed_from_int(0);
    }
    if (!pico8_entity_get_field(pt, "me", &me_val) || me_val.kind != PICO8_VALUE_STRING ||
        strcmp(me_val.as.string, "ly") != 0) {
        return pico8_fixed_from_int(0);
    }

    jy = field_list(pt, "jy");
    cc = field_list(pt, "cc");
    if (jy != NULL) {
        for (i = 0; i < jy->count; i++) {
            pico8_value_t i_val = jy->items[i];
            pico8_ob_table_t *i_table;
            pico8_value_t qe_val;
            int gc = 0;
            int j;

            if (i_val.kind != PICO8_VALUE_REF || i_val.as.ref == NULL) {
                continue;
            }
            i_table = (pico8_ob_table_t *)i_val.as.ref;
            if (!pico8_ob_table_get_name(i_table, "qe", &qe_val)) {
                continue;
            }
            if (cc != NULL) {
                for (j = 0; j < cc->count; j++) {
                    pico8_value_t im_val = cc->items[j];
                    pico8_struct_header_t *im;
                    pico8_value_t im_qe;
                    if (im_val.kind != PICO8_VALUE_REF || im_val.as.ref == NULL) {
                        continue;
                    }
                    im = (pico8_struct_header_t *)im_val.as.ref;
                    /* TROU-3 */
                    if (pico8_entity_get_field(im, "qe", &im_qe) && economy_value_equal_content(im_qe, qe_val)) {
                        gc++;
                    }
                }
            }
            pico8_ob_table_set_name(i_table, "gc", pico8_value_fixed(pico8_fixed_from_int(gc)));
            jd = pico8_fixed_mul(jd, pico8_fixed_from_int(gc));
        }
    }

    if (jd == pico8_fixed_from_int(0)) {
        return pico8_fixed_from_int(1);
    }

    if (field_truthy(pt, "nl")) {
        int count = 0;
        if (jy != NULL) {
            for (i = 0; i < jy->count; i++) {
                pico8_value_t i_val = jy->items[i];
                const pico8_ob_table_t *i_table;
                pico8_value_t gc_val;
                if (i_val.kind != PICO8_VALUE_REF || i_val.as.ref == NULL) {
                    continue;
                }
                i_table = (const pico8_ob_table_t *)i_val.as.ref;
                if (pico8_ob_table_get_name(i_table, "gc", &gc_val) && gc_val.kind == PICO8_VALUE_FIXED &&
                    gc_val.as.fixed > pico8_fixed_from_int(0)) {
                    count++;
                }
            }
        }
        return fixed_min(pico8_fixed_from_int(count), pico8_fixed_from_int(3));
    }

    if (!field_truthy(pt, "hf")) {
        if (jy != NULL && jy->count > 0) {
            pico8_value_t last_val = jy->items[jy->count - 1];
            if (last_val.kind == PICO8_VALUE_REF && last_val.as.ref != NULL) {
                const pico8_ob_table_t *last_table = (const pico8_ob_table_t *)last_val.as.ref;
                pico8_value_t gc_val;
                if (pico8_ob_table_get_name(last_table, "gc", &gc_val) && gc_val.kind == PICO8_VALUE_FIXED) {
                    return fixed_min(pico8_fixed_add(gc_val.as.fixed, pico8_fixed_from_int(1)),
                                      pico8_fixed_from_int(4));
                }
            }
        }
        return pico8_fixed_from_int(2);
    }

    {
        const pico8_list_t *by = field_list(pt, "by");
        int by_count = (by != NULL) ? by->count : 0;
        int cc_count = (cc != NULL) ? cc->count : 0;
        pico8_fixed_t mid_val =
            pico8_fixed_mid(pico8_fixed_from_int(by_count), pico8_fixed_from_int(cc_count), pico8_fixed_from_int(1));
        return fixed_min(pico8_fixed_from_int(4), pico8_fixed_add(pico8_fixed_from_int(1), mid_val));
    }
}
