#include "devtools/gif.h"

#include "p8/gfx.h"
#include "p8/palette.h"

#include <string.h>

#define SIDE (P8_SCREEN_WIDTH * GIF_SCALE)
#define MIN_CODE_SIZE 4
#define CLEAR_CODE (1 << MIN_CODE_SIZE)
#define EOI_CODE (CLEAR_CODE + 1)
#define MAX_CODES 4096

static void put(gif_writer_t *w, const void *bytes, size_t n) {
    if (w->succeeded && fwrite(bytes, 1, n, w->file) != n) {
        w->succeeded = 0;
    }
}

static void put_byte(gif_writer_t *w, int b) {
    unsigned char c = (unsigned char)b;
    put(w, &c, 1);
}

static void put_u16(gif_writer_t *w, int v) {
    put_byte(w, v & 0xFF);
    put_byte(w, (v >> 8) & 0xFF);
}

int gif_open(gif_writer_t *writer, FILE *file) {
    writer->file = file;
    writer->succeeded = file != NULL;
    put(writer, "GIF89a", 6);
    put_u16(writer, SIDE);
    put_u16(writer, SIDE);
    /* Global color table present, 8-bit resolution, 2^(3+1) = 16 colors. */
    put_byte(writer, 0xF3);
    put_byte(writer, 0);
    put_byte(writer, 0);
    for (int i = 0; i < P8_PALETTE_COLORS; i++) {
        put_byte(writer, P8_PALETTE_RGB[i].red);
        put_byte(writer, P8_PALETTE_RGB[i].green);
        put_byte(writer, P8_PALETTE_RGB[i].blue);
    }
    put_byte(writer, 0x21);
    put_byte(writer, 0xFF);
    put_byte(writer, 11);
    put(writer, "NETSCAPE2.0", 11);
    put_byte(writer, 3);
    put_byte(writer, 1);
    put_u16(writer, 0);
    put_byte(writer, 0);
    return writer->succeeded;
}

typedef struct {
    gif_writer_t *writer;
    unsigned char block[255];
    int block_len;
    unsigned long bits;
    int bit_count;
} bit_sink_t;

static void sink_byte(bit_sink_t *s, unsigned char b) {
    s->block[s->block_len++] = b;
    if (s->block_len == 255) {
        put_byte(s->writer, 255);
        put(s->writer, s->block, 255);
        s->block_len = 0;
    }
}

static void sink_code(bit_sink_t *s, int code, int width) {
    s->bits |= (unsigned long)code << s->bit_count;
    s->bit_count += width;
    while (s->bit_count >= 8) {
        sink_byte(s, (unsigned char)(s->bits & 0xFF));
        s->bits >>= 8;
        s->bit_count -= 8;
    }
}

static void sink_flush(bit_sink_t *s) {
    if (s->bit_count > 0) {
        sink_byte(s, (unsigned char)(s->bits & 0xFF));
    }
    if (s->block_len > 0) {
        put_byte(s->writer, s->block_len);
        put(s->writer, s->block, (size_t)s->block_len);
    }
    put_byte(s->writer, 0);
}

/* Indexed by a string's code and the color index after it: the code of the
 * longer string, 0 if absent (0 is never a child code: they start at 18). */
static unsigned short child[MAX_CODES][P8_PALETTE_COLORS];

static void lzw_encode(gif_writer_t *w, const unsigned char *pixels, long count) {
    bit_sink_t s;
    memset(&s, 0, sizeof s);
    s.writer = w;
    int width = MIN_CODE_SIZE + 1;
    int next = EOI_CODE + 1;
    memset(child, 0, sizeof child);
    sink_code(&s, CLEAR_CODE, width);
    int prefix = pixels[0];
    for (long i = 1; i < count; i++) {
        int c = pixels[i];
        int found = child[prefix][c];
        if (found != 0) {
            prefix = found;
            continue;
        }
        sink_code(&s, prefix, width);
        if (next < MAX_CODES) {
            child[prefix][c] = (unsigned short)next;
            /* The decoder adds this entry one code later: it moves to
             * width+1 bits just in time to read the first code that
             * could refer to it. */
            if (next == (1 << width) && width < 12) {
                width++;
            }
            next++;
        } else {
            sink_code(&s, CLEAR_CODE, width);
            memset(child, 0, sizeof child);
            width = MIN_CODE_SIZE + 1;
            next = EOI_CODE + 1;
        }
        prefix = c;
    }
    sink_code(&s, prefix, width);
    sink_code(&s, EOI_CODE, width);
    sink_flush(&s);
}

static void draw_ring(unsigned char *screen, int cx, int cy, int radius, unsigned char color) {
    int lo = radius * radius - radius, hi = radius * radius + radius;
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int d2 = dx * dx + dy * dy, x = cx + dx, y = cy + dy;
            if (d2 >= lo && d2 <= hi && x >= 0 && x < P8_SCREEN_WIDTH && y >= 0 && y < P8_SCREEN_HEIGHT) {
                screen[y * P8_SCREEN_WIDTH + x] = color;
            }
        }
    }
}

int gif_write_frame(gif_writer_t *writer, const unsigned char *pixels, const p8_input_frame_t *event) {
    static unsigned char screen[P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT];
    static unsigned char scaled[SIDE * SIDE];
    memcpy(screen, pixels, sizeof screen);
    if (event != NULL) {
        if (event->mouse_buttons != 0) {
            draw_ring(screen, event->mouse_x, event->mouse_y, 3, 8);
        } else {
            draw_ring(screen, event->mouse_x, event->mouse_y, 2, 7);
        }
    }
    for (int y = 0; y < SIDE; y++) {
        for (int x = 0; x < SIDE; x++) {
            scaled[y * SIDE + x] = screen[(y / GIF_SCALE) * P8_SCREEN_WIDTH + x / GIF_SCALE] & 15;
        }
    }
    /* Graphic Control Extension: no transparency, a delay of three hundredths of a second. */
    put_byte(writer, 0x21);
    put_byte(writer, 0xF9);
    put_byte(writer, 4);
    put_byte(writer, 0x04);
    put_u16(writer, 3);
    put_byte(writer, 0);
    put_byte(writer, 0);
    put_byte(writer, 0x2C);
    put_u16(writer, 0);
    put_u16(writer, 0);
    put_u16(writer, SIDE);
    put_u16(writer, SIDE);
    put_byte(writer, 0);
    put_byte(writer, MIN_CODE_SIZE);
    lzw_encode(writer, scaled, (long)SIDE * SIDE);
    return writer->succeeded;
}

int gif_close(gif_writer_t *writer) {
    put_byte(writer, 0x3B);
    return writer->succeeded;
}
