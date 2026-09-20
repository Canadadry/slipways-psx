#ifndef PICO8_TRADE_H
#define PICO8_TRADE_H

#include "pico8_class.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Phase 3, Chunk 3 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `trade`, a value-object representing one trade route (a chain
 * of planets `lp` connected by slipways `rl`, carrying one resource `qe`).
 * Confirmed by reading the real source (not assumed from the PRD's own
 * framing, which raised the question of whether trade even has instances
 * via the class system): `trade=kv:jr()` -- trade IS declared through the
 * same real class system as ke/lq/jb (a root class, `kv:jr()`, no spec
 * string), NOT a plain ad-hoc Lua table built by hand each time. Every
 * real trade IS an instance built via pico8_trade_new (this file's own
 * `trade(...)` equivalent, mirroring pico8_lq_new/pico8_jb_new's
 * established convention, pico8_layout.h). Not ke-descended, so (like
 * lq/jb) trade never self-registers into hv/cv/cg, and has no `pq` of its
 * own (never assigned anywhere in the real source).
 *
 * trade=kv:jr()function trade:to()
 * return self.lp[#self.lp]
 * end
 * function trade:kg(np,jc)
 * return trade({lp=la(self.lp,{np}),rl=la(self.rl,{jc}),qe=self.qe
 * })end
 * function trade.ns.__eq(l,r)
 * return l.lp[1]==r.lp[1]
 * and l:to()==r:to()end
 * (pico8/lua.lua:603-611.)
 *
 * `trade.ns.__eq`: a real Lua metamethod (custom `==` on trade instances),
 * assigned onto the class's own precomputed instance metatable (`.ns`,
 * `{__index=rj}`) rather than the class table itself -- but per
 * pico8_class.h's own top comment, `.ns` has no C equivalent here (this
 * port achieves the same instance-metatable role via
 * pico8_instance_t.cls plus pico8_instance_get_field's own live chain
 * walk instead). There is no native operator-overloading mechanism in
 * this port for the same reason Phase 2's `km` vector type has none (see
 * pico8_vec.h's own top comment on why pico8_vec_add/sub/mul/div/neg are
 * plain named functions, not `operator+` etc.): pico8_trade_eq below
 * follows that exact same precedent -- a plain named comparison function,
 * not an attempt at real `==` overloading in C. Nothing in this chunk's
 * own real scope calls it yet (the trade-propagation cluster that would,
 * `jo:cn`/`jc:eb`/etc., is Chunks 13-15 -- see this PRD's own "mutually-
 * recursive trade cluster" Implementation Decision); a documented,
 * evidence-bounded gap for that future cluster to note: this port's
 * generic list-membership helpers (`pico8_gu`/`pico8_value_equal`,
 * pico8_value.h/pico8_collect.h) compare a PICO8_VALUE_REF held for a
 * trade instance by pointer identity, NOT by pico8_trade_eq's own
 * semantic "same start and end planet" rule -- exactly the same
 * "generic list helper isn't aware of a custom __eq" gap `pico8_gu`'s own
 * header comment already flagged for Lua string interning (Chunk 1); a
 * future chunk that needs `gu()`/`del()`-style membership on a list of
 * trade instances needs its own pico8_trade_eq-aware sibling (matching
 * pico8_gu_content's own precedent), not a silent reuse of the generic
 * one. */
void pico8_trade_init(void);
void pico8_trade_shutdown(void);

/* The real `trade` root class, built once by pico8_trade_init(). Mirrors
 * pico8_ke_class()/pico8_lq_class()'s own convention -- not retrievable
 * via pico8_class_lookup() (registers under the shared "" name-registry
 * slot, like ke/lq/jb -- see pico8_ke.h's own comment on why). Returns
 * NULL before pico8_trade_init() (or after pico8_trade_shutdown()). */
pico8_class_t *pico8_trade_class(void);

/* `trade(overrides)` -- see this file's own top comment. */
pico8_instance_t *pico8_trade_new(const pico8_ob_table_t *overrides);

/* function trade:to()
 * return self.lp[#self.lp]
 * end
 * (pico8/lua.lua:603-605.) Returns 1 and writes *out_value when self has a
 * non-empty "lp" list field, matching pico8_instance_get_field's own
 * "absent" convention; 0 (out_value untouched) for a NULL self, a missing/
 * wrong-kind "lp" field, or an EMPTY "lp" (`self.lp[#self.lp]` on an empty
 * list would be `self.lp[0]`, Lua nil -- not evidenced by any real trade,
 * every one of which is seeded with a one-element lp before :kg() ever
 * runs, pico8/lua.lua:504, Chunk 13/14 -- not yet ported; defensive rather
 * than an out-of-bounds C array read). */
int pico8_trade_to(const pico8_instance_t *self, pico8_value_t *out_value);

/* function trade:kg(np,jc)
 * return trade({lp=la(self.lp,{np}),rl=la(self.rl,{jc}),qe=self.qe
 * })end
 * (pico8/lua.lua:606-608.) Extends self's own route by one more planet
 * (np) and slipway (jc), keeping the same resource (qe); returns a brand
 * new trade instance (self is never mutated, matching the real Lua's own
 * `return trade({...})`, which always constructs fresh). A NULL self
 * returns NULL; a missing "lp"/"rl" field on self reads as an empty list
 * (not evidenced by any real trade, see pico8_trade_to's own comment) --
 * matches this port's own established defensive-default convention rather
 * than crashing. */
pico8_instance_t *pico8_trade_kg(const pico8_instance_t *self, pico8_value_t np, pico8_value_t jc);

/* function trade.ns.__eq(l,r)
 * return l.lp[1]==r.lp[1]
 * and l:to()==r:to()end
 * (pico8/lua.lua:609-611.) See this file's own top comment for why this
 * is a plain named function, not operator overloading, and for the
 * documented gap against this port's generic list-membership helpers.
 * Two trades are equal iff they start (`lp[1]`) AND end (`:to()`) at the
 * same planet -- compared by pico8_value_equal (pointer identity for
 * PICO8_VALUE_REF, i.e. the same planet instance), regardless of what's
 * in between. Returns 0 (not equal) for a NULL l/r, a missing/wrong-kind/
 * empty "lp" on either side, matching this port's own established
 * defensive-default convention. */
int pico8_trade_eq(const pico8_instance_t *l, const pico8_instance_t *r);

#endif
