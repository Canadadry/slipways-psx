#ifndef PICO8_LIST_H
#define PICO8_LIST_H

#include "pico8_type_tag.h"
#include "pico8_value.h"

/* PICO-8's own list primitives -- add()/del()/all()/foreach() -- are NOT
 * stock Lua's table.insert/table.remove/pairs/ipairs: PICO-8 documents
 * distinct semantics for them (see the per-function comments below), and
 * this module ports those, not generic Lua table behavior.
 *
 * One growable array of pico8_value_t (pico8_value.h) is the single list
 * representation in this port; the ob() table (pico8_ob.h) is the other
 * shape, and the two are not interchangeable -- see pico8_type_tag.h. */
typedef struct {
    /* Always PICO8_TYPE_TAG_LIST, written by pico8_list_init(). Kept
     * FIRST so pico8_type_tag_of() can read it off a bare REF. */
    pico8_type_tag_t tag;
    pico8_value_t *items;
    int count;
    int capacity;
} pico8_list_t;

/* Construct an empty list / release its backing storage. There is no
 * garbage collector here: a list is created and freed by C code, and
 * pico8_list_free releases only `items` -- whatever a STRING or REF
 * element points at is the caller's to free separately. */
void pico8_list_init(pico8_list_t *list);
void pico8_list_free(pico8_list_t *list);

/* add(t,v): appends v to the end of t, growing the backing storage as
 * needed. Any pico8_list_iter_t over t holds a pointer to the list, not
 * to `items`, so it survives the reallocation. */
void pico8_list_add(pico8_list_t *list, pico8_value_t value);

/* del(t,v): removes the FIRST element equal in value to v (via
 * pico8_value_equal, see pico8_value.h), not by index -- PICO-8's own
 * documented del() semantics, distinct from stock Lua's table.remove
 * (which removes by index). Source: PICO-8 Del wiki,
 * https://pico-8.fandom.com/wiki/Del
 * Elements after the removed one are shifted down by one to close the
 * gap. If no element matches, the list is left unchanged (a documented
 * no-op, not an error). Returns 1 if an element was removed, else 0. */
int pico8_list_del(pico8_list_t *list, pico8_value_t value);

/* all(t): a stateful, index-based iterator over t's elements in order,
 * mirroring PICO-8's own implementation. Source: PICO-8 All wiki,
 * https://pico-8.fandom.com/wiki/All
 *
 * Deletion tolerance, which is PICO-8's behavior and not an invention of
 * this port: the iterator remembers the index of the element it last
 * returned and the count observed right after that return. On the next
 * call, if the count has dropped, it assumes THAT SAME element was the
 * one deleted -- since del() shifts later elements down, the next real
 * element has slid into that slot, so the iterator re-reads the slot
 * instead of advancing past it. That is exactly the documented caveat:
 * "capable of recovering after the element at the iteration point is
 * deleted, but not elements before it". Deleting a different, earlier or
 * duplicate-valued element mid-iteration can still skip or re-visit an
 * element, matching the original rather than "improving" on it. */
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

/* foreach(t,fn): calls fn once per element of t, in order, passing `ctx`
 * through unchanged (C's stand-in for a Lua closure's upvalues -- pass
 * NULL when fn needs none). Unlike all(), this has NO deletion tolerance:
 * a callback that adds to or deletes from `list` is not supported. */
typedef void (*pico8_list_foreach_fn)(pico8_value_t value, void *ctx);
void pico8_list_foreach(const pico8_list_t *list, pico8_list_foreach_fn fn, void *ctx);

#endif
