/* Host tool of make gif:
 *   gif <capture.bin> <events.json> <output.gif>
 * Each frame of the capture gets the mouse of the event at the same index;
 * past the last event, no mouse. On failure: non-zero exit code, message
 * on stderr. */

#include "devtools/gif.h"
#include "p8/input.h"
#include "devtools/screencast.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: gif <capture.bin> <events.json> <output.gif>\n");
        return 2;
    }
    screencast_reader_t reader;
    if (screencast_reader_open(&reader, argv[1]) != SCREENCAST_OK) {
        fprintf(stderr, "gif: capture %s unreadable, empty or truncated\n", argv[1]);
        return 1;
    }
    p8_input_t input;
    p8_input_init(&input);
    if (!p8_input_load(&input, argv[2])) {
        fprintf(stderr, "gif: %s missing or malformed\n", argv[2]);
        screencast_reader_close(&reader);
        return 1;
    }
    FILE *out = fopen(argv[3], "wb");
    int ok = out != NULL;
    gif_writer_t writer;
    ok = ok && gif_open(&writer, out);
    static unsigned char pixels[SCREENCAST_FRAME_PIXELS];
    int frames = screencast_frame_count(&reader);
    for (int i = 0; ok && i < frames; i++) {
        ok = screencast_read_frame(&reader, i, pixels)
            && gif_write_frame(&writer, pixels, i < input.frame_count ? &input.frames[i] : NULL);
    }
    ok = ok && gif_close(&writer);
    if (out != NULL && fclose(out) != 0) {
        ok = 0;
    }
    if (!ok) {
        fprintf(stderr, "gif: cannot write %s\n", argv[3]);
    }
    p8_input_free(&input);
    screencast_reader_close(&reader);
    return !ok;
}
