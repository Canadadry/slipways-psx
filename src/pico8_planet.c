#include "pico8_planet.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_economy.h"
#include "pico8_entity_geom.h"
#include "pico8_gen.h"
#include "pico8_ke.h"
#include "pico8_msgbox.h"
#include "pico8_pi.h"
#include "pico8_probe.h"
#include "pico8_research.h"
#include "pico8_research_display.h"
#include "pico8_stash.h"
#include "pico8_tech_gate.h"

static pico8_class_t *g_jo_class = NULL;

/* jo.oc/jo.ir (this file's own top comment on the sub-lot 2 `hh`
 * declaration) -- module-private storage for the two class-level
 * generator-closure fields, set only by pico8_jo_set_oc/pico8_jo_set_ir
 * (Chunk 20's own future `bw()` port). Always NULL until then. */
static pico8_gn_state_t *g_jo_oc = NULL;
static pico8_jf_state_t *g_jo_ir = NULL;

static int pico8_jo_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* field_truthy/field_fixed/field_list/field_ref_table: the same small
 * "read a named field off an instance, with a documented default when
 * absent/wrong-kind" helpers already independently established (and
 * already flagged as an accepted, documented duplication) in
 * pico8_economy.c/pico8_economy_tiers.c/pico8_economy_formulas.c -- a
 * private per-module copy here, not shared, per this port's pervasive
 * "small per-module private helper" convention. */
static int field_truthy(const pico8_instance_t *self, const char *name) {
    pico8_value_t v;
    if (self == NULL || !pico8_instance_get_field(self, name, &v)) {
        return 0;
    }
    return pico8_jo_truthy(v);
}

static pico8_fixed_t field_fixed(const pico8_instance_t *self, const char *name) {
    pico8_value_t v;
    if (self != NULL && pico8_instance_get_field(self, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static const pico8_list_t *field_list(const pico8_instance_t *self, const char *name) {
    pico8_value_t v;
    if (self != NULL && pico8_instance_get_field(self, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (const pico8_list_t *)v.as.ref;
    }
    return NULL;
}

static const pico8_ob_table_t *field_ref_table(const pico8_instance_t *self, const char *name) {
    pico8_value_t v;
    if (self != NULL && pico8_instance_get_field(self, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (const pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

/* Not a port of any named Lua function: private per-module bridge from
 * "positional entries of a pico8_ob_table_t" to a pico8_list_t (used here
 * for `jo.cq`, `jo:gh`'s own `gu(jo.cq,self.kw.qe)`) -- the same
 * established pattern already independently duplicated by
 * pico8_research_display_positional_values/pico8_layout_positional_values/
 * pico8_msgbox_positional_values (documented, accepted duplication). */
static pico8_list_t pico8_jo_positional_values(const pico8_ob_table_t *table) {
    pico8_list_t list;
    int i;

    pico8_list_init(&list);
    if (table == NULL) {
        return list;
    }
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
            pico8_list_add(&list, table->entries[i].value);
        }
    }
    return list;
}

static void pico8_jo_pq(pico8_instance_t *self);

static pico8_value_t pico8_jo_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_gk((const pico8_instance_t *)self, arg0, arg1);
}

static pico8_value_t pico8_jo_gg_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_jo_gg((const pico8_instance_t *)self);
}

static pico8_value_t pico8_jo_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_jo_hb((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* "gv" is not one of `bm`'s 5 real dispatch names, but is still a real
 * mv()-routed field (`ui:jz("gv",held)`, pico8_ui.c) -- same "field
 * function, no `cv` auto-registration" treatment as `gg`/`hb` above. */
static pico8_value_t pico8_jo_gv_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    const pico8_instance_t *s = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        s = (const pico8_instance_t *)arg0.as.ref;
    }
    return pico8_jo_gv((const pico8_instance_t *)self, s);
}

static pico8_value_t pico8_jo_hk_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_jo_hk((const pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

void pico8_jo_init(void) {
    pico8_jo_shutdown();
    pico8_pi_init();
    g_jo_class = pico8_class_declare(pico8_ke_class(),
                                      "\n"
                                      "nu=o(\"jo\",\"np\",\"prober\"),\n"
                                      "me=\"unknown\",\n"
                                      "hf=1,hs=1,\n"
                                      "fa=6,\n"
                                      "kq=b(-7,-7,7,7),\n"
                                      "ma=0,\n"
                                      "ka=o(2,3,9,12),\n"
                                      "dz=o(\n"
                                      "o(\"unhappy\",c=8),\n"
                                      "o(\"content\",c=3),\n"
                                      "o(\"prosperous\",c=9),\n"
                                      "o(\"rich\",c=12),\n"
                                      "),\n"
                                      "cq=o(12,36,24,25),\n"
                                      "kw=o(mu=0),\n"
                                      "gq=v(0,11),ca=9,\n"
                                      "em=o(f,3,0,-1),\n");
    pico8_class_set_pq(g_jo_class, pico8_jo_pq);
    pico8_ob_table_set_name(g_jo_class->defaults, "hd", pico8_value_function(pico8_jo_hd_dispatch));
    pico8_ob_table_set_name(g_jo_class->defaults, "gg", pico8_value_function(pico8_jo_gg_dispatch));
    pico8_ob_table_set_name(g_jo_class->defaults, "hb", pico8_value_function(pico8_jo_hb_dispatch));
    pico8_ob_table_set_name(g_jo_class->defaults, "gv", pico8_value_function(pico8_jo_gv_dispatch));
    pico8_ob_table_set_name(g_jo_class->defaults, "hk", pico8_value_function(pico8_jo_hk_dispatch));
    pico8_class_register_named("jo", g_jo_class);
}

void pico8_jo_shutdown(void) {
    if (g_jo_class != NULL) {
        pico8_class_free(g_jo_class);
        g_jo_class = NULL;
    }
    if (g_jo_oc != NULL) {
        pico8_gn_free(g_jo_oc);
        g_jo_oc = NULL;
    }
    if (g_jo_ir != NULL) {
        pico8_jf_free(g_jo_ir);
        g_jo_ir = NULL;
    }
}

void pico8_jo_set_oc(pico8_gn_state_t *oc) {
    if (g_jo_oc == oc) {
        return;
    }
    /* Bare, unconditional call: pico8_gn_free (src/pico8_gen.c) now starts
     * with the same NULL guard as every other teardown function in this
     * port (pico8_instance_free, pico8_class_free, pico8_ob_table_free,
     * pico8_jf_free below...) -- the local workaround this setter used to
     * need (guarding the call here, since the real fix belonged to a
     * different chunk) is no longer needed now that pico8_gn_free itself
     * is fixed. */
    pico8_gn_free(g_jo_oc);
    g_jo_oc = oc;
}

void pico8_jo_set_ir(pico8_jf_state_t *ir) {
    if (g_jo_ir == ir) {
        return;
    }
    pico8_jf_free(g_jo_ir);
    g_jo_ir = ir;
}

pico8_class_t *pico8_jo_class(void) {
    return g_jo_class;
}

pico8_instance_t *pico8_jo_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_jo_class, overrides);
}

static void pico8_jo_pq(pico8_instance_t *self) {
    pico8_value_t hp_val;
    pico8_list_t *oe;

    if (self == NULL) {
        return;
    }

    oe = malloc(sizeof *oe);
    pico8_list_init(oe);
    pico8_ob_table_set_name(self->fields, "oe", pico8_value_ref(oe));

    if (pico8_instance_get_field(self, "hp", &hp_val) && pico8_jo_truthy(hp_val)) {
        const pico8_ob_table_t *gb = pico8_gb();
        pico8_value_t target;
        int found = 0;

        if (hp_val.kind == PICO8_VALUE_FIXED) {
            found = pico8_ob_table_get_index(gb, (int)pico8_fixed_to_double(hp_val.as.fixed), &target);
        } else if (hp_val.kind == PICO8_VALUE_STRING) {
            found = pico8_ob_table_get_name(gb, hp_val.as.string, &target);
        }
        if (found && target.kind == PICO8_VALUE_REF && target.as.ref != NULL) {
            pico8_jo_lz(self, (pico8_instance_t *)target.as.ref);
        }
    }
}

static int pico8_jo_lz_mw_step(pico8_value_t e, void *ctx, pico8_value_t *out_value) {
    (void)ctx;
    if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
        *out_value = pico8_value_ref(pico8_mw((const pico8_ob_table_t *)e.as.ref));
        return 1;
    }
    return 0;
}

void pico8_jo_lz(pico8_instance_t *self, pico8_instance_t *hp_instance) {
    pico8_value_t i_val;
    pico8_value_t o_val;
    pico8_list_t jy;
    pico8_list_t *jy_heap;
    pico8_ob_table_t *kw;

    if (self == NULL) {
        return;
    }

    pico8_ob_table_set_name(self->fields, "hn", pico8_value_ref(hp_instance));
    pico8_ob_table_set_name(self->fields, "ep", pico8_value_ref(NULL));

    pico8_list_init(&jy);
    if (hp_instance != NULL && pico8_instance_get_field(hp_instance, "i", &i_val) &&
        i_val.kind == PICO8_VALUE_REF && i_val.as.ref != NULL) {
        jy = pico8_nv((const pico8_list_t *)i_val.as.ref, pico8_jo_lz_mw_step, NULL);
    }
    jy_heap = malloc(sizeof *jy_heap);
    *jy_heap = jy;
    pico8_ob_table_set_name(self->fields, "jy", pico8_value_ref(jy_heap));

    kw = pico8_ob_table_new();
    if (hp_instance != NULL && pico8_instance_get_field(hp_instance, "o", &o_val)) {
        pico8_ob_table_set_name(kw, "qe", o_val);
    }
    pico8_ob_table_set_name(kw, "mu", pico8_value_fixed(pico8_fixed_from_int(0)));
    pico8_ob_table_set_name(self->fields, "kw", pico8_value_ref(kw));
}

pico8_value_t pico8_jo_gg(const pico8_instance_t *self) {
    pico8_value_t hf_val;
    pico8_value_t me_val;

    if (!pico8_instance_get_field(self, "hf", &hf_val)) {
        return pico8_value_ref(NULL);
    }
    if (!pico8_jo_truthy(hf_val)) {
        return hf_val;
    }
    if (pico8_instance_get_field(self, "me", &me_val) && me_val.kind == PICO8_VALUE_STRING) {
        return pico8_value_bool(strcmp(me_val.as.string, "ly") == 0);
    }
    return pico8_value_bool(0);
}

void pico8_jo_kd(pico8_instance_t *self) {
    if (self == NULL) {
        return;
    }
    pico8_ob_table_set_name(self->fields, "ma", pico8_value_fixed(pico8_fixed_from_double(1.25)));
}

void pico8_jo_ll(pico8_instance_t *self, pico8_fixed_t t) {
    if (self == NULL) {
        return;
    }
    pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(t > pico8_fixed_from_int(60)));
}

void pico8_jo_hb(pico8_instance_t *self) {
    pico8_value_t jy_val;
    pico8_value_t kw_val;
    pico8_value_t by_val;
    const pico8_list_t *jy_list = NULL;
    const pico8_list_t *by_list = NULL;
    pico8_value_t qe = pico8_value_ref(NULL);
    pico8_fixed_t mu = pico8_fixed_from_int(0);
    pico8_instance_t *lq_result;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "jy", &jy_val) && jy_val.kind == PICO8_VALUE_REF && jy_val.as.ref != NULL) {
        jy_list = (const pico8_list_t *)jy_val.as.ref;
    }
    if (pico8_instance_get_field(self, "kw", &kw_val) && kw_val.kind == PICO8_VALUE_REF && kw_val.as.ref != NULL) {
        const pico8_ob_table_t *kw = (const pico8_ob_table_t *)kw_val.as.ref;
        pico8_value_t mu_val;
        if (!pico8_ob_table_get_name(kw, "qe", &qe)) {
            qe = pico8_value_ref(NULL);
        }
        if (pico8_ob_table_get_name(kw, "mu", &mu_val) && mu_val.kind == PICO8_VALUE_FIXED) {
            mu = mu_val.as.fixed;
        }
    }
    if (pico8_instance_get_field(self, "by", &by_val) && by_val.kind == PICO8_VALUE_REF && by_val.as.ref != NULL) {
        by_list = (const pico8_list_t *)by_val.as.ref;
    }

    lq_result = pico8_jo_ex(jy_list, qe, mu, pico8_fixed_from_int(by_list != NULL ? by_list->count : 0),
                             pico8_fixed_from_int(0));
    pico8_ob_table_set_name(self->fields, "lq", pico8_value_ref(lq_result));
}

/* Sub-lot 2 (see this file's own header top comment): gh, gv, hu, hh, hk. */

typedef struct {
    pico8_list_t *ji;
} pico8_jo_gh_ctx_t;

/* jp(self.jy,function(i)for j=1,i.gc do add(ji,i)end return i.gc==0 end):
 * the predicate's own side effect (appending `i` to the captured `ji`
 * list `i.gc` times) threaded through pico8_jp's own ctx parameter. */
static int pico8_jo_gh_jp_step(pico8_value_t e, void *ctx_ptr) {
    pico8_jo_gh_ctx_t *ctx = (pico8_jo_gh_ctx_t *)ctx_ptr;
    pico8_fixed_t gc = pico8_fixed_from_int(0);
    int count;
    int j;

    if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
        pico8_value_t gc_val;
        if (pico8_ob_table_get_name((const pico8_ob_table_t *)e.as.ref, "gc", &gc_val) &&
            gc_val.kind == PICO8_VALUE_FIXED) {
            gc = gc_val.as.fixed;
        }
    }
    count = (int)pico8_fixed_to_double(gc);
    for (j = 0; j < count; j++) {
        pico8_list_add(ctx->ji, e);
    }
    return gc == pico8_fixed_from_int(0);
}

/* `{lq=<list>}` -- one of jo:gh's own `bs` rows (pico8_pz's own documented
 * `{lq=...,h_=...}` row shape, pico8_msgbox.h). */
static pico8_ob_table_t *pico8_jo_gh_row(pico8_list_t *lq_list) {
    pico8_ob_table_t *row = pico8_ob_table_new();
    pico8_ob_table_set_name(row, "lq", pico8_value_ref(lq_list));
    return row;
}

void pico8_jo_gh(const pico8_instance_t *self) {
    pico8_list_t bs;
    pico8_ob_table_t *bs1;
    pico8_value_t px_val;
    pico8_value_t icon_val = pico8_value_ref(NULL);
    pico8_value_t kr_val;
    pico8_value_t h_field;
    pico8_list_t *ji;
    pico8_jo_gh_ctx_t ctx;
    const pico8_list_t *jy;
    pico8_list_t jx;
    int i;

    if (self == NULL) {
        return;
    }

    pico8_list_init(&bs);

    /* bs={{lq=self.px or jo.dz[self.mi]}} */
    bs1 = pico8_ob_table_new();
    if (pico8_instance_get_field(self, "px", &px_val) && pico8_jo_truthy(px_val)) {
        icon_val = px_val;
    } else {
        pico8_value_t mi_val;
        if (pico8_instance_get_field(self, "mi", &mi_val) && mi_val.kind == PICO8_VALUE_FIXED &&
            g_jo_class != NULL) {
            pico8_value_t dz_val;
            if (pico8_ob_table_get_name(g_jo_class->defaults, "dz", &dz_val) && dz_val.kind == PICO8_VALUE_REF &&
                dz_val.as.ref != NULL) {
                pico8_ob_table_get_index((const pico8_ob_table_t *)dz_val.as.ref,
                                          (int)pico8_fixed_to_double(mi_val.as.fixed), &icon_val);
            }
        }
    }
    pico8_ob_table_set_name(bs1, "lq", icon_val);

    /* bs[1].h_=self.kr and {ne(self.kr).."$",bg=self.kr>=0 and 1} */
    if (pico8_instance_get_field(self, "kr", &kr_val) && pico8_jo_truthy(kr_val)) {
        pico8_ob_table_t *h_tuple = pico8_ob_table_new();
        int discard_color = 0;
        pico8_fixed_t kr_fixed = (kr_val.kind == PICO8_VALUE_FIXED) ? kr_val.as.fixed : pico8_fixed_from_int(0);
        char *text = pico8_ne(kr_fixed, 0, NULL, &discard_color);
        size_t text_len = strlen(text);
        char *with_suffix = malloc(text_len + 2);
        memcpy(with_suffix, text, text_len);
        with_suffix[text_len] = '$';
        with_suffix[text_len + 1] = '\0';
        free(text);
        pico8_ob_table_set_index(h_tuple, 1, pico8_value_string(with_suffix));
        pico8_ob_table_set_name(h_tuple, "bg",
                                 (kr_val.kind == PICO8_VALUE_FIXED && kr_val.as.fixed >= pico8_fixed_from_int(0))
                                     ? pico8_value_fixed(pico8_fixed_from_int(1))
                                     : pico8_value_bool(0));
        h_field = pico8_value_ref(h_tuple);
    } else {
        h_field = pico8_value_ref(NULL);
    }
    pico8_ob_table_set_name(bs1, "h_", h_field);
    pico8_list_add(&bs, pico8_value_ref(bs1));

    /* local ji={"receives:"} */
    ji = malloc(sizeof *ji);
    pico8_list_init(ji);
    pico8_list_add(ji, pico8_value_string("receives:"));
    ctx.ji = ji;

    /* local jx=jp(self.jy,function(i)...end) */
    jy = field_list(self, "jy");
    jx = pico8_jp(jy, pico8_jo_gh_jp_step, &ctx);

    if (jx.count > 0) {
        pico8_list_t prefix;
        pico8_list_t *merged = malloc(sizeof *merged);
        pico8_list_init(&prefix);
        pico8_list_add(&prefix, pico8_value_string("needs:"));
        *merged = pico8_la(&prefix, &jx);
        pico8_list_free(&prefix);
        pico8_list_add(&bs, pico8_value_ref(pico8_jo_gh_row(merged)));
    } else {
        pico8_fixed_t mi_fixed = field_fixed(self, "mi");
        const pico8_ob_table_t *kw = field_ref_table(self, "kw");
        pico8_value_t kw_qe = pico8_value_ref(NULL);
        pico8_value_t r_w;
        int qe_is_r_w;

        if (kw != NULL) {
            pico8_ob_table_get_name(kw, "qe", &kw_qe);
        }
        qe_is_r_w = pico8_ob_table_get_name(pico8_r(), "w", &r_w) && pico8_value_equal(kw_qe, r_w);

        if (mi_fixed < pico8_fixed_from_int(4) && jy != NULL && jy->count > 0 && !qe_is_r_w) {
            pico8_list_t prefix;
            pico8_list_t *merged = malloc(sizeof *merged);
            pico8_list_init(&prefix);
            pico8_list_add(&prefix, pico8_value_string("wants more:"));
            if (field_truthy(self, "qh")) {
                pico8_list_t last_only;
                pico8_list_init(&last_only);
                pico8_list_add(&last_only, jy->items[jy->count - 1]);
                *merged = pico8_la(&prefix, &last_only);
                pico8_list_free(&last_only);
            } else {
                *merged = pico8_la(&prefix, jy);
            }
            pico8_list_free(&prefix);
            pico8_list_add(&bs, pico8_value_ref(pico8_jo_gh_row(merged)));
        }
    }
    pico8_list_free(&jx);

    /* add(bs,ji[2]and{lq=ji}): ji[2] is Lua nil (not false) when nothing
     * was ever appended -- add(t,nil) is a documented no-op. */
    if (ji->count >= 2) {
        pico8_list_add(&bs, pico8_value_ref(pico8_jo_gh_row(ji)));
    } else {
        pico8_list_free(ji);
        free(ji);
    }

    /* if self.kw.mu>0 then ... end */
    {
        const pico8_ob_table_t *kw = field_ref_table(self, "kw");
        pico8_fixed_t mu = pico8_fixed_from_int(0);
        pico8_value_t qe_val = pico8_value_ref(NULL);

        if (kw != NULL) {
            pico8_value_t mu_val;
            if (pico8_ob_table_get_name(kw, "mu", &mu_val) && mu_val.kind == PICO8_VALUE_FIXED) {
                mu = mu_val.as.fixed;
            }
            pico8_ob_table_get_name(kw, "qe", &qe_val);
        }

        if (mu > pico8_fixed_from_int(0)) {
            pico8_list_t *lb = malloc(sizeof *lb);
            int makes = 0;

            pico8_list_init(lb);

            if (g_jo_class != NULL) {
                pico8_value_t cq_val;
                if (pico8_ob_table_get_name(g_jo_class->defaults, "cq", &cq_val) && cq_val.kind == PICO8_VALUE_REF &&
                    cq_val.as.ref != NULL) {
                    pico8_list_t cq_list = pico8_jo_positional_values((const pico8_ob_table_t *)cq_val.as.ref);
                    makes = pico8_gu(&cq_list, qe_val);
                    pico8_list_free(&cq_list);
                }
            }

            if (makes) {
                char *mu_str = pico8_value_tostr(pico8_value_fixed(mu));
                pico8_list_add(lb, pico8_value_string("makes:"));
                pico8_list_add(lb, pico8_value_string(mu_str));
            } else {
                const pico8_list_t *by = field_list(self, "by");
                int by_count = (by != NULL) ? by->count : 0;
                char *count_str = pico8_value_tostr(pico8_value_fixed(pico8_fixed_from_int(by_count)));
                char *mu_str = pico8_value_tostr(pico8_value_fixed(mu));
                size_t count_len = strlen(count_str);
                size_t mu_len = strlen(mu_str);
                char *combined = malloc(count_len + 1 + mu_len + 1);
                memcpy(combined, count_str, count_len);
                combined[count_len] = '/';
                memcpy(combined + count_len + 1, mu_str, mu_len + 1);
                free(count_str);
                free(mu_str);
                pico8_list_add(lb, pico8_value_string("exports:"));
                pico8_list_add(lb, pico8_value_string(combined));
            }
            pico8_list_add(lb, pico8_value_ref((void *)kw));
            pico8_list_add(&bs, pico8_value_ref(pico8_jo_gh_row(lb)));
        }
    }

    /* for i=2,#bs do add(bs[i].lq,{false,1}) end */
    for (i = 1; i < bs.count; i++) {
        pico8_value_t row_val = bs.items[i];
        if (row_val.kind == PICO8_VALUE_REF && row_val.as.ref != NULL) {
            pico8_ob_table_t *row = (pico8_ob_table_t *)row_val.as.ref;
            pico8_value_t lq_val;
            if (pico8_ob_table_get_name(row, "lq", &lq_val) && lq_val.kind == PICO8_VALUE_REF &&
                lq_val.as.ref != NULL) {
                pico8_ob_table_t *tuple = pico8_ob_table_new();
                pico8_ob_table_set_index(tuple, 1, pico8_value_bool(0));
                pico8_ob_table_set_index(tuple, 2, pico8_value_fixed(pico8_fixed_from_int(1)));
                pico8_list_add((pico8_list_t *)lq_val.as.ref, pico8_value_ref(tuple));
            }
        }
    }

    pico8_pz(self, &bs);
    pico8_list_free(&bs);
}

pico8_value_t pico8_jo_gv(const pico8_instance_t *self, const pico8_instance_t *s) {
    pico8_value_t me_val;
    pico8_value_t qu_val;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }
    if (!pico8_instance_get_field(self, "me", &me_val) || me_val.kind != PICO8_VALUE_STRING ||
        strcmp(me_val.as.string, "ly") != 0) {
        return pico8_value_ref(NULL);
    }
    if (s != NULL && pico8_instance_get_field(s, "qu", &qu_val) && qu_val.kind == PICO8_VALUE_REF &&
        qu_val.as.ref == (const void *)self) {
        pico8_jo_gh(self);
        return pico8_value_ref(NULL);
    }
    return pico8_value_bool(1);
}

void pico8_jo_hu(pico8_instance_t *self, pico8_instance_t *hp_instance) {
    pico8_instance_t *working_self;
    pico8_fixed_t fp = pico8_fixed_from_int(2);
    pico8_value_t cw_val;
    pico8_value_t go_val;
    pico8_value_t rn_val;

    if (self == NULL) {
        return;
    }
    working_self = self;

    if (hp_instance != NULL && pico8_instance_get_field(hp_instance, "cw", &cw_val) && pico8_jo_truthy(cw_val)) {
        pico8_class_t *target_class = NULL;
        pico8_ob_table_t *overrides;
        pico8_value_t p_val;
        pico8_value_t ra_val;
        pico8_value_t id_val;

        pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(1));

        if (cw_val.kind == PICO8_VALUE_STRING) {
            target_class = pico8_class_lookup_named(cw_val.as.string);
        }

        overrides = pico8_ob_table_new();
        if (pico8_instance_get_field(self, "p", &p_val)) {
            pico8_ob_table_set_name(overrides, "p", p_val);
        }
        if (pico8_instance_get_field(self, "ra", &ra_val)) {
            pico8_ob_table_set_name(overrides, "ra", ra_val);
        }
        if (pico8_instance_get_field(self, "id", &id_val)) {
            pico8_ob_table_set_name(overrides, "id", id_val);
        }

        working_self = (target_class != NULL) ? pico8_class_instantiate(target_class, overrides) : NULL;
        pico8_ob_table_free(overrides);
    } else if (hp_instance != NULL && pico8_instance_get_field(hp_instance, "go", &go_val) &&
               pico8_jo_truthy(go_val)) {
        pico8_value_t nk_val = pico8_value_ref(NULL);
        int found = 0;

        if (go_val.kind == PICO8_VALUE_FIXED) {
            found = pico8_ob_table_get_index(pico8_df(), (int)pico8_fixed_to_double(go_val.as.fixed), &nk_val);
        } else if (go_val.kind == PICO8_VALUE_STRING) {
            found = pico8_ob_table_get_name(pico8_df(), go_val.as.string, &nk_val);
        }
        if (!found) {
            nk_val = pico8_value_ref(NULL);
        }
        pico8_ob_table_set_name(self->fields, "nk", nk_val);
    } else {
        pico8_value_t fp_val;
        pico8_ke_ks(self, pico8_value_string("ly"));
        pico8_jo_lz(self, hp_instance);
        pico8_ob_table_set_name(self->fields, "fx", pico8_value_bool(1));
        fp_val = pico8_ov(pico8_value_string("autoassemblers"), pico8_value_fixed(pico8_fixed_from_int(3)),
                           pico8_value_fixed(pico8_fixed_from_int(2)));
        if (fp_val.kind == PICO8_VALUE_FIXED) {
            fp = fp_val.as.fixed;
        }
    }

    pico8_hy_nx(pico8_fm(), fp);

    if (hp_instance != NULL && pico8_instance_get_field(hp_instance, "rn", &rn_val) && pico8_jo_truthy(rn_val)) {
        pico8_rn(rn_val);
    } else {
        pico8_rn(pico8_value_fixed(pico8_fixed_from_int(55)));
    }

    pico8_jo_kd(working_self);
}

void pico8_jo_hh(pico8_instance_t *self) {
    pico8_value_t nk_val = pico8_value_ref(NULL);
    pico8_value_t d_val = pico8_value_ref(NULL);
    pico8_value_t me_val;
    pico8_value_t ks_arg;

    if (self == NULL) {
        return;
    }

    if (g_jo_ir != NULL) {
        pico8_fixed_t roll = pico8_jf_next(g_jo_ir);
        int use_ll = 0;
        pico8_value_t ll_val = pico8_value_ref(NULL);

        if (roll < pico8_fixed_from_double(0.45)) {
            use_ll = pico8_ob_table_get_name(pico8_df(), "ll", &ll_val) && pico8_jo_truthy(ll_val);
        }
        if (use_ll) {
            nk_val = ll_val;
        } else if (g_jo_oc != NULL) {
            nk_val = pico8_gn_next(g_jo_oc);
        }
    }
    pico8_ob_table_set_name(self->fields, "nk", nk_val);

    if (nk_val.kind == PICO8_VALUE_REF && nk_val.as.ref != NULL) {
        pico8_ob_table_get_name((const pico8_ob_table_t *)nk_val.as.ref, "d", &d_val);
    }
    pico8_ob_table_set_name(self->fields, "ep", d_val);

    if (nk_val.kind == PICO8_VALUE_REF && nk_val.as.ref != NULL &&
        pico8_ob_table_get_name((const pico8_ob_table_t *)nk_val.as.ref, "me", &me_val) && pico8_jo_truthy(me_val)) {
        ks_arg = me_val;
    } else {
        ks_arg = pico8_value_string("known");
    }
    pico8_ke_ks(self, ks_arg);
    pico8_jo_kd(self);
}

void pico8_jo_hk(const pico8_instance_t *self) {
    pico8_value_t kr_val;

    if (self == NULL) {
        return;
    }
    if (!pico8_instance_get_field(self, "kr", &kr_val) || !pico8_jo_truthy(kr_val)) {
        return;
    }
    if (kr_val.kind == PICO8_VALUE_FIXED && kr_val.as.fixed == pico8_fixed_from_int(0)) {
        return;
    }
    if (!pico8_gd(self)) {
        return;
    }
    {
        pico8_ob_table_t *overrides = pico8_ob_table_new();
        pico8_value_t ra_val;
        int color = 0;
        char *text;

        if (pico8_instance_get_field(self, "ra", &ra_val)) {
            pico8_ob_table_set_name(overrides, "ra", ra_val);
        }
        text = pico8_ne(kr_val.kind == PICO8_VALUE_FIXED ? kr_val.as.fixed : pico8_fixed_from_int(0), 10, "$",
                        &color);
        pico8_ob_table_set_index(overrides, 1, pico8_value_string(text));
        pico8_ob_table_set_index(overrides, 2, pico8_value_fixed(pico8_fixed_from_int(color)));
        pico8_iv_new(overrides);
        pico8_ob_table_free(overrides);
        /* `text` intentionally NOT freed here: pico8_iv_new's own shallow
         * copy stores this SAME pointer (pico8_value_string never
         * duplicates) into the new `iv` instance's own fields, read back
         * every frame it stays on screen (pico8_iv_kj) -- freeing it here
         * would be the exact dangling-pointer bug this sub-lot's own
         * research found already shipped at a DIFFERENT call site
         * (pico8_hy_lu, pico8_economy.c -- see docs/portage-progress.md's
         * own note on this chunk for the full trace, and docs/plan.md's
         * "Notes de portage" for the fix: pico8_hy_lu now matches this
         * same non-freeing pattern). */
    }
}
