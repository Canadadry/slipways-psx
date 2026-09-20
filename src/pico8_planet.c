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
#include "pico8_planet_display.h"
#include "pico8_probe.h"
#include "pico8_research.h"
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

/* field_truthy/field_fixed/field_list/field_ref_table and
 * pico8_jo_positional_values (both only ever used by `jo:gh`) moved to
 * pico8_planet_display.c alongside `gh` itself in this sub-lot's own
 * module split -- see that file's own comments for their unchanged
 * bodies. */

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

/* "ldown" is not one of `bm`'s 5 real dispatch names, but is still a real
 * mv()-routed field -- `ui:jz`'s own `self:jz(bt.."down",self.qa,je.ra)`
 * (bt="l" giving "ldown", pico8_ui_button.h's own comment on this exact
 * dynamic name, Chunk 5) reaches it by name with 2 extra arguments; real
 * `jo:ldown()` itself takes none, so both are simply discarded here, the
 * same "bridge ignores every argument" treatment `pico8_jo_hb_dispatch`/
 * `pico8_jo_hk_dispatch` already established. */
static pico8_value_t pico8_jo_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_jo_ldown((pico8_instance_t *)self);
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
    pico8_ob_table_set_name(g_jo_class->defaults, "ldown", pico8_value_function(pico8_jo_ldown_dispatch));
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

/* `hb`, `gh`, `gv` (sub-lot 2's own ports, plus their private helpers
 * `pico8_jo_gh_ctx_t`/`pico8_jo_gh_jp_step`/`pico8_jo_gh_row`) moved to
 * pico8_planet_display.c/.h in this sub-lot's own pure module split (see
 * pico8_planet_display.h's own top comment for the full trace, and this
 * file's own header top comment for the split's overall plan) -- no
 * logic changed, only file location. `pico8_jo_hb_dispatch`/
 * `pico8_jo_gv_dispatch` just below stay here, alongside the rest of
 * `pico8_jo_init()`'s own class-registration wiring, and now call
 * `pico8_jo_hb`/`pico8_jo_gv` across the file boundary via
 * pico8_planet_display.h (included above) exactly as they already called
 * them as plain sibling functions before the split. */

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

/* Sub-lot 3 (see this file's own header top comment): ldown. `fe` itself
 * lands in pico8_planet_display.c/.h instead -- see that header's own top
 * "STEP 2 REBALANCE" comment for why. */

pico8_value_t pico8_jo_ldown(pico8_instance_t *self) {
    pico8_value_t me_val;

    if (self == NULL || !pico8_instance_get_field(self, "me", &me_val) || me_val.kind != PICO8_VALUE_STRING) {
        return pico8_value_ref(NULL);
    }
    if (strcmp(me_val.as.string, "known") == 0) {
        pico8_jo_fe(self);
        return pico8_value_bool(1);
    }
    if (strcmp(me_val.as.string, "ly") == 0) {
        pico8_class_t *jc_class = pico8_class_lookup_named("jc");
        pico8_instance_t *made;
        pico8_ob_table_t *overrides;

        if (jc_class == NULL) {
            return pico8_value_ref(NULL);
        }
        overrides = pico8_ob_table_new();
        pico8_ob_table_set_name(overrides, "qu", pico8_value_ref(self));
        made = pico8_class_instantiate(jc_class, overrides);
        pico8_ob_table_free(overrides);
        return pico8_value_ref(made);
    }
    return pico8_value_ref(NULL);
}
