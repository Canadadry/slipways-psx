#ifndef PICO8_AUDIO_H
#define PICO8_AUDIO_H

#include "pico8_value.h"

/* Phase 3, Chunk 1, second sub-lot: the two audio no-op stubs
 * docs/plan.md's own "son hors perimetre" guardrail promises but Phase 1
 * never actually defined (its own demo never called them) -- see this
 * PRD's Implementation Decisions, "sfx/music stubs (Chunk 1)". This is the
 * FIRST AND ONLY place either is ever defined; per the plan's guardrail,
 * neither ever gets real behavior in this project. rn() (the first real
 * caller of sfx()) and its own `ih` forward-referenced global live here
 * too: rn is entirely about deciding whether to fire the (inert) sfx
 * stub, a natural fit for this module's own narrow concern. */

/* sfx(n,[channel],...) / music(n) -- PICO-8 builtins this port never
 * implements for real. Signatures cover exactly what rn() (below) and
 * Chunk 21's difficulty-screen sfx/music-toggle callbacks (not yet
 * ported) actually call: sfx(n,channel) and music(n), no further optional
 * PICO-8 parameters (offset/length for sfx, fade time for music), since
 * nothing in this cart's own scope ever passes them. */
void pico8_sfx(int n, int channel);
void pico8_music(int n);

/* `ih` (pico8/lua.lua:331's `if (no and not ih)sfx(no,3)end`, rn's own
 * only reference to it): a global this sub-lot's own function references
 * but that isn't actually ASSIGNED anywhere until Chunk 21 (a difficulty-
 * screen mute toggle, per this sub-lot's own scoping read of the file) --
 * "forward reference, ported not deferred", the same treatment already
 * established for bg:ldown's mz/g_ and gd's mo. Modeled as a module-level
 * flag with an accessor pair, mirroring pico8_entity.h's own pico8_mo()
 * placeholder-singleton convention, rather than a raw extern: defaults to
 * 0 (falsy, matching Lua's own "never assigned" == nil == falsy), and
 * whichever future chunk actually assigns the real `ih` does so through
 * pico8_set_ih() rather than a direct global write. */
int pico8_ih(void);
void pico8_set_ih(int value);

/* function rn(no)
 * if (no and not ih)sfx(no,3)
 * end
 * `no` may be a genuine PICO8_VALUE_FIXED sfx id (`rn(57)`, `rn(p.eo or
 * 62)`) or PICO8_VALUE_BOOL false (`rn(self.my and 60)` when self.my
 * itself is falsy) -- a plain `int` parameter can't represent both, so
 * this takes the shared pico8_value_t instead, matching Lua's own dynamic
 * typing here. Only a PICO8_VALUE_BOOL false is treated as falsy
 * ("truthy" mirrors Lua's own "only nil/false are falsy" rule); every
 * other kind (in practice, always PICO8_VALUE_FIXED at real call sites)
 * is truthy and, when `ih` is also falsy, triggers the (inert) sfx(no,3)
 * stub with `no` coerced to a plain int id. */
void pico8_rn(pico8_value_t no);

#endif
