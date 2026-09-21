#ifndef PICO8_PI_H
#define PICO8_PI_H

#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_ob.h"

/* Phase 3, Chunk 2 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): the `pi` table's SECOND consumer plus bl() itself. Confirmed
 * real source (pico8/lua.lua:133): `pi={b=qz,v=v,br=rk,qr=io,rf=eq,s=spr}`.
 *
 * Per this PRD's own Implementation Decisions ("The pi table has two
 * independent consumers needing two separate lookup tables"): ob()'s own
 * single-character `sc`-based nested-constructor branch is already served
 * by Phase 2's char-keyed pico8_ob_register_constructor/
 * pico8_ob_lookup_constructor (pico8_ob.h) -- only 'b' and 'v' (and, for
 * completeness, 's') are ever reachable through THAT path, since `sc` is
 * always exactly one character. bl()'s OWN `oo(pi[e.fn],e)` call (see
 * pico8_bl below) indexes `pi` by the FULL STRING held in an entity's
 * `.fn` field ("br", "qr", "rf", "ld", "s") -- a second, independent,
 * genuinely string-keyed lookup, which is what this module adds. */

/* String-keyed registry serving bl()'s own oo(pi[e.fn],e) path. Reuses
 * pico8_ob_constructor_fn (pico8_ob.h) as-is for the registered value
 * type -- exactly the shape oo() needs (it unpacks an entity's first five
 * positional entries and calls the looked-up function with them,
 * pico8_collect.h's own pico8_oo) -- so no new function-pointer type is
 * invented here, per this chunk's own instructions.
 *
 * NOT built on pico8_ob_table_t, this project's other established generic
 * key/value container (checked first, per this phase's duplication-grep
 * policy): a pico8_ob_table_t's value slots are pico8_value_t, whose only
 * function-pointer member (PICO8_VALUE_FUNCTION's pico8_value_function_t,
 * `pico8_value_t(*)(void*,int,pico8_value_t,pico8_value_t)` -- Chunk 10's
 * mv()/dc() shape, extended to this argc-plus-two-slots form by Phase 3
 * Chunk 5, pico8_value.h) is a genuinely different function-pointer type
 * than
 * pico8_ob_constructor_fn (`pico8_value_t(*)(pico8_value_t,pico8_value_t,
 * pico8_value_t,pico8_value_t,pico8_value_t)`) -- storing one as the other
 * would need an unsound function-pointer-to-object-pointer cast, not a
 * real reuse. This is therefore a small, separate, purpose-built table --
 * but deliberately NOT a third independent "doubling-capacity growable
 * array" (the duplicated pattern already flagged in pico8_value.c/
 * pico8_ob.c/pico8_class.c/pico8_entity.c, see docs/plan.md's Notes de
 * portage): this table's real name set is fixed and fully evidenced by
 * `pi`'s own one real Lua table literal (5 entries: "br","qr","rf","ld",
 * "s") and every registration happens once, at startup, from this
 * module's own pico8_pi_init() below -- nothing in scope ever grows this
 * table at runtime. A small fixed-capacity array with linear strcmp
 * lookup, sized with a little headroom, is therefore a plain,
 * non-speculative fit -- closer in spirit to pico8_ob.c's own fixed
 * 256-slot char-keyed array (pico8_ob_constructors) than to the flagged
 * growable-doubling pattern. */
#define PICO8_PI_DISPATCH_CAPACITY 8

void pico8_pi_dispatch_register(const char *name, pico8_ob_constructor_fn fn);
pico8_ob_constructor_fn pico8_pi_dispatch_lookup(const char *name);
/* Test-support only (not part of the Lua original): resets the table to
 * empty, mirroring pico8_ob_clear_constructors/pico8_class_registry_clear's
 * established test-isolation convention. */
void pico8_pi_dispatch_clear(void);

/* function bl(rb,nc)for e in all(rb)do
 * if nc then
 * camera(nc,0)nc=-nc
 * end
 * oo(pi[e.fn],e)end
 * end
 * (pico8/lua.lua:597-602.) For each entity e in list rb: if a camera-shake
 * offset `nc` was supplied, sets camera(nc,0) then flips nc's sign for the
 * NEXT iteration (so successive entities alternate a shake left/right);
 * then dispatches through the string-keyed `pi` table by e's own `.fn`
 * field, exactly as pico8_oo (pico8_collect.h) already does for ob()'s own
 * char-keyed path -- oo(NULL,e) (e.fn absent, or present but not yet
 * registered -- see pico8_pi_init()'s own note on "ld") is a documented,
 * faithful no-op: bl() itself discards oo()'s return value either way, so
 * pico8_oo's own already-flagged "or a" fallback gap (pico8_collect.h)
 * never actually matters here.
 *
 * `rb` is a pico8_list_t here, matching every OTHER real all()-consuming
 * standalone function this chunk depends on (pico8_gu/pico8_jp/pico8_nv/
 * pico8_la, Chunk 1) -- the project's one established concrete
 * representation for "list iterated via all()". One of bl()'s three real
 * call sites (`mh:ev()`'s `bl(self,nc)`, Chunk 8, not yet ported) actually
 * passes an INSTANCE directly (Lua's `all()` walking the instance table's
 * own array part), which this signature doesn't accept as-is; that is
 * Chunk 8's own call-site problem to solve (e.g. give mh a genuine
 * pico8_list_t field to add() onto, or add a from-instance list-view
 * helper), flagged here rather than guessed at now, since neither of
 * this chunk's own two other real call sites (`bl(la(pj.i_,pj.ou))`,
 * `bl(self.rb)`) has this shape.
 *
 * `e` itself (each list element) is expected to be a PICO8_VALUE_REF to a
 * pico8_ob_table_t -- oo()'s own a[1]..a[5] positional unpacking only
 * makes sense against that type (pico8_collect.h's pico8_oo), and every
 * real bl() call site builds its list from ob()-parsed icon/button specs
 * exactly shaped that way.
 *
 * `nc` needs to distinguish "provided" from Lua nil (nc==0 is a real,
 * TRUTHY fixed-point value in Lua, not "no shake"), so this follows this
 * exact subsystem's own already-established two-entry-point convention
 * for an optional trailing Lua argument (pico8_hud.h's pico8_dv/
 * pico8_dv_default, pico8_rk/pico8_rk_y1, pico8_io/pico8_io_default):
 * pico8_bl_default(gfx,mem,rb) is the omitted-nc case (`bl(la(...))`/
 * `bl(self.rb)`), matching the real Lua's own "nc stays nil for every
 * iteration once it starts nil" invariant (the `if nc then...end` branch
 * only ever reassigns nc when it was already truthy). `gfx`/`mem` are
 * threaded through explicitly (matching every other Phase-1-primitive-
 * touching function in this area, e.g. pico8_rk/pico8_eq/pico8_io) so
 * bl() can call camera() directly; they are ALSO stashed into this
 * module's own small internal draw-context (see pico8_pi.c) for the
 * string-keyed adapters (br/qr/rf/s) to read when oo() calls through to
 * them, since pico8_ob_constructor_fn's fixed 5-pico8_value_t-argument
 * shape has no room to carry gfx/mem itself -- see pico8_pi.c's own top
 * comment for why this is a real, new problem this chunk surfaces (not a
 * pre-existing pattern), and its scope (bl() sets it on every call; a
 * spec string that invoked `s(...)` through ob()'s OWN char-keyed path,
 * which never goes through bl(), would see whatever context bl() last
 * set, or none at all -- moot today since no real spec string does this,
 * per this PRD's own Implementation Decisions). */
void pico8_bl(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_list_t *rb, pico8_fixed_t nc);
void pico8_bl_default(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_list_t *rb);

/* Registers every `pi` entry this phase has a real, already-ported target
 * function for, into whichever of the two registries actually needs it:
 *   - char-keyed (pico8_ob.h, ob()'s own path): 'b'->qz (Chunk 1's own
 *     pico8_qz_register_constructor(), not yet called by anyone before
 *     this function), 'v'->pico8_vec_v, 's'->spr (registered "for
 *     completeness" per this PRD's own Implementation Decisions -- no
 *     confirmed real spec string invokes it via `s(...)` construction
 *     syntax).
 *   - string-keyed (this module's own pico8_pi_dispatch_register, bl()'s
 *     path): "br"->pico8_rk, "qr"->pico8_io, "rf"->pico8_eq, "s"->spr
 *     (same adapter function as the char-keyed 's' registration above,
 *     not duplicated).
 *   - "ld"->lq's `ol` method: RESOLVED by Chunk 3 (pico8_pi.c's own
 *     pico8_pi_ld_constructor, registered by this same pico8_pi_init()
 *     below) now that `lq`/`lq:ol` exist (src/pico8_layout.h/.c). This was
 *     Chunk 2's own deliberately-left-open gap (`lq` didn't exist yet at
 *     Chunk 2 time) -- see pico8_pi.c's own comment on
 *     pico8_pi_ld_constructor for the full trace.
 *
 * A single grouped init function (rather than N scattered standalone call
 * sites) mirrors pico8_entity_init()'s own "groups related setup calls"
 * precedent (Phase 2) -- one obvious, discoverable place for whichever
 * future bootstrap sequence (Chunk 6's completed jl(), most likely) needs
 * to wire `pi` up once, rather than requiring every future chunk to
 * remember its own individual registration call site. Not yet called by
 * anything in this chunk's own scope -- same "ready but unwired" status
 * this project already established for pico8_qz_register_constructor()
 * itself (Chunk 1) and for 'v' (Phase 2's own pico8_ob.h comment). Safe to
 * call more than once (each pico8_ob_register_constructor/
 * pico8_pi_dispatch_register call simply overwrites its own slot). */
void pico8_pi_init(void);

#endif
