#include "game/bindings.h"

#include "p8/input.h"

#include <assert.h>
#include <stddef.h>

int bindings_read_button_number(const char *text, const char **end) {
    assert(text != NULL && "bindings_read_button_number needs a text");
    assert(end != NULL && "bindings_read_button_number needs somewhere to say where the digits stop");
    *end = text;
    if (*text < '1' || *text > '9') {
        return 0;
    }
    const char *cursor = text;
    int number = 0;
    while (*cursor >= '0' && *cursor <= '9') {
        if (number <= P8_PAD_BUTTON_COUNT) {
            number = number * 10 + (*cursor - '0');
        }
        cursor++;
    }
    if (number > P8_PAD_BUTTON_COUNT) {
        return 0;
    }
    *end = cursor;
    return number;
}
