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

static int has_pad_header(const char *json, const char *head_end) {
    static const char key[] = "\"input\":";
    const char *p = strstr(json, key);
    if (!p || p >= head_end) {
        return 0;
    }
    p += strlen(key);
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    return strncmp(p, "\"pad\"", 5) == 0;
}

static const char *const pad_button_names[] = {
    "up", "down", "left", "right", "cross", "circle", "square", "triangle", "l1", "r1", "start", "select",
};
#define PAD_BUTTON_COUNT ((int)(sizeof(pad_button_names) / sizeof(pad_button_names[0])))

static int pad_button_from_name(const char *name, size_t length) {
    for (int bit = 0; bit < PAD_BUTTON_COUNT; bit++) {
        if (strlen(pad_button_names[bit]) == length && strncmp(pad_button_names[bit], name, length) == 0) {
            return 1 << bit;
        }
    }
    return 0;
}

static int is_json_separator(char c) {
    return c == ' ' || c == ',' || c == '\n' || c == '\r' || c == '\t';
}

/* Fills only out->pad_buttons, which must start at 0. */
static int parse_pad_frame(const char *obj, p8_input_frame_t *out) {
    static const char *const mouse_keys[] = {"\"mouse_x\"", "\"mouse_y\"", "\"mouse_buttons\"", "\"dpad\""};
    for (size_t i = 0; i < sizeof(mouse_keys) / sizeof(mouse_keys[0]); i++) {
        if (strstr(obj, mouse_keys[i])) {
            return 0;
        }
    }
    const char *p = strstr(obj, "\"pad\":");
    if (!p) {
        return 0;
    }
    p = strchr(p, '[');
    if (!p) {
        return 0;
    }
    p++;

    for (;;) {
        while (is_json_separator(*p)) {
            p++;
        }
        if (*p == ']') {
            return 1;
        }
        if (*p >= '1' && *p <= '9') {
            int number = 0;
            while (*p >= '0' && *p <= '9' && number <= PAD_BUTTON_COUNT) {
                number = number * 10 + (*p - '0');
                p++;
            }
            int ends_element = is_json_separator(*p) || *p == ']';
            if (!ends_element || number > PAD_BUTTON_COUNT || (out->pad_buttons & (1 << (number - 1)))) {
                return 0;
            }
            out->pad_buttons |= 1 << (number - 1);
            continue;
        }
        if (*p != '"') {
            return 0;
        }
        const char *name = p + 1;
        const char *name_end = strchr(name, '"');
        if (!name_end) {
            return 0;
        }
        int button = pad_button_from_name(name, (size_t)(name_end - name));
        if (!button || (out->pad_buttons & button)) {
            return 0;
        }
        out->pad_buttons |= button;
        p = name_end + 1;
    }
}

void p8_input_free(p8_input_t *input) {
    free(input->frames);
    input->frames = NULL;
    input->frame_count = 0;
    input->current_frame = 0;
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
    if (!has_pad_header(json, cursor)) {
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
        while (is_json_separator(*cursor)) {
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

        p8_input_frame_t frame = {0};
        int found = parse_pad_frame(frame_copy, &frame);
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
    return 1;
}

#define JSON_HEADER "{\n  \"input\": \"pad\",\n"
#define JSON_FRAMES_OPEN "  \"frames\": [\n"
#define JSON_PREFIX JSON_HEADER JSON_FRAMES_OPEN
#define HEADER_MEMBER_INDENT "  "
#define HEADER_MEMBER_END ",\n"
#define JSON_SUFFIX "\n  ]\n}\n"
#define FRAME_SEPARATOR ",\n"
#define FRAME_LINE_MAX 160

static int format_pad_frame_line(char *buf, size_t size, const p8_input_frame_t *fr) {
    assert((fr->pad_buttons & ~((1 << PAD_BUTTON_COUNT) - 1)) == 0 && "a bit past P8_PAD_SELECT has no button number");
    int used = snprintf(buf, size, "    { \"pad\": [");
    const char *separator = "";
    for (int bit = 0; bit < PAD_BUTTON_COUNT; bit++) {
        if (used < 0 || (size_t)used >= size) {
            return 0;
        }
        if (fr->pad_buttons & (1 << bit)) {
            used += snprintf(buf + used, size - (size_t)used, "%s%d", separator, bit + 1);
            separator = ", ";
        }
    }
    if (used < 0 || (size_t)used >= size) {
        return 0;
    }
    int rc = snprintf(buf + used, size - (size_t)used, "] }");
    return rc > 0 && (size_t)(used + rc) < size;
}

int p8_input_save(const p8_input_frame_t *frames, int frame_count, const char *path) {
    if (!frames || frame_count <= 0) {
        return 0;
    }

    size_t tmp_path_size = strlen(path) + 5;
    char *tmp_path = malloc(tmp_path_size);
    if (!tmp_path) {
        return 0;
    }
    snprintf(tmp_path, tmp_path_size, "%s.tmp", path);
    assert(strlen(tmp_path) == strlen(path) + 4);

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
        if (!format_pad_frame_line(line, sizeof(line), &frames[i])) {
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

int p8_input_append(const p8_input_frame_t *frame, const char *path, long *tail_offset, const char *header_member) {
    if (!frame || !path || !tail_offset) {
        return 0;
    }

    char line[FRAME_LINE_MAX];
    if (!format_pad_frame_line(line, sizeof(line), frame)) {
        return 0;
    }

    int fresh = (*tail_offset <= 0);
    const char *member = (fresh && header_member) ? header_member : NULL;
    long start = fresh ? 0 : *tail_offset;

    char fixed_tail[sizeof(JSON_PREFIX) + FRAME_LINE_MAX + sizeof(JSON_SUFFIX)];
    char *tail = fixed_tail;
    size_t tail_size = sizeof(fixed_tail);
    if (member) {
        tail_size += sizeof(HEADER_MEMBER_INDENT) + strlen(member) + sizeof(HEADER_MEMBER_END);
        tail = malloc(tail_size);
        if (!tail) {
            return 0;
        }
    }

    int written;
    if (!fresh) {
        written = snprintf(tail, tail_size, "%s%s%s", FRAME_SEPARATOR, line, JSON_SUFFIX);
    } else if (member) {
        written = snprintf(tail, tail_size, "%s%s%s%s%s%s%s", JSON_HEADER, HEADER_MEMBER_INDENT, member,
                           HEADER_MEMBER_END, JSON_FRAMES_OPEN, line, JSON_SUFFIX);
    } else {
        written = snprintf(tail, tail_size, "%s%s%s", JSON_PREFIX, line, JSON_SUFFIX);
    }

    int ok = written > 0 && (size_t)written < tail_size;
    FILE *f = ok ? fopen(path, fresh ? "wb" : "r+b") : NULL;
    ok = f != NULL;
    if (ok && !fresh && fseek(f, start, SEEK_SET) != 0) {
        ok = 0;
    }

    /* One fwrite of the whole tail: a process killed mid-session must never leave a torn frame. */
    size_t tail_len = ok ? (size_t)written : 0;
    if (ok && fwrite(tail, 1, tail_len, f) != tail_len) {
        ok = 0;
    }
    if (f && fclose(f) != 0) {
        ok = 0;
    }
    if (tail != fixed_tail) {
        free(tail);
    }
    if (!ok) {
        return 0;
    }

    *tail_offset = start + (long)tail_len - (long)strlen(JSON_SUFFIX);
    return 1;
}
