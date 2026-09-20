#include "pico8_planet_display.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_economy.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_hud.h"
#include "pico8_msgbox.h"
#include "pico8_planet.h"
#include "pico8_probe.h"
#include "pico8_research.h"
#include "pico8_research_display.h"
#include "pico8_stash.h"

/* Phase 3, Chunk 13, sub-lot 3's own pure module split (see
 * pico8_planet_display.h's own top comment for the full trace): `gh`,
 * `gv`, `hb` moved here verbatim from pico8_planet.c, along with the
 * private helpers used only by them. `pico8_jo_truthy` is intentionally
 * duplicated here (a plain copy, not shared via a header) rather than
 * exposed from pico8_planet.c, matching this port's own established
 * "small per-module private helper, documented duplication accepted"
 * convention already applied to this exact helper's own sibling copies
 * (pico8_probe_truthy/pico8_research_truthy/pico8_tech_gate_truthy/etc,
 * see pico8_planet.c's own comment on pico8_jo_truthy) -- both this copy
 * and pico8_planet.c's copy must stay behaviorally identical, since both
 * are ports of the exact same Lua truthiness rule. */
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
 * "small per-module private helper" convention. Used only by `jo:gh`
 * below (moved here alongside it, unlike `pico8_jo_truthy` above which
 * pico8_planet.c's own remaining methods still need too). */
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
            pico8_jo_class() != NULL) {
            pico8_value_t dz_val;
            if (pico8_ob_table_get_name(pico8_jo_class()->defaults, "dz", &dz_val) && dz_val.kind == PICO8_VALUE_REF &&
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

            if (pico8_jo_class() != NULL) {
                pico8_value_t cq_val;
                if (pico8_ob_table_get_name(pico8_jo_class()->defaults, "cq", &cq_val) &&
                    cq_val.kind == PICO8_VALUE_REF && cq_val.as.ref != NULL) {
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

/* Per-item context for pico8_jo_fe_step below: `self` (the planet)
 * threaded through pico8_nv's own `ctx` parameter, matching this port's
 * established "closure -> context struct" convention (see e.g.
 * pico8_planet.c's own pico8_jo_lz_mw_step, or this same file's own
 * pico8_jo_gh_jp_step above) for the parts of the real closure's own
 * captured upvalues that a SYNCHRONOUS map step can just read straight
 * off a C local -- unlike `od`/`b` themselves, which the "my" closure
 * needs to survive PAST this map (see pico8_jo_fe_my_dispatch below for
 * why those two are threaded a different way). */
typedef struct {
    pico8_instance_t *self;
} pico8_jo_fe_ctx_t;

/* `my=function()if fm:lu(od)then self:hu(b)end end`: the one real Lua
 * closure inside `jo:fe`'s own mapper that is NOT resolved synchronously
 * (unlike `lq`/`h_`/`jb`, computed once per item right here) -- it is
 * stored as a genuine PICO8_VALUE_FUNCTION field value on the built row,
 * to be called LATER, generically, by `mv(rowEntity,"my")` (pico8_value.h's
 * own comment on pico8_value_function_t already lists this EXACT real call
 * shape: `mv(self,"my")`, js:ldown/fq:ldown, 0 extra arguments) -- once the
 * row becomes a real `js` button instance via `pz()`'s own `js(b)`
 * construction (Chunk 8, already built: `pico8_class_instantiate` copies
 * EVERY field of the row table verbatim onto the new instance, per its own
 * documented "mw(ob)=qj({},ob)" contract).
 *
 * C has no closures (this port's own pervasive limitation, already
 * documented at pico8_layout.c's own top comment) -- and unlike every
 * closure this port has represented so far (a `ctx` pointer threaded
 * through a SYNCHRONOUS pico8_jp/pico8_nv/pico8_pz_step call, resolved
 * before that call returns), THIS closure must still be callable an
 * arbitrary time LATER, through the generic, single-C-function-pointer
 * `pico8_value_function_t` shape (no userdata/context slot of its own).
 * Resolved here by exploiting the exact same fact `pico8_pz_step` already
 * relies on: whichever entity `pico8_mv()` is eventually called ON (the
 * real Lua `self` a colon-call would receive) IS the row/button instance
 * itself -- so its own real captured upvalues (`self` here meaning the
 * PLANET, `b` the `hp` instance, `od` the fixed value) are stashed as
 * three EXTRA fields on the very same row `pico8_ob_table_t` being built
 * (`"_jo_fe_pt"`/`"_jo_fe_b"`/`"_jo_fe_od"` -- leading underscore, `jo_fe`
 * prefix: clearly NOT real Lua field names, pure C-port plumbing with no
 * Lua-source equivalent, the same kind of new, clearly-named infrastructure
 * this port's Out of Scope section already carves out an exception for,
 * e.g. `pico8_pi_dispatch_register`). Since `pico8_class_instantiate` (and
 * therefore `js(b)`/`pz()`) copies EVERY field of `overrides` verbatim --
 * confirmed by reading `pico8_class_instantiate`'s own real body,
 * pico8_class.c -- these three extra fields ride along onto the final `js`
 * instance untouched, and this bridge reads them straight back off its own
 * `self` parameter (which, at call time, WILL be that same instance) via
 * the ordinary `pico8_instance_get_field` fallback-chain lookup. This is a
 * NEW representational technique this sub-lot introduces (the first real
 * Lua closure in scope whose own upvalues must survive past a synchronous
 * map call) -- not a rewrite of any existing convention, and it changes no
 * OBSERVABLE behavior: the extra fields are never read by anything else
 * (`pz()`'s own real source only ever writes `ra`/`qf`/`fa`, never
 * enumerates a row's fields), so this is invisible to every other real
 * call site. Ignores its own dispatched `self`/`argc`/`arg0`/`arg1` for
 * the SAME reason `jo:hb`/`jo:hk`'s own bridges do: the real Lua closure
 * here (`function()...end`) declares no parameters of its own, so real
 * Lua's own "extra call arguments are simply discarded" semantics apply
 * regardless of what mv() actually passes. */
static pico8_value_t pico8_jo_fe_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    const pico8_instance_t *inst = (const pico8_instance_t *)self;
    pico8_value_t pt_val;
    pico8_value_t b_val;
    pico8_value_t od_val;
    pico8_instance_t *pt = NULL;
    pico8_instance_t *b = NULL;
    pico8_fixed_t od = pico8_fixed_from_int(0);

    (void)argc;
    (void)arg0;
    (void)arg1;

    if (inst == NULL) {
        return pico8_value_ref(NULL);
    }
    if (pico8_instance_get_field(inst, "_jo_fe_pt", &pt_val) && pt_val.kind == PICO8_VALUE_REF &&
        pt_val.as.ref != NULL) {
        pt = (pico8_instance_t *)pt_val.as.ref;
    }
    if (pico8_instance_get_field(inst, "_jo_fe_b", &b_val) && b_val.kind == PICO8_VALUE_REF && b_val.as.ref != NULL) {
        b = (pico8_instance_t *)b_val.as.ref;
    }
    if (pico8_instance_get_field(inst, "_jo_fe_od", &od_val) && od_val.kind == PICO8_VALUE_FIXED) {
        od = od_val.as.fixed;
    }

    if (pico8_hy_lu(pico8_fm(), od)) {
        pico8_jo_hu(pt, b);
    }
    return pico8_value_ref(NULL);
}

/* function(b)local od=b:od()
 * return {lq=mv(b,"lq"),h_=od>0 and {od.."$",c=13},my=function()if fm:lu(od)then
 * self:hu(b)end
 * end,jb=b:jb()}
 * end
 * (pico8/lua.lua:418-422, jo:fe's own inline `nv()` mapper.) Builds one
 * row per research entry `b` (an `hp` instance, per `bj()`'s own
 * documented return shape, pico8_research_display.h). `od=b:od()`: a
 * direct, known same-class call (`pico8_hp_od`, pico8_research.h). `lq=
 * mv(b,"lq")`: a genuine `mv()` dispatch (see pico8_research.h's own
 * "CORRECTION" note on why `hp`'s class now wires "lq" for exactly this
 * call site) -- `pico8_mv` returning 0 (field genuinely absent, never
 * evidenced as a real case once the fix lands) leaves the row's own "lq"
 * key UNSET, matching Lua's own `{lq=nil}` never creating that key. `h_=
 * od>0 and {od.."$",c=13}`: `od>0` is a genuine Lua COMPARISON, always a
 * real boolean (unlike `jo:gh`'s own `self.kr and {...}`, where the
 * left-hand side is a plain field read) -- so the falsy branch is the
 * literal boolean `false`, not `od` itself; the truthy branch's `od..
 * "$"` uses `pico8_value_tostr` (this port's own established "concatenate
 * a fixed value with a string suffix" convention, matching `jo:gh`'s own
 * identical `#self.by.."/"..self.kw.mu` treatment). `jb=b:jb()`: another
 * direct, known same-class call (`pico8_hp_jb`, pico8_research_display.h).
 * `my=...`: see pico8_jo_fe_my_dispatch above for the full closure-
 * representation trace. Always contributes a row (never a falsy/nil `nv()`
 * skip -- the real Lua table constructor here is always truthy), so this
 * always returns 1 for a well-formed element. */
static int pico8_jo_fe_step(pico8_value_t e, void *ctx_ptr, pico8_value_t *out_value) {
    const pico8_jo_fe_ctx_t *ctx = (const pico8_jo_fe_ctx_t *)ctx_ptr;
    pico8_instance_t *b;
    pico8_fixed_t od;
    pico8_ob_table_t *row;
    pico8_value_t lq_val;
    pico8_value_t h_val;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    b = (pico8_instance_t *)e.as.ref;
    od = pico8_hp_od(b);

    row = pico8_ob_table_new();

    if (pico8_mv(b, "lq", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &lq_val)) {
        pico8_ob_table_set_name(row, "lq", lq_val);
    }

    if (od > pico8_fixed_from_int(0)) {
        pico8_ob_table_t *h_tuple = pico8_ob_table_new();
        char *od_str = pico8_value_tostr(pico8_value_fixed(od));
        size_t od_len = strlen(od_str);
        char *with_suffix = malloc(od_len + 2);
        memcpy(with_suffix, od_str, od_len);
        with_suffix[od_len] = '$';
        with_suffix[od_len + 1] = '\0';
        free(od_str);
        pico8_ob_table_set_index(h_tuple, 1, pico8_value_string(with_suffix));
        pico8_ob_table_set_name(h_tuple, "c", pico8_value_fixed(pico8_fixed_from_int(13)));
        h_val = pico8_value_ref(h_tuple);
    } else {
        h_val = pico8_value_bool(0);
    }
    pico8_ob_table_set_name(row, "h_", h_val);

    pico8_ob_table_set_name(row, "_jo_fe_pt", pico8_value_ref((void *)ctx->self));
    pico8_ob_table_set_name(row, "_jo_fe_b", pico8_value_ref(b));
    pico8_ob_table_set_name(row, "_jo_fe_od", pico8_value_fixed(od));
    pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_jo_fe_my_dispatch));

    pico8_ob_table_set_name(row, "jb", pico8_value_ref(pico8_hp_jb(b)));

    *out_value = pico8_value_ref(row);
    return 1;
}

void pico8_jo_fe(pico8_instance_t *self) {
    pico8_list_t bj_list;
    pico8_list_t rows;
    pico8_jo_fe_ctx_t ctx;

    if (self == NULL) {
        return;
    }

    bj_list = pico8_bj(self);
    ctx.self = self;
    rows = pico8_nv(&bj_list, pico8_jo_fe_step, &ctx);
    pico8_list_free(&bj_list);

    pico8_pz(self, &rows);
    pico8_list_free(&rows);
}

void pico8_jo_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p) {
    pico8_vec_t adjusted;
    pico8_value_t me_val;
    int px;
    int py;
    int spr_index = 43;
    pico8_value_t nk_val;
    pico8_value_t s_val;

    if (self == NULL || !pico8_gd(self)) {
        return;
    }

    /* p=ma(p,self): re-derives the screen position AND decays self.ma
     * (pico8_ma's own documented side effect) -- `adjusted` is used by
     * every remaining line below, never the raw parameter `p`. */
    adjusted = pico8_ma(p, self);

    /* if self.me=="ll" and rnd(30)<self.t-30 then return end -- NOT dead
     * code: jo:hh() (pico8_planet.c's own pico8_jo_hh, ported from
     * pico8/lua.lua:407-411) does `self.nk=jo.ir()<0.45 and df.ll or
     * jo.oc()` then `self:ks(self.nk.me or "known")`, and df.ll
     * (pico8/lua.lua:1378, `o("ll",45,0,me="ll")`) has `me="ll"` -- so
     * roughly 45% of real jo:hh() calls DO set self.me="ll" for real, via
     * ke:ks() (pico8/lua.lua:185, `self.me,self.t=me,0`, the base-class
     * method jo inherits). Genuinely REACHABLE once Chunk 20 wires
     * jo.ir/jo.oc to a real generator (g_jo_ir/g_jo_oc are still NULL
     * today, pico8_planet.c) -- simply not yet EXERCISED by any
     * already-ported code path. See this file's own header comment on
     * pico8_jo_kj for the full trace. */
    if (pico8_instance_get_field(self, "me", &me_val) && me_val.kind == PICO8_VALUE_STRING &&
        strcmp(me_val.as.string, "ll") == 0) {
        pico8_fixed_t t = field_fixed(self, "t");
        if (pico8_fixed_rnd(pico8_fixed_from_int(30)) < pico8_fixed_sub(t, pico8_fixed_from_int(30))) {
            return;
        }
    }

    px = (int)pico8_fixed_to_double(adjusted.x);
    py = (int)pico8_fixed_to_double(adjusted.y);

    /* circfill(p.x,p.y,7,0) */
    pico8_circfill(gfx, mem, px, py, 7, 0);

    /* dv(self.qw): self.qw is never set on jo ITSELF, but Chunk 15's `nl`
     * (a real jo-family subclass, jo->qh->nl, not yet ported) DOES set it
     * for real -- as a class-level default (nl=qh:jr([[...od=20,qw=15,...
     * ]]), pico8/lua.lua:1057-1066) and again as a per-instance override
     * (nl:ly(t)'s own self.qw=8+t%42/6 when self.mi>=2). Until Chunk 15
     * lands, no real jo-family instance in scope ever has qw set, so
     * absent reads as 0 (this file's own field_fixed) remains the
     * correct, evidenced behavior today -- same observable effect as
     * pico8_dv_default. See this file's own header comment on pico8_jo_kj
     * for the full trace. */
    pico8_dv(mem, field_fixed(self, "qw"));
    pico8_palt(gfx, 3, 0);
    pico8_palt(gfx, 0, 1);

    /* spr(self.nk and self.nk.s or self.s or 43,p.x-4,p.y-4,2,2) */
    if (pico8_instance_get_field(self, "nk", &nk_val) && pico8_jo_truthy(nk_val) && nk_val.kind == PICO8_VALUE_REF &&
        nk_val.as.ref != NULL &&
        pico8_ob_table_get_name((const pico8_ob_table_t *)nk_val.as.ref, "s", &s_val) &&
        s_val.kind == PICO8_VALUE_FIXED) {
        spr_index = (int)pico8_fixed_to_double(s_val.as.fixed);
    } else if (pico8_instance_get_field(self, "s", &s_val) && s_val.kind == PICO8_VALUE_FIXED) {
        spr_index = (int)pico8_fixed_to_double(s_val.as.fixed);
    }
    pico8_spr(gfx, mem, spr_index, px - 4, py - 4, 2, 2);
    pico8_dv_default(mem);

    /* if self:gg()then circ(p.x,p.y,6,jo.ka[self.mi])spr(self.mi+1,p.x-3,p.y-6)end */
    if (pico8_jo_truthy(pico8_jo_gg(self))) {
        int mi = (int)pico8_fixed_to_double(field_fixed(self, "mi"));
        int color = 0;
        pico8_value_t ka_val;
        pico8_value_t ka_entry;

        if (pico8_jo_class() != NULL && pico8_ob_table_get_name(pico8_jo_class()->defaults, "ka", &ka_val) &&
            ka_val.kind == PICO8_VALUE_REF && ka_val.as.ref != NULL &&
            pico8_ob_table_get_index((const pico8_ob_table_t *)ka_val.as.ref, mi, &ka_entry) &&
            ka_entry.kind == PICO8_VALUE_FIXED) {
            color = (int)pico8_fixed_to_double(ka_entry.as.fixed);
        }
        pico8_circ(gfx, mem, px, py, 6, color);
        pico8_spr_default(gfx, mem, mi + 1, px - 3, py - 6);
    }

    /* mv(self.lq,"ol",p+v(0,7),0.5) -- the game's own mv() global
     * function, ported through the real pico8_mv() mechanism (not a
     * direct call). See this file's own header comment on pico8_jo_kj for
     * why lq's own "ol" field needed to be newly wired for this. */
    {
        pico8_value_t lq_val;
        if (pico8_instance_get_field(self, "lq", &lq_val) && lq_val.kind == PICO8_VALUE_REF &&
            lq_val.as.ref != NULL) {
            pico8_vec_t offset =
                pico8_vec_add(adjusted, pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(7)));
            pico8_value_t offset_val = pico8_value_ref(&offset);
            pico8_value_t a_val = pico8_value_fixed(pico8_fixed_from_double(0.5));
            pico8_value_t discard;
            pico8_mv((pico8_instance_t *)lq_val.as.ref, "ol", 2, offset_val, a_val, &discard);
        }
    }

    /* if self.fo then fillp(0b1010101010101010.1)circfill(p.x,p.y,
     * sqrt(self.fo.dw)*128,2)fillp()self.fo=nil end */
    {
        pico8_value_t fo_val;
        if (pico8_instance_get_field(self, "fo", &fo_val) && pico8_jo_truthy(fo_val) &&
            fo_val.kind == PICO8_VALUE_REF && fo_val.as.ref != NULL) {
            pico8_fixed_t dw = pico8_fixed_from_int(0);
            pico8_value_t dw_val;
            pico8_fixed_t radius_f;
            int radius;

            if (pico8_instance_get_field((const pico8_instance_t *)fo_val.as.ref, "dw", &dw_val) &&
                dw_val.kind == PICO8_VALUE_FIXED) {
                dw = dw_val.as.fixed;
            }
            radius_f = pico8_fixed_mul(pico8_fixed_sqrt(dw), pico8_fixed_from_int(128));
            radius = (int)pico8_fixed_to_double(radius_f);

            /* 0b1010101010101010.1 -- see this file's own header comment
             * on pico8_jo_kj for why the trailing .1 fractional flag is
             * simply dropped (pico8_fillp's own already-shipped semantics
             * already model it unconditionally). */
            pico8_fillp(gfx, 0xAAAAu);
            pico8_circfill(gfx, mem, px, py, radius, 2);
            pico8_fillp_reset(gfx);
            pico8_ob_table_set_name(self->fields, "fo", pico8_value_ref(NULL));
        }
    }
}
