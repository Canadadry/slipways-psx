#ifndef PICO8_KE_H
#define PICO8_KE_H

#include "pico8_class.h"
#include "pico8_value.h"

/* Phase 3, Chunk 3 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `ke`, confirmed by reading pico8/lua.lua's own later classes
 * (nf/js/fq/qb/bg/jo/mh/jc/g_/mz/pj/ie/iv all declared `X=ke:jr([[...]])`,
 * Chunks 4/7/8/10/13/14/16/18/19 -- not assumed from the name) to be the
 * actual base of the whole entity hierarchy: every later game entity class
 * extends ke, directly or (via jo/qh/hj, Chunk 13/15) transitively. `ke`
 * itself is declared as a real *root* class of the kv:jr system
 * (`kv:jr(...)`, i.e. pico8_class_declare(NULL, ...) here), not an
 * instance of anything.
 *
 * ke=kv:jr([[
 * me="ph",t=0,
 * ]])ke.pq=lt
 * function ke:ks(me)self.me,self.t=me,0
 * end
 * (pico8/lua.lua:182-186.)
 *
 * `ke.pq=lt`: a plain field assignment, not a `function ke:pq()...end`
 * definition -- ke's own "extra constructor" (pico8_class_pq_fn, see
 * pico8_class.h) IS lt() itself (pico8_lt, pico8_entity.h): every
 * ke-descended instance self-registers into hv/cv/cg (pico8_hv/pico8_cv/
 * pico8_cg) the moment it is instantiated, via pico8_class_instantiate's
 * own pq-walk -- not via mv()/dc()'s dispatch mechanism, which is a
 * separate, unrelated function-pointer shape (pico8_value_function_t,
 * pico8_value.h) that ke.pq never goes through. Ported as a small adapter
 * (see pico8_ke.c) bridging pico8_class_pq_fn's real signature
 * (`void(*)(pico8_instance_t*)`) to pico8_lt's own real signature
 * (`pico8_instance_t*(*)(pico8_instance_t*)`) by discarding lt()'s
 * returned pointer (the same instance it was given, per pico8_lt's own
 * comment) -- pico8_class_set_pq has no room for a return value, and
 * nothing here needs one.
 *
 * This is the first chunk to instantiate a REAL class whose pq genuinely
 * self-registers a fresh instance into hv/cv/cg -- Phase 2's own smoke
 * test (tests/test_pico8_entity_smoke.c) used a FAKE pq that only sets a
 * marker field, and called pico8_lt() itself, separately, afterward. See
 * that test file's own new ke-specific case (added by this chunk) for the
 * targeted coverage of this genuinely new structural exercise, per this
 * PRD's own per-lot "entity/dispatch mechanism coverage" check. */
void pico8_ke_init(void);
void pico8_ke_shutdown(void);

/* The real `ke` root class, built once by pico8_ke_init(). Needed by
 * Chunk 4's own `X=ke:jr([[...]])` subclasses (nf/js/fq/qb, none of which
 * this chunk builds), so this accessor -- mirroring pico8_entity.h's own
 * pico8_mo()-style convention for a module-owned singleton other modules
 * must read -- is this chunk's own hand-off point for them. Returns NULL
 * before pico8_ke_init() is called (or after pico8_ke_shutdown()). Not
 * retrievable via pico8_class_lookup(): ke's own spec sets no `fz` field,
 * so (like lq/jb/trade, this same chunk's other three classes) it
 * registers into the class-name registry under the shared "" slot,
 * exactly as the real kv[rj.fz or ""]=rj does -- see pico8_class.h's own
 * comment on why that collision is left as-is rather than "fixed". This
 * accessor, not name lookup, is the reliable way to reach this specific
 * class. */
pico8_class_t *pico8_ke_class(void);

/* function ke:ks(me)self.me,self.t=me,0
 * end
 * (pico8/lua.lua:185-186.) Resets self's own "me"/"t" fields (an icon/
 * animation-state name and its timer). Called directly (`self:ks(...)`,
 * e.g. pico8/lua.lua:411/458/571 -- all real Chunk 8/13 call sites, not
 * yet ported) using plain Lua method-call syntax on a concrete self, never
 * through mv()/dc()'s generic dispatch (`ks` is not one of bm's five
 * dispatch names, see pico8_entity.c's own bm literal) -- so this is
 * ported as a direct C function, not wired into any dispatch table. A
 * NULL self is a no-op (nothing to write onto), matching
 * pico8_entity_geom.c's own established defensive-write convention. */
void pico8_ke_ks(pico8_instance_t *self, pico8_value_t me);

#endif
