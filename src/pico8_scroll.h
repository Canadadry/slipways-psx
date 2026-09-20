#ifndef PICO8_SCROLL_H
#define PICO8_SCROLL_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_ob.h"

/* Phase 3, Chunk 4 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `qb`, the camera/scroll singleton class (a real `ke`-
 * descended class: `qb=ke:jr(...)`, pico8/lua.lua:1128 -- confirmed by
 * reading the source, matching the PRD's own later text identifying it as
 * Chunk 6's future `mo=qb()`).
 *
 * fr=nr:jr([[...]])qb=ke:jr([[
 * v=v(0,0),
 * ma=0,
 * p=v(192,192),
 * ra=v(192,192),
 * pc=o(v(-1,0),v(1,0),v(0,-1),v(0,1)),
 * ]])function qb:ph()local ip=v(0,0)for b=0,3 do
 * if btn(b)or btn(b,1)then
 * ip+=self.pc[b+1]*3
 * end
 * end
 * self.p+=self.v
 * self.v+=(ip-self.v)*0.2
 * self.p.x=mid(-64,320,self.p.x)self.p.y=mid(-64,320,self.p.y)self.ra=ma(self.p,self)end
 * function qb:md(fy)local cp=self.ra*(fy or 1)camera(cp.x,cp.y)end
 * (pico8/lua.lua:1128-1142, the leading `fr=nr:jr(...)` is a DIFFERENT,
 * unrelated class immediately preceding qb's own declaration in the file
 * -- not part of qb's own spec.)
 *
 * Class spec: `v`/`p`/`ra` are vectors (Chunk 1/2's 'v' char-keyed pi
 * constructor -- same ordering hazard as pico8_mouse_reader.h's own `nf`,
 * see this file's own pico8_qb_init() comment), `ma` a plain fixed 0
 * (inherited/overridden the same way any other instance field is), `pc` a
 * positional table of 4 unit direction vectors built via the SAME 'v'
 * constructor nested inside `o(...)`'s own unregistered-identifier
 * fallback (pico8_ob.c's "identifier(...)" branch: `o` itself is never
 * registered, so `oo(nil,parsed_table)`'s own `or a` returns the parsed
 * positional table of 4 vector REFs as-is).
 *
 * qb has NO own `pq` -- unlike nf/js, it purely inherits ke's own
 * `pq=lt` (auto-registration), needing no pico8_class_set_pq() call here. */
void pico8_qb_init(void);
void pico8_qb_shutdown(void);

/* The real `qb` class, built once by pico8_qb_init(). Mirrors
 * pico8_mouse_reader.h's own pico8_nf_class() convention (not retrievable
 * via pico8_class_lookup(), same shared "" registry-slot reasoning).
 * Returns NULL before pico8_qb_init() (or after pico8_qb_shutdown()). */
pico8_class_t *pico8_qb_class(void);

/* `qb(overrides)` -- the real Lua constructor call (Chunk 6's future
 * `mo=qb()`). Thin wrapper around pico8_class_instantiate(). */
pico8_instance_t *pico8_qb_new(const pico8_ob_table_t *overrides);

/* function qb:ph()local ip=v(0,0)for b=0,3 do
 * if btn(b)or btn(b,1)then
 * ip+=self.pc[b+1]*3
 * end
 * end
 * self.p+=self.v
 * self.v+=(ip-self.v)*0.2
 * self.p.x=mid(-64,320,self.p.x)self.p.y=mid(-64,320,self.p.y)self.ra=ma(self.p,self)end
 * (pico8/lua.lua:1134-1141.) The literal, directly-callable port.
 *
 * `btn(b,1)` -- ALREADY-DOCUMENTED project decision (PRD's Implementation
 * Decisions and User Story 9): a second-controller check with no second-
 * player representation anywhere in `pico8_input_t`/events.json, and none
 * being added. Applied at THIS exact call site (pico8_scroll.c's own
 * pico8_qb_ph body): `btn(b,1)` is hardcoded to 0/false rather than
 * extending pico8_btn's own signature for a player this whole port never
 * supports -- see that function's own one-line comment at the exact spot.
 *
 * `self.p+=self.v`/`self.v+=...` are field REBINDS (`self.p=self.p+
 * self.v`, a fresh vector, not a component mutation) -- a NEW heap vector
 * is allocated and written onto `self->fields` for each, exactly like
 * every other "field write always lands on the instance" case already
 * established (pico8_class.c's own top comment); `self.p.x=mid(...)`/
 * `self.p.y=mid(...)` right after ARE in-place component mutations, safe
 * here specifically because `self.p` was JUST rebound to a fresh,
 * exclusively-owned heap vector this same call, so no other reader could
 * be holding a reference to it yet. `self.ra=ma(self.p,self)` (Chunk 1's
 * pico8_ma, pico8_entity_geom.h) reads the now-clamped `self.p` and writes
 * a fresh heap vector to `self.ra`.
 *
 * Needs `input` (btn()); pico8_qb_ph_dispatch (pico8_scroll.c) is the
 * small adapter reading pico8_dispatch_input() for dg()'s own generic
 * `qc[qc.me](qc,qc.t)` per-tick dispatch (same treatment as
 * pico8_mouse_reader.h's own nf:ph -- see that file's comment). A NULL
 * `self` or `input` is a safe no-op. */
void pico8_qb_ph(const pico8_input_t *input, pico8_instance_t *self);

/* function qb:md(fy)local cp=self.ra*(fy or 1)camera(cp.x,cp.y)end
 * (pico8/lua.lua:1142.) The literal, directly-callable port: `fy`'s own
 * `fy or 1` Lua fallback is resolved by the caller before this function is
 * reached (see pico8_qb_md_dispatch, pico8_scroll.c) -- this port's own
 * single evidenced real call site (`mo:md(ci[o])`, dc()'s "kj" branch,
 * pico8_entity.c) always provides a genuine fy, so (mirroring
 * pico8_layout.h's own pico8_jb_ol single-entry-point precedent) there is
 * no separate `_default` entry point here either.
 *
 * THIS is the function that finally gives real behavior to Phase 2's
 * long-dead-but-compiled `dc("kj")` branch (`mo:md(ci[o])`,
 * pico8_entity.c's own pico8_dc()) -- once Chunk 6 makes `mo` a real `qb`
 * instance (via the completed `jl()`) with this class's own "md" field
 * (registered below by pico8_qb_init()) resolvable through the ordinary
 * class-chain fallback, `pico8_mv(mo,"md",ci_val,&discard)` will find and
 * call this for real. It does NOT need to be exercised yet (nothing calls
 * dc("kj") for real until Chunk 6/22 build a real `mo` singleton and a
 * real drawing loop) -- flagged here as the connection this PRD's own
 * task explicitly asks to note, not something this chunk needs to make
 * work end to end.
 *
 * Needs `gfx` (camera()); pico8_qb_md_dispatch is the adapter reading
 * pico8_dispatch_gfx() -- pico8_value_function_t's fixed (self,arg) shape
 * has no room for gfx itself, same problem (and same shared-context
 * solution, pico8_dispatch_context.h) as pico8_ui_button.h's own js:kj/
 * js:ev. A NULL `self` or `gfx` is a safe no-op. */
void pico8_qb_md(pico8_gfx_t *gfx, pico8_instance_t *self, pico8_fixed_t fy);

#endif
