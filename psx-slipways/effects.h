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

#endif
