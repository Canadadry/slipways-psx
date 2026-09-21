#ifndef PICO8_MOUSE_H
#define PICO8_MOUSE_H

#include "pico8_input.h"
#include "pico8_ob.h"

/* Phase 3, Chunk 1, second sub-lot: bu(qn,oa) (pico8/lua.lua:204-208) --
 * mouse-button edge-detection glue, turning stat(34)'s raw bitmask polling
 * into held/just-pressed/just-released state. Its own module: a distinct,
 * nameable concern (mouse-button edge detection for the future `nf`
 * class, Chunk 4) from every other Chunk 1 helper. */

/* function bu(qn,oa)local on,ia=
 * band(stat(34),oa)>0,qn.on
 * qn.on,qn.jn,qn.hm=
 * on,on and not ia,ia and not on
 * end
 * `qn` is a plain nested table (`l=o(),r=o()` in nf's real class spec,
 * Chunk 4 -- an empty pico8_ob_table_t at construction, populated here on
 * first use), not a pico8_instance_t: no class/prototype-chain fallback is
 * ever involved for qn's own on/jn/hm fields, so this reads/writes them
 * directly via pico8_ob_table_get_name/set_name rather than
 * pico8_instance_get_field. `band(stat(34),oa)`: both stat(34) and `oa`
 * (1 or 6 at real call sites, `bu(self.l,1)`/`bu(self.r,6)`) are plain
 * small integers here, not fractional fixed-point values, so PICO-8's
 * bitwise band() on their RAW representations is exactly equivalent to
 * ordinary C integer `&` on the plain `int` values pico8_stat already
 * returns (pico8_input.h) -- no pico8_fixed_band round-trip needed, unlike
 * pico8_ob.c's own number-literal masking, which operates in a genuinely
 * fractional-fixed-point context. */
void pico8_bu(const pico8_input_t *input, pico8_ob_table_t *qn, int oa);

#endif
