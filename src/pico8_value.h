#ifndef PICO8_VALUE_H
#define PICO8_VALUE_H

#include "pico8_fixed.h"

/* pico8/lua.lua's `add`/`del`/`all`/`foreach` (list primitives, e.g. lines
 * 156-167's lt/he, and every all()/foreach() call site) and the later
 * ob()/fi() parser output and entity key/value representation all need to
 * hold "any one of a small handful of value shapes" in a plain array or
 * field slot. Rather than let each of those consumers invent its own
 * ad-hoc tagged union (the exact "duplicated helper" mistake this phase's
 * PRD calls out from Phase 1's warn-review finding on `read_whole_file`),
 * this single small tagged-value type is the one shared shape all of them
 * are built on.
 *
 * Kept deliberately minimal ("at least" per the PRD, not "every kind we
 * can imagine"): the four kinds below are exactly what this phase's
 * ob()/fi() grammar (Chunk 8, not yet built) can produce -- a parsed
 * number, a quoted string, the literal `false`, and a nested-table/
 * constructor-call result (represented as an opaque reference, since this
 * chunk doesn't need to know what it points to) -- plus what this chunk's
 * own add/del/all/foreach need to store and compare. No separate vector
 * kind: `v(...)`/nested-table values are opaque references here, not a
 * dedicated pico8_vec_t member; nothing in this chunk's or chunk 8's scope
 * needs a value slot to know it's holding a vector specifically. */
typedef enum {
    PICO8_VALUE_FIXED,  /* a PICO-8 fixed-point number */
    PICO8_VALUE_STRING, /* a C string (not owned by the value; see below) */
    PICO8_VALUE_BOOL,   /* a boolean (0 or 1) */
    PICO8_VALUE_REF     /* an opaque pointer/reference (e.g. an entity) */
} pico8_value_kind_t;

/* A value never owns the memory a PICO8_VALUE_STRING or PICO8_VALUE_REF
 * points to -- it's a plain tagged union of a scalar or a borrowed
 * pointer, matching Lua's own by-reference semantics for strings/tables
 * (assigning a Lua value never deep-copies a string or table). Callers
 * are responsible for whatever the pointer's lifetime needs to be. */
typedef struct {
    pico8_value_kind_t kind;
    union {
        pico8_fixed_t fixed;
        const char *string;
        int boolean;
        void *ref;
    } as;
} pico8_value_t;

pico8_value_t pico8_value_fixed(pico8_fixed_t x);
pico8_value_t pico8_value_string(const char *s);
pico8_value_t pico8_value_bool(int b);
pico8_value_t pico8_value_ref(void *p);

/* "Equal in value", as used by del() (PICO-8 Del wiki:
 * https://pico-8.fandom.com/wiki/Del) to find the first matching element.
 * Two values of different kinds are never equal.
 *
 * Per-kind comparison, and how far each is built out:
 *   - PICO8_VALUE_FIXED: compared numerically (raw 16.16 equality).
 *   - PICO8_VALUE_BOOL:  compared as plain booleans.
 *   - PICO8_VALUE_REF:   compared by pointer identity.
 *   - PICO8_VALUE_STRING: compared by pointer identity, NOT by content
 *     (no strcmp). Every del() call site found by grepping lua.lua in
 *     this phase's scope (pico8/lua.lua:163,164,167,484,676,704,740 --
 *     he()'s del(hv,e)/del(cv[p],e)/del(cg[t],e), and four more inline
 *     uses) deletes an entity/table reference, never a string or a
 *     number -- there is no real call site in scope that ever needs
 *     "equal string content, different pointer" to count as a match.
 *     Pointer-identity comparison is therefore all this phase's evidenced
 *     usage needs; content comparison is deliberately not built, per the
 *     PRD's Out of Scope guidance against speculative generality. If a
 *     future phase's del() call needs by-content string equality, that's
 *     a real, evidenced need to extend this function for at that point,
 *     not one to guess at now. */
int pico8_value_equal(pico8_value_t a, pico8_value_t b);

/* tostr(v): converts any of the shared value type's kinds to a
 * newly heap-allocated, NUL-terminated string the caller owns and must
 * free() -- tostr() produces a new, independent Lua string value, the
 * same ownership convention pico8_sub (pico8_str.h) uses.
 *
 * Every tostr() call site found by grepping pico8/lua.lua in this phase's
 * scope (lines 443, 530, 884, 957, 1418, 1420) passes a fixed-point number
 * (a field holding a count/score/id) -- never a string, boolean, or
 * reference. So:
 *   - PICO8_VALUE_FIXED is the kind this phase's real usage actually
 *     needs, and gets a real (if simple) decimal formatting: printed with
 *     4 fractional digits, then trailing zeros and a bare trailing '.'
 *     are trimmed, so an exact integer value like 3 renders "3", not
 *     "3.0000". PICO-8's own exact tostr() decimal-formatting rules for
 *     arbitrary fixed-point values (trailing-zero/precision edge cases)
 *     were not separately sourced beyond "produces a human-readable
 *     decimal" -- this phase's own tests only assert on small integers
 *     this phase itself controls, per the PRD's Out of Scope; a specific
 *     formatted value Phase 3's UI code needs is deferred to whichever
 *     call site first needs it.
 *   - PICO8_VALUE_STRING is never actually passed to tostr() by any
 *     in-scope call site either, but is trivial and unambiguous (identity/
 *     passthrough of the string's content) so it's implemented for
 *     completeness rather than left to assert/crash.
 *   - PICO8_VALUE_BOOL and PICO8_VALUE_REF are likewise never passed to
 *     tostr() by any in-scope call site; both get a minimal, defensive
 *     result (BOOL: "true"/"false"; REF: an empty string) rather than a
 *     researched PICO-8-accurate formatting, since no real call site in
 *     scope exercises either -- a real future need is evidence to build
 *     that out properly, not something to guess at now. */
char *pico8_value_tostr(pico8_value_t v);

#endif
