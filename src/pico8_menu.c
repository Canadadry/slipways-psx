#include "pico8_menu.h"

#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_class.h"
#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_dispatch_context.h"
#include "pico8_economy.h"
#include "pico8_fixed.h"
#include "pico8_input.h"
#include "pico8_layout.h"
#include "pico8_list.h"
#include "pico8_msgbox.h"
#include "pico8_pi.h"
#include "pico8_probe.h"
#include "pico8_score_panel.h"
#include "pico8_score_panel_display.h"
#include "pico8_tech_gate.h"
#include "pico8_ui_button.h"
#include "pico8_value.h"
#include "pico8_ref_audit.h"

static pico8_ob_table_t *g_ix = NULL;
static pico8_ob_table_t *g_ha = NULL;

static int cr_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static char *cr_concat2(const char *a, const char *b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char *out = malloc(la + lb + 1);
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = '\0';
    return out;
}

static pico8_ob_table_t *pj_cr_table(void) {
    pico8_class_t *cls = pico8_pj_class();
    pico8_value_t v;
    if (cls != NULL && cls->defaults != NULL && pico8_ob_table_get_name(cls->defaults, "cr", &v) &&
        v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

/* `my=t.e_ and function()fm:kt(t)end` (one `cr()` row's own "my" field,
 * pico8/lua.lua:957). See pico8_menu.h's own top comment on pico8_cr()
 * for the full trace of the jo:fe-style closure technique this bridge
 * implements: the captured upvalue `t` was stashed as this row's own
 * extra `"_cr_t"` field by pico8_cr_step below, and is read back here via
 * `self` (the row, by then a real instance) at dispatch time. */
static pico8_value_t pico8_cr_row_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *inst = (pico8_instance_t *)self;
    pico8_value_t t_val;
    (void)argc;
    (void)arg0;
    (void)arg1;
    if (inst == NULL) {
        return pico8_value_ref(NULL);
    }
    if (pico8_instance_get_field(inst, "_cr_t", &t_val) && t_val.kind == PICO8_VALUE_REF && t_val.as.ref != NULL) {
        pico8_hy_kt(pico8_fm(), (const pico8_ob_table_t *)t_val.as.ref);
    }
    return pico8_value_ref(NULL);
}

/* `function(t)return {lq={t.nk,t.px,c=6},h_={{24,5},tostr(gi(t))},
 * jb=t.oy,my=t.e_ and function()fm:kt(t)end}end` -- the closure
 * `nv(cb(true),...)` maps over every `nb` entry `t` (pico8/lua.lua:955-
 * 958). See pico8_menu.h's own top comment on pico8_cr() for the full
 * trace, including the evidenced correction on `t.e_`'s falsy branch
 * (a real PICO8_VALUE_BOOL(0), not nil -- `cb()`'s own real body always
 * sets `t.e_` to a genuine boolean before this closure ever sees `t`). */
static int pico8_cr_step(pico8_value_t e, void *ctx, pico8_value_t *out_value) {
    pico8_ob_table_t *t;
    pico8_ob_table_t *row;
    pico8_ob_table_t *lq_table;
    pico8_ob_table_t *h_table;
    pico8_ob_table_t *pair_table;
    pico8_value_t nk_val = pico8_value_ref(NULL);
    pico8_value_t px_val = pico8_value_ref(NULL);
    pico8_value_t oy_val;
    pico8_value_t e_val;
    pico8_fixed_t gi_result;
    char *gi_str;
    (void)ctx;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    t = (pico8_ob_table_t *)e.as.ref;

    row = pico8_ob_table_new();

    pico8_ob_table_get_name(t, "nk", &nk_val);
    pico8_ob_table_get_name(t, "px", &px_val);
    lq_table = pico8_ob_table_new();
    pico8_ob_table_set_index(lq_table, 1, nk_val);
    pico8_ob_table_set_index(lq_table, 2, px_val);
    pico8_ob_table_set_name(lq_table, "c", pico8_value_fixed(pico8_fixed_from_int(6)));
    pico8_ob_table_set_name(row, "lq", pico8_value_ref(lq_table));

    pair_table = pico8_ob_table_new();
    pico8_ob_table_set_index(pair_table, 1, pico8_value_fixed(pico8_fixed_from_int(24)));
    pico8_ob_table_set_index(pair_table, 2, pico8_value_fixed(pico8_fixed_from_int(5)));

    gi_result = pico8_gi(t);
    gi_str = pico8_value_tostr(pico8_value_fixed(gi_result));

    h_table = pico8_ob_table_new();
    pico8_ob_table_set_index(h_table, 1, pico8_value_ref(pair_table));
    pico8_ob_table_set_index(h_table, 2, pico8_value_string(gi_str));
    pico8_ob_table_set_name(row, "h_", pico8_value_ref(h_table));

    if (pico8_ob_table_get_name(t, "oy", &oy_val)) {
        pico8_ob_table_set_name(row, "jb", oy_val);
    }

    if (pico8_ob_table_get_name(t, "e_", &e_val) && cr_truthy(e_val)) {
        pico8_ob_table_set_name(row, "_cr_t", pico8_value_ref(t));
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_cr_row_my_dispatch));
    } else {
        pico8_ob_table_set_name(row, "my", pico8_value_bool(0));
    }

    *out_value = pico8_value_ref(row);
    return 1;
}

/* Was a private per-module copy of the "pico8_list_t -> fresh positional
 * pico8_ob_table_t" bridge; now a thin alias for the one SHARED converter,
 * `pico8_ob_table_from_list()` (pico8_ob.h), which this copy was promoted
 * into once `js:pq`'s own "lq" field turned out to need exactly the same
 * conversion (pico8_ui_button.c's own pico8_js_lq_arg_table). Kept as a
 * named local wrapper only so this call site's own trace below still reads
 * as one statement. Needed here because `cr()`'s own
 * "extra" row below sets its `"jb"` field straight from `fm.invented`,
 * a genuine `pico8_list_t*` (pico8_economy.c's own `pico8_hy_pq`), NOT a
 * `pico8_ob_table_t*` like every other row's own "jb" field
 * (`pico8_hp_jb()`'s return) -- real Lua's `jb(self.jb)` doesn't care
 * (`pairs()` walks a list-shaped table exactly like any other), but this
 * port's `pico8_ob_table_t`/`pico8_list_t` are structurally DISTINCT
 * ({entries;count;capacity} of `pico8_ob_entry_t` vs {items;count;
 * capacity} of the smaller `pico8_value_t`), so passing the raw list REF
 * through unconverted -- as the code did before this fix -- reinterprets
 * the list's own backing array at the WRONG element stride the moment
 * `js:pq()`'s own `self.jb=self.jb and jb(self.jb)` step reads it back
 * (`pico8_jb_new`->`pico8_class_instantiate`), a real, ASan-confirmed
 * heap-buffer-overflow (same representation-ambiguity family as this
 * file's own sibling fix, `pico8_js_lq_arg_table`, pico8_ui_button.c --
 * found during this port's own Phase 5 pre-flight representation-cast
 * audit, docs/plan.md's own "Notes de portage" journal). Returns a new,
 * heap-allocated table
 * the caller owns; this call site never frees it (same accepted
 * "temporary override table, shallow-copied by the constructor chain
 * that eventually consumes it, then left for the rest of the program"
 * leak this port's row-building call sites already establish, e.g.
 * `pico8_hp_jb()`'s own table above). */
static pico8_ob_table_t *pico8_menu_list_to_ob_table(const pico8_list_t *list) {
    return pico8_ob_table_from_list(list);
}

/* function cr()local bs,rd,gr={},{},#fm.invented
 * local tl=fm.ew
 * local qi=ek[tl-1]or 0
 * local bs=nv(cb(true),function(t)...end)
 * if gr>0 then
 * add(bs,{lq={30,gr.." invented",c=3},jb=fm.invented})
 * end
 * pj.cr.ep="level "..tl.." ("..(gr-qi).."/"..(ek[tl]-qi)..")"
 * pz(pj.cr,bs)end
 * (pico8/lua.lua:952-964.) See pico8_menu.h's own top comment on this
 * declaration for the full trace, including the `ek[tl-1]or 0` edge case
 * and the `pico8_pz`-vs-`pj.cr`-representation wrapper. */
void pico8_cr(void) {
    pico8_instance_t *fm = pico8_fm();
    pico8_fixed_t tl = pico8_fixed_from_int(0);
    int gr = 0;
    pico8_fixed_t qi = pico8_fixed_from_int(0);
    pico8_value_t tl_val;
    pico8_value_t invented_val;
    const pico8_ob_table_t *ek;
    pico8_list_t cb_list;
    pico8_list_t bs;
    pico8_ob_table_t *cr_table;

    if (fm != NULL && pico8_instance_get_field(fm, "ew", &tl_val) && tl_val.kind == PICO8_VALUE_FIXED) {
        tl = tl_val.as.fixed;
    }
    if (fm != NULL && pico8_instance_get_field(fm, "invented", &invented_val) &&
        invented_val.kind == PICO8_VALUE_REF && invented_val.as.ref != NULL) {
        /* REVERSE-CAST AUDIT (pico8_ref_audit.h): SAFE BY CONSTRUCTION.
         * Producer: pico8_hy_pq() (src/pico8_economy.c), sole writer of
         * "invented", which replaces the spec string's own
         * `invented=o()` ob_table with a real heap list. */
        gr = PICO8_REF_AS_LIST_CONST(invented_val.as.ref)->count;
    }

    ek = pico8_ek();
    if (ek != NULL) {
        pico8_value_t qi_val;
        int idx = (int)pico8_fixed_to_double(tl) - 1;
        if (pico8_ob_table_get_index(ek, idx, &qi_val) && qi_val.kind == PICO8_VALUE_FIXED) {
            qi = qi_val.as.fixed;
        }
    }

    cb_list = pico8_cb(pico8_value_bool(1));
    bs = pico8_nv(&cb_list, pico8_cr_step, NULL);
    pico8_list_free(&cb_list);

    if (gr > 0) {
        pico8_ob_table_t *extra = pico8_ob_table_new();
        pico8_ob_table_t *lq2 = pico8_ob_table_new();
        char *gr_str = pico8_value_tostr(pico8_value_fixed(pico8_fixed_from_int(gr)));
        char *label = cr_concat2(gr_str, " invented");
        free(gr_str);
        pico8_ob_table_set_index(lq2, 1, pico8_value_fixed(pico8_fixed_from_int(30)));
        pico8_ob_table_set_index(lq2, 2, pico8_value_string(label));
        pico8_ob_table_set_name(lq2, "c", pico8_value_fixed(pico8_fixed_from_int(3)));
        pico8_ob_table_set_name(extra, "lq", pico8_value_ref(lq2));
        /* `jb=fm.invented` -- see pico8_menu_list_to_ob_table's own top
         * comment above for why `invented_val` (a genuine `pico8_list_t*`)
         * is bridged into a fresh positional `pico8_ob_table_t*` here
         * rather than stored as-is. */
        {
            const pico8_list_t *invented_list =
                /* REVERSE-CAST AUDIT: SAFE BY CONSTRUCTION, same
                 * pico8_hy_pq() producer as the `gr` read above. */
                (invented_val.kind == PICO8_VALUE_REF) ? PICO8_REF_AS_LIST_CONST(invented_val.as.ref) : NULL;
            pico8_ob_table_set_name(extra, "jb", pico8_value_ref(pico8_menu_list_to_ob_table(invented_list)));
        }
        pico8_list_add(&bs, pico8_value_ref(extra));
    }

    cr_table = pj_cr_table();
    if (cr_table != NULL) {
        pico8_fixed_t ek_tl = pico8_fixed_from_int(0);
        pico8_value_t ek_tl_val;
        char *tl_str;
        char *diff1_str;
        char *diff2_str;
        char *tmp1;
        char *tmp2;
        char *tmp3;
        char *ep;
        pico8_instance_t wrapper;

        if (ek != NULL && pico8_ob_table_get_index(ek, (int)pico8_fixed_to_double(tl), &ek_tl_val) &&
            ek_tl_val.kind == PICO8_VALUE_FIXED) {
            ek_tl = ek_tl_val.as.fixed;
        }

        tl_str = pico8_value_tostr(pico8_value_fixed(tl));
        diff1_str = pico8_value_tostr(pico8_value_fixed(pico8_fixed_sub(pico8_fixed_from_int(gr), qi)));
        diff2_str = pico8_value_tostr(pico8_value_fixed(pico8_fixed_sub(ek_tl, qi)));

        tmp1 = cr_concat2("level ", tl_str);
        tmp2 = cr_concat2(tmp1, " (");
        free(tmp1);
        tmp3 = cr_concat2(tmp2, diff1_str);
        free(tmp2);
        tmp1 = cr_concat2(tmp3, "/");
        free(tmp3);
        tmp2 = cr_concat2(tmp1, diff2_str);
        free(tmp1);
        ep = cr_concat2(tmp2, ")");
        free(tmp2);

        free(tl_str);
        free(diff1_str);
        free(diff2_str);

        pico8_ob_table_set_name(cr_table, "ep", pico8_value_string(ep));

        wrapper.tag = PICO8_TYPE_TAG_INSTANCE;
        wrapper.fields = cr_table;
        wrapper.cls = NULL;
        pico8_pz(&wrapper, &bs);
    }

    pico8_list_free(&bs);
}

static pico8_ob_table_t *pico8_menu_index_table(pico8_ob_table_t *table, int idx) {
    pico8_value_t v;
    if (table != NULL && pico8_ob_table_get_index(table, idx, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

static pico8_list_t pico8_menu_positional_values(const pico8_ob_table_t *table) {
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

/* `ix[1].my=function()fm.lo=1 fm:hk()end` (pico8/lua.lua:979-980). See
 * pico8_menu.h's own top comment on pico8_ix() for the full trace. */
static pico8_value_t pico8_ix1_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *fm = pico8_fm();
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    if (fm != NULL) {
        pico8_ob_table_set_name(fm->fields, "lo", pico8_value_fixed(pico8_fixed_from_int(1)));
    }
    pico8_hy_hk(fm);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_ix2_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_cr();
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_ix2_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *inst = (pico8_instance_t *)self;
    pico8_list_t cb_list;
    int cb_count;
    pico8_ob_table_t *overrides;
    pico8_instance_t *built;
    (void)argc;
    (void)arg0;
    (void)arg1;

    if (inst == NULL) {
        return pico8_value_ref(NULL);
    }

    cb_list = pico8_cb(pico8_value_ref(NULL));
    cb_count = cb_list.count;
    pico8_list_free(&cb_list);

    overrides = pico8_ob_table_new();
    pico8_ob_table_set_index(overrides, 1, pico8_value_fixed(pico8_fixed_from_int(cb_count > 0 ? 24 : 37)));
    built = pico8_lq_new(overrides);
    pico8_ob_table_free(overrides);

    pico8_ob_table_set_name(inst->fields, "lq", pico8_value_ref(built));

    return pico8_value_ref(NULL);
}

/* `ix[3].my=function()ju.c_=true fq(de)end` (pico8/lua.lua:984-985). See
 * pico8_menu.h's own top comment on pico8_ix() for the full trace,
 * including the `pico8_de()` accessor this reuses from `pj:ph()`. */
static pico8_value_t pico8_ix3_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *ju = pico8_ju();
    pico8_instance_t *built;
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    if (ju != NULL) {
        pico8_ob_table_set_name(ju->fields, "c_", pico8_value_bool(1));
    }
    built = pico8_fq_new(pico8_de());
    (void)built;
    return pico8_value_ref(NULL);
}

/* `function()music(-sgn(stat(20)))end` (`ha[2].my`, "toggle music" --
 * one of `ix[4].my`'s own 4 wired callbacks, pico8/lua.lua:993). See
 * pico8_menu.h's own top comment on pico8_ix()/pico8_ha() for the full
 * trace. */
static pico8_value_t pico8_ha2_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    int stat_val;
    pico8_fixed_t s;
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    stat_val = pico8_stat(pico8_dispatch_input(), 20);
    s = pico8_fixed_sgn(pico8_fixed_from_int(stat_val));
    pico8_music(-(int)pico8_fixed_to_double(s));
    return pico8_value_ref(NULL);
}

/* `function()ih=not ih end` ("toggle sfx", ha[3].my). CORRECTION (found
 * during this chunk's own verification): a prior version of this file
 * toggled a private, disconnected module-local static instead of the
 * real `ih` global -- Chunk 1 already built exactly this forward-
 * reference singleton for `ih` (`pico8_ih()`/`pico8_set_ih()`,
 * src/pico8_audio.h), purpose-built for whichever chunk wires the real
 * "toggle sfx" button (this one), and already read by `pico8_rn()`'s own
 * `if (no and not ih)sfx(no,3)end` (pico8/lua.lua:331, already ported).
 * Toggling a second, never-synchronized `ih` here would silently break
 * that mute check. Fixed to read/write the real singleton. */
static pico8_value_t pico8_ha3_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_set_ih(!pico8_ih());
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_ha4_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(NULL);
}

/* `ix[4].my=function()for i,fn in pairs({_init,function()music(-sgn(
 * stat(20)))end,function()ih=not ih end,function()end})do ha[i].my=fn end
 * pz(ha,ha)end` (pico8/lua.lua:993-996). See pico8_menu.h's own top
 * comment on pico8_ix()/pico8_ha() for the full trace, including the
 * `pz(ha,ha)` self-referential wrapper. */
static pico8_value_t pico8_ix4_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_ob_table_t *row;
    pico8_list_t bs;
    pico8_instance_t wrapper;
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;

    row = pico8_menu_index_table(g_ha, 1);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_init_field_value());
    }
    row = pico8_menu_index_table(g_ha, 2);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ha2_my_dispatch));
    }
    row = pico8_menu_index_table(g_ha, 3);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ha3_my_dispatch));
    }
    row = pico8_menu_index_table(g_ha, 4);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ha4_my_dispatch));
    }

    if (g_ha != NULL) {
        bs = pico8_menu_positional_values(g_ha);
        wrapper.tag = PICO8_TYPE_TAG_INSTANCE;
        wrapper.fields = g_ha;
        wrapper.cls = NULL;
        pico8_pz(&wrapper, &bs);
        pico8_list_free(&bs);
    }

    return pico8_value_ref(NULL);
}

const pico8_ob_table_t *pico8_ix(void) {
    return g_ix;
}

const pico8_ob_table_t *pico8_ha(void) {
    return g_ha;
}

void pico8_menu_init(void) {
    pico8_ob_table_t *row;

    pico8_menu_shutdown();

    pico8_pi_init();

    g_ix = pico8_ob("\n"
                     "o(\n"
                     "ra=v(118,0),\n"
                     "lq=o(o(15,7)),\n"
                     "),o(\n"
                     "ra=v(0,0),\n"
                     "lq=o(o(37,7)),\n"
                     "),o(\n"
                     "ra=v(47,0),\n"
                     "lq=o(o(25,7)),\n"
                     "),o(\n"
                     "ra=v(0,117),\n"
                     "lq=o(o(8,7)),\n"
                     "),\n",
                     NULL);

    g_ha = pico8_ob("\n"
                     "o(lq=o(31,\"restart     \")),\n"
                     "o(lq=o(42,\"toggle music\")),\n"
                     "o(lq=o(40,\"toggle sfx  \")),\n"
                     "o(lq=o(26,\"how to play?\"),fg=5,),\n"
                     "hi=v(0,107),gq=v(0,-11),\n"
                     "ca=14,\n",
                     NULL);

    row = pico8_menu_index_table(g_ix, 1);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ix1_my_dispatch));
    }
    row = pico8_menu_index_table(g_ix, 2);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ix2_my_dispatch));
        pico8_ob_table_set_name(row, "hb", pico8_value_function(pico8_ix2_hb_dispatch));
    }
    row = pico8_menu_index_table(g_ix, 3);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ix3_my_dispatch));
    }
    row = pico8_menu_index_table(g_ix, 4);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ix4_my_dispatch));
    }
}

void pico8_menu_shutdown(void) {
    if (g_ix != NULL) {
        pico8_ob_table_free(g_ix);
        g_ix = NULL;
    }
    if (g_ha != NULL) {
        pico8_ob_table_free(g_ha);
        g_ha = NULL;
    }
    /* `ih` is NOT reset here: it's the real, persistent Lua global
     * (pico8_ih()/pico8_set_ih(), src/pico8_audio.h, owned by Chunk 1's
     * audio module) -- rebuilding/tearing down this module's own ix/ha
     * button tables has no real-Lua-equivalent effect on it. */
}
