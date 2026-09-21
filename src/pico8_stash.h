#ifndef PICO8_STASH_H
#define PICO8_STASH_H

#include "pico8_mem.h"
#include "pico8_ob.h"

/* Phase 3, Chunk 1, second sub-lot: unstash() (pico8/lua.lua:79-84) and the
 * two real values it's used to build, r and iz (line 1377's `r,iz=
 * ob(unstash(8192)),ob(unstash(8259))`). Kept in its own module: this is
 * data-decoding glue over raw PICO-8 memory (pico8_mem.h), a distinct
 * concern from pico8_collect.c's plain-value/list glue and
 * pico8_entity_geom.c's live-entity-field glue.
 *
 * unstash()'s real source, read directly (not guessed at, per this
 * sub-lot's own explicit instruction -- the prior sub-lot's own progress
 * notes had guessed "RLE-per-character" before actually reading this far,
 * which this comment corrects):
 *   gt="abcdefghijklmnopqrstuvwxyz0123456789 ().,=-+_/\"'?%\n"
 *   function unstash(pd)local s=""repeat
 *   local i=peek(pd)s=s..sub(gt,i,i)
 *   pd+=1
 *   until i==0
 *   return s
 *   end
 * This is NOT run-length encoding: it's a plain byte-to-character lookup.
 * Each byte at memory address `pd` (starting at the caller's own `pd`,
 * advancing by 1 every iteration) is used as a 1-indexed lookup into `gt`
 * (a 51-character alphabet -- confirmed by literally counting `gt`'s own
 * source characters: 26 lowercase letters + 10 digits + 15 punctuation/
 * space/newline characters, NOT the 44 a prior sub-lot's own restante note
 * had guessed before reading this far). A byte value of 0 stops the loop
 * (PICO-8's own repeat/until runs the body at least once, so a leading 0
 * byte still executes one iteration -- `sub(gt,0,0)`, which pico8_sub's
 * own documented clamping rule makes an empty string, contributing
 * nothing, exactly mirrored by pico8_sub(PICO8_GT,0,0) here). The decoded
 * addresses (8192=0x2000, 8259=0x2043, and Chunk 11's own not-yet-ported
 * 8383/8494/9807/10457) all fall inside PICO8_ADDR_MAP's own 8192-byte
 * region (pico8_mem.h) -- this cart reuses unused map memory to embed
 * extra compile-time string constants, a real, documented PICO-8 trick,
 * not a bug in this reading. */

/* Returns a newly heap-allocated, NUL-terminated string the caller owns
 * and must free() -- matches pico8_sub/pico8_value_tostr's own ownership
 * convention (this is, after all, a decoded Lua string value). */
char *pico8_unstash(const pico8_mem_t *mem, int pd);

/* r,iz=ob(unstash(8192)),ob(unstash(8259)) (pico8/lua.lua:1377): builds
 * both from REAL cart memory (mem must already have pico8/map.json loaded,
 * e.g. via pico8_load_map_json -- main.c's own demo already does this
 * before any drawing/input assertions run). Safe to call more than once
 * (frees whatever a previous call built first), mirroring
 * pico8_entity_init()'s own established re-init convention. */
void pico8_stash_init(pico8_mem_t *mem);

/* Releases r/iz's own backing storage. Not part of the Lua original (no
 * manual teardown under GC), same reasoning as every other _shutdown in
 * this port. Safe to call more than once, and safe to call before the
 * first pico8_stash_init(). */
void pico8_stash_shutdown(void);

/* r/iz themselves, owned by this module (built/freed by
 * pico8_stash_init()/_shutdown()); callers must not free them. Consumed
 * directly within this sub-lot by pico8_collect.c's own pico8_ho_apply
 * (`r[ri[i]]`) -- every other real r/iz consumer (hy's economy formulas,
 * etc.) is a later chunk, out of this sub-lot's own scope. */
const pico8_ob_table_t *pico8_r(void);
const pico8_ob_table_t *pico8_iz(void);

#endif
