/* Outil d'hote sur une capture de pico8_replay :
 *   pico8_frames hashes <capture> <sortie>  une empreinte par frame, ligne 1 = frame 0
 *   pico8_frames frame <capture> <N>        la frame N sur stdout, au format de pico8_dump_screen
 * Sur echec : code non nul, message sur stderr, et aucun fichier <sortie>. */

#include "pico8_dump.h"
#include "pico8_frame_hash.h"
#include "pico8_screencast.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *status_text(pico8_screencast_status_t status) {
    switch (status) {
    case PICO8_SCREENCAST_OK:
        return "ok";
    case PICO8_SCREENCAST_UNREADABLE:
        return "illisible ou absente";
    case PICO8_SCREENCAST_EMPTY:
        return "vide";
    case PICO8_SCREENCAST_TRUNCATED:
        return "tronquee (taille non multiple d'une frame)";
    }
    return "etat inconnu";
}

static int open_capture(pico8_screencast_reader_t *reader, const char *path) {
    pico8_screencast_status_t status = pico8_screencast_reader_open(reader, path);
    if (status != PICO8_SCREENCAST_OK) {
        fprintf(stderr, "pico8_frames: capture %s : %s\n", path, status_text(status));
        return 0;
    }
    return 1;
}

static int write_hashes(const char *capture_path, const char *out_path) {
    pico8_screencast_reader_t reader;
    if (!open_capture(&reader, capture_path)) {
        return 0;
    }

    FILE *out = fopen(out_path, "w");
    if (!out) {
        fprintf(stderr, "pico8_frames: impossible d'ecrire %s\n", out_path);
        pico8_screencast_reader_close(&reader);
        return 0;
    }

    static unsigned char pixels[PICO8_SCREENCAST_FRAME_PIXELS];
    int ok = 1;
    int count = pico8_screencast_frame_count(&reader);
    for (int k = 0; k < count && ok; k++) {
        if (!pico8_screencast_read_frame(&reader, k, pixels)) {
            fprintf(stderr, "pico8_frames: capture %s : lecture de la frame %d impossible\n", capture_path, k);
            ok = 0;
        } else if (fprintf(out, "%016" PRIx64 "\n", pico8_frame_hash(pixels)) < 0) {
            fprintf(stderr, "pico8_frames: ecriture de %s impossible\n", out_path);
            ok = 0;
        }
    }
    pico8_screencast_reader_close(&reader);

    if (fclose(out) != 0 && ok) {
        fprintf(stderr, "pico8_frames: ecriture de %s impossible\n", out_path);
        ok = 0;
    }
    if (!ok) {
        remove(out_path);
    }
    return ok;
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

    pico8_screencast_reader_t reader;
    if (!open_capture(&reader, capture_path)) {
        return 0;
    }

    static unsigned char pixels[PICO8_SCREENCAST_FRAME_PIXELS];
    int count = pico8_screencast_frame_count(&reader);
    int read_ok = pico8_screencast_read_frame(&reader, index, pixels);
    pico8_screencast_reader_close(&reader);
    if (!read_ok) {
        fprintf(stderr, "pico8_frames: capture %s : pas de frame %d (%d frame(s), index 0 a %d)\n",
                capture_path, index, count, count - 1);
        return 0;
    }

    if (!pico8_dump_pixels(pixels, stdout) || fflush(stdout) != 0) {
        fprintf(stderr, "pico8_frames: ecriture de la frame %d impossible\n", index);
        return 0;
    }
    return 1;
}

static int usage(void) {
    fprintf(stderr,
            "usage: pico8_frames hashes <capture> <sortie>\n"
            "       pico8_frames frame <capture> <N>\n");
    return 2;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        return usage();
    }
    if (strcmp(argv[1], "hashes") == 0) {
        return write_hashes(argv[2], argv[3]) ? 0 : 1;
    }
    if (strcmp(argv[1], "frame") == 0) {
        return write_frame(argv[2], argv[3]) ? 0 : 1;
    }
    return usage();
}
