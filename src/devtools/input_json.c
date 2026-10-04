#include "p8/input.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_whole_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long size = ftell(f);
    if (size < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }

    char *buf = malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t got = fread(buf, 1, (size_t)size, f);
    fclose(f);
    buf[got] = '\0';
    return buf;
}

static int find_int_field(const char *obj, const char *key, int *out) {
    const char *p = strstr(obj, key);
    if (!p) {
        return 0;
    }
    p += strlen(key);
    char *end;
    long val = strtol(p, &end, 10);
    if (end == p) {
        return 0;
    }
    *out = (int)val;
    return 1;
}

static int find_dpad_field(const char *obj, int dpad_out[6]) {
    const char *p = strstr(obj, "\"dpad\":");
    if (!p) {
        return 0;
    }
    p = strchr(p, '[');
    if (!p) {
        return 0;
    }
    p++;

    int count = 0;
    for (;;) {
        while (*p == ' ' || *p == ',' || *p == '\n' || *p == '\r' || *p == '\t') {
            p++;
        }
        if (*p == ']' || count == 6) {
            break;
        }
        if (strncmp(p, "true", 4) == 0) {
            dpad_out[count++] = 1;
            p += 4;
        } else if (strncmp(p, "false", 5) == 0) {
            dpad_out[count++] = 0;
            p += 5;
        } else {
            return 0;
        }
    }
    if (*p != ']' || (count != 4 && count != 6)) {
        return 0;
    }
    for (; count < 6; count++) {
        dpad_out[count] = 0;
    }
    return 1;
}

/* Searches only json[0, head_end): a key inside a frame or after the frames
 * array is not the session's declaration. Returns 0 on a value other than
 * true or false; an absent key reads as 0. */
static int find_keyboard_cursor(const char *json, const char *head_end, int *out) {
    static const char key[] = "\"keyboard_cursor\":";
    const char *p = strstr(json, key);
    *out = 0;
    if (!p || p >= head_end) {
        return 1;
    }
    p += strlen(key);
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    if (strncmp(p, "true", 4) == 0) {
        *out = 1;
        return 1;
    }
    return strncmp(p, "false", 5) == 0;
}

void p8_input_free(p8_input_t *input) {
    free(input->frames);
    input->frames = NULL;
    input->frame_count = 0;
    input->current_frame = 0;
    input->keyboard_cursor = 0;
}

int p8_input_load(p8_input_t *input, const char *path) {
    char *json = read_whole_file(path);
    if (!json) {
        return 0;
    }

    const char *cursor = strstr(json, "\"frames\":");
    if (!cursor) {
        free(json);
        return 0;
    }
    int keyboard_cursor;
    if (!find_keyboard_cursor(json, cursor, &keyboard_cursor)) {
        free(json);
        return 0;
    }
    cursor = strchr(cursor, '[');
    if (!cursor) {
        free(json);
        return 0;
    }
    cursor++;

    int capacity = 4;
    int count = 0;
    int succeeded = 1;
    p8_input_frame_t *frames = malloc(sizeof(p8_input_frame_t) * (size_t)capacity);
    if (!frames) {
        free(json);
        return 0;
    }

    while (*cursor) {
        while (*cursor == ' ' || *cursor == ',' || *cursor == '\n' || *cursor == '\r' || *cursor == '\t') {
            cursor++;
        }
        if (*cursor == ']') {
            break;
        }
        if (*cursor != '{') {
            succeeded = 0;
            break;
        }

        const char *frame_start = cursor;
        const char *frame_end = strchr(cursor, '}');
        if (!frame_end) {
            succeeded = 0;
            break;
        }

        size_t frame_length = (size_t)(frame_end - frame_start) + 1;
        char *frame_copy = malloc(frame_length + 1);
        if (!frame_copy) {
            succeeded = 0;
            break;
        }
        memcpy(frame_copy, frame_start, frame_length);
        frame_copy[frame_length] = '\0';

        p8_input_frame_t frame;
        int found = find_int_field(frame_copy, "\"mouse_x\":", &frame.mouse_x) &&
                    find_int_field(frame_copy, "\"mouse_y\":", &frame.mouse_y) &&
                    find_int_field(frame_copy, "\"mouse_buttons\":", &frame.mouse_buttons) &&
                    find_dpad_field(frame_copy, frame.dpad);
        free(frame_copy);

        if (!found) {
            succeeded = 0;
            break;
        }

        if (count == capacity) {
            capacity *= 2;
            p8_input_frame_t *bigger = realloc(frames, sizeof(p8_input_frame_t) * (size_t)capacity);
            if (!bigger) {
                succeeded = 0;
                break;
            }
            frames = bigger;
        }
        frames[count++] = frame;

        cursor = frame_end + 1;
    }

    free(json);

    if (!succeeded || count == 0) {
        free(frames);
        return 0;
    }

    free(input->frames);
    input->frames = frames;
    input->frame_count = count;
    input->current_frame = 0;
    input->keyboard_cursor = keyboard_cursor;
    return 1;
}

/* The file's fixed head and tail, and the single spelling of a frame line,
 * shared by p8_input_save and p8_input_append: the two writers must not
 * drift into different bytes for the same frames (see p8_input_append's
 * contract in p8/input.h). Frame lines are joined by FRAME_SEPARATOR, the
 * last one followed by JSON_SUFFIX. */
#define JSON_PREFIX "{\n  \"frames\": [\n"
#define JSON_PREFIX_KEYBOARD_CURSOR "{\n  \"keyboard_cursor\": true,\n  \"frames\": [\n"
#define JSON_SUFFIX "\n  ]\n}\n"
#define FRAME_SEPARATOR ",\n"
#define FRAME_LINE_MAX 160

/* Never "false": an absent member is the only spelling of a mouse session. */
static const char *json_prefix(int keyboard_cursor) {
    return keyboard_cursor ? JSON_PREFIX_KEYBOARD_CURSOR : JSON_PREFIX;
}

static int format_frame_line(char *buf, size_t size, const p8_input_frame_t *fr) {
    int rc = snprintf(
        buf,
        size,
        "    { \"mouse_x\": %d, \"mouse_y\": %d, \"mouse_buttons\": %d, \"dpad\": [%s, %s, %s, %s, %s, %s] }",
        fr->mouse_x,
        fr->mouse_y,
        fr->mouse_buttons,
        fr->dpad[0] ? "true" : "false",
        fr->dpad[1] ? "true" : "false",
        fr->dpad[2] ? "true" : "false",
        fr->dpad[3] ? "true" : "false",
        fr->dpad[4] ? "true" : "false",
        fr->dpad[5] ? "true" : "false");
    return rc > 0 && (size_t)rc < size;
}

int p8_input_save(const p8_input_frame_t *frames, int frame_count, const char *path, int keyboard_cursor) {
    if (!frames || frame_count <= 0) {
        return 0;
    }

    size_t tmp_path_size = strlen(path) + 5;
    char *tmp_path = malloc(tmp_path_size);
    if (!tmp_path) {
        return 0;
    }
    snprintf(tmp_path, tmp_path_size, "%s.tmp", path);
    /* tmp_path_size was sized for this exact suffix; a longer one would
     * silently truncate the path and rename over the wrong file. */
    assert(strlen(tmp_path) == strlen(path) + 4);

    FILE *f = fopen(tmp_path, "wb");
    if (!f) {
        free(tmp_path);
        return 0;
    }

    int ok = 1;
    if (fputs(json_prefix(keyboard_cursor), f) < 0) {
        ok = 0;
    }
    for (int i = 0; ok && i < frame_count; i++) {
        char line[FRAME_LINE_MAX];
        if (!format_frame_line(line, sizeof(line), &frames[i])) {
            ok = 0;
            break;
        }
        if (fputs(line, f) < 0 || fputs((i + 1 < frame_count) ? FRAME_SEPARATOR : JSON_SUFFIX, f) < 0) {
            ok = 0;
        }
    }

    if (fclose(f) != 0) {
        ok = 0;
    }

    if (!ok) {
        remove(tmp_path);
        free(tmp_path);
        return 0;
    }

    if (rename(tmp_path, path) != 0) {
        remove(tmp_path);
        free(tmp_path);
        return 0;
    }

    free(tmp_path);
    return 1;
}

int p8_input_append(const p8_input_frame_t *frame, const char *path, long *tail_offset, int keyboard_cursor) {
    if (!frame || !path || !tail_offset) {
        return 0;
    }

    char line[FRAME_LINE_MAX];
    if (!format_frame_line(line, sizeof(line), frame)) {
        return 0;
    }

    /* `*tail_offset` is where JSON_SUFFIX currently starts. Writing
     * there turns that suffix into a separator, the new line and a fresh
     * suffix -- strictly longer, so the file only grows and nothing ever
     * needs truncating. */
    int fresh = (*tail_offset <= 0);
    const char *head = fresh ? json_prefix(keyboard_cursor) : FRAME_SEPARATOR;
    long start = fresh ? 0 : *tail_offset;

    char tail[sizeof(JSON_PREFIX_KEYBOARD_CURSOR) + FRAME_LINE_MAX + sizeof(JSON_SUFFIX)];
    int written = snprintf(tail, sizeof(tail), "%s%s%s", head, line, JSON_SUFFIX);
    if (written <= 0 || (size_t)written >= sizeof(tail)) {
        return 0;
    }

    FILE *f = fopen(path, fresh ? "wb" : "r+b");
    if (!f) {
        return 0;
    }
    if (!fresh && fseek(f, start, SEEK_SET) != 0) {
        fclose(f);
        return 0;
    }

    /* One fwrite of the whole tail: a process killed mid-session leaves the
     * recording either without this frame or with it, never torn. */
    size_t tail_len = (size_t)written;
    if (fwrite(tail, 1, tail_len, f) != tail_len) {
        fclose(f);
        return 0;
    }
    if (fclose(f) != 0) {
        return 0;
    }

    *tail_offset = start + (long)strlen(head) + (long)strlen(line);
    /* The new offset must land exactly on the JSON_SUFFIX just written,
     * which is where the next frame overwrites it. */
    assert(*tail_offset + (long)strlen(JSON_SUFFIX) == start + (long)written);
    return 1;
}
