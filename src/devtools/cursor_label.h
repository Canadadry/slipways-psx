#ifndef DEVTOOLS_CURSOR_LABEL_H
#define DEVTOOLS_CURSOR_LABEL_H

#include "game/ui.h"

/* What the cursor aims at: the first entity of the highest non-empty layer
 * of `hovered_by_layer`, the one ui_send_to_hovered offers a click to first.
 * "carte" for the background, for no entity, and for a NULL `ui` or a NULL
 * `hovered_by_layer`. A static string; reads `ui`, writes and allocates
 * nothing. Temporary (plan 12). */
const char *cursor_label_read(const ui_t *ui);

/* cart_update60(), then appends to `path` the label of the ui whose hit test
 * dispatched this frame's click, and a newline: line n+1 is frame n. A click
 * that starts a game runs reset_world() inside ui_update, so main_ui() is
 * then a new ui whose hovered_by_layer is still NULL; the old one and its
 * layered list stay readable until the next frame's flush and zone reset.
 * `path` is opened (truncated) at the first call, kept open, and closed at
 * exit. 0, with a message on stderr, when it cannot be opened or written;
 * 1 otherwise. Temporary (plan 12). */
int cursor_label_update60(const char *path);

#endif
