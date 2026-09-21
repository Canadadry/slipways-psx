#include "pico8_slipway_display.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_entity_geom.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

static int pico8_jc_display_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static pico8_value_t pico8_jc_display_field_value(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v)) {
        return v;
    }
    return pico8_value_ref(NULL);
}

static pico8_instance_t *pico8_jc_display_field_instance(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v = pico8_jc_display_field_value(inst, name);
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (pico8_instance_t *)v.as.ref;
    }
    return NULL;
}

static pico8_fixed_t pico8_jc_display_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v = pico8_jc_display_field_value(inst, name);
    if (v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static pico8_vec_t pico8_jc_display_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t pico8_jc_display_field_vec(const pico8_instance_t *inst, const char *name) {
    return pico8_jc_display_value_vec(pico8_jc_display_field_value(inst, name));
}

static pico8_vec_t pico8_jc_display_table_field_vec(const pico8_ob_table_t *table, const char *name) {
    pico8_value_t v = pico8_value_ref(NULL);
    if (table != NULL) {
        pico8_ob_table_get_name(table, name, &v);
    }
    return pico8_jc_display_value_vec(v);
}

static pico8_fixed_t pico8_jc_display_table_field_fixed(const pico8_ob_table_t *table, const char *name) {
    pico8_value_t v;
    if (table != NULL && pico8_ob_table_get_name(table, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static int pico8_jc_display_get_by_value(const pico8_ob_table_t *table, pico8_value_t key, pico8_value_t *out) {
    if (table == NULL) {
        return 0;
    }
    if (key.kind == PICO8_VALUE_FIXED) {
        return pico8_ob_table_get_index(table, (int)pico8_fixed_to_double(key.as.fixed), out);
    }
    if (key.kind == PICO8_VALUE_STRING && key.as.string != NULL) {
        return pico8_ob_table_get_name(table, key.as.string, out);
    }
    return 0;
}

static int pico8_jc_display_to_int(pico8_fixed_t x) {
    return (int)pico8_fixed_to_double(x);
}

void pico8_jc_ch(pico8_instance_t *self, const pico8_instance_t *f, const pico8_instance_t *t) {
    pico8_value_t hg_val;

    if (self == NULL) {
        return;
    }
    if (pico8_fixed_rnd_default() >= pico8_fixed_from_double(0.08)) {
        return;
    }

    hg_val = pico8_jc_display_field_value(self, "hg");
    if (hg_val.kind == PICO8_VALUE_REF && hg_val.as.ref != NULL) {
        pico8_value_t t_id = pico8_jc_display_field_value(t, "id");
        pico8_value_t hg_entry;
        if (pico8_jc_display_get_by_value((const pico8_ob_table_t *)hg_val.as.ref, t_id, &hg_entry) &&
            pico8_jc_display_truthy(hg_entry)) {
            pico8_value_t ts_val = pico8_jc_display_field_value(self, "ts");
            if (ts_val.kind == PICO8_VALUE_REF && ts_val.as.ref != NULL) {
                pico8_vec_t f_ra = pico8_jc_display_field_vec(f, "ra");
                pico8_vec_t t_ra = pico8_jc_display_field_vec(t, "ra");
                pico8_vec_t d = pico8_vec_sub(t_ra, f_ra);
                pico8_vec_t v = pico8_vec_nn(d);
                pico8_ob_table_t *particle = pico8_ob_table_new();
                pico8_vec_t *ra_heap = malloc(sizeof *ra_heap);
                pico8_vec_t *v_heap = malloc(sizeof *v_heap);

                *ra_heap = pico8_vec_add(f_ra, pico8_vec_pa(v));
                pico8_ob_table_set_name(particle, "ra", pico8_value_ref(ra_heap));
                *v_heap = v;
                pico8_ob_table_set_name(particle, "v", pico8_value_ref(v_heap));
                pico8_ob_table_set_name(particle, "l", pico8_value_fixed(pico8_vec_qx(d)));

                pico8_list_add((pico8_list_t *)ts_val.as.ref, pico8_value_ref(particle));
            }
        }
    }
}

void pico8_jc_lg(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_fixed_t l,
                  pico8_fixed_t h, pico8_fixed_t s, int c) {
    pico8_instance_t *qu;
    pico8_instance_t *re;
    pico8_fixed_t t;
    pico8_vec_t qu_pt;
    pico8_vec_t re_pt;
    pico8_fixed_t dx;

    if (self == NULL) {
        return;
    }

    qu = pico8_jc_display_field_instance(self, "qu");
    re = pico8_jc_display_field_instance(self, "re");
    t = pico8_jc_display_field_fixed(self, "t");

    qu_pt = pico8_vec_add(pico8_jc_display_field_vec(qu, "ra"),
                           pico8_vec_qm(pico8_fixed_from_double(1.5), pico8_fixed_div(t, pico8_fixed_from_int(126))));
    re_pt = pico8_vec_add(pico8_jc_display_field_vec(re, "ra"),
                           pico8_vec_qm(pico8_fixed_from_double(1.5), pico8_fixed_div(t, pico8_fixed_from_int(176))));

    for (dx = l; dx <= h; dx = pico8_fixed_add(dx, s)) {
        pico8_fixed_t dy;
        for (dy = l; dy <= h; dy = pico8_fixed_add(dy, s)) {
            int x1 = pico8_jc_display_to_int(pico8_fixed_add(qu_pt.x, dx));
            int y1 = pico8_jc_display_to_int(pico8_fixed_add(qu_pt.y, dy));
            int x2 = pico8_jc_display_to_int(pico8_fixed_add(re_pt.x, dx));
            int y2 = pico8_jc_display_to_int(pico8_fixed_add(re_pt.y, dy));
            pico8_line(gfx, mem, x1, y1, x2, y2, c);
        }
    }
}

void pico8_jc_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self) {
    pico8_instance_t *qu;
    pico8_instance_t *re;

    if (self == NULL) {
        return;
    }

    qu = pico8_jc_display_field_instance(self, "qu");
    re = pico8_jc_display_field_instance(self, "re");

    if (pico8_gd(qu) || pico8_gd(re)) {
        pico8_fixed_t nh = pico8_jc_display_field_fixed(self, "nh");
        int nh_i = pico8_jc_display_to_int(nh);
        pico8_fixed_t jv = pico8_fixed_from_int(0);
        pico8_value_t k_val = pico8_jc_display_field_value(self, "k_");
        int et_or_fo;
        int mt_truthy;
        pico8_fixed_t c;
        pico8_fixed_t bc;
        pico8_fixed_t tc;

        if (k_val.kind == PICO8_VALUE_REF && k_val.as.ref != NULL) {
            pico8_value_t entry;
            if (pico8_ob_table_get_index((const pico8_ob_table_t *)k_val.as.ref, nh_i, &entry) &&
                entry.kind == PICO8_VALUE_FIXED) {
                jv = entry.as.fixed;
            }
        }

        if (nh < pico8_fixed_from_int(20)) {
            pico8_ob_table_set_name(self->fields, "nh", pico8_value_fixed(pico8_fixed_add(nh, pico8_fixed_from_int(1))));
        }

        et_or_fo = pico8_jc_display_truthy(pico8_jc_display_field_value(self, "et")) ||
                   pico8_jc_display_truthy(pico8_jc_display_field_value(self, "fo"));
        mt_truthy = pico8_jc_display_truthy(pico8_jc_display_field_value(self, "mt"));

        c = et_or_fo ? pico8_fixed_from_int(2) : (mt_truthy ? pico8_fixed_from_int(0) : jv);
        bc = mt_truthy ? jv : pico8_fixed_from_int(0);
        tc = mt_truthy ? pico8_fixed_from_int(5) : pico8_fixed_from_int(13);

        pico8_jc_lg(gfx, mem, self, pico8_fixed_from_int(-1), pico8_fixed_from_int(2), pico8_fixed_from_int(3),
                    pico8_jc_display_to_int(bc));
        pico8_jc_lg(gfx, mem, self, pico8_fixed_from_int(0), pico8_fixed_from_int(1), pico8_fixed_from_int(1),
                    pico8_jc_display_to_int(c));

        {
            pico8_value_t ts_val = pico8_jc_display_field_value(self, "ts");
            if (ts_val.kind == PICO8_VALUE_REF && ts_val.as.ref != NULL) {
                pico8_list_t *ts = (pico8_list_t *)ts_val.as.ref;
                int i;
                for (i = 0; i < ts->count; i++) {
                    pico8_value_t t_val = ts->items[i];
                    pico8_ob_table_t *t;
                    pico8_vec_t ra;
                    pico8_vec_t v;
                    pico8_vec_t new_ra;
                    pico8_vec_t *heap_ra;
                    pico8_fixed_t new_l;

                    if (t_val.kind != PICO8_VALUE_REF || t_val.as.ref == NULL) {
                        continue;
                    }
                    t = (pico8_ob_table_t *)t_val.as.ref;

                    ra = pico8_jc_display_table_field_vec(t, "ra");
                    v = pico8_jc_display_table_field_vec(t, "v");
                    new_ra = pico8_vec_add(ra, v);
                    heap_ra = malloc(sizeof *heap_ra);
                    *heap_ra = new_ra;
                    pico8_ob_table_set_name(t, "ra", pico8_value_ref(heap_ra));

                    new_l = pico8_fixed_sub(pico8_jc_display_table_field_fixed(t, "l"), pico8_fixed_from_int(1));
                    pico8_ob_table_set_name(t, "l", pico8_value_fixed(new_l));

                    if (new_l <= pico8_fixed_from_int(0)) {
                        pico8_list_del(ts, t_val);
                    }

                    {
                        int x = pico8_jc_display_to_int(new_ra.x);
                        int y = pico8_jc_display_to_int(new_ra.y);
                        pico8_rectfill(gfx, mem, x, y, x + 1, y + 1, 0);
                        pico8_pset(gfx, mem, x, y, pico8_jc_display_to_int(tc));
                    }
                }
            }
        }

        pico8_jc_ch(self, qu, re);
        pico8_jc_ch(self, re, qu);
    }

    pico8_ob_table_set_name(self->fields, "fo", pico8_value_ref(NULL));
}
