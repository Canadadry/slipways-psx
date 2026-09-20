/* See pico8_research.h's own top comment for the full source/trace of the
 * `hp` class's own core (pq/po/od, plus its own gb registry) ported here.
 * `hp:lq`/`hp:jb`, the standalone `jo.ex`, and `bj` live in the sibling
 * module pico8_research_display.c/.h instead -- this module alone crossed
 * the ~430-540-line range already flagged twice by warn-review (before the
 * split, ~780 lines), so it is split proactively along the same "class
 * core" vs. "display/query helpers built on top of it" seam this chunk's
 * own PRD text already groups these functions by (see docs/portage-
 * progress.md's own Vague 11 entry for the size-check trace). */

#include "pico8_research.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_research_display.h"
#include "pico8_stash.h"
#include "pico8_tech_gate.h"

static pico8_class_t *g_hp_class = NULL;
static pico8_ob_table_t *g_gb = NULL;
/* `rr`'s own string is borrowed directly from `fd`'s backing storage (see
 * pico8_research_build_gb below) -- never copied/freed by this module. */
static const char *g_rr = NULL;

static void pico8_hp_pq(pico8_instance_t *self);

/* Lua truthiness (nil/false falsy, everything else truthy), extended to
 * also treat PICO8_VALUE_REF(NULL) as falsy -- same extension
 * pico8_probe.c's own pico8_probe_truthy/pico8_tech_gate.c's own
 * pico8_tech_gate_truthy already establish, needed for `self.ii`/`self.il`
 * checks below. Private per-module copy, matching this port's own
 * pervasive per-module-truthy-helper convention. */
static int pico8_research_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* Not a port of any named Lua function: a private per-module bridge from
 * "positional entries of a pico8_ob_table_t" to a pico8_list_t, matching
 * pico8_layout_positional_values'/pico8_msgbox_positional_values' own
 * established per-module-duplicate convention (pico8_layout.c/
 * pico8_msgbox.c) -- used here for `hp.gp` (a positional class-default
 * field) and `fd` (this module's own gb-building loop). A separate copy
 * lives in the sibling pico8_research_display.c (used there for `gb`
 * itself, `pico8_bj`'s own `jp(gb,...)`) rather than sharing this one
 * across the module-size split -- same documented-duplication tradeoff as
 * every other per-module copy of this helper in this port. Returns a new,
 * heap-backed list the caller owns and must release with
 * pico8_list_free(). */
static pico8_list_t pico8_research_positional_values(const pico8_ob_table_t *table) {
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

/* Not part of the Lua original -- see pico8_research_shutdown's own header
 * comment on why only `gb`'s POSITIONAL entries need freeing here (its
 * NAMED entries are always aliases of the same instances). */
static void pico8_research_free_gb_instances(void) {
    int i;

    if (g_gb == NULL) {
        return;
    }
    for (i = 0; i < g_gb->count; i++) {
        if (g_gb->entries[i].key.kind == PICO8_OB_KEY_INDEX && g_gb->entries[i].value.kind == PICO8_VALUE_REF &&
            g_gb->entries[i].value.as.ref != NULL) {
            pico8_instance_free(g_gb->entries[i].value.as.ref);
        }
    }
}

/* Bridges to pico8_hp_lq's own signature (pico8_research_display.h) --
 * see pico8_research.h's own top comment for why this registration exists
 * (a real gap found while porting `jo:fe`, Chunk 13, sub-lot 3: the real
 * source's `mv(b,"lq")` needs `hp`'s class-level "lq" field to actually be
 * PICO8_VALUE_FUNCTION-kind, which this module never wired). */
static pico8_value_t pico8_hp_lq_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(pico8_hp_lq((pico8_instance_t *)self));
}

/* hp=kv:jr([[
 * mr=o("px","lr","fj","ja","i","o","il","dm"),
 * gp=o("o","dm"),
 * fk=o(f,2),
 * ]])
 * (pico8/lua.lua:1399-1402.) See pico8_research.h's own top comment for the
 * full trace. A ROOT class (`kv:jr`, parent NULL), never `ke`-descended. */
void pico8_research_init(void) {
    pico8_research_shutdown();
    g_hp_class =
        pico8_class_declare(NULL, "\nmr=o(\"px\",\"lr\",\"fj\",\"ja\",\"i\",\"o\",\"il\",\"dm\"),\n"
                                   "gp=o(\"o\",\"dm\"),\n"
                                   "fk=o(f,2),\n");
    pico8_class_set_pq(g_hp_class, pico8_hp_pq);
    pico8_ob_table_set_name(g_hp_class->defaults, "lq", pico8_value_function(pico8_hp_lq_dispatch));
}

void pico8_research_shutdown(void) {
    pico8_research_free_gb_instances();
    pico8_ob_table_free(g_gb);
    g_gb = NULL;
    g_rr = NULL;
    if (g_hp_class != NULL) {
        pico8_class_free(g_hp_class);
        g_hp_class = NULL;
    }
}

pico8_class_t *pico8_hp_class(void) {
    return g_hp_class;
}

pico8_instance_t *pico8_hp_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_hp_class, overrides);
}

/* `function(c) return {qe=r[c]} end` -- hp:pq's own fi() callback (see
 * pico8_hp_pq below and pico8/lua.lua:1406-1407). Always contributes a
 * value (a real Lua table constructor `{qe=...}` is never nil, even when
 * `r[c]` itself is absent), matching pico8_fi's own "1 = contribute"
 * convention unconditionally, never skipping a character. */
static int pico8_hp_pq_i_step(char c, int i, void *ctx, pico8_value_t *out_value) {
    pico8_ob_table_t *wrapper = pico8_ob_table_new();
    pico8_value_t qe;
    char key[2];

    (void)i;
    (void)ctx;
    key[0] = c;
    key[1] = '\0';
    if (!pico8_ob_table_get_name(pico8_r(), key, &qe)) {
        qe = pico8_value_ref(NULL);
    }
    pico8_ob_table_set_name(wrapper, "qe", qe);
    *out_value = pico8_value_ref(wrapper);
    return 1;
}

/* function hp:pq()ho(gb,hp.mr,hp.gp)(self)if self.ii then
 * self.lq={self.px,32,{self.ii,8}}
 * end
 * self.i=fi(self.i or "",function(c)
 * return {qe=r[c]}
 * end)end
 * (pico8/lua.lua:1403-1408.) Wired as `hp`'s own class `pq` -- runs
 * automatically at every `hp(...)` instantiation (this chunk's own only
 * real call site: pico8_research_build_gb's `add(gb,hp(ri))`, below).
 *
 * `ho(gb,hp.mr,hp.gp)(self)`: `hp.mr`/`hp.gp` are read directly off
 * `hp`'s own class defaults (`g_hp_class->defaults`), matching `bg:pq`'s
 * own established "read a class's own default field directly" precedent,
 * not `self`'s (though the prototype-chain fallback would find the exact
 * same values either way, since neither `mr` nor `gp` is ever overridden
 * per-instance in any real ri record). `hp.gp` (a positional REF'd
 * ob_table) is converted to a pico8_list_t via this module's own
 * pico8_research_positional_values, since pico8_ho_apply's own `gp`
 * parameter needs one. THE REAL `rc[ri.px]=ri` TAIL OF `ho()`'S OWN
 * RETURNED CLOSURE STORES THE WHOLE INSTANCE `self` (real Lua has no
 * separate "fields sub-object" -- an instance IS its own table), NOT just
 * its own fields table -- so pico8_ho_apply (which was built for df/nb's
 * OWN shape, where `ri` genuinely is a bare pico8_ob_table_t) is called
 * here against a THROWAWAY scratch table (absorbing its own built-in
 * registration harmlessly), and the REAL `gb[self.px]=self` registration
 * is done separately, storing `self` (a pico8_instance_t*), matching what
 * pico8_research_build_gb's own `add(gb,hp(ri))` already stores
 * positionally -- keeping gb's own two registration paths (positional
 * add(), named `[px]=`) consistently typed. This was found and fixed via
 * this chunk's own disposable verification program (a real
 * heap-use-after-free-shaped bug caught by mismatched types, not by ASan
 * directly -- see this chunk's own final report).
 *
 * `if self.ii then self.lq={self.px,32,{self.ii,8}} end`: a plain DATA
 * field write, genuinely named "lq" -- SHADOWING the class-level `hp.lq`
 * METHOD slot for ordinary `self.lq` field access when `self.ii` is
 * truthy (see pico8_research_display.h's own pico8_hp_lq comment for the full
 * discussion of this preserved-not-fixed naming collision).
 *
 * `self.i=fi(self.i or "",...)`: `self.i` read BEFORE this line already
 * holds whatever the `ho()` renaming pass above just wrote (position 5's
 * raw value, renamed to "i") -- absent/non-STRING reads defensively as ""
 * (matching `self.i or ""`'s own Lua truthiness, and this port's
 * established "wrong-kind field reads as its safe default" divergence).
 * The fi() result replaces `self.i` entirely with a fresh heap
 * pico8_list_t (REF-wrapped, matching this port's own "Lua list field ->
 * heap pico8_list_t* REF" convention, e.g. pico8_layout.c's own `self.fs`). */
static void pico8_hp_pq(pico8_instance_t *self) {
    pico8_value_t mr_val;
    pico8_value_t gp_val;
    const pico8_ob_table_t *mr = NULL;
    pico8_list_t gp_list;
    pico8_value_t ii_val;
    pico8_value_t i_val;
    pico8_list_t new_i;
    pico8_list_t *new_i_heap;
    const char *i_src = "";

    if (self == NULL || g_hp_class == NULL) {
        return;
    }

    if (pico8_ob_table_get_name(g_hp_class->defaults, "mr", &mr_val) && mr_val.kind == PICO8_VALUE_REF) {
        mr = mr_val.as.ref;
    }
    pico8_list_init(&gp_list);
    if (pico8_ob_table_get_name(g_hp_class->defaults, "gp", &gp_val) && gp_val.kind == PICO8_VALUE_REF) {
        gp_list = pico8_research_positional_values(gp_val.as.ref);
    }
    {
        pico8_ob_table_t *scratch_rc = pico8_ob_table_new();
        pico8_value_t px_val;

        pico8_ho_apply(scratch_rc, mr, &gp_list, self->fields);
        pico8_ob_table_free(scratch_rc);

        if (pico8_ob_table_get_name(self->fields, "px", &px_val) && px_val.kind == PICO8_VALUE_STRING) {
            pico8_ob_table_set_name(g_gb, px_val.as.string, pico8_value_ref(self));
        }
    }
    pico8_list_free(&gp_list);

    if (pico8_instance_get_field(self, "ii", &ii_val) && pico8_research_truthy(ii_val)) {
        pico8_value_t px_val;
        pico8_ob_table_t *inner = pico8_ob_table_new();
        pico8_ob_table_t *outer = pico8_ob_table_new();

        pico8_ob_table_set_index(inner, 1, ii_val);
        pico8_ob_table_set_index(inner, 2, pico8_value_fixed(pico8_fixed_from_int(8)));
        if (pico8_instance_get_field(self, "px", &px_val)) {
            pico8_ob_table_set_index(outer, 1, px_val);
        }
        pico8_ob_table_set_index(outer, 2, pico8_value_fixed(pico8_fixed_from_int(32)));
        pico8_ob_table_set_index(outer, 3, pico8_value_ref(inner));
        pico8_ob_table_set_name(self->fields, "lq", pico8_value_ref(outer));
    }

    if (pico8_instance_get_field(self, "i", &i_val) && i_val.kind == PICO8_VALUE_STRING && i_val.as.string != NULL) {
        i_src = i_val.as.string;
    }
    new_i = pico8_fi(i_src, pico8_hp_pq_i_step, NULL);
    new_i_heap = malloc(sizeof *new_i_heap);
    *new_i_heap = new_i;
    pico8_ob_table_set_name(self->fields, "i", pico8_value_ref(new_i_heap));
}

/* gb,rr={}
 * for ri in all(fd)do
 * if type(ri)=="string" then
 * rr=ri
 * else
 * ri.rr=rr
 * add(gb,hp(ri))end
 * end
 * (pico8/lua.lua:1434-1441.) See pico8_research.h's own top comment on
 * this declaration for the full trace. `fd`'s own positional values are
 * walked via this module's own pico8_research_positional_values (fd is a
 * pico8_ob_table_t, not a pico8_list_t) -- a fresh snapshot list, immune
 * to `g_gb`'s own concurrent growth (unlike walking `fd`'s raw `entries`
 * array directly, which would need to account for `fd` never itself being
 * mutated here -- `fd` stays read-only throughout this loop; only `ri`,
 * fd's OWN nested per-record tables, are mutated, via `ri.rr=rr`). */
void pico8_research_build_gb(const pico8_ob_table_t *fd) {
    pico8_list_t items;
    pico8_list_iter_t it;
    pico8_value_t entry;
    int next_index = 1;

    pico8_research_free_gb_instances();
    pico8_ob_table_free(g_gb);
    g_gb = pico8_ob_table_new();
    g_rr = NULL;

    items = pico8_research_positional_values(fd);
    it = pico8_list_all(&items);
    while (pico8_list_iter_next(&it, &entry)) {
        if (entry.kind == PICO8_VALUE_STRING) {
            g_rr = entry.as.string;
        } else if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
            pico8_ob_table_t *ri = entry.as.ref;
            pico8_instance_t *inst;

            pico8_ob_table_set_name(ri, "rr", g_rr != NULL ? pico8_value_string(g_rr) : pico8_value_ref(NULL));
            inst = pico8_hp_new(ri);
            pico8_ob_table_set_index(g_gb, next_index++, pico8_value_ref(inst));
        }
    }
    pico8_list_free(&items);
}

const pico8_ob_table_t *pico8_gb(void) {
    return g_gb;
}

/* Not a port of any named Lua function: `hc[key]`'s own polymorphic
 * FIXED-index-or-STRING-name lookup, matching pico8_ho_apply's own
 * established r[ri[i]] convention (pico8_collect.c) -- the same "the real
 * Lua key's type isn't known until runtime" split, applied here to `hc`. */
static int pico8_research_lookup(const pico8_ob_table_t *table, pico8_value_t key, pico8_value_t *out) {
    if (table == NULL) {
        return 0;
    }
    if (key.kind == PICO8_VALUE_FIXED) {
        return pico8_ob_table_get_index(table, (int)pico8_fixed_to_double(key.as.fixed), out);
    }
    if (key.kind == PICO8_VALUE_STRING && key.as.string != NULL) {
        return pico8_ob_table_get_name(table, key.as.string, out);
    }
    return 0;
}

/* function hp:po()local qp=hc[self.ja]
 * return self.il and oj(self.il)and hc[qp.mc]
 * or qp
 * end
 * (pico8/lua.lua:1422-1425.) See pico8_research.h's own top comment on
 * this declaration for the full trace. */
const pico8_ob_table_t *pico8_hp_po(const pico8_instance_t *self) {
    pico8_value_t ja_val;
    pico8_value_t qp_val;
    const pico8_ob_table_t *qp = NULL;
    pico8_value_t il_val;

    if (self == NULL) {
        return NULL;
    }
    if (pico8_instance_get_field(self, "ja", &ja_val) && pico8_research_lookup(pico8_hc(), ja_val, &qp_val) &&
        qp_val.kind == PICO8_VALUE_REF) {
        qp = qp_val.as.ref;
    }

    if (pico8_instance_get_field(self, "il", &il_val) && pico8_research_truthy(il_val) && pico8_oj(il_val) &&
        qp != NULL) {
        pico8_value_t mc_val;

        if (pico8_ob_table_get_name(qp, "mc", &mc_val)) {
            pico8_value_t hc_mc;

            if (pico8_research_lookup(pico8_hc(), mc_val, &hc_mc) && hc_mc.kind == PICO8_VALUE_REF) {
                return hc_mc.as.ref;
            }
        }
    }
    return qp;
}

/* function hp:od()local od=self.fj
 * if self.il and oj(self.il)then
 * od*=1.5
 * end
 * if not self.ii and oj("nanomaterials")then
 * od*=0.85
 * end
 * return flr(od)end
 * (pico8/lua.lua:1426-1433.) See pico8_research.h's own top comment on
 * this declaration for the full trace. */
pico8_fixed_t pico8_hp_od(const pico8_instance_t *self) {
    pico8_value_t fj_val;
    pico8_value_t il_val;
    pico8_value_t ii_val;
    pico8_fixed_t od = pico8_fixed_from_int(0);
    int has_ii;

    if (self != NULL && pico8_instance_get_field(self, "fj", &fj_val) && fj_val.kind == PICO8_VALUE_FIXED) {
        od = fj_val.as.fixed;
    }
    if (self != NULL && pico8_instance_get_field(self, "il", &il_val) && pico8_research_truthy(il_val) &&
        pico8_oj(il_val)) {
        od = pico8_fixed_mul(od, pico8_fixed_from_double(1.5));
    }
    has_ii = self != NULL && pico8_instance_get_field(self, "ii", &ii_val) && pico8_research_truthy(ii_val);
    if (!has_ii && pico8_oj(pico8_value_string("nanomaterials"))) {
        od = pico8_fixed_mul(od, pico8_fixed_from_double(0.85));
    }
    return pico8_fixed_flr(od);
}

