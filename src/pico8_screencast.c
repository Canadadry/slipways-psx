/* Voir pico8_screencast.h pour ce qu'est ce module et pourquoi il
 * existe hors des modules raylib. */

#include "pico8_screencast.h"

#include <assert.h>
#include <stddef.h>

/* Le PRD fixe 8 192 octets par frame comme une constante du format, pas
 * comme une consequence des dimensions de l'ecran : c'est ce nombre
 * qu'un script jetable ecrira en dur pour lire le fichier, et c'est
 * celui dont depend "la frame N est a l'offset N x 8192". La constante
 * est neanmoins derivee de PICO8_SCREEN_WIDTH/HEIGHT pour ne pas avoir
 * deux verites ; cette assertion de compilation relie les deux, de
 * sorte qu'un changement des dimensions de l'ecran casse la
 * construction ici au lieu de changer le format en silence. */
typedef char pico8_screencast_frame_is_8192_bytes[(PICO8_SCREENCAST_FRAME_BYTES == 8192) ? 1 : -1];

int pico8_screencast_writer_open(pico8_screencast_writer_t *writer, const char *path) {
    assert(writer);
    assert(path);

    /* "wb" : un fichier existant est tronque, jamais complete. Deux
     * sessions concatenees seraient la capture d'une partie que
     * personne n'a jouee, et sa taille resterait un multiple de 8 192,
     * donc aucune validation ne la rattraperait. */
    writer->file = fopen(path, "wb");
    return writer->file != NULL;
}

int pico8_screencast_write_frame(pico8_screencast_writer_t *writer, const pico8_mem_t *mem) {
    unsigned char packed[PICO8_SCREENCAST_FRAME_BYTES];

    assert(writer);
    assert(writer->file);
    assert(mem);

    /* L'empaquetage passe par un tampon local puis un seul fwrite,
     * plutot que par 8 192 fputc : la capture s'execute une fois par
     * frame de rejeu (9 729 fois sur l'events.json du depot, ~1,5 s au
     * total d'apres la mesure du PRD), et ce temps s'ajoute a chaque
     * `make test` pendant le chantier que cet instrument sert. Le
     * tampon fait 8 Ko sur la pile, ce que la pile d'un pilote headless
     * absorbe sans difficulte. */
    for (int i = 0; i < PICO8_SCREENCAST_FRAME_BYTES; i++) {
        unsigned char even = pico8_peek(mem, PICO8_ADDR_SCREEN + i * 2);
        unsigned char odd = pico8_peek(mem, PICO8_ADDR_SCREEN + i * 2 + 1);
        /* Meme precondition que pico8_dump_screen : la memoire ecran
         * ne contient que des index de palette. Un octet hors bornes
         * deborderait sur le quartet voisin et corromprait le pixel
         * d'a cote, ce qui se lirait plus tard comme une divergence de
         * rendu -- donc on s'arrete ici, ou la cause est visible. */
        assert(even <= 0x0f);
        assert(odd <= 0x0f);
        packed[i] = (unsigned char)(even | (odd << 4));
    }

    return fwrite(packed, 1, sizeof(packed), writer->file) == sizeof(packed);
}

int pico8_screencast_writer_close(pico8_screencast_writer_t *writer) {
    assert(writer);

    if (!writer->file) {
        return 1;
    }
    int ok = fclose(writer->file) == 0;
    writer->file = NULL;
    return ok;
}

pico8_screencast_status_t pico8_screencast_reader_open(pico8_screencast_reader_t *reader, const char *path) {
    assert(reader);
    assert(path);

    reader->file = NULL;
    reader->frame_count = 0;

    FILE *file = fopen(path, "rb");
    if (!file) {
        return PICO8_SCREENCAST_UNREADABLE;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return PICO8_SCREENCAST_UNREADABLE;
    }
    /* La taille est lue sur le fichier lui-meme plutot que declaree
     * quelque part : c'est la seule reponse qui ne peut pas diverger de
     * ce qui a ete ecrit, et c'est ce qui rend la validation ci-dessous
     * capable de detecter une capture interrompue. */
    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return PICO8_SCREENCAST_UNREADABLE;
    }
    if (size == 0) {
        fclose(file);
        return PICO8_SCREENCAST_EMPTY;
    }

    if (size % PICO8_SCREENCAST_FRAME_BYTES != 0) {
        fclose(file);
        return PICO8_SCREENCAST_TRUNCATED;
    }

    reader->file = file;
    reader->frame_count = (int)(size / PICO8_SCREENCAST_FRAME_BYTES);
    return PICO8_SCREENCAST_OK;
}

int pico8_screencast_frame_count(const pico8_screencast_reader_t *reader) {
    assert(reader);

    return reader->frame_count;
}

int pico8_screencast_read_frame(pico8_screencast_reader_t *reader, int index, unsigned char *out_pixels) {
    unsigned char packed[PICO8_SCREENCAST_FRAME_BYTES];

    assert(reader);
    assert(reader->file);
    assert(out_pixels);

    /* La borne est verifiee ici plutot que laissee a fread : un fseek
     * au-dela de la fin est licite, et le refus dependrait alors d'une
     * lecture courte, c'est a dire d'un detail de la bibliotheque
     * standard plutot que d'une decision du module. */
    if (index < 0 || index >= reader->frame_count) {
        return 0;
    }

    /* Le cast en long precede la multiplication : sur un `int` de 32
     * bits, index x 8192 deborde des 262 144 frames, soit environ 72
     * minutes de jeu, bien avant la limite de fseek lui-meme. */
    if (fseek(reader->file, (long)index * PICO8_SCREENCAST_FRAME_BYTES, SEEK_SET) != 0) {
        return 0;
    }
    if (fread(packed, 1, sizeof(packed), reader->file) != sizeof(packed)) {
        return 0;
    }

    /* Le depaquetage a lieu ici et nulle part ailleurs : l'appelant
     * recoit un octet par pixel, la meme forme que la memoire ecran du
     * jeu, donc ni le visualiseur ni un futur comparateur n'ont a
     * connaitre les quartets. */
    for (int i = 0; i < PICO8_SCREENCAST_FRAME_BYTES; i++) {
        out_pixels[i * 2] = (unsigned char)(packed[i] & 0x0f);
        out_pixels[i * 2 + 1] = (unsigned char)((packed[i] >> 4) & 0x0f);
    }

    return 1;
}

void pico8_screencast_reader_close(pico8_screencast_reader_t *reader) {
    assert(reader);

    if (reader->file) {
        fclose(reader->file);
        reader->file = NULL;
    }
    reader->frame_count = 0;
}
