#include "pico8_score_panel_display.h"

#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_economy_score.h"
#include "pico8_entity.h"
#include "pico8_layout.h"
#include "pico8_list.h"
#include "pico8_msgbox.h"
#include "pico8_probe.h"
#include "pico8_score_panel.h"
#include "pico8_ui_button.h"

/* See pico8_score_panel_display.h's own top comment for the full
 * source/trace of `pj:ph()` and `de`. */

static pico8_ob_table_t *g_de = NULL;

/* This module's own private Lua-truthiness helper (nil/false falsy,
 * everything else -- including 0 -- truthy), matching `pico8_ui.c`'s own
 * `pico8_ui_truthy`/`pico8_economy_score.c`'s own `economy_truthy`
 * established per-module-copy convention -- widened past those two
 * precedents' own BOOL-only check the same way `pico8_score_panel.c`'s
 * own now-removed `pj_truthy` was (see that file's own NOTE on why it was
 * removed rather than left as dead code once its only caller, `pj:hk`,
 * moved here): `fm.hl`, `st.score`-lookups, and `pj:hk`'s own `pp` can
 * all genuinely come back as this port's own `pico8_value_ref(NULL)`
 * "absent" filler as well as a real BOOL false, and both must read as
 * falsy. */
static int pj_display_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* This module's own private `fm` field-reader helper -- see
 * pico8_score_panel.c's own identical `fm_field_value` for the full
 * rationale (another accepted per-module duplicate). */
static pico8_value_t fm_field_value(const pico8_instance_t *fm, const char *name) {
    pico8_value_t v;
    if (fm != NULL && pico8_instance_get_field(fm, name, &v)) {
        return v;
    }
    return pico8_value_ref(NULL);
}

/* This module's own private two-piece string concat helper -- see
 * pico8_score_panel.c's own identical `pj_concat2` for the full
 * rationale (another accepted per-module duplicate; used here to build
 * `(fm.hl or "current score").." ("..dk.px..")"` via repeated two-piece
 * concatenation). Returns a newly heap-allocated string the caller owns
 * and must free(). */
static char *pj_concat2(const char *a, const char *b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char *out = malloc(la + lb + 1);
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = '\0';
    return out;
}

/* This module's own private "read `pj`'s OWN class-level field `name`"
 * helper -- see pico8_score_panel.c's own identical `pj_class_field_ref_
 * table` for the full rationale (another accepted per-module duplicate,
 * needed here now that `pj:hk`'s own body -- which reads `pj.ea`/`pj.gs`
 * this same class-level way -- lives in this file). */
static pico8_ob_table_t *pj_class_field_ref_table(const char *name) {
    pico8_value_t v;
    const pico8_class_t *cls = pico8_pj_class();
    if (cls != NULL && cls->defaults != NULL && pico8_ob_table_get_name(cls->defaults, name, &v) &&
        v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

/* Captured `t` upvalue for `pj:hk`'s own `function iw.lh() t-=1 return
 * t>0 end` closure -- see pico8_score_panel_display.h's own comment on
 * pico8_pj_hk for the full rationale on why a single module-level static
 * (not a per-call heap allocation) is a fully faithful representation. */
static int g_pj_hk_lh_t = 0;

static pico8_value_t pico8_pj_hk_lh_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    g_pj_hk_lh_t -= 1;
    return pico8_value_bool(g_pj_hk_lh_t > 0);
}

/* function pj:hk(yr)local iw,pp=
 * pj.ea,pj.gs[3426-yr]
 * iw[2][1],iw[3]=
 * "year "..yr,pp or nil
 * if pp then
 * local t=90
 * function iw.lh()
 * t-=1
 * return t>0
 * end
 * end
 * mh(iw)
 * end
 * (pico8/lua.lua:856-869.) See pico8_score_panel_display.h's own comment
 * on this declaration for the full trace. */
void pico8_pj_hk(pico8_fixed_t yr) {
    pico8_ob_table_t *iw = pj_class_field_ref_table("ea");
    const pico8_ob_table_t *gs = pj_class_field_ref_table("gs");
    pico8_value_t pp = pico8_value_ref(NULL);
    pico8_value_t iw2_val;
    char *yr_str;
    char *year_label;
    int idx;

    if (iw == NULL) {
        return;
    }

    idx = 3426 - (int)pico8_fixed_to_double(yr);
    if (gs != NULL) {
        pico8_ob_table_get_index(gs, idx, &pp);
    }

    yr_str = pico8_value_tostr(pico8_value_fixed(yr));
    year_label = pj_concat2("year ", yr_str);
    free(yr_str);

    if (pico8_ob_table_get_index(iw, 2, &iw2_val) && iw2_val.kind == PICO8_VALUE_REF && iw2_val.as.ref != NULL) {
        pico8_ob_table_set_index((pico8_ob_table_t *)iw2_val.as.ref, 1, pico8_value_string(year_label));
    }

    if (pj_display_truthy(pp)) {
        pico8_ob_table_set_index(iw, 3, pp);
        g_pj_hk_lh_t = 90;
        pico8_ob_table_set_name(iw, "lh", pico8_value_function(pico8_pj_hk_lh_dispatch));
    } else {
        pico8_ob_table_remove_index(iw, 3);
    }

    {
        pico8_instance_t *made = pico8_mh_new(iw);
        (void)made;
    }
}

/* This module's own private "ob_table positional entries -> pico8_list_t"
 * bridge -- see pico8_score_panel.c's own identical `pico8_score_panel_
 * positional_values` for the full cross-reference tally (this is that
 * same established per-module-duplicate pattern's 12th copy). Used here
 * to feed `bz`'s own 12 positional entries into `pico8_nv()`. */
static pico8_list_t pico8_score_panel_display_positional_values(const pico8_ob_table_t *table) {
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

/* Not a port of any named Lua function: the MIRROR IMAGE of the helper
 * above -- a NEW bridge this chunk introduces (no prior copy anywhere in
 * this port), needed because real Lua's own `local iw=nv(bz,...)  iw.rh=3
 * function iw.lh()...end` freely mixes `nv()`'s own positional results
 * with two EXTRA named fields on the very same table, something this
 * port's own `pico8_nv()` (which returns a plain array-only `pico8_list_t`)
 * has no room for. Builds a fresh `pico8_ob_table_t*` with one positional
 * entry per list element (1-based, in order) -- named fields (`rh`/`lh`)
 * are set by the caller afterward via ordinary `pico8_ob_table_set_name`.
 * Returns a new, heap-allocated table the caller owns and must release
 * with `pico8_ob_table_free()`. */
static pico8_ob_table_t *pico8_score_panel_display_list_to_ob_table(const pico8_list_t *list) {
    pico8_ob_table_t *table = pico8_ob_table_new();
    pico8_list_iter_t it;
    pico8_value_t v;
    int i = 1;

    if (list == NULL) {
        return table;
    }
    it = pico8_list_all(list);
    while (pico8_list_iter_next(&it, &v)) {
        pico8_ob_table_set_index(table, i, v);
        i++;
    }
    return table;
}

/* `function()ju.c_ = not ju.c_ end` (`de`'s own "my" field) -- a
 * complete, real implementation (not a placeholder): flips `pico8_ju()`'s
 * own "c_" field. A NULL `pico8_ju()` (before Chunk 22 reconciles it) is
 * a safe no-op, matching this port's pervasive defensive convention for
 * every not-yet-reconciled singleton read. */
static pico8_value_t pico8_pj_de_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *ju = pico8_ju();
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    if (ju != NULL) {
        pico8_value_t c_val;
        int current = pico8_instance_get_field(ju, "c_", &c_val) && pj_display_truthy(c_val);
        pico8_ob_table_set_name(ju->fields, "c_", pico8_value_bool(!current));
    }
    return pico8_value_ref(NULL);
}

/* `function iw.lh() return self.c_ end` (`pj:ph()`'s own closure) -- see
 * this file's own top comment for why reading `pico8_ju()`'s own "c_"
 * field directly (rather than a dedicated static self-capture, unlike
 * `pj:hk`'s own `t` upvalue) is the evidenced-correct representation
 * here: `self` inside `pj:ph()` is PROVABLY always `pico8_ju()` itself
 * (`pj()` is constructed exactly once in the whole real cart). A NULL
 * `pico8_ju()` reads as falsy (nothing to hold the box open for). */
static pico8_value_t pico8_pj_ph_lh_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *ju = pico8_ju();
    pico8_value_t c_val;
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    if (ju != NULL && pico8_instance_get_field(ju, "c_", &c_val)) {
        return pico8_value_bool(pj_display_truthy(c_val));
    }
    return pico8_value_bool(0);
}

/* Per-`nv()`-call captured context: just `s` (`ky()`'s own 6-entry
 * score-breakdown table) -- this port's established ctx-pointer stand-in
 * for a captured Lua upvalue (matching `pico8_jp_predicate_fn`/`pico8_nv_
 * fn`'s own convention, pico8_collect.h). */
typedef struct {
    const pico8_ob_table_t *s;
} pico8_pj_ph_ctx_t;

/* `function(st)if st.score then...else return st end end` (`pj:ph()`'s
 * own per-row `nv(bz,...)` mapper) -- see pico8_score_panel_display.h's
 * own top comment for the full field-by-field trace (the `s[0]`/`tostr`
 * divergence, the `dk.px` read, the score==6 star-rating rewrite, and the
 * `lq(st)` wrapper). Entries with no `.score` field pass through
 * unchanged (`e` itself, same REF); scored entries are mutated in place
 * (`st` is one of `bz`'s own shared class-level sub-tables, never
 * copied) and a fresh `{lq(st),st.ra,0.5,fn="ld"}` wrapper is returned. */
static int pico8_pj_ph_map(pico8_value_t e, void *ctx_ptr, pico8_value_t *out_value) {
    const pico8_pj_ph_ctx_t *ctx = (const pico8_pj_ph_ctx_t *)ctx_ptr;
    pico8_ob_table_t *st;
    pico8_value_t score_val;
    int score_i;
    pico8_value_t s_val;
    char *s_str;
    pico8_value_t ra_val = pico8_value_ref(NULL);
    pico8_instance_t *lq_inst;
    pico8_ob_table_t *wrapped;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        *out_value = e;
        return 1;
    }
    st = (pico8_ob_table_t *)e.as.ref;

    if (!pico8_ob_table_get_name(st, "score", &score_val) || score_val.kind != PICO8_VALUE_FIXED) {
        *out_value = e;
        return 1;
    }

    score_i = (int)pico8_fixed_to_double(score_val.as.fixed);

    s_val = pico8_value_ref(NULL);
    if (ctx->s != NULL) {
        pico8_ob_table_get_index(ctx->s, score_i, &s_val);
    }
    /* `s[0]` (score_i==0) is genuinely absent -- see this file's own top
     * comment on the resulting tostr(nil)-vs-tostr(ref(NULL)) divergence. */
    s_str = pico8_value_tostr(s_val);
    pico8_ob_table_set_index(st, 3, pico8_value_string(s_str));
    /* s_str is intentionally NOT freed here -- it is now st[3]'s own
     * value, rebind-leaked on the NEXT pj:ph() refresh exactly like every
     * other class-level-table string rebind in this chunk (see
     * pico8_score_panel.c's own pico8_pj_hb comment). */

    if (score_i == 0) {
        const pico8_instance_t *fm = pico8_fm();
        pico8_value_t hl_val = fm_field_value(fm, "hl");
        const char *hl_str = (hl_val.kind == PICO8_VALUE_STRING) ? hl_val.as.string : "current score";
        const pico8_ob_table_t *dk = pico8_dk();
        const char *px = "";
        pico8_value_t px_val;
        char *piece1;
        char *piece2;
        char *label;

        if (dk != NULL && pico8_ob_table_get_name(dk, "px", &px_val) && px_val.kind == PICO8_VALUE_STRING) {
            px = px_val.as.string;
        }
        piece1 = pj_concat2(hl_str, " (");
        piece2 = pj_concat2(piece1, px);
        free(piece1);
        label = pj_concat2(piece2, ")");
        free(piece2);
        pico8_ob_table_set_index(st, 1, pico8_value_string(label));
    }

    if (score_i == 6) {
        pico8_value_t s6_val = pico8_value_fixed(pico8_fixed_from_int(0));
        int i;
        if (ctx->s != NULL) {
            pico8_ob_table_get_index(ctx->s, 6, &s6_val);
        }
        for (i = 1; i <= 5; i++) {
            int icon = (i <= (int)pico8_fixed_to_double(s6_val.as.fixed)) ? 56 : 57;
            pico8_ob_table_set_index(st, i, pico8_value_fixed(pico8_fixed_from_int(icon)));
        }
    }

    pico8_ob_table_get_name(st, "ra", &ra_val);
    lq_inst = pico8_lq_new(st);
    wrapped = pico8_ob_table_new();
    pico8_ob_table_set_index(wrapped, 1, pico8_value_ref(lq_inst));
    pico8_ob_table_set_index(wrapped, 2, ra_val);
    pico8_ob_table_set_index(wrapped, 3, pico8_value_fixed(pico8_fixed_from_double(0.5)));
    pico8_ob_table_set_name(wrapped, "fn", pico8_value_string("ld"));
    *out_value = pico8_value_ref(wrapped);
    return 1;
}

/* function pj:ph()if fm.hl and not self.gz then
 * fq()js(ob([[...]],{my=_init}))js(ob([[...]],de))self.c_,self.gz,self.fa=
 * true,0,13
 * end
 * if self.c_ and kc~=self.ed then
 * ...
 * self.ed=mh(iw)end
 * end
 * (pico8/lua.lua:870-896.) See pico8_score_panel_display.h's own top
 * comment for the complete field-by-field trace of both blocks. */
void pico8_pj_ph(pico8_instance_t *self) {
    const pico8_instance_t *fm = pico8_fm();
    pico8_value_t hl_val = fm_field_value(fm, "hl");
    pico8_value_t gz_val;
    int has_gz = (self != NULL) && pico8_instance_get_field(self, "gz", &gz_val);

    if (pj_display_truthy(hl_val) && !has_gz) {
        pico8_ob_table_t *my_init_overrides;
        pico8_ob_table_t *spec1;
        pico8_ob_table_t *spec2;
        pico8_instance_t *made1;
        pico8_instance_t *made2;

        (void)pico8_fq_new(NULL);

        my_init_overrides = pico8_ob_table_new();
        pico8_ob_table_set_name(my_init_overrides, "my", pico8_init_field_value());
        spec1 = pico8_ob("\nfa=15,\nra=v(51,100),\nlq=o(\" again? \"),\n", my_init_overrides);
        pico8_ob_table_free(my_init_overrides);
        made1 = pico8_js_new(spec1);
        (void)made1;
        pico8_ob_table_free(spec1);

        spec2 = pico8_ob("\nfa=15,\nra=v(39,100),\nlq=o(o(10,7)),\n", g_de);
        made2 = pico8_js_new(spec2);
        (void)made2;
        pico8_ob_table_free(spec2);

        if (self != NULL) {
            pico8_ob_table_set_name(self->fields, "c_", pico8_value_bool(1));
            pico8_ob_table_set_name(self->fields, "gz", pico8_value_fixed(pico8_fixed_from_int(0)));
            pico8_ob_table_set_name(self->fields, "fa", pico8_value_fixed(pico8_fixed_from_int(13)));
        }
    }

    {
        pico8_value_t c_val;
        int c_truthy = (self != NULL) && pico8_instance_get_field(self, "c_", &c_val) && pj_display_truthy(c_val);
        pico8_value_t ed_val;
        int ed_equals_kc = 0;

        /* Real Lua `kc~=self.ed`: `self.ed`'s own class-level default is
         * the NUMBER 1 (`ed=1`, pj's own class spec above), not nil -- Lua
         * `~=` across two different types (nil `kc` vs. number 1) is
         * unconditionally true, so this block fires on the very first
         * eligible tick. A prior version of this port only ever built a
         * comparable `ed_ptr` when `self.ed` already held a PICO8_VALUE_REF
         * (i.e. only after THIS SAME block had already run once), which
         * left `ed_ptr` at its own NULL default just like `pico8_kc()`'s
         * own NULL default -- `NULL!=NULL` is false, so the block never
         * fired at all on a fresh `pj` instance. Found and fixed during
         * Chunk 18's own verification: a non-REF `self.ed` (the class
         * default, never yet replaced by a real mh() instance) is never
         * equal to `pico8_kc()`, whatever `pico8_kc()` currently holds. */
        if (self != NULL && pico8_instance_get_field(self, "ed", &ed_val) && ed_val.kind == PICO8_VALUE_REF) {
            ed_equals_kc = (pico8_kc() == (pico8_instance_t *)ed_val.as.ref);
        }

        if (c_truthy && !ed_equals_kc) {
            pico8_ob_table_t *s = pico8_ky();
            pico8_list_t bz_list = pico8_score_panel_display_positional_values(pico8_bz());
            pico8_pj_ph_ctx_t ctx;
            pico8_list_t iw_list;
            pico8_ob_table_t *iw_table;
            pico8_instance_t *made;

            ctx.s = s;
            iw_list = pico8_nv(&bz_list, pico8_pj_ph_map, &ctx);
            pico8_list_free(&bz_list);

            iw_table = pico8_score_panel_display_list_to_ob_table(&iw_list);
            pico8_list_free(&iw_list);

            pico8_ob_table_set_name(iw_table, "rh", pico8_value_fixed(pico8_fixed_from_int(3)));
            pico8_ob_table_set_name(iw_table, "lh", pico8_value_function(pico8_pj_ph_lh_dispatch));

            made = pico8_mh_new(iw_table);
            pico8_ob_table_free(iw_table);
            pico8_ob_table_free(s);

            if (self != NULL) {
                pico8_ob_table_set_name(self->fields, "ed", pico8_value_ref(made));
            }
        }
    }
}

/* Builds the top-level `de` table once -- see pico8_score_panel_display.h's
 * own comment on this declaration. */
void pico8_score_panel_display_init(void) {
    pico8_score_panel_display_shutdown();
    g_de = pico8_ob_table_new();
    pico8_ob_table_set_name(g_de, "my", pico8_value_function(pico8_pj_de_my_dispatch));
    g_pj_hk_lh_t = 0;
}

void pico8_score_panel_display_shutdown(void) {
    if (g_de != NULL) {
        pico8_ob_table_free(g_de);
        g_de = NULL;
    }
}
