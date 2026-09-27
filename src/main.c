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
 * --from-title, before argv[1], replays a raylib recording: no difficulty
 * is picked before frame 0. */

#include "pico8_mem.h"
#include "pico8_assets.h"
#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_cheat.h"
#include "pico8_dump.h"
#include "pico8_screencast.h"
#include "pico8_dispatch_context.h"
#include "pico8_cart.h"
#include "pico8_help.h"
#include "pico8_ui_button.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    int from_title = argc > 1 && strcmp(argv[1], "--from-title") == 0;
    if (from_title) {
        argv++;
        argc--;
    }
    if (argc < 2) {
        fprintf(stderr,
                "usage: pico8_replay [--from-title] <events.json> [<capture.bin> [<dump.txt>]]\n");
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

    /* Entry 2 of bo() is the difficulty every scripted replay starts on; the
     * driver picks it for the player instead of waiting for a title-screen
     * click. A raylib recording holds that click itself: --from-title. */
    if (!from_title) {
        pico8_value_t difficulty_entry;
        assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
        assert(difficulty_entry.kind == PICO8_VALUE_REF);
        pico8_instance_t *difficulty_button =
            pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref);
        pico8_eu(difficulty_button);
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
         * tests/test_pico8_headless_capture.c le detecte en comparant la
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

    pico8_input_free(&input);

    printf("pico8 replay: %d frame(s) run against %s; framebuffer dumped to %s\n",
           frame_count, events_path, dump_path);
    if (capture_path) {
        printf("pico8 replay: %d frame(s) captured to %s\n", frame_count, capture_path);
    }
    return 0;
}
