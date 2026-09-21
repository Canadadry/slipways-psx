#ifndef PICO8_OB_H
#define PICO8_OB_H

#include "pico8_list.h"
#include "pico8_type_tag.h"
#include "pico8_value.h"

/* pico8/lua.lua's ob()/fi() data-description mini-language (lines 55-78):
 * a hand-rolled, character-by-character parser that turns a compact
 * string literal like "x=-1,y=-1,c=0," into a Lua table -- used all over
 * the cart for class-default spec strings (kv:jr) and flat data literals
 * (bm, ci, er, ...). Ported here as a literal state machine over the
 * shared pico8_value_t/pico8_list_t types (pico8_value.h/pico8_list.h),
 * not as a general-purpose recursive-descent grammar -- see pico8_ob.c's
 * top comment and the per-function comments below for exactly which Lua
 * source lines each piece mirrors.
 *
 * A real Lua table produced by ob() mixes POSITIONAL keys (1, 2, 3, ...,
 * auto-incrementing) and NAMED keys ("x", "y", ...) in the same table, so
 * this ordered-map representation supports both: a growable list of
 * {key, value} pairs where a key is tagged as either an integer index or
 * a string name. */
typedef enum {
    PICO8_OB_KEY_INDEX,
    PICO8_OB_KEY_NAME
} pico8_ob_key_kind_t;

typedef struct {
    pico8_ob_key_kind_t kind;
    int index; /* valid when kind == PICO8_OB_KEY_INDEX */
    char *name; /* valid when kind == PICO8_OB_KEY_NAME; owned by whichever
                 * pico8_ob_table_t this key sits in (see pico8_ob_table_free) */
} pico8_ob_key_t;

typedef struct {
    pico8_ob_key_t key;
    pico8_value_t value;
} pico8_ob_entry_t;

/* ob()'s result table: an ordered list of {key, value} pairs, appended in
 * parse order (so a purely positional spec's entries come back in the
 * same order the original list was written in, mirroring Lua's own
 * array-part iteration order for a table built the same way). A value of
 * kind PICO8_VALUE_REF may itself point to a nested pico8_ob_table_t
 * (see pico8_ob.c's "identifier(...)" branch) -- pico8_value_t's REF kind
 * already anticipated exactly this case (see pico8_value.h). As with every
 * other pico8_value_t-holding structure in this phase, a table does NOT
 * own the memory a PICO8_VALUE_STRING or PICO8_VALUE_REF value points to
 * (see pico8_value.h's own ownership note) -- only this table's own key
 * names are owned and freed by pico8_ob_table_free. */
typedef struct {
    /* Always PICO8_TYPE_TAG_OB_TABLE -- see pico8_type_tag.h, and
     * pico8_list_t's own `tag` comment (pico8_list.h), for why all three
     * of this port's Lua-table-modelling structs lead with this. */
    pico8_type_tag_t tag;
    pico8_ob_entry_t *entries;
    int count;
    int capacity;
} pico8_ob_table_t;

/* Constructs an empty table / releases a table's own backing storage and
 * owned key-name strings. Every value stored in the table -- including a
 * nested pico8_ob_table_t reached through a PICO8_VALUE_REF -- is the
 * caller's responsibility to free separately, exactly like pico8_value_t's
 * own established borrowed-pointer convention (pico8_value.h); this keeps
 * a single, uniform ownership rule across the whole shared value type
 * rather than a special case for tables specifically. */
pico8_ob_table_t *pico8_ob_table_new(void);
void pico8_ob_table_free(pico8_ob_table_t *table);

/* Not a port of any named Lua function: the "pico8_list_t -> fresh
 * positional pico8_ob_table_t" bridge this port needs wherever a value
 * built as an array-shaped list (`{"exports:","0/1",kw}`, `fm.invented`,
 * ...) is then consumed by something that copies a table's key/value
 * pairs (`mw(ob)`/`qj({},ob)` -- pico8_class_instantiate). Real Lua needs
 * no such bridge, since `pairs()` walks a list-shaped table exactly like
 * any other; this port does, because its two shapes have different
 * element types (see pico8_type_tag.h).
 *
 * Entries come out at positional keys 1..#list in list order, which is
 * exactly the order `pairs()` yields a Lua table's own array part built
 * by repeated `add()`.
 *
 * Promoted here from pico8_menu.c, which had it as a private
 * `pico8_menu_list_to_ob_table()` for the identical bug in `js:pq`'s own
 * "jb" field, once pico8_ui_button.c's own "lq" field turned out to need
 * the very same conversion -- one shared converter rather than a second
 * copy of it. Returns a NEW, heap-allocated table the CALLER owns; the
 * row-building call sites never free it, the same accepted "temporary
 * override table, shallow-copied by the constructor chain that eventually
 * consumes it, then left for the rest of the program" convention this
 * port's row builders already establish (e.g. pico8_hp_jb()'s own table)
 * -- this port has no GC (pico8_value.h's own ownership note). A NULL
 * list yields an empty table, mirroring `pairs(nil or {})`. */
pico8_ob_table_t *pico8_ob_table_from_list(const pico8_list_t *list);

/* Sets table[index]/table[name] = value, Lua-table-assignment style: if an
 * entry with an equal key (by value, not by pointer, for a name) already
 * exists, its value is overwritten in place; otherwise a new entry is
 * appended. A name is always copied into the table's own storage -- the
 * caller's `name` argument is never retained by reference. These are the
 * two primitives every write into an ob() result table goes through
 * (parsing a field, and merging in a `defaults` table -- see pico8_ob.c). */
void pico8_ob_table_set_index(pico8_ob_table_t *table, int index, pico8_value_t value);
void pico8_ob_table_set_name(pico8_ob_table_t *table, const char *name, pico8_value_t value);

/* Looks up table[index]/table[name]. Returns 1 and writes *out_value if
 * found, 0 (leaving *out_value untouched) otherwise -- "absent" is
 * distinct from "present but false/zero", matching Lua's own nil-vs-false
 * distinction (see the PRD's Entity representation section), even though
 * this chunk's own scope doesn't yet need that distinction observed by any
 * caller beyond these two functions themselves. */
int pico8_ob_table_get_index(const pico8_ob_table_t *table, int index, pico8_value_t *out_value);
int pico8_ob_table_get_name(const pico8_ob_table_t *table, const char *name, pico8_value_t *out_value);

/* Phase 3, Chunk 18 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): removes table[index] entirely (as opposed to
 * pico8_ob_table_set_index, which can only ever ASSIGN a value, never take
 * an entry away) -- the C equivalent of real Lua's own `t[index]=nil`
 * field-removal statement. Added by this chunk because `pj:hk(yr)`'s own
 * real `iw[3]=pp or nil` (pico8/lua.lua, Chunk 18) genuinely needs to
 * REMOVE a previously-set index-3 entry, on the ordinary/exercised path
 * (whenever `pp` is falsy), not merely skip writing one that was never
 * there -- unlike every earlier "field write skipped when the real Lua
 * value would be nil" case in this port (e.g. `fq:ldown`'s own `self.p_=
 * mv(self,"my")`, pico8_ui_button.h), where the target field was never
 * previously set at all, so simply not writing it was already faithful.
 * Here the SAME class-level table can carry a real value from an earlier
 * call that must be taken away again on a later one, and this port's
 * shared `pico8_ob_table_t` had no primitive for that until now (every
 * prior consumer only ever read or overwrote entries, confirmed by
 * grepping every existing caller of pico8_ob_table_set_index/set_name
 * across src/). A no-op if `table` is NULL or no PICO8_OB_KEY_INDEX entry
 * at `index` exists. Does not free the removed entry's own VALUE (a
 * pico8_ob_table_t never owns what a PICO8_VALUE_REF/STRING value points
 * to, per this file's own top comment) -- only this table's own {key,
 * value} slot is spliced out and the backing array compacted. No
 * pico8_ob_table_remove_name() companion is added: no real call site in
 * this port's own scope (as of this chunk) ever needs to remove a
 * NAMED entry, only an indexed one -- add that symmetrically, following
 * this same shape, if/when one genuinely does. */
void pico8_ob_table_remove_index(pico8_ob_table_t *table, int index);

/* The `pi` constructor table (pico8/lua.lua:133, `pi={b=qz,v=v,br=rk,
 * qr=io,rf=eq,s=spr}`): a name -> constructor-function lookup that ob()'s
 * "identifier(...)" branch consults. ob()'s own `sc` (the token's FIRST
 * character, see pico8_ob.c) is always exactly one character, so despite
 * `pi` itself containing some two-letter keys in the original source
 * ("br", "qr", "rf" -- which sc, being length 1, could therefore never
 * actually reach), this table is keyed by a single `char`, matching what
 * ob() can actually look up.
 *
 * This chunk (Phase 2, Chunk 8) deliberately does NOT register any of the
 * real constructors (`v` the vector constructor, `s` for `spr`, etc.) --
 * those belong to modules this phase either already built for other
 * reasons (`v`, in pico8_vec.h) or hasn't built yet (`spr` is Phase 1's
 * pico8_gfx, `qz`/`rk`/`io`/`eq` are Phase 3 game code not yet ported).
 * Wiring them in here would be premature/speculative for a chunk whose
 * own scope is just the parsing mechanism. Only the *mechanism* --
 * register-by-single-character, look-up-by-single-character, and a clear
 * function for test isolation -- is built here, exercised by this chunk's
 * own unit tests with a trivial test-only registration. Wiring in the real
 * constructors is Phase 3's job, once the functions they name actually
 * exist to call. */
typedef pico8_value_t (*pico8_ob_constructor_fn)(pico8_value_t a1, pico8_value_t a2,
                                                  pico8_value_t a3, pico8_value_t a4,
                                                  pico8_value_t a5);

void pico8_ob_register_constructor(char id, pico8_ob_constructor_fn fn);
pico8_ob_constructor_fn pico8_ob_lookup_constructor(char id);
/* Test-support only (not part of the Lua original, which never
 * "unregisters" anything from `pi`): resets the table to empty so one
 * test's registration can't leak into another's. */
void pico8_ob_clear_constructors(void);

/* fi(qs,fn) (pico8/lua.lua:74-78): calls fn(c, i, ctx) once for every
 * character c of qs in order (i is the 1-based index, matching PICO-8's
 * own 1-indexed strings). The original Lua collects fn's non-nil return
 * values into a list and returns it; since pico8_value_t has no "nil"
 * kind, that's represented here as fn writing *out_value and returning 1
 * when it wants to contribute a value, 0 when it doesn't (nothing is
 * appended for that character). The returned list is heap-backed and must
 * be released with pico8_list_free(). ob() itself (below) only relies on
 * fi()'s per-character callback side effects, never on this returned list
 * -- its own callback never produces a value, exactly mirroring the real
 * ob()'s callback, which has no `return` statement and so always yields
 * Lua nil. */
typedef int (*pico8_fi_fn)(char c, int i, void *ctx, pico8_value_t *out_value);
pico8_list_t pico8_fi(const char *s, pico8_fi_fn fn, void *ctx);

/* ob(qs,mr) (pico8/lua.lua:55-73): parses qs into a new table exactly as
 * described in pico8_ob.c's own top comment (the full character-by-
 * character rule set), then merges the optional `defaults` table (`mr` in
 * the original) into it and returns the result. The returned table (and
 * any nested pico8_ob_table_t reached through it) is heap-allocated and
 * owned by the caller -- release it with pico8_ob_table_free() (see that
 * function's own comment on what it does and doesn't own).
 *
 * IMPORTANT, evidenced directly from every real top-level ob() call site
 * in pico8/lua.lua (bm, ci, er, es, b_, co, ek, ...): the spec string
 * `qs` must end with a trailing comma for its LAST field to be parsed at
 * all -- ob()'s own comma branch is what flushes a pending field into the
 * result table, and nothing flushes whatever token is still pending when
 * the string simply ends. This is not a convenience this port adds; it's
 * a literal consequence of the original's own control flow (a field
 * without a following comma is silently dropped), reproduced as-is rather
 * than "fixed". The one place this chunk's own C code must itself supply
 * that trailing comma -- because the *nested* identifier(...) content it
 * recurses into was never written with one by whoever wrote the outer
 * spec string -- is the recursive call in pico8_ob.c's "identifier(...)"
 * branch, exactly mirroring pico8/lua.lua:66's own
 * `ob(sub(qs,s+2,i-2)..",")`. */
pico8_ob_table_t *pico8_ob(const char *s, const pico8_ob_table_t *defaults);

#endif
