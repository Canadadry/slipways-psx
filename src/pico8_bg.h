#ifndef PICO8_BG_H
#define PICO8_BG_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Phase 3, Chunk 7 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `bg`, a real `ke`-descended class (confirmed: `bg=ke:jr(...)`,
 * pico8/lua.lua:359 -- not assumed from the PRD's own prose). Reading its
 * own 5 real methods below, `bg` turns out to be exactly what the PRD's own
 * paraphrase says -- background/starfield rendering (`pq` procedurally
 * paints the PICO-8 map with a noise-driven starfield, `kj` draws the map
 * plus a faint dashed grid overlay and a border) plus camera-drag
 * (`rdown`/`hq`: right-mouse-drag pans the camera by accumulating mouse
 * deltas into the camera singleton `mo`'s own velocity field) -- confirmed
 * from the source, not just trusted. `ldown` (left-click) is a separate,
 * unrelated concern: it forward-references `mz`/`g_` (Chunks 10/16, not yet
 * ported) to start a probe/wormhole action, see this file's own comment on
 * pico8_bg_ldown below for the full trace of how that forward reference is
 * handled.
 *
 * bg=ke:jr([[
 * fa=1,
 * hd=1,
 * pl=o(0,0,0,72,73,74,75,88,89,90,91,88,91,89,91),
 * ]])function bg:pq()local vs,vh,oh,m=
 * {},0,jf(0.1,0)for y=0,31 do
 * for x=0,31 do
 * vh=oh()*8-4+
 * (vh+(vs[x]or 1))*0.5
 * vs[x]=vh
 * mset(x,y,rnd()<0.004
 * and 122+rnd(5)or bg.pl[flr(vh+rnd(4))])end
 * end
 * end
 * function bg:kj()map(0,0,0,0,32,32)fillp(0b1010010110100101)for xy=3,262,32 do
 * rectfill(xy,0,xy,262,1)rectfill(0,xy,262,xy)end
 * fillp()rect(0,0,262,262,5)end
 * function bg:ldown(mp)
 * return (#(cg.g_ or {})>0 and mz or g_)()end
 * function bg:rdown(mp,rp)self.mq=rp
 * return self
 * end
 * function bg:hq(mp,rp)mo.v=self.mq-rp
 * self.mq=rp
 * end
 * (pico8/lua.lua:359-383.)
 *
 * Class spec: `fa=1` (draw-priority bucket, read by `ui:gw()`'s own hit-
 * test-priority sort, Chunk 5 -- bg is priority 1, the lowest of anything
 * this port has seen so far, matching "background: everything else hit-
 * tests over it"), `hd=1` is FLAT DATA (like `fq.hd=1`, Chunk 4) -- bg
 * always "hits" `ui:gw()`'s hover test regardless of mouse position,
 * exactly like `fq`'s always-hit modal backdrop, since there's nothing
 * behind the background to miss. `pl=o(...)` a positional 15-entry number
 * table (ob()'s own unregistered-identifier fallback, pico8_ob.c's
 * "identifier(...)" branch -- `o` itself is never registered in `pi`, so
 * this needs no `pico8_pi_init()` defensive call the way `nf`/`qb`'s own
 * `v(...)`/`o(v(...))` specs did, Chunk 4's own established ordering
 * hazard: `pl`'s own positional entries are all plain numbers, never a
 * nested `v(...)`), read back as `bg.pl` (the CLASS table, not `self`) by
 * `bg:pq()` -- see pico8_bg_pq's own comment below for why this port reads
 * it from the class's own defaults directly rather than through the
 * instance field-lookup chain.
 *
 * `bg:pq()` is bg's OWN distinct extra-constructor, exactly like `nf:pq()`/
 * `js:pq()` (Chunk 4) -- fires SEPARATELY from `ke.pq=lt`'s inherited
 * auto-registration, both exactly once (pico8_class.c's own two-worked-
 * traces comment). `bg:kj()` IS one of `bm`'s 5 dispatch names (draw),
 * wired as a real PICO8_VALUE_FUNCTION field so `dc("kj")` can find and
 * call it, matching `js:kj`/`ui:ev`'s own precedent. `ldown`/`rdown`/`hq`
 * are NOT `bm` names, but are still wired as real PICO8_VALUE_FUNCTION
 * fields -- they're dispatched generically through `mv()`'s own unrestricted
 * by-name field lookup, the SAME mechanism `js.hd=gk`/`js:ldown`/`fq:ldown`
 * already use (`bm` only gates `dc()`, never `mv()`) -- via `ui:bh()`'s own
 * real call sites (pico8/lua.lua:237-247, already read for Chunk 5):
 * `self:jz(bt.."down",self.qa,je.ra)` (bt is "l" or "r", giving "ldown"/
 * "rdown", argc=2) on left/right mouse-button edge, and
 * `mv(held,"hq",self.qa,je.ra)` (argc=2) every frame the button stays held.
 * All three real bg methods take `(mp,rp)` in that argument order
 * (`mp`=`self.qa`, the world-space mouse position; `rp`=`je.ra`, the raw
 * screen-space one) even where the body only ever reads one of the two --
 * kept as real C parameters either way (`(void)mp;` where genuinely
 * unused) for literal signature fidelity with the pasted Lua source above,
 * the same choice this port already makes for e.g. `nf:pq`'s unused
 * `self`. */
void pico8_bg_init(void);
void pico8_bg_shutdown(void);

/* The real `bg` class, built once by pico8_bg_init(). Mirrors
 * pico8_scroll.h's own pico8_qb_class() convention -- not retrievable via
 * pico8_class_lookup() (registers under the shared "" name-registry slot,
 * like every other unnamed `:jr()` class so far). Returns NULL before
 * pico8_bg_init() (or after pico8_bg_shutdown()). */
pico8_class_t *pico8_bg_class(void);

/* `bg(overrides)` -- the real Lua constructor call (`_init()`'s own real
 * `bg()`, pico8/lua.lua:1182, Chunk 22 -- confirmed by grep, always called
 * with no overrides). Thin wrapper around pico8_class_instantiate(). */
pico8_instance_t *pico8_bg_new(const pico8_ob_table_t *overrides);

/* function bg:pq()local vs,vh,oh,m=
 * {},0,jf(0.1,0)for y=0,31 do
 * for x=0,31 do
 * vh=oh()*8-4+
 * (vh+(vs[x]or 1))*0.5
 * vs[x]=vh
 * mset(x,y,rnd()<0.004
 * and 122+rnd(5)or bg.pl[flr(vh+rnd(4))])end
 * end
 * end
 * (pico8/lua.lua:363-372.) Procedurally paints the 32x32 map region with a
 * smoothed-noise "terrain height" (`vh`, a 1D value per map column `x`,
 * blended row over row via `vs[x]` -- Lua's `vs` starts as an empty table,
 * so `vs[x]or 1` only ever falls back to 1 during the very first row (y=0);
 * every later row reads back the previous row's own `vh` at that same `x`)
 * driving a lookup into `bg.pl` (the 15-entry tile palette, read from the
 * CLASS's own default fields -- `bg.pl` in the real source is a plain
 * global-class-table read, not `self.pl`, and this class is never
 * subclassed, so reading straight off pico8_bg_class()'s own `defaults`
 * here is exactly equivalent to walking self's own class chain, just more
 * direct about which table the real source names), with a rare (0.4%)
 * chance per cell of a bright star-cluster tile (122+rnd(5)) instead.
 *
 * `local vs,vh,oh,m=...` -- Lua's own multi-assignment with 3 values for 4
 * names leaves `m` as nil, an unused/dead local in the real source (never
 * read again) -- reproduced faithfully by simply never declaring/using a
 * `m` in C, since it has no observable effect either way.
 *
 * `oh=jf(0.1,0)` (Chunk 1's pico8_jf, pico8_gen.h): a bounded-random-walk
 * generator, called once per cell (`oh()`) and freed once this function's
 * own 1024-cell loop finishes -- a genuinely local, single-use resource
 * (unlike an instance field this port otherwise leaves un-freed, see
 * pico8_class.h's own comment on instance ownership), so pico8_jf_free()
 * IS called here.
 *
 * `bg.pl[flr(vh+rnd(4))]` -- an index that can fall outside `pl`'s own
 * valid 1..15 range for extreme `vh` values (never separately bounds-
 * checked by the real source either): a missing/wrong-kind entry reads as
 * tile 0 here, the same documented defensive-absent-field convention this
 * port uses everywhere else (e.g. pico8_entity_geom.c's own gk/gd/na/ma) --
 * not a crash the way real Lua's own `mset(x,y,nil)` would be, and not
 * separately confirmed to ever actually occur for this noise algorithm's
 * own real amplitude (not exercised by any test in this chunk's own
 * scope). `mset`'s own `val` (a PICO-8 built-in taking a plain integer
 * tile id) truncates whatever fixed-point tile-id expression the real
 * source computes (`122+rnd(5)`, or a `pl` entry that itself is always a
 * whole number in the real spec) via the same truncating-cast convention
 * already established elsewhere in this port (e.g. pico8_ui_button.c's own
 * pico8_ui_to_int) -- not separately sourced PICO-8 rounding behavior,
 * since every real value this cart's own `pl` table or `rnd(5)` produces is
 * already a small non-negative number where truncation and rounding would
 * coincide in the overwhelming majority of draws anyway.
 *
 * Needs `mem` (mset()); pico8_bg_pq_dispatch (pico8_bg.c) is the small
 * adapter reading pico8_dispatch_mem() for pico8_class_pq_fn's own fixed
 * `void(*)(pico8_instance_t*)` shape, the same treatment as
 * pico8_mouse_reader.h's own nf:pq. A NULL `mem` is a safe no-op. */
void pico8_bg_pq(pico8_mem_t *mem, pico8_instance_t *self);

/* function bg:kj()map(0,0,0,0,32,32)fillp(0b1010010110100101)for xy=3,262,32 do
 * rectfill(xy,0,xy,262,1)rectfill(0,xy,262,xy)end
 * fillp()rect(0,0,262,262,5)end
 * (pico8/lua.lua:373-375.) Draws the already-painted 32x32 map region
 * (`map(0,0,0,0,32,32)`), then a faint dashed grid every 32px
 * (`fillp(0b1010010110100101)` = 0xA5A5, the pattern converted to hex for
 * the C call while the comment above keeps the exact Lua binary literal),
 * then a plain white-ish border (`rect(...,5)`).
 *
 * `rectfill(0,xy,262,xy)` -- the SECOND rectfill in this loop body omits
 * its trailing color argument, which real PICO-8 resolves to "whatever
 * color the previous draw call last used" (a `pico8_gfx_t`-level draw-color
 * state this port's Phase 1 gfx layer doesn't model -- the SAME general gap
 * already flagged, unresolved, for `pj:kj`'s own `rk(127-h,127)`, Chunk 18,
 * docs/portage-progress.md's Vague 1). Unlike that flagged case, THIS
 * specific omission is fully resolvable without modeling any such state at
 * all: the immediately preceding statement, on the very same loop
 * iteration with no branch or call in between, is `rectfill(xy,0,xy,262,1)`
 * -- color 1 -- so "the last color drawn" is statically known here to be 1,
 * not read from any runtime-tracked state. This port therefore passes `1`
 * explicitly for this call too, a literal resolution of a fully-determined
 * value, not a guess at the general "last drawn color" gap Chunk 18 still
 * has to solve for its own, less locally-determined call site.
 *
 * Needs `gfx`+`mem` (map()/fillp()/rectfill()/rect()); pico8_bg_kj_dispatch
 * (pico8_bg.c) reads pico8_dispatch_context.h's shared gfx/mem for dc()'s
 * own "kj" bm-name dispatch, the same treatment as `js:kj`/`js:ev`. A NULL
 * `gfx` or `mem` is a safe no-op. */
void pico8_bg_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self);

/* function bg:ldown(mp)
 * return (#(cg.g_ or {})>0 and mz or g_)()end
 * (pico8/lua.lua:376-377.) If any `g_`-tagged entity currently exists
 * (`cg.g_`, pico8_cg("g_") -- a probe already in flight), instantiate a new
 * `mz` (the wormhole/probe-arrival entity); otherwise instantiate a new
 * `g_` (a fresh probe) -- left-click on the background starts (or
 * escalates) a probe launch. Returns the freshly built instance as a
 * PICO8_VALUE_REF (a Lua "table"), matching `ui:bh()`'s own real
 * `type(pw.kn)=="table"` check (pico8/lua.lua:239) that decides whether to
 * hold onto it as `self.lheld`.
 *
 * FORWARD REFERENCE, PORTED NOT DEFERRED (per this PRD's own Implementation
 * Decisions): `mz`/`g_` (Chunks 10/16) don't exist yet when this chunk is
 * written. Neither ever sets an `fz` field (confirmed by reading pico8/
 * lua.lua -- only `pm`/`ga`, Chunk 15, do, for the UNRELATED `jo:hu`
 * forward reference), so they can't be reached through
 * pico8_class_lookup()'s existing fz-keyed registry without colliding with
 * every other unnamed class's shared "" slot. Resolved with a NEW, small,
 * separate registry this chunk adds (pico8_class_register_named/
 * pico8_class_lookup_named, pico8_class.h) keyed by an explicit lookup
 * name this port chooses ("mz"/"g_", matching the Lua global variable
 * names for readability, though the registry itself doesn't care what the
 * name is): the right class name is looked up via
 * pico8_class_lookup_named() here, TODAY returning NULL for both
 * (documented, "compiles, not yet exercisable" -- this function then
 * safely returns "nothing held", distinct from any real instance, rather
 * than crashing) until whichever of Chunks 10/16 lands FIRST calls
 * pico8_class_register_named("g_", ...) or
 * pico8_class_register_named("mz", ...) alongside its own
 * pico8_class_declare() call -- at which point THIS function starts
 * resolving and instantiating for real, with no further change needed
 * here. `self`/`mp` are both unused (the real Lua body references neither),
 * kept as real parameters for literal signature fidelity with the pasted
 * source above and this port's own established dispatch-adapter shape. */
pico8_value_t pico8_bg_ldown(pico8_instance_t *self, pico8_value_t mp);

/* function bg:rdown(mp,rp)self.mq=rp
 * return self
 * end
 * (pico8/lua.lua:378-380.) Right-mouse-down: remembers the raw mouse
 * position (`rp`) on `self.mq` (a plain field REBIND -- the same
 * pico8_value_t `rp` this port received is stored as-is, matching every
 * other "field write always lands on the instance" case, e.g.
 * pico8_scroll.c's own `self.p+=self.v`) and returns `self`, becoming
 * `ui.rheld` for as long as the button stays down (see this file's own top
 * comment on `ui:bh()`'s real call sites) -- the first half of camera-drag,
 * `hq` below is the second. `mp` is unused (the real Lua body never
 * references it). */
pico8_value_t pico8_bg_rdown(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp);

/* function bg:hq(mp,rp)mo.v=self.mq-rp
 * self.mq=rp
 * end
 * (pico8/lua.lua:381-383.) Called every frame the right button stays held
 * (via `ui:bh()`'s own `mv(held,"hq",self.qa,je.ra)`): sets the camera
 * singleton `mo`'s (pico8_mo(), pico8_entity.h) own `v` (velocity) field to
 * the delta between the PREVIOUSLY remembered raw mouse position
 * (`self.mq`, set by `rdown` above or the previous `hq` call) and the
 * current one (`rp`), then updates `self.mq` to `rp` for the next frame --
 * this is the camera-drag itself: `qb:ph()` (Chunk 4) integrates `mo.v`
 * into `mo.p`/`mo.ra` every tick the same way it does for D-pad input.
 * Order matters and is preserved: `self.mq` (the OLD value) is read before
 * being overwritten. `mp` is unused (the real Lua body never references
 * it). A NULL `self` is a safe no-op (nothing to read/write). */
void pico8_bg_hq(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp);

#endif
