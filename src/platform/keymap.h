#ifndef PLATFORM_KEYMAP_H
#define PLATFORM_KEYMAP_H

#include "p8/input.h"

#define KEYMAP_MAX_BINDINGS 4
#define KEYMAP_MAX_LINE_LENGTH 127

typedef enum { KEYMAP_KEYBOARD, KEYMAP_GAMEPAD } keymap_device_t;

typedef struct {
    keymap_device_t device;
    int code;
} keymap_binding_t;

typedef struct {
    keymap_binding_t bindings[KEYMAP_MAX_BINDINGS];
    int count;
} keymap_button_t;

typedef struct {
    keymap_button_t buttons[P8_PAD_BUTTON_COUNT];
} keymap_t;

typedef struct {
    int line;
    char message[192];
} keymap_error_t;

typedef int (*keymap_is_down_fn)(keymap_device_t device, int code, void *context);

extern const keymap_t keymap_compiled;

int keymap_parse(const char *text, keymap_t *out, keymap_error_t *error);

int keymap_pad_buttons(const keymap_t *keymap, keymap_is_down_fn is_down, void *context);

int keymap_find_binding(const char *name, keymap_binding_t *out);

#endif
