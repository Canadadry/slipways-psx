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

#endif
