#ifndef GAME_SCORE_PANEL_DISPLAY_H
#define GAME_SCORE_PANEL_DISPLAY_H

#include "p8/fixed.h"
#include "game/score_panel.h"

/* has_curtain is tested, not the curtain's value: the 0 written into it
 * counts as set, and it is what makes score_panel_draw() draw the reveal
 * curtain from that frame on. */
void score_panel_update(score_panel_t *self);

/* The banner's third article is dropped, not merely left unwritten, when the
 * year has no countdown message: a previous year's message would otherwise
 * stay on the box. Its keep_open is deliberately left stale in that case. */
void score_panel_new_year(p8_fixed_t yr);

/* The panel toggle's callback: the backdrop that the galaxy menu's third
 * row opens reads nothing else. */
modal_backdrop_on_click_fn_t score_panel_toggle_on_click(void);

#endif
