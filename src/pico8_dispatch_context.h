#ifndef PICO8_DISPATCH_CONTEXT_H
#define PICO8_DISPATCH_CONTEXT_H

#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_mem.h"

/* Phase 3, Chunk 4 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): a small, SHARED answer to a problem this phase's generic
 * dispatch mechanism (pico8_value_function_t, pico8_value.h -- the shape
 * `(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1)`, Phase
 * 3 Chunk 5's own arity extension) has had since Chunk 2 first hit it
 * (pico8_pi.c's own private `g_pi_gfx`/`g_pi_mem` statics, set by bl()
 * right before each dispatch): a dispatched method has no parameter slot
 * for gfx/mem/input, yet several of THIS chunk's own real methods need one
 * to do their real work once they're actually exercised --
 * nf:ph/qb:ph (stat()/btn(), need input), nf:pq (poke(), needs mem),
 * js:kj/js:ev (rect()/rectfill()/spr() via lq:ol/jb:ol, need gfx+mem), and
 * qb:md (camera(), needs gfx -- this is also the function that finally
 * gives real behavior to Phase 2's long-dead-but-compiled `dc("kj")`
 * branch, see pico8_scroll.h's own comment on pico8_qb_md).
 *
 * Rather than re-invent pico8_pi.c's own private-static-context pattern
 * independently a THIRD, FOURTH and FIFTH time in as many new modules this
 * one chunk adds, this is a single small shared "current frame's
 * gfx/mem/input" set of accessors: set once per real frame by whichever
 * future chunk's own top-level loop actually drives real dispatch
 * (dg()/_update60()/_draw(), Chunk 6/22 -- neither exists yet), read by
 * any dispatch adapter in this chunk that needs one. pico8_pi.c's own
 * existing g_pi_gfx/g_pi_mem statics are deliberately left exactly as they
 * are (out of this chunk's own scope to touch already-shipped Chunk 2
 * code) -- this is only for this chunk's OWN new dispatch adapters, to
 * avoid tripling a pattern that would otherwise recur a third time within
 * this same chunk alone.
 *
 * NOT wired to anything yet: no real caller exists before Chunk 6/22, so
 * every getter defaults to NULL, and every consumer in this chunk treats a
 * NULL context as a safe, documented no-op -- exactly the same "compiles,
 * not yet exercised" forward-reference treatment this whole PRD already
 * establishes for bg:ldown's mz/g_, gd's mo, etc. This is deliberately
 * narrow (get/set only, three fixed slots, no general "ambient global
 * state" facility) -- a genuinely evidenced, bounded need for THIS
 * chunk's own three new modules, not speculative generality. */

void pico8_dispatch_set_gfx(pico8_gfx_t *gfx);
pico8_gfx_t *pico8_dispatch_gfx(void);

void pico8_dispatch_set_mem(pico8_mem_t *mem);
pico8_mem_t *pico8_dispatch_mem(void);

void pico8_dispatch_set_input(const pico8_input_t *input);
const pico8_input_t *pico8_dispatch_input(void);

#endif
