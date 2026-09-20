#ifndef PICO8_PARTICLE_H
#define PICO8_PARTICLE_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Phase 3, Chunk 9 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `f_`, `ez`, `ei` -- pico8/lua.lua's shared particle-system
 * helpers (lines 685-707), used by the particle-emitting entity classes
 * this port hasn't built yet (`g_`, Chunk 10; `hj`/`ga`, Chunk 15) but with
 * no unported dependency of their own, so ported now per the PRD's own
 * "forward references are ported, not deferred" convention.
 *
 * function f_(self)self.qy={}
 * end
 * function ez(self,n)while rnd()<n do
 * add(self.qy,self:mm())
 * n-=1
 * end
 * end
 * function ei(self)
 * if (self.fa<10 and not gd(self))return
 * local cx,cy,qo,qg,ro,q_,cs=
 * self.ra.x,self.ra.y,self.qo,self.qg,self.ro,self.q_,self.k_
 * for _,p in pairs(self.qy)do
 * local pv=p.v
 * p.p+=pv
 * p.v.x=qo*pv.x+ro*pv.y
 * p.v.y=qg*pv.x+q_*pv.y
 * if (p.a)p.v+=p.a
 * p.l-=0.14
 * if p.l<=1 then
 * del(self.qy,p)else
 * pset(cx+p.p.x,cy+p.p.y,cs[flr(p.l)])end
 * end
 * ez(self,self.fc)end
 *
 * Confirmed real usage (grepped across the whole file, not guessed from
 * the PRD's own paraphrase): `g_.pq=f_`/`g_.kj=ei` (pico8/lua.lua:725-726,
 * Chunk 10) and `hj.pq=f_` (line 1012, Chunk 15) assign these two
 * standalone functions DIRECTLY as class fields -- no wrapper, no
 * inheritance, the exact same Lua function object reused verbatim by two
 * unrelated classes. `ez` is never itself assigned to any class field;
 * it's called as a plain function, both internally by `ei` (the line
 * above) and directly by `ga:pq()` (line 1079, Chunk 15's `f_(self)
 * ez(self,140)mo.ma=3`). `ei` is ALSO called directly as a plain function
 * (not through dc()/mv()) by `hj:kj` (line 1015) and `fv:kj` (line 1170,
 * Chunk 17) -- both wrap it in their own dispatched method body rather
 * than reusing it as their own `kj` field wholesale.
 *
 * What each one shapes out to be, and why this chunk gives it the C
 * signature it does:
 *
 * `f_(self)`: sets self.qy to a fresh empty particle list. Its one Lua
 * parameter (self) and its one Lua statement (an instance-field write, no
 * other side effect) already match pico8_class_pq_fn's real signature
 * (pico8_class.h) -- `void(*)(pico8_instance_t*)` -- EXACTLY, character
 * for character. So `pico8_f_` below IS that pq function, with no
 * bridging adapter needed at all: `g_.pq=f_`/`hj.pq=f_` (Chunks 10/15)
 * port as a direct `pico8_class_set_pq(g_class, pico8_f_)` /
 * `pico8_class_set_pq(hj_class, pico8_f_)` call, exactly the same
 * "signature already matches, no adapter" precedent already established
 * by `mh:pq` (pico8_msgbox.c) and `js`/`fq`'s own plain pq's own
 * dispatch-free cases.
 *
 * `ei(self)`: this chunk's OTHER real dispatch-field case, and a
 * genuinely different shape problem from `f_`'s. `ei` is a bm dispatch
 * name (`kj`, one of `bm`'s 5 names, pico8_entity.c) when reached via
 * `g_.kj=ei` and dc()/mv()'s call convention (`entity["kj"](entity,
 * entity.ra)`), which needs the `pico8_value_function_t` shape (`(void
 * *self, int argc, pico8_value_t arg0, pico8_value_t arg1) ->
 * pico8_value_t`, pico8_value.h) -- but `ei`'s own real body also needs
 * `gfx`/`mem` (for `pset`), which that fixed dispatch shape has no slot
 * for, exactly the same problem `bg:kj` (pico8_bg.c's
 * `pico8_bg_kj`/`pico8_bg_kj_dispatch` split) already solved for a
 * method-syntax function. `ei` additionally needs to stay callable as a
 * PLAIN function (`ei(self)`, no dc()/mv() involved) for `hj:kj`/`fv:kj`'s
 * own direct calls, which -- being ordinary dispatched methods themselves
 * -- already have `gfx`/`mem` in scope to pass down explicitly, the same
 * way `bg:kj` does. So this chunk ports BOTH halves of that same split up
 * front, ready for either calling convention:
 *   - `pico8_ei(gfx, mem, self)`: the real business logic, explicit
 *     `gfx`/`mem` parameters (bg_kj's own convention) -- this is what
 *     `hj:kj`/`fv:kj` (Chunks 15/17) call directly, and what `pico8_ez`
 *     itself calls at the very end of its own body.
 *   - `pico8_ei_dispatch(self, argc, arg0, arg1)`: the
 *     pico8_value_function_t-shaped bridge, reading `gfx`/`mem` from the
 *     shared per-frame dispatch context (pico8_dispatch_context.h, the
 *     same mechanism `pico8_bg_kj_dispatch` already reads from) -- this is
 *     what Chunk 10 wires verbatim as `g_.kj=ei`:
 *     `pico8_ob_table_set_name(g_class->defaults, "kj",
 *     pico8_value_function(pico8_ei_dispatch))`. Built here, not deferred
 *     to Chunk 10, so the field-assignment site itself needs no shape
 *     decision of its own -- the same "port now, ready for direct field
 *     assignment" treatment this chunk's own brief asks for.
 *   dc()'s/mv()'s own `entity.ra` second argument is silently dropped by
 *   real Lua when calling through `g_.kj=ei` (`ei` only declares one
 *   parameter, `self`) -- `pico8_ei_dispatch` reproduces this by simply
 *   never reading `arg0`/`arg1`, not by widening `pico8_ei`'s own
 *   parameter list to accept a position it never uses.
 *
 * `ez(self,n)`: never itself a dispatch-field value (see above), so it
 * keeps a plain `(pico8_instance_t *self, pico8_fixed_t n)` signature --
 * no adapter needed, since nothing ever looks it up by name through
 * dc()/mv(). Its own `self:mm()` call IS a genuinely polymorphic Lua
 * method call, though: `ez` itself is shared verbatim across every
 * particle-emitting class (`g_`, `hj` via `pq=f_` reuse, `ga` via its own
 * direct `ez(self,140)` call), each of which is expected to define its
 * OWN, DIFFERENT `mm` (`g_:mm`, line 727; `hj.mm=g_.mm`, line 1013;
 * `ga:mm`, line 1081) -- `ez` itself never knows, and must never assume,
 * which concrete `mm` a given `self` carries. This is exactly the
 * dynamic-field-lookup shape `pico8_mv()` (pico8_entity.h) already exists
 * for, not a class-name forward reference like `bg:ldown`'s `mz`/`g_`
 * (which needed the NEW named-class registry, pico8_class.h) -- `mm` is
 * looked up as an ordinary field on `self`'s own class chain, exactly like
 * `js.hd=gk`'s own `mv(e,"hd",...)` dispatch (pico8_ui.c's `ui:gw`).
 * `self:mm()` takes no Lua arguments, so this calls `pico8_mv(self, "mm",
 * 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &result)` -- argc=0,
 * both argument slots filled with this port's established "no real Lua
 * value here" convention (pico8_value.h's own comment on
 * pico8_value_function_t), matching the 0-argument `mv()` sites already
 * ported by Chunk 4 (`js:pq`'s `mv(self,"lq")`).
 *
 * **`mm` forward-reference status, confirmed inert today**: no class in
 * this port's scope so far (Chunks 1-8) defines a field named `mm` on any
 * instance -- `g_`/`ga`/`hj` (the only three real definers, per the grep
 * above) are Chunks 10/15, not yet ported. So `pico8_mv(self, "mm", ...)`
 * returns 0 (field not found anywhere on the class chain) for every
 * instance any code in this repository can construct today; per
 * `pico8_mv`'s own documented "absent" contract (pico8_entity.h), this
 * chunk's `pico8_ez` simply skips the `pico8_list_add` in that case rather
 * than crashing or fabricating a particle -- the same defensive-divergence
 * treatment already established for every other not-yet-resolvable
 * forward reference in this phase (`bg:ldown`'s `mz`/`g_` returning "held
 * nothing", `gd()`'s pre-Chunk-6 zero-vector `mo.ra`, etc.), not a new
 * pattern. `pico8_ez` is therefore "compiles and runs safely, but adds no
 * particles" until Chunk 10/15 defines a real `mm` -- the same "compiles,
 * not yet meaningfully exercised" status this whole PRD already gives
 * every other forward reference, confirmed here rather than assumed.
 *
 * **Particle representation, a new convention this chunk establishes**:
 * each element of `self.qy` (and each value `self:mm()` is expected to
 * return, once Chunks 10/15 define it) is a PICO8_VALUE_REF pointing to a
 * heap-allocated `pico8_ob_table_t` with named fields "p" (position,
 * REF-to-heap-pico8_vec_t), "v" (velocity, same), "a" (acceleration, same,
 * OPTIONAL -- absent is a real, checked-for case, `if (p.a)` in the source
 * above) and "l" (life, PICO8_VALUE_FIXED) -- mirroring `ga:mm`'s own real
 * literal table (`{p=p,v=p*self.ms,a=p*self.iu,l=rnd(self.py)+self.py}`,
 * line 1084, the only concrete real shape any future `mm` in scope
 * produces). This reuses the exact "plain nested Lua table, not a
 * pico8_instance_t" representation pico8_mouse.c's `bu(qn,oa)` already
 * established for `nf`'s own `l=o()`/`r=o()` fields (Chunk 1) -- not a new
 * bespoke struct -- since a table built directly with `{...}` syntax
 * (never through `kv:jr`/a class) has no class/prototype-chain fallback to
 * model, exactly like `qn` there. `self.qy` itself is a PICO8_VALUE_REF to
 * a heap `pico8_list_t*` (the "vector-valued/growable field" convention
 * pico8_entity_geom.h's own top comment already established for `.ra`/`.p`
 * -- extended here to any growable list-valued field, not just a single
 * vector). Chunks 10/15's own `mm` implementations must build particle
 * tables this exact same way for `pico8_ei`'s field reads below to see
 * them correctly -- this is the one piece of NEW, this-port-chosen
 * structure `pico8_ez`/`pico8_ei` require of whatever `mm` eventually
 * returns, since the PRD's own text only promises `mm`'s EXISTENCE, not
 * its C-side shape. */

/* pico8_class_pq_fn-shaped: `g_.pq=f_`/`hj.pq=f_` port as a direct
 * `pico8_class_set_pq(cls, pico8_f_)`, no adapter (see this file's own top
 * comment). Allocates a fresh, empty heap `pico8_list_t` and stores it as
 * `self`'s own "qy" field -- mirrors `pico8_ui.c`'s own `heap_le`
 * construction (`malloc`+`pico8_list_init`), the established pattern for a
 * freshly-built empty growable-list-valued field. A NULL `self` (or one
 * with no `fields` table -- never true for a real `pico8_class_instantiate`
 * result, but defensive the same way every other pq/field-writing function
 * in this port already is) is a safe no-op. */
void pico8_f_(pico8_instance_t *self);

/* Plain `(self, n)` signature -- never itself a dispatch-field value (see
 * this file's own top comment). Adds `pico8_fixed_flr`-rounded-down-by-
 * repeated-subtraction-of-1 many particles (PICO-8's own probabilistic
 * `while rnd()<n do ... n-=1 end` idiom, reproduced literally, not
 * "simplified" into `flr(n)` -- a fractional `n` genuinely has a
 * probability of spawning one MORE particle than `flr(n)`, exactly as
 * written), each one `self:mm()`'s own return value, added to `self.qy`
 * unchanged. A NULL `self`, or one with no "qy" field
 * (PICO8_VALUE_REF-kind, non-NULL) yet, makes this a safe no-op loop that
 * still runs to completion (matching the real Lua's own `while` condition,
 * which depends only on `n`, never on `self.qy`'s presence) but adds
 * nothing -- see this file's own top comment on why `self:mm()` not being
 * found anywhere on `self`'s class chain is expected and inert today. */
void pico8_ez(pico8_instance_t *self, pico8_fixed_t n);

/* The real particle-simulation-and-draw business logic, explicit
 * `gfx`/`mem` parameters (bg_kj's own convention, pico8_bg.h) -- this is
 * what `hj:kj`/`fv:kj` (Chunks 15/17) call directly as a plain function,
 * and what `pico8_ez`'s own trailing `ez(self,self.fc)` call sits
 * alongside inside `pico8_ei`'s own body (see this file's own top comment
 * for the full source and the field-mutation-ordering/pairs()+del()
 * divergence notes in pico8_particle.c). A NULL `gfx`/`mem`/`self` is a
 * safe no-op (matches this port's established defensive-context
 * convention, e.g. pico8_bg_pq's own `mem == NULL` guard). */
void pico8_ei(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self);

/* pico8_value_function_t-shaped bridge to pico8_ei above, reading
 * `gfx`/`mem` from the shared per-frame dispatch context
 * (pico8_dispatch_context.h) -- this is what Chunk 10 wires verbatim as
 * `g_.kj=ei` (`pico8_ob_table_set_name(g_class->defaults, "kj",
 * pico8_value_function(pico8_ei_dispatch))`); see this file's own top
 * comment for why this split exists. `argc`/`arg0`/`arg1` (dc()'s own
 * `entity.ra` second argument) are never read, matching real Lua silently
 * dropping an excess argument to a function declared with fewer
 * parameters. */
pico8_value_t pico8_ei_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

#endif
