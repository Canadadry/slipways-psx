#include "pico8_bg.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_gen.h"
#include "pico8_ke.h"
#include "pico8_vec.h"

static pico8_class_t *g_bg_class = NULL;

static int pico8_bg_to_int(pico8_fixed_t x) {
    return (int)pico8_fixed_to_double(x);
}

/* An absent field, a wrong kind or a NULL instance all read as a zero
 * vector; same for a REF that holds NULL in the value reader below. */
static pico8_vec_t pico8_bg_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t pico8_bg_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static void pico8_bg_pq_dispatch(pico8_instance_t *instance) {
    pico8_bg_pq(pico8_dispatch_mem(), instance);
}

static pico8_value_t pico8_bg_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_bg_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_bg_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg1;
    return pico8_bg_ldown((pico8_instance_t *)self, arg0);
}

static pico8_value_t pico8_bg_rdown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_bg_rdown((pico8_instance_t *)self, arg0, arg1);
}

static pico8_value_t pico8_bg_hq_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    pico8_bg_hq((pico8_instance_t *)self, arg0, arg1);
    return pico8_value_ref(NULL);
}

/* No `pico8_pi_init()` needed: the spec holds no nested `v(...)`/`b(...)`,
 * only the plain numbers of `pl=o(...)`. */
void pico8_bg_init(void) {
    pico8_bg_shutdown();
    g_bg_class = pico8_class_declare(
        pico8_ke_class(), "\nfa=1,hd=1,pl=o(0,0,0,72,73,74,75,88,89,90,91,88,91,89,91),\n");
    pico8_class_set_pq(g_bg_class, pico8_bg_pq_dispatch);
    pico8_ob_table_set_name(g_bg_class->defaults, "kj", pico8_value_function(pico8_bg_kj_dispatch));
    pico8_ob_table_set_name(g_bg_class->defaults, "ldown", pico8_value_function(pico8_bg_ldown_dispatch));
    pico8_ob_table_set_name(g_bg_class->defaults, "rdown", pico8_value_function(pico8_bg_rdown_dispatch));
    pico8_ob_table_set_name(g_bg_class->defaults, "hq", pico8_value_function(pico8_bg_hq_dispatch));
}

void pico8_bg_shutdown(void) {
    if (g_bg_class != NULL) {
        pico8_class_free(g_bg_class);
        g_bg_class = NULL;
    }
}

pico8_class_t *pico8_bg_class(void) {
    return g_bg_class;
}

pico8_instance_t *pico8_bg_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_bg_class, overrides);
}

/* `vs[x] or 1` is only ever taken on row y=0; every later row reads back the
 * previous row's own `vh`. `oh` is local to the call and freed here. */
void pico8_bg_pq(pico8_mem_t *mem, pico8_instance_t *self) {
    pico8_fixed_t vs[32];
    int vs_set[32];
    pico8_fixed_t vh = pico8_fixed_from_int(0);
    pico8_jf_state_t *oh;
    const pico8_ob_table_t *pl_table = NULL;
    pico8_value_t pl_val;
    int x, y;

    (void)self;

    if (mem == NULL || g_bg_class == NULL) {
        return;
    }

    for (x = 0; x < 32; x++) {
        vs_set[x] = 0;
    }

    if (pico8_ob_table_get_name(g_bg_class->defaults, "pl", &pl_val) && pl_val.kind == PICO8_VALUE_REF &&
        pl_val.as.ref != NULL) {
        pl_table = (const pico8_ob_table_t *)pl_val.as.ref;
    }

    oh = pico8_jf(pico8_fixed_from_double(0.1), pico8_fixed_from_int(0));

    for (y = 0; y <= 31; y++) {
        for (x = 0; x <= 31; x++) {
            pico8_fixed_t prev = vs_set[x] ? vs[x] : pico8_fixed_from_int(1);
            pico8_fixed_t step =
                pico8_fixed_sub(pico8_fixed_mul(pico8_jf_next(oh), pico8_fixed_from_int(8)), pico8_fixed_from_int(4));
            pico8_fixed_t roll;
            int tile;

            vh = pico8_fixed_add(step, pico8_fixed_mul(pico8_fixed_add(vh, prev), pico8_fixed_from_double(0.5)));
            vs[x] = vh;
            vs_set[x] = 1;

            roll = pico8_fixed_rnd_default();
            if (roll < pico8_fixed_from_double(0.004)) {
                tile = 122 + pico8_bg_to_int(pico8_fixed_rnd(pico8_fixed_from_int(5)));
            } else {
                pico8_fixed_t idx_f = pico8_fixed_flr(pico8_fixed_add(vh, pico8_fixed_rnd(pico8_fixed_from_int(4))));
                int idx = pico8_bg_to_int(idx_f);
                pico8_value_t pl_entry;

                tile = 0;
                if (pl_table != NULL && pico8_ob_table_get_index(pl_table, idx, &pl_entry) &&
                    pl_entry.kind == PICO8_VALUE_FIXED) {
                    tile = pico8_bg_to_int(pl_entry.as.fixed);
                }
            }

            pico8_mset(mem, x, y, tile);
        }
    }

    pico8_jf_free(oh);
}

void pico8_bg_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self) {
    int xy;

    (void)self;

    if (gfx == NULL || mem == NULL) {
        return;
    }

    pico8_map(gfx, mem, 0, 0, 0, 0, 32, 32);
    pico8_fillp(gfx, 0xA5A5u);
    for (xy = 3; xy <= 262; xy += 32) {
        pico8_rectfill(gfx, mem, xy, 0, xy, 262, 1);
        pico8_rectfill(gfx, mem, 0, xy, 262, xy, 1);
    }
    pico8_fillp_reset(gfx);
    pico8_rect(gfx, mem, 0, 0, 262, 262, 5);
}

pico8_value_t pico8_bg_ldown(pico8_instance_t *self, pico8_value_t mp) {
    const pico8_list_t *g_tag;
    int count;
    pico8_class_t *cls;
    pico8_instance_t *made;

    (void)self;
    (void)mp;

    g_tag = pico8_cg("g_");
    count = (g_tag != NULL) ? g_tag->count : 0;
    cls = pico8_class_lookup_named(count > 0 ? "mz" : "g_");

    if (cls == NULL) {
        return pico8_value_ref(NULL);
    }

    made = pico8_class_instantiate(cls, NULL);
    return pico8_value_ref(made);
}

pico8_value_t pico8_bg_rdown(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp) {
    (void)mp;

    if (self != NULL) {
        pico8_ob_table_set_name(self->fields, "mq", rp);
    }
    return pico8_value_ref(self);
}

/* `self.mq` is read here and overwritten by the last statement of the
 * function: that read-then-replace is the drag delta itself. */
void pico8_bg_hq(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp) {
    pico8_vec_t mq;
    pico8_vec_t rp_vec;
    pico8_instance_t *mo;

    (void)mp;

    if (self == NULL) {
        return;
    }

    mq = pico8_bg_field_vec(self, "mq");
    rp_vec = pico8_bg_value_vec(rp);
    mo = pico8_mo();
    if (mo != NULL) {
        /* TROU-7 */
        pico8_vec_t *v_new = malloc(sizeof *v_new);
        *v_new = pico8_vec_sub(mq, rp_vec);
        pico8_ob_table_set_name(mo->fields, "v", pico8_value_ref(v_new));
    }

    pico8_ob_table_set_name(self->fields, "mq", rp);
}
