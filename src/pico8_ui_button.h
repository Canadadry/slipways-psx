#ifndef PICO8_UI_BUTTON_H
#define PICO8_UI_BUTTON_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Phase 3, Chunk 4 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `js`/`fq`. See pico8_ui_button.c's own top comment for what
 * these classes turned out to represent (a general-purpose clickable
 * icon/text button, and its modal-backdrop sibling) and for the exact
 * real Lua source of every method declared below.
 *
 * js=ke:jr([[
 * fa=11,
 * ]])js.hd=gk
 * function js:pq()self.lq=lq(mv(self,"lq"))self.mb=self.lq.w+2
 * self.kq=qz(0,-1,self.mb,11)self.h_=self.h_ and lq(self.h_)self.jb=self.jb and jb(self.jb)end
 * function js:ph()if self.qf then
 * local d=self.qf-self.ra
 * self.ra+=(#d<=1 and d or d*0.4)
 * end
 * end
 * function js:ev()if self.jb and self.m_ then
 * self.jb:ol(127,1)end
 * end
 * function js:kj(p)local qt,dn,c=
 * p.x+self.mb-1,p.y+9,self.my
 * rect(p.x-1,p.y,qt+1,dn+1,1)rectfill(p.x,p.y,qt,dn,c and (self.m_ and 12 or 13)or 5)rectfill(p.x,p.y,qt,p.y,c and 6 or 5)self.lq:ol(p+v(1,2))local c=self.h_
 * if c then
 * rectfill(qt+3,p.y+1,qt+c.w+4,dn,c.bg or 2)c:ol(p+v(self.mb+3,2))end
 * end
 * function js:ldown()rn(self.my and 60)mv(self,"my")
 * return true
 * end
 * (pico8/lua.lua:258-279.)
 *
 * `js.hd=gk` -- a plain field assignment (like `ke.pq=lt`), NOT a
 * `function js:hd()...end` definition: js's own "hd" (hit-test) dispatch
 * method literally IS Chunk 1's standalone `gk` (pico8_entity_geom.h).
 * `hd` is one of bm's 5 dispatch names, so this makes every js instance
 * register into cv["hd"] (lt()'s own field-presence check, pico8_entity.c)
 * and reachable via `mv(e,"hd",self.qa,je.ra)` (`ui:gw()`, Chunk 5's own
 * real 2-argument mv() call site). pico8_gk's own real signature takes
 * TWO extra values (mp,rp); pico8_value_function_t's CURRENT shape (Chunk
 * 5 hasn't extended it yet) only carries one. pico8_js_hd_dispatch
 * (pico8_ui_button.c) is therefore a KNOWINGLY INCOMPLETE adapter: it
 * passes the SAME single `arg` for both mp and rp, which is only
 * correct when the real 2-argument call happens to pass the same vector
 * twice (never evidenced) -- flagged here explicitly as needing a real
 * fix once Chunk 5's mv()/pico8_value_function_t arity extension lands
 * and `ui:gw()` is ported, not silently left as if it were already
 * correct. Not exercised by anything in this chunk's own scope (no `ui`,
 * no smoke-test case dispatches through cv["hd"] this way) -- the same
 * "compiles, not yet exercised" forward-reference treatment as everywhere
 * else in this PRD.
 *
 * `js:pq/ph/ev/kj/ldown` are all real, dispatchable class fields too (not
 * just plain C functions): `kj`/`ev` are bm names (dc()'s own generic
 * per-layer draw dispatch will find them once Chunk 6/22 build a real
 * drawing loop); `ph` is dispatched via dg()'s own `qc[qc.me](qc,qc.t)`
 * (ke's own default `me="ph"`, Chunk 6); `ldown` is dispatched by name via
 * `ui:jz(rm,...)`'s own `mv(h,rm,...)` (`rm="ldown"`/`"rdown"`, Chunk 5).
 * Each has a small dispatch adapter (pico8_ui_button.c) bridging
 * pico8_value_function_t's fixed shape to that method's own real
 * signature, reading pico8_dispatch_context.h's shared context for
 * whichever of gfx/mem it needs (kj/ev only -- ph/ldown/pq are otherwise
 * self-contained). None of this is exercised by anything in this chunk's
 * own scope either, for the same forward-reference reasons as `hd` above.
 *
 * fq=ke:jr([[
 * fa=14,
 * hd=1,
 * ]])function fq:ldown()self.p_=mv(self,"my")
 * return true
 * end
 * (pico8/lua.lua:324-329.) `fq`'s own `hd=1` is plain DATA (a
 * PICO8_VALUE_FIXED spec-string field, not a function) -- ob() parses it
 * as-is, no dispatch adapter needed: mv()'s own "not a function -> return
 * the field's value as-is" branch (pico8_entity.c) already handles this
 * correctly, matching every other plain-data bm field already ported. */
/* Declares the real `js` class (parent pico8_ke_class(), no `v(...)`/
 * `o(...)` in its own spec so, unlike nf/qb, this doesn't need
 * pico8_pi_init() first) and wires pq (its own distinct extra-
 * constructor, separate from ke's inherited lt()) plus hd/ph/ev/kj/ldown
 * (dispatch adapters) onto its defaults table. See this file's own top
 * comment. */
void pico8_js_init(void);
void pico8_js_shutdown(void);

/* Mirrors pico8_scroll.h's own pico8_qb_class() convention. */
pico8_class_t *pico8_js_class(void);

/* `js(overrides)` -- thin wrapper around pico8_class_instantiate(). */
pico8_instance_t *pico8_js_new(const pico8_ob_table_t *overrides);

/* function js:pq()self.lq=lq(mv(self,"lq"))self.mb=self.lq.w+2
 * self.kq=qz(0,-1,self.mb,11)self.h_=self.h_ and lq(self.h_)self.jb=self.jb and jb(self.jb)end
 * (pico8/lua.lua:261-262.) The literal, directly-callable port. */
void pico8_js_pq(pico8_instance_t *self);

/* function js:ph()if self.qf then
 * local d=self.qf-self.ra
 * self.ra+=(#d<=1 and d or d*0.4)
 * end
 * end
 * (pico8/lua.lua:263-267.) The literal, directly-callable port: eases
 * `self.ra` toward an optional target `self.qf` (present only when set by
 * a caller -- js's own class spec never sets it). */
void pico8_js_ph(pico8_instance_t *self);

/* function js:ev()if self.jb and self.m_ then
 * self.jb:ol(127,1)end
 * end
 * (pico8/lua.lua:268-270.) The literal, directly-callable port. Needs
 * gfx/mem for jb:ol's own drawing (Chunk 3's pico8_jb_ol). */
void pico8_js_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self);

/* function js:kj(p)local qt,dn,c=
 * p.x+self.mb-1,p.y+9,self.my
 * rect(p.x-1,p.y,qt+1,dn+1,1)rectfill(p.x,p.y,qt,dn,c and (self.m_ and 12 or 13)or 5)rectfill(p.x,p.y,qt,p.y,c and 6 or 5)self.lq:ol(p+v(1,2))local c=self.h_
 * if c then
 * rectfill(qt+3,p.y+1,qt+c.w+4,dn,c.bg or 2)c:ol(p+v(self.mb+3,2))end
 * end
 * (pico8/lua.lua:271-276.) The literal, directly-callable port: draws the
 * button's own frame/fill, its `lq`-laid-out label, and (if present) a
 * tooltip box `h_` beside it. Needs gfx/mem. */
void pico8_js_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p);

/* function js:ldown()rn(self.my and 60)mv(self,"my")
 * return true
 * end
 * (pico8/lua.lua:277-279.) The literal, directly-callable port: plays the
 * (inert) click sfx via Chunk 1's rn(), invokes the optional `my` click
 * callback via mv() with the established "no real Lua argument"
 * pico8_value_ref(NULL) convention (see docs/portage-progress.md's Vague
 * 4 -- this port's own zero-extra-argument mv() call convention), and
 * always returns true (click consumed). */
pico8_value_t pico8_js_ldown(pico8_instance_t *self);

/* Declares the real `fq` class (parent pico8_ke_class()) and wires
 * `ldown` onto its defaults table; `hd=1` needs no wiring (plain data,
 * parsed straight from the spec string). See this file's own top comment. */
void pico8_fq_init(void);
void pico8_fq_shutdown(void);
pico8_class_t *pico8_fq_class(void);

/* `fq(overrides)` -- thin wrapper around pico8_class_instantiate(). */
pico8_instance_t *pico8_fq_new(const pico8_ob_table_t *overrides);

/* function fq:ldown()self.p_=mv(self,"my")
 * return true
 * end
 * (pico8/lua.lua:327-329.) The literal, directly-callable port: `self.p_`
 * is only written when `mv(self,"my")` actually finds a "my" field
 * (matching Lua's own `self.p_=nil` no-op when it doesn't -- there is no
 * "unset a field" primitive on pico8_ob_table_t, so this port simply
 * skips the write rather than writing a synthetic "absent" value). `.p_`
 * is dg()'s own future self-destruct flag (`if qc.p_ then he(qc) end`,
 * Chunk 6) -- fq is a one-shot, self-dismissing modal backdrop. */
pico8_value_t pico8_fq_ldown(pico8_instance_t *self);

#endif
