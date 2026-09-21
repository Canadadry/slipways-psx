#include "pico8_score_panel.h"

#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_hud.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_msgbox.h"
#include "pico8_pi.h"
#include "pico8_probe.h"
#include "pico8_score_panel_display.h"
#include "pico8_stash.h"
#include "pico8_vec.h"

/* See pico8_score_panel.h's own top comment for the full source/trace of
 * `pj`, `ef`, `bz`, and the `_init`/`ju` forward-reference placeholders.
 * `pj:ph()` itself lives in pico8_score_panel_display.c/.h. */

static pico8_class_t *g_pj_class = NULL;
static pico8_ob_table_t *g_ef = NULL;
static pico8_ob_table_t *g_bz = NULL;
static pico8_instance_t *g_ju = NULL;

/* Not a port of any named Lua function: this module's own private
 * "positional entries of a pico8_ob_table_t -> pico8_list_t" bridge,
 * matching pico8_tech_gate_positional_values'/pico8_layout_positional_
 * values'/pico8_msgbox_positional_values'/pico8_research_positional_
 * values'/pico8_data_init_positional_values'/the two `positional_values`
 * copies in pico8_planet_trade.c and pico8_planet_family_trade.c/pico8_jo_
 * positional_values' (pico8_planet_display.c)/pico8_help_positional_
 * values' (pico8_help.c) own established per-module-duplicate convention
 * -- an 11th copy on an independent recount, following the exact same
 * shape as all ten. Used by `pico8_pj_kj` for both `pj.i_` and `pj.ou`.
 * Returns a new, heap-backed list the caller owns and must release with
 * pico8_list_free(). */
static pico8_list_t pico8_score_panel_positional_values(const pico8_ob_table_t *table) {
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

/* This module's own private copy of `pico8_economy_score.c`'s established
 * `fixed_min` helper (that file's own top comment already documents this
 * exact "private fixed_min/fixed_max, added once THIS chunk's own
 * repeated need for it shows up" per-module-duplicate convention) -- needed
 * by `pj:kj`'s own `min(self.gz,3.2)`. */
static pico8_fixed_t fixed_min(pico8_fixed_t a, pico8_fixed_t b) {
    return (a < b) ? a : b;
}

/* This module's own private field-reader helpers for the `fm` singleton
 * (Chunk 12's `hy` economy instance, pico8_probe.h's `pico8_fm()`) --
 * matching pico8_economy_score.c's own `field_fixed`/`field_ref_table`
 * established shape exactly (another accepted per-module duplicate: `fm`
 * is read defensively from many separate modules across this port, never
 * through one shared accessor family). `pico8_fm()` itself may still be
 * the Chunk-10 classless placeholder (no real `hy:pq()` fields set) --
 * every read below tolerates that the same defensive way
 * `pico8_tech_gate.c`'s own `pico8_fm() != NULL && ...`-guarded sites do. */
static pico8_fixed_t fm_field_fixed(const pico8_instance_t *fm, const char *name) {
    pico8_value_t v;
    if (fm != NULL && pico8_instance_get_field(fm, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static pico8_value_t fm_field_value(const pico8_instance_t *fm, const char *name) {
    pico8_value_t v;
    if (fm != NULL && pico8_instance_get_field(fm, name, &v)) {
        return v;
    }
    return pico8_value_ref(NULL);
}

static const pico8_ob_table_t *fm_field_ref_table(const pico8_instance_t *fm, const char *name) {
    pico8_value_t v;
    if (fm != NULL && pico8_instance_get_field(fm, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (const pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

/* This module's own private copy of `pico8_economy_score.c`'s established
 * `get_by_value` helper -- looks `table[key]` up the same kind-polymorphic
 * way `pico8_ho_apply`'s own `r[ri[i]]` read already documents (a
 * PICO8_VALUE_FIXED key indexes by number, a PICO8_VALUE_STRING key
 * indexes by name), needed here for `pj:hb`'s own `fm.po[r.k]`. */
static int get_by_value(const pico8_ob_table_t *table, pico8_value_t key, pico8_value_t *out) {
    if (table == NULL) {
        return 0;
    }
    if (key.kind == PICO8_VALUE_FIXED) {
        return pico8_ob_table_get_index(table, (int)pico8_fixed_to_double(key.as.fixed), out);
    } else if (key.kind == PICO8_VALUE_STRING && key.as.string != NULL) {
        return pico8_ob_table_get_name(table, key.as.string, out);
    }
    return 0;
}

/* This module's own private generalization of `pico8_economy_score.c`'s
 * established `concat_percent` helper (that file's own top comment
 * already flags string concatenation as "a smaller, distinct need" than
 * `pico8_hp_jb_join`'s own growable-buffer shape) -- a plain two-piece
 * concat, covering every real Lua `..` this chunk's own methods perform
 * (`fm.nz.."$"`, `"+"..N`, `fm.happiness.."%"`, `"year "..yr`, and the
 * `hl.." ("..px..")"` label built from repeated two-piece concatenation in
 * pico8_score_panel_display.c). Returns a newly heap-allocated string the
 * caller owns and must free(). */
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
 * helper -- see pico8_score_panel.h's own top comment on why `pj.ea`/
 * `pj.gs`/`pj.cr`/`pj.i_`/`pj.ou` are read/written as the CLASS'S OWN
 * shared spec table, never `self.<field>`, mirroring `bv()`'s own
 * `pico8_ui_class()->defaults`-style direct-class-field read
 * (pico8_probe.h). Returns NULL if `pj`'s class isn't built yet, or the
 * named field is absent/not a REF. */
static pico8_ob_table_t *pj_class_field_ref_table(const char *name) {
    pico8_value_t v;
    if (g_pj_class != NULL && g_pj_class->defaults != NULL &&
        pico8_ob_table_get_name(g_pj_class->defaults, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

/* NOTE: this module used to also carry a private `pj_truthy` Lua-
 * truthiness helper (nil/false falsy, everything else -- including 0 --
 * truthy, WIDENED past `pico8_ui.c`'s own `pico8_ui_truthy`/`pico8_
 * economy_score.c`'s own `economy_truthy` BOOL-only precedent to also
 * treat this port's own `pico8_value_ref(NULL)` "absent" filler as
 * falsy) -- its only real caller, `pico8_pj_hk`, moved to
 * `pico8_score_panel_display.c` (see that file's own identical `pj_
 * display_truthy` copy) once this file's own line count crossed the
 * ~430-540-line range this port's own warn-review threshold flags, so
 * this copy was removed rather than left as dead code. */

/* *** THE `_init` FORWARD-REFERENCE PLACEHOLDER *** -- see
 * pico8_score_panel.h's own top comment for the full trace. */
static pico8_value_t pico8_pj_init_placeholder(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(NULL);
}

static pico8_value_function_t g_init_dispatch = pico8_pj_init_placeholder;

pico8_value_t pico8_init_field_value(void) {
    return pico8_value_function(g_init_dispatch);
}

void pico8_score_panel_set_init(pico8_value_function_t fn) {
    g_init_dispatch = (fn != NULL) ? fn : pico8_pj_init_placeholder;
}

/* `eg()` -- see pico8_score_panel.h's own top comment for the full trace
 * on why this is a deliberate, documented stub (Chunk 21's own real
 * function, not this chunk's job to port) rather than a get/set
 * placeholder pair. */
void pico8_eg(void) {
    /* Intentional no-op -- see this file's own top comment on this
     * declaration. */
}

/* function pj:hb()local cf=ne(fm.ge)local ou={fm.nz.."$",cf,fm.jk,
 * "+"..fm.po[r.k]+1,fm.happiness.."%",ef[fm.lo],fm.yr
 * }
 * for i,p in pairs(ou)do
 * pj.ou[i][1]=p
 * end
 * eg()end
 * (pico8/lua.lua:829-834.) See pico8_score_panel.h's own comment on this
 * declaration for the dispatch-name trace. Field by field: `cf=ne(fm.ge)`
 * keeps only `pico8_ne`'s own FIRST return value (its forced color is
 * discarded, matching `ne(fm.ge)`'s own real single-variable receiver --
 * see pico8_collect.h's own comment on pico8_ne for why any `c` is fine
 * when the caller never reads it back); `fm.nz.."$"`/`fm.happiness.."%"`/
 * `"+"..(fm.po[r.k]+1)` (`..` binds looser than `+`, so this is `"+"..
 * (fm.po[r.k]+1)`, not `("+"..fm.po[r.k])+1`) are built via `pico8_value_
 * tostr` + this module's own `pj_concat2`; `fm.jk`/`fm.yr` are copied
 * through AS-IS (no coercion at all in the real source -- whatever kind
 * `fm.jk`/`fm.yr` hold, `pj.ou[3][1]`/`pj.ou[7][1]` simply become that same
 * value, read later by `bl()`'s own `fn="qr"`-dispatched drawing, out of
 * this chunk's own scope to interpret further); `ef[fm.lo]` is a plain
 * 1-based `pico8_ob_table_get_index` into `ef` (this chunk's own data
 * literal, above). Every `ou[i]` is then written into `pj.ou[i][1]`
 * (`pj_class_field_ref_table("ou")`, this module's own class-level-field
 * helper) -- `pj.ou`'s OWN 7 sub-tables' own first positional slot,
 * REBINDING whatever heap string a PRIOR `pj:hb()` call put there (the
 * SAME accepted "field-rebind leaks the old value" convention this port
 * has documented pervasively since `pico8_msgbox.h`'s own `kc` comment,
 * not a new gap this chunk introduces). Finally calls `eg()` (this
 * module's own deliberate stub, see its own comment above). Needs no
 * `self`: every field this reads/writes is either the `fm`/`r`/`ef`
 * singletons or `pj`'s own class-level `ou` table. */
void pico8_pj_hb(void) {
    const pico8_instance_t *fm = pico8_fm();
    pico8_fixed_t fm_ge = fm_field_fixed(fm, "ge");
    pico8_fixed_t fm_nz = fm_field_fixed(fm, "nz");
    pico8_value_t fm_jk = fm_field_value(fm, "jk");
    pico8_fixed_t fm_happiness = fm_field_fixed(fm, "happiness");
    pico8_fixed_t fm_lo = fm_field_fixed(fm, "lo");
    pico8_value_t fm_yr = fm_field_value(fm, "yr");
    const pico8_ob_table_t *fm_po = fm_field_ref_table(fm, "po");
    const pico8_ob_table_t *r = pico8_r();
    pico8_ob_table_t *ou_table;
    pico8_value_t ou[7];
    char *cf;
    char *nz_str;
    char *happiness_str;
    char *plus_num;
    int discard_color = 0;
    int i;

    cf = pico8_ne(fm_ge, 0, NULL, &discard_color);

    nz_str = pico8_value_tostr(pico8_value_fixed(fm_nz));
    ou[0] = pico8_value_string(pj_concat2(nz_str, "$"));
    free(nz_str);

    ou[1] = pico8_value_string(cf);

    ou[2] = fm_jk;

    {
        pico8_value_t k_key;
        pico8_value_t po_val = pico8_value_fixed(pico8_fixed_from_int(0));
        if (r != NULL && pico8_ob_table_get_name(r, "k", &k_key)) {
            pico8_value_t looked;
            if (get_by_value(fm_po, k_key, &looked) && looked.kind == PICO8_VALUE_FIXED) {
                po_val = looked;
            }
        }
        plus_num =
            pico8_value_tostr(pico8_value_fixed(pico8_fixed_add(po_val.as.fixed, pico8_fixed_from_int(1))));
        ou[3] = pico8_value_string(pj_concat2("+", plus_num));
        free(plus_num);
    }

    happiness_str = pico8_value_tostr(pico8_value_fixed(fm_happiness));
    ou[4] = pico8_value_string(pj_concat2(happiness_str, "%"));
    free(happiness_str);

    {
        pico8_value_t ef_val = pico8_value_ref(NULL);
        if (g_ef != NULL) {
            pico8_ob_table_get_index(g_ef, (int)pico8_fixed_to_double(fm_lo), &ef_val);
        }
        ou[5] = ef_val;
    }

    ou[6] = fm_yr;

    ou_table = pj_class_field_ref_table("ou");
    if (ou_table != NULL) {
        for (i = 0; i < 7; i++) {
            pico8_value_t sub_val;
            if (pico8_ob_table_get_index(ou_table, i + 1, &sub_val) && sub_val.kind == PICO8_VALUE_REF &&
                sub_val.as.ref != NULL) {
                pico8_ob_table_set_index((pico8_ob_table_t *)sub_val.as.ref, 1, ou[i]);
            }
        }
    }

    pico8_eg();
}

static pico8_value_t pico8_pj_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_pj_hb();
    return pico8_value_ref(NULL);
}

/* function pj:kj()bl(la(pj.i_,pj.ou))if self.gz then
 * self.gz+=0.2
 * local h=min(self.gz,3.2)^2
 * rk(0,h,0)rk(127-h,127)end
 * end
 * (pico8/lua.lua:848-855.) See pico8_score_panel.h's own comment on this
 * declaration for the full trace. */
void pico8_pj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self) {
    pico8_ob_table_t *i_table = pj_class_field_ref_table("i_");
    pico8_ob_table_t *ou_table = pj_class_field_ref_table("ou");
    pico8_list_t i_list = pico8_score_panel_positional_values(i_table);
    pico8_list_t ou_list = pico8_score_panel_positional_values(ou_table);
    pico8_list_t combined = pico8_la(&i_list, &ou_list);
    pico8_value_t gz_val;

    pico8_bl_default(gfx, mem, &combined);

    pico8_list_free(&i_list);
    pico8_list_free(&ou_list);
    pico8_list_free(&combined);

    if (self != NULL && pico8_instance_get_field(self, "gz", &gz_val) && gz_val.kind == PICO8_VALUE_FIXED) {
        pico8_fixed_t gz = pico8_fixed_add(gz_val.as.fixed, pico8_fixed_from_double(0.2));
        pico8_fixed_t capped = fixed_min(gz, pico8_fixed_from_double(3.2));
        pico8_fixed_t h = pico8_fixed_mul(capped, capped);

        pico8_ob_table_set_name(self->fields, "gz", pico8_value_fixed(gz));
        pico8_rk(gfx, mem, pico8_fixed_from_int(0), h, 0);
        pico8_rk_default(gfx, mem, pico8_fixed_sub(pico8_fixed_from_int(127), h), pico8_fixed_from_int(127));
    }
}

static pico8_value_t pico8_pj_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg1;
    (void)arg0;
    pico8_pj_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* `pico8_pj_hk`'s own BODY (plus its own `iw.lh`-closure static/dispatch)
 * lives in `pico8_score_panel_display.c`, alongside `pj:ph()` -- moved
 * there once this file's own line count crossed the ~430-540-line range
 * this port's own warn-review threshold flags (see this file's own
 * bottom-of-chunk note in docs/portage-progress.md's Vague 18 entry for
 * the exact before/after tally). This dispatch bridge stays HERE,
 * alongside `pj`'s own class-registration wiring (`pico8_score_panel_
 * init()`, below) that installs it -- the SAME "dispatch bridge lives
 * with the registration code, function lives in the sibling module"
 * precedent `pico8_jo_hb_dispatch`/`pico8_jo_gv_dispatch`
 * (`pico8_planet.c`, calling across to `pico8_planet_display.h`'s
 * `pico8_jo_hb`/`pico8_jo_gv`) already establishes. */
static pico8_value_t pico8_pj_hk_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t yr = pico8_fixed_from_int(0);
    (void)self;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        yr = arg0.as.fixed;
    }
    pico8_pj_hk(yr);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_pj_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_pj_ph((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* pj=ke:jr([[...]]) -- see pico8_score_panel.h's own top comment for the
 * full trace. Also builds `ef`/`bz` (this chunk's own two data literals,
 * single consumer each, folded into this shared init/shutdown pair the
 * same way `fv.h`'s own `bo` already is). `pico8_pi_init()` is called
 * defensively first (idempotent) since both the class spec (`cr`'s own
 * `v(0,11)`) and `bz` (every score row's own `ra=v(...)`) need `pi['v']`
 * already registered. */
void pico8_score_panel_init(void) {
    pico8_score_panel_shutdown();

    pico8_pi_init();

    g_ef = pico8_ob(
        "\"JAN\",\"FEB\",\"MAR\",\"APR\",\"MAY\",\"JUN\",\"JUL\",\"AUG\",\"SEP\",\"OCT\",\"NOV\",\"DEC\",\"UND\","
        "\"DUO\",\"TER\",",
        NULL);

    g_bz = pico8_ob("\n"
                     "o(26,86,0,fn=\"br\"),\n"
                     "o(51,59,1,fn=\"br\"),\n"
                     "o(76,77,1,fn=\"br\"),\n"
                     "o(24,25,1,fn=\"br\"),\n"
                     "o(33,41,1,fn=\"br\"),\n"
                     "o(\"\",score=0,ra=v(64,21),c=5),\n"
                     "o(55,\"planets:   \",\n"
                     "score=1,ra=v(64,34)),\n"
                     "o(19,\"population:\",\n"
                     "score=2,ra=v(64,43)),\n"
                     "o(24,\"technology:\",\n"
                     "score=3,ra=v(64,52)),\n"
                     "o(o(27,5),o(f,0),f,o(25,5),\"happiness\",\n"
                     "score=4,ra=v(64,61)),\n"
                     "o(\"total:\",\"\",\n"
                     "score=5,ra=v(64,74)),\n"
                     "o(score=6,ra=v(64,82)),\n",
                     NULL);

    g_pj_class = pico8_class_declare(
        pico8_ke_class(),
        "\nfa=10,\nea=o(\no(42,43,1,fn=\"br\"),\no(\"\",64,40,6,0.5,fn=\"qr\"),\n),\n"
        "gs=o(\no(\"last year!\",64,48,9,0.5,fn=\"qr\"),\no(\"2 years remain\",64,48,2,0.5,fn=\"qr\"),\nf,f,\n"
        "o(\"5 years remain\",64,48,2,0.5,fn=\"qr\"),\n),\n"
        "cr=o(\nhi=v(0,11),gq=v(0,11),ca=10,\n),\n"
        "i_=o(\no(126,127,1,fn=\"br\"),\no(0,1,1,fn=\"br\"),\no(8,118,34,fn=\"rf\"),\no(0,-3,26,fn=\"rf\"),"
        "\no(81,-3,117,fn=\"rf\"),\n),\n"
        "ou=o(\no(f,13,121,9,fn=\"qr\"),\no(f,37,121,2,fn=\"qr\"),\no(f,13,2,9,fn=\"qr\"),\no(f,29,2,2,fn=\"qr\"),"
        "\no(f,59,2,2,fn=\"qr\"),\no(f,85,2,13,fn=\"qr\"),\no(f,99,2,13,fn=\"qr\"),\n),\n"
        "ed=1,\n");

    pico8_ob_table_set_name(g_pj_class->defaults, "hb", pico8_value_function(pico8_pj_hb_dispatch));
    pico8_ob_table_set_name(g_pj_class->defaults, "kj", pico8_value_function(pico8_pj_kj_dispatch));
    pico8_ob_table_set_name(g_pj_class->defaults, "hk", pico8_value_function(pico8_pj_hk_dispatch));
    pico8_ob_table_set_name(g_pj_class->defaults, "ph", pico8_value_function(pico8_pj_ph_dispatch));

    pico8_score_panel_set_init(NULL);
    g_ju = NULL;

    pico8_score_panel_display_init();
}

/* Not part of the Lua original (no manual teardown under GC). `ju` is
 * reset to NULL WITHOUT freeing whatever instance it pointed to -- the
 * same deliberate "never frees, it's an independently hv/cv/cg-owned
 * entity" convention `pico8_msgbox_set_kc`'s own comment already
 * documents for `kc` (pico8_msgbox.h), for the identical reason: `ju` is
 * a plain Lua-style bookmark into the one real `pj` instance, not
 * something this module owns the lifetime of. */
void pico8_score_panel_shutdown(void) {
    pico8_score_panel_display_shutdown();
    if (g_pj_class != NULL) {
        pico8_class_free(g_pj_class);
        g_pj_class = NULL;
    }
    if (g_ef != NULL) {
        pico8_ob_table_free(g_ef);
        g_ef = NULL;
    }
    if (g_bz != NULL) {
        pico8_ob_table_free(g_bz);
        g_bz = NULL;
    }
    pico8_score_panel_set_init(NULL);
    g_ju = NULL;
}

pico8_class_t *pico8_pj_class(void) {
    return g_pj_class;
}

pico8_instance_t *pico8_pj_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_pj_class, overrides);
}

const pico8_ob_table_t *pico8_ef(void) {
    return g_ef;
}

const pico8_ob_table_t *pico8_bz(void) {
    return g_bz;
}

pico8_instance_t *pico8_ju(void) {
    return g_ju;
}

void pico8_pj_set_ju(pico8_instance_t *instance) {
    g_ju = instance;
}
