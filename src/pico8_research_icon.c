#include "pico8_research_icon.h"

#include <assert.h>
#include <stddef.h>
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

static pico8_ob_table_t *g_ie_chain_spec = NULL;

/* Owns the `kq` box and the `ra`/`eh` vectors every `ie` points at;
 * the shutdown frees the three with it. */
static pico8_ob_table_t *g_ie_spec = NULL;

/* `ie.gy`, `ie.id`: mutated on the class by the cart, so one cursor and one
 * counter shared by every icon, never a member. `gy` is rebound, never
 * written through: an icon's `ra`, and `eh`, may hold its REF. Only `_init`
 * resets `gy`, only pico8_research_icon_init() resets `id`. */
static pico8_value_t g_ie_gy = {PICO8_VALUE_REF, {.ref = NULL}};
static pico8_value_t g_ie_id = {PICO8_VALUE_REF, {.ref = NULL}};

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

/* Returns the kind held in `self.ke`, or NULL when the member is nil or
 * holds no REF. The result is borrowed. */
static const pico8_entity_kind_t *ie_self_ke_kind(const pico8_ie_t *self) {
    if (self != NULL && self->ke.kind == PICO8_VALUE_REF && self->ke.as.ref != NULL) {
        assert(pico8_type_tag_of(self->ke.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
               "ie.ke holds a kind, not an entity or a table");
        return (const pico8_entity_kind_t *)self->ke.as.ref;
    }
    return NULL;
}

static int pico8_ie_ke_view_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_ie_ke_t *view = self;
    pico8_value_t value;

    if (strcmp(name, "rr") == 0) {
        value = view->rr;
    } else if (strcmp(name, "s") == 0) {
        value = view->s;
    } else if (strcmp(name, "me") == 0) {
        value = view->me;
    } else if (strcmp(name, "od") == 0) {
        value = view->od;
    } else {
        return 0;
    }
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return 0;
    }
    *out_value = value;
    return 1;
}

static const pico8_struct_class_t g_ie_ke_view_struct_class = {
    {0},
    pico8_ie_ke_view_field,
    NULL,
};

pico8_ie_ke_t pico8_ie_ke_view(const pico8_ie_t *self) {
    const pico8_entity_kind_t *kind = ie_self_ke_kind(self);
    pico8_ie_ke_t view;

    view.header.tag = PICO8_TYPE_TAG_STRUCT;
    view.header.cls = &g_ie_ke_view_struct_class;
    view.rr = pico8_value_ref(NULL);
    view.s = pico8_value_ref(NULL);
    view.me = pico8_value_ref(NULL);
    view.od = pico8_value_ref(NULL);
    if (kind != NULL) {
        (void)kind->field("rr", &view.rr);
        (void)kind->field("s", &view.s);
        (void)kind->field("me", &view.me);
        (void)kind->field("od", &view.od);
    }
    PICO8_MEMBER_CENSUS_SEE(&view, pico8_ie_ke_t);
    return view;
}

const pico8_struct_class_t *pico8_ie_ke_view_struct_class(void) {
    return &g_ie_ke_view_struct_class;
}

/* A missing or wrong-kind value reads as the zero vector. */
static pico8_vec_t ie_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

void pico8_ie_hb(pico8_ie_t *self) {
    pico8_value_t rr_val;

    if (self == NULL) {
        return;
    }

    if (ie_truthy(self->kh)) {
        return;
    }

    /* `qh`/`os` never set `rr`, so `rr_val` stays absent for them and
     * pico8_oj() then answers true unconditionally. */
    rr_val = pico8_ie_ke_view(self).rr;

    if (!pico8_oj(rr_val)) {
        return;
    }

    assert(g_ie_chain_spec != NULL && "pico8_research_icon_init() must run before ie:hb writes ie.gy");

    /* `self.ra` takes the REF `ie.gy` holds now. The `-=` below rebinds
     * `ie.gy` to a fresh vector rather than writing through the old one, so
     * this snapshot survives it. */
    self->ra = g_ie_gy;
    self->kh = pico8_value_bool(1);

    {
        pico8_vec_t current = ie_value_vec(g_ie_gy);
        pico8_vec_t delta = pico8_vec_v(pico8_fixed_from_int(10), pico8_fixed_from_int(0));
        pico8_vec_t *heap = malloc(sizeof *heap);
        *heap = pico8_vec_sub(current, delta);
        g_ie_gy = pico8_value_ref(heap);
    }
}

static pico8_value_t pico8_ie_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_ie_hb((pico8_ie_t *)self);
    return pico8_value_ref(NULL);
}

pico8_value_t pico8_ie_ldown(pico8_ie_t *self, pico8_value_t mp) {
    pico8_fixed_t new_id;
    const pico8_entity_kind_t *ke_kind;
    char *id_num_str;
    char *id_str;
    pico8_ob_table_t *overrides;
    void *made;

    (void)mp;

    assert(g_ie_chain_spec != NULL && "pico8_research_icon_init() must run before ie:ldown writes ie.id");
    new_id = pico8_fixed_add(g_ie_id.kind == PICO8_VALUE_FIXED ? g_ie_id.as.fixed : pico8_fixed_from_int(0),
                             pico8_fixed_from_int(1));
    g_ie_id = pico8_value_fixed(new_id);

    ke_kind = ie_self_ke_kind(self);
    if (ke_kind == NULL) {
        return pico8_value_ref(NULL);
    }

    id_num_str = pico8_value_tostr(pico8_value_fixed(new_id));
    id_str = ie_concat2("s", id_num_str);
    free(id_num_str);

    /* The constructor copies each entry, so `overrides` is freed right
     * after -- but `id_str` is not: the new planet's `id` keeps that exact
     * pointer. */
    overrides = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides, "id", pico8_value_string(id_str));
    made = ke_kind->new_entity(overrides);
    pico8_ob_table_free(overrides);

    return pico8_value_ref(made);
}

static pico8_value_t pico8_ie_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg1;
    return pico8_ie_ldown((pico8_ie_t *)self, arg0);
}

void pico8_ie_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ie_t *self, pico8_vec_t p) {
    pico8_ie_ke_t view;

    /* An icon not yet revealed by `ie:hb` draws nothing at all. */
    if (self == NULL || !ie_truthy(self->kh)) {
        return;
    }
    if (gfx == NULL || mem == NULL) {
        return;
    }

    pico8_palt(mem, 3, 0);

    view = pico8_ie_ke_view(self);
    pico8_spr_default(gfx, mem, view.s.kind == PICO8_VALUE_FIXED ? ie_to_int(view.s.as.fixed) : 0, ie_to_int(p.x),
                      ie_to_int(p.y));

    /* `dl(self.ke)` is not inert: every kind `self.ke` can hold inherits
     * `qh`'s `me="kz"`, so `dl` passes its own state check and draws the
     * kind-level build cost. */
    if (ie_truthy(self->m_)) {
        if (ie_self_ke_kind(self) != NULL) {
            pico8_dl(gfx, mem, (pico8_struct_header_t *)&view);
        }
        if (self->jb.kind == PICO8_VALUE_REF && self->jb.as.ref != NULL) {
            assert(pico8_type_tag_of(self->jb.as.ref) == PICO8_TYPE_TAG_STRUCT &&
                   "ie: unlike self.ke, self.jb holds a built jb struct");
            pico8_jb_ol(gfx, mem, (const pico8_jb_t *)self->jb.as.ref, pico8_fixed_from_int(0),
                        pico8_fixed_from_int(0));
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
    pico8_ie_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_ie_t *)self, p);
    return pico8_value_ref(NULL);
}

/* `ie.hd=gk`: a field assignment in the cart, not a method definition. */
static pico8_value_t pico8_ie_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_gk((const pico8_struct_header_t *)self, arg0, arg1);
}

/* `gy` and `id` read off an icon answer the shared state, as the class
 * chain did. */
static int pico8_ie_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_ie_t *ie = self;
    static const struct {
        const char *name;
        size_t offset;
    } members[] = {
        {"kq", offsetof(pico8_ie_t, kq)}, {"fa", offsetof(pico8_ie_t, fa)}, {"ra", offsetof(pico8_ie_t, ra)},
        {"eh", offsetof(pico8_ie_t, eh)}, {"ke", offsetof(pico8_ie_t, ke)}, {"jb", offsetof(pico8_ie_t, jb)},
        {"kh", offsetof(pico8_ie_t, kh)}, {"m_", offsetof(pico8_ie_t, m_)},
    };
    pico8_value_t value;
    size_t i;

    if (strcmp(name, "gy") == 0) {
        value = g_ie_gy;
    } else if (strcmp(name, "id") == 0) {
        value = g_ie_id;
    } else {
        for (i = 0; i < sizeof members / sizeof members[0]; i++) {
            if (strcmp(name, members[i].name) == 0) {
                break;
            }
        }
        if (i == sizeof members / sizeof members[0]) {
            return pico8_ke_field(self, name, out_value);
        }
        value = *(const pico8_value_t *)((const char *)ie + members[i].offset);
    }
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return 0;
    }
    *out_value = value;
    return 1;
}

/* `m_` (the hover loop, on `ok.o_` and `ok.gj`): the only name a generic
 * writer ever writes on an `ie`. */
static int pico8_ie_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_ie_t *ie = self;

    if (strcmp(name, "m_") != 0) {
        return 0;
    }
    ie->m_ = value;
    return 1;
}

static const pico8_struct_class_t g_ie_struct_class = {
    {
        [PICO8_METHOD_HD] = pico8_ie_hd_dispatch,
        [PICO8_METHOD_HB] = pico8_ie_hb_dispatch,
        [PICO8_METHOD_KJ] = pico8_ie_kj_dispatch,
        [PICO8_METHOD_LDOWN] = pico8_ie_ldown_dispatch,
    },
    pico8_ie_field,
    pico8_ie_set_field,
};

static pico8_value_t pico8_ie_spec_ref(const char *name) {
    pico8_value_t v = pico8_value_ref(NULL);
    int found = g_ie_spec != NULL && pico8_ob_table_get_name(g_ie_spec, name, &v);

    assert(found && v.kind == PICO8_VALUE_REF && v.as.ref != NULL &&
           "pico8_research_icon_init() must run before an ie is built: kq, ra and eh are the module's");
    (void)found;
    return v;
}

static void *pico8_ie_struct_new(const pico8_ob_table_t *overrides) {
    pico8_ie_t *self = malloc(sizeof *self);
    int i;

    pico8_ke_struct_init(&self->ke_base, &g_ie_struct_class);
    self->kq = pico8_ie_spec_ref("kq");
    self->fa = pico8_value_fixed(pico8_fixed_from_int(12));
    self->ra = pico8_ie_spec_ref("ra");
    self->eh = pico8_ie_spec_ref("eh");
    self->ke = pico8_value_ref(NULL);
    self->jb = pico8_value_ref(NULL);
    self->kh = pico8_value_ref(NULL);
    self->m_ = pico8_value_ref(NULL);
    for (i = 0; overrides != NULL && i < overrides->count; i++) {
        pico8_ob_key_t key = overrides->entries[i].key;
        int is_ke = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "ke") == 0;
        int is_jb = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "jb") == 0;

        assert((is_ke || is_jb) && "ie: the only measured construction keys are ke and jb (eu); another would be "
                                   "a field under no rule of plan-3 section 5 -- stop, do not add a member");
        if (is_ke) {
            self->ke = overrides->entries[i].value;
        } else {
            self->jb = overrides->entries[i].value;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("ie");
#endif
    pico8_ke_struct_pq(&self->ke_base);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_ie_t);
    return self;
}

void pico8_research_icon_init(void) {
    pico8_research_icon_shutdown();

    pico8_pi_init();

    g_ie_chain_spec = pico8_ob("\nkq=b(0,0,9,8),\nfa=12,\nid=0,\nra=v(999,999),\neh=v(118,120),\n", NULL);

    g_ie_spec = pico8_ob("\nkq=b(0,0,9,8),\nra=v(999,999),\neh=v(118,120),\n", NULL);
    g_ie_gy = pico8_value_ref(NULL);
    g_ie_id = pico8_value_fixed(pico8_fixed_from_int(0));
}

/* TROU-7 */
void pico8_research_icon_shutdown(void) {
    pico8_ob_table_free(g_ie_chain_spec);
    g_ie_chain_spec = NULL;
    if (g_ie_spec != NULL) {
        int i;
        for (i = 0; i < g_ie_spec->count; i++) {
            free(g_ie_spec->entries[i].value.as.ref);
        }
        pico8_ob_table_free(g_ie_spec);
        g_ie_spec = NULL;
    }
    g_ie_gy = pico8_value_ref(NULL);
}

int pico8_ie_ready(void) {
    return g_ie_chain_spec != NULL;
}

int pico8_ie_default(const char *name, pico8_value_t *out_value) {
    return g_ie_chain_spec != NULL &&
           (pico8_ob_table_get_name(g_ie_chain_spec, name, out_value) || pico8_ke_default(name, out_value));
}

const pico8_struct_class_t *pico8_ie_struct_class(void) {
    return &g_ie_struct_class;
}

pico8_ie_t *pico8_ie_new(const pico8_ob_table_t *overrides) {
    return pico8_ie_struct_new(overrides);
}

pico8_value_t pico8_ie_eh(void) {
    return g_ie_spec == NULL ? pico8_value_ref(NULL) : pico8_ie_spec_ref("eh");
}

void pico8_ie_set_gy(pico8_value_t gy) {
    g_ie_gy = gy;
}
