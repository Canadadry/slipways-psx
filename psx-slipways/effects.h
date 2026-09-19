#ifndef EFFECTS_H
#define EFFECTS_H

#include "fixed.h"
#include "rng.h"

/* One trade-flow particle traveling along a slipway segment. All fields are
 * fixed_t (Q16.16), screen-space units -- mirrors t.ra (position), t.v
 * (velocity), t.l (lifetime) inside jc:kj's self.ts table
 * (pico8/lua.lua:657-660, 673-678). See docs/prd/triage/psx-effects.md,
 * "Data model and function signatures". */
typedef struct {
    fixed_t x, y;
    fixed_t vx, vy;
    fixed_t lifetime;
} effects_flow_particle_t;

/* Spawn-trial threshold -- ports jc:ch's literal "rnd()<0.08"
 * (pico8/lua.lua:657: "function jc:ch(f,t)if rnd()<0.08 and
 * self.hg[t.id]then"). PICO-8's rnd() (no argument) draws uniformly in
 * [0,1); a plain Bernoulli(0.08) trial, independent per call. See
 * docs/prd/triage/psx-effects.md, Implementation Decisions / "The spawn
 * formula" for the full trail. */
#define EFFECTS_FLOW_SPAWN_CHANCE ((fixed_t)(0.08 * (double)FX_ONE))

/*
 * effects_flow_try_spawn -- ports jc:ch(f,t), pico8/lua.lua:657-660:
 *
 *   function jc:ch(f,t)if rnd()<0.08 and self.hg[t.id]then
 *   local d=t.ra-f.ra
 *   local v=d:nn()add(self.ts,{ra=f.ra+v:pa(),v=v,l=d:qx()})end
 *   end
 *
 * Draws one Bernoulli(0.08) spawn trial via the project's injected rng_t
 * (rng_next_range(rng, 0, FX_ONE) < EFFECTS_FLOW_SPAWN_CHANCE -- the exact
 * "a fixed_t span's raw int32_t value used directly as rng_next_range's
 * exclusive bound" idiom rules_discovery_roll_empty/rules_discovery.c
 * already establishes for this codebase). `active` stands in for
 * self.hg[t.id] -- true only when a (not-yet-existing) live trade
 * computation has marked that trade is currently flowing from `from`
 * toward `to`; see docs/prd/triage/psx-effects.md's Implementation
 * Decisions for exactly which live state (bd(), pico8/lua.lua:1265-1275,
 * itself driven by pt:ib()) would set this flag if it existed. No live
 * equivalent exists anywhere in this port yet, so the caller supplies it
 * explicitly, mirroring rules_discovery_probe_in_range's identical
 * caller-supplied-not-a-live-global precedent.
 *
 * Lua's `and` short-circuits left-to-right, but rnd() is the LEFT operand
 * of "rnd()<0.08 and self.hg[t.id]" -- so source always draws its rnd()
 * value first, on every call, regardless of whether self.hg[t.id] (here,
 * `active`) is true or false. This port mirrors that exact evaluation
 * order: the RNG draw always happens first, unconditionally, even when
 * `active` is 0 -- an `active=0` call still consumes one rng_next_range
 * draw, it just never spawns regardless of that draw's result.
 *
 * On a successful spawn (draw < EFFECTS_FLOW_SPAWN_CHANCE, active nonzero,
 * and from != to): `d = to - from`; the unit direction `v = d:nn()`
 * (km:nn(), pico8/lua.lua:113-114) becomes the particle's velocity
 * (vx, vy); the spawn position is offset from `from` by `v:pa()`
 * (km:pa(), pico8/lua.lua:115-116: an exact 90-degree rotation
 * (x,y)->(-y,x), magnitude-preserving, so v:pa() is also unit-length),
 * i.e. `out->x = from_x + (-vy)`, `out->y = from_y + vx`; the lifetime is
 * `d:qx()` (km:qx(), pico8/lua.lua:117-118), the LINEAR (not squared)
 * distance from `from` to `to`.
 *
 * Because the spawn position is already offset from `from` by one
 * perpendicular unit before the particle travels distance `l` in direction
 * `v`, the particle's real-valued endpoint is `spawn + v*l = from + v:pa()
 * + (to-from) = to + v:pa()` -- offset from `to` by that same one-unit
 * perpendicular vector, NOT exactly `to`. This is inherent to the source
 * formula itself (both the spawn offset and the direct-distance lifetime
 * are taken from `from`/`to`, not from the already-offset spawn point),
 * not a porting bug -- a faithful port reproduces this same near-`to`
 * convergence rather than exact arrival at `to`.
 *
 * `from == to` (both x and y equal) is a defensive no-spawn case: source
 * structurally never calls jc:ch with f==t (a slipway's two endpoints are
 * always distinct planets), so km:nn()'s implicit division-by-zero-length
 * case is never exercised in source, but this function still defines a
 * behavior for it (no spawn) purely as ordinary defensive robustness for a
 * host-tested pure function.
 *
 * Returns 1 and populates *out on a successful spawn, 0 otherwise (the
 * Bernoulli trial failed, active was false, or from == to). rng is threaded
 * by the caller, matching every other RNG-injected function in this
 * codebase.
 */
int effects_flow_try_spawn(fixed_t from_x, fixed_t from_y, fixed_t to_x, fixed_t to_y, int active, rng_t *rng, effects_flow_particle_t *out);

/*
 * effects_flow_step_particle -- ports the per-particle update block inside
 * jc:kj's loop, pico8/lua.lua:673-678:
 *
 *   self:lg(-1,2,3,bc)self:lg(0,1,1,c)for _,t in pairs(self.ts)do
 *   t.ra+=t.v
 *   t.l-=1
 *   if (t.l<=0)del(self.ts,t)
 *   local x,y=t.ra.x,t.ra.y
 *   rectfill(x,y,x+1,y+1,0)pset(x,y,tc)end
 *
 * Position advance (t.ra+=t.v, line 674): straight-line addition of the
 * particle's stored (unit) velocity, once per call -- ported as
 * `p->x = fixed_add(p->x, p->vx); p->y = fixed_add(p->y, p->vy);`.
 *
 * Lifetime and death (t.l-=1 then if(t.l<=0)del(...), line 675-676): PICO-8's
 * own numbers are natively Q16.16 fixed-point -- the exact same
 * representation fixed_t already uses throughout this codebase. So `t.l`'s
 * literal "1" here is not a plain integer frame counter that needs
 * rescaling on the way in; it is already one full PICO-8 number-unit in
 * that same Q16.16 representation, i.e. exactly FX_ONE in this port's own
 * convention. Ported as `p->lifetime = fixed_sub(p->lifetime, FX_ONE);`,
 * keeping `lifetime` a fixed_t throughout rather than truncating it to a
 * plain `int` frame count upfront -- matching `l`'s literal
 * fractional-capable representation in source (see
 * docs/prd/triage/psx-effects.md, Implementation Decisions / "The
 * particle-update formula").
 *
 * Returns 1 if the particle is still alive (lifetime > 0) after the
 * decrement, or 0 if it just died (lifetime <= 0) -- source's own
 * `t.l<=0` deletion test, inverted to an alive/dead result. The caller,
 * not this function, is responsible for removing a dead particle from
 * whatever list it lives in (this step introduces no such list -- see
 * docs/prd/triage/psx-effects.md's Out of Scope).
 *
 * Mutates *p in place, matching effects_flow_try_spawn's established
 * in/out-parameter convention (the caller owns and threads the particle's
 * state across calls).
 *
 * The drawing calls (rectfill/pset, line 677-678) and the self:ch() spawn
 * calls immediately following the update loop (line 679) are not part of
 * this function -- pure rendering and pure spawning are each already
 * covered (or, for rendering, explicitly out of scope) elsewhere.
 */
int effects_flow_step_particle(effects_flow_particle_t *p);

/*
 * effects_kick_t -- the planet "kick" wobble's decaying amplitude, ported
 * from `ma(p,e)`/`jo:kd()` (pico8/lua.lua:349-350,468-469):
 *
 *   function ma(p,e)e.ma=max(0,e.ma-0.1)
 *   return p+qm(e.ma,e.t*0.2)end
 *   ...
 *   function jo:kd()self.ma=1.25
 *   end
 *
 * `self.ma` (a single planet-entity field in source) is the whole of this
 * effect's state -- carried here as its own small struct, mirroring
 * effects_flow_particle_t's established "one struct per traced piece of
 * per-instance state" shape, rather than a bare fixed_t the caller has to
 * remember the meaning of. See docs/prd/triage/psx-juice-effects.md,
 * "### 2. Planet "kick" wobble" for the full trace (trigger call sites,
 * decay-per-draw-call cadence, and the exact 13-call-to-zero lifecycle) and
 * the Data Model section for this struct's/these functions' signatures.
 */
typedef struct {
    fixed_t amplitude;
} effects_kick_t;

/*
 * effects_kick_start -- ports `jo:kd()` (pico8/lua.lua:468-469):
 * `self.ma=1.25`. Sets *k's amplitude to the literal starting value
 * `1.25*FX_ONE`, matching source's own literal constant exactly (not
 * derived from anything else). Called whenever source would call `jo:kd()`
 * -- on discovery, colonization, or a new slipway connecting to this
 * planet (see effects.h's block comment above for all three cited call
 * sites; none is wired to a live event in this port yet, matching this
 * step's Out of Scope).
 */
void effects_kick_start(effects_kick_t *k);

/*
 * effects_kick_step -- ports `ma(p,e)`'s own amplitude decay,
 * `e.ma=max(0,e.ma-0.1)` (pico8/lua.lua:349), in isolation from that same
 * function's position-offset return value (which effects_kick_offset below
 * covers separately, matching this codebase's established "one function per
 * traced piece of behavior" style rather than one function doing both).
 * Decays *k's amplitude by exactly `0.1*FX_ONE` per call, floored at 0 --
 * source's own `max(0, ...)`. Called once per draw call, exactly like
 * source's own `ma()` (see effects.h's block comment above: PICO-8's
 * `jo:kj` is a `dc("kj")` draw-pass function, not a tick function, though
 * this port's own established 60fps 1:1 frame correspondence means the two
 * cadences coincide in practice).
 */
void effects_kick_step(effects_kick_t *k);

/*
 * effects_kick_offset -- ports `ma(p,e)`'s own position-offset return value,
 * `return p+qm(e.ma,e.t*0.2)` (pico8/lua.lua:350), in isolation from that
 * same function's amplitude decay (effects_kick_step above covers that
 * separately). Writes the offset (NOT `p` itself -- the caller adds this to
 * whatever base position `p` it's tracking, exactly like effects_flow's own
 * in/out-parameter convention) to *out_dx and *out_dy: a point at distance
 * k->amplitude from the origin, at an angle sweeping `0.2` turns per tick of
 * age_ticks -- `qm(e.ma, e.t*0.2)`, i.e. `fixed_polar(k->amplitude,
 * fixed_mul(age_ticks, <0.2 turns in Q16.16>), out_dx, out_dy)`. Note the
 * argument order: source scales the AGE by 0.2 turns/call, not the
 * amplitude -- `e.t*0.2`, not `e.ma*0.2` -- so fixed_polar's first argument
 * (magnitude) is k->amplitude and its second (turns) is the scaled age, not
 * the other way around. A thin wrapper around fixed_polar, but per this
 * step's Data Model it is tested and called as its own named function, not
 * inlined at call sites.
 */
void effects_kick_offset(const effects_kick_t *k, fixed_t age_ticks, fixed_t *out_dx, fixed_t *out_dy);

/*
 * effects_idle_bob_delta -- ports `na(self,t)`'s per-tick vertical delta,
 * pico8/lua.lua:577-579:
 *
 *   function na(self,t)
 *   self.ra.y+=sin(t*0.005)*0.06
 *   end
 *
 * `na()` itself **directly mutates** `self.ra.y` (the entity's own
 * screen-space render position) in place, every tick. Unlike that, this is
 * ported as a pure function returning that same per-tick delta for the
 * caller to add -- matching `ma()`'s/effects_kick_offset's own
 * function-not-mutation shape rather than `na()`'s literal
 * mutate-in-place shape, since every other primitive in this codebase (and
 * docs/prd/psx-effects.md's own established convention) returns a value for
 * the caller to apply, never reaches into a shared struct field directly.
 * See docs/prd/triage/psx-juice-effects.md, "### 3. Trade-league idle bob"
 * for the full trace -- trigger state (`self.me=="ly"`, the "trade league"
 * special-resource planet type's settled state), cadence, and why this
 * stays unwired (no live "trade league" planet-type assignment exists in
 * this port yet, `rules_planets.c`'s own 3-of-10-type coverage doesn't
 * include it).
 *
 * `sin(t*0.005)`: `t` (age_ticks) scaled by `0.005` turns/tick, fed to
 * fixed_sin -- NOT fixed_cos; this is a plain sine wave, unlike
 * effects_kick_offset's combined cos/sin pair via fixed_polar. PICO-8's own
 * turns-based, sign-flipped sin() convention applies here exactly as it
 * does everywhere else fixed_sin is used (see fixed.h's fixed_sin/fixed_cos
 * block comment) -- a continuous, non-decaying oscillation for as long as
 * the caller keeps advancing age_ticks (unlike the kick wobble, this never
 * settles to zero).
 *
 * `*0.06`: source's own literal amplitude, in PICO-8's native 128-unit
 * coordinate space -- kept here as a literal Q16.16 constant, NOT rescaled
 * to this port's own screen resolution, exactly matching
 * EFFECTS_KICK_START_AMPLITUDE's identical "keep the traced literal,
 * caller applies whatever screen-space scale it needs" precedent
 * (effects.c) -- this function has no opinion on screen_map.c's own
 * PICO-8-128-to-PSX-screen-space rescale (screen_map_planet), since unlike
 * that rescale (which converts a WORLD position), this is a small
 * *offset* added on top of an already-placed screen-space position, and
 * no live "trade league" caller exists yet to settle what scale it would
 * need applied at. A future caller wiring this in decides that, the same
 * way effects_kick_offset's own amplitude is left unscaled today.
 *
 * age_ticks follows this codebase's established "PICO-8's own numbers are
 * natively Q16.16" convention (see effects_flow_step_particle's doc
 * comment on `t.l`/effects_kick_offset's own age_ticks): a plain tick count
 * is passed as that many whole FX_ONE units (e.g. tick 200 is
 * `200 * FX_ONE`, not the plain integer `200`), matching every other
 * age/tick-counter argument in this file.
 */
fixed_t effects_idle_bob_delta(fixed_t age_ticks);

/*
 * EFFECTS_SLIPWAY_REVEAL_RAMP -- ports `jc`'s own `k_` prototype field,
 * pico8/lua.lua:618: `k_=o(0,0,7,7,10,10,9,9,9,9,9,9,9,9,4,4,4,2,2,1)`. 20
 * entries, PICO-8 palette indices, captured verbatim as raw index data (no
 * RGB conversion at this layer) -- the same "traced-but-unmapped
 * color/index data captured as-is" precedent sprite_atlas.h's own `SPRITE_*`
 * constants already establish, and following FIXED_QUARTER_SIN_TABLE's own
 * "static const array declared directly in its header" convention (fixed.h)
 * since this codebase has no other precedent for an extern-const-array
 * split across a .h/.c pair. See docs/prd/triage/psx-juice-effects.md,
 * "### 4. Slipway newly-built color-reveal ramp" for the full trace.
 */
#define EFFECTS_SLIPWAY_REVEAL_RAMP_LENGTH 20

static const int EFFECTS_SLIPWAY_REVEAL_RAMP[EFFECTS_SLIPWAY_REVEAL_RAMP_LENGTH] = {
    0, 0, 7, 7, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 4, 4, 4, 2, 2, 1,
};

/*
 * effects_reveal_ramp_step -- ports `jc:kj`'s own read-then-advance of the
 * reveal-ramp counter `self.nh` against `self.k_`, pico8/lua.lua:667-670:
 *
 *   function jc:kj()if gd(self.qu)or gd(self.re)then
 *   local jv=
 *   self.k_[self.nh]
 *   if (self.nh<20)self.nh+=1
 *
 * `self.nh` (a single `jc`-instance field in source, prototype-defaulted to
 * `20` at pico8/lua.lua:619 -- an in-progress/never-released drag always
 * reads the last, fully-settled ramp entry) is reset to `1` on a successful
 * drag-release by `jc:ey(re)` (pico8/lua.lua:633-640: `self.nh,self.re=1,re`,
 * inside the "drag completed into a new connection" success path). Neither
 * that prototype default nor the reset call is this function's job -- both
 * are ordinary caller-owned state, exactly like effects_flow_step_particle's
 * own particle struct: the caller allocates and initializes its own `int nh`
 * (to `20` for "never dragged"/"in progress", or resets it to `1` on a
 * successful release, mirroring `jc:ey`) and threads it through this
 * function via the `int *nh` out-parameter, matching this codebase's
 * established in/out-parameter convention (effects_flow_step_particle,
 * effects_kick_step) rather than this function owning any state itself.
 *
 * Each call reads `*nh` (the index to use THIS call) BEFORE advancing it --
 * source's own `local jv=self.k_[self.nh]` runs before `self.nh+=1`, so the
 * value returned by call N (from a fresh reset, N starting at 1) is exactly
 * N itself, for N in [1,20], then holds at 20 forever after (source's own
 * `if (self.nh<20)self.nh+=1`, i.e. the increment is skipped once `self.nh`
 * already equals its 20 cap -- never overshoots). Returns that pre-increment
 * value (the 1-based index into EFFECTS_SLIPWAY_REVEAL_RAMP above the caller
 * should read for this call -- source's own `local jv=self.k_[self.nh]`,
 * i.e. `EFFECTS_SLIPWAY_REVEAL_RAMP[effects_reveal_ramp_step(&nh) - 1]` for
 * this function's 1-based return value against this port's 0-based array),
 * NOT the color value itself -- this function is the pure counter only,
 * mirroring this codebase's "one function per traced piece of behavior"
 * style (effects_kick_step/effects_kick_offset's own identical split)
 * rather than folding the table lookup in here too.
 *
 * This chunk is independently implementable: unlike every other function in
 * this file, it has no dependency on fixed_t/fixed_sin/fixed_polar at all --
 * a pure int-counter, per docs/prd/triage/psx-juice-effects.md's
 * Implementation Chunks item 5.
 */
int effects_reveal_ramp_step(int *nh);

/*
 * effects_probe_pulse_radius / effects_probe_sweep_radius -- port the probe
 * entity `mz`'s two radius-envelope formulas, `mz:kz(t)`/`mz:ej(t)`
 * (pico8/lua.lua:772-796):
 *
 *   function mz:kz(t)self.oi=ov("space folding",29,35)*
 *   (min(t/15,1)+sin(t/40)*0.1)end
 *   ...
 *   function mz:ej(t)
 *   return min(sqrt(t/15),1)*self.oi
 *   end
 *
 * Two independently-traced formulas, each its own function, matching this
 * file's established "one function per traced piece of behavior" style
 * (effects_kick_step/effects_kick_offset's own identical split). Neither the
 * `mz:kz(t)`/`mz:pr(t)`/`mz:ej(t)` state-name dispatch (`self.me`, same
 * mechanism as the idle bob's own `self.me=="ly"` gate) nor the discovery
 * marking inside `mz:pr` (`pt:hh()`) is this pair's job -- both are out of
 * scope here (no live probe entity exists yet; see
 * docs/prd/triage/psx-juice-effects.md's Out of Scope, "Probe reveal-sweep
 * pulse's live wiring, and the discovery logic inside `mz:pr` itself"). See
 * that PRD's "### 5. Probe reveal-sweep pulse" section for the full trace.
 *
 * `t` follows this file's established "PICO-8's own numbers are natively
 * Q16.16, so a plain tick count is passed as that many whole FX_ONE units"
 * convention (effects_kick_offset's/effects_idle_bob_delta's own age_ticks;
 * e.g. tick 15 is `15*FX_ONE`, not the plain integer `15`) -- and, per
 * `ej`'s own use inside `mz:kj`'s "pr"-state draw branch (`self:ej(self.t-dt)`,
 * pico8/lua.lua:788), can legitimately be fractional (a non-integer number of
 * ticks), which is exactly why this pair's own host tests include a
 * fractional `t` (7.5) for the sweep, not just whole-tick values.
 */

/*
 * effects_probe_pulse_radius -- ports `mz:kz(t)`'s own single assignment,
 * `self.oi = base_radius * (min(t/15,1) + sin(t/40)*0.1)`. `base_radius` is
 * `ov("space folding",29,35)` in source -- a tech-gated constant (29
 * PICO-8-pixel units base, 35 with the "space folding" tech researched, the
 * same tech-gated-constant idiom `docs/prd/psx-rules-tech.md` already
 * established) -- but the tech lookup itself is NOT this function's job,
 * matching this step's own explicit scope ("take `base_radius` as a plain
 * parameter... the tech-gated lookup itself belongs to whichever future step
 * wires this live"); the caller supplies whichever of the two literal values
 * (or any other) applies.
 *
 * The envelope has two parts, added together before scaling by
 * `base_radius`:
 *   - `min(t/15,1)`: a linear ramp-up over the first 15 ticks, reaching 1
 *     and staying there -- clamped here the same way effects_probe_sweep_
 *     radius's own `min(sqrt(t/15),1)` is clamped below (a plain
 *     greater-than-FX_ONE clamp, matching effects_kick_step's own
 *     `max(0,...)` floor's identical inline-ternary style rather than a new
 *     fixed_min/fixed_max helper this codebase has no other precedent for).
 *   - `sin(t/40)*0.1`: a small, PERMANENT, non-decaying sinusoidal
 *     "breathing" term -- unlike every other effect in this file (kick
 *     wobble, idle bob's own continuous-but-never-settling oscillation
 *     aside, reveal ramp), this term never turns off for as long as the
 *     caller keeps advancing `t`. `t/40` is a plain numeric ratio in the
 *     min()/sqrt() sense (t divided by 40), NOT already "turns" by itself --
 *     but PICO-8's own sin() interprets whatever argument it's given as
 *     turns (see fixed.h's fixed_sin block comment), so this same ratio,
 *     once computed, is already in exactly the right units to feed
 *     fixed_sin directly with no further conversion -- the identical
 *     "the ratio IS the turns argument, per PICO-8's own sin() convention"
 *     pattern effects_idle_bob_delta's own `sin(t*0.005)` already
 *     establishes (effects.h's doc comment on that function).
 *
 * `t/15` and `t/40` are both ordinary divisions by a compile-time-constant
 * divisor -- fixed.h has no fixed_div (chunk 1 only added fixed_sqrt
 * alongside the pre-existing fixed_add/fixed_sub/fixed_mul), so, per this
 * codebase's own established convention for dividing a fixed_t by a known
 * constant (RULES_ECONOMY_SLIPWAY_COST_DIVISOR_RECIP, rules_economy.c: `x/n`
 * becomes `fixed_mul(x, 1/n)`, the reciprocal folded once at compile time,
 * NOT effects.c's own separate `effects_fixed_div` helper, which is for
 * dividing by a runtime-computed value, e.g. a particle's own travel
 * distance -- not this pair's case), both divisions here are a `fixed_mul`
 * against a precomputed reciprocal constant.
 */
fixed_t effects_probe_pulse_radius(fixed_t base_radius, fixed_t t);

/*
 * effects_probe_sweep_radius -- ports `mz:ej(t)`'s own return value,
 * `min(sqrt(t/15),1)*self.oi`. `frozen_radius` is `self.oi` -- NOT computed
 * by this function -- source's own comment at `mz:pr`'s call site frames it
 * as "the frozen `self.oi` value" (captured from whatever
 * effects_probe_pulse_radius last computed when scanning stopped); the
 * caller owns and threads that value exactly like effects_kick_offset's own
 * caller-owned `effects_kick_t` amplitude.
 *
 * A square-root-eased (fast-then-slowing, unlike effects_probe_pulse_
 * radius's own LINEAR ramp-up) expansion from 0 to `frozen_radius` over 15
 * ticks: `t/15` (same reciprocal-multiply division idiom as
 * effects_probe_pulse_radius above -- a separately-named/defined constant
 * from that function's own `t/15` divisor, even though both are literally
 * 1/15, since `kz` and `ej` are two independently-traced source formulas,
 * matching this file's "one macro per traced literal, tied to its own
 * source citation" style elsewhere) is fed to `fixed_sqrt` (already in
 * fixed.h, from chunk 1's own predecessor -- this step's own "don't touch
 * fixed.h" constraint applies here unmodified: `fixed_sqrt` already exists
 * and needs no changes), then clamped to at most `FX_ONE` -- source's own
 * `min(sqrt(t/15),1)` -- the same inline-ternary clamp style
 * effects_probe_pulse_radius's own ramp-up clamp uses above, before scaling
 * by `frozen_radius`.
 */
fixed_t effects_probe_sweep_radius(fixed_t frozen_radius, fixed_t t);

/*
 * effects_spiral_spawn_count -- ports `ez(self,n)`'s spawn-count formula,
 * pico8/lua.lua:687-691:
 *
 *   function ez(self,n)while rnd()<n do
 *   add(self.qy,self:mm())
 *   n-=1
 *   end
 *   end
 *
 * This chunk is ONLY the counting formula itself (the `while rnd()<n do ...
 * n-=1 end` loop) -- `add(self.qy,self:mm())` (constructing and appending an
 * actual particle) is `effects_spiral_try_spawn_particle`'s job, a separate
 * chunk not implemented here; this function only tells the caller HOW MANY
 * particles to spawn this call, matching this file's established "one
 * function per traced piece of behavior" style (effects_kick_step/
 * effects_kick_offset's own identical split).
 *
 * PICO-8's `rnd()` (no argument) draws uniformly in [0,1) -- the same
 * "raw int32_t span used directly as rng_next_range's exclusive bound"
 * idiom effects_flow_try_spawn (above) and
 * rules_discovery_roll_empty (rules_discovery.c) already establish for this
 * codebase, via `rng_next_range(rng, 0, FX_ONE)`. Since a [0,1) draw is
 * strictly less than any integer k>=1, every iteration up to `n`'s integer
 * part always succeeds (`rnd()<k` is always true for integer k>=1), so this
 * function guarantees exactly `floor(rate/FX_ONE)` spawns, plus one more
 * with probability equal to `rate`'s own fractional part (the final
 * iteration, whose remaining rate is that fractional part, e.g.
 * `rnd()<0.5`) -- expected value exactly `rate` (in real terms) for any
 * `rate>=0`. See docs/prd/triage/psx-juice-effects.md, "### 1. Spiral
 * particle engine" for the full trace and Implementation Chunks item 7 for
 * this chunk's own scope.
 *
 * `rate<=0` returns 0 immediately, never looping or drawing -- source's own
 * `while rnd()<n do` never enters its body for n<=0 either (rnd() is always
 * >=0, so `rnd()<n` is always false when n<=0), so the observable spawn
 * count is identical either way; this port simply checks `rate>0` up front
 * rather than drawing once just to discover the condition is already false.
 *
 * `rate` and each loop iteration's decrement (`n-=1`, ported as subtracting
 * exactly `FX_ONE`) follow this file's established "PICO-8's own numbers
 * are natively Q16.16, so a plain per-particle unit is exactly FX_ONE, not
 * a separately-rescaled integer" convention (effects_flow_step_particle's
 * own `t.l-=1` -> `FX_ONE` precedent). `rng` is threaded by the caller,
 * matching every other RNG-injected function in this codebase.
 */
int effects_spiral_spawn_count(fixed_t rate, rng_t *rng);

/*
 * effects_spiral_particle_t -- one particle owned by the generic spiral
 * particle engine (`f_`/`ez`/`ei`/`g_:mm`, pico8/lua.lua:685-730). Ports
 * `ei`'s own per-particle table entry, `{p=...,v=...,l=...}` (and, for the
 * `ga:mm` variant this PRD traces but does not port -- see
 * docs/prd/triage/psx-juice-effects.md, Implementation Decisions item 1,
 * "Variants not fully ported" -- an optional `.a` acceleration field):
 *
 *   x, y      -- p (position), Q16.16, engine-center-relative.
 *   vx, vy    -- v (velocity), Q16.16, added to x/y once per step
 *                (effects_spiral_step_particle, chunk 9 -- NOT this chunk).
 *   life      -- l (lifetime/age-remaining), Q16.16, decremented by a flat
 *                per-tick amount and compared against a `<=1*FX_ONE` (NOT
 *                `<=0`) death threshold in source -- both effects_spiral_
 *                step_particle's job, not this struct's or this chunk's.
 *   ax, ay    -- a (the `ga:mm`-only optional acceleration term), added to
 *                v once per step when present.
 *   has_accel -- an explicit flag for whether ax/ay applies, ported in
 *                place of Lua's own `if(p.a)` truthy-table-or-nil check --
 *                matching this codebase's established preference for
 *                explicit flags over sentinel values (a magic ax==ay==0
 *                meaning "no acceleration" would collide with a real,
 *                legitimate zero acceleration).
 *
 * effects_spiral_try_spawn_particle (below) always initializes has_accel to
 * 0 and ax/ay to 0 -- the wormhole's own particles (`g_:mm`, this chunk's
 * ported reference variant) never set `.a`; only `ga:mm` does, and `ga` is
 * out of scope for this whole PRD (Implementation Decisions item 1).
 */
typedef struct {
    fixed_t x, y;
    fixed_t vx, vy;
    fixed_t life;
    fixed_t ax, ay;
    int has_accel;
} effects_spiral_particle_t;

/*
 * effects_spiral_try_spawn_particle -- ports `g_:mm()`, the wormhole's own
 * spawn-particle constructor and this generic engine's ported reference
 * variant (pico8/lua.lua:727-730):
 *
 *   function g_:mm()local a=(flr(rnd(self.pn))+rnd(0.5))/self.pn+self.t*0.001
 *   local p=qm(self.du+rnd(),a)local v=p*self.py
 *   return {p=p,v=self.pk and v:pa()or v,l=self.ki
 *   }
 *   end
 *
 * Called once per new particle by whatever loop calls `ez(self,n)`'s own
 * `add(self.qy,self:mm())` -- that loop (drive this function N times, once
 * per spawn count from effects_spiral_spawn_count) is the caller's job, not
 * this function's; this is only the single-particle constructor `self:mm()`
 * itself.
 *
 * Parameters map directly to g_:mm's own per-type constants/state (source
 * name -> parameter, wormhole's own literal value in parens, per this PRD's
 * "Wormhole's full parameter set"):
 *   base_radius    -- du (2*FX_ONE)     -- base spawn radius.
 *   sectors        -- pn (3)            -- angle-quantization sector count.
 *   tangential     -- pk (1/true)       -- velocity mode: 0 = radially
 *                                          outward, nonzero = rotated 90
 *                                          degrees (tangential/orbital).
 *   age_ticks      -- self.t            -- the owning entity's own
 *                                          age-since-:ks() counter (same
 *                                          field effects.h's dispatch-
 *                                          mechanism note above effects_
 *                                          spiral_spawn_count already cites).
 *   velocity_scale -- py (0.06*FX_ONE)  -- scales the radial spawn vector
 *                                          into the initial velocity.
 *   lifetime       -- ki (9*FX_ONE)     -- flat initial lifetime, no
 *                                          randomness (unlike position).
 *
 * **Angle** (`a=(flr(rnd(pn))+rnd(0.5))/pn+t*0.001`): three RNG draws occur
 * here, in this exact order -- matching source's own left-to-right
 * evaluation, the same "preserve Lua's own draw order" precedent effects_
 * flow_try_spawn's doc comment above already establishes:
 *   1. `flr(rnd(pn))` -- a uniform INTEGER draw in [0,sectors), via
 *      rng_next_range(rng, 0, sectors) directly. Unlike every other
 *      rng_next_range call in this codebase (which passes a fixed_t span --
 *      "a fixed_t span's raw int32_t value used directly as rng_next_
 *      range's exclusive bound", rules_discovery_roll_empty/planetgen_
 *      generate), this one's bound is the plain int `sectors` itself: flr()
 *      collapses the draw to an integer regardless, so this port draws that
 *      integer directly (uniform among 0..sectors-1) rather than drawing a
 *      fixed_t value and then flooring it -- an equivalent, simpler result.
 *   2. `rnd(0.5)` -- a uniform fixed_t draw in [0, FX_ONE/2) via
 *      rng_next_range(rng, 0, FX_ONE/2), the same fixed_t-span idiom cited
 *      above -- the within-sector jitter, asymmetric (biased toward each
 *      sector's own upper half, per this PRD's own explicit caveat), ported
 *      literally, not smoothed into a symmetric jitter.
 *   3. `rnd()` inside `du+rnd()` below -- drawn AFTER the angle's own two
 *      draws (source evaluates `a` fully before `p`), a uniform fixed_t
 *      draw in [0,FX_ONE) via rng_next_range(rng, 0, FX_ONE) -- the same
 *      idiom effects_flow_try_spawn's own spawn-chance draw already
 *      establishes.
 * The integer sector draw and the fractional jitter draw are summed as a
 * single fixed_t (sector_index*FX_ONE + jitter), then divided by `sectors`
 * (widened to a fixed_t) using this file's existing effects_fixed_div
 * helper -- `sectors` is a runtime parameter here (not a compile-time
 * constant), exactly the case effects_fixed_div's own doc comment reserves
 * it for, unlike the EFFECTS_PROBE_*_RECIP_* precomputed-reciprocal macros
 * elsewhere in this file, which are for dividing by a compile-time literal.
 * The age-drift term (age_ticks scaled by a truncated 0.001*FX_ONE-per-tick
 * constant, the same "traced literal -> truncated Q16.16 macro" convention
 * every other constant in this file follows) is then added on top. The
 * result can legitimately reach or exceed one full turn as age_ticks grows
 * large -- fixed_sin/fixed_cos already wrap any turns value via fixed_std_
 * sincos's own modulo, so no extra clamping is needed here.
 *
 * **Position** (`p=qm(du+rnd(),a)`): fixed_polar(base_radius+radius_jitter,
 * angle, &out->x, &out->y), where radius_jitter is RNG draw #3 above
 * (uniform [0,FX_ONE)) -- the spawn radius is base_radius plus a further
 * [0,1) jitter, exactly matching `du+rnd()`.
 *
 * **Velocity** (`v=p*py`, then `self.pk and v:pa() or v`): the position
 * vector scaled by velocity_scale gives the RADIAL velocity
 * (fixed_mul(out->x,velocity_scale), fixed_mul(out->y,velocity_scale)). If
 * tangential is nonzero, that radial vector is instead rotated 90 degrees
 * via `v:pa()` -- `km:pa()`, pico8/lua.lua:115-116, confirmed by direct
 * re-read (`function km:pa()return v(-self.y,self.x)end`, i.e.
 * (x,y)->(-y,x)) -- the SAME rotation helper and direction effects_flow_
 * try_spawn's own doc comment above already cites for its spawn-position
 * offset, applied here to a velocity instead of a unit vector (rotating
 * "radially outward" into "tangential"/orbital motion, per this PRD's own
 * "Position"/"Velocity" trace).
 *
 * **Lifetime** (`l=self.ki`): out->life is set to `lifetime` directly -- no
 * randomness and no further scaling; `lifetime` is already the caller-
 * supplied, pre-scaled fixed_t constant (e.g. 9*FX_ONE for the wormhole),
 * matching this file's established "PICO-8's own numbers are natively
 * Q16.16, the caller passes the already-scaled fixed_t" convention
 * (effects_kick_start's own EFFECTS_KICK_START_AMPLITUDE precedent).
 *
 * has_accel/ax/ay are always initialized to 0/0/0 -- see effects_spiral_
 * particle_t's own doc comment above for why.
 *
 * **Return value**: `sectors<=0` is a defensive no-spawn case (returns 0,
 * *out left untouched) -- source structurally never calls this with
 * `self.pn<=0` (the wormhole's own literal `pn=3`, and no other traced
 * caller of this generic engine's `:mm()` slot overrides `pn` to a
 * non-positive value), so this guard is never exercised by any traced
 * source path; it exists purely as ordinary defensive robustness for a
 * host-tested pure function against a division by zero in the angle
 * formula's own `/pn` step -- the same "defensive case source never
 * exercises, but still defined" posture effects_flow_try_spawn's own
 * from==to guard already establishes. Otherwise always returns 1 and
 * populates *out -- unlike effects_flow_try_spawn, `g_:mm()` has no
 * Bernoulli gate or other real failure mode of its own; the `int` return
 * here exists purely to match this file's established `effects_*_try_*`
 * naming/return convention (mirroring effects_flow_try_spawn's own
 * signature shape), not because this particular formula can meaningfully
 * fail under any sectors>0 input. `rng` is threaded by the caller, matching
 * every other RNG-injected function in this codebase.
 */
int effects_spiral_try_spawn_particle(fixed_t base_radius, int sectors, int tangential, fixed_t age_ticks, fixed_t velocity_scale, fixed_t lifetime, rng_t *rng, effects_spiral_particle_t *out);

/*
 * effects_spiral_step_particle -- ports `ei(self)`'s per-particle loop body,
 * the generic spiral particle engine's tick step (pico8/lua.lua:692-706,
 * inside the `for _,p in pairs(self.qy)do ... end` loop -- the surrounding
 * visibility guard, the `for` loop itself, and the trailing `ez(self,self.fc)`
 * respawn call are all the caller's job, not this function's; see
 * effects_spiral_particle_t's own doc comment for this file's established
 * split between struct/spawn/step):
 *
 *   local pv=p.v
 *   p.p+=pv
 *   p.v.x=qo*pv.x+ro*pv.y
 *   p.v.y=qg*pv.x+q_*pv.y
 *   if (p.a)p.v+=p.a
 *   p.l-=0.14
 *   if p.l<=1 then
 *   del(self.qy,p)else
 *   pset(cx+p.p.x,cy+p.p.y,cs[flr(p.l)])
 *   end
 *
 * **Position advance** (`local pv=p.v` then `p.p+=pv`): position advances by
 * the OLD velocity, read into `pv` BEFORE the velocity update below
 * overwrites it -- source's own exact evaluation order, preserved here by
 * capturing `p->vx`/`p->vy` into locals first and using those locals (not
 * `p->vx`/`p->vy` re-read after the update) for the position add.
 *
 * **Velocity update** (`p.v.x=qo*pv.x+ro*pv.y`, `p.v.y=qg*pv.x+q_*pv.y`): a
 * GENERAL 2x2 linear transform of the OLD velocity -- NOT a hardcoded
 * rotation formula, since the four per-type constants (`qo,ro,qg,q_`) aren't
 * required to form a literal rotation matrix (see docs/prd/triage/
 * psx-juice-effects.md, "### 1. Spiral particle engine," item 103's own
 * "general 2x2 linear map, not literally a rotation matrix" note). Parameters
 * map directly: `m00`=`qo`, `m01`=`ro` (new vx = m00*old_vx + m01*old_vy),
 * `m10`=`qg`, `m11`=`q_` (new vy = m10*old_vx + m11*old_vy). The wormhole's
 * own reference constants are `qo=1.02,ro=-0.0525,qg=0.0525,q_=1.02` (a
 * slight rotation-plus-outward-scale each tick), but this function makes no
 * assumption about the caller's specific matrix -- correct for any type's own
 * constants, matching source's own fully general formula.
 *
 * **Optional acceleration** (`if(p.a)p.v+=p.a`): only applies when
 * `p->has_accel` is set, added to velocity AFTER the matrix transform above
 * (matching source's own statement order: the `if(p.a)` line runs after the
 * `p.v.x=.../p.v.y=...` assignments). `effects_spiral_try_spawn_particle`
 * (chunk 8) always initializes `has_accel` to 0 for the wormhole's own
 * reference variant, but this function implements the check as a genuine
 * runtime conditional (not skipped or always-applied), matching
 * `effects_spiral_particle_t`'s own general-purpose struct design -- a real,
 * if here always-false-in-practice, `if(p.a)` branch, per this codebase's
 * "implement the struct's own general shape faithfully" convention.
 *
 * **Aging and death** (`p.l-=0.14`, then `if p.l<=1 then del(...) else ...`):
 * lifetime decays by a flat `0.14*FX_ONE` every call, independent of any
 * per-type constant (unlike the velocity matrix above). The death threshold
 * is `<=1*FX_ONE`, DELIBERATELY NOT `<=0` -- unlike this file's existing
 * `effects_flow_step_particle`, whose own `t.l<=0` threshold this function's
 * threshold does NOT match (a particle that would draw with `cs[0]`, out of
 * source's 1-indexed `cs` table range, is killed one tick early instead; see
 * docs/prd/triage/psx-juice-effects.md item 105). Returns 1 if the particle
 * is still alive (life > 1*FX_ONE) after the decrement, or 0 if it just died
 * (life <= 1*FX_ONE) -- matching effects_flow_step_particle's own "1=alive,
 * 0=dead" return convention, just against this function's own different
 * threshold.
 *
 * The drawing call (`pset(cx+p.p.x,cy+p.p.y,cs[flr(p.l)])`) is NOT part of
 * this function -- pure rendering, out of scope for this pure-function step,
 * matching every other unwired module in this PRD. The caller is responsible
 * for removing a dead particle from whatever list it lives in, exactly
 * matching effects_flow_step_particle's own established convention.
 *
 * Mutates *p in place, matching effects_flow_step_particle's/effects_spiral_
 * try_spawn_particle's own established in/out-parameter convention.
 */
int effects_spiral_step_particle(effects_spiral_particle_t *p, fixed_t m00, fixed_t m01, fixed_t m10, fixed_t m11);

/*
 * effects_popup_t -- the money-spent/income floating popup's motion/fade
 * state, ported from `iv` (pico8/lua.lua:941-951, `iv:ph`'s per-tick update
 * block; see also pico8/lua.lua:339-345 (`ne()`, the value-formatting
 * helper this popup's out-of-scope text draw uses), 515-516 (`jo:hk()`, the
 * income-popup trigger), 1217-1225 (`hy:lu()`, the spend/insufficient-funds
 * trigger)):
 *
 *   iv=ke:jr([[
 *   fa=9,
 *   l=0,v=v(0,-0.34),
 *   ]])function iv:pq()self.ra=self.ra or
 *   je.ra+mo.ra-v(0,5)end
 *   function iv:ph()
 *   self.ra+=self.v
 *   self.l+=0.1
 *   self.p_=self.l>=5
 *   end
 *   function iv:kj(p)dv(self.l)io(self[1],p.x,p.y,self[2],0.5)dv()end
 *
 * `self.ra` (position) and `self.v`/`self.l` (velocity/age-timer -- this
 * struct's own `vy`/`age`) are the whole of this effect's traced state --
 * carried here as its own small struct, matching effects_kick_t's/effects_
 * spiral_particle_t's own established "one struct per traced piece of
 * per-instance state" shape. `self.v` (`v=(0,-0.34)`) is a FIXED constant
 * in source, never reassigned after the prototype default -- this struct
 * still stores it as a mutable `vy` field rather than applying the constant
 * directly inside effects_popup_step, matching this step's Data Model's
 * exact struct shape (docs/prd/triage/psx-juice-effects.md's own
 * `effects_popup_t` entry) rather than optimizing the field away.
 *
 * `iv:pq()`'s own anchor-position setup (`self.ra=self.ra or je.ra+mo.ra-v
 * (0,5)`) and `iv:kj`'s own text draw (`io(self[1],p.x,p.y,self[2],0.5)`,
 * wrapped in `dv(self.l)...dv()`'s palette-swap bracket,
 * docs/prd/psx-sprites-audio-extraction.md's own already-traced mechanism)
 * are both out of scope for this chunk -- `pq()` is a one-time spawn-anchor
 * default this chunk's own effects_popup_start (below) replaces with an
 * explicit caller-supplied (x,y) instead (that default computes the anchor
 * from two other live entities' own positions, `je`/`mo`, out of scope
 * here, matching effects_flow_try_spawn's own caller-supplied-not-a-live-
 * global precedent), and `kj()`'s text rendering needs `io()` (PICO-8's
 * print), which no font/glyph rendering anywhere in this port implements
 * (re-confirmed by every prior PRD's identical finding) -- see this PRD's
 * "### 6. Money-spent/income floating popup" section for the full trace.
 */
typedef struct {
    fixed_t x, y, vy, age;
} effects_popup_t;

/*
 * effects_popup_start -- initializes a fresh popup at the caller-supplied
 * spawn position (x,y), standing in for `iv:pq()`'s own one-time anchor
 * default -- see effects_popup_t's doc comment above for why that default
 * itself is out of scope here; this function simply takes the already-
 * resolved (x,y) directly, matching effects_kick_start's/effects_spiral_
 * try_spawn_particle's own "caller resolves whatever live state feeds this
 * effect, this function just applies it" convention. Sets *p's position to
 * (x,y), `vy` to the fixed constant `v=(0,-0.34)` (EFFECTS_POPUP_VELOCITY_Y,
 * effects.c), and `age` to 0 -- matching `l=0` in `iv`'s own prototype
 * default.
 */
void effects_popup_start(effects_popup_t *p, fixed_t x, fixed_t y);

/*
 * effects_popup_step -- ports `iv:ph()`'s own per-tick update body (see
 * effects_popup_t's doc comment above for the full source excerpt):
 *
 *   self.ra+=self.v
 *   self.l+=0.1
 *   self.p_=self.l>=5
 *
 * Position advance (`self.ra+=self.v`): straight addition of the stored
 * (fixed, never-changing) velocity `p->vy` to `p->y` only -- `v=(0,-0.34)`
 * has no x component, matching source's own "no horizontal motion" literal
 * exactly (p->x is left untouched by this function).
 *
 * Age/fade timer (`self.l+=0.1`): advances `p->age` by exactly
 * EFFECTS_POPUP_AGE_PER_STEP (0.1*FX_ONE, truncated -- effects.c) per call
 * -- PICO-8's own numbers are natively Q16.16, matching this file's
 * established "a plain per-tick literal is exactly that many FX_ONE units,
 * not a separately-rescaled integer" convention (effects_flow_step_
 * particle's/effects_spiral_step_particle's own identical precedent).
 *
 * Death (`self.p_=self.l>=5`): returns 1 while still alive (age <
 * 5*FX_ONE after the increment), 0 once dead (age >= 5*FX_ONE) -- matching
 * effects_flow_step_particle's/effects_spiral_step_particle's own
 * established "1=alive, 0=dead" return convention, inverted from source's
 * own "pending delete" boolean sense, exactly like those two functions
 * invert their own source `del(...)`/`p_` tests. Because EFFECTS_POPUP_
 * AGE_PER_STEP is a TRUNCATED representation of the real 0.1 (0.1*FX_ONE
 * ==6553.6, truncated to 6553 -- see effects.c), this accumulates slightly
 * slower than the real 0.1/tick rate, so death does NOT land on the
 * naively-expected 50th call (50*0.1==5.0 in real terms) -- see
 * effects_test.c's own hand-derivation for the exact call this actually
 * crosses on.
 *
 * Mutates *p in place, matching this file's established in/out-parameter
 * convention (effects_flow_step_particle, effects_kick_step, effects_
 * spiral_step_particle).
 */
int effects_popup_step(effects_popup_t *p);

/*
 * effects_popup_fade_index -- ports `iv:kj`'s own use of `self.l` as a
 * direct palette-swap-table index (`dv(self.l)...io(...)...dv()`, see
 * effects_popup_t's doc comment above for the full source excerpt): `self.l`
 * (0 at spawn, rising toward 5 at death, in steps of 0.1/tick) is used AS
 * the index directly, not smoothly interpolated -- a discrete indexed
 * palette remap, NOT an alpha fade (PICO-8 has no alpha), matching this
 * PRD's own explicit "discrete indexed remap" framing.
 *
 * Returns `(int)(p->age >> 16)` -- the integer part of `age` in Q16.16, the
 * same truncate-to-integer idiom rules_planets_job_cost's own `(int)
 * (cost >> 16)` already establishes for this codebase (rules_planets.c) --
 * clamped to `[0,4]` (5 distinct states, matching this PRD's own Data Model
 * spec exactly). The upper clamp is NOT purely defensive dead code: age can
 * legitimately reach or exceed `5*FX_ONE` once effects_popup_step has
 * already reported this popup dead, if the caller still calls this once
 * more (e.g. one last draw before removal) -- `age>>16` would then be
 * exactly 5 or more, outside source's own 5-entry `cs` swap-table range
 * `[0,5)`, so the clamp is a real, exercised case -- see effects_test.c's
 * own test for the exact age values this clamp catches. `age` is always
 * >=0 in this struct's own normal lifecycle (effects_popup_start zeroes it,
 * effects_popup_step only ever adds a positive amount), so the lower clamp
 * is ordinary defensive robustness for a host-tested pure function, the
 * same posture effects_flow_try_spawn's own from==to guard already
 * establishes, not a traced source path.
 */
int effects_popup_fade_index(const effects_popup_t *p);

/*
 * effects_growbar_t -- the end-game reveal's growing frame-border bars'
 * counter state, ported from `pj:kj`'s own `self.gz` field (pico8/lua.lua:
 * 853-856):
 *
 *   function pj:kj()bl(la(pj.i_,pj.ou))if self.gz then
 *   self.gz+=0.2
 *   local h=min(self.gz,3.2)^2
 *   rk(0,h,0)rk(127-h,127)end
 *   end
 *
 * Only the `self.gz` branch matters for this chunk -- `bl(la(pj.i_,pj.ou))`
 * at the top is the milestone-popup screen shake (`mh`/`bl()`), already out
 * of scope for this whole PRD (no camera system -- see
 * docs/prd/triage/psx-juice-effects.md's Out of Scope), and NOT part of
 * what this struct/function pair ports. `self.gz` is only non-nil during
 * the end-game reveal (source's own `if self.gz then...end` guard around
 * the whole per-tick update) -- this port doesn't model that live/absent
 * state, so this chunk just ports the pure `gz`/height formula assuming the
 * caller already knows it's active; see effects.h's doc comment on
 * effects_growbar_step below for the full trace.
 *
 * `self.gz` (a single `pj`-instance field in source) is the whole of this
 * effect's traced state -- carried here as its own small struct, matching
 * effects_kick_t's/effects_popup_t's own established "one struct per traced
 * piece of per-instance state" shape, rather than a bare fixed_t the caller
 * has to remember the meaning of.
 */
typedef struct {
    fixed_t gz;
} effects_growbar_t;

/*
 * effects_growbar_step -- ports `pj:kj`'s own per-tick `self.gz`
 * advance-then-height-formula (see effects_growbar_t's doc comment above
 * for the full source excerpt): `self.gz+=0.2` then
 * `h=min(self.gz,3.2)^2`.
 *
 * Advance (`self.gz+=0.2`): advances `g->gz` by exactly
 * EFFECTS_GROWBAR_STEP (0.2*FX_ONE, truncated -- effects.c) per call,
 * UNCAPPED -- source's own `self.gz+=0.2` has no ceiling on `gz` itself,
 * only on its USE in the height formula below (`min(self.gz,3.2)`, not
 * `self.gz=min(self.gz+0.2,3.2)`), so `g->gz` keeps growing without bound
 * for as long as the caller keeps calling this function, matching this
 * file's established "PICO-8's own numbers are natively Q16.16, so a plain
 * per-tick literal is exactly that many FX_ONE units" convention
 * (effects_popup_step's own `self.l+=0.1` precedent).
 *
 * Height (`h=min(self.gz,3.2)^2`): the (just-advanced) `g->gz` is clamped
 * to at most EFFECTS_GROWBAR_HEIGHT_CAP (3.2*FX_ONE, truncated -- effects.c)
 * FIRST, then squared via `fixed_mul` -- matching source's own exact
 * `min(gz,3.2)^2` order (clamp then square, not square then clamp). No
 * sqrt is needed anywhere in this formula. Because EFFECTS_GROWBAR_STEP is
 * a TRUNCATED representation of the real 0.2 (0.2*FX_ONE==13107.2,
 * truncated to 13107 -- see effects.c), `g->gz` accumulates slightly slower
 * than the real 0.2/tick rate, so the cap does NOT first engage on the
 * naively-expected 16th call (16*0.2==3.2 in real terms) -- see
 * effects_test.c's own hand-derivation for the exact call this actually
 * crosses on.
 *
 * `rk(0,h,0)`/`rk(127-h,127)` (drawing the two growing frame bars, top and
 * bottom) are NOT part of this function -- pure rendering, out of scope for
 * this pure-function step, matching every other unwired module in this
 * file.
 *
 * Returns the computed height directly (unlike effects_flow_step_particle's/
 * effects_popup_step's own alive/dead int return -- this formula has no
 * lifecycle to report, it is a pure per-call computation, matching
 * effects_probe_pulse_radius's/effects_probe_sweep_radius's own "return the
 * computed value directly" shape instead). Mutates *g in place (advancing
 * gz), matching this file's established in/out-parameter convention
 * (effects_flow_step_particle, effects_kick_step, effects_popup_step).
 */
fixed_t effects_growbar_step(effects_growbar_t *g);

/*
 * effects_settle_t -- the menu row slide-in-and-settle's position/target
 * state, ported from `js`'s own `self.ra`/`self.qf` fields, `js:ph()`
 * (pico8/lua.lua:263-266):
 *
 *   function js:ph()if self.qf then
 *   local d=self.qf-self.ra
 *   self.ra+=(#d<=1 and d or d*0.4)
 *   end
 *   end
 *
 * `self.ra` (current position) and `self.qf` (target position) are the
 * whole of this effect's traced state -- carried here as its own small
 * struct, matching effects_kick_t's/effects_popup_t's/effects_growbar_t's
 * own established "one struct per traced piece of per-instance state"
 * shape. Unlike those, this struct has TWO position fields (current and
 * target), not one scalar/one position -- matching `js`'s own two distinct
 * fields exactly, rather than collapsing them into one.
 *
 * This chunk is ONLY the pure per-call math primitive (`js:ph()` itself) --
 * NOT `pz(p,bs)`'s own menu-row construction/layout (pico8/lua.lua:280-288,
 * which sets each row's initial `ra`/`qf` from live entity positions this
 * port doesn't have yet) and NOT any live wiring into a real tech-menu
 * tile -- see docs/prd/triage/psx-juice-effects.md, "### 7. Menu row
 * slide-in-and-settle" for the full trace, and Implementation Chunks item
 * 12 (this chunk, the pure primitive) vs item 13 (a separate, not-yet-
 * implemented future chunk that wires this into `rules_tech.c`'s/
 * `game_draw.c`'s tech-menu tiles) for the exact split.
 *
 * `self.qf`'s own `if self.qf then` nil-guard (a menu row not yet given a
 * target never eases at all) is not modeled here -- this struct always has
 * both `ra_x/ra_y` and `qf_x/qf_y` set to *something* (the caller decides
 * what "no target yet" means, e.g. by setting qf equal to ra, which this
 * function trivially settles on the very first call since d is then
 * exactly (0,0)), matching effects_flow_try_spawn's own "caller resolves
 * whatever live state feeds this effect" convention rather than adding an
 * extra has-target flag this PRD's own Data Model doesn't call for.
 */
typedef struct {
    fixed_t ra_x, ra_y, qf_x, qf_y;
} effects_settle_t;

/*
 * effects_settle_step -- ports `js:ph()`'s own per-tick update body (see
 * effects_settle_t's doc comment above for the full source excerpt):
 * `local d=self.qf-self.ra` then `self.ra+=(#d<=1 and d or d*0.4)`.
 *
 * `d = qf - ra` (vector from the current position toward the target):
 * `dx = fixed_sub(s->qf_x, s->ra_x)`, `dy = fixed_sub(s->qf_y, s->ra_y)`.
 *
 * `#d` -- PICO-8's vector-length operator -- is, per this codebase's own
 * already-established `km:__len` precedent (the same squared-distance-
 * threshold-compare idiom `liaison.c`'s `liaison_point_blocks_segment`/
 * `liaison_check` and `rules_discovery.c`'s `rules_discovery_probe_in_range`
 * already rely on, cited in docs/prd/triage/psx-juice-effects.md's
 * Implementation Decisions), actually SQUARED magnitude, not linear
 * distance. So `#d<=1` compares `dx*dx+dy*dy` (via `fixed_mul` -- both dx
 * and dy are expected to be modest menu-row screen-space deltas, well
 * within the range `fixed_mul`'s own truncate-to-`fixed_t` result can hold,
 * unlike `effects_length`'s own dedicated overflow-safe int64 path reserved
 * for potentially-large trade-flow segments) against `1*FX_ONE`, i.e. `1`
 * full PICO-8 number-unit, SQUARED -- NOT `sqrt(dx*dx+dy*dy)<=1`; no sqrt
 * appears anywhere in this function, deliberately. Because comparing
 * squared-distance to `1` happens to equal comparing linear distance to
 * `sqrt(1)=1` (1 is sqrt's own fixed point), this particular threshold
 * VALUE gives the same yes/no answer either way -- effects_test.c's own
 * doc comment on this point explains why no single (dx,dy) case can
 * behaviorally distinguish "squared-compared-to-1" from
 * "sqrt-then-compared-to-1" at this exact threshold, and instead
 * constructs a different, genuinely distinguishing case (a combined-vs-
 * per-axis-threshold mixup) that a `sqrt`/formula bug could plausibly take
 * the shape of. The comparison ITSELF must still be done in squared units,
 * not linear -- that's what makes no `sqrt` call necessary at all, which is
 * the whole point of this primitive.
 *
 * Within that squared-distance-1 threshold (`mag_sq <= FX_ONE`, inclusive,
 * matching source's own `<=`): `self.ra+=d` -- snaps `*s`'s position
 * directly to the target (`s->ra_x = s->qf_x`, `s->ra_y = s->qf_y`,
 * computed as the exact target values directly, not as `ra+d`'s own
 * fixed-point-arithmetic result, since those are mathematically identical
 * and the former is simpler and exact by construction rather than by
 * cancellation). Returns 0 -- "no longer moving," matching this codebase's
 * established 1=alive/0=dead-style return convention
 * (effects_flow_step_particle, effects_popup_step,
 * effects_spiral_step_particle) applied here to "still easing toward the
 * target" instead of "still alive."
 *
 * Otherwise: `self.ra+=d*0.4` -- eases exactly 40% of the remaining
 * distance (`s->ra_x += fixed_mul(dx, EFFECTS_SETTLE_EASE_FACTOR)`, same
 * for y, effects.c). Returns 1 -- "still moving."
 *
 * A textbook exponential-decay lerp-toward-target with a snap threshold: no
 * overshoot, no oscillation (each step's remaining distance strictly
 * shrinks -- either to exactly 0 via the snap, or by a genuine fraction of
 * itself via the ease branch, since `0.4` is used as a `fixed_mul`'d
 * fraction of `d`, never a full step past the target), and the position
 * always ends EXACTLY at the target once it settles.
 *
 * Mutates *s in place, matching this file's established in/out-parameter
 * convention (effects_flow_step_particle, effects_kick_step,
 * effects_popup_step, effects_spiral_step_particle, effects_growbar_step).
 */
int effects_settle_step(effects_settle_t *s);

#endif
