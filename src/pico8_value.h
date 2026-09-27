#ifndef PICO8_VALUE_H
#define PICO8_VALUE_H

#include "pico8_fixed.h"

/* The one tagged value type every value slot in this port is built on: a
 * list element (pico8_list.h), an ob() table entry (pico8_ob.h), an
 * entity field (pico8_struct.h). Lua has a single dynamically-typed value
 * and this stands in for it, rather than each consumer inventing its own
 * tagged union.
 *
 * There is no nil kind. Where Lua would see nil, this port passes
 * pico8_value_ref(NULL) as a filler; that is not a distinguished absent
 * value and the kinds below do not tell it apart from a real NULL ref. */
typedef enum {
    PICO8_VALUE_FIXED,   /* a PICO-8 fixed-point number */
    PICO8_VALUE_STRING,  /* a C string (not owned by the value; see below) */
    PICO8_VALUE_BOOL,    /* a boolean (0 or 1) */
    PICO8_VALUE_REF,     /* an opaque pointer/reference (e.g. an entity) */
    PICO8_VALUE_FUNCTION /* an entity method; see pico8_value_function_t */
} pico8_value_kind_t;

/* Forward-declared here so pico8_value_function_t below can name it by
 * value; the struct's own definition cannot come first, since it has a
 * member of that same function pointer type. */
typedef struct pico8_value pico8_value_t;

/* An entity method: a field like .kj/.ev/.hd/.hk/.hb, called as
 * `entity[nt](entity,entity.ra)` by dc()/mv() (pico8_entity.h).
 *
 * `self` is a `void *`, not a pico8_struct_header_t *, because pico8_value.h
 * sits below pico8_struct.h in the include graph (pico8_struct.h -> here)
 * and must not depend upward on it; pico8_entity.c casts `self` back
 * itself. Do not "fix" the type.
 *
 * A fixed argument list, not a variadic one: no dispatch site in the cart
 * passes more than two extra arguments. A dispatched function must check
 * `argc` before trusting `arg0`/`arg1` -- a slot argc says is not real
 * holds whatever filler the caller passed, never a distinguished nil. */
typedef pico8_value_t (*pico8_value_function_t)(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* A value never owns the memory a PICO8_VALUE_STRING or PICO8_VALUE_REF
 * points to -- it is a plain tagged union of a scalar or a borrowed
 * pointer, matching Lua's own by-reference semantics for strings/tables
 * (assigning a Lua value never deep-copies a string or table). Callers
 * are responsible for whatever the pointer's lifetime needs to be. */
struct pico8_value {
    pico8_value_kind_t kind;
    union {
        pico8_fixed_t fixed;
        const char *string;
        int boolean;
        void *ref;
        pico8_value_function_t function;
    } as;
};

pico8_value_t pico8_value_fixed(pico8_fixed_t x);
pico8_value_t pico8_value_string(const char *s);
pico8_value_t pico8_value_bool(int b);
pico8_value_t pico8_value_ref(void *p);
pico8_value_t pico8_value_function(pico8_value_function_t fn);

/* "Equal in value", as del() uses it to find the first matching element
 * (PICO-8 Del wiki: https://pico-8.fandom.com/wiki/Del). Two values of
 * different kinds are never equal.
 *
 * Per-kind rules -- everything but numbers and booleans is IDENTITY:
 *   - PICO8_VALUE_FIXED:    numeric (raw 16.16) equality.
 *   - PICO8_VALUE_BOOL:     plain boolean equality.
 *   - PICO8_VALUE_REF:      pointer identity.
 *   - PICO8_VALUE_FUNCTION: function-pointer identity.
 *   - PICO8_VALUE_STRING:   POINTER identity, not content -- no strcmp.
 *     Lua interns its literals, so there two equal strings are the same
 *     object; this port allocates a fresh string per parsed token, so
 *     they are not. A caller that wants content equality must ask for it
 *     explicitly: pico8_gu_content (pico8_collect.h) exists for exactly
 *     that, and several modules compare string contents themselves. Two
 *     neighbouring call sites can therefore legitimately differ on which
 *     comparison they use; neither is to be "fixed" after the other. */
/* TROU-3 */
int pico8_value_equal(pico8_value_t a, pico8_value_t b);

/* tostr(v): renders any kind to a newly heap-allocated, NUL-terminated
 * string the caller owns and must free() -- the ownership convention
 * pico8_sub (pico8_str.h) also uses.
 *
 * FIXED is printed with 4 fractional digits rounded half to even, then
 * trailing zeros and a bare trailing '.' are trimmed, so an exact 3
 * renders "3" and not "3.0000". This reproduces 13 of the 14 block-C
 * witnesses of tools/probe_out.txt; magnitudes below 0.0001 are TROU-22.
 * STRING passes its content through, BOOL renders "true"/"false",
 * and REF and FUNCTION render the empty string. The empty string is also
 * what an absent field yields, since this port spells "absent" as
 * pico8_value_ref(NULL) and PICO-8 would print nil there. */
/* TROU-13 */
char *pico8_value_tostr(pico8_value_t v);

#endif
