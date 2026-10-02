#ifndef GAME_SCORE_PANEL_DISPLAY_H
#define GAME_SCORE_PANEL_DISPLAY_H

#include "p8/fixed.h"
#include "game/score_panel.h"

/* `gz` is tested for PRESENCE, not for a non-zero value: the 0 written into
 * it is truthy in Lua, and it is what makes `pj:kj` draw the reveal curtain
 * from that frame on. */
void score_panel_update(score_panel_t *self);

/* `iw[3]` is removed, not merely left unwritten, when `pp` is falsy: a
 * previous year's message would otherwise stay on the box. `iw.lh` is
 * deliberately left stale in that same case, as in the Lua. */
void score_panel_new_year(p8_fixed_t yr);

/* The function `de.my` holds: `fq(de)` of `ix[3]` reads nothing else. */
modal_backdrop_on_click_fn_t score_panel_toggle_on_click(void);

#endif
