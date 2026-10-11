/* usage: psx_pad_log <tty> <pad-script> <events.json>
 * Turns the PSX game's pad log into <events.json> and compares its pad states with the script's.
 * Prints one "psx-pad-log: log: ..." and, when the log parsed, one "psx-pad-log: states: ..." line.
 * Exit 0 when both are ok, 1 otherwise, 2 on usage. */

#include "devtools/pad_log.h"

#include <stdio.h>
#include <stdlib.h>

/* Returns a NUL-terminated copy of the file, for the caller to free, or NULL when unreadable. */
static char *read_text(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) return NULL;
    size_t size = 0;
    size_t capacity = 4096;
    char *text = malloc(capacity);
    while (text != NULL) {
        size += fread(text + size, 1, capacity - size - 1, file);
        if (size < capacity - 1) break;
        capacity *= 2;
        char *grown = realloc(text, capacity);
        if (grown == NULL) free(text);
        text = grown;
    }
    int failed = ferror(file);
    fclose(file);
    if (text == NULL || failed) {
        free(text);
        return NULL;
    }
    text[size] = '\0';
    return text;
}

static void print_state(int index, int count, const int *states) {
    if (index < count) {
        printf("%03x", states[index]);
    } else {
        printf("none");
    }
}

static int compare_states(const pad_log_t *log, const char *script_path) {
    char *text = read_text(script_path);
    if (text == NULL) {
        printf("psx-pad-log: states: %s: unreadable\n", script_path);
        return 0;
    }
    static pad_script_t script;
    char error[PAD_LOG_ERROR_CAP];
    int parsed = pad_log_parse_script(text, &script, error);
    free(text);
    if (!parsed) {
        printf("psx-pad-log: states: %s: %s\n", script_path, error);
        return 0;
    }
    int index = pad_log_first_difference(log, &script);
    if (index < 0) {
        printf("psx-pad-log: states: ok, %d states\n", script.count);
        return 1;
    }
    printf("psx-pad-log: states: first differing state, %d: log ", index);
    print_state(index, log->state_count, log->states);
    printf(" script ");
    print_state(index, script.count, script.states);
    printf("\n");
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: psx_pad_log <tty> <pad-script> <events.json>\n");
        return 2;
    }
    char *text = read_text(argv[1]);
    if (text == NULL) {
        printf("psx-pad-log: log: %s: unreadable\n", argv[1]);
        return 1;
    }
    pad_log_t log;
    char error[PAD_LOG_ERROR_CAP];
    int parsed = pad_log_parse(text, &log, error);
    free(text);
    if (!parsed) {
        printf("psx-pad-log: log: %s: %s\n", argv[1], error);
        return 1;
    }
    int saved = pad_log_save(&log, argv[3]);
    if (saved) {
        printf("psx-pad-log: log: ok, %d frames\n", log.frame_count);
    } else {
        printf("psx-pad-log: log: cannot write %s\n", argv[3]);
    }
    int states_match = compare_states(&log, argv[2]);
    pad_log_free(&log);
    return saved && states_match ? 0 : 1;
}
