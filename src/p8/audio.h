#ifndef P8_AUDIO_H
#define P8_AUDIO_H

#include "p8/fixed.h"

/* TROU-10 */
void p8_sfx(int n, int channel);
void p8_music(int n);

/* Module-level flag with an accessor pair rather than a raw extern, so the
 * one writer cannot end up toggling a private copy. Defaults to 0, Lua's
 * never-assigned value. */
int p8_sfx_muted(void);
void p8_set_sfx_muted(int value);

void p8_play_sfx(p8_fixed_t no);

#endif
