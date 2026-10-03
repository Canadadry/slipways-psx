#include "devtools/screencast.h"

#include <assert.h>
#include <stddef.h>

/* 8192 is a constant of the published format, but the macro derives from
 * the screen size: this compile-time assertion ties the two, so that a
 * change of screen size breaks the build here instead of silently
 * changing the format. */
typedef char screencast_frame_is_8192_bytes_t[(SCREENCAST_FRAME_BYTES == 8192) ? 1 : -1];

int screencast_writer_open(screencast_writer_t *writer, const char *path) {
    assert(writer);
    assert(path);

    /* "wb" and never "ab": two concatenated captures would be a game
     * nobody played, and their size would still be a multiple of 8192, so
     * no validation would catch them. */
    writer->file = fopen(path, "wb");
    return writer->file != NULL;
}

int screencast_write_frame(screencast_writer_t *writer, const p8_vram_t *mem) {
    unsigned char packed[SCREENCAST_FRAME_BYTES];

    assert(writer);
    assert(writer->file);
    assert(mem);

    int i = 0;
    for (int y = 0; y < P8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < P8_SCREEN_WIDTH; x += 2) {
            unsigned char even = mem->screen[y][x];
            unsigned char odd = mem->screen[y][x + 1];
            /* A byte above 0x0f would spill into the neighbouring nibble and
             * corrupt the pixel next to it, which would only show later, as a
             * rendering divergence. */
            assert(even <= 0x0f);
            assert(odd <= 0x0f);
            packed[i++] = (unsigned char)(even | (odd << 4));
        }
    }

    return fwrite(packed, 1, sizeof(packed), writer->file) == sizeof(packed);
}

int screencast_writer_close(screencast_writer_t *writer) {
    assert(writer);

    if (!writer->file) {
        return 1;
    }
    int ok = fclose(writer->file) == 0;
    writer->file = NULL;
    return ok;
}

screencast_status_t screencast_reader_open(screencast_reader_t *reader, const char *path) {
    assert(reader);
    assert(path);

    reader->file = NULL;
    reader->frame_count = 0;

    FILE *file = fopen(path, "rb");
    if (!file) {
        return SCREENCAST_UNREADABLE;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return SCREENCAST_UNREADABLE;
    }
    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return SCREENCAST_UNREADABLE;
    }
    if (size == 0) {
        fclose(file);
        return SCREENCAST_EMPTY;
    }

    if (size % SCREENCAST_FRAME_BYTES != 0) {
        fclose(file);
        return SCREENCAST_TRUNCATED;
    }

    reader->file = file;
    reader->frame_count = (int)(size / SCREENCAST_FRAME_BYTES);
    return SCREENCAST_OK;
}

int screencast_frame_count(const screencast_reader_t *reader) {
    assert(reader);

    return reader->frame_count;
}

int screencast_read_frame(screencast_reader_t *reader, int index, unsigned char *out_pixels) {
    unsigned char packed[SCREENCAST_FRAME_BYTES];

    assert(reader);
    assert(reader->file);
    assert(out_pixels);

    if (index < 0 || index >= reader->frame_count) {
        return 0;
    }

    /* The cast to long comes before the multiplication, and must stay
     * there: with a 32-bit `int`, index x 8192 overflows past 262144
     * frames (72 minutes at 60 Hz), well before fseek's own limit. */
    if (fseek(reader->file, (long)index * SCREENCAST_FRAME_BYTES, SEEK_SET) != 0) {
        return 0;
    }
    if (fread(packed, 1, sizeof(packed), reader->file) != sizeof(packed)) {
        return 0;
    }

    for (int i = 0; i < SCREENCAST_FRAME_BYTES; i++) {
        out_pixels[i * 2] = (unsigned char)(packed[i] & 0x0f);
        out_pixels[i * 2 + 1] = (unsigned char)((packed[i] >> 4) & 0x0f);
    }

    return 1;
}

void screencast_reader_close(screencast_reader_t *reader) {
    assert(reader);

    if (reader->file) {
        fclose(reader->file);
        reader->file = NULL;
    }
    reader->frame_count = 0;
}
