#ifndef PICO8_GALAXY_H
#define PICO8_GALAXY_H

#include "pico8_fixed.h"

/* Spawns `n` planets -- each one registers itself into hv/cv/cg through
 * jo's inherited constructor, so the returned pointers are dropped -- then
 * installs the class-level jo.oc/jo.ir generators. */
void pico8_bw(pico8_fixed_t n);

/* Releases only the pico8_jf_state_t this module pairs with jo.oc; jo.oc
 * and jo.ir themselves are pico8_jo_shutdown()'s to free. Safe to call
 * more than once, or before the first pico8_bw(). */
void pico8_galaxy_shutdown(void);

#endif
