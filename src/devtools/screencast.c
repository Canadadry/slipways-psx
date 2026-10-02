#include "devtools/screencast.h"

#include <assert.h>
#include <stddef.h>

/* 8192 est une constante du format publie, mais la macro est derivee des
 * dimensions de l'ecran : cette assertion de compilation relie les deux,
 * de sorte qu'un changement de dimensions casse la construction ici au
 * lieu de changer le format en silence. */
typedef char screencast_frame_is_8192_bytes_t[(SCREENCAST_FRAME_BYTES == 8192) ? 1 : -1];

int screencast_writer_open(screencast_writer_t *writer, const char *path) {
    assert(writer);
    assert(path);

    /* "wb" et jamais "ab" : deux captures concatenees seraient une
     * partie que personne n'a jouee, et leur taille resterait un
     * multiple de 8192, donc aucune validation ne les rattraperait. */
    writer->file = fopen(path, "wb");
    return writer->file != NULL;
}

int screencast_write_frame(screencast_writer_t *writer, const p8_memory_t *mem) {
    unsigned char packed[SCREENCAST_FRAME_BYTES];

    assert(writer);
    assert(writer->file);
    assert(mem);

    for (int i = 0; i < SCREENCAST_FRAME_BYTES; i++) {
        unsigned char even = p8_peek(mem, P8_ADDR_SCREEN + i * 2);
        unsigned char odd = p8_peek(mem, P8_ADDR_SCREEN + i * 2 + 1);
        /* Un octet > 0x0f deborderait sur le quartet voisin et
         * corromprait le pixel d'a cote, ce qui ne se verrait que plus
         * tard, comme une divergence de rendu. */
        assert(even <= 0x0f);
        assert(odd <= 0x0f);
        packed[i] = (unsigned char)(even | (odd << 4));
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

    /* Le cast en long precede la multiplication, et doit y rester : sur
     * un `int` de 32 bits, index x 8192 deborde des 262144 frames (72
     * minutes a 60 Hz), bien avant la limite de fseek lui-meme. */
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
