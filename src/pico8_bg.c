#include "pico8_bg.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_gen.h"
#include "pico8_ke.h"
#include "pico8_probe.h"
#include "pico8_vec.h"
#include "pico8_wormhole.h"

static pico8_ob_table_t *g_bg_spec = NULL;
static pico8_ob_table_t *g_bg_pl = NULL;

static int pico8_bg_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

/* Nil and wrong-kind both read as the zero vector. */
static pico8_vec_t pico8_bg_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_value_t pico8_bg_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_bg_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_bg_t *)self);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_bg_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg1;
    return pico8_bg_ldown((pico8_bg_t *)self, arg0);
}

static pico8_value_t pico8_bg_rdown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_bg_rdown((pico8_bg_t *)self, arg0, arg1);
}

static pico8_value_t pico8_bg_hq_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    pico8_bg_hq((pico8_bg_t *)self, arg0, arg1);
    return pico8_value_ref(NULL);
}

static int pico8_bg_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_bg_t *bg = self;
    static const struct {
        const char *name;
        size_t offset;
    } members[] = {
        {"fa", offsetof(pico8_bg_t, fa)}, {"hd", offsetof(pico8_bg_t, hd)}, {"pl", offsetof(pico8_bg_t, pl)},
        {"mq", offsetof(pico8_bg_t, mq)}, {"m_", offsetof(pico8_bg_t, m_)},
    };
    size_t i;

    for (i = 0; i < sizeof members / sizeof members[0]; i++) {
        if (strcmp(name, members[i].name) == 0) {
            pico8_value_t value = *(const pico8_value_t *)((const char *)bg + members[i].offset);
            if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
                return 0;
            }
            *out_value = value;
            return 1;
        }
    }
    return pico8_ke_field(self, name, out_value);
}

/* `m_` (the hover loop `gw`): the only name a generic writer ever writes
 * on a `bg`. */
static int pico8_bg_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_bg_t *bg = self;

    if (strcmp(name, "m_") == 0) {
        bg->m_ = value;
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_bg_struct_class = {
    {
        [PICO8_METHOD_KJ] = pico8_bg_kj_dispatch,
        [PICO8_METHOD_LDOWN] = pico8_bg_ldown_dispatch,
        [PICO8_METHOD_RDOWN] = pico8_bg_rdown_dispatch,
        [PICO8_METHOD_HQ] = pico8_bg_hq_dispatch,
    },
    pico8_bg_field,
    pico8_bg_set_field,
};

static void *pico8_bg_struct_new(const pico8_ob_table_t *overrides) {
    pico8_bg_t *self = malloc(sizeof *self);

    pico8_ke_struct_init(&self->ke, &g_bg_struct_class);
    self->fa = pico8_value_fixed(pico8_fixed_from_int(1));
    self->hd = pico8_value_fixed(pico8_fixed_from_int(1));
    self->pl = pico8_value_ref(g_bg_pl);
    self->mq = pico8_value_ref(NULL);
    self->m_ = pico8_value_ref(NULL);
    assert((overrides == NULL || overrides->count == 0) &&
           "bg: no construction table was measured; a key would be a field under no rule of plan-3 section 5 "
           "-- stop, do not add a member");
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("bg");
#endif
    pico8_bg_pq(pico8_dispatch_mem(), self);
    pico8_ke_struct_pq(&self->ke);
    return self;
}

/* No `pico8_pi_init()` needed: the spec holds no nested `v(...)`/`b(...)`,
 * only the plain numbers of `pl=o(...)`. */
void pico8_bg_init(void) {
    pico8_bg_shutdown();
    g_bg_spec = pico8_ob("\nfa=1,hd=1,pl=o(0,0,0,72,73,74,75,88,89,90,91,88,91,89,91),\n", NULL);
    g_bg_pl = pico8_ob("0,0,0,72,73,74,75,88,89,90,91,88,91,89,91,", NULL);
}

void pico8_bg_shutdown(void) {
    pico8_ob_table_free(g_bg_spec);
    g_bg_spec = NULL;
    pico8_ob_table_free(g_bg_pl);
    g_bg_pl = NULL;
}

int pico8_bg_ready(void) {
    return g_bg_spec != NULL;
}

int pico8_bg_default(const char *name, pico8_value_t *out_value) {
    return g_bg_spec != NULL &&
           (pico8_ob_table_get_name(g_bg_spec, name, out_value) || pico8_ke_default(name, out_value));
}

const pico8_struct_class_t *pico8_bg_struct_class(void) {
    return &g_bg_struct_class;
}

pico8_bg_t *pico8_bg_new(const pico8_ob_table_t *overrides) {
    return pico8_bg_struct_new(overrides);
}

/* `vs[x] or 1` is only ever taken on row y=0; every later row reads back the
 * previous row's own `vh`. `oh` is local to the call and freed here. */
void pico8_bg_pq(pico8_mem_t *mem, pico8_bg_t *self) {
    pico8_fixed_t vs[32];
    int vs_set[32];
    pico8_fixed_t vh = pico8_fixed_from_int(0);
    pico8_jf_state_t *oh;
    const pico8_ob_table_t *pl_table = g_bg_pl;
    int x, y;

    (void)self;

    if (mem == NULL || pl_table == NULL) {
        return;
    }

    for (x = 0; x < 32; x++) {
        vs_set[x] = 0;
    }

    oh = pico8_jf(PICO8_FIXED_LIT(0x00001999, 0.1), pico8_fixed_from_int(0));

    for (y = 0; y <= 31; y++) {
        for (x = 0; x <= 31; x++) {
            pico8_fixed_t prev = vs_set[x] ? vs[x] : pico8_fixed_from_int(1);
            pico8_fixed_t step =
                pico8_fixed_sub(pico8_fixed_mul(pico8_jf_next(oh), pico8_fixed_from_int(8)), pico8_fixed_from_int(4));
            pico8_fixed_t roll;
            int tile;

            vh = pico8_fixed_add(step, pico8_fixed_mul(pico8_fixed_add(vh, prev), PICO8_FIXED_LIT(0x00008000, 0.5)));
            vs[x] = vh;
            vs_set[x] = 1;

            roll = pico8_fixed_rnd_default();
            if (roll < PICO8_FIXED_LIT(0x00000106, 0.004)) {
                tile = 122 + pico8_bg_to_int(pico8_fixed_rnd(pico8_fixed_from_int(5)));
            } else {
                pico8_fixed_t idx_f = pico8_fixed_flr(pico8_fixed_add(vh, pico8_fixed_rnd(pico8_fixed_from_int(4))));
                int idx = pico8_bg_to_int(idx_f);
                pico8_value_t pl_entry;

                tile = 0;
                if (pico8_ob_table_get_index(pl_table, idx, &pl_entry) &&
                    pl_entry.kind == PICO8_VALUE_FIXED) {
                    tile = pico8_bg_to_int(pl_entry.as.fixed);
                }
            }

            pico8_mset(mem, x, y, tile);
        }
    }

    pico8_jf_free(oh);
}

void pico8_bg_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_bg_t *self) {
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

pico8_value_t pico8_bg_ldown(pico8_bg_t *self, pico8_value_t mp) {
    const pico8_list_t *g_tag;
    int count;
    void *made;

    (void)self;
    (void)mp;

    g_tag = pico8_cg("g_");
    count = (g_tag != NULL) ? g_tag->count : 0;
    if (count > 0) {
        assert(pico8_mz_ready() && "pico8_wormhole_init() must run before bg:ldown builds an mz");
        made = pico8_mz_new(NULL);
    } else {
        made = pico8_g__new(NULL);
    }
    return pico8_value_ref(made);
}

pico8_value_t pico8_bg_rdown(pico8_bg_t *self, pico8_value_t mp, pico8_value_t rp) {
    (void)mp;

    if (self != NULL) {
        self->mq = rp;
    }
    return pico8_value_ref(self);
}

/* `self.mq` is read here and overwritten by the last statement of the
 * function: that read-then-replace is the drag delta itself. */
void pico8_bg_hq(pico8_bg_t *self, pico8_value_t mp, pico8_value_t rp) {
    pico8_vec_t mq;
    pico8_vec_t rp_vec;
    pico8_instance_t *mo;

    (void)mp;

    if (self == NULL) {
        return;
    }

    mq = pico8_bg_value_vec(self->mq);
    rp_vec = pico8_bg_value_vec(rp);
    mo = pico8_mo();
    if (mo != NULL) {
        /* TROU-7 */
        pico8_vec_t *v_new = malloc(sizeof *v_new);
        *v_new = pico8_vec_sub(mq, rp_vec);
        pico8_entity_set_field(mo, "v", pico8_value_ref(v_new));
    }

    self->mq = rp;
}
