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
    if (fprintf(f, "{\n  \"frames\": [\n") < 0) {
        ok = 0;
    }
    for (int i = 0; ok && i < frame_count; i++) {
        const pico8_input_frame_t *fr = &frames[i];
        int rc = fprintf(
            f,
            "    { \"mouse_x\": %d, \"mouse_y\": %d, \"mouse_buttons\": %d, \"dpad\": [%s, %s, %s, %s] }%s\n",
            fr->mouse_x,
            fr->mouse_y,
            fr->mouse_buttons,
            fr->dpad[0] ? "true" : "false",
            fr->dpad[1] ? "true" : "false",
            fr->dpad[2] ? "true" : "false",
            fr->dpad[3] ? "true" : "false",
            (i + 1 < frame_count) ? "," : "");
        if (rc < 0) {
            ok = 0;
        }
    }
    if (ok && fprintf(f, "  ]\n}\n") < 0) {
        ok = 0;
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
