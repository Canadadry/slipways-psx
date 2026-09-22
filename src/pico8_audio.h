#ifndef PICO8_AUDIO_H
#define PICO8_AUDIO_H

#include "pico8_value.h"

/* TROU-10 */
void pico8_sfx(int n, int channel);
void pico8_music(int n);

/* Module-level flag with an accessor pair rather than a raw extern, so the
 * one writer cannot end up toggling a private copy. Defaults to 0, Lua's
 * never-assigned value. */
int pico8_ih(void);
void pico8_set_ih(int value);

/* `no` is the shared value type, not an int, because call sites pass either
 * an sfx id or a plain false (`rn(self.my and 60)`). Only PICO8_VALUE_BOOL
 * false is falsy; anything else plays. */
void pico8_rn(pico8_value_t no);

#endif
