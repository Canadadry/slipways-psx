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
 * existed -- tests/test_headless_capture.c asserts that.
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

#include "p8/mem.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/cheat.h"
#include "devtools/dump.h"
#include "devtools/screencast.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "game/ui_zone.h"
#include "game/particle_zone.h"
#include "engine/game_zone.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const char *const SORTE_NAMES[PICO8_SORTE_COUNT] = {"button", "modal_backdrop", "message_box", "toast", "rangee", "le"};
static const char *const PARTICULES_NAMES[PICO8_PARTICULES_COUNT] = {"particule", "qy", "qy_fv", "burst", "ts"};

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

    pico8_mem_t mem;
    pico8_mem_init(&mem);

    pico8_gfx_t gfx;
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, events_path));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    /* Un echec tue le processus : une capture absente ou tronquee
     * produirait une empreinte fausse, indiscernable d'une regression.
     * fprintf plutot qu'un assert, pour que le message nomme le chemin. */
    pico8_screencast_writer_t capture = { NULL };
    if (capture_path && !pico8_screencast_writer_open(&capture, capture_path)) {
        fprintf(stderr, "pico8 replay: impossible d'ouvrir le fichier de capture '%s'\n",
                capture_path);
        return 1;
    }

    int frame_count = input.frame_count;
    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (int i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();
        /* APRES pico8__draw() et avant la frame suivante : c'est le seul
         * instant ou la memoire ecran tient la frame i complete. En tete
         * de boucle, la capture entiere serait decalee d'une frame --
         * tests/test_headless_capture.c le detecte en comparant la
         * derniere frame capturee au dump texte. */
        if (capture_path && !pico8_screencast_write_frame(&capture, &mem)) {
            fprintf(stderr, "pico8 replay: echec d'ecriture de la frame %d dans '%s'\n",
                    i, capture_path);
            pico8_screencast_writer_close(&capture);
            return 1;
        }
        if (i < frame_count - 1) {
            pico8_input_step(&input);
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
    int dump_ok = pico8_dump_screen(&mem, dump_out);
    fclose(dump_out);
    assert(dump_ok);

    const char *remplissage_path = getenv("PICO8_REMPLISSAGE");
    if (remplissage_path != NULL) {
        write_remplissage(remplissage_path);
    }

    pico8_input_free(&input);

    printf("pico8 replay: %d frame(s) run against %s; framebuffer dumped to %s\n",
           frame_count, events_path, dump_path);
    if (capture_path) {
        printf("pico8 replay: %d frame(s) captured to %s\n", frame_count, capture_path);
    }
    return 0;
}
