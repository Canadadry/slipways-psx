#ifndef PICO8_LAYOUT_H
#define PICO8_LAYOUT_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Phase 3, Chunk 3 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `lq` and `jb`, the cart's own text/icon line-layout engine --
 * `lq` lays out ONE horizontal row of items (strings and/or sprites),
 * `jb` stacks several `lq` rows into a vertical, framed box (used for
 * tooltips/help text -- e.g. `js:pq`'s `self.h_`/`self.jb`, Chunk 4/8, not
 * yet ported). Both are ROOT classes of the kv:jr system declared with NO
 * spec string (`kv:jr()`, i.e. pico8_class_declare(NULL, NULL) here) --
 * NOT ke-descended, so neither ever self-registers into hv/cv/cg (see
 * pico8_ke.h for the class that does that).
 *
 * lq=kv:jr()function lq:pq()local w,s,fs,f,fw=0,0
 * self.fs=nv(self,function(f)f,fw,fs=self:mn(f)local x=w
 * w+=fw+fs
 * return {fn=f,d=v(x,0)}
 * end)self.w=w-fs
 * end
 * function lq:ol(p,a)
 * if (a)p-=v(a*self.w,0)
 * for f in all(self.fs)do
 * f.fn(p+f.d)end
 * end
 * function lq:mn(f)local w,qw,qv,fn=6,0
 * if type(f)=="table" then
 * if (f.qe)f={f.qe,5}
 * f,w,qw,qv=f[1],f[2],f[3]or qw,f[4]
 * end
 * if type(f)=="string" then
 * local c=self.c or 6
 * return function(p)io(f,p.x+2,p.y+1,c)end,#f*4+3,1
 * elseif f then
 * fn=function(p)dv(qw)spr(f,p.x,p.y)dv()end
 * end
 * return fn or function()end,w,qv or 2
 * end
 * pi.ld=lq.ol
 * jb=kv:jr()function jb:pq()self.ls=nv(self,function(ln)
 * return lq(type(ln)=="string" and {ln}or ln)end)end
 * function jb:ol(om,a)local h=#self*8+2
 * om-=h*a
 * local dn=om+h
 * fillp(0b0000111100001111)rk(om,dn,0x10)fillp()for l in all(self.ls)do
 * l:ol(v(64,om+2),0.5)
 * om+=8
 * end
 * end
 * (pico8/lua.lua:289-323.)
 *
 * `lq(...)`/`jb(...)` are called throughout the rest of the cart as plain
 * constructor calls (e.g. `self.lq=lq(mv(self,"lq"))`, `self.jb=jb(self.jb)`,
 * pico8/lua.lua:261-262, Chunk 4 -- not yet ported); pico8_lq_new/
 * pico8_jb_new below are this port's direct equivalent (a thin wrapper
 * around pico8_class_instantiate, mirroring the real Lua "calling a class
 * like a function" idiom -- see pico8_layout.c's own top comment for why
 * this is the first chunk to establish this small convention).
 *
 * `lq:mn`/the internal `fs`/`ls` list representations are this module's
 * own private implementation detail (see pico8_layout.c) -- not exposed
 * here, since nothing outside this file calls `mn` directly (the real
 * Lua's own `self:mn(f)` is an ordinary, non-virtual method call, not
 * dispatched through mv()/dc()'s generic mechanism -- see this PRD's own
 * Chunk 3 instructions on checking whether any of ke/lq/jb/trade's methods
 * need the 2-arg mv() extension: none of lq/jb/trade's methods are ever
 * called through mv()/dc() at all, so that extension stays out of this
 * chunk's scope, as planned). */
void pico8_layout_init(void);
void pico8_layout_shutdown(void);

/* The real `lq`/`jb` root classes, built once by pico8_layout_init().
 * Mirrors pico8_ke.h's own pico8_ke_class() convention -- neither is
 * retrievable via pico8_class_lookup() (both register under the shared ""
 * name-registry slot, like ke and trade -- see pico8_ke.h's own comment on
 * why). Return NULL before pico8_layout_init() (or after
 * pico8_layout_shutdown()). */
pico8_class_t *pico8_lq_class(void);
pico8_class_t *pico8_jb_class(void);

/* `lq(overrides)`/`jb(overrides)` -- see this file's own top comment. */
pico8_instance_t *pico8_lq_new(const pico8_ob_table_t *overrides);
pico8_instance_t *pico8_jb_new(const pico8_ob_table_t *overrides);

/* function lq:ol(p,a)
 * if (a)p-=v(a*self.w,0)
 * for f in all(self.fs)do
 * f.fn(p+f.d)end
 * end
 * (pico8/lua.lua:295-299.) `a` omitted (pico8_lq_ol_default) is exactly
 * equivalent, numerically, to `a` provided as 0 (`p-=v(0*self.w,0)` is a
 * no-op vector subtraction either way) -- real Lua's own `if (a)` branch
 * is only ever entered by a caller who already has a genuine alignment
 * factor to apply, so this port doesn't need a separate "was `a`
 * provided" flag: pico8_lq_ol_default simply calls pico8_lq_ol with
 * a=0, matching every other two-entry-point optional-argument function
 * already established in this area of the port (pico8_dv/pico8_rk/
 * pico8_io, pico8_hud.h). A NULL self is a no-op (matches
 * pico8_entity_geom.c's own established defensive convention). */
void pico8_lq_ol(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p,
                  pico8_fixed_t a);
void pico8_lq_ol_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p);

/* function jb:ol(om,a)local h=#self*8+2
 * om-=h*a
 * local dn=om+h
 * fillp(0b0000111100001111)rk(om,dn,0x10)fillp()for l in all(self.ls)do
 * l:ol(v(64,om+2),0.5)
 * om+=8
 * end
 * end
 * (pico8/lua.lua:316-322.) Only ONE entry point, unlike lq:ol above: both
 * real call sites in scope (`self.jb:ol(127,1)`, `self.jb:ol(0,0)`,
 * pico8/lua.lua:269/938, Chunk 4/8 -- not yet ported) always pass BOTH
 * arguments, so there is no evidenced omitted-`a` case here to build a
 * `_default` entry point for (unlike lq:ol's own two real shapes). `gfx`
 * is non-const (unlike pico8_lq_ol's) since fillp()/fillp_reset() mutate
 * pico8_gfx_t's own fillp_pattern field. */
void pico8_jb_ol(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_fixed_t om, pico8_fixed_t a);

#endif
