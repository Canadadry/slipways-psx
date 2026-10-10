#ifndef GAME_BINDINGS_H
#define GAME_BINDINGS_H

/* Returns the button number 1..P8_PAD_BUTTON_COUNT that text starts with (digits only, no leading zero), 0 otherwise.
 * On success *end points just past the last digit; on failure *end is text. What follows the digits is the caller's
 * to check. end must not be NULL. */
int bindings_read_button_number(const char *text, const char **end);

#endif
