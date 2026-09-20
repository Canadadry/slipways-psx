#ifndef PICO8_ENTITY_GEOM_H
#define PICO8_ENTITY_GEOM_H

#include "pico8_class.h"
#include "pico8_list.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Phase 3, Chunk 1, second sub-lot (docs/prd/triage/phase3-etape1-
 * function-by-function-c-port.md): pico8/lua.lua's standalone helpers that
 * read/write a LIVE entity instance's own vector-valued fields (`.ra`/`.p`
 * position, `.kq` a qz() box predicate, `.fa`/`.t`/`.ma` plain fixed
 * fields) -- gk, gd, na, ma, ij. Kept in their own module, separate from
 * pico8_collect.c (plain-value/list glue with no pico8_instance_t
 * dependency) and pico8_geom.c (pure vector geometry with no entity/global
 * dependency), matching this chunk's own "name a module after the real
 * shared concern" instruction.
 *
 * Establishes, for the first time in this port, the concrete
 * representation for a Lua `v(x,y)` vector value STORED AS AN ENTITY
 * FIELD: PICO8_VALUE_REF pointing to a heap-allocated pico8_vec_t --
 * exactly what pico8_value.h's own top comment already anticipated
 * ("v(...)/nested-table values are opaque references here"), just not yet
 * exercised by any function until this one. A field never owns the
 * pico8_vec_t it references, mirroring every other PICO8_VALUE_REF's
 * established borrowed-pointer convention (pico8_value.h).
 *
 * Field-absence handling follows the SAME defensive, documented divergence
 * already established by pico8_entity.c's own dc()/he() (".fa-less entity
 * dropped from every bucket", "missing cg[t] bucket is a no-op") rather
 * than reproducing real Lua's "index a nil value" crash: a missing vector
 * field reads as the zero vector, a missing fixed field reads as 0, and a
 * missing qz() box field makes gk() return false. None of these are
 * exercised by any real, evidenced call site yet (gk/gd/na/ma are all only
 * reachable once Chunks 4/6/13/15's own classes/singletons exist), so this
 * is the same "compiles, not yet meaningfully testable" forward-reference
 * treatment already used for bg:ldown/mz, bv/dl/d_'s fm:lu(), etc. */

/* function gk(e,mp,rp)
 * return e.kq((e.fa>=10 and rp or mp)-e.ra
 * )end
 * `js.hd=gk` (Chunk 4) makes this js's own "hd" (hover/hit-test) dispatch
 * method: e.kq is a qz()-built box predicate (pico8_qz_box_t, stored as a
 * PICO8_VALUE_REF field, see pico8_geom.h), tested against either rp or mp
 * (both REF-to-pico8_vec_t values, matching mv()'s real 2-extra-argument
 * `mv(e,"hd",self.qa,je.ra)` call site, Chunk 5's own mv() arity
 * extension) minus the entity's own position. Returns a PICO8_VALUE_BOOL,
 * matching the real Lua boolean the box predicate itself returns. */
pico8_value_t pico8_gk(const pico8_instance_t *e, pico8_value_t mp, pico8_value_t rp);

/* function gd(e)
 * return abs(e.ra.x-mo.ra.x-64)<=76 and abs(e.ra.y-mo.ra.y-64)<=76
 * end
 * Screen-visibility test: is e within 76px of the camera's centered 128px
 * viewport. `mo` (pico8_mo(), pico8_entity.h) is Phase 2's own bare
 * placeholder until Chunk 6 makes it a real qb() camera instance with a
 * real `.ra` field -- until then this reads mo.ra as absent (zero vector,
 * per this file's own top comment), same forward-reference treatment as
 * bg:ldown's mz/g_. Returns a plain C int (0/1), not a pico8_value_t: gd()
 * is always called directly (`if gd(self)`, never through mv()/dc()) at
 * every real call site found in scope. */
int pico8_gd(const pico8_instance_t *e);

/* function na(self,t)
 * self.ra.y+=sin(t*0.005)*0.06
 * end
 * Vertical bobbing: mutates self.ra's own y component IN PLACE through the
 * heap pico8_vec_t the `.ra` field references (no metatable __newindex
 * customizes plain field writes in the original, so a real `self.ra=...`
 * would rebind the field, but `self.ra.y=...` -- as here -- mutates the
 * SAME vector object every other reader of self.ra also sees, exactly
 * mirrored by mutating the pointee in place here). `t` is the per-tick
 * counter dc()/dg() pass as the dispatched method's own argument (see
 * pico8_entity.c's dc()), not a field -- a plain parameter here too. */
void pico8_na(pico8_instance_t *self, pico8_fixed_t t);

/* function ma(p,e)e.ma=max(0,e.ma-0.1)
 * return p+qm(e.ma,e.t*0.2)end
 * Particle-halo decay + polar shift: decrements e.ma (clamped at 0,
 * writing the new value back onto e's own instance table -- a plain field
 * write, not a vector-component mutation, so this goes through
 * e->fields via pico8_ob_table_set_name, same as any other instance field
 * write in this port) and returns p offset by a unit vector at angle
 * e.t*0.2 scaled by the now-decremented e.ma (pico8_vec_qm, pico8_vec.h). */
pico8_vec_t pico8_ma(pico8_vec_t p, pico8_instance_t *e);

/* function ij(p,j_)local mk,nd=32767
 * for o in all(j_)do
 * local d=#(p-o.p)if p~=o.p and d<mk then
 * nd,mk=o,d
 * end
 * end
 * return mk,nd
 * end
 * Nearest-neighbor search: `j_` is a list of entities (e.g. cg.jo/cg.np,
 * pico8_entity.h's pico8_cg()), each read through its own `.p` field
 * (distinct from `.ra` -- probes/planets use `.p` for position, matching
 * every real ij() call site found by grepping pico8/lua.lua). Returns the
 * squared distance to the nearest DIFFERENT-position entity (mk, Lua's
 * `#(p-o.p)` squared-length op, pico8_vec_len_sq) and writes the nearest
 * entity itself to *out_nearest as a PICO8_VALUE_REF (pico8_value_ref(NULL)
 * if j_ is empty or every entity shares p's own exact position, mirroring
 * Lua's un-initialized `nd` staying nil). */
pico8_fixed_t pico8_ij(pico8_vec_t p, const pico8_list_t *j_, pico8_value_t *out_nearest);

#endif
