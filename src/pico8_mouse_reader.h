#ifndef PICO8_MOUSE_READER_H
#define PICO8_MOUSE_READER_H

#include "pico8_class.h"
#include "pico8_input.h"
#include "pico8_mem.h"
#include "pico8_ob.h"

/* Phase 3, Chunk 4 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `nf`, a real `ke`-descended class (confirmed: `nf=ke:jr(...)`,
 * pico8/lua.lua:200 -- not assumed from the PRD's own prose, which
 * describes it as "the mouse/D-pad reader singleton class", Chunk 6's
 * future `je=nf()`). Reading its own two real methods below, `nf` turns
 * out to be a MOUSE reader only -- its own `ph()` reads stat(32)/stat(33)
 * (mouse x/y) and left/right mouse-button edge state via Chunk 1's `bu()`
 * (pico8_mouse.h); it never reads btn()/the D-pad at all (that's `qb`'s
 * own job, see pico8_scroll.h). The PRD's "mouse/D-pad reader" phrase
 * describes `je`'s eventual COMBINED role once `ui`/other code reads both
 * `je` (mouse) and btn() (D-pad) side by side -- not this class's own
 * methods in isolation.
 *
 * nf=ke:jr([[
 * l=o(),r=o(),ra=v(0,0),
 * ]])function nf:pq()poke(0x5f2d,1)end
 * function nf:ph()self.ra=v(stat(32),stat(33))bu(self.l,1)bu(self.r,6)end
 * (pico8/lua.lua:200-203.)
 *
 * Class spec: `l`/`r` are empty nested tables (`o()`, ob()'s own
 * unregistered-identifier fallback -- see pico8_ob.c's "identifier(...)"
 * branch -- populated in place by bu()'s own on/jn/hm writes, never
 * reassigned), `ra` a zero vector (Chunk 1's `'v'` char-keyed pi
 * constructor -- see this file's own pico8_nf_init() comment for why
 * pico8_pi_init() must run before this spec string is parsed).
 *
 * `nf:pq()` is nf's OWN distinct extra-constructor (`function nf:pq()...
 * end` = a plain `nf.pq=function(self)...end` field assignment, exactly
 * like `ke.pq=lt` -- see pico8_class.c's own two-worked-traces comment on
 * why a leaf's own distinct pq fires SEPARATELY from an inherited
 * ancestor's, both exactly once): `poke(0x5f2d,1)` enables the PICO-8
 * devkit mouse-reporting flag. Needs `mem` but pico8_class_pq_fn's fixed
 * shape (`void(*)(pico8_instance_t*)`) has no room for it -- see this
 * file's own pico8_nf_pq_dispatch (pico8_mouse_reader.c) for the small
 * adapter reading pico8_dispatch_mem() (pico8_dispatch_context.h).
 *
 * `nf:ph()` is dispatched generically too, NOT via mv()/dc() (`ph` isn't
 * one of bm's five names) but via dg()'s own `qc[qc.me](qc,qc.t)` (Chunk
 * 6, not yet ported) -- ke's own default `me="ph"` makes this nf instance's
 * (and every other unmodified ke-descendant's) current per-tick method.
 * Needs `input` (stat()/btn()); same private-dispatch-context treatment as
 * pq above (pico8_nf_ph_dispatch, reading pico8_dispatch_input()).
 * Forward reference, ported not deferred: nothing in this chunk's own
 * scope constructs a real `nf` instance or calls dg(), so this stays
 * "compiles, not yet exercised" until Chunk 6. */
void pico8_nf_init(void);
void pico8_nf_shutdown(void);

/* The real `nf` class, built once by pico8_nf_init(). Mirrors
 * pico8_ke.h's own pico8_ke_class() convention -- not retrievable via
 * pico8_class_lookup() (registers under the shared "" name-registry slot,
 * like every other unnamed `:jr()` class so far -- see pico8_ke.h's own
 * comment on why). Returns NULL before pico8_nf_init() (or after
 * pico8_nf_shutdown()). */
pico8_class_t *pico8_nf_class(void);

/* `nf(overrides)` -- the real Lua constructor call (e.g. Chunk 6's future
 * `je=nf()`). Thin wrapper around pico8_class_instantiate(), mirroring
 * pico8_layout.h's own pico8_lq_new/pico8_jb_new convention. */
pico8_instance_t *pico8_nf_new(const pico8_ob_table_t *overrides);

/* function nf:pq()poke(0x5f2d,1)end
 * (pico8/lua.lua:202.) The literal, directly-callable port -- `self` is
 * unused (the real Lua body never references it either), kept only so
 * this signature matches pico8_class_pq_fn's own instance-taking shape
 * for anyone calling it directly (e.g. a future test). A NULL `mem` is a
 * safe no-op (nothing to poke onto), matching this codebase's established
 * defensive-context convention (pico8_dispatch_context.h). */
void pico8_nf_pq(pico8_mem_t *mem, pico8_instance_t *self);

/* function nf:ph()self.ra=v(stat(32),stat(33))bu(self.l,1)bu(self.r,6)end
 * (pico8/lua.lua:203.) The literal, directly-callable port: rebinds
 * `self.ra` to a fresh heap vector holding the current mouse position
 * (stat(32)/stat(33)), then runs Chunk 1's bu() edge-detection over
 * `self.l`/`self.r` (oa=1 for the left button's own bitmask, oa=6 for the
 * right button's -- see pico8_mouse.h's own comment on bu()'s `oa`
 * parameter) -- `self.l`/`self.r` are the SAME shared nested tables `nf`'s
 * own class spec builds via `o()` (mutated in place by bu(), exactly
 * matching real Lua's own single-nf-singleton behavior, see this file's
 * own top comment). A NULL `self` or `input` is a safe no-op. */
void pico8_nf_ph(const pico8_input_t *input, pico8_instance_t *self);

#endif
