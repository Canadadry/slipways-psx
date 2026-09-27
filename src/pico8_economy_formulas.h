#ifndef PICO8_ECONOMY_FORMULAS_H
#define PICO8_ECONOMY_FORMULAS_H

#include "pico8_struct.h"
#include "pico8_fixed.h"
#include "pico8_list.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Throughout this module an absent or wrong-kind field reads as its
 * zero/false default rather than crashing, and a NULL instance is a safe
 * no-op. */

/* Returns a fresh table the caller owns and must release with
 * pico8_ob_table_free(). Its keys are `r`'s VALUES, not `r`'s keys; a
 * value may be FIXED or STRING, and both are usable as a key. */
pico8_ob_table_t *pico8_cm(pico8_fixed_t jm);

/* Returns a fresh table the caller owns; `hy:lj()` stores it in `fm.po`
 * and frees it on the next recompute. */
pico8_ob_table_t *pico8_bx(void);

/* Returns a fresh table the caller owns; `hy:lj()` stores it in `fm.fu`
 * and frees it on the next recompute. */
pico8_ob_table_t *pico8_ck(void);

/* `#(to.p-pb.p)` is the SQUARED distance, not the Euclidean one. */
pico8_fixed_t pico8_dj(const pico8_struct_header_t *pb, const pico8_struct_header_t *to);

/* An absent `cg.jc` counts as zero slipways. */
pico8_fixed_t pico8_bk(void);

#endif
