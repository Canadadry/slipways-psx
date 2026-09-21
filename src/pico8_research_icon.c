#include "pico8_research_icon.h"

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

/* See pico8_research_icon.h's own top comment for the full source/trace of
 * `ie`, the `self.ke`-as-class-wrapper resolution (including the `dl(self.
 * ke)` no-op finding), and the `self.ra=ie.gy` snapshot-vs-alias trace. */

static pico8_class_t *g_ie_class = NULL;

/* The shared, never-written, empty `fields` table every `ie_ke_wrapper()`
 * call below points its own transient wrapper's `fields` at -- see this
 * file's own `ie_ke_wrapper` comment for why one shared static table is
 * safe to reuse across every call (pico8_instance_get_field only ever
 * reads through `fields`, never writes it). */
static pico8_ob_table_t g_ie_empty_fields = {NULL, 0, 0};

/* Truncating fixed->int conversion, the same private per-module copy
 * convention already established throughout this port (e.g. pico8_bg.c's
 * own pico8_bg_to_int, pico8_probe.c's own pico8_probe_to_int). */
static int ie_to_int(pico8_fixed_t x) {
    return (int)pico8_fixed_to_double(x);
}

/* Real Lua truthiness (nil/false falsy, everything else -- including fixed
 * 0 -- truthy), ALSO treating this port's own `pico8_value_ref(NULL)`
 * "absent" filler as falsy -- the same widened rule pico8_probe.c's own
 * `pico8_probe_truthy` already establishes, needed here for `self.kh`
 * (`ie:hb`/`ie:kj`) and `self.m_` (`ie:kj`). */
static int ie_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* This module's own private two-piece concat helper -- the same shape as
 * pico8_score_panel.c's own established `pj_concat2` (that file's own top
 * comment already documents this exact per-module-duplicate convention),
 * needed here for `ie:ldown`'s own `"s"..ie.id`. Returns a newly
 * heap-allocated string the caller owns and must free(). */
static char *ie_concat2(const char *a, const char *b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char *out = malloc(la + lb + 1);
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = '\0';
    return out;
}

/* Reads `self.ke` -- a real per-INSTANCE field, set at construction by
 * whoever built this `ie` instance (Chunk 20's own `ie({ke=b,...})`), and
 * genuinely a CLASS pointer, not an instance -- see this file's own header
 * top comment for the full representation trace. Returns NULL if `self` is
 * NULL or `self.ke` is absent/wrong-kind (defensive; every real `ie`
 * instance always has a real `self.ke`, so this never actually fires on a
 * real instance). */
static pico8_class_t *ie_self_ke_class(const pico8_instance_t *self) {
    pico8_value_t ke_val;
    if (self != NULL && pico8_instance_get_field(self, "ke", &ke_val) && ke_val.kind == PICO8_VALUE_REF &&
        ke_val.as.ref != NULL) {
        return (pico8_class_t *)ke_val.as.ref;
    }
    return NULL;
}

/* Builds the classless `pico8_instance_t` wrapper this file's own header
 * comment describes in full (`cls=self.ke`, `fields=`the one shared,
 * never-written empty table above) -- a small, transient, STACK value with
 * no heap allocation and no teardown of its own, unlike `mz.jj`'s own
 * necessarily-persistent wrapper (pico8_wormhole.c): different `ie`
 * instances hold different classes in their own `self.ke`, so this cannot
 * be built once and shared the way `mz.jj`'s single, always-the-same-table
 * wrapper can. Used by both `ie:hb`'s own `self.ke.rr` read, `ie:kj`'s own
 * `self.ke.s` read, and `ie:kj`'s own `dl(self.ke)` call. */
static pico8_instance_t ie_ke_wrapper(pico8_class_t *cls) {
    pico8_instance_t wrapper;
    wrapper.fields = &g_ie_empty_fields;
    wrapper.cls = cls;
    return wrapper;
}

/* Combines the two helpers above: reads `self.ke.<name>` through the
 * classless wrapper's own class-chain walk (`pico8_instance_get_field`'s
 * usual own-fields-then-`cls`-defaults-then-`cls->parent`-defaults
 * fallback). Returns 0 (absent, `*out` untouched) if `self.ke` itself is
 * absent/wrong-kind. */
static int ie_ke_field(const pico8_instance_t *self, const char *name, pico8_value_t *out) {
    pico8_class_t *cls = ie_self_ke_class(self);
    pico8_instance_t wrapper;
    if (cls == NULL) {
        return 0;
    }
    wrapper = ie_ke_wrapper(cls);
    return pico8_instance_get_field(&wrapper, name, out);
}

/* This module's own private "read `ie`'s OWN class-level field `name`"
 * helper -- the same "read a class's own default field directly" idiom
 * `pj_class_field_ref_table` already establishes (pico8_score_panel.c,
 * Chunk 18), here returning the raw `pico8_value_t` (rather than narrowing
 * to a REF-to-table) since `ie.gy`/`ie.id` are read for their own value,
 * not walked as a nested table. Returns `pico8_value_ref(NULL)` (this
 * port's own "absent" filler) if `ie`'s class isn't built yet or the named
 * field genuinely has no class-level default (`ie.gy`, before Chunk 22's
 * own real `_init()` sets it for the first time -- see this file's own
 * header top comment). */
static pico8_value_t ie_class_field_value(const char *name) {
    pico8_value_t v;
    if (g_ie_class != NULL && g_ie_class->defaults != NULL && pico8_ob_table_get_name(g_ie_class->defaults, name, &v)) {
        return v;
    }
    return pico8_value_ref(NULL);
}

/* `ie.id` narrowed to fixed, defaulting to 0 -- matching this port's
 * pervasive "absent numeric field reads as 0" convention (needed here only
 * as a defensive fallback; `ie.id`'s own real class-spec default is
 * already fixed 0, so this only matters if `ie`'s class isn't built yet). */
static pico8_fixed_t ie_class_field_fixed(const char *name) {
    pico8_value_t v = ie_class_field_value(name);
    if (v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

/* Defensive vector-value reader -- the same shape as pico8_probe.c's own
 * `pico8_probe_value_vec`/pico8_bg.c's own `pico8_bg_value_vec`: a missing
 * or wrong-kind value reads as the zero vector. Used for `ie.gy`'s own
 * ARITHMETIC read (the `-=` itself) -- see this file's own header top
 * comment for why the SEPARATE, exact-copy `self.ra,self.kh=ie.gy,true`
 * assignment does NOT go through this helper (it needs the raw value,
 * REF-and-all, not a defensively-defaulted vec). */
static pico8_vec_t ie_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t ie_class_field_vec(const char *name) {
    return ie_value_vec(ie_class_field_value(name));
}

/* function ie:hb()if not self.kh and oj(self.ke.rr)then
 * self.ra,self.kh=
 * ie.gy,true
 * ie.gy-=v(10,0)
 * end
 * end
 * (pico8/lua.lua:923-928.) See pico8_research_icon.h's own comment on this
 * declaration for the full trace of the `self.ke.rr` class-wrapper read and
 * the `self.ra=ie.gy` snapshot-vs-alias resolution. `not self.kh` short-
 * circuits `oj(self.ke.rr)` exactly as real Lua's own `and` does -- once
 * `self.kh` is already truthy (a prior `hb()` call already claimed a
 * reveal position), this function is a no-op on every later call, ported
 * as a plain early return before `self.ke`/`oj` are even touched. */
void pico8_ie_hb(pico8_instance_t *self) {
    pico8_value_t kh_val;
    pico8_value_t rr_val = pico8_value_ref(NULL);

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "kh", &kh_val) && ie_truthy(kh_val)) {
        return;
    }

    /* `self.ke.rr` -- absent (nil) if `self.ke` names `qh`/`os` (neither
     * ever sets `rr` anywhere in their own class chain), leaving `rr_val`
     * at its own `pico8_value_ref(NULL)` default; `oj`'s own `not px`
     * short-circuit then makes `oj(nil)` unconditionally true. */
    ie_ke_field(self, "rr", &rr_val);

    /* `oj` (Chunk 11's pico8_oj, pico8_tech_gate.h) already internally
     * guards `pico8_fm()!=NULL` -- called directly, no extra guard here. */
    if (!pico8_oj(rr_val)) {
        return;
    }

    /* `self.ra,self.kh=ie.gy,true` -- a real Lua parallel assignment: both
     * writes use values read BEFORE either write happens (matching this
     * port's own established multiple-assignment-ordering care, e.g.
     * Chunk 9's particle-rotation note). `gy_snapshot` is the RAW value
     * `ie.gy` currently holds (a REF aliasing the SAME heap vector every
     * other reader of `ie.gy` currently sees, or `pico8_value_ref(NULL)`
     * if `ie.gy` is still genuinely absent) -- copied byte-for-byte into
     * `self.ra`, exactly mirroring real Lua's "assigning a table value
     * copies the reference" semantics. See this file's own header top
     * comment for why the LATER `ie.gy-=v(10,0)` rebind below does not
     * retroactively change what this snapshot already captured. */
    {
        pico8_value_t gy_snapshot = ie_class_field_value("gy");
        pico8_ob_table_set_name(self->fields, "ra", gy_snapshot);
        pico8_ob_table_set_name(self->fields, "kh", pico8_value_bool(1));
    }

    /* `ie.gy-=v(10,0)` -- a SEPARATE statement, run strictly after the
     * assignment above. `pico8_vec_sub` is pure (returns a fresh value,
     * mutates neither operand -- confirmed by reading its own current
     * body), so this allocates a BRAND NEW heap vector for the result and
     * REBINDS `ie.gy`'s own class-defaults slot to point at it, leaving
     * whatever heap vector `self.ra` already captured a REF to completely
     * untouched -- the same "rebind, never mutate the old heap value in
     * place" convention this port already uses for every other vector
     * field write (e.g. `pico8_bv`'s own `self.p`). `current` defensively
     * reads `ie.gy` as the zero vector if it is still genuinely absent
     * (true throughout this chunk's own scope, until Chunk 22's real
     * `_init()` lands -- see this file's own header top comment for why
     * that is a "compiles safely today" situation, not a real gap). */
    {
        pico8_vec_t current = ie_class_field_vec("gy");
        pico8_vec_t delta = pico8_vec_v(pico8_fixed_from_int(10), pico8_fixed_from_int(0));
        pico8_vec_t *heap = malloc(sizeof *heap);
        *heap = pico8_vec_sub(current, delta);
        pico8_ob_table_set_name(g_ie_class->defaults, "gy", pico8_value_ref(heap));
    }
}

/* Bridges to pico8_ie_hb's own signature; a real `bm` name, wired on
 * `ie`'s own class defaults below. `ie:hb()` declares no Lua parameters at
 * all beyond `self`, so every one of `argc`/`arg0`/`arg1` is silently
 * dropped, matching e.g. `pico8_pj_hb_dispatch`'s own identical shape. */
static pico8_value_t pico8_ie_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_ie_hb((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* function ie:ldown(mp)
 * ie.id+=1
 * return self.ke({id="s"..ie.id
 * })end
 * (pico8/lua.lua:929-933.) See pico8_research_icon.h's own comment on this
 * declaration for the full trace. `self.ke` is called directly as a
 * constructor here (`pico8_class_instantiate`), not looked up by name --
 * unlike `jo:ldown`'s own `jc({...})`, `self.ke` already IS the real class
 * value, not a name to resolve. */
pico8_value_t pico8_ie_ldown(pico8_instance_t *self, pico8_value_t mp) {
    pico8_fixed_t new_id;
    pico8_class_t *ke_cls;
    char *id_num_str;
    char *id_str;
    pico8_ob_table_t *overrides;
    pico8_instance_t *made;

    /* `mp` (the click's own world-space mouse position) is declared, per
     * this port's literal-signature-fidelity convention, but genuinely
     * unused by the real body. */
    (void)mp;

    /* `ie.id+=1` -- a plain class-level counter increment. */
    new_id = pico8_fixed_add(ie_class_field_fixed("id"), pico8_fixed_from_int(1));
    pico8_ob_table_set_name(g_ie_class->defaults, "id", pico8_value_fixed(new_id));

    ke_cls = ie_self_ke_class(self);
    if (ke_cls == NULL) {
        return pico8_value_ref(NULL);
    }

    /* `"s"..ie.id` -- the NEW (already-incremented) counter value, as a
     * fresh, heap-allocated string. */
    id_num_str = pico8_value_tostr(pico8_value_fixed(new_id));
    id_str = ie_concat2("s", id_num_str);
    free(id_num_str);

    /* `self.ke({id="s"..ie.id})`: a fresh one-field override table, then a
     * direct pico8_class_instantiate() call -- `overrides` itself is freed
     * right after (pico8_class_instantiate shallow-COPIES each entry into
     * the new instance's own fields, so the temporary table is safe to
     * free here); `id_str` itself is NOT freed -- it is now retained,
     * as-is, inside the newly constructed instance's own "id" field,
     * matching this port's pervasive "a freshly built string is stored
     * directly, not deep-copied again" convention (e.g. `pico8_pj_hb`'s
     * own `ou[i]` fields). */
    overrides = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides, "id", pico8_value_string(id_str));
    made = pico8_class_instantiate(ke_cls, overrides);
    pico8_ob_table_free(overrides);

    return pico8_value_ref(made);
}

/* Bridges to pico8_ie_ldown's own signature; NOT a `bm` name, reached only
 * through `mv()`'s own unrestricted by-name field lookup (`ui:bh()`'s own
 * real `self:jz("ldown",self.qa,je.ra)` call site, argc=2) -- matching
 * `pico8_bg_ldown_dispatch`'s own identical "not bm, still a real
 * PICO8_VALUE_FUNCTION field" shape. `arg0` (`mp`) is forwarded; `arg1`
 * (`rp`) is unused (matching `ie:ldown`'s own declared single parameter). */
static pico8_value_t pico8_ie_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg1;
    return pico8_ie_ldown((pico8_instance_t *)self, arg0);
}

/* function ie:kj(p)if self.kh then
 * palt(3,false)spr(self.ke.s,p.x,p.y)if self.m_ then
 * dl(self.ke)self.jb:ol(0,0)end
 * palt(3,true)end
 * end
 * (pico8/lua.lua:934-937.) See pico8_research_icon.h's own comment on this
 * declaration for the full trace, including why `dl(self.ke)` is a REAL,
 * evidenced call here (draws the class-level build-cost tooltip), not the
 * no-op a shallower reading might predict. */
void pico8_ie_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p) {
    pico8_value_t kh_val;

    /* `if self.kh then...end` wraps the whole function -- a genuinely
     * absent/falsy `self.kh` (never yet "revealed" by `ie:hb`) makes this a
     * total no-op. */
    if (self == NULL || !pico8_instance_get_field(self, "kh", &kh_val) || !ie_truthy(kh_val)) {
        return;
    }
    if (gfx == NULL || mem == NULL) {
        return;
    }

    pico8_palt(mem, 3, 0);

    /* `spr(self.ke.s,p.x,p.y)` -- `self.ke.s`, whichever planet-upgrade
     * class's own sprite id (`self.ke.rr`'s own class-wrapper read, see
     * this file's own header top comment). */
    {
        pico8_value_t s_val;
        int s_id = 0;
        if (ie_ke_field(self, "s", &s_val) && s_val.kind == PICO8_VALUE_FIXED) {
            s_id = ie_to_int(s_val.as.fixed);
        }
        pico8_spr_default(gfx, mem, s_id, ie_to_int(p.x), ie_to_int(p.y));
    }

    /* `if self.m_ then dl(self.ke) self.jb:ol(0,0) end` -- see this file's
     * own header top comment for the full evidenced trace of why
     * `dl(self.ke)` genuinely draws a real tooltip here (not a no-op). */
    {
        pico8_value_t m_val;
        if (pico8_instance_get_field(self, "m_", &m_val) && ie_truthy(m_val)) {
            pico8_class_t *ke_cls = ie_self_ke_class(self);
            pico8_value_t jb_val;

            if (ke_cls != NULL) {
                pico8_instance_t wrapper = ie_ke_wrapper(ke_cls);
                pico8_dl(gfx, mem, &wrapper);
            }
            /* `self.jb:ol(0,0)` -- unlike `self.ke`, `self.jb` really is
             * stored as a real `jb`-class INSTANCE ref, an ordinary
             * `pico8_instance_get_field` read, no representation trick
             * needed. */
            if (pico8_instance_get_field(self, "jb", &jb_val) && jb_val.kind == PICO8_VALUE_REF &&
                jb_val.as.ref != NULL) {
                pico8_jb_ol(gfx, mem, (const pico8_instance_t *)jb_val.as.ref, pico8_fixed_from_int(0),
                            pico8_fixed_from_int(0));
            }
        }
    }

    pico8_palt(mem, 3, 1);
}

/* Bridges to pico8_ie_kj's own signature, reading gfx/mem from the shared
 * dispatch context -- the same split established by `pico8_pj_kj_dispatch`/
 * `pico8_ga_kj_dispatch`. A real `bm` name; `dc("kj")`'s own real
 * `qc[nt](qc,qc.ra)` call shape means `arg0` is always `self.ra` -- read as
 * a REF-to-vec, matching every other `kj` dispatch bridge in this port. */
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

/* `ie.hd=gk` -- see pico8_research_icon.h's own comment on this
 * declaration for the full trace. Replicates `pico8_jo_hd_dispatch`'s
 * exact shape verbatim. */
static pico8_value_t pico8_ie_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_gk((const pico8_instance_t *)self, arg0, arg1);
}

/* ie=ke:jr([[
 * kq=b(0,0,9,8),
 * fa=12,
 * id=0,
 * ra=v(999,999),
 * eh=v(118,120),
 * ]])ie.hd=gk
 * (pico8/lua.lua:919-924.) See pico8_research_icon.h's own top comment for
 * the full trace. `pico8_pi_init()` is called defensively first
 * (idempotent) since this spec's own nested `v(...)` (`ra`,`eh`) and `b(...)`
 * (`kq`) constructors need `pi['v']`/`pi['b']` already registered, the same
 * established convention every other class with a nested-constructor spec
 * already uses. */
void pico8_research_icon_init(void) {
    pico8_research_icon_shutdown();

    pico8_pi_init();

    g_ie_class =
        pico8_class_declare(pico8_ke_class(), "\nkq=b(0,0,9,8),\nfa=12,\nid=0,\nra=v(999,999),\neh=v(118,120),\n");

    pico8_ob_table_set_name(g_ie_class->defaults, "hd", pico8_value_function(pico8_ie_hd_dispatch));
    pico8_ob_table_set_name(g_ie_class->defaults, "hb", pico8_value_function(pico8_ie_hb_dispatch));
    pico8_ob_table_set_name(g_ie_class->defaults, "kj", pico8_value_function(pico8_ie_kj_dispatch));
    pico8_ob_table_set_name(g_ie_class->defaults, "ldown", pico8_value_function(pico8_ie_ldown_dispatch));
}

/* Not part of the Lua original (no manual teardown under GC) -- releases
 * `ie`'s own class (its `defaults` table, including whatever heap vector
 * `ie.gy` may have been rebound to by a prior `hb()` call -- freed via
 * pico8_class_free, matching this port's pervasive "nested class-spec
 * structures are never explicitly deep-freed beyond the class's own
 * defaults table" convention). */
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
