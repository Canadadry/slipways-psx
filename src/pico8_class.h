#ifndef PICO8_CLASS_H
#define PICO8_CLASS_H

#include "pico8_ob.h"

/* pico8/lua.lua's `kv:jr` class system (line 85-97, see pico8_class.c's
 * own top comment for the exact source and a full read-through). A class
 * is one C struct with 4 members mirroring the 4 things the PRD says a
 * class is ("kv:jr class system" bullet, Implementation Decisions):
 * a name (`.fz`, used to register the class into a name->class table,
 * `kv[rj.fz or ""]=rj`), a parent-class pointer (`.iq`, NULL for a root
 * class -- the original's root classes are declared as `kv:jr(...)`,
 * i.e. their "parent" is the plain `kv` table itself, which is not a real
 * class and has no fields/pq of its own, so NULL is the faithful C
 * equivalent), a default-fields table (the class's own spec string,
 * `ob()`-parsed -- reusing pico8_ob_table_t, Chunk 8's ob()-result type,
 * exactly as this chunk's own instructions call for, rather than a third
 * parallel table shape), and an optional single "extra constructor"
 * function pointer (`.pq`, settable after class creation via
 * pico8_class_set_pq(), mirroring the original's separate `ke.pq=lt`
 * statement -- never passed into pico8_class_declare() itself).
 *
 * Deliberately NOT stored inside the same generic default-fields table as
 * the class's actual data fields (x=.../c=.../etc.): in the real Lua, `fz`/
 * `iq`/`ns`/`pq` sit as ordinary entries on the very same table that also
 * holds the parsed spec fields, because Lua has only one kind of table.
 * This port instead keeps them as dedicated C struct members, since (a)
 * the PRD's own prose describes a class as having exactly these 4 things,
 * not "an ob() table that happens to also carry class bookkeeping", and
 * (b) keeping them separate is what lets Chunk 10 later reuse `defaults`
 * as a plain, uncontaminated key/value table for entity default fields --
 * see pico8_class.c's top comment for the line-by-line trace confirming
 * this doesn't change behavior. */
typedef struct pico8_class pico8_class_t;

/* An instantiated entity (what `ke()`/`ke(overrides)` -- the `__call`
 * metamethod -- produces). Chunk 10's real entity representation
 * (hv/cv/cg/lt/he/dc/mv) is explicitly out of this chunk's scope, but per
 * this chunk's own brief, this type is shaped so Chunk 10 can plausibly
 * grow it rather than needing a fourth parallel table shape: `fields` is
 * the instance's own shallow-copied-from-overrides table (`mw(ob)` --
 * see pico8_class.c), and `cls` is the class it was instantiated through,
 * kept so field lookups can walk the live prototype-chain fallback (see
 * pico8_instance_get_field below) the same way Lua's `__index=rj`
 * instance metatable plus each class's own `__index=parent` metatable
 * does, without ever copying a class's default fields into the instance
 * at construction time -- see pico8_class.c's top comment for why this
 * chunk deliberately chose live lookup over a one-time copy. */
typedef struct pico8_instance pico8_instance_t;

/* `pq` (pico8/lua.lua:90-92's `iy(ob)`): a class's own "extra constructor",
 * called with the freshly-constructed instance during instantiation (see
 * pico8_class_instantiate below). Takes the whole instance, not just its
 * raw fields table, because the original's own pq bodies read/write
 * fields on `self` using ordinary (fallback-including) Lua field access --
 * e.g. `hy:pq()` does `qj(self,ob([[...]]))`, mutating self's own table
 * directly, and other pq bodies (e.g. `lq:pq()`) read inherited fields off
 * self via the normal metatable chain. */
typedef void (*pico8_class_pq_fn)(pico8_instance_t *instance);

struct pico8_class {
    char *name;
    pico8_class_t *parent;
    pico8_ob_table_t *defaults;
    pico8_class_pq_fn pq;
};

struct pico8_instance {
    /* Always PICO8_TYPE_TAG_INSTANCE -- see pico8_type_tag.h, and
     * pico8_list_t's own `tag` comment (pico8_list.h), for why all three
     * of this port's Lua-table-modelling structs lead with this. Set by
     * pico8_class_instantiate() and by the handful of modules that build
     * a bare, classless instance by hand (`mo`, `je`, `fm`, the `mz.jj`/
     * `ie.ke` wrappers, pz()'s own row wrappers, ...). */
    pico8_type_tag_t tag;
    pico8_ob_table_t *fields;
    pico8_class_t *cls;
};

/* "Declaring a class" (`ParentClass:jr(spec_string)`, pico8/lua.lua:86-87's
 * `rj=ob(rj or "")rj.iq,rj.ns,kv[rj.fz or ""]=self,{__index=rj},rj`):
 * ob()-parses `spec` (NULL treated as "", mirroring `rj or ""` for the
 * no-argument `kv:jr()` call sites, e.g. line 289's `lq=kv:jr()`) into the
 * new class's default-fields table, links it to `parent` (NULL for a root
 * class), and registers it by name (its own `fz` field if the parsed spec
 * set one, else "" -- see pico8_class.c for why classes silently overwrite
 * each other at the "" registry slot exactly as the original does, and why
 * that's left as-is rather than "fixed"). `.ns` (the original's
 * precomputed `{__index=rj}` instance metatable) has no C equivalent here:
 * it existed only to make `setmetatable(mw(ob),rj.ns)` fast/reusable at
 * every instantiation, which this port achieves instead via
 * pico8_instance_t.cls plus pico8_instance_get_field's live chain walk --
 * see pico8_class.c's top comment. Returns a heap-allocated class the
 * caller owns; release it with pico8_class_free(). */
pico8_class_t *pico8_class_declare(pico8_class_t *parent, const char *spec);

/* Releases this class's own owned storage (its name copy and its
 * default-fields table) -- NOT part of the Lua original, which has no
 * manual class teardown (Lua's GC would eventually collect an
 * unreferenced class table); needed here since C has no GC. Does not
 * touch `parent` (not owned) or any instance built from this class
 * (pico8_instance_t keeps its own `cls` pointer alive only for as long as
 * the caller keeps the class alive -- freeing a class out from under a
 * still-live instance is the caller's bug to avoid, exactly as it would
 * be for any other borrowed-pointer relationship in this phase's shared
 * value type, see pico8_value.h). */
void pico8_class_free(pico8_class_t *cls);

/* Sets `cls.pq` as a separate step (mirroring the original's own
 * `ke.pq=lt` / `function jb:pq(...)...end` statements, both of which
 * assign `pq` onto an already-`:jr()`-declared class table, never as part
 * of `jr` itself) -- see pico8_class.c's pico8_class_instantiate for how
 * this gets consulted (with inheritance and the dedup rule) at
 * instantiation time. */
void pico8_class_set_pq(pico8_class_t *cls, pico8_class_pq_fn fn);

/* `kv[name]` (pico8/lua.lua:86's `kv[rj.fz or ""]=rj`, and line 454's
 * `kv[hp.cw](...)` -- a real call site that looks a class up by a name
 * held in a *runtime* string field and instantiates it, confirming this
 * registry is genuinely consulted dynamically, not just written to and
 * never read). Returns NULL if no class was ever registered under `name`.
 * `pico8_class_registry_clear()` is test-support only (not part of the
 * Lua original, which never "unregisters" a class): resets the registry
 * to empty, mirroring pico8_ob_clear_constructors's established
 * test-isolation convention, so one test's class names/"" slot can't leak
 * into another's. It frees only the registry's own bookkeeping (its owned
 * name-string copies), never the pico8_class_t objects themselves, which
 * the caller still owns and must free separately with pico8_class_free(). */
pico8_class_t *pico8_class_lookup(const char *name);
void pico8_class_registry_clear(void);

/* Phase 3, Chunk 7 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): a SEPARATE small registry from kv[]/pico8_class_lookup above,
 * added for `bg:ldown`'s forward reference to `mz`/`g_` (pico8/lua.lua:377's
 * `(#(cg.g_ or {})>0 and mz or g_)()` -- calling one of two Lua GLOBAL CLASS
 * VARIABLES by name, neither of which exists yet: `mz`/`g_` are Chunks
 * 10/16, not yet ported). Unlike `jo:hu`'s `kv[hp.cw]` dynamic reclassing
 * (Chunk 13, a genuinely different forward reference that the kv[]/
 * pico8_class_lookup registry above already serves, since `hp.cw` names a
 * class that DOES set a real `fz` field -- confirmed by reading pico8/
 * lua.lua: only `pm`/`ga`, Chunk 15, ever do), `mz`/`g_` never set an `fz`
 * field at all. Reusing kv[]'s shared "" registry slot here would silently
 * collide with EVERY other unnamed class registered so far (`ke`, `lq`,
 * `jb`, `trade`, `nf`, `js`, `fq`, `qb`, `ui`, `bg` itself) -- kv[""] holds
 * whichever of those was declared last, not a reliable way to tell "mz"
 * from "g_" apart. This registry is keyed instead by an explicit, THIS-
 * PORT-CHOSEN lookup name (never a Lua source field, just a name Chunk 7
 * and its forward-referenced classes agree on) -- one entry per forward-
 * referenced Lua global class variable this port needs to compile a call
 * to before that class exists. `pico8_bg_ldown` (pico8_bg.c) looks "mz"/
 * "g_" up here today via pico8_class_lookup_named, getting NULL (a
 * documented "not yet built" result -- the same "compiles, not yet
 * exercised" treatment this whole PRD already gives every other forward
 * reference, e.g. Phase 2's `mo:md()`/`"kj"` dead branch); whichever wave
 * later builds mz/g_ (Chunks 10/16) calls pico8_class_register_named once,
 * right alongside its own pico8_class_declare() call, using this exact
 * same name -- see pico8_bg.h's own top comment for the full trace of the
 * calling side. Fixed capacity (8 slots, linear strcmp scan), matching
 * pico8_pi.c's own PICO8_PI_DISPATCH_CAPACITY precedent, deliberately NOT a
 * third independently-reinvented doubling-capacity growable array (the
 * pattern already flagged across pico8_value.c/pico8_ob.c/pico8_class.c/
 * pico8_entity.c) -- the real evidenced need is exactly 2 names ("mz",
 * "g_"), so 8 leaves comfortable headroom without inventing a growth story
 * nothing in this port's scope needs.
 * pico8_class_registry_named_clear() is test-support only (mirrors
 * pico8_class_registry_clear()'s own test-isolation convention), never part
 * of the Lua original. */
#define PICO8_CLASS_NAMED_REGISTRY_CAPACITY 8

void pico8_class_register_named(const char *name, pico8_class_t *cls);
pico8_class_t *pico8_class_lookup_named(const char *name);
void pico8_class_registry_named_clear(void);

/* "Instantiating a class" (`ke()`/`ke(overrides)`, the `__call` closure at
 * pico8/lua.lua:88-96): shallow-copies `overrides` (NULL treated as no
 * overrides, mirroring `ke()`'s own `ob=nil` case flowing through
 * `mw(nil)=qj({},nil)`, i.e. `pairs(nil or {})` -- an empty copy) into a
 * fresh instance attached to `cls` for prototype-chain fallback, then
 * walks the class chain from `cls` up through each `.parent` link, calling
 * each *distinct* effective `pq` found exactly once -- see
 * pico8_class.c's pico8_class_instantiate for the line-by-line trace of
 * why this reproduces the original's `ko.pq and ko.pq~=iy` dedup rule
 * exactly, not a simplified approximation of it. Returns a heap-allocated
 * instance the caller owns; release it with pico8_instance_free(). */
pico8_instance_t *pico8_class_instantiate(pico8_class_t *cls, const pico8_ob_table_t *overrides);

/* Releases this instance's own fields table -- not part of the Lua
 * original for the same reason pico8_class_free isn't (no GC in C). Does
 * not touch `cls` (borrowed, not owned). */
void pico8_instance_free(pico8_instance_t *inst);

/* Field lookup with the original's full `__index` prototype-chain
 * fallback: the instance's own (overridden/shallow-copied) fields first,
 * then `cls`'s own default-fields table, then `cls->parent`'s, and so on
 * up to the root class -- reproducing `instance -> rj (leaf class, via
 * `ns={__index=rj}`) -> rj's own metatable's `__index=parent` -> ...`
 * chain via ordinary field-lookup fallback in C, exactly as the PRD's
 * "kv:jr class system" bullet specifies, rather than a metatable. This is
 * a genuine per-call chain WALK, not a value copied into the instance
 * once at construction time -- see pico8_class.c's top comment for why
 * that "live" behavior (not "copy defaults in at construction") was the
 * chosen reading of the PRD's own "reproducing the __index prototype
 * chain via ordinary field-lookup fallback" wording. Returns 1 and writes
 * *out_value if `name` is found anywhere on the chain, 0 (leaving
 * *out_value untouched) if it's absent everywhere -- matching
 * pico8_ob_table_get_name's own "absent" convention (pico8_ob.h). */
int pico8_instance_get_field(const pico8_instance_t *inst, const char *name, pico8_value_t *out_value);

#endif
