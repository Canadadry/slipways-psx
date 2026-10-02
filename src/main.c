/* The headless replay driver: loads the cart's assets, boots the cart, steps it
 * one frame per events.json entry, and dumps the final framebuffer as text.
 * It asserts nothing about the game itself -- that is
 * what tests/ is for.
 *
 * argv[1], required, names the script to replay; without it the driver
 * prints its usage and exits 2.
 *
 * argv[2], optional, is where each frame's pixels are written. Absent means no
 * capture, and the driver then behaves exactly as it did before capture
 * existed -- tests/test_pico8_headless_capture.c asserts that.
 *
 * argv[3], optional, is where the text dump goes; absent means
 * tmp/pico8_replay_dump.txt. Giving it requires giving argv[2].
 *
 * Every replay starts on the title screen: the file itself clicks the
 * difficulty. The driver takes no option; an argument starting with '-'
 * gets the usage and exit 2.
 *
 * PICO8_REMPLISSAGE=<file>, optional, is where the live places of each
 * interface sort, then of each particle sort, are written after the last
 * frame, one "<sort> <count>" line each, read after flushing the exit
 * queue; then the session's maxima of each particle sort, one
 * "max_vivantes|max_octets|max_elements <sort> <n>" line each; then the
 * bytes taken in the game zone, "partie <n>", and their session maximum,
 * "max_octets partie <n>"; then the same two for the loading sub-zone,
 * "chargement <n>" and "max_octets chargement <n>". Capture, dump and exit
 * code stay the same. */

#include "pico8_dump.h"
#include "pico8_rejeu.h"
#include "pico8_screencast.h"
#include "pico8_cart.h"
#include "pico8_zone_interface.h"
#include "pico8_zone_particules.h"
#include "pico8_zone_partie.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const char *const SORTE_NAMES[PICO8_SORTE_COUNT] = {"js", "fq", "mh", "iv", "rangee", "le"};
static const char *const PARTICULES_NAMES[PICO8_PARTICULES_COUNT] = {"particule", "qy", "qy_fv", "ga", "ts"};

static void write_remplissage(const char *path) {
    FILE *out = fopen(path, "w");
    if (out == NULL) {
        fprintf(stderr, "pico8 replay: impossible d'ouvrir le fichier de remplissage '%s'\n", path);
        return;
    }
    pico8_cart_vider_file_sortie();
    for (int s = 0; s < PICO8_SORTE_COUNT; s++) {
        fprintf(out, "%s %d\n", SORTE_NAMES[s], pico8_zone_interface_vivantes((pico8_sorte_t)s));
    }
    for (int s = 0; s < PICO8_PARTICULES_COUNT; s++) {
        fprintf(out, "%s %d\n", PARTICULES_NAMES[s], pico8_zone_particules_vivantes((pico8_particules_sorte_t)s));
    }
    for (int s = 0; s < PICO8_PARTICULES_COUNT; s++) {
        fprintf(out, "max_vivantes %s %d\n", PARTICULES_NAMES[s],
                pico8_zone_particules_max_vivantes((pico8_particules_sorte_t)s));
    }
    for (int s = PICO8_PARTICULES_QY; s < PICO8_PARTICULES_COUNT; s++) {
        fprintf(out, "max_octets %s %lu\n", PARTICULES_NAMES[s],
                (unsigned long)pico8_zone_particules_max_octets((pico8_particules_sorte_t)s));
    }
    for (int s = PICO8_PARTICULES_QY; s < PICO8_PARTICULES_COUNT; s++) {
        if (s == PICO8_PARTICULES_GA) continue;
        fprintf(out, "max_elements %s %d\n", PARTICULES_NAMES[s],
                pico8_zone_particules_max_elements((pico8_particules_sorte_t)s));
    }
    fprintf(out, "partie %lu\n", (unsigned long)pico8_zone_partie_prise());
    fprintf(out, "max_octets partie %lu\n", (unsigned long)pico8_zone_partie_max());
    fprintf(out, "chargement %lu\n", (unsigned long)pico8_zone_chargement_prise());
    fprintf(out, "max_octets chargement %lu\n", (unsigned long)pico8_zone_chargement_max());
    fclose(out);
}

int main(int argc, char **argv) {
    if (argc < 2 || argv[1][0] == '-') {
        fprintf(stderr, "usage: pico8_replay <events.json> [<capture.bin> [<dump.txt>]]\n");
        return 2;
    }
    const char *events_path = argv[1];
    const char *capture_path = (argc > 2) ? argv[2] : NULL;
    const char *dump_path = (argc > 3) ? argv[3] : "tmp/pico8_replay_dump.txt";

    static pico8_rejeu_t rejeu;
    pico8_rejeu_ouvrir(&rejeu, events_path);

    /* Un echec tue le processus : une capture absente ou tronquee
     * produirait une empreinte fausse, indiscernable d'une regression.
     * fprintf plutot qu'un assert, pour que le message nomme le chemin. */
    pico8_screencast_writer_t capture = { NULL };
    if (capture_path && !pico8_screencast_writer_open(&capture, capture_path)) {
        fprintf(stderr, "pico8 replay: impossible d'ouvrir le fichier de capture '%s'\n",
                capture_path);
        return 1;
    }

    int frame_count = pico8_rejeu_frames(&rejeu);
    for (int i = 0; i < frame_count; i++) {
        pico8_rejeu_jouer_frame(&rejeu, i);
        /* APRES la frame i et avant la suivante : c'est le seul
         * instant ou la memoire ecran tient la frame i complete. En tete
         * de boucle, la capture entiere serait decalee d'une frame --
         * tests/test_pico8_headless_capture.c le detecte en comparant la
         * derniere frame capturee au dump texte. */
        if (capture_path && !pico8_screencast_write_frame(&capture, &rejeu.mem)) {
            fprintf(stderr, "pico8 replay: echec d'ecriture de la frame %d dans '%s'\n",
                    i, capture_path);
            pico8_screencast_writer_close(&capture);
            return 1;
        }
    }

    /* La fermeture est verifiee au meme titre que les ecritures : fclose
     * est le dernier endroit ou une ecriture tamponnee peut echouer.
     * Elle a lieu AVANT le dump texte, et un echec renonce donc a
     * l'ecrire : un dump frais ferait passer pour
     * normale une execution dont la capture est perdue. */
    if (capture_path && !pico8_screencast_writer_close(&capture)) {
        fprintf(stderr, "pico8 replay: echec de fermeture du fichier de capture '%s'\n",
                capture_path);
        return 1;
    }

    FILE *dump_out = fopen(dump_path, "w");
    assert(dump_out != NULL);
    int dump_ok = pico8_dump_screen(&rejeu.mem, dump_out);
    fclose(dump_out);
    assert(dump_ok);

    const char *remplissage_path = getenv("PICO8_REMPLISSAGE");
    if (remplissage_path != NULL) {
        write_remplissage(remplissage_path);
    }

    pico8_rejeu_fermer(&rejeu);

    printf("pico8 replay: %d frame(s) run against %s; framebuffer dumped to %s\n",
           frame_count, events_path, dump_path);
    if (capture_path) {
        printf("pico8 replay: %d frame(s) captured to %s\n", frame_count, capture_path);
    }
    return 0;
}
