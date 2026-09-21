#ifndef PICO8_COLLECT_H
#define PICO8_COLLECT_H

#include "pico8_geom.h"
#include "pico8_list.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Phase 3, Chunk 1 (docs/prd/triage/phase3-etape1-function-by-function-c-port.md),
 * first sub-lot: pico8/lua.lua's own small, generic, top-level table/list
 * helper functions (ft, qj, mw, gu, jp, nv, la) plus the constructor-
 * dispatch helper (oo) the ob()/pi mechanism uses. None of these depend on
 * anything beyond Phase 1/2 primitives (add/all/foreach) and the shared
 * pico8_list_t/pico8_ob_table_t/pico8_value_t types already built by
 * Phase 2 (pico8_list.h/pico8_ob.h/pico8_value.h) -- no entity/global-state
 * dependency, so this is a self-contained module distinct from pico8_ob.c
 * (which is already at/above the ~430-540-line size flagged by warn-review
 * -- see docs/portage-progress.md's note on this module for why `oo`,
 * despite being pi/ob-adjacent, lives here rather than growing pico8_ob.c
 * further). `rc`/`o`/`mr`/`a` (Lua tables, possibly with named AND
 * positional keys) are represented by pico8_ob_table_t; `qk`/`a`/`b`
 * (Lua lists, built via add()) are represented by pico8_list_t -- matching
 * how each Lua value in these functions' real call sites is actually
 * built/iterated (a table indexed by pairs()/rc[k], vs. a list walked by
 * all()/foreach()). */

/* function ft(rc,nt,pu)rc[nt]=rc[nt]or {}
 * add(rc[nt],pu)end
 * Appends pu to the bucket rc[nt], creating an empty bucket first if rc[nt]
 * doesn't exist yet. `nt` (the bucket key) is either a Lua string (e.g.
 * lt()'s ft(cv,p,e), a field name) or a Lua number (e.g. dc()'s
 * ft(fl,qc.fa,qc), a priority/order value) at different real call sites,
 * so this port -- like pico8_ob_table_t's own set_index/set_name split --
 * exposes two entry points rather than one polymorphic-key function.
 * A bucket is a heap-allocated pico8_list_t stored in rc via a
 * PICO8_VALUE_REF, following pico8_ob_table_t's own established borrowed-
 * pointer convention: rc does not know how to free it, exactly like a
 * nested pico8_ob_table_t reached through a REF value (see pico8_ob.h). */
void pico8_ft_index(pico8_ob_table_t *rc, int nt, pico8_value_t pu);
void pico8_ft_name(pico8_ob_table_t *rc, const char *nt, pico8_value_t pu);

/* function qj(o,mr)for k,v in pairs(mr or {})do
 * o[k]=v
 * end
 * return o
 * end
 * Merges mr INTO o (mr's values win on a shared key -- see pico8_ob.c's
 * own comment on ob()'s identical merge direction, confirmed there against
 * every real qj() call site including this function's own two others,
 * mw(o) and hy:pq()'s qj(self,ob(...))) and returns o. mr may be NULL
 * (mirrors `mr or {}`: nothing to merge). This is the exact same merge
 * pico8_ob.c's own pico8_ob() already performs inline for its `defaults`
 * argument; not refactored to share code with that already-large,
 * already-tested module (see this file's top comment) -- a small,
 * intentionally accepted duplication, the same kind of tradeoff Phase 2's
 * own warn-review notes already logged and accepted elsewhere (see
 * docs/plan.md's "Notes de portage"), noted here rather than silently
 * left unremarked. */
pico8_ob_table_t *pico8_qj(pico8_ob_table_t *o, const pico8_ob_table_t *mr);

/* function mw(o)
 * return qj({},o)end
 * Shallow-copies o into a freshly allocated table (merge o into an empty
 * table). The returned table is heap-allocated and owned by the caller,
 * released with pico8_ob_table_free() like any other pico8_ob_table_t. */
pico8_ob_table_t *pico8_mw(const pico8_ob_table_t *o);

/* function gu(qk,kb)for e in all(qk)do
 * if (e==kb)return true
 * end
 * end
 * True iff kb is a member of list qk, compared with Lua's plain `==`
 * (ported here as pico8_value_equal -- see pico8_value.h's own comment on
 * that function's per-kind rules). qk==NULL mirrors PICO-8's own
 * documented all(nil) contract (no iteration, not an error), matching
 * real call sites that can pass an absent list (e.g. ho()'s gu(gp,p) with
 * gp omitted). NOTE, flagged rather than silently worked around: at least
 * one real call site (ho(), Chunk 1's later sub-lot) compares Lua STRING
 * values with gu() and, because Lua interns all string literals, actually
 * needs CONTENT equality there -- but pico8_value_equal deliberately
 * compares PICO8_VALUE_STRING by pointer identity only, per its own
 * documented, evidenced-at-the-time Phase 2 scope decision (see
 * pico8_value.h and the existing
 * test_equal_string_by_pointer_identity_not_content unit test). Since no
 * real call site in *this* sub-lot's own scope exercises that gap, it is
 * documented here and in docs/portage-progress.md rather than changed
 * now -- widening pico8_value_equal's string rule is a real decision for
 * whichever chunk first actually exercises ho()'s real call sites
 * (Chunk 11), not one to make speculatively here. */
int pico8_gu(const pico8_list_t *qk, pico8_value_t kb);

/* function jp(qk,ow)local f={}
 * for e in all(qk)do
 * if (ow(e))add(f,e)
 * end
 * return f
 * end
 * Filters qk, keeping every element e for which ow(e) is truthy. ow is a
 * predicate over one pico8_value_t plus a ctx pointer (C's stand-in for a
 * captured Lua upvalue, matching pico8_list_foreach_fn's own convention --
 * see pico8_list.h). Returns a new, heap-backed list the caller owns and
 * must release with pico8_list_free(). */
typedef int (*pico8_jp_predicate_fn)(pico8_value_t e, void *ctx);
pico8_list_t pico8_jp(const pico8_list_t *qk, pico8_jp_predicate_fn ow, void *ctx);

/* function nv(qk,kp)local m={}
 * for e in all(qk)do
 * add(m,kp(e)or nil)end
 * return m
 * end
 * Maps qk through kp, but `kp(e) or nil` means a falsy/nil result from kp
 * contributes NOTHING to m (add(t,nil) adds no element) rather than a nil
 * placeholder -- exactly the "collect non-nil results" shape pico8_fi()
 * already established (see pico8_ob.h's pico8_fi comment) for the same
 * reason (pico8_value_t has no nil kind), so kp is ported the same way:
 * writes *out_value and returns 1 to contribute a value, 0 to contribute
 * nothing for that element. Returns a new, heap-backed list the caller
 * owns and must release with pico8_list_free(). */
typedef int (*pico8_nv_fn)(pico8_value_t e, void *ctx, pico8_value_t *out_value);
pico8_list_t pico8_nv(const pico8_list_t *qk, pico8_nv_fn kp, void *ctx);

/* function la(a,b)local t={}
 * local kg=function(e)add(t,e)end
 * foreach(a,kg)foreach(b,kg)
 * return t
 * end
 * Concatenates a then b into a new list (a's elements first, in order,
 * then b's). Ported via pico8_list_foreach (not a plain loop) to mirror
 * the original's own use of foreach() rather than an explicit for-in,
 * even though no real call site in scope needs foreach()'s specific
 * (lack of) deletion-tolerance here. Returns a new, heap-backed list the
 * caller owns and must release with pico8_list_free(). */
pico8_list_t pico8_la(const pico8_list_t *a, const pico8_list_t *b);

/* function oo(fn,a)
 * return fn
 * and fn(a[1],a[2],a[3],a[4],a[5])or a
 * end
 * If fn is non-nil, calls it with a's first five POSITIONAL entries
 * (a[1]..a[5], each defaulting to Lua nil -- pico8_value_ref(NULL) here,
 * matching pico8_ob.c's own identical convention for a missing
 * constructor argument) and returns fn's result; otherwise returns `a`
 * itself unchanged (oo's own `or a` fallback), wrapped as a
 * PICO8_VALUE_REF so a's identity/lifetime are preserved exactly as the
 * original's plain table reference would be. `fn` is a
 * pico8_ob_constructor_fn (pico8_ob.h) -- the exact shape ob()'s own `pi`
 * constructor-dispatch table already uses, since oo(pi[sc],...) is where
 * the real Lua calls this. pico8_ob.c's own "identifier(...)" parsing
 * branch already implements this identical fn-call-or-fallback logic
 * inline (built before this function existed, Phase 2); see this file's
 * top comment for why it isn't refactored to share this new function
 * (avoiding further growth of an already zoomed-in module), an accepted,
 * documented duplication rather than a silent one.
 * NOTE, flagged rather than silently worked around (same treatment as
 * gu()'s note above): real Lua's `fn and fn(...) or a` falls back to `a`
 * whenever fn(...) itself returns a FALSY result (nil or false), not just
 * when fn is nil -- but this port only special-cases fn==NULL and
 * otherwise returns fn(...)'s result unconditionally, with no "or a"
 * fallback for a falsy call result. This exactly mirrors the pre-existing,
 * already-shipped inline duplicate of this same logic in pico8_ob.c's
 * pico8_ob_value_for_token (see above), so it isn't a new gap -- but it
 * is not exercised by any real call site today, since no constructor
 * currently registered in the pi/char-keyed table can return a falsy
 * result. This becomes a real gap once a void-returning host builtin like
 * spr (registered as 's' per docs/prd/triage/phase3-etape1-...md's
 * Chunk 2 decisions) is wired in as a constructor -- resolve then, not
 * speculatively here. */
pico8_value_t pico8_oo(pico8_ob_constructor_fn fn, const pico8_ob_table_t *a);

/* Phase 3, Chunk 1, second sub-lot's own additions to this module: ne, ho,
 * plus two small pieces of ob()/pi registry glue (pico8_gu_content,
 * pico8_qz_register_constructor) this sub-lot's own work surfaced a real
 * need for -- see each declaration's own comment below. */

/* gu()/pico8_value_equal's PICO8_VALUE_STRING comparison is pointer
 * identity, correct for every real del()-based call site pico8_value.h's
 * own comment evidences (pico8_gu, this module) -- but this port's own
 * ob() (pico8_ob.c's pico8_ob_value_for_token) allocates a FRESH string
 * for every quoted token it parses, never interning, unlike real Lua where
 * an identical string literal genuinely IS the same object. ho()'s real
 * gu(gp,p) call (see pico8_ho below) compares a field-name string parsed
 * out of one ob() table (`mr`) against entries of a SEPARATE list (`gp`,
 * e.g. the real `{"nk"}` at pico8/lua.lua:1443) -- two independently-
 * produced strings that are never the same pointer even when equal in
 * content, so pico8_gu's pointer-identity rule would silently never match
 * here. Rather than widen pico8_value_equal's own documented, tested
 * contract (still correct for its own callers) this is a narrowly-scoped
 * sibling: same list-membership shape as pico8_gu, but PICO8_VALUE_STRING
 * pairs are compared by content (strcmp); every other kind still goes
 * through pico8_value_equal unchanged. This is the resolution for the gap
 * pico8_gu's own header comment (above) flagged as left open for "whichever
 * chunk first needs it" -- this sub-lot's own ho() is that chunk. */
int pico8_gu_content(const pico8_list_t *qk, pico8_value_t kb);

/* function ne(d,c,ku)if d>=0 then
 * d="+"..d
 * else
 * c=8
 * end
 * return d..(ku or ""),c
 * end
 * Signed-number formatting with a forced color for negatives: d>=0 gets a
 * "+" prefix and keeps caller's own color `c`; d<0 keeps its own natural
 * "-" sign (from pico8_value_tostr's own PICO8_VALUE_FIXED formatting,
 * reused rather than duplicating number-to-string logic -- see this
 * module's own top comment on avoiding duplicated helpers) and forces
 * color 8 (red). `ku` (an optional trailing unit suffix, e.g. "$") is
 * appended last, defaulting to "" when NULL (`ku or ""`). Returns a
 * newly heap-allocated string the caller owns and must free() (matching
 * pico8_value_tostr/pico8_sub's own ownership convention) and writes the
 * resulting color to *out_color. `c` itself has no faithful "omitted"
 * representation in a plain `int` parameter; every real call site that
 * omits it (`ne(self.kr)`, `ne(fm.ge)`) only ever uses ne()'s FIRST
 * return value afterward, so *out_color's value when d>=0 and no
 * meaningful `c` was on hand is never actually observed by any real,
 * evidenced call site -- callers with no real `c` may pass any int (e.g.
 * 0) for it. */
char *pico8_ne(pico8_fixed_t d, int c, const char *ku, int *out_color);

/* function ho(rc,mr,gp)
 * return function(ri)for i,p in pairs(mr)do
 * ri[p]=gu(gp,p)and r[ri[i]]or ri[i]
 * end
 * rc[ri.px]=ri
 * end
 * end
 * A positional-to-named field-renaming pass, used as
 * `foreach(df,ho(df,mr,gp))`/`foreach(nb,ho(nb,mr,gp))` (Chunk 11) to
 * rename each raw ob()-parsed data record `ri` in place (adding named
 * fields alongside its existing positional ones, per `mr`'s index->name
 * map) and register it into `rc` keyed by its own new `.px` field. Ported
 * as this file's own established "closure factory -> context + a function
 * taking that context's own captured values as plain parameters, applied
 * once per record" pattern (matching pico8_jp/pico8_nv's ctx convention),
 * rather than a native C closure: pico8_ho_apply IS the returned
 * function's own body, called once per `ri` by whatever later ports
 * ho()'s own foreach() call sites (Chunk 11, out of this sub-lot's scope).
 *
 * `mr` is always purely positional in every real ho() call site in scope
 * (its own index->name map, e.g. ob([["px","lv","nk","od","oy",]])), so
 * only PICO8_OB_KEY_INDEX entries of `mr` are walked. `gp` (`gu(gp,p)`'s
 * own list, e.g. the real `{"nk"}` literal at pico8/lua.lua:1443) may be
 * NULL, mirroring the real ho()'s own optional third parameter (`ho(df,
 * mr)`, no gp, at df's own call site) -- pico8_gu_content already treats a
 * NULL list as "never a member" (see pico8_gu's own identical NULL
 * handling). `r[ri[i]]` indexes the global `r` table (pico8_r(),
 * pico8_stash.h, built earlier in this SAME sub-lot) by ri[i]'s own value
 * -- a PICO8_VALUE_FIXED indexes r by number, a PICO8_VALUE_STRING indexes
 * it by name, mirroring pico8_ft_index/pico8_ft_name's own established
 * polymorphic-key split for exactly this "the real Lua key's type isn't
 * known until runtime" situation. `ri[i]` absent (no positional value at
 * that index) is left un-renamed (`ri[p]` simply isn't set), the same
 * defensive "field absence stays absence" divergence pico8_entity.c's
 * dc()/he() already established for a not-really-reachable Lua-would-crash
 * case. */
void pico8_ho_apply(pico8_ob_table_t *rc, const pico8_ob_table_t *mr, const pico8_list_t *gp, pico8_ob_table_t *ri);

/* The `pi` table's `'b'` entry (pico8/lua.lua:133's `pi={b=qz,...}`): an
 * pico8_ob_constructor_fn adapter around pico8_qz (pico8_geom.h),
 * unpacking a1..a4 (always PICO8_VALUE_FIXED at the one real spec-string
 * call site in scope, `hw=b(10,10,118,118)`, ui's own class spec, Chunk 5)
 * into pico8_qz's own four pico8_fixed_t parameters. Registering it lives
 * here (not pico8_geom.c, which stays a pure module with no pico8_ob.h
 * registry dependency, matching this module's own "constructor-dispatch
 * adapters" theme already established by pico8_oo's own header comment)
 * per this sub-lot's own explicit instruction to wire `'b'`->qz into the
 * existing registry now. Like `'v'`'s own still-unregistered status
 * (pico8_ob.h's own comment on pico8_vec_v), nothing in THIS sub-lot's own
 * scope calls pico8_qz_register_constructor() yet -- it is ready for
 * whichever future init sequence (Chunk 2's bl()/game bootstrap, most
 * likely) actually needs pi['b'] resolvable through ob()'s own parser. */
void pico8_qz_register_constructor(void);

#endif
