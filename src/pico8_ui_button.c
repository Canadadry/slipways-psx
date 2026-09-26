#include "pico8_ui_button.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_geom.h"
#include "pico8_ke.h"
#include "pico8_layout.h"
#include "pico8_ref_audit.h"

static pico8_class_t *g_js_class = NULL;
static pico8_class_t *g_fq_class = NULL;

/* The overrides table `lq(x)` copies from: NULL when the field is absent or
 * holds no REF, else the REF itself (borrowed) for an instance or a table, or
 * a freshly allocated one for a list. All three shapes reach this field. */
static const pico8_ob_table_t *pico8_js_lq_arg_table(pico8_value_t v) {
    if (v.kind != PICO8_VALUE_REF || v.as.ref == NULL) {
        return NULL;
    }

    switch (pico8_type_tag_of(v.as.ref)) {
    case PICO8_TYPE_TAG_INSTANCE:
        return pico8_instance_fields((const pico8_instance_t *)v.as.ref);
    case PICO8_TYPE_TAG_OB_TABLE:
        return (const pico8_ob_table_t *)v.as.ref;
    case PICO8_TYPE_TAG_LIST:
        /* TROU-7 */
        return pico8_ob_table_from_list(PICO8_REF_AS_LIST_CONST(v.as.ref));
    default:
        assert(0 && "untagged REF reached js:pq's lq()/h_() argument");
        return NULL;
    }
}

static int pico8_ui_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

/* An `lq` instance's field: absent, a wrong kind or a NULL instance read as
 * 0. */
static pico8_fixed_t pico8_ui_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static int pico8_js_truthy(pico8_value_t v) {
    return !(v.kind == PICO8_VALUE_REF && v.as.ref == NULL) && !(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

static pico8_value_t pico8_js_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_js_ph(self);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_js_ev_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_js_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), self);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_js_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) ? *(const pico8_vec_t *)arg0.as.ref
                                                                           : pico8_vec_v(pico8_fixed_from_int(0),
                                                                                         pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    pico8_js_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), self, p);
    return pico8_value_ref(NULL);
}

/* `js.hd=gk`: a field assignment in the cart, not a method definition. */
static pico8_value_t pico8_js_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t mp = arg0;
    pico8_value_t rp = (argc >= 2) ? arg1 : arg0;
    return pico8_gk((const pico8_instance_t *)self, mp, rp);
}

static pico8_value_t pico8_js_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_js_ldown(self);
}

static const struct {
    const char *name;
    size_t offset;
} g_js_members[] = {
    {"fa", offsetof(pico8_js_t, fa)},
    {"lq", offsetof(pico8_js_t, lq)},
    {"mb", offsetof(pico8_js_t, mb)},
    {"kq", offsetof(pico8_js_t, kq)},
    {"h_", offsetof(pico8_js_t, h_)},
    {"jb", offsetof(pico8_js_t, jb)},
    {"ra", offsetof(pico8_js_t, ra)},
    {"qf", offsetof(pico8_js_t, qf)},
    {"my", offsetof(pico8_js_t, my)},
    {"m_", offsetof(pico8_js_t, m_)},
    {"hb", offsetof(pico8_js_t, hb)},
    {"fg", offsetof(pico8_js_t, fg)},
    {"px", offsetof(pico8_js_t, px)},
    {"trade", offsetof(pico8_js_t, trade)},
    {"en", offsetof(pico8_js_t, en)},
    {"_cr_t", offsetof(pico8_js_t, _cr_t)},
    {"_jo_fe_pt", offsetof(pico8_js_t, _jo_fe_pt)},
    {"_jo_fe_b", offsetof(pico8_js_t, _jo_fe_b)},
    {"_jo_fe_od", offsetof(pico8_js_t, _jo_fe_od)},
};

/* The member named `name`, or NULL when a `js` has none. */
static pico8_value_t *pico8_js_member(const pico8_js_t *js, const char *name) {
    size_t i;
    for (i = 0; i < sizeof g_js_members / sizeof g_js_members[0]; i++) {
        if (strcmp(name, g_js_members[i].name) == 0) {
            return (pico8_value_t *)((const char *)js + g_js_members[i].offset);
        }
    }
    return NULL;
}

static int pico8_js_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_value_t *member = pico8_js_member(self, name);

    if (member == NULL) {
        return pico8_ke_field(self, name, out_value);
    }
    if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
        return 0;
    }
    *out_value = *member;
    return 1;
}

/* `m_` (the hover loop) and `lq` (`ix[2].hb`): the only names a generic
 * writer ever writes on a `js`. */
static int pico8_js_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_js_t *js = self;

    if (strcmp(name, "m_") == 0) {
        js->m_ = value;
    } else if (strcmp(name, "lq") == 0) {
        js->lq = value;
    } else {
        return 0;
    }
    return 1;
}

static const pico8_struct_class_t g_js_struct_class = {
    {
        [PICO8_METHOD_HD] = pico8_js_hd_dispatch,
        [PICO8_METHOD_PH] = pico8_js_ph_dispatch,
        [PICO8_METHOD_EV] = pico8_js_ev_dispatch,
        [PICO8_METHOD_KJ] = pico8_js_kj_dispatch,
        [PICO8_METHOD_LDOWN] = pico8_js_ldown_dispatch,
    },
    pico8_js_field,
    pico8_js_set_field,
};

static void *pico8_js_struct_new(const pico8_ob_table_t *overrides) {
    pico8_js_t *self = malloc(sizeof *self);
    size_t i;
    int k;

    pico8_ke_struct_init(&self->ke, &g_js_struct_class);
    for (i = 0; i < sizeof g_js_members / sizeof g_js_members[0]; i++) {
        *(pico8_value_t *)((char *)self + g_js_members[i].offset) = pico8_value_ref(NULL);
    }
    self->fa = pico8_value_fixed(pico8_fixed_from_int(11));
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        pico8_value_t *member = NULL;

        if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "mb") != 0 && strcmp(key.name, "kq") != 0 &&
            strcmp(key.name, "m_") != 0) {
            member = pico8_js_member(self, key.name);
        }
        assert(member != NULL && "js: the only measured construction keys are fa, lq, h_, jb, ra, qf, my, hb, fg, "
                                 "px, trade, en, _cr_t and _jo_fe_pt/_b/_od; another would be a field under no rule "
                                 "of plan-3 section 5 -- stop, do not add a member");
        *member = overrides->entries[k].value;
        if ((member == &self->h_ || member == &self->jb) && member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
            /* TROU-23 */
            *member = pico8_value_bool(1);
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("js");
#endif
    pico8_js_pq(self);
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_js_init(void) {
    pico8_js_shutdown();
    g_js_class = pico8_class_declare(pico8_ke_class(), "\nfa=11,\n");
    PICO8_CLASS_COVERAGE_NAME(g_js_class, "js");
    pico8_class_set_struct_new(g_js_class, pico8_js_struct_new);
}

void pico8_js_shutdown(void) {
    if (g_js_class != NULL) {
        pico8_class_free(g_js_class);
        g_js_class = NULL;
    }
}

pico8_class_t *pico8_js_class(void) {
    return g_js_class;
}

const pico8_struct_class_t *pico8_js_struct_class(void) {
    return &g_js_struct_class;
}

pico8_js_t *pico8_js_new(const pico8_ob_table_t *overrides) {
    return pico8_js_struct_new(overrides);
}

void pico8_js_pq(pico8_js_t *self) {
    pico8_instance_t *new_lq;
    pico8_value_t w_val;
    pico8_fixed_t mb;

    if (self == NULL) {
        return;
    }

    new_lq = pico8_lq_new(pico8_js_lq_arg_table(self->lq));
    self->lq = pico8_value_ref(new_lq);

    mb = pico8_fixed_from_int(2);
    if (pico8_instance_get_field(new_lq, "w", &w_val) && w_val.kind == PICO8_VALUE_FIXED) {
        mb = pico8_fixed_add(w_val.as.fixed, mb);
    }
    self->mb = pico8_value_fixed(mb);

    self->kq = pico8_value_ref(
        pico8_qz(pico8_fixed_from_int(0), pico8_fixed_from_int(-1), mb, pico8_fixed_from_int(11)));

    if (pico8_js_truthy(self->h_)) {
        self->h_ = pico8_value_ref(pico8_lq_new(pico8_js_lq_arg_table(self->h_)));
    }

    if (pico8_js_truthy(self->jb)) {
        const pico8_ob_table_t *jb_overrides =
            (self->jb.kind == PICO8_VALUE_REF) ? (const pico8_ob_table_t *)self->jb.as.ref : NULL;

        assert((jb_overrides == NULL || pico8_type_tag_of(jb_overrides) == PICO8_TYPE_TAG_OB_TABLE) &&
               "js:pq's jb field must hold a plain spec table");
        self->jb = pico8_value_ref(pico8_jb_new(jb_overrides));
    }
}

void pico8_js_ph(pico8_js_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->qf.kind == PICO8_VALUE_REF && self->qf.as.ref != NULL) {
        pico8_vec_t qf = *(const pico8_vec_t *)self->qf.as.ref;
        pico8_vec_t ra = (self->ra.kind == PICO8_VALUE_REF && self->ra.as.ref != NULL)
                             ? *(const pico8_vec_t *)self->ra.as.ref
                             : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
        pico8_vec_t d = pico8_vec_sub(qf, ra);
        pico8_fixed_t len_sq = pico8_vec_len_sq(d);
        pico8_vec_t delta = (len_sq <= pico8_fixed_from_int(1)) ? d : pico8_vec_mul(d, PICO8_FIXED_LIT(0x00006666, 0.4));
        /* TROU-7 */
        pico8_vec_t *heap_ra = malloc(sizeof *heap_ra);
        *heap_ra = pico8_vec_add(ra, delta);
        self->ra = pico8_value_ref(heap_ra);
    }
}

void pico8_js_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_js_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->jb.kind == PICO8_VALUE_REF && self->jb.as.ref != NULL && pico8_js_truthy(self->m_)) {
        assert(pico8_type_tag_of(self->jb.as.ref) == PICO8_TYPE_TAG_STRUCT &&
               "js:ev: the jb field must hold the jb struct js:pq built, not a spec table");
        pico8_jb_ol(gfx, mem, (const pico8_jb_t *)self->jb.as.ref, pico8_fixed_from_int(127), pico8_fixed_from_int(1));
    }
}

void pico8_js_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_js_t *self, pico8_vec_t p) {
    pico8_fixed_t mb;
    pico8_fixed_t qt;
    pico8_fixed_t dn;
    int has_my;
    int m_flag;
    int ix_p;
    int iy_p;
    int ix_qt;
    int iy_dn;

    if (self == NULL) {
        return;
    }

    mb = self->mb.kind == PICO8_VALUE_FIXED ? self->mb.as.fixed : pico8_fixed_from_int(0);
    qt = pico8_fixed_sub(pico8_fixed_add(p.x, mb), pico8_fixed_from_int(1));
    dn = pico8_fixed_add(p.y, pico8_fixed_from_int(9));
    has_my = pico8_js_truthy(self->my);
    m_flag = pico8_js_truthy(self->m_);

    ix_p = pico8_ui_to_int(p.x);
    iy_p = pico8_ui_to_int(p.y);
    ix_qt = pico8_ui_to_int(qt);
    iy_dn = pico8_ui_to_int(dn);

    pico8_rect(gfx, mem, ix_p - 1, iy_p, ix_qt + 1, iy_dn + 1, 1);
    pico8_rectfill(gfx, mem, ix_p, iy_p, ix_qt, iy_dn, has_my ? (m_flag ? 12 : 13) : 5);
    pico8_rectfill(gfx, mem, ix_p, iy_p, ix_qt, iy_p, has_my ? 6 : 5);

    if (self->lq.kind == PICO8_VALUE_REF && self->lq.as.ref != NULL) {
        pico8_vec_t off = pico8_vec_add(p, pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(2)));
        pico8_lq_ol_default(gfx, mem, (const pico8_instance_t *)self->lq.as.ref, off);
    }

    if (self->h_.kind == PICO8_VALUE_REF && self->h_.as.ref != NULL) {
        const pico8_instance_t *c = (const pico8_instance_t *)self->h_.as.ref;
        pico8_fixed_t c_w = pico8_ui_field_fixed(c, "w");
        pico8_value_t bg_val;
        int bg = 2;
        pico8_fixed_t x2 = pico8_fixed_add(qt, c_w);

        if (pico8_instance_get_field(c, "bg", &bg_val) && bg_val.kind == PICO8_VALUE_FIXED) {
            bg = pico8_ui_to_int(bg_val.as.fixed);
        }
        pico8_rectfill(gfx, mem, pico8_ui_to_int(qt) + 3, iy_p + 1, pico8_ui_to_int(x2) + 4, iy_dn, bg);

        {
            pico8_vec_t off2 =
                pico8_vec_add(p, pico8_vec_v(pico8_fixed_add(mb, pico8_fixed_from_int(3)), pico8_fixed_from_int(2)));
            pico8_lq_ol_default(gfx, mem, c, off2);
        }
    }
}

pico8_value_t pico8_js_ldown(pico8_js_t *self) {
    pico8_value_t discard;
    int has_my = self != NULL && pico8_js_truthy(self->my);

    pico8_rn(has_my ? pico8_value_fixed(pico8_fixed_from_int(60)) : pico8_value_bool(0));
    pico8_mv((pico8_instance_t *)self, "my", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &discard);

    return pico8_value_bool(1);
}

static pico8_value_t pico8_fq_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_fq_ldown((pico8_fq_t *)self);
}

static int pico8_fq_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_fq_t *fq = self;
    const pico8_value_t *member;

    if (strcmp(name, "fa") == 0) {
        member = &fq->fa;
    } else if (strcmp(name, "hd") == 0) {
        member = &fq->hd;
    } else if (strcmp(name, "my") == 0) {
        member = &fq->my;
    } else if (strcmp(name, "p_") == 0) {
        member = &fq->p_;
    } else if (strcmp(name, "m_") == 0) {
        member = &fq->m_;
    } else {
        return pico8_ke_field(self, name, out_value);
    }
    if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
        return 0;
    }
    *out_value = *member;
    return 1;
}

static int pico8_fq_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_fq_t *fq = self;

    if (strcmp(name, "m_") != 0) {
        return 0;
    }
    fq->m_ = value;
    return 1;
}

static const pico8_struct_class_t g_fq_struct_class = {
    {[PICO8_METHOD_LDOWN] = pico8_fq_ldown_dispatch},
    pico8_fq_field,
    pico8_fq_set_field,
};

static void *pico8_fq_struct_new(const pico8_ob_table_t *overrides) {
    pico8_fq_t *self = malloc(sizeof *self);
    int i;

    pico8_ke_struct_init(&self->ke, &g_fq_struct_class);
    self->fa = pico8_value_fixed(pico8_fixed_from_int(14));
    self->hd = pico8_value_fixed(pico8_fixed_from_int(1));
    self->my = pico8_value_ref(NULL);
    self->p_ = pico8_value_ref(NULL);
    self->m_ = pico8_value_ref(NULL);
    for (i = 0; overrides != NULL && i < overrides->count; i++) {
        pico8_ob_key_t key = overrides->entries[i].key;

        if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "my") == 0) {
            self->my = overrides->entries[i].value;
        } else {
            assert(0 && "fq: the only measured key is my; another would be a field under no rule of plan-3 "
                        "section 5 -- stop, do not add a member");
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("fq");
#endif
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_fq_init(void) {
    pico8_fq_shutdown();
    g_fq_class = pico8_class_declare(pico8_ke_class(), "\nfa=14,hd=1,\n");
    PICO8_CLASS_COVERAGE_NAME(g_fq_class, "fq");
    pico8_class_set_struct_new(g_fq_class, pico8_fq_struct_new);
}

void pico8_fq_shutdown(void) {
    if (g_fq_class != NULL) {
        pico8_class_free(g_fq_class);
        g_fq_class = NULL;
    }
}

pico8_class_t *pico8_fq_class(void) {
    return g_fq_class;
}

const pico8_struct_class_t *pico8_fq_struct_class(void) {
    return &g_fq_struct_class;
}

pico8_fq_t *pico8_fq_new(const pico8_ob_table_t *overrides) {
    return pico8_fq_struct_new(overrides);
}

pico8_value_t pico8_fq_ldown(pico8_fq_t *self) {
    pico8_value_t my_result;

    if (self == NULL) {
        return pico8_value_bool(1);
    }
    if (!pico8_mv((pico8_instance_t *)self, "my", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &my_result)) {
        my_result = pico8_value_ref(NULL);
    }
    self->p_ = my_result;
    return pico8_value_bool(1);
}
