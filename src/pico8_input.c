#include "pico8_input.h"

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

static int find_dpad_field(const char *obj, int dpad_out[4]) {
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
    while (count < 4) {
        while (*p == ' ' || *p == ',' || *p == '\n' || *p == '\r' || *p == '\t') {
            p++;
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
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') {
        p++;
    }
    return *p == ']';
}

void pico8_input_init(pico8_input_t *input) {
    input->frames = NULL;
    input->frame_count = 0;
    input->current_frame = 0;
}

void pico8_input_free(pico8_input_t *input) {
    free(input->frames);
    input->frames = NULL;
    input->frame_count = 0;
    input->current_frame = 0;
}

int pico8_input_load(pico8_input_t *input, const char *path) {
    char *json = read_whole_file(path);
    if (!json) {
        return 0;
    }

    const char *p = strstr(json, "\"frames\":");
    if (!p) {
        free(json);
        return 0;
    }
    p = strchr(p, '[');
    if (!p) {
        free(json);
        return 0;
    }
    p++;

    int capacity = 4;
    int count = 0;
    int ok = 1;
    pico8_input_frame_t *frames = malloc(sizeof(pico8_input_frame_t) * (size_t)capacity);
    if (!frames) {
        free(json);
        return 0;
    }

    while (*p) {
        while (*p == ' ' || *p == ',' || *p == '\n' || *p == '\r' || *p == '\t') {
            p++;
        }
        if (*p == ']') {
            break;
        }
        if (*p != '{') {
            ok = 0;
            break;
        }

        const char *obj_start = p;
        const char *obj_end = strchr(p, '}');
        if (!obj_end) {
            ok = 0;
            break;
        }

        size_t obj_len = (size_t)(obj_end - obj_start) + 1;
        char *obj_buf = malloc(obj_len + 1);
        if (!obj_buf) {
            ok = 0;
            break;
        }
        memcpy(obj_buf, obj_start, obj_len);
        obj_buf[obj_len] = '\0';

        pico8_input_frame_t frame;
        int found = find_int_field(obj_buf, "\"mouse_x\":", &frame.mouse_x) &&
                    find_int_field(obj_buf, "\"mouse_y\":", &frame.mouse_y) &&
                    find_int_field(obj_buf, "\"mouse_buttons\":", &frame.mouse_buttons) &&
                    find_dpad_field(obj_buf, frame.dpad);
        free(obj_buf);

        if (!found) {
            ok = 0;
            break;
        }

        if (count == capacity) {
            capacity *= 2;
            pico8_input_frame_t *bigger = realloc(frames, sizeof(pico8_input_frame_t) * (size_t)capacity);
            if (!bigger) {
                ok = 0;
                break;
            }
            frames = bigger;
        }
        frames[count++] = frame;

        p = obj_end + 1;
    }

    free(json);

    if (!ok || count == 0) {
        free(frames);
        return 0;
    }

    free(input->frames);
    input->frames = frames;
    input->frame_count = count;
    input->current_frame = 0;
    return 1;
}

/* The events.json file's fixed head and tail, and the one place a frame
 * object's own line is spelled out -- shared by pico8_input_save and
 * pico8_input_append so the two writers cannot drift into producing
 * different bytes for the same frames. Between two frame lines comes
 * FRAME_SEPARATOR; after the last one comes JSON_SUFFIX. */
#define JSON_PREFIX "{\n  \"frames\": [\n"
#define JSON_SUFFIX "\n  ]\n}\n"
#define FRAME_SEPARATOR ",\n"
#define FRAME_LINE_MAX 160

static int format_frame_line(char *buf, size_t size, const pico8_input_frame_t *fr) {
    int rc = snprintf(
        buf,
        size,
        "    { \"mouse_x\": %d, \"mouse_y\": %d, \"mouse_buttons\": %d, \"dpad\": [%s, %s, %s, %s] }",
        fr->mouse_x,
        fr->mouse_y,
        fr->mouse_buttons,
        fr->dpad[0] ? "true" : "false",
        fr->dpad[1] ? "true" : "false",
        fr->dpad[2] ? "true" : "false",
        fr->dpad[3] ? "true" : "false");
    return rc > 0 && (size_t)rc < size;
}

int pico8_input_save(const pico8_input_frame_t *frames, int frame_count, const char *path) {
    if (!frames || frame_count <= 0) {
        return 0;
    }

    size_t tmp_len = strlen(path) + 5;
    char *tmp_path = malloc(tmp_len);
    if (!tmp_path) {
        return 0;
    }
    snprintf(tmp_path, tmp_len, "%s.tmp", path);

    FILE *f = fopen(tmp_path, "wb");
    if (!f) {
        free(tmp_path);
        return 0;
    }

    int ok = 1;
    if (fputs(JSON_PREFIX, f) < 0) {
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

int pico8_input_append(const pico8_input_frame_t *frame, const char *path, long *io_tail_offset) {
    if (!frame || !path || !io_tail_offset) {
        return 0;
    }

    char line[FRAME_LINE_MAX];
    if (!format_frame_line(line, sizeof(line), frame)) {
        return 0;
    }

    /* `*io_tail_offset` is where the previous frame's object ended, i.e.
     * where JSON_SUFFIX currently starts. Writing the new frame there
     * turns that suffix into a separator plus the new frame's line plus a
     * fresh suffix, which is strictly longer -- the file only ever grows,
     * so nothing has to be truncated. */
    int fresh = (*io_tail_offset <= 0);
    const char *head = fresh ? JSON_PREFIX : FRAME_SEPARATOR;
    long start = fresh ? 0 : *io_tail_offset;

    char tail[sizeof(JSON_PREFIX) + FRAME_LINE_MAX + sizeof(JSON_SUFFIX)];
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

    /* One fwrite of the whole tail, so a process killed mid-session cannot
     * land between two halves of it: the recording on disk is either the
     * one without this frame or the one with it, never a torn file. That is
     * the same all-or-nothing guarantee pico8_input_save gets from writing
     * a .tmp file and renaming it. */
    size_t tail_len = (size_t)written;
    if (fwrite(tail, 1, tail_len, f) != tail_len) {
        fclose(f);
        return 0;
    }
    if (fclose(f) != 0) {
        return 0;
    }

    *io_tail_offset = start + (long)strlen(head) + (long)strlen(line);
    return 1;
}

void pico8_input_step(pico8_input_t *input) {
    assert(input->current_frame + 1 < input->frame_count);
    input->current_frame++;
}

int pico8_btn(const pico8_input_t *input, int b) {
    if (b < 0 || b > 3) {
        return 0;
    }
    if (input->frame_count == 0 || input->current_frame >= input->frame_count) {
        return 0;
    }
    return input->frames[input->current_frame].dpad[b];
}

int pico8_stat(const pico8_input_t *input, int n) {
    if (input->frame_count == 0 || input->current_frame >= input->frame_count) {
        return 0;
    }
    const pico8_input_frame_t *f = &input->frames[input->current_frame];
    switch (n) {
        case 32:
            return f->mouse_x;
        case 33:
            return f->mouse_y;
        case 34:
            return f->mouse_buttons;
        default:
            return 0;
    }
}
