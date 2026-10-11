#include "devtools/pad_log.h"

#include "p8/input.h"
#include "platform/psx_pad_map.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { LINE_CAP = 256, MAX_WORDS = 7, PAD_BUTTONS_MASK = (P8_PAD_BUTTON_12 << 1) - 1, PAD_SCRIPT_LINE_WORDS = 17 };

typedef struct {
    const char *at;
    int number;
    char line[LINE_CAP];
} line_reader_t;

/* Returns 0 at the end of text; a line longer than LINE_CAP - 1 is cut, CR stripped. */
static int next_line(line_reader_t *reader) {
    if (*reader->at == '\0') return 0;
    const char *end = strchr(reader->at, '\n');
    size_t length = end != NULL ? (size_t)(end - reader->at) : strlen(reader->at);
    size_t kept = length < LINE_CAP - 1 ? length : LINE_CAP - 1;
    memcpy(reader->line, reader->at, kept);
    reader->line[kept] = '\0';
    while (kept > 0 && reader->line[kept - 1] == '\r') reader->line[--kept] = '\0';
    reader->at += length + (end != NULL);
    reader->number++;
    return 1;
}

static void start_reader(line_reader_t *reader, const char *text) {
    reader->at = text;
    reader->number = 0;
}

/* Returns the word count, at most max_words + 1; the words point into line, which is cut. */
static int split(char *line, char **words, int max_words) {
    int count = 0;
    char *at = line;
    while (count <= max_words) {
        while (*at != '\0' && isspace((unsigned char)*at)) at++;
        if (*at == '\0') break;
        words[count++] = at;
        while (*at != '\0' && !isspace((unsigned char)*at)) at++;
        if (*at != '\0') *at++ = '\0';
    }
    return count;
}

static int parse_decimal(const char *word, int *out) {
    int value = 0;
    if (*word == '\0' || strlen(word) > 9) return 0;
    for (const char *c = word; *c != '\0'; c++) {
        if (!isdigit((unsigned char)*c)) return 0;
        value = value * 10 + (*c - '0');
    }
    *out = value;
    return 1;
}

static int parse_hex(const char *word, int *out) {
    int value = 0;
    if (*word == '\0' || strlen(word) > 4) return 0;
    for (const char *c = word; *c != '\0'; c++) {
        if (!isxdigit((unsigned char)*c)) return 0;
        value = value * 16 + (isdigit((unsigned char)*c) ? *c - '0' : tolower((unsigned char)*c) - 'a' + 10);
    }
    *out = value;
    return 1;
}

static int refuse(char *error, int line_number, const char *reason) {
    if (line_number > 0) {
        snprintf(error, PAD_LOG_ERROR_CAP, "line %d: %s", line_number, reason);
    } else {
        snprintf(error, PAD_LOG_ERROR_CAP, "%s", reason);
    }
    return 0;
}

static int is_line_of(char **words, int count, const char *first, const char *second) {
    return count >= 2 && strcmp(words[0], first) == 0 && strcmp(words[1], second) == 0;
}

static int find_done_line(const char *text, int *frame_count, char *error) {
    line_reader_t reader;
    char *words[MAX_WORDS + 1];
    char reason[PAD_LOG_ERROR_CAP];
    int done_line = 0;
    start_reader(&reader, text);
    while (next_line(&reader)) {
        int count = split(reader.line, words, MAX_WORDS);
        if (!is_line_of(words, count, "psx-replay:", "done")) continue;
        int vsyncs;
        if (count != 6 || strcmp(words[4], "vsyncs") != 0 || !parse_decimal(words[3], frame_count)
            || !parse_decimal(words[5], &vsyncs)) {
            return refuse(error, reader.number, "malformed done line");
        }
        if (done_line != 0) {
            snprintf(reason, sizeof reason, "second done line (the first is line %d)", done_line);
            return refuse(error, reader.number, reason);
        }
        done_line = reader.number;
    }
    if (done_line == 0) return refuse(error, 0, "no psx-replay: done line");
    return 1;
}

static int read_pad_lines(const char *text, pad_log_t *log, char *error) {
    line_reader_t reader;
    char *words[MAX_WORDS + 1];
    char reason[PAD_LOG_ERROR_CAP];
    int previous_frame = -1;
    int previous_value = 0;
    start_reader(&reader, text);
    while (next_line(&reader)) {
        int count = split(reader.line, words, MAX_WORDS);
        if (!is_line_of(words, count, "psx-game:", "pad")) continue;
        int frame, value;
        if (count != 4 || !parse_decimal(words[2], &frame) || !parse_hex(words[3], &value)) {
            return refuse(error, reader.number, "malformed pad line");
        }
        if (previous_frame < 0 && frame != 0) {
            snprintf(reason, sizeof reason, "first pad line at frame %d, not 0", frame);
            return refuse(error, reader.number, reason);
        }
        if (previous_frame >= 0 && frame <= previous_frame) {
            snprintf(reason, sizeof reason, "frame %d does not increase past %d", frame, previous_frame);
            return refuse(error, reader.number, reason);
        }
        if (value & ~PAD_BUTTONS_MASK) {
            snprintf(reason, sizeof reason, "pad %s has bits outside the twelve pad buttons", words[3]);
            return refuse(error, reader.number, reason);
        }
        if (frame >= log->frame_count) {
            snprintf(reason, sizeof reason, "pad line at frame %d, at or after the done line's %d frames", frame,
                     log->frame_count);
            return refuse(error, reader.number, reason);
        }
        for (int held = previous_frame + 1; held < frame; held++) log->pad_buttons[held] = previous_value;
        log->pad_buttons[frame] = value;
        if (log->state_count == 0 || log->states[log->state_count - 1] != value) {
            log->states[log->state_count++] = value;
        }
        previous_frame = frame;
        previous_value = value;
    }
    if (previous_frame < 0) return refuse(error, 0, "no psx-game: pad line");
    for (int held = previous_frame + 1; held < log->frame_count; held++) log->pad_buttons[held] = previous_value;
    return 1;
}

int pad_log_parse(const char *text, pad_log_t *log, char error[PAD_LOG_ERROR_CAP]) {
    memset(log, 0, sizeof *log);
    if (!find_done_line(text, &log->frame_count, error)) return 0;
    size_t count = log->frame_count > 0 ? (size_t)log->frame_count : 1;
    log->pad_buttons = malloc(count * sizeof *log->pad_buttons);
    log->states = malloc(count * sizeof *log->states);
    if (log->pad_buttons == NULL || log->states == NULL) {
        pad_log_free(log);
        return refuse(error, 0, "out of memory for the log's frames");
    }
    if (!read_pad_lines(text, log, error)) {
        pad_log_free(log);
        return 0;
    }
    return 1;
}

void pad_log_free(pad_log_t *log) {
    free(log->pad_buttons);
    free(log->states);
    memset(log, 0, sizeof *log);
}

int pad_log_save(const pad_log_t *log, const char *path) {
    assert(log->frame_count > 0 && "a parsed log has a pad line at frame 0, so at least one frame");
    p8_input_frame_t *frames = calloc((size_t)log->frame_count, sizeof *frames);
    if (frames == NULL) return 0;
    for (int frame = 0; frame < log->frame_count; frame++) frames[frame].pad_buttons = log->pad_buttons[frame];
    int saved = p8_input_save(frames, log->frame_count, path, NULL);
    free(frames);
    return saved;
}

static int add_state(pad_script_t *script, int value) {
    if (script->states[script->count - 1] == value) return 1;
    if (script->count == PAD_SCRIPT_STATE_CAP) return 0;
    script->states[script->count++] = value;
    return 1;
}

int pad_log_parse_script(const char *text, pad_script_t *script, char error[PAD_LOG_ERROR_CAP]) {
    line_reader_t reader;
    char *words[PAD_SCRIPT_LINE_WORDS + 1];
    char reason[PAD_LOG_ERROR_CAP];
    int previous_offset = -1;
    int last_step_held = 0;
    script->count = 1;
    script->states[0] = 0;
    start_reader(&reader, text);
    while (next_line(&reader)) {
        int count = split(reader.line, words, PAD_SCRIPT_LINE_WORDS);
        if (count == 0 || words[0][0] == '#') continue;
        int offset;
        if (!parse_decimal(words[0], &offset)) {
            snprintf(reason, sizeof reason, "vsync offset is not an integer: %s", words[0]);
            return refuse(error, reader.number, reason);
        }
        if (offset <= previous_offset) {
            snprintf(reason, sizeof reason, "vsync offset %d does not increase", offset);
            return refuse(error, reader.number, reason);
        }
        if (count < 2) return refuse(error, reader.number, "no button set (use - for none)");
        if (count > PAD_SCRIPT_LINE_WORDS) return refuse(error, reader.number, "more than 16 buttons");
        previous_offset = offset;
        uint16_t held = 0;
        if (!(count == 2 && strcmp(words[1], "-") == 0)) {
            for (int i = 1; i < count; i++) {
                uint16_t bit = psx_pad_map_hardware_bit(words[i]);
                if (bit == 0) {
                    snprintf(reason, sizeof reason, "unknown button %s", words[i]);
                    return refuse(error, reader.number, reason);
                }
                held |= bit;
            }
        }
        last_step_held = held != 0;
        if (!add_state(script, psx_pad_map_buttons(held))) {
            return refuse(error, reader.number, "more pad states than a script can hold");
        }
    }
    if (previous_offset < 0) return refuse(error, 0, "no step");
    if (last_step_held) return refuse(error, 0, "last step must be - (no button held)");
    return 1;
}

int pad_log_first_difference(const pad_log_t *log, const pad_script_t *script) {
    int shorter = log->state_count < script->count ? log->state_count : script->count;
    for (int i = 0; i < shorter; i++) {
        if (log->states[i] != script->states[i]) return i;
    }
    return log->state_count == script->count ? -1 : shorter;
}
