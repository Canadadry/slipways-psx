#ifndef DEVTOOLS_PAD_LOG_H
#define DEVTOOLS_PAD_LOG_H

/* The PSX game's TTY: "psx-game: pad <frame> <hex>" on frame 0 and on each change of pad_buttons,
 * then one "psx-replay: done <name> <frames> vsyncs <n>"; every other line is ignored. A pad
 * script: "<offset> <BUTTON>..." or "<offset> -", offsets increasing, the last step "-", blank and
 * '#' lines skipped. */

enum { PAD_LOG_ERROR_CAP = 160, PAD_SCRIPT_STATE_CAP = 256 };

typedef struct {
    int frame_count;
    int *pad_buttons;
    int state_count;
    int *states;
} pad_log_t;

typedef struct {
    int count;
    int states[PAD_SCRIPT_STATE_CAP];
} pad_script_t;

/* Returns 1 with *log filled, to free with pad_log_free; or 0 with error naming the line and
 * nothing to free. states holds the ordered distinct consecutive pad_buttons. */
int pad_log_parse(const char *text, pad_log_t *log, char error[PAD_LOG_ERROR_CAP]);

void pad_log_free(pad_log_t *log);

/* Writes the frames through p8_input_save, with no initial_save; returns 0 on a write error. */
int pad_log_save(const pad_log_t *log, const char *path);

/* Returns 1 with the script's ordered distinct states, starting from 0 before its first step, or 0
 * with error naming the line. */
int pad_log_parse_script(const char *text, pad_script_t *script, char error[PAD_LOG_ERROR_CAP]);

/* Returns -1 when the log's states are the script's, else the index of the first differing state. */
int pad_log_first_difference(const pad_log_t *log, const pad_script_t *script);

#endif
