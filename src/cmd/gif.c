/* Outil d'hôte de make gif :
 *   pico8_gif <capture.bin> <events.json> <sortie.gif>
 * La frame n de la capture reçoit la souris de l'event n ; au-delà du
 * dernier event, pas de souris. Sur échec : code non nul, message sur
 * stderr. */

#include "devtools/gif.h"
#include "p8/input.h"
#include "devtools/screencast.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: pico8_gif <capture.bin> <events.json> <sortie.gif>\n");
        return 2;
    }
    pico8_screencast_reader_t reader;
    if (pico8_screencast_reader_open(&reader, argv[1]) != PICO8_SCREENCAST_OK) {
        fprintf(stderr, "pico8_gif: capture %s illisible, vide ou tronquée\n", argv[1]);
        return 1;
    }
    pico8_input_t input;
    pico8_input_init(&input);
    if (!pico8_input_load(&input, argv[2])) {
        fprintf(stderr, "pico8_gif: %s absent ou malformé\n", argv[2]);
        pico8_screencast_reader_close(&reader);
        return 1;
    }
    FILE *out = fopen(argv[3], "wb");
    int ok = out != NULL;
    pico8_gif_writer_t writer;
    ok = ok && pico8_gif_open(&writer, out);
    static unsigned char pixels[PICO8_SCREENCAST_FRAME_PIXELS];
    int frames = pico8_screencast_frame_count(&reader);
    for (int i = 0; ok && i < frames; i++) {
        ok = pico8_screencast_read_frame(&reader, i, pixels)
            && pico8_gif_write_frame(&writer, pixels, i < input.frame_count ? &input.frames[i] : NULL);
    }
    ok = ok && pico8_gif_close(&writer);
    if (out != NULL && fclose(out) != 0) {
        ok = 0;
    }
    if (!ok) {
        fprintf(stderr, "pico8_gif: écriture de %s impossible\n", argv[3]);
    }
    pico8_input_free(&input);
    pico8_screencast_reader_close(&reader);
    return !ok;
}
