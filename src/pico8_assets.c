#include "pico8_assets.h"

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

static int parse_json_int_field(const char *json, const char *key) {
    const char *p = strstr(json, key);
    if (!p) {
        return -1;
    }
    p += strlen(key);
    return (int)strtol(p, NULL, 10);
}

typedef void (*int_array_sink_fn)(int index, int value, void *ctx);

static int parse_json_int_array(const char *json, const char *key,
                                 int_array_sink_fn sink, void *ctx) {
    const char *p = strstr(json, key);
    if (!p) {
        return -1;
    }
    p = strchr(p, '[');
    if (!p) {
        return -1;
    }
    p++;

    int index = 0;
    while (*p && *p != ']') {
        char *end;
        long val = strtol(p, &end, 10);
        if (end == p) {
            p++;
            continue;
        }
        sink(index, (int)val, ctx);
        index++;
        p = end;
    }
    return index;
}

typedef struct {
    pico8_mem_t *mem;
    int width;
} asset_sink_ctx_t;

static int load_flat_json_array(const char *path, const char *array_key,
                                 int_array_sink_fn sink, pico8_mem_t *mem) {
    char *json = read_whole_file(path);
    if (!json) {
        return 0;
    }

    int width = parse_json_int_field(json, "\"width\":");
    if (width <= 0) {
        free(json);
        return 0;
    }

    asset_sink_ctx_t ctx;
    ctx.mem = mem;
    ctx.width = width;

    int count = parse_json_int_array(json, array_key, sink, &ctx);
    free(json);
    return count > 0;
}

static void gfx_pixel_sink(int index, int value, void *ctx_ptr) {
    asset_sink_ctx_t *ctx = (asset_sink_ctx_t *)ctx_ptr;
    int row = index / ctx->width;
    int col = index % ctx->width;
    int addr = PICO8_ADDR_SPRITE_SHEET + row * ctx->width + col;
    pico8_poke(ctx->mem, addr, (unsigned char)value);
}

int pico8_load_gfx_json(pico8_mem_t *mem, const char *path) {
    return load_flat_json_array(path, "\"pixels\":", gfx_pixel_sink, mem);
}

#define PICO8_MAP_BASE_ROWS 32

static void map_cell_sink(int index, int value, void *ctx_ptr) {
    asset_sink_ctx_t *ctx = (asset_sink_ctx_t *)ctx_ptr;
    int row = index / ctx->width;
    int col = index % ctx->width;
    int addr;
    if (row < PICO8_MAP_BASE_ROWS) {
        addr = PICO8_ADDR_MAP + row * ctx->width + col;
    } else {
        addr = PICO8_ADDR_SPRITE_MAP_SHARED + (row - PICO8_MAP_BASE_ROWS) * ctx->width + col;
    }
    pico8_poke(ctx->mem, addr, (unsigned char)value);
}

int pico8_load_map_json(pico8_mem_t *mem, const char *path) {
    return load_flat_json_array(path, "\"cells\":", map_cell_sink, mem);
}
