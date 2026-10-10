#ifndef PLATFORM_KEYMAP_H
#define PLATFORM_KEYMAP_H

#define KEYMAP_BUTTON_COUNT 12
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

/* buttons[i] is the PSX button of bit 1 << i of the P8_PAD_BUTTON_* mask. */
typedef struct {
    keymap_button_t buttons[KEYMAP_BUTTON_COUNT];
} keymap_t;

/* line is 0 when the error is about the file as a whole. */
typedef struct {
    int line;
    char message[192];
} keymap_error_t;

typedef enum {
    KEYMAP_LOADED,
    KEYMAP_MISSING,
    KEYMAP_INVALID
} keymap_load_status_t;

typedef int (*keymap_is_down_fn)(keymap_device_t device, int code, void *context);

const char *keymap_default_text(void);

void keymap_defaults(keymap_t *out);

/* Returns 0 and fills *error on the first invalid line; *out is then unusable. */
int keymap_parse(const char *text, keymap_t *out, keymap_error_t *error);

/* KEYMAP_MISSING fills *out with the defaults; KEYMAP_INVALID fills *error. */
keymap_load_status_t keymap_load(const char *path, keymap_t *out, keymap_error_t *error);

int keymap_pad_buttons(const keymap_t *keymap, keymap_is_down_fn is_down, void *context);

/* Returns 0 when the name is not a raylib key or gamepad button name. */
int keymap_find_binding(const char *name, keymap_binding_t *out);

const char *keymap_button_name(int index);

#endif
