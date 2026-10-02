/* Outil d'hote sur une capture de pico8_replay :
 *   pico8_frames frame <capture> <N>  la frame N sur stdout, au format de pico8_dump_screen
 * Sur echec : code non nul, message sur stderr. */

#include "devtools/dump.h"
#include "devtools/screencast.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *status_text(screencast_status_t status) {
    switch (status) {
    case SCREENCAST_OK:
        return "ok";
    case SCREENCAST_UNREADABLE:
        return "illisible ou absente";
    case SCREENCAST_EMPTY:
        return "vide";
    case SCREENCAST_TRUNCATED:
        return "tronquee (taille non multiple d'une frame)";
    }
    return "etat inconnu";
}

static int open_capture(screencast_reader_t *reader, const char *path) {
    screencast_status_t status = screencast_reader_open(reader, path);
    if (status != SCREENCAST_OK) {
        fprintf(stderr, "pico8_frames: capture %s : %s\n", path, status_text(status));
        return 0;
    }
    return 1;
}

static int parse_index(const char *text, int *out_index) {
    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value < 0 || value > INT_MAX) {
        return 0;
    }
    *out_index = (int)value;
    return 1;
}

static int write_frame(const char *capture_path, const char *index_text) {
    int index;
    if (!parse_index(index_text, &index)) {
        fprintf(stderr, "pico8_frames: index de frame invalide : %s\n", index_text);
        return 0;
    }

    screencast_reader_t reader;
    if (!open_capture(&reader, capture_path)) {
        return 0;
    }

    static unsigned char pixels[SCREENCAST_FRAME_PIXELS];
    int count = screencast_frame_count(&reader);
    int read_ok = screencast_read_frame(&reader, index, pixels);
    screencast_reader_close(&reader);
    if (!read_ok) {
        fprintf(stderr, "pico8_frames: capture %s : pas de frame %d (%d frame(s), index 0 a %d)\n",
                capture_path, index, count, count - 1);
        return 0;
    }

    if (!dump_pixels(pixels, stdout) || fflush(stdout) != 0) {
        fprintf(stderr, "pico8_frames: ecriture de la frame %d impossible\n", index);
        return 0;
    }
    return 1;
}

static int usage(void) {
    fprintf(stderr, "usage: pico8_frames frame <capture> <N>\n");
    return 2;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        return usage();
    }
    if (strcmp(argv[1], "frame") == 0) {
        return write_frame(argv[2], argv[3]) ? 0 : 1;
    }
    return usage();
}
