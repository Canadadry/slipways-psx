#ifndef P8_ASSETS_H
#define P8_ASSETS_H

#include "p8/mem.h"

/* Load the exported sprite sheet -- {"width": <width>, "pixels": [...]} --
 * into mem->sprite_sheet. Returns 1 if at least one value was read, 0 if the
 * file is missing, unreadable, has no positive "width", or holds no values.
 * Values falling outside the 128x128 sheet are dropped, not reported. */
int p8_load_gfx_json(p8_memory_t *mem, const char *path);

#endif
