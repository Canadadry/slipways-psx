#ifndef PICO8_ECONOMY_H
#define PICO8_ECONOMY_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Phase 3, Chunk 12 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md), "hy class + economy formulas": the `hy` class itself
 * (pico8/lua.lua:1198-1238, read directly) -- the economy singleton `fm`
 * (mo/je/ok's own precedent, but see the FM RECONCILIATION note below for
 * why THIS chunk deliberately does NOT build the real instance). Confirmed
 * by reading the real source (`hy=kv:jr()`, line 1198) to be a ROOT class
 * of the `kv:jr` system -- like `lq`/`jb`/`trade`/`hp` (Chunks 3/11), NOT
 * `ke`-descended, so it never self-registers into `hv`/`cv`/`cg`; none of
 * its own methods (`pq`,`nx`,`hk`,`lu`,`lj`,`kt`) are `bm` dispatch names
 * or ever called via mv()/dc() in the real source (every real call site
 * uses plain `self:method()` syntax, e.g. `d_`'s `fm:lu(...)`/`fm:nx(...)`,
 * Chunk 10) -- so all are ported as direct, known C functions, matching
 * `pico8_ke_ks`'s/`pico8_hp_po`'s own "known method, no dispatch table"
 * precedent, nothing structurally new for the entity/dispatch smoke test
 * to cover.
 *
 * Kept in its own module, separate from pico8_economy_formulas.c (the 13
 * standalone formulas `hy:lj()`/`hy:hk()` call into) -- the same
 * class-vs-standalone-functions split pico8_research.c/pico8_tech_gate.c
 * already established for `hp` vs `oj`/`ov`/`gi`/`cb` (Chunk 11). Not
 * pico8_probe.c (Chunk 10's own `pico8_fm()` PLACEHOLDER accessor, which
 * this module's own init deliberately does NOT touch -- see the FM
 * RECONCILIATION note below) or any of the already-oversized modules
 * (pico8_class.c/pico8_entity.c/pico8_ui.c/pico8_msgbox.c).
 *
 * *** FM RECONCILIATION DECISION *** (mirroring pico8_probe.h's own
 * "CHUNK 12 RECONCILIATION NOTE" verbatim in spirit, and resolving it
 * explicitly rather than guessing): Chunk 10's own placeholder note said
 * "whoever builds the real `hy` class must call
 * pico8_probe_set_fm(pico8_class_instantiate(hy_class, NULL))". Reading
 * the real source's own call graph settles WHO that is: `eu(nj)dk=nj
 * jl()bg()bw(90)fm,ju=hy(),pj()...` (pico8/lua.lua:1181-1186) -- `fm=hy()`
 * is built by `eu()`, Chunk 22, NOT by any earlier bootstrap step, and the
 * PRD's own "Singleton globals ownership" Implementation Decision already
 * says so explicitly ("`fm` ... constructed by `eu()`, Chunk 22"). So
 * THIS chunk's own `pico8_economy_init()` below builds ONLY the `hy`
 * CLASS (and its `pq` wiring) -- it deliberately does NOT call
 * `pico8_probe_set_fm()` and does NOT instantiate a real `fm` here, unlike
 * how `pico8_bootstrap.c`'s `pico8_jl()` reconciles `mo`/`je`/`ok` the
 * moment `qb`/`nf`/`ui` exist (Chunk 6) -- `fm` is a DIFFERENT case
 * because the PRD explicitly ties its construction to `eu()`'s own later
 * timing, not to "as soon as the class exists". Chunk 22's own `eu()`
 * must call `pico8_probe_set_fm(pico8_hy_new(NULL))` once
 * `pico8_economy_init()` has run (and, per the same real source line,
 * `pico8_economy_set_dk(...)`/`pj()`'s own construction alongside it) --
 * NOT done here, and NOT wired into `pico8_jl()`/`pico8_bootstrap.c`
 * prematurely. `pico8_fm()`'s own declaration/signature
 * (pico8_probe.h) is unchanged by this decision; every existing real
 * reader (`pico8_d_`, `pico8_oj`/`pico8_gi`/`pico8_cb`,
 * pico8_economy_formulas.c's own `ky`/`bf`) keeps reading through it
 * unmodified, still seeing the classless placeholder until Chunk 22 runs. */

void pico8_economy_init(void);
void pico8_economy_shutdown(void);

/* The real `hy` root class, built once by pico8_economy_init(). Not
 * retrievable via pico8_class_lookup() (registers under the shared ""
 * name-registry slot, like ke/lq/jb/trade/hp -- hy's own spec never sets
 * an `fz` field). Returns NULL before pico8_economy_init() (or after
 * pico8_economy_shutdown()). */
pico8_class_t *pico8_hy_class(void);

/* `hy(overrides)` -- the real Lua constructor call (`hy()`, line 1182,
 * always with NO overrides at its one real call site). Thin wrapper
 * around pico8_class_instantiate, matching pico8_hp_new/pico8_trade_new's
 * own established convention. Chunk 22's own `eu()` is the real caller
 * that must feed this result to pico8_probe_set_fm() -- see this file's
 * own top comment. */
pico8_instance_t *pico8_hy_new(const pico8_ob_table_t *overrides);

/* function hy:nx(hx)local kx=ov("time compression",12,15)
 * self.lo+=hx
 * self:lj()if self.lo>kx then
 * self.lo-=kx
 * self:hk()end
 * end
 * (pico8/lua.lua:1204-1209.) Advances the game clock by `hx` ticks:
 * accumulates `self.lo` (a tick counter), recomputes the economy every
 * call (`self:lj()`, unconditionally), then rolls over into a full
 * "year" (`self:hk()`) once `self.lo` exceeds the time-compression
 * threshold (`ov("time compression",12,15)`, Chunk 11's already-built
 * `pico8_ov` -- 15 once that upgrade is researched, 12 otherwise). A NULL
 * `self` is a safe no-op. */
void pico8_hy_nx(pico8_instance_t *self, pico8_fixed_t hx);

/* function hy:hk()
 * self.nz+=self.ge
 * self.jk+=self.po[r.k]+1
 * self.yr+=1
 * self:lj()for e in all(cv.hk)do
 * e:hk(self.yr)end
 * end
 * (pico8/lua.lua:1210-1216.) One full "year" tick: applies income
 * (`self.ge`) to credits (`self.nz`), accrues research points from the
 * "kind k" research-output entry of `self.po` (set by `self:lj()`, a REF
 * to a heap pico8_ob_table_t -- pico8_economy_formulas.h's own `bx()`),
 * advances the year, recomputes the economy, then dispatches `:hk(year)`
 * to every entity registered under the real `bm` name "hk" (`cv.hk`,
 * pico8_cv("hk")) -- a genuinely dynamic, cross-class dispatch (the whole
 * reason `bm`/`cv` exist), routed through pico8_mv() since the concrete
 * receiver class isn't known here, matching this port's established
 * forward-ref/dynamic-dispatch convention. `cv.hk` is empty until later
 * chunks' classes register real "hk" fields -- compiles, inert today. A
 * NULL `self` is a safe no-op. */
void pico8_hy_hk(pico8_instance_t *self);

/* function hy:lu(jt)if self.nz<jt then
 * iv({"-no money-",8})
 * return
 * end
 * self.nz-=jt
 * if jt~=0 then
 * iv({ne(-jt,10,"$")})end
 * return true
 * end
 * (pico8/lua.lua:1217-1225.) "Can afford / spend `jt`?": if `self.nz`
 * (credits) is short, shows a "-no money-" floating toast (`iv({...})`,
 * Chunk 8's ALREADY-BUILT `pico8_iv_new` -- a direct call, positional
 * overrides `self[1]`/`self[2]` matching `iv`'s own established
 * "text,color" real call-site shape, pico8_msgbox.h) and returns falsy
 * (no explicit `return` in the real else-less branch -> nil, ported as
 * 0); otherwise deducts `jt` and, if it's non-zero, shows a signed
 * spend/refund toast (`ne(-jt,10,"$")`, Chunk 1's already-built
 * `pico8_ne`, whose two return values fill the toast's own two
 * positional overrides exactly like the real Lua table constructor's own
 * multi-return expansion) and returns 1 (true). A NULL `self` reads
 * `self.nz` as 0 (this port's numeric-absence default), so it always
 * takes the "-no money-" branch for any positive `jt`. */
int pico8_hy_lu(pico8_instance_t *self, pico8_fixed_t jt);

/* function hy:lj()repeat
 * bd()until not bb()self.po,self.fu=
 * bx(),ck()local dd=0
 * for p in all(cg.jo)do
 * p.kr=flr(db(p)-
 * da(p))
 * dd+=p.kr
 * end
 * self.ge=flr(dd*ov("skill implants",1,1.15))- cz()self.happiness=
 * bf()self.hl=
 * self.yr>=3426 and "final score"or self.ge<=0 and self.nz<3 and "bankrupt"for e in all(cv.hb)do
 * e:hb()end
 * end
 * (pico8/lua.lua:1226-1238.) The full economy recompute: settles every
 * planet's development tier to a fixed point (`repeat bd() until not
 * bb()`, pico8_economy_formulas.h), rebuilds the production/demand tables
 * (`self.po,self.fu=bx(),ck()`, releasing whatever this instance's own
 * previous call built first), sums every planet's net output
 * (`db(p)-da(p)`, flr'd, into `p.kr`), derives income (`self.ge`, scaled
 * by the "skill implants" research upgrade via `pico8_ov`, minus the
 * upkeep formula `cz()`), happiness (`self.happiness=bf()`), and the
 * end-of-game status string `self.hl` -- `(self.yr>=3426 and "final
 * score") or (self.ge<=0 and self.nz<3 and "bankrupt")`, which is real
 * Lua boolean `false` (not nil) whenever NEITHER condition holds (the
 * `and`-chain's own last falsy operand is what the whole `or` expression
 * evaluates to) -- ported as PICO8_VALUE_BOOL(0) in that case, not a
 * guessed default. Finally dispatches `:hb()` (no arguments) to every "hb"
 * `bm`-registered entity (`cv.hb`), the same forward-ref pico8_mv()
 * dispatch as `hy:hk()` above. A NULL `self` is a safe no-op. */
void pico8_hy_lj(pico8_instance_t *self);

/* function hy:kt(t)
 * self.jk-=gi(t)
 * add(self.invented,t.px)
 * if self.ew<4
 * and #self.invented>=ek[self.ew]then
 * self.ew+=1
 * end
 * self:lj()
 * end
 * (pico8/lua.lua:1446-1454.) Commits a researched tech `t` (an `nb`
 * entry, Chunk 11): spends its cost (`gi(t)`, Chunk 11's already-built
 * `pico8_gi`), records it (`add(self.invented,t.px)` -- `self.invented`,
 * a REF to a heap pico8_list_t, see pico8_economy.c's own `pico8_hy_pq`
 * comment on why this differs from its own raw `invented=o()` spec
 * value), advances the tech era (`self.ew`) once enough techs are
 * invented for the current era's own threshold (`ek[self.ew]`, Chunk
 * 11's already-built pico8_ek()), then recomputes the economy
 * (`self:lj()`). `t` is `const` (gi() doesn't mutate it, and `t.px` is
 * only read). A NULL `self` is a safe no-op. */
void pico8_hy_kt(pico8_instance_t *self, const pico8_ob_table_t *t);

#endif
