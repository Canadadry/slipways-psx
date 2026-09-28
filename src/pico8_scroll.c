#include "pico8_scroll.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity_geom.h"
#include "pico8_pi.h"
#include "pico8_value.h"
#include "pico8_vec.h"

static pico8_ob_table_t *g_qb_spec = NULL;
static pico8_vec_t g_qb_v = {0, 0};
static pico8_vec_t g_qb_p = {0, 0};
static pico8_vec_t g_qb_ra = {0, 0};

/* NULL reads as the zero vector. */
static pico8_vec_t pico8_qb_vec(const pico8_vec_t *v) {
    if (v != NULL) {
        return *v;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* dg() dispatches the per-tick state method as `qc[qc.me](qc,qc.t)`, so
 * arg0 is the frame counter, which qb:ph does not read. */
static pico8_value_t pico8_qb_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_qb_ph(pico8_dispatch_input(), (pico8_qb_t *)self);
    return pico8_value_ref(NULL);
}

/* Resolves Lua's `fy or 1` here, so pico8_qb_md itself always gets a real
 * factor; dc()'s "kj" branch passes `ci[o]` as arg0. */
static pico8_value_t pico8_qb_md_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t fy = (arg0.kind == PICO8_VALUE_FIXED) ? arg0.as.fixed : pico8_fixed_from_int(1);
    (void)argc;
    (void)arg1;
    pico8_qb_md(pico8_dispatch_gfx(), (pico8_qb_t *)self, fy);
    return pico8_value_ref(NULL);
}

static int pico8_qb_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_qb_t *qb = self;

    if (strcmp(name, "v") == 0) {
        if (qb->v == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(qb->v);
        return 1;
    }
    if (strcmp(name, "ma") == 0) {
        *out_value = pico8_value_fixed(qb->ma);
        return 1;
    }
    if (strcmp(name, "p") == 0) {
        if (qb->p == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(qb->p);
        return 1;
    }
    if (strcmp(name, "ra") == 0) {
        if (qb->ra == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(qb->ra);
        return 1;
    }
    if (strcmp(name, "pc") == 0) {
        if (qb->pc == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)qb->pc);
        return 1;
    }
    return pico8_ke_field(self, name, out_value);
}

static void qb_take_vec(pico8_value_t value, pico8_vec_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "qb.v: a write neither nil nor a vector ref does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

static void qb_take_fixed(pico8_value_t value, pico8_fixed_t *out) {
    assert(value.kind == PICO8_VALUE_FIXED &&
           "qb.ma: a write that is not a fixed does not fit the member's type -- stop, do not widen the member");
    *out = value.as.fixed;
}

/* `ma` (ma(), ga:pq) and `v` (bg:hq): the only names a generic writer ever
 * writes on `mo`. */
static int pico8_qb_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_qb_t *qb = self;

    if (strcmp(name, "v") == 0) {
        qb_take_vec(value, &qb->v);
        return 1;
    }
    if (strcmp(name, "ma") == 0) {
        qb_take_fixed(value, &qb->ma);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_qb_struct_class = {
    {[PICO8_METHOD_PH] = pico8_qb_ph_dispatch, [PICO8_METHOD_MD] = pico8_qb_md_dispatch},
    pico8_qb_field,
    pico8_qb_set_field,
};

static void *pico8_qb_struct_new(const pico8_ob_table_t *overrides) {
    pico8_qb_t *self = malloc(sizeof *self);

    pico8_ke_struct_init(&self->ke, &g_qb_struct_class);
    self->v = &g_qb_v;
    self->ma = pico8_fixed_from_int(0);
    self->p = &g_qb_p;
    self->ra = &g_qb_ra;
    self->pc = &pico8_qb_pc_vecs;
    assert((overrides == NULL || overrides->count == 0) &&
           "qb: takes no construction table; a key has no member -- stop, do not add one");
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("qb");
#endif
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_qb_init(void) {
    pico8_qb_shutdown();
    pico8_pi_init();
    assert(pico8_ob_lookup_constructor('v') != NULL && "qb's spec parses v(...) as vectors");
    assert(pico8_ke_ready() && "qb descends from ke");
    g_qb_spec = pico8_ob("\nv=v(0,0),ma=0,p=v(192,192),ra=v(192,192),"
                         "pc=o(v(-1,0),v(1,0),v(0,-1),v(0,1)),\n",
                         NULL);
    g_qb_p = pico8_vec_v(pico8_fixed_from_int(192), pico8_fixed_from_int(192));
    g_qb_ra = pico8_vec_v(pico8_fixed_from_int(192), pico8_fixed_from_int(192));
}

void pico8_qb_shutdown(void) {
    pico8_ob_table_free(g_qb_spec);
    g_qb_spec = NULL;
}

int pico8_qb_ready(void) {
    return g_qb_spec != NULL;
}

int pico8_qb_default(const char *name, pico8_value_t *out_value) {
    return g_qb_spec != NULL &&
           (pico8_ob_table_get_name(g_qb_spec, name, out_value) || pico8_ke_default(name, out_value));
}

const pico8_struct_class_t *pico8_qb_struct_class(void) {
    return &g_qb_struct_class;
}

pico8_qb_t *pico8_qb_new(const pico8_ob_table_t *overrides) {
    return pico8_qb_struct_new(overrides);
}

void pico8_qb_ph(const pico8_input_t *input, pico8_qb_t *self) {
    pico8_vec_t ip = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    const pico8_vec_seq_t *pc = NULL;
    pico8_vec_t p;
    pico8_vec_t v;
    pico8_vec_t *p_new;
    pico8_vec_t *v_new;
    pico8_vec_t *ra_new;
    int b;

    if (self == NULL || input == NULL) {
        return;
    }

    pc = self->pc;

    for (b = 0; b <= 3; b++) {
        int btn_p1 = pico8_btn(input, b);
        /* TROU-2 */
        int btn_p2 = 0;

        if (btn_p1 || btn_p2) {
            pico8_vec_t dir = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
            if (pc != NULL && b < pc->count) {
                dir = pc->items[b];
            }
            ip = pico8_vec_add(ip, pico8_vec_mul(dir, pico8_fixed_from_int(3)));
        }
    }

    p = pico8_qb_vec(self->p);
    v = pico8_qb_vec(self->v);

    p = pico8_vec_add(p, v);
    v = pico8_vec_add(v, pico8_vec_mul(pico8_vec_sub(ip, v), PICO8_FIXED_LIT(0x00003333, 0.2)));

    p.x = pico8_fixed_mid(pico8_fixed_from_int(-64), pico8_fixed_from_int(320), p.x);
    p.y = pico8_fixed_mid(pico8_fixed_from_int(-64), pico8_fixed_from_int(320), p.y);

    /* TROU-7 */
    p_new = malloc(sizeof *p_new);
    *p_new = p;
    v_new = malloc(sizeof *v_new);
    *v_new = v;
    self->p = p_new;
    self->v = v_new;

    ra_new = malloc(sizeof *ra_new);
    *ra_new = pico8_ma(p, (pico8_struct_header_t *)self);
    self->ra = ra_new;
}

void pico8_qb_md(pico8_gfx_t *gfx, pico8_qb_t *self, pico8_fixed_t fy) {
    pico8_vec_t cp;

    if (self == NULL || gfx == NULL) {
        return;
    }

    cp = pico8_vec_mul(pico8_qb_vec(self->ra), fy);
    pico8_camera(gfx, pico8_fixed_flr_int(cp.x), pico8_fixed_flr_int(cp.y));
}
