/* Le GIF d'une capture, relu par un décodeur LZW écrit ici, indépendant de
 * l'encodeur : un aller-retour entre deux moitiés du même code passerait sur
 * une largeur de code fausse des deux côtés. */

#include "devtools/gif.h"

#include "p8/palette.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SRC 128
#define SCALE 3
#define DST (SRC * SCALE)

typedef struct {
    unsigned char *bytes;
    long size;
    long pos;
} buf_t;

static buf_t slurp(FILE *file) {
    buf_t b = {0, 0, 0};
    fseek(file, 0, SEEK_END);
    b.size = ftell(file);
    rewind(file);
    b.bytes = malloc((size_t)b.size);
    assert(b.bytes != NULL);
    assert(fread(b.bytes, 1, (size_t)b.size, file) == (size_t)b.size);
    return b;
}

static int u8(buf_t *b) {
    assert(b->pos < b->size);
    return b->bytes[b->pos++];
}

static int u16(buf_t *b) {
    int lo = u8(b);
    return lo | (u8(b) << 8);
}

/* Les sous-blocs d'une image, concaténés. */
static unsigned char *read_sub_blocks(buf_t *b, long *out_len) {
    long cap = 1 << 16, len = 0;
    unsigned char *data = malloc((size_t)cap);
    assert(data != NULL);
    for (int n = u8(b); n != 0; n = u8(b)) {
        if (len + n > cap) {
            cap *= 2;
            data = realloc(data, (size_t)cap);
            assert(data != NULL);
        }
        for (int i = 0; i < n; i++) {
            data[len++] = (unsigned char)u8(b);
        }
    }
    *out_len = len;
    return data;
}

/* Décode `len` octets LZW en exactement `pixel_count` index ; rend le
 * nombre de codes clear rencontrés après le premier. */
static int lzw_decode(const unsigned char *data, long len, int min_code_size, unsigned char *out, long pixel_count) {
    static int prefix[4096];
    static unsigned char suffix[4096];
    static unsigned char stack[4096];
    const int clear = 1 << min_code_size;
    const int eoi = clear + 1;
    int width = min_code_size + 1;
    int next = eoi + 1;
    int prev = -1;
    int clears = 0;
    int seen_first_clear = 0;
    long bitpos = 0, written = 0;
    for (int i = 0; i < clear; i++) {
        prefix[i] = -1;
        suffix[i] = (unsigned char)i;
    }
    for (;;) {
        assert(bitpos + width <= len * 8);
        int code = 0;
        for (int i = 0; i < width; i++, bitpos++) {
            code |= ((data[bitpos >> 3] >> (bitpos & 7)) & 1) << i;
        }
        if (code == clear) {
            clears += seen_first_clear;
            seen_first_clear = 1;
            width = min_code_size + 1;
            next = eoi + 1;
            prev = -1;
            continue;
        }
        if (code == eoi) {
            break;
        }
        assert(code <= next);
        int sp = 0;
        int c = code == next ? prev : code;
        assert(c >= 0);
        while (c >= clear) {
            stack[sp++] = suffix[c];
            c = prefix[c];
        }
        unsigned char first = (unsigned char)c;
        stack[sp++] = first;
        if (code == next) {
            /* KwKwK : la chaîne de prev suivie de son propre premier octet. */
            memmove(stack + 1, stack, (size_t)sp);
            stack[0] = first;
            sp++;
        }
        while (sp > 0) {
            assert(written < pixel_count);
            out[written++] = stack[--sp];
        }
        if (prev >= 0 && next < 4096) {
            prefix[next] = prev;
            suffix[next] = first;
            next++;
            if (next == (1 << width) && width < 12) {
                width++;
            }
        }
        prev = code;
    }
    assert(written == pixel_count);
    return clears;
}

typedef struct {
    int width;
    int height;
    int loops_forever;
    int frame_count;
    int clears;
    int delays_ok;
    unsigned char *pixels; /* frame_count images DST x DST */
} gif_t;

static gif_t parse_gif(FILE *file) {
    gif_t g;
    memset(&g, 0, sizeof g);
    g.delays_ok = 1;
    buf_t b = slurp(file);
    assert(b.size > 6 && memcmp(b.bytes, "GIF89a", 6) == 0);
    b.pos = 6;
    g.width = u16(&b);
    g.height = u16(&b);
    int packed = u8(&b);
    assert(packed & 0x80);
    assert((2 << (packed & 7)) == P8_PALETTE_COLORS);
    u8(&b);
    u8(&b);
    for (int i = 0; i < P8_PALETTE_COLORS; i++) {
        assert(u8(&b) == P8_PALETTE_RGB[i].red);
        assert(u8(&b) == P8_PALETTE_RGB[i].green);
        assert(u8(&b) == P8_PALETTE_RGB[i].blue);
    }
    for (;;) {
        int tag = u8(&b);
        if (tag == 0x3B) {
            assert(b.pos == b.size);
            break;
        }
        if (tag == 0x21) {
            int label = u8(&b);
            if (label == 0xFF) {
                assert(u8(&b) == 11);
                g.loops_forever = memcmp(b.bytes + b.pos, "NETSCAPE2.0", 11) == 0;
                b.pos += 11;
                assert(u8(&b) == 3 && u8(&b) == 1);
                g.loops_forever = g.loops_forever && u16(&b) == 0;
                assert(u8(&b) == 0);
            } else if (label == 0xF9) {
                assert(u8(&b) == 4);
                u8(&b);
                g.delays_ok = g.delays_ok && u16(&b) == 3;
                u8(&b);
                assert(u8(&b) == 0);
            } else {
                assert(!"extension inattendue");
            }
            continue;
        }
        assert(tag == 0x2C);
        assert(u16(&b) == 0 && u16(&b) == 0);
        assert(u16(&b) == g.width && u16(&b) == g.height);
        assert(u8(&b) == 0);
        int min_code_size = u8(&b);
        assert(min_code_size == 4);
        long len;
        unsigned char *data = read_sub_blocks(&b, &len);
        long n = (long)g.width * g.height;
        g.pixels = realloc(g.pixels, (size_t)(n * (g.frame_count + 1)));
        assert(g.pixels != NULL);
        g.clears += lzw_decode(data, len, min_code_size, g.pixels + n * g.frame_count, n);
        g.frame_count++;
        free(data);
    }
    free(b.bytes);
    return g;
}

static unsigned char dst_pixel(const gif_t *g, int frame, int x, int y) {
    return g->pixels[(long)frame * DST * DST + (long)y * DST + x];
}

/* Chaque pixel source devient un carré SCALE x SCALE de sa couleur. */
static void assert_scaled(const gif_t *g, int frame, const unsigned char *src) {
    for (int y = 0; y < DST; y++) {
        for (int x = 0; x < DST; x++) {
            assert(dst_pixel(g, frame, x, y) == src[(y / SCALE) * SRC + x / SCALE]);
        }
    }
}

static gif_t encode(const unsigned char *frames, int n, const p8_input_frame_t *events) {
    FILE *file = tmpfile();
    assert(file != NULL);
    pico8_gif_writer_t writer;
    assert(pico8_gif_open(&writer, file));
    for (int i = 0; i < n; i++) {
        assert(pico8_gif_write_frame(&writer, frames + (long)i * SRC * SRC, events ? &events[i] : NULL));
    }
    assert(pico8_gif_close(&writer));
    gif_t g = parse_gif(file);
    fclose(file);
    return g;
}

static void test_frames_round_trip_scaled_by_three(void) {
    static unsigned char frames[2 * SRC * SRC];
    for (int i = 0; i < SRC * SRC; i++) {
        frames[i] = (unsigned char)((i / SRC / 8 + i % SRC / 8) % 16);
        frames[SRC * SRC + i] = (unsigned char)(i % SRC < 64 ? 1 : 12);
    }
    gif_t g = encode(frames, 2, NULL);
    assert(g.width == DST && g.height == DST);
    assert(g.loops_forever);
    assert(g.delays_ok);
    assert(g.frame_count == 2);
    assert_scaled(&g, 0, frames);
    assert_scaled(&g, 1, frames + SRC * SRC);
    free(g.pixels);
}

/* Anneau de rayon 2 en couleur 7 sans bouton, de rayon 3 en couleur 8 avec
 * un bouton ; le centre reste le pixel du jeu. */
static void test_mouse_marker_overlays_the_event(void) {
    static unsigned char frames[2 * SRC * SRC];
    static unsigned char expected[2 * SRC * SRC];
    memset(frames, 1, sizeof frames);
    p8_input_frame_t events[2];
    memset(events, 0, sizeof events);
    events[0].mouse_x = 10;
    events[0].mouse_y = 20;
    events[1].mouse_x = 50;
    events[1].mouse_y = 60;
    events[1].mouse_buttons = 1;
    gif_t g = encode(frames, 2, events);
    assert(g.frame_count == 2);
    memcpy(expected, frames, sizeof expected);
    unsigned char *e0 = expected, *e1 = expected + SRC * SRC;
    e0[20 * SRC + 12] = 7;
    e0[20 * SRC + 8] = 7;
    e0[22 * SRC + 10] = 7;
    e0[18 * SRC + 10] = 7;
    e0[21 * SRC + 11] = 7;
    e0[19 * SRC + 9] = 7;
    e0[21 * SRC + 12] = 7;
    e1[60 * SRC + 53] = 8;
    e1[62 * SRC + 52] = 8;
    e1[57 * SRC + 50] = 8;
    for (int y = 0; y < DST; y++) {
        for (int x = 0; x < DST; x++) {
            int sx = x / SCALE, sy = y / SCALE;
            unsigned char got0 = dst_pixel(&g, 0, x, y);
            unsigned char got1 = dst_pixel(&g, 1, x, y);
            if (e0[sy * SRC + sx] != 1) {
                assert(got0 == e0[sy * SRC + sx]);
            }
            if (e1[sy * SRC + sx] != 1) {
                assert(got1 == e1[sy * SRC + sx]);
            }
        }
    }
    assert(dst_pixel(&g, 0, 10 * SCALE, 20 * SCALE) == 1);
    assert(dst_pixel(&g, 0, 13 * SCALE, 20 * SCALE) == 1);
    assert(dst_pixel(&g, 1, 52 * SCALE, 60 * SCALE) == 1);
    assert(dst_pixel(&g, 1, 50 * SCALE, 60 * SCALE) == 1);
    free(g.pixels);
}

/* Une souris hors écran ne déborde pas du tampon : seuls les pixels de
 * l'anneau dans l'écran changent. */
static void test_mouse_marker_is_clipped_at_the_edge(void) {
    static unsigned char frames[SRC * SRC];
    memset(frames, 3, sizeof frames);
    p8_input_frame_t event;
    memset(&event, 0, sizeof event);
    event.mouse_x = 127;
    event.mouse_y = 0;
    gif_t g = encode(frames, 1, &event);
    assert(dst_pixel(&g, 0, 125 * SCALE, 0) == 7);
    assert(dst_pixel(&g, 0, 127 * SCALE, 2 * SCALE) == 7);
    free(g.pixels);
}

/* Une frame de bruit remplit la table des 4096 codes : l'encodeur doit
 * émettre un clear et repartir à 5 bits, sinon le décodage diverge. */
static void test_noise_fills_the_table_and_emits_clear(void) {
    static unsigned char frames[SRC * SRC];
    unsigned int seed = 12345;
    for (int i = 0; i < SRC * SRC; i++) {
        seed = seed * 1103515245u + 12345u;
        frames[i] = (unsigned char)((seed >> 16) & 15);
    }
    gif_t g = encode(frames, 1, NULL);
    assert(g.frame_count == 1);
    assert(g.clears > 0);
    assert_scaled(&g, 0, frames);
    free(g.pixels);
}

int main(void) {
    test_frames_round_trip_scaled_by_three();
    test_mouse_marker_overlays_the_event();
    test_mouse_marker_is_clipped_at_the_edge();
    test_noise_fills_the_table_and_emits_clear();
    printf("test_pico8_gif: ok\n");
    return 0;
}
