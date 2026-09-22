/* Phase 4, Chunk 1 (docs/prd/triage/phase4-etape1-headless-replay-wiring-
 * and-playability-verification.md): the project's one real headless game
 * driver -- loads cart assets, wires the dispatch context, boots the real
 * cart (pico8_cart_load/pico8__init/pico8_eu), steps it frame-by-frame
 * from a scripted events.json, and dumps the final framebuffer as text.
 * No game-specific assertions live here (that's tests/test_pico8_
 * playability.c's job, Chunk 2/3) -- this is a thin, general-purpose
 * shell, reusable as-is by Phase 5's raylib backend. Phase 1's own demo
 * assertions, formerly this file's entire content, now live verbatim in
 * tests/test_pico8_demo.c.
 *
 * Chunk 2 de docs/prd/triage/screencast-capture-and-viewer.md ("La
 * capture dans le pilote headless") ajoute a ce pilote UN argument
 * optionnel de plus -- le chemin ou ecrire les pixels de chaque frame --
 * et rien d'autre. Le PRD refuse explicitement un second pilote :
 * "Le depot a deja duplique une sequence de demarrage pour le backend
 * raylib, et la raison y etait enoncee : le pilote raylib a une boucle
 * et une source d'entree differentes. Ici ce n'est pas le cas -- c'est
 * exactement le meme rejeu, avec une ecriture en plus. Une troisieme
 * copie de la sequence d'amorcage creerait un second rejeu subtilement
 * different, et la baseline decrirait alors un deroulement que personne
 * d'autre n'execute." C'est la raison pour laquelle la capture est ici,
 * dans le fichier qui etait deja le rejeu de tout le monde, et non dans
 * un src/main_screencast.c qui aurait ete plus simple a ecrire.
 *
 * SANS CET ARGUMENT, CE FICHIER SE COMPORTE EXACTEMENT COMME AVANT :
 * meme amorcage, meme boucle, meme dump texte de la derniere frame vers
 * tmp/pico8_replay_dump.txt (ni remplace ni modifie -- c'est encore la
 * seule sortie que `make replay` produit), meme ligne sur stdout, meme
 * code de sortie. tests/test_pico8_headless_capture.c en fait une
 * assertion et pas une intention : il rejoue la meme fixture avec et
 * sans capture et compare les deux dumps. */

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

    /* L'ouverture a lieu AVANT la premiere frame, et son echec tue le
     * processus avant qu'une seule frame ne soit rejouee. Le PRD veut
     * l'inverse de la politique de pico8_session_recorder, qui absorbe
     * un echec d'ecriture plutot que de tuer la partie d'un humain :
     * "Ici le contexte est inverse : personne ne joue, et un fichier
     * tronque produirait silencieusement une empreinte fausse,
     * c'est-a-dire un faux rouge indiscernable d'une regression." Une
     * capture qui n'aurait jamais commence est le cas extreme de ce
     * fichier tronque -- le chunk 3 calculerait une empreinte sur un
     * fichier absent ou sur celui d'un rejeu precedent.
     *
     * fprintf(stderr) et un code de sortie plutot qu'un assert() comme
     * le reste de ce fichier : un assert imprime une expression C et
     * pas le chemin fautif, alors que "disque plein, chemin invalide"
     * sont precisement les erreurs qu'un humain corrige en lisant le
     * chemin qu'il a tape. Le message le nomme, donc.
     *
     * Place avant l'amorcage du jeu et pas juste avant la boucle : un
     * chemin mal tape est signale tout de suite, et non apres le
     * chargement de la cartouche. Le fichier vide qu'une ouverture
     * reussie suivie d'un amorcage avorte laisserait derriere elle est
     * le cas PICO8_SCREENCAST_EMPTY, que la lecture refuse deja
     * explicitement -- il ne peut pas passer pour une capture. */
    pico8_screencast_writer_t capture = { NULL };
    if (capture_path && !pico8_screencast_writer_open(&capture, capture_path)) {
        fprintf(stderr, "pico8 replay: impossible d'ouvrir le fichier de capture '%s'\n",
                capture_path);
        return 1;
    }

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
