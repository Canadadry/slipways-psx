#ifndef PICO8_LIST_H
#define PICO8_LIST_H

#include "pico8_type_tag.h"
#include "pico8_value.h"

/* PICO-8's own list primitives -- add()/del()/all()/foreach(), used all
 * over pico8/lua.lua (e.g. lt()/he() at lines 156-167) -- are NOT stock
 * Lua's table.insert/table.remove/pairs/ipairs: PICO-8 documents its own
 * distinct semantics for them (see the per-function comments below), and
 * this module ports those, not generic Lua table behavior.
 *
 * One small growable array of the shared pico8_value_t (pico8_value.h) is
 * the single list representation this chunk introduces, reused as-is by
 * the ob()/fi() parser's output and the entity key/value representation
 * later in this phase (see pico8_value.h's own top comment) -- not
 * reinvented per consumer. */
typedef struct {
    /* Always PICO8_TYPE_TAG_LIST, set by pico8_list_init() -- see
     * pico8_type_tag.h for why this port's three Lua-table-modelling
     * structs all carry an explicit first-member discriminator. Kept
     * FIRST (and first in every sibling struct) so pico8_type_tag_of()
     * can read it off a bare REF without knowing the type yet. */
    pico8_type_tag_t tag;
    pico8_value_t *items;
    int count;
    int capacity;
} pico8_list_t;

/* pico8_list_init/pico8_list_free: construct an empty list / release the
 * backing storage. Every list used by this phase is created and owned by
 * C code directly (no garbage collector), so these bracket a list's
 * lifetime the way Lua's table creation/collection is implicit. */
void pico8_list_init(pico8_list_t *list);
void pico8_list_free(pico8_list_t *list);

/* add(t,v): appends v to the end of t, growing the backing storage
 * (capacity doubling, starting at a small initial capacity) as needed. */
void pico8_list_add(pico8_list_t *list, pico8_value_t value);

/* del(t,v): removes the FIRST element equal in value to v (via
 * pico8_value_equal, see pico8_value.h), not by index -- PICO-8's own
 * documented del() semantics, distinct from stock Lua's table.remove
 * (which removes by index). Source: PICO-8 Del wiki,
 * https://pico-8.fandom.com/wiki/Del
 * Elements after the removed one are shifted down by one to close the
 * gap, matching the wiki's own description. If no element matches, the
 * list is left unchanged (documented no-op case, not an error). Returns
 * 1 if an element was removed, 0 if nothing matched. */
int pico8_list_del(pico8_list_t *list, pico8_value_t value);

/* all(t): a stateful, index-based iterator over t's elements in order,
 * mirroring PICO-8's own implementation (per the wiki's own description
 * of its internals). Source: PICO-8 All wiki,
 * https://pico-8.fandom.com/wiki/All
 *
 * Deletion tolerance: the iterator remembers the index of the element it
 * last returned and the list's element count as observed right after
 * that return. On the next call, if the list's count has dropped by
 * exactly one since then, the iterator assumes THAT SAME element (the
 * one it just returned) was the one deleted -- since del() shifts later
 * elements down to fill the gap, the next real element has now slid into
 * that same slot, so the iterator re-reads that slot instead of
 * advancing past it. This is exactly the documented caveat: "capable of
 * recovering after the element at the iteration point is deleted, but
 * not elements before it" -- deleting a DIFFERENT, duplicate-valued (or
 * earlier) element while iterating is not specially guarded against and
 * can still skip or re-visit an element, matching the original rather
 * than "improving" on it. */
typedef struct {
    const pico8_list_t *list;
    int index;      /* index of the last element returned, or -1 before the first call */
    int last_count; /* list->count as observed right after that return (or at init) */
} pico8_list_iter_t;

pico8_list_iter_t pico8_list_all(const pico8_list_t *list);

/* Advances the iterator and writes the next element to *out_value.
 * Returns 1 if a value was produced, 0 once iteration is exhausted (in
 * which case *out_value is left untouched). */
int pico8_list_iter_next(pico8_list_iter_t *iter, pico8_value_t *out_value);

/* foreach(t,fn): calls fn once per element of t, in order, passing an
 * extra ctx pointer through unchanged (C's stand-in for a Lua closure's
 * captured upvalues -- pass NULL when fn needs none). No deletion
 * tolerance is implemented here, matching the PRD: no call site in this
 * phase's scope mutates the list from inside a foreach() callback, and
 * none is documented as needing to. */
typedef void (*pico8_list_foreach_fn)(pico8_value_t value, void *ctx);
void pico8_list_foreach(const pico8_list_t *list, pico8_list_foreach_fn fn, void *ctx);

#endif
