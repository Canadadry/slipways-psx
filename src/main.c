/* The headless replay driver: loads the cart's assets, boots the cart, steps it
 * one frame per events.json entry, and dumps the final framebuffer as text to
 * tmp/pico8_replay_dump.txt. It asserts nothing about the game itself -- that is
 * what tests/ is for.
 *
 * argv[1] names the script to replay. The default, events.json at the repo
 * root, is a working recording the player's own session overwrites; it is
 * not a fixture, and nothing should be pinned to its contents. Tests use
 * their own files under tests/fixtures/.
 *
 * argv[2], optional, is where each frame's pixels are written. Absent means no
 * capture, and the driver then behaves exactly as it did before capture
 * existed -- tests/test_pico8_headless_capture.c asserts that. */

#include "pico8_mem.h"
#include "pico8_assets.h"
#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_dump.h"
#include "pico8_screencast.h"
#include "pico8_dispatch_context.h"
#include "pico8_cart.h"
#include "pico8_help.h"
#include "pico8_ui_button.h"

#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv) {
    const char *events_path = (argc > 1) ? argv[1] : "events.json";
    /* Le deuxieme argument, optionnel, est le chemin du fichier de
     * pixels. Absent == capture desactivee, ce qui est le defaut exige
     * par le PRD ("La capture est optionnelle et desactivee par
     * defaut"). Un drapeau nomme (--screencast=...) aurait demande a ce
     * pilote un analyseur d'arguments qu'il n'a pas : il lit argv[1] de
     * la meme facon depuis le chunk 1 de la phase 4, et src/main_raylib.c
     * a ajoute son propre argument optionnel positionnel a l'identique.
     * Deux conventions d'arguments dans deux pilotes du meme depot
     * n'auraient achete rien du tout. */
    const char *capture_path = (argc > 2) ? argv[2] : NULL;

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

    /* Ouverte avant la première frame, et un échec tue le processus : une capture
     * absente ou tronquée produirait une empreinte fausse, indiscernable d'une
     * régression. fprintf plutôt qu'un assert, pour que le message nomme le
     * chemin fautif -- c'est ce qu'un humain corrige. */
    pico8_screencast_writer_t capture = { NULL };
    if (capture_path && !pico8_screencast_writer_open(&capture, capture_path)) {
        fprintf(stderr, "pico8 replay: impossible d'ouvrir le fichier de capture '%s'\n",
                capture_path);
        return 1;
    }

    /* Entry 2 of bo() is the difficulty every replay starts on; the driver
     * picks it for the player instead of waiting for a title-screen click. */
    pico8_value_t difficulty_entry;
    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    pico8_instance_t *difficulty_button =
        pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref);
    pico8_eu(difficulty_button);

    int frame_count = input.frame_count;
    for (int i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8__update60();
        pico8__draw();
        /* APRES pico8__draw() et avant la frame suivante, comme le PRD
         * le prescrit : c'est le seul instant ou la memoire ecran tient
         * la frame i complete. Une ecriture placee en tete de boucle
         * capturerait la frame precedente et decalerait toute la
         * capture d'une frame, ce que tests/test_pico8_headless_
         * capture.c detecte en comparant la DERNIERE frame capturee au
         * dump texte.
         *
         * La capture est en lecture seule sur `mem` (pico8_screencast_
         * write_frame prend un const pico8_mem_t *), et c'est la
         * propriete qui compte le plus ici : "une capture qui
         * modifierait ce qu'elle observe invaliderait tout
         * l'instrument". Rien n'est ajoute a l'etat du jeu, aucune
         * variable de boucle ne depend de la capture, et le meme test
         * verifie que le dump final est identique avec et sans elle. */
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

    /* La fermeture est verifiee au meme titre que les ecritures :
     * fclose est le dernier endroit ou une ecriture tamponnee peut
     * echouer, et une capture dont la derniere frame n'a pas atteint le
     * disque est exactement le fichier tronque dont le chunk 3 tirerait
     * une empreinte fausse. Sans capture demandee, rien n'a ete ouvert
     * et rien n'est ferme ici.
     *
     * Elle a lieu AVANT le dump texte, et un echec renonce donc a
     * ecrire ce dump : un rejeu dont la capture est perdue n'a pas
     * reussi, et laisser derriere lui un tmp/pico8_replay_dump.txt
     * frais le ferait passer pour une execution normale aux yeux de
     * tout ce qui regarde ce fichier. */
    if (capture_path && !pico8_screencast_writer_close(&capture)) {
        fprintf(stderr, "pico8 replay: echec de fermeture du fichier de capture '%s'\n",
                capture_path);
        return 1;
    }

    FILE *dump_out = fopen("tmp/pico8_replay_dump.txt", "w");
    assert(dump_out != NULL);
    int dump_ok = pico8_dump_screen(&mem, dump_out);
    fclose(dump_out);
    assert(dump_ok);

    pico8_input_free(&input);

    printf("pico8 replay: %d frame(s) run against %s; framebuffer dumped to tmp/pico8_replay_dump.txt\n",
           frame_count, events_path);
    /* Une ligne EN PLUS, et non une ligne modifiee : la ligne ci-dessus
     * est celle que `make replay` imprime depuis la phase 4 et elle
     * reste octet pour octet la meme quand aucune capture n'est
     * demandee. */
    if (capture_path) {
        printf("pico8 replay: %d frame(s) captured to %s\n", frame_count, capture_path);
    }
    return 0;
}
