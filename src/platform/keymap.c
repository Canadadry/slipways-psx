#include "platform/keymap.h"

#include "game/bindings.h"
#include "p8/input.h"
#include "platform/keymap_names.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void set_error(keymap_error_t *error, int line, const char *format, ...) {
    va_list arguments;
    error->line = line;
    va_start(arguments, format);
    vsnprintf(error->message, sizeof error->message, format, arguments);
    va_end(arguments);
}

/* Space drops a marker, the page keys and End set the replay speed, Escape
 * closes the raylib window. */
static const char *const reserved_names[] = { "SPACE", "PAGE_UP", "PAGE_DOWN", "END", "ESCAPE" };

typedef struct {
    const char *name;
    keymap_device_t device;
    int code;
} name_entry_t;

#define KEYMAP_KEYBOARD_ENTRY(name, code) { #name, KEYMAP_KEYBOARD, code },
#define KEYMAP_GAMEPAD_ENTRY(name, code) { #name, KEYMAP_GAMEPAD, code },

static const name_entry_t name_entries[] = {
    KEYMAP_KEYBOARD_NAMES(KEYMAP_KEYBOARD_ENTRY)
    KEYMAP_GAMEPAD_NAMES(KEYMAP_GAMEPAD_ENTRY)
};

#define NAME_ENTRY_COUNT ((int)(sizeof name_entries / sizeof name_entries[0]))

int keymap_find_binding(const char *name, keymap_binding_t *out) {
    for (int i = 0; i < NAME_ENTRY_COUNT; i++) {
        if (strcmp(name_entries[i].name, name) == 0) {
            out->device = name_entries[i].device;
            out->code = name_entries[i].code;
            return 1;
        }
    }
    return 0;
}

static const char *skip_blanks(const char *start, const char *end) {
    while (start < end && (*start == ' ' || *start == '\t')) {
        start++;
    }
    return start;
}

static const char *trim_end(const char *start, const char *end) {
    while (end > start && (end[-1] == ' ' || end[-1] == '\t')) {
        end--;
    }
    return end;
}

static void copy_token(char *dst, const char *start, const char *end) {
    size_t length = (size_t)(end - start);
    memcpy(dst, start, length);
    dst[length] = '\0';
}

static int name_reserved(const char *name) {
    for (size_t i = 0; i < sizeof reserved_names / sizeof reserved_names[0]; i++) {
        if (strcmp(reserved_names[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

static int binding_used(const keymap_t *keymap, keymap_binding_t binding) {
    for (int i = 0; i < P8_PAD_BUTTON_COUNT; i++) {
        for (int j = 0; j < keymap->buttons[i].count; j++) {
            const keymap_binding_t *used = &keymap->buttons[i].bindings[j];
            if (used->device == binding.device && used->code == binding.code) {
                return 1;
            }
        }
    }
    return 0;
}

static int parse_line(const char *start, const char *end, int line_number, keymap_t *out, keymap_error_t *error) {
    const char *equal = memchr(start, '=', (size_t)(end - start));
    char token[KEYMAP_MAX_LINE_LENGTH + 1];
    if (equal == NULL) {
        copy_token(token, start, end);
        set_error(error, line_number, "no '=' in \"%s\"", token);
        return 0;
    }
    const char *name_start = skip_blanks(start, equal);
    copy_token(token, name_start, trim_end(name_start, equal));
    const char *digits_end = NULL;
    int number = bindings_read_button_number(token, &digits_end);
    if (number == 0 || *digits_end != '\0') {
        set_error(error, line_number, "bad button \"%s\": expected a number from 1 to %d", token, P8_PAD_BUTTON_COUNT);
        return 0;
    }
    keymap_button_t *button = &out->buttons[number - 1];
    if (button->count > 0) {
        set_error(error, line_number, "button %d already given", number);
        return 0;
    }
    const char *item = equal + 1;
    for (;;) {
        const char *comma = memchr(item, ',', (size_t)(end - item));
        const char *item_end = comma != NULL ? comma : end;
        const char *item_start = skip_blanks(item, item_end);
        copy_token(token, item_start, trim_end(item_start, item_end));
        if (token[0] == '\0') {
            set_error(error, line_number, "empty name in the line of button %d", number);
            return 0;
        }
        if (button->count == KEYMAP_MAX_BINDINGS) {
            set_error(error, line_number, "more than %d names, at \"%s\"", KEYMAP_MAX_BINDINGS, token);
            return 0;
        }
        keymap_binding_t binding;
        if (!keymap_find_binding(token, &binding)) {
            set_error(error, line_number, "unknown key or gamepad button \"%s\"", token);
            return 0;
        }
        if (name_reserved(token)) {
            set_error(error, line_number, "\"%s\" is reserved by the game", token);
            return 0;
        }
        if (binding_used(out, binding)) {
            set_error(error, line_number, "\"%s\" already bound", token);
            return 0;
        }
        button->bindings[button->count] = binding;
        button->count++;
        if (comma == NULL) {
            return 1;
        }
        item = comma + 1;
    }
}

int keymap_parse(const char *text, keymap_t *out, keymap_error_t *error) {
    memset(out, 0, sizeof *out);
    memset(error, 0, sizeof *error);
    const char *line = text;
    int line_number = 0;
    while (*line != '\0') {
        line_number++;
        const char *newline = strchr(line, '\n');
        const char *end = newline != NULL ? newline : line + strlen(line);
        const char *next = newline != NULL ? newline + 1 : end;
        if (end > line && end[-1] == '\r') {
            end--;
        }
        if (end - line > KEYMAP_MAX_LINE_LENGTH) {
            set_error(error, line_number, "line longer than %d characters", KEYMAP_MAX_LINE_LENGTH);
            return 0;
        }
        const char *content_end = end;
        const char *hash = memchr(line, '#', (size_t)(end - line));
        if (hash != NULL) {
            content_end = hash;
        }
        const char *content_start = skip_blanks(line, content_end);
        content_end = trim_end(content_start, content_end);
        if (content_start < content_end && !parse_line(content_start, content_end, line_number, out, error)) {
            return 0;
        }
        line = next;
    }
    for (int i = 0; i < P8_PAD_BUTTON_COUNT; i++) {
        if (out->buttons[i].count == 0) {
            set_error(error, 0, "no line for button %d", i + 1);
            return 0;
        }
    }
    return 1;
}

int keymap_pad_buttons(const keymap_t *keymap, keymap_is_down_fn is_down, void *context) {
    int mask = 0;
    for (int i = 0; i < P8_PAD_BUTTON_COUNT; i++) {
        const keymap_button_t *button = &keymap->buttons[i];
        for (int j = 0; j < button->count; j++) {
            if (is_down(button->bindings[j].device, button->bindings[j].code, context)) {
                mask |= 1 << i;
                break;
            }
        }
    }
    return mask;
}
