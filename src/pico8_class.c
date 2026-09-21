/* pico8/lua.lua:85-97 -- `kv:jr`, ported literally (per this chunk's own
 * brief: no per-function unit test for this chunk, verified by close
 * source/port comparison instead). The exact source, read character by
 * character directly from pico8/lua.lua:
 *
 *   kv={}
 *   function kv:jr(rj)rj=ob(rj or "")rj.iq,rj.ns,kv[rj.fz or ""]=
 *   self,{__index=rj},rj
 *   return setmetatable(rj,{__index=self,__call=function(self,ob)ob=setmetatable(mw(ob),rj.ns)local ko,iy=rj
 *   while ko do
 *   if ko.pq and ko.pq~=iy then
 *   iy=ko.pq
 *   iy(ob)end
 *   ko=ko.iq
 *   end
 *   return ob
 *   end
 *   })end
 *
 * plus, referenced by the body above (already ported/available):
 *   function mw(o)
 *   return qj({},o)end
 *   function qj(o,mr)for k,v in pairs(mr or {})do
 *   o[k]=v
 *   end
 *   return o
 *   end
 *
 * Read line by line:
 *
 * `kv={}` -- a plain table, the name->class registry (`pico8_class_lookup`/
 * `pico8_class_register` below). Never a real "class" itself: it has no
 * `.iq`/`.ns`/`.pq`, and a root class's `.iq` (parent) is `self` at the
 * point a root is declared as `kv:jr(...)` -- i.e. `kv` itself. Ported as
 * a root class's parent being C `NULL`, not a sentinel "kv" class object,
 * since nothing ever reads fields off that parent besides "is it there"
 * (the `while ko do` / `ko=ko.iq` loop, and pico8_class_effective_pq's own
 * walk below) -- both loops already stop correctly at NULL exactly where
 * the original's loop would take one extra, wholly inert pass over `kv`
 * itself (`kv.pq` is nil, `kv.iq` is nil, so that pass never calls
 * anything and never changes `ko`'s next value) and then stop. Confirmed
 * empty-body-equivalence, not assumed.
 *
 * `function kv:jr(rj)` -- called as `ParentClass:jr(spec)`: Lua's `:`
 * syntax makes `self`=ParentClass (the table `:jr` was invoked on) and
 * `rj`=spec (the first explicit argument). Ported as
 * `pico8_class_declare(pico8_class_t *parent, const char *spec)`.
 *
 * `rj=ob(rj or "")` -- `rj` is REASSIGNED from "the spec string" to "the
 * freshly parsed table" (`rj or ""` handles the no-argument
 * `kv:jr()` call sites, e.g. line 289 `lq=kv:jr()`, by parsing an empty
 * string). This SAME table becomes the new class -- in the original, one
 * Lua table plays double duty as both "the class's default-fields" (its
 * ob()-parsed data entries, e.g. x=.../c=...) and "the class descriptor"
 * (`.iq`/`.ns`/`.fz`/`.pq`, ordinary entries on that same table). This
 * port's pico8_class_t keeps the parsed spec (`cls->defaults`, still the
 * exact pico8_ob() result, `fz` included if the spec set one -- nothing is
 * stripped out of it) separate from the 3 pieces of bookkeeping the PRD's
 * own "kv:jr class system" bullet calls out as belonging to the class
 * struct itself (name/parent/pq) -- see pico8_class.h's own top comment
 * for why this split doesn't change observable behavior anywhere in this
 * chunk's scope.
 *
 * `rj.iq,rj.ns,kv[rj.fz or ""]=self,{__index=rj},rj` -- three parallel
 * assignments:
 *   - `rj.iq=self` -- the new class's parent pointer. Ported as
 *     `cls->parent = parent`.
 *   - `rj.ns={__index=rj}` -- a metatable, precomputed once per class and
 *     reused at every instantiation (`setmetatable(mw(ob),rj.ns)` below),
 *     that makes an INSTANCE's field lookup fall back to `rj` (this
 *     class's own default-fields) when the instance itself doesn't have
 *     the field. This port has no metatable to precompute: the same
 *     effect (instance misses -> fall to `cls->defaults` -> fall to
 *     `cls->parent->defaults` -> ...) is produced by
 *     pico8_instance_get_field's own chain walk below, done live at every
 *     lookup rather than installed once as a metatable at construction --
 *     see this file's own instantiate-time discussion further down for
 *     why "live" (not "copy defaults into the instance once") is the
 *     chosen, PRD-directed reading.
 *   - `kv[rj.fz or ""]=rj` -- registers the class by name, "" if the
 *     parsed spec never set an `fz` field (confirmed by grepping
 *     pico8/lua.lua: only 2 of ~30 `:jr()` call sites ever set `fz`
 *     explicitly, lines 1018/1068's `fz="pm"`/`fz="ga"` -- every other
 *     `:jr()` call registers, and silently overwrites, the SAME `kv[""]`
 *     slot; nothing in scope ever reads `kv[""]` back out, so this is
 *     inert in practice but reproduced literally rather than "cleaned up",
 *     per this chunk's own no-simplification instruction). `kv[hp.cw]`
 *     at line 454 is real evidence this registry genuinely gets read back
 *     by name at runtime elsewhere in the cart (Phase 3's problem to wire
 *     up; this chunk just builds the mechanism `pico8_class_lookup` needs
 *     to exist for that).
 *
 * `return setmetatable(rj,{__index=self,__call=function(self,ob)...end})`
 * -- installs TWO things on the class table `rj` itself via one
 * metatable:
 *   - `__index=self` -- CLASS-level attribute fallback (parent, i.e. the
 *     original `self` of this `jr` call): this is what makes `:jr` itself
 *     callable on any class (`kv.jr` is the only real definition; every
 *     subclass reaches it by falling through this same chain up to `kv`),
 *     and, just as importantly, what makes `ko.pq` (in the `__call` body
 *     below) resolve to an INHERITED ancestor's `pq` when `ko` itself has
 *     no own `.pq` entry -- ordinary Lua field access on `ko`, not a
 *     separate explicit inheritance lookup. Ported as
 *     `pico8_class_effective_pq` below, since C struct field access has
 *     no such automatic fallback.
 *   - `__call=function(self,ob)...end` -- "instantiating a class". Ported
 *     as `pico8_class_instantiate(pico8_class_t *cls, const
 *     pico8_ob_table_t *overrides)` (the closure's `self` parameter is
 *     always literally `rj` itself here -- Lua's `__call` metamethod is
 *     invoked as `mt.__call(t,...)`, and `t` IS `rj` at every call site
 *     `rj(...)` -- so this port just calls the parameter `cls` throughout,
 *     never needing two names for the same value the way the closure's
 *     `self`/upvalue-`rj` pair does).
 *
 * Inside `__call`:
 *
 * `ob=setmetatable(mw(ob),rj.ns)` -- `mw(ob)` is `qj({},ob)`, i.e. a fresh
 * empty table with every key/value pair of `ob` (the overrides argument)
 * copied in via `pairs()` -- both the array part (positional keys) and
 * the hash part (named keys), whichever `ob` actually has; `pairs(nil or
 * {})` when `ob` (`ke()`'s no-argument case) is nil makes this an empty
 * copy. Ported as: allocate `inst->fields` (a fresh, empty
 * pico8_ob_table_t), then, if `overrides != NULL`, copy EVERY entry
 * (index-keyed and name-keyed alike, matching `pairs()`'s indifference to
 * key kind) into it. `setmetatable(...,rj.ns)` then attaches the
 * `__index=rj` fallback discussed above; ported as `inst->cls = cls`,
 * consulted live by pico8_instance_get_field rather than baked in via a
 * metatable pointer.
 *
 * `local ko,iy=rj` -- `ko=rj` (start the chain walk at the LEAF class,
 * i.e. `cls`, the one actually called), `iy=nil` (Lua's multi-assignment:
 * only one value on the right-hand side, so `iy` gets `nil` -- "no pq
 * called yet"). Ported as `ko = cls; iy = NULL` (using C's `pico8_class_pq_fn`
 * NULL as the "no pq called yet" sentinel, the direct C equivalent of Lua
 * `nil` here since a `pico8_class_pq_fn` value is never itself meaningfully
 * "falsy" any other way).
 *
 * `while ko do if ko.pq and ko.pq~=iy then iy=ko.pq iy(ob)end ko=ko.iq end`
 * -- the dedup-preserving chain walk, THE load-bearing rule this whole
 * chunk exists to preserve exactly (PRD: "not simplified"). `ko.pq` is an
 * ordinary Lua field read on the CURRENT `ko`, which (per the `__index`
 * chain installed above) transparently resolves to the nearest ancestor's
 * `pq` if `ko` itself has none set directly -- so as `ko` walks up from
 * `cls` to the root, `ko.pq` can keep re-resolving to the SAME already
 * inherited ancestor function at every step until `ko` itself becomes
 * that ancestor. The `ko.pq~=iy` half of the guard is precisely what
 * catches that re-resolution and skips it, while still allowing a
 * DIFFERENT (distinct-function) `pq` -- either the leaf's own override, or
 * a different ancestor's own `pq` further up -- to fire separately. Ported
 * as `pico8_class_effective_pq(ko)` (the explicit C stand-in for the
 * `__index`-chain field read `ko.pq`) compared by function-pointer
 * identity against `iy`, called via `iy(inst)` on a match, then
 * `ko = ko->parent` (`ko=ko.iq`) -- see pico8_class_instantiate below for
 * the literal loop. Two concrete traces worth keeping in mind (also
 * exercised by this chunk's own scratchpad sanity checks, since this
 * chunk has no unit test proper):
 *   - Instantiate a MIDDLE class that has no own `pq`, inheriting a root
 *     ancestor's: `ko=middle`, `effective_pq(middle)` resolves through to
 *     the root's `pq` (call it F); `F~=nil` so it fires once, `iy=F`.
 *     `ko=root`; `effective_pq(root)=F` (root's own); `F==iy` now, so it
 *     is skipped -- F fires exactly once, not twice. This is the case the
 *     PRD's own prose calls out by name.
 *   - Instantiate a LEAF class that sets its OWN distinct `pq` (call it
 *     G), whose parent inherits the SAME root ancestor's F: `ko=leaf`,
 *     `effective_pq(leaf)=G` (its own), fires (`iy=G`). `ko=middle`,
 *     `effective_pq(middle)=F` (inherited from root), `F~=G` so it ALSO
 *     fires (`iy=F`). `ko=root`, `effective_pq(root)=F==iy`, skipped. Both
 *     G and F fire, exactly once each, derived-to-base order -- matching
 *     the PRD's "a subclass that does set its own distinct pq still gets
 *     both its own and (separately) any ancestor's distinct pq called".
 *
 * `return ob` -- the finished instance. Ported as `return inst`. */

#include "pico8_class.h"

#include <stdlib.h>
#include <string.h>

/* `kv[name]=cls` (pico8/lua.lua:86). A plain growable array of
 * {name, class} pairs is enough here: this phase's own scope only ever
 * declares a handful of classes (a smoke test's 1-2 fake ones), so an
 * O(n) linear scan is not a real cost -- matching this phase's own
 * established style for small, bounded collections (e.g. pico8_ob_table_t
 * itself, pico8_ob.h's own comment on why `cv` will similarly be "a linear
 * array of {name, list} pairs" in Chunk 10). Not a hash map: no evidenced
 * need for one within this chunk's scope. */
typedef struct {
    char *name;
    pico8_class_t *cls;
} pico8_class_registry_entry_t;

static pico8_class_registry_entry_t *g_class_registry = NULL;
static int g_class_registry_count = 0;
static int g_class_registry_capacity = 0;

/* Not strdup() (POSIX, not C99 -- this codebase's build is -std=c99), and
 * not pico8_ob.c's own private copy of the same 4-line helper either: per
 * that file's own comment on why it didn't export/reuse pico8_value.c's
 * copy, a private per-module copy of a 4-line owned-string-copy helper is
 * not worth widening another module's public header surface for. */
static char *pico8_class_strdup(const char *s) {
    size_t n = strlen(s);
    char *out = malloc(n + 1);
    memcpy(out, s, n + 1);
    return out;
}

/* `kv[name]=cls`, Lua-table-assignment style: overwrite an existing
 * same-named entry's class pointer in place (this is exactly how the
 * original's own `kv[""]=rj` repeatedly clobbers the same "" slot across
 * every unnamed `:jr()` call -- see this file's top comment -- reproduced
 * here rather than "fixed" into an error or a multi-value slot), else
 * append a new entry with its own owned copy of `name`. */
static void pico8_class_register(const char *name, pico8_class_t *cls) {
    int i;

    for (i = 0; i < g_class_registry_count; i++) {
        if (strcmp(g_class_registry[i].name, name) == 0) {
            g_class_registry[i].cls = cls;
            return;
        }
    }

    if (g_class_registry_count == g_class_registry_capacity) {
        int new_capacity = g_class_registry_capacity == 0 ? 4 : g_class_registry_capacity * 2;
        pico8_class_registry_entry_t *new_entries =
            realloc(g_class_registry, (size_t)new_capacity * sizeof *new_entries);
        if (new_entries == NULL) {
            abort();
        }
        g_class_registry = new_entries;
        g_class_registry_capacity = new_capacity;
    }

    g_class_registry[g_class_registry_count].name = pico8_class_strdup(name);
    g_class_registry[g_class_registry_count].cls = cls;
    g_class_registry_count++;
}

/* `kv[name]` read back (line 454's `kv[hp.cw](...)` is real evidence this
 * is genuinely read, not write-only). */
pico8_class_t *pico8_class_lookup(const char *name) {
    int i;

    for (i = 0; i < g_class_registry_count; i++) {
        if (strcmp(g_class_registry[i].name, name) == 0) {
            return g_class_registry[i].cls;
        }
    }
    return NULL;
}

/* Test-support only (see pico8_class.h's own comment on this
 * declaration) -- not part of the Lua original, which never
 * "unregisters" a class from `kv`. */
void pico8_class_registry_clear(void) {
    int i;

    for (i = 0; i < g_class_registry_count; i++) {
        free(g_class_registry[i].name);
    }
    free(g_class_registry);
    g_class_registry = NULL;
    g_class_registry_count = 0;
    g_class_registry_capacity = 0;
}

/* Phase 3, Chunk 7's own named-class forward-reference registry -- see
 * pico8_class.h's own comment on pico8_class_register_named/
 * pico8_class_lookup_named for the full trace of why this exists as a
 * SEPARATE table from g_class_registry above. Fixed capacity, linear
 * strcmp scan -- same shape as pico8_pi.c's own PICO8_PI_DISPATCH_CAPACITY
 * table, not a third independently-reinvented doubling-capacity growable
 * array. */
typedef struct {
    char name[8];
    pico8_class_t *cls;
} pico8_class_named_entry_t;

static pico8_class_named_entry_t g_class_named_registry[PICO8_CLASS_NAMED_REGISTRY_CAPACITY];
static int g_class_named_registry_count = 0;

void pico8_class_register_named(const char *name, pico8_class_t *cls) {
    int i;

    for (i = 0; i < g_class_named_registry_count; i++) {
        if (strcmp(g_class_named_registry[i].name, name) == 0) {
            g_class_named_registry[i].cls = cls;
            return;
        }
    }

    if (g_class_named_registry_count >= PICO8_CLASS_NAMED_REGISTRY_CAPACITY) {
        abort();
    }
    strncpy(g_class_named_registry[g_class_named_registry_count].name, name,
            sizeof g_class_named_registry[g_class_named_registry_count].name - 1);
    g_class_named_registry[g_class_named_registry_count]
        .name[sizeof g_class_named_registry[g_class_named_registry_count].name - 1] = '\0';
    g_class_named_registry[g_class_named_registry_count].cls = cls;
    g_class_named_registry_count++;
}

pico8_class_t *pico8_class_lookup_named(const char *name) {
    int i;

    for (i = 0; i < g_class_named_registry_count; i++) {
        if (strcmp(g_class_named_registry[i].name, name) == 0) {
            return g_class_named_registry[i].cls;
        }
    }
    return NULL;
}

void pico8_class_registry_named_clear(void) {
    g_class_named_registry_count = 0;
}

/* `kv:jr` (pico8/lua.lua:86-97) -- see this file's own top comment for the
 * full line-by-line trace this function's body follows. */
pico8_class_t *pico8_class_declare(pico8_class_t *parent, const char *spec) {
    pico8_class_t *cls = malloc(sizeof *cls);
    pico8_value_t name_val;
    const char *name = "";

    cls->defaults = pico8_ob(spec != NULL ? spec : "", NULL);
    cls->parent = parent;
    cls->pq = NULL;

    if (pico8_ob_table_get_name(cls->defaults, "fz", &name_val) && name_val.kind == PICO8_VALUE_STRING) {
        name = name_val.as.string;
    }
    cls->name = pico8_class_strdup(name);

    pico8_class_register(cls->name, cls);

    return cls;
}

/* Not part of the Lua original (no manual class teardown under GC) --
 * see pico8_class.h's own comment on this declaration for what is and
 * isn't freed here and why. */
void pico8_class_free(pico8_class_t *cls) {
    if (cls == NULL) {
        return;
    }
    free(cls->name);
    pico8_ob_table_free(cls->defaults);
    free(cls);
}

/* `ke.pq=lt` / `function jb:pq(...)...end` (both real call sites, lines
 * 184 and 314 among others) -- a plain field assignment on an
 * already-declared class, ported as a plain setter since this port keeps
 * `pq` as its own dedicated struct member rather than a generic table
 * entry (see pico8_class.h's top comment). */
void pico8_class_set_pq(pico8_class_t *cls, pico8_class_pq_fn fn) {
    cls->pq = fn;
}

/* `ko.pq` (pico8/lua.lua:90's `if ko.pq and ko.pq~=iy then`) -- an
 * ordinary Lua field read on a class table, which the `__index=self`
 * (parent) metatable installed by `kv:jr` (this file's top comment) makes
 * transparently fall back to the nearest ancestor's own `.pq` when `cls`
 * itself has none set directly. This is the C stand-in for that
 * metatable-mediated fallback: struct field access has no such automatic
 * behavior, so it's made explicit here as its own small walk, called at
 * every step of pico8_class_instantiate's own outer chain walk below
 * (i.e. it runs once per ancestor visited, exactly mirroring how the
 * original re-evaluates `ko.pq` fresh at every `while ko do` iteration
 * rather than caching it). */
static pico8_class_pq_fn pico8_class_effective_pq(const pico8_class_t *cls) {
    while (cls != NULL) {
        if (cls->pq != NULL) {
            return cls->pq;
        }
        cls = cls->parent;
    }
    return NULL;
}

/* The `__call` closure (pico8/lua.lua:88-96) -- see this file's own top
 * comment for the full line-by-line trace. Design note on the
 * live-fallback-vs-copy-defaults-at-construction question this chunk's
 * own brief calls out explicitly: this function does NOT copy `cls`'s (or
 * any ancestor's) default fields into `inst->fields` at construction
 * time. Only `overrides` gets copied in (mirroring `mw(ob)=qj({},ob)`
 * exactly -- `mw` only ever copies from `ob`, the overrides argument,
 * never from `rj`/`cls`). A default field lives only on the class object
 * itself and is reached by pico8_instance_get_field's own live chain walk
 * at every lookup, never duplicated onto the instance -- this is the
 * direct C equivalent of the original's `setmetatable(mw(ob),rj.ns)`,
 * where `rj.ns={__index=rj}` makes Lua consult `rj` (and, transitively,
 * `rj`'s own `__index=parent` metatable) FRESH on every single field
 * access that misses on the instance, not once at construction. Chosen
 * over a one-time copy for two reasons: (1) it is what the PRD's own
 * "kv:jr class system" bullet asks for verbatim ("reproducing the
 * __index prototype chain via ordinary field-lookup fallback in C rather
 * than a metatable" -- a fallback CHAIN consulted per-lookup, not a
 * flattening performed once); (2) a one-time copy would be observably
 * different the moment a class's own default-fields table is ever
 * mutated after instances already exist (a copy would leave existing
 * instances holding the OLD value; live fallback would show every
 * existing instance the NEW value, matching Lua's real `__index`
 * semantics, which re-reads `rj` fresh every time). No call site within
 * `pico8/lua.lua`'s own class declarations was found (by reading every
 * `:jr`/`.pq=` site listed in this file's top comment) that mutates a
 * class's default fields post-declaration -- every real mutation found is
 * either `.pq=` (kept as its own dedicated struct field, not a default
 * field, precisely so this question doesn't even arise for it) or a
 * `qj(self,...)` inside a `pq` body, which mutates the INSTANCE
 * (`self`=`ob`), not the class -- so this decision is not forced by any
 * single evidenced call site either way; it is made on the PRD's own
 * explicit wording, documented here per this chunk's instructions rather
 * than guessed at silently. */
pico8_instance_t *pico8_class_instantiate(pico8_class_t *cls, const pico8_ob_table_t *overrides) {
    pico8_instance_t *inst = malloc(sizeof *inst);
    const pico8_class_t *ko;
    pico8_class_pq_fn iy = NULL;
    int i;

    /* mw(ob)=qj({},ob): a fresh, empty table with every key/value pair of
     * `overrides` copied in, whichever kind of key it is (qj's `pairs()`
     * doesn't distinguish array-part from hash-part keys) -- `overrides ==
     * NULL` mirrors `ke()`'s own `ob=nil` case (`pairs(nil or {})` is an
     * empty iteration), so nothing is copied and `inst->fields` starts
     * empty, exactly like `mw(nil)`. */
    inst->tag = PICO8_TYPE_TAG_INSTANCE;
    inst->fields = pico8_ob_table_new();
    inst->cls = cls;

    if (overrides != NULL) {
        for (i = 0; i < overrides->count; i++) {
            pico8_ob_key_t key = overrides->entries[i].key;
            if (key.kind == PICO8_OB_KEY_NAME) {
                pico8_ob_table_set_name(inst->fields, key.name, overrides->entries[i].value);
            } else {
                pico8_ob_table_set_index(inst->fields, key.index, overrides->entries[i].value);
            }
        }
    }

    /* local ko,iy=rj -- ko starts at the LEAF class (cls, the one actually
     * called); iy starts nil ("no pq called yet"), the C NULL sentinel of
     * the same meaning since a function pointer has no other falsy value. */
    ko = cls;
    while (ko != NULL) {
        /* ko.pq and ko.pq~=iy then iy=ko.pq iy(ob) -- see
         * pico8_class_effective_pq's own comment for why this call
         * reproduces the original's inheriting `ko.pq` field read, and
         * this file's top comment for the two worked traces confirming
         * the dedup rule holds exactly as specified. */
        pico8_class_pq_fn pq = pico8_class_effective_pq(ko);
        if (pq != NULL && pq != iy) {
            iy = pq;
            iy(inst);
        }
        /* ko=ko.iq */
        ko = ko->parent;
    }

    return inst;
}

/* Not part of the Lua original (no manual instance teardown under GC) --
 * see pico8_class.h's own comment on this declaration. */
void pico8_instance_free(pico8_instance_t *inst) {
    if (inst == NULL) {
        return;
    }
    pico8_ob_table_free(inst->fields);
    free(inst);
}

/* The `__index` prototype chain, walked live at every call: instance's
 * own fields first (never a metatable miss for these, since Lua field
 * ASSIGNMENT always writes directly onto the instance table itself, with
 * no `__newindex` in play here -- only reads fall through `__index`), then
 * `inst->cls`'s own default-fields table (`ns={__index=rj}`), then that
 * class's own `__index=parent` metatable's target, and so on -- see
 * pico8_class.h's own comment on this declaration and
 * pico8_class_instantiate's design-note comment above for why this is a
 * per-call walk rather than a construction-time copy. */
int pico8_instance_get_field(const pico8_instance_t *inst, const char *name, pico8_value_t *out_value) {
    const pico8_class_t *cls;

    if (pico8_ob_table_get_name(inst->fields, name, out_value)) {
        return 1;
    }
    for (cls = inst->cls; cls != NULL; cls = cls->parent) {
        if (pico8_ob_table_get_name(cls->defaults, name, out_value)) {
            return 1;
        }
    }
    return 0;
}
