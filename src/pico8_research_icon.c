#include "pico8_research_icon.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity_geom.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_layout.h"
#include "pico8_pi.h"
#include "pico8_probe.h"
#include "pico8_tech_gate.h"

static pico8_class_t *g_ie_class = NULL;

/* Shared by every wrapper below, and safe to share only because
 * pico8_instance_get_field() reads through `fields` and never writes it. */
static pico8_ob_table_t g_ie_empty_fields = {PICO8_TYPE_TAG_OB_TABLE, NULL, 0, 0};

static int ie_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

/* Lua truthiness -- fixed 0 included -- widened so this port's "absent"
 * filler REF(NULL) is falsy too. */
static int ie_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* Returns a newly heap-allocated string the caller owns and must free(). */
static char *ie_concat2(const char *a, const char *b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char *out = malloc(la + lb + 1);
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = '\0';
    return out;
}

/* Returns the CLASS held in `self.ke`, or NULL when the field is absent or
 * holds no REF. The result is borrowed. */
static pico8_class_t *ie_self_ke_class(const pico8_instance_t *self) {
    pico8_value_t ke_val;
    if (self != NULL && pico8_instance_get_field(self, "ke", &ke_val) && ke_val.kind == PICO8_VALUE_REF &&
        ke_val.as.ref != NULL) {
        assert(pico8_type_tag_of(ke_val.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
               "ie.ke holds a class, not an instance or a table");
        return (pico8_class_t *)ke_val.as.ref;
    }
    return NULL;
}

/* A transient, stack-allocated classless instance whose `cls` is the class
 * to read fields off. It cannot be built once and shared: different icons
 * hold different classes in `self.ke`. */
static pico8_instance_t ie_ke_wrapper(pico8_class_t *cls) {
    pico8_instance_t wrapper;
    wrapper.tag = PICO8_TYPE_TAG_INSTANCE;
    wrapper.fields = &g_ie_empty_fields;
    wrapper.cls = cls;
    return wrapper;
}

/* Reads `self.ke.<name>` through the wrapper's class-chain walk. Returns 0
 * and leaves `*out` untouched when `self.ke` is absent. */
static int ie_ke_field(const pico8_instance_t *self, const char *name, pico8_value_t *out) {
    pico8_class_t *cls = ie_self_ke_class(self);
    pico8_instance_t wrapper;
    if (cls == NULL) {
        return 0;
    }
    wrapper = ie_ke_wrapper(cls);
    return pico8_instance_get_field(&wrapper, name, out);
}

/* Reads one of `ie`'s own class-level defaults, returning this port's
 * "absent" filler when the class is not built or the field has none. */
static pico8_value_t ie_class_field_value(const char *name) {
    pico8_value_t v;
    if (g_ie_class != NULL && g_ie_class->defaults != NULL && pico8_ob_table_get_name(g_ie_class->defaults, name, &v)) {
        return v;
    }
    return pico8_value_ref(NULL);
}

static pico8_fixed_t ie_class_field_fixed(const char *name) {
    pico8_value_t v = ie_class_field_value(name);
    if (v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

/* A missing or wrong-kind value reads as the zero vector. */
static pico8_vec_t ie_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t ie_class_field_vec(const char *name) {
    return ie_value_vec(ie_class_field_value(name));
}

void pico8_ie_hb(pico8_instance_t *self) {
    pico8_value_t kh_val;
    pico8_value_t rr_val = pico8_value_ref(NULL);

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "kh", &kh_val) && ie_truthy(kh_val)) {
        return;
    }

    /* `qh`/`os` never set `rr`, so `rr_val` stays absent for them and
     * pico8_oj() then answers true unconditionally. */
    ie_ke_field(self, "rr", &rr_val);

    if (!pico8_oj(rr_val)) {
        return;
    }

    assert(g_ie_class != NULL && "pico8_research_icon_init() must run before ie:hb writes ie.gy");

    /* `self.ra` takes the REF `ie.gy` holds now. The `-=` below rebinds
     * `ie.gy` to a fresh vector rather than writing through the old one, so
     * this snapshot survives it. */
    {
        pico8_value_t own_gy;
        assert(!pico8_ob_table_get_name(self->fields, "gy", &own_gy) &&
               "gy is one cursor shared by every ie: a per-instance gy breaks the reveal");
        (void)own_gy;
    }
    {
        pico8_value_t gy_snapshot = ie_class_field_value("gy");
        pico8_ob_table_set_name(self->fields, "ra", gy_snapshot);
        pico8_ob_table_set_name(self->fields, "kh", pico8_value_bool(1));
    }

    {
        pico8_vec_t current = ie_class_field_vec("gy");
        pico8_vec_t delta = pico8_vec_v(pico8_fixed_from_int(10), pico8_fixed_from_int(0));
        pico8_vec_t *heap = malloc(sizeof *heap);
        *heap = pico8_vec_sub(current, delta);
        pico8_ob_table_set_name(g_ie_class->defaults, "gy", pico8_value_ref(heap));
    }
}

static pico8_value_t pico8_ie_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_ie_hb((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

pico8_value_t pico8_ie_ldown(pico8_instance_t *self, pico8_value_t mp) {
    pico8_fixed_t new_id;
    pico8_class_t *ke_cls;
    char *id_num_str;
    char *id_str;
    pico8_ob_table_t *overrides;
    pico8_instance_t *made;

    (void)mp;

    assert(g_ie_class != NULL && "pico8_research_icon_init() must run before ie:ldown writes ie.id");
    new_id = pico8_fixed_add(ie_class_field_fixed("id"), pico8_fixed_from_int(1));
    pico8_ob_table_set_name(g_ie_class->defaults, "id", pico8_value_fixed(new_id));

    ke_cls = ie_self_ke_class(self);
    if (ke_cls == NULL) {
        return pico8_value_ref(NULL);
    }

    id_num_str = pico8_value_tostr(pico8_value_fixed(new_id));
    id_str = ie_concat2("s", id_num_str);
    free(id_num_str);

    /* pico8_class_instantiate() shallow-copies each entry, so `overrides`
     * is freed right after -- but `id_str` is not: the new instance's "id"
     * field keeps that exact pointer. */
    overrides = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides, "id", pico8_value_string(id_str));
    made = pico8_class_instantiate(ke_cls, overrides);
    pico8_ob_table_free(overrides);

    return pico8_value_ref(made);
}

static pico8_value_t pico8_ie_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg1;
    return pico8_ie_ldown((pico8_instance_t *)self, arg0);
}

void pico8_ie_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p) {
    pico8_value_t kh_val;

    /* An icon not yet revealed by `ie:hb` draws nothing at all. */
    if (self == NULL || !pico8_instance_get_field(self, "kh", &kh_val) || !ie_truthy(kh_val)) {
        return;
    }
    if (gfx == NULL || mem == NULL) {
        return;
    }

    pico8_palt(mem, 3, 0);

    {
        pico8_value_t s_val;
        int s_id = 0;
        if (ie_ke_field(self, "s", &s_val) && s_val.kind == PICO8_VALUE_FIXED) {
            s_id = ie_to_int(s_val.as.fixed);
        }
        pico8_spr_default(gfx, mem, s_id, ie_to_int(p.x), ie_to_int(p.y));
    }

    /* `dl(self.ke)` is not inert: every class `self.ke` can hold inherits
     * `qh`'s `me="kz"`, so `dl` passes its own state check and draws the
     * class-level build cost. */
    {
        pico8_value_t m_val;
        if (pico8_instance_get_field(self, "m_", &m_val) && ie_truthy(m_val)) {
            pico8_class_t *ke_cls = ie_self_ke_class(self);
            pico8_value_t jb_val;

            if (ke_cls != NULL) {
                pico8_instance_t wrapper = ie_ke_wrapper(ke_cls);
                pico8_dl(gfx, mem, &wrapper);
            }
            if (pico8_instance_get_field(self, "jb", &jb_val) && jb_val.kind == PICO8_VALUE_REF &&
                jb_val.as.ref != NULL) {
                assert(pico8_type_tag_of(jb_val.as.ref) == PICO8_TYPE_TAG_STRUCT &&
                       "ie: unlike self.ke, self.jb holds a built jb struct");
                pico8_jb_ol(gfx, mem, (const pico8_jb_t *)jb_val.as.ref, pico8_fixed_from_int(0),
                            pico8_fixed_from_int(0));
            }
        }
    }

    pico8_palt(mem, 3, 1);
}

/* `dc("kj")` calls `qc[nt](qc,qc.ra)`, so `arg0` is always `self.ra`. */
static pico8_value_t pico8_ie_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_ie_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self, p);
    return pico8_value_ref(NULL);
}

/* `ie.hd=gk`: a field assignment in the cart, not a method definition. */
static pico8_value_t pico8_ie_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_gk((const pico8_instance_t *)self, arg0, arg1);
}

void pico8_research_icon_init(void) {
    pico8_research_icon_shutdown();

    pico8_pi_init();

    g_ie_class =
        pico8_class_declare(pico8_ke_class(), "\nkq=b(0,0,9,8),\nfa=12,\nid=0,\nra=v(999,999),\neh=v(118,120),\n");
    PICO8_CLASS_COVERAGE_NAME(g_ie_class, "ie");

    pico8_ob_table_set_name(g_ie_class->defaults, "hd", pico8_value_function(pico8_ie_hd_dispatch));
    pico8_ob_table_set_name(g_ie_class->defaults, "hb", pico8_value_function(pico8_ie_hb_dispatch));
    pico8_ob_table_set_name(g_ie_class->defaults, "kj", pico8_value_function(pico8_ie_kj_dispatch));
    pico8_ob_table_set_name(g_ie_class->defaults, "ldown", pico8_value_function(pico8_ie_ldown_dispatch));
}

/* TROU-7 */
void pico8_research_icon_shutdown(void) {
    if (g_ie_class != NULL) {
        pico8_class_free(g_ie_class);
        g_ie_class = NULL;
    }
}

pico8_class_t *pico8_ie_class(void) {
    return g_ie_class;
}

pico8_instance_t *pico8_ie_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_ie_class, overrides);
}
