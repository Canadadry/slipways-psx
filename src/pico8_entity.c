/* pico8/lua.lua's entity registration/dispatch mechanism (Chunk 10) --
 * ported literally, per this chunk's own brief: no per-function unit test
 * for this chunk, verified by close source/port comparison plus Chunk
 * 11's smoke test instead. The exact source for each piece, read
 * character by character directly from pico8/lua.lua:
 *
 * ft(rc,nt,pu) (lines 3-4, also used directly by dc() below):
 *   function ft(rc,nt,pu)rc[nt]=rc[nt]or {}
 *   add(rc[nt],pu)end
 * -- "get-or-create the list at rc[nt], then append pu to it." This has
 * no single standalone C port of its own; every real call site in scope
 * (lt/he's `ft(cv,p,e)`, dc's `ft(fl,qc.fa,qc)`) is ported inline below,
 * since "rc" is a different concrete collection (cv, a fixed-name array;
 * cg, a growable-by-tag array; dc's own local per-layer buckets) at each
 * call site, and this port already has dedicated storage/helpers for
 * each of those shapes rather than one generic "table of tables" they'd
 * otherwise need to be routed through.
 *
 * mv(e,rm,...) (lines 5-9):
 *   function mv(e,rm,...)local fn=e and e[rm]
 *   if type(fn)=="function" then
 *   return fn(e,...)end
 *   return fn
 *   end
 * -- `e and e[rm]`: if `e` itself is falsy (nil/false), `fn` is that same
 * falsy value and `e[rm]` is never even evaluated; otherwise `fn` is
 * whatever `e[rm]` resolves to (through the class-chain `__index` fallback
 * discussed in pico8_class.c). If `fn` is a function, call it with `e`
 * plus every extra argument mv() itself received, and return the result;
 * otherwise just return `fn` as-is (which is Lua nil when `e` had no such
 * field at all). Every real mv()-equivalent dispatch call site inside
 * this chunk's (Chunk 10's) OWN scope -- dc()'s `entity[nt](entity,entity.ra)`
 * (line 196, not literally spelled "mv(...)" but the exact same
 * "resolve-then-call-with-self-plus-one-arg" shape the PRD's own "dc/mv"
 * decision explicitly says to route through this same mechanism) and
 * `mo:md(ci[o])` (line 194) -- passes exactly ONE extra argument, never
 * more, never a variable count, so Phase 2's own pico8_mv() took exactly
 * one extra argument (`arg`, a plain pico8_value_t, not a variadic list),
 * per Chunk 10's own explicit instruction to build only what was actually
 * called within its own scope rather than a speculative generic variadic
 * dispatcher.
 *
 * UPDATE, Phase 3 Chunk 5 (docs/prd/triage/phase3-etape1-function-by-
 * function-c-port.md): mv() is also called elsewhere in pico8/lua.lua
 * (lines 228, 244, 245, 250, 261, 277, 327, 419, 548, 589) with 0-2 extra
 * arguments -- real Phase 3 game code, out of Chunk 10's own original
 * scope, but squarely in Chunk 5's own scope (the `ui` class, the first
 * real 0- and 2-argument mv() call site). Chunk 5 re-grepped every real
 * `mv(` call site across the WHOLE cart and confirmed the maximum extra-
 * argument count is 2, never 3+ -- see pico8_value.h's own comment on
 * pico8_value_function_t for the full confirmation trail. pico8_mv() (and
 * pico8_value_function_t) are therefore extended below to a small
 * FIXED-size argument list (`argc` plus `arg0`/`arg1`), not a variadic
 * one; this chunk's (Chunk 10's) own two call sites below keep working
 * unchanged by passing argc=1 with `arg1` an unused filler. "Is a
 * function" is PICO8_VALUE_FUNCTION-kind (pico8_value.h) instead of Lua's
 * `type(fn)=="function"` check. `e and e[rm]` being nil when `e` is nil is
 * mirrored by treating a NULL `e` as "absent" up front, before even
 * attempting the field lookup -- a real pico8_instance_t* is never itself
 * "falsy" any other way in this port, so NULL is the only case that
 * matters here.
 *
 * jl()'s entity-registry init (lines 153-155, this chunk's own narrow
 * slice of it):
 *   function jl()hv,cv,cg={},{},{}
 *   mo,je,ok=
 *   qb(),nf(),ui()end
 * -- `hv,cv,cg={},{},{}`: three fresh empty tables. Ported as
 * pico8_entity_init()'s own hv/cv/cg (re)initialization below. `mo=qb()`
 * assigns the REAL camera/palette singleton (`qb`, a whole class defined
 * later in the cart) to `mo` -- entirely Phase 3 work. This chunk instead
 * builds `mo` as its own bare placeholder (see this file's own comment on
 * pico8_entity_init()'s mo construction further down) purely so dc()'s
 * literal `mo:md(ci[o])` line compiles/links, per the PRD's explicit
 * "ci et le placeholder mo global" scope carve-out. `je=nf()`/`ok=ui()`
 * are likewise real Phase 3 singletons, wholly out of this chunk's scope,
 * and are not ported here at all (not even as placeholders -- nothing in
 * this chunk's own scope ever references `je`/`ok`, unlike `mo`, which
 * dc()'s own literal body does reference).
 *
 * lt(e) (lines 156-162):
 *   function lt(e)add(hv,e)for p in all(bm)do
 *   if (e[p])ft(cv,p,e)
 *   end
 *   for t in all(e.nu)do
 *   ft(cg,t,e)end
 *   return e
 *   end
 * -- add `e` to `hv`; for every name `p` in `bm` (see bm's own source
 * below), if `e[p]` is present, append `e` to `cv[p]` (get-or-create'd via
 * ft -- but cv's own 5 buckets are pre-created once at init time in this
 * port, per the PRD's "a small FIXED set of named lists" wording, so this
 * port's version of that get-or-create is really just "look the
 * already-existing bucket up by name"); for every tag `t` in `e.nu` (a
 * list of tag strings, e.g. `nu=o("jo","np","prober")` at real call
 * sites -- see pico8_entity.h's own comment on `.nu`'s shape), append `e`
 * to `cg[t]` (get-or-create'd via ft -- cg's tag universe is NOT bounded
 * ahead of time, so this port really does create a new bucket here, on
 * first use of a given tag, per the PRD's "cg... grown on demand"
 * wording). `if (e[p])`: per this chunk's own explicit instruction, ported
 * as a field-PRESENCE check (pico8_instance_get_field's own found/absent
 * distinction), not a Lua-truthiness check -- so a field explicitly set to
 * `false` would still count as "present" here, diverging from Lua's own
 * `if(e[p])` (in real Lua, only `nil` and `false` are falsy -- unlike
 * C/Python/JS, `0` is truthy). Every real bm field (`kj`/`ev`/`hd`/`hk`/
 * `hb`) holds a METHOD when set at any real class-declaration call site
 * found in pico8/lua.lua, never `false` or any other value, so
 * presence-vs-truthiness never actually differs for these specific
 * fields -- this divergence is not evidenced to change any real entity's
 * cv membership -- it is a deliberate scope decision (per this chunk's own
 * "presence distinct from present-but-false/zero" instruction), not an
 * oversight.
 *
 * he(e) (lines 163-168), the exact del()-based mirror of lt():
 *   function he(e)del(hv,e)for p in all(bm)do
 *   if (e[p])del(cv[p],e)
 *   end
 *   for t in all(e.nu)do
 *   del(cg[t],e)end
 *   end
 * -- same walk as lt(), removing (by value, via pico8_list_del) from hv,
 * cv[p] and cg[t] instead of appending, and not returning anything (`he`
 * has no `return` statement, so it's void). `del(cg[t],e)` when `t` was
 * never lt()'d as a tag before (cg[t] itself nil) would error in real Lua
 * (indexing a nil table); since every real call site pairs a given
 * entity's he() with an earlier lt() of that SAME entity (which would
 * already have created cg[t] for every tag in e.nu), this never actually
 * arises in evidenced usage -- this port defensively looks the bucket up
 * and silently does nothing if it's missing, rather than reproducing a
 * crash no known real call site ever triggers.
 *
 * bm (lines 169-172), the field-name list lt/he walk:
 *   bm=ob([[
 *   "kj","ev",
 *   "hd",
 *   "hk","hb",
 *   ]])
 * -- a purely positional ob() spec, 5 quoted-string values: "kj", "ev",
 * "hd", "hk", "hb" (confirmed directly from this exact source text, not
 * assumed from the PRD's own prose gloss on it, which quotes the same 5
 * names in the same order). Ported as a plain, fixed 5-element C array of
 * name strings (g_bm_names below), not re-run through pico8_ob() itself:
 * the PRD's own "hv/cv/cg, lt, he" decision explicitly says cv's shape
 * needs only "a linear array of {name, list} pairs... no hash map", and
 * bm's own 5 names are exactly that array's keys -- routing them through
 * pico8_ob() first would add a whole extra owned-table's worth of
 * bookkeeping (and matching cleanup) to produce the same 5 constant C
 * string literals this comment already transcribes directly and
 * verifiably from the real source above.
 *
 * ci (line 187):
 *   ci=ob([[0.5,1,1,1,1,1,1,1,1,0,0,0,0,0,0,]])
 * -- a purely positional, purely numeric ob() spec: 15 fixed-point values,
 * 0.5 then fourteen more (1 x8, then 0 x6) -- confirmed directly from this
 * exact source text. Ported by actually calling pico8_ob() on this exact
 * literal (see pico8_entity_init() below) rather than hand-building the
 * 15 values in C: unlike bm above, ci is genuinely "just data" with no
 * cv-shaped bookkeeping to avoid duplicating, so routing it through the
 * real, already-tested ob() parser (applying its own 0xffff.fffe number
 * masking, for whatever that's worth on these particular round values) is
 * the more literal, lower-risk port of the two available here.
 *
 * dc(nt) (lines 188-199):
 *   function dc(nt)
 *   local fl={}
 *   for _,qc in pairs(cv[nt])do
 *   ft(fl,qc.fa,qc)
 *   end
 *   for o=1,15 do
 *   if (nt=="kj" and fl[o])mo:md(ci[o])
 *   for _,qc in pairs(fl[o]or {})do
 *   qc[nt](qc,qc.ra)
 *   end
 *   end
 *   end
 * -- group every entity registered under `cv[nt]` into 15 buckets `fl[1..
 * 15]` keyed by each entity's own `.fa` (an integer 1-15 layer/priority);
 * then, for `o` from 1 to 15 in order: if `nt=="kj"` AND bucket `o` is
 * non-empty, call `mo:md(ci[o])` (Lua's `:` method-call sugar, which reads
 * `mo.md` through the class-chain fallback exactly like `e[rm]` above and
 * calls it with `mo` as the implicit first argument and `ci[o]` as the
 * second -- ported via the SAME pico8_mv() mechanism as every other
 * generic dispatch in this file, per this chunk's own explicit
 * instruction, rather than a direct, unchecked field-call that would
 * crash outright since `mo` never has an `md` field set); then, for every
 * entity `qc` in bucket `o`, call `qc[nt](qc,qc.ra)` (again ported via
 * pico8_mv()). `qc.fa`/`qc.ra` absent (nil in Lua) would make
 * `fl[nil]=...`/`qc[nt](qc,nil)` respectively -- the former genuinely
 * errors in real Lua (assigning a table with a nil key), the latter is a
 * perfectly ordinary "pass nil as the second argument" call. This port
 * treats a `.fa`-less entity as dropped from every bucket (a defensive,
 * non-crashing divergence -- see pico8_dc()'s own comment below) and a
 * `.ra`-less entity as receiving `pico8_value_ref(NULL)` (this port's
 * chosen C stand-in for Lua nil as a call argument -- there is no
 * separate "nil" kind in pico8_value_t, see pico8_value.h's own top
 * comment on why REF is the closest existing analog: a reference that
 * points at nothing). The `"kj"` special case is ported intact, not
 * stripped, exactly as the PRD's "dc/mv" decision requires -- it is
 * simply never exercised by Chunk 11's own smoke test (which never calls
 * dc("kj")), so `mo` (this file's own placeholder, see
 * pico8_entity_init()'s comment) never actually needs a real `md` method
 * for anything in this phase's own scope to pass. */

#include "pico8_entity.h"

#include <stdlib.h>
#include <string.h>

/* bm (pico8/lua.lua:169-172) -- see this file's own top comment for why
 * this is a plain constant array rather than an actual pico8_ob() call.
 * Fixed cardinality, so cv (below) is a fixed-size array in parallel,
 * never grown/reallocated -- pointers returned by pico8_cv() stay valid
 * for the module's whole lifetime, unlike pico8_cg()'s own growable
 * storage (see its own comment further down). */
#define PICO8_BM_COUNT 5
static const char *const g_bm_names[PICO8_BM_COUNT] = {"kj", "ev", "hd", "hk", "hb"};

typedef struct {
    const char *name; /* borrowed from g_bm_names -- not owned here */
    pico8_list_t list;
} pico8_cv_entry_t;

static pico8_cv_entry_t g_cv[PICO8_BM_COUNT];

/* cg (pico8/lua.lua:152's `cg={}`, grown by lt() on line 160's
 * `ft(cg,t,e)`): unlike cv above, cg's tag universe isn't known ahead of
 * time, so this is a genuinely growable array of {tag, list} pairs, each
 * tag an owned copy (pico8_entity_strdup below) since a tag string
 * arrives borrowed from an entity's own `.nu` table, which this module
 * doesn't keep alive past the lt() call that reads it. A linear scan is
 * enough here too: this phase's own scope (a smoke test's 1-2 fake
 * entities) never has more than a handful of distinct tags -- matching
 * every other small, bounded collection in this phase (pico8_class.c's
 * own class registry, pico8_ob.c's constructor table notwithstanding). */
typedef struct {
    char *tag;
    pico8_list_t list;
} pico8_cg_entry_t;

static pico8_cg_entry_t *g_cg = NULL;
static int g_cg_count = 0;
static int g_cg_capacity = 0;

static pico8_list_t g_hv;
static pico8_ob_table_t *g_ci = NULL;
static pico8_instance_t *g_mo = NULL;
static int g_initialized = 0;

/* Not strdup() (POSIX, not C99 -- this codebase's build is -std=c99), and
 * a private per-module copy rather than exporting one of the several
 * other modules' own identical private copies, matching pico8_class.c's
 * and pico8_ob.c's own stated reasoning for doing the same. */
static char *pico8_entity_strdup(const char *s) {
    size_t n = strlen(s);
    char *out = malloc(n + 1);
    memcpy(out, s, n + 1);
    return out;
}

static pico8_cg_entry_t *pico8_cg_find(const char *tag) {
    int i;
    for (i = 0; i < g_cg_count; i++) {
        if (strcmp(g_cg[i].tag, tag) == 0) {
            return &g_cg[i];
        }
    }
    return NULL;
}

/* Get-or-create (`ft(cg,t,e)`'s own `rc[nt]=rc[nt] or {}` half): returns
 * the existing bucket for `tag` if one was already created by an earlier
 * lt() call, else grows g_cg (realloc, doubling capacity) and creates a
 * fresh empty one. Callers must not hold onto the returned pointer across
 * a LATER call to this function that grows g_cg -- exactly the same
 * caveat pico8_ob_table_t's own entries array already carries for its own
 * realloc-based growth (pico8_ob.h). pico8_cg() (this module's own public
 * accessor) re-derives its pointer fresh on every call for this reason. */
static pico8_cg_entry_t *pico8_cg_find_or_create(const char *tag) {
    pico8_cg_entry_t *found = pico8_cg_find(tag);

    if (found != NULL) {
        return found;
    }

    if (g_cg_count == g_cg_capacity) {
        int new_capacity = g_cg_capacity == 0 ? 4 : g_cg_capacity * 2;
        pico8_cg_entry_t *new_entries = realloc(g_cg, (size_t)new_capacity * sizeof *new_entries);
        if (new_entries == NULL) {
            abort();
        }
        g_cg = new_entries;
        g_cg_capacity = new_capacity;
    }

    g_cg[g_cg_count].tag = pico8_entity_strdup(tag);
    pico8_list_init(&g_cg[g_cg_count].list);
    g_cg_count++;
    return &g_cg[g_cg_count - 1];
}

/* Not part of the Lua original (no manual teardown under GC) -- see
 * pico8_entity.h's own comment on this declaration for what is and isn't
 * freed here. Safe to call more than once, and safe to call before the
 * first pico8_entity_init(). */
void pico8_entity_shutdown(void) {
    int i;

    if (!g_initialized) {
        return;
    }

    pico8_list_free(&g_hv);

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        pico8_list_free(&g_cv[i].list);
        g_cv[i].name = NULL;
    }

    for (i = 0; i < g_cg_count; i++) {
        pico8_list_free(&g_cg[i].list);
        free(g_cg[i].tag);
    }
    free(g_cg);
    g_cg = NULL;
    g_cg_count = 0;
    g_cg_capacity = 0;

    pico8_ob_table_free(g_ci);
    g_ci = NULL;

    pico8_instance_free(g_mo);
    g_mo = NULL;

    g_initialized = 0;
}

/* jl()'s own `hv,cv,cg={},{},{}` (pico8/lua.lua:153), narrowed to this
 * chunk's own scope -- see this file's top comment. Also builds ci (line
 * 187) and this chunk's own placeholder mo (NOT `mo=qb()`, line 154 --
 * see pico8_entity.h's own comment on pico8_mo() for why a bare,
 * classless instance is this port's deliberate choice here: a real
 * pico8_class_t would need registering by name in pico8_class.c's own
 * shared, global registry, which risks colliding with whatever name(s)
 * Chunk 11's own fake test classes register under (most of them "",
 * per pico8_class.c's own top comment on how many real :jr() call sites
 * ever set `fz`) -- a bare instance sidesteps that entirely, and nothing
 * about mo's own placeholder role needs a class at all: it has no
 * fields, no parent to fall back to, and no pq constructor to run). */
void pico8_entity_init(void) {
    int i;

    pico8_entity_shutdown();

    pico8_list_init(&g_hv);

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        g_cv[i].name = g_bm_names[i];
        pico8_list_init(&g_cv[i].list);
    }

    g_ci = pico8_ob("0.5,1,1,1,1,1,1,1,1,0,0,0,0,0,0,", NULL);

    g_mo = malloc(sizeof *g_mo);
    g_mo->tag = PICO8_TYPE_TAG_INSTANCE;
    g_mo->fields = pico8_ob_table_new();
    g_mo->cls = NULL;

    g_initialized = 1;
}

const pico8_list_t *pico8_hv(void) {
    return &g_hv;
}

const pico8_list_t *pico8_cv(const char *name) {
    int i;
    for (i = 0; i < PICO8_BM_COUNT; i++) {
        if (strcmp(g_cv[i].name, name) == 0) {
            return &g_cv[i].list;
        }
    }
    return NULL;
}

const pico8_list_t *pico8_cg(const char *tag) {
    pico8_cg_entry_t *entry = pico8_cg_find(tag);
    return entry != NULL ? &entry->list : NULL;
}

/* lt(e) (pico8/lua.lua:156-162) -- see this file's own top comment for
 * the full line-by-line trace. */
pico8_instance_t *pico8_lt(pico8_instance_t *e) {
    int i;
    pico8_value_t discard;
    pico8_value_t nu_val;

    /* add(hv,e) */
    pico8_list_add(&g_hv, pico8_value_ref(e));

    /* for p in all(bm) do if (e[p]) ft(cv,p,e) end -- presence, not
     * truthiness (see this file's own top comment on this deliberate
     * choice). */
    for (i = 0; i < PICO8_BM_COUNT; i++) {
        if (pico8_instance_get_field(e, g_cv[i].name, &discard)) {
            pico8_list_add(&g_cv[i].list, pico8_value_ref(e));
        }
    }

    /* for t in all(e.nu) do ft(cg,t,e) end -- e.nu, when present, is a
     * nested ob()-parsed table of positional string values (see
     * pico8_entity.h's own comment on `.nu`'s shape and this file's top
     * comment on dc()'s own `.ra`/`.fa` absence handling for the same
     * "field simply not set" case, which real Lua's `all(nil)` would
     * itself typically just iterate zero times over rather than error on
     * -- treated the same way here). */
    if (pico8_instance_get_field(e, "nu", &nu_val) && nu_val.kind == PICO8_VALUE_REF && nu_val.as.ref != NULL) {
        const pico8_ob_table_t *nu_table = nu_val.as.ref;
        for (i = 0; i < nu_table->count; i++) {
            if (nu_table->entries[i].value.kind == PICO8_VALUE_STRING) {
                pico8_cg_entry_t *bucket = pico8_cg_find_or_create(nu_table->entries[i].value.as.string);
                pico8_list_add(&bucket->list, pico8_value_ref(e));
            }
        }
    }

    return e;
}

/* he(e) (pico8/lua.lua:163-168) -- the exact del()-based mirror of lt()
 * above; see this file's own top comment for why a missing cg[t] bucket
 * is a defensive no-op here rather than reproducing Lua's own
 * index-a-nil-table crash. */
void pico8_he(pico8_instance_t *e) {
    int i;
    pico8_value_t discard;
    pico8_value_t nu_val;

    /* del(hv,e) */
    pico8_list_del(&g_hv, pico8_value_ref(e));

    /* for p in all(bm) do if (e[p]) del(cv[p],e) end */
    for (i = 0; i < PICO8_BM_COUNT; i++) {
        if (pico8_instance_get_field(e, g_cv[i].name, &discard)) {
            pico8_list_del(&g_cv[i].list, pico8_value_ref(e));
        }
    }

    /* for t in all(e.nu) do del(cg[t],e) end */
    if (pico8_instance_get_field(e, "nu", &nu_val) && nu_val.kind == PICO8_VALUE_REF && nu_val.as.ref != NULL) {
        const pico8_ob_table_t *nu_table = nu_val.as.ref;
        for (i = 0; i < nu_table->count; i++) {
            if (nu_table->entries[i].value.kind == PICO8_VALUE_STRING) {
                pico8_cg_entry_t *bucket = pico8_cg_find(nu_table->entries[i].value.as.string);
                if (bucket != NULL) {
                    pico8_list_del(&bucket->list, pico8_value_ref(e));
                }
            }
        }
    }
}

/* mv(e,rm,...) (pico8/lua.lua:5-9) -- see this file's own top comment for
 * the full line-by-line trace and Phase 3 Chunk 5's own arity-extension
 * rationale (argc plus up to two extra pico8_value_t slots, per
 * pico8_value.h's own comment on pico8_value_function_t). */
int pico8_mv(pico8_instance_t *e, const char *method_name, int argc, pico8_value_t arg0, pico8_value_t arg1,
             pico8_value_t *out_value) {
    pico8_value_t field_val;

    /* local fn=e and e[rm] -- e's own falsiness short-circuits before
     * e[rm] is even attempted. */
    if (e == NULL) {
        return 0;
    }
    if (!pico8_instance_get_field(e, method_name, &field_val)) {
        return 0;
    }

    /* if type(fn)=="function" then return fn(e,...) end; return fn */
    if (field_val.kind == PICO8_VALUE_FUNCTION) {
        *out_value = field_val.as.function(e, argc, arg0, arg1);
    } else {
        *out_value = field_val;
    }
    return 1;
}

/* dc(nt) (pico8/lua.lua:188-199) -- see this file's own top comment for
 * the full line-by-line trace, including the deliberate, defensive
 * divergences from Lua's own nil-key-assignment crash for a `.fa`-less
 * entity. */
void pico8_dc(const char *nt) {
    const pico8_list_t *bucket_src;
    pico8_list_t layers[15];
    int i, o;

    for (i = 0; i < 15; i++) {
        pico8_list_init(&layers[i]);
    }

    /* for _,qc in pairs(cv[nt]) do ft(fl,qc.fa,qc) end */
    bucket_src = pico8_cv(nt);
    if (bucket_src != NULL) {
        for (i = 0; i < bucket_src->count; i++) {
            pico8_instance_t *qc = bucket_src->items[i].as.ref;
            pico8_value_t fa_val;
            int layer;

            if (!pico8_instance_get_field(qc, "fa", &fa_val) || fa_val.kind != PICO8_VALUE_FIXED) {
                continue;
            }
            layer = (int)pico8_fixed_to_double(fa_val.as.fixed);
            if (layer < 1 || layer > 15) {
                continue;
            }
            pico8_list_add(&layers[layer - 1], pico8_value_ref(qc));
        }
    }

    for (o = 1; o <= 15; o++) {
        pico8_list_t *bucket = &layers[o - 1];

        /* if (nt=="kj" and fl[o]) mo:md(ci[o]) */
        if (strcmp(nt, "kj") == 0 && bucket->count > 0) {
            pico8_value_t ci_val;
            if (pico8_ob_table_get_index(g_ci, o, &ci_val)) {
                pico8_value_t discard;
                pico8_mv(g_mo, "md", 1, ci_val, pico8_value_ref(NULL), &discard);
            }
        }

        /* for _,qc in pairs(fl[o] or {}) do qc[nt](qc,qc.ra) end */
        for (i = 0; i < bucket->count; i++) {
            pico8_instance_t *qc = bucket->items[i].as.ref;
            pico8_value_t ra_val;
            pico8_value_t discard;

            if (!pico8_instance_get_field(qc, "ra", &ra_val)) {
                ra_val = pico8_value_ref(NULL);
            }
            pico8_mv(qc, nt, 1, ra_val, pico8_value_ref(NULL), &discard);
        }
    }

    for (i = 0; i < 15; i++) {
        pico8_list_free(&layers[i]);
    }
}

const pico8_ob_table_t *pico8_ci(void) {
    return g_ci;
}

pico8_instance_t *pico8_mo(void) {
    return g_mo;
}

/* See pico8_entity.h's own comment on this declaration.
 *
 * REAL BUG FOUND AND FIXED (Phase 5, raylib interactive backend): used
 * to unconditionally pico8_instance_free() the outgoing g_mo before
 * replacing it -- see pico8_ui.c's pico8_ui_set_je()/pico8_ui_set_ok()
 * for the full trace of the confirmed ASan heap-use-after-free this
 * exact pattern produces (mo/je/ok are the same trio pico8_jl() rebuilds
 * unconditionally on every call, including from inside a real mouse
 * click's own dispatch chain). Fixed the same way, for the same reason:
 * stop freeing the outgoing instance and let it leak, matching real
 * Lua's own deferred-reclaim (GC) semantics -- zero Lua-observable
 * behavior change. */
void pico8_entity_set_mo(pico8_instance_t *instance) {
    if (instance == g_mo) {
        return;
    }
    g_mo = instance;
}
