#ifndef DEVTOOLS_CURSOR_LABEL_H
#define DEVTOOLS_CURSOR_LABEL_H

#include "game/ui.h"

/* What the cursor aims at: the first entity of the highest non-empty layer
 * of `hovered_by_layer`, the one ui_send_to_hovered offers a click to first.
 * "carte" for the background, for no entity, and for a NULL `ui` or a NULL
 * `hovered_by_layer`. A static string; reads `ui`, writes and allocates
 * nothing. Temporary (plan 12). */
const char *cursor_label_read(const ui_t *ui);

#endif
