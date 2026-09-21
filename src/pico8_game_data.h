#ifndef PICO8_GAME_DATA_H
#define PICO8_GAME_DATA_H

#include "pico8_fixed.h"
#include "pico8_ob.h"

/* Phase 3, Chunk 1, second sub-lot's own last piece: the pure data-literal
 * globals this sub-lot's own task list required getting off the
 * "restante" list (docs/portage-progress.md) that no function ported so
 * far -- in this sub-lot or the prior one -- actually consumes yet: es,
 * ct, b_, co (Chunk 12's economy formulas, da/db/ky/bf), bn/bq (Chunk 12's
 * dj/bk), fb (Chunk 21's eg()). Grouped into one small module rather than
 * scattered into each literal's own eventual consumer's not-yet-existing
 * module, mirroring pico8_entity.c's own established "data ported ahead
 * of its consumer, accessor-based" precedent (bm/ci) for exactly this
 * situation. Each accessor is picked up and actually used by whichever
 * future chunk ports that literal's own real consumer function -- none of
 * them are exercised by anything in this sub-lot's own scope. */

/* Builds/rebuilds every global this module owns. Safe to call more than
 * once (frees a previous build first), mirroring pico8_entity_init()'s/
 * pico8_stash_init()'s own established re-init convention. Not part of
 * the Lua original (these run unconditionally at cart-load time in the
 * source, no explicit "init" call) -- same reasoning as every other
 * *_init in this port (no GC-backed implicit construction in C). */
void pico8_game_data_init(void);
void pico8_game_data_shutdown(void);

/* es=ob([[1,0,1,2,]]) (pico8/lua.lua:1311) -- da()'s own lookup table. */
const pico8_ob_table_t *pico8_es(void);

/* ct=
 * ob([[-0.333,0,0.25,0.5,]]) (pico8/lua.lua:1323-1324) -- db()'s own
 * per-trade-partner-tier bonus table. */
const pico8_ob_table_t *pico8_ct(void);

/* b_=ob([[0,100,200,400,]]) (pico8/lua.lua:1338) -- ky()'s own score
 * table. Named pico8_b_ (trailing underscore, matching the Lua global's
 * own trailing-underscore minified name -- a legal, distinct C
 * identifier). */
const pico8_ob_table_t *pico8_b_(void);

/* co=ob([[-5,0,1,1,]]) (pico8/lua.lua:1364) -- bf()'s own happiness-
 * modifier table. */
const pico8_ob_table_t *pico8_co(void);

/* fb=ob([[
 * 2,2,5,5,7,7,10,10,14,14,14,19,19,19,
 * ]]) (pico8/lua.lua:1171-1172) -- eg()'s own palette-address-offset
 * table. */
const pico8_ob_table_t *pico8_fb(void);

/* bn,bq=
 * 6,0.0977 (pico8/lua.lua:1257-1258) -- dj()'s own tuning constants,
 * plain Lua number literals (not ob() spec-string fields, so parsed by
 * PICO-8's own ordinary Lua-number-literal reader rather than this port's
 * ob()-specific decimal parser, pico8_ob.c's own pico8_ob_parse_number --
 * that parser is private to pico8_ob.c and scoped to ob() spec-string
 * tokens specifically). `bn` (a whole integer) is exact via
 * pico8_fixed_from_int; `bq` (0.0977) uses pico8_fixed_from_double despite
 * that constructor's own "test/debugging convenience, PICO-8 itself never
 * goes through this" header comment -- the least-bad available option for
 * converting a genuine bare decimal literal outside any ob() spec string,
 * given the standard scaled-decimal-to-fixed rounding either path would
 * apply is not expected to differ for this value. */
pico8_fixed_t pico8_bn(void);
pico8_fixed_t pico8_bq(void);

#endif
