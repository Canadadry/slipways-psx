/* Chunk 4 de docs/prd/triage/screencast-capture-and-viewer.md ("Le
 * visualiseur") : les deux seules decisions que le visualiseur prend et
 * qui ne demandent pas de fenetre -- quelle frame afficher maintenant,
 * et ce qu'on dit a un humain dont le fichier ne peut pas etre joue.
 *
 * CE QUE CE FICHIER PEUT AFFIRMER, ET CE QU'IL NE PEUT PAS. Il ne peut
 * pas affirmer qu'une fenetre s'ouvre, que l'image est a l'echelle 6,
 * que PAGE UP accelere reellement la lecture, ni que la ligne d'etat est
 * lisible : tout cela demande un serveur graphique et un humain, et
 * CLAUDE.md interdit par ailleurs d'ouvrir cette fenetre a ce stade. Le
 * PRD en tire la consequence sur le decoupage plutot que sur les tests :
 * "si une partie du visualiseur n'est pas testable sans fenetre, c'est
 * qu'elle est au mauvais endroit". Ce fichier teste donc ce qui a ete
 * mis au bon endroit, et ne simule ni touche ni pixel.
 *
 * Le motif d'assertion suit celui de tests/test_pico8_replay_source.c et
 * de tests/test_pico8_screencast.c : on affirme la SUITE des index
 * servis, un par un et dans l'ordre, jamais leur nombre. Un test qui
 * compterait les avances passerait sur un curseur qui resservirait
 * toujours la frame 0, ou qui les servirait a l'envers -- remarque deja
 * consignee a propos du module de rejeu, et que le PRD reprend mot pour
 * mot pour ce chunk-ci. */

#include "../src/pico8_screencast_player.h"

#include "../src/pico8_palette.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* La lecture commence sur la premiere frame du fichier et avance d'une
 * frame par appel, dans l'ordre : c'est la seule chose qu'un
 * visualiseur de video doit faire, et c'est la logique neuve de ce
 * chunk (le format et la lecture indexee sont au chunk 1, la vitesse
 * est dans pico8_replay_control). */
static void test_playback_starts_at_the_first_frame_and_walks_in_order(void) {
    pico8_screencast_player_t player;
    pico8_screencast_player_init(&player, 5);

    assert(pico8_screencast_player_current_frame(&player) == 0);
    pico8_screencast_player_advance(&player);
    assert(pico8_screencast_player_current_frame(&player) == 1);
    pico8_screencast_player_advance(&player);
    assert(pico8_screencast_player_current_frame(&player) == 2);
    pico8_screencast_player_advance(&player);
    assert(pico8_screencast_player_current_frame(&player) == 3);
    pico8_screencast_player_advance(&player);
    assert(pico8_screencast_player_current_frame(&player) == 4);
}

/* LA FIN DU FICHIER, QUI EST LA VRAIE DECISION DE CE MODULE. La lecture
 * reprend a la premiere frame au lieu de s'arreter sur la derniere. Le
 * PRD demande que le visualiseur joue le fichier "en continu", et la
 * raison est la meme que celle de la ligne de progression du rejeu
 * interactif : une image figee est indiscernable d'un programme plante,
 * et c'est exactement le doute que ce visualiseur existe pour lever.
 * Un humain qui veut revoir un passage n'a alors rien a relancer.
 *
 * La suite complete est affirmee, et pas seulement le retour a zero :
 * un curseur qui repartirait de 0 puis resterait coince y passerait un
 * test qui ne regarderait que la premiere reprise. */
static void test_playback_loops_back_to_the_first_frame_at_the_end(void) {
    pico8_screencast_player_t player;
    pico8_screencast_player_init(&player, 3);

    assert(pico8_screencast_player_current_frame(&player) == 0);
    pico8_screencast_player_advance(&player);
    assert(pico8_screencast_player_current_frame(&player) == 1);
    pico8_screencast_player_advance(&player);
    assert(pico8_screencast_player_current_frame(&player) == 2);
    pico8_screencast_player_advance(&player);
    assert(pico8_screencast_player_current_frame(&player) == 0);
    pico8_screencast_player_advance(&player);
    assert(pico8_screencast_player_current_frame(&player) == 1);
    pico8_screencast_player_advance(&player);
    assert(pico8_screencast_player_current_frame(&player) == 2);
    pico8_screencast_player_advance(&player);
    assert(pico8_screencast_player_current_frame(&player) == 0);
}

/* Un fichier d'une seule frame est une capture valide (8 192 octets), et
 * c'est le cas ou une avance mal ecrite sort du fichier le plus vite :
 * l'index reste 0, quel que soit le nombre d'avances. Ce que le chunk 1
 * refuserait de lire -- un index hors bornes -- ne doit donc jamais lui
 * etre demande. */
static void test_a_single_frame_capture_stays_on_its_only_frame(void) {
    pico8_screencast_player_t player;
    pico8_screencast_player_init(&player, 1);

    for (int i = 0; i < 100; i++) {
        assert(pico8_screencast_player_current_frame(&player) == 0);
        pico8_screencast_player_advance(&player);
    }
}

/* LE REFUS, QUI EST L'AUTRE MOITIE DU MODULE. Le chunk 1 sait deja
 * DISTINGUER les trois facons dont un fichier de pixels peut etre
 * injouable (absent/illisible, vide, tronque) ; ce qu'il ne fait pas,
 * et ne doit pas faire, c'est decider quoi en dire a un humain. C'est
 * ici, et c'est teste ici, parce que le PRD en fait une exigence de
 * fond : "une fenetre qui s'ouvre sur du noir est indiscernable d'une
 * session qui commence par un ecran noir", donc le refus doit etre
 * EXPLICITE et doit arriver AVANT la fenetre.
 *
 * Les trois messages sont affirmes un par un et en entier. Un test qui
 * se contenterait de verifier qu'une chaine non vide sort passerait sur
 * la mutation la plus probable de tout ce module -- un seul message
 * generique pour les trois cas -- qui est precisement le defaut que le
 * PRD nomme : "le message est la seule interface humaine d'un echec".
 * Chacun nomme le chemin (celui qu'on a tape n'est pas toujours celui
 * qu'on croit), dit la cause, et dit qu'aucune fenetre ne s'est
 * ouverte. */
static void test_each_refusal_names_the_path_and_its_own_reason(void) {
    char out[PICO8_SCREENCAST_PLAYER_REFUSAL_MAX];

    pico8_screencast_player_format_refusal(PICO8_SCREENCAST_UNREADABLE, "tmp/absent.bin", out, sizeof out);
    assert(strcmp(out,
                  "screencast-viewer: 'tmp/absent.bin' est introuvable ou illisible -- aucune fenetre n'a ete ouverte. "
                  "Fabriquer les pixels : make screencast") == 0);

    pico8_screencast_player_format_refusal(PICO8_SCREENCAST_EMPTY, "tmp/vide.bin", out, sizeof out);
    assert(strcmp(out,
                  "screencast-viewer: 'tmp/vide.bin' est vide -- aucune fenetre n'a ete ouverte. "
                  "Une capture interrompue avant sa premiere frame donne ce fichier ; la refaire : make screencast") == 0);

    pico8_screencast_player_format_refusal(PICO8_SCREENCAST_TRUNCATED, "tmp/coupe.bin", out, sizeof out);
    assert(strcmp(out,
                  "screencast-viewer: 'tmp/coupe.bin' est tronque : sa taille n'est pas un multiple de 8192 octets, "
                  "donc il se termine au milieu d'une frame -- aucune fenetre n'a ete ouverte. "
                  "La refaire : make screencast") == 0);
}

/* Le meme contrat que pico8_replay_control_format_status : une sortie
 * toujours terminee par un NUL, tronquee plutot que debordee. Un
 * message d'erreur n'a pas le droit d'etre lui-meme la faute -- et
 * celui-ci est ecrit juste avant un exit, la ou plus rien ne rattrape
 * un debordement. */
static void test_a_refusal_truncates_rather_than_overflowing(void) {
    char out[16];
    char guard[8];

    memset(guard, 0x7f, sizeof guard);
    pico8_screencast_player_format_refusal(PICO8_SCREENCAST_UNREADABLE, "tmp/absent.bin", out, sizeof out);

    assert(out[sizeof out - 1] == '\0');
    assert(strlen(out) < sizeof out);
    for (size_t i = 0; i < sizeof guard; i++) {
        assert(guard[i] == 0x7f);
    }
}

/* LA PALETTE, ET POURQUOI ELLE EST ASSERTIONNEE DANS CE FICHIER-CI. Le
 * PRD interdit au visualiseur de redefinir sa palette : "la conversion
 * d'un index de palette PICO-8 vers une couleur est DEJA faite par le
 * backend interactif ; le visualiseur affiche exactement la meme chose".
 * Les seize couleurs etaient jusqu'ici une table `static const Color`
 * enfermee dans src/pico8_raylib_backend.c -- inatteignable depuis un
 * second binaire raylib autrement qu'en la recopiant, c'est-a-dire en
 * creant la deuxieme verite que le PRD refuse. Elles sont donc sorties
 * dans src/pico8_palette.{c,h}, sans raylib (trois octets par couleur,
 * pas un `Color`), et les deux fichiers raylib les lisent la.
 *
 * Deplacer quarante-huit octets hexadecimaux ecrits a la main est
 * exactement l'operation ou une faute de frappe se cache sans rien
 * casser de visible -- un bleu legerement faux ne fait echouer aucune
 * compilation et ne se remarque qu'a l'oeil, sur une fenetre que
 * personne ici ne peut ouvrir. D'ou ces seize assertions, une par
 * couleur. Elles vivent dans ce binaire plutot que dans une cible a
 * elles parce qu'une cible de plus pour seize constantes serait plus de
 * Makefile que de test, et parce que la palette est, comme le reste de
 * ce fichier, ce que le visualiseur decide sans fenetre. Les valeurs
 * sont celles, publiees et universellement documentees, de la palette
 * par defaut de PICO-8 -- pas une fonction portee depuis pico8/lua.lua. */
static void test_the_shared_palette_is_the_published_pico8_one(void) {
    static const unsigned char expected[PICO8_PALETTE_COLORS][3] = {
        { 0x00, 0x00, 0x00 }, { 0x1D, 0x2B, 0x53 }, { 0x7E, 0x25, 0x53 }, { 0x00, 0x87, 0x51 },
        { 0xAB, 0x52, 0x36 }, { 0x5F, 0x57, 0x4F }, { 0xC2, 0xC3, 0xC7 }, { 0xFF, 0xF1, 0xE8 },
        { 0xFF, 0x00, 0x4D }, { 0xFF, 0xA3, 0x00 }, { 0xFF, 0xEC, 0x27 }, { 0x00, 0xE4, 0x36 },
        { 0x29, 0xAD, 0xFF }, { 0x83, 0x76, 0x9C }, { 0xFF, 0x77, 0xA8 }, { 0xFF, 0xCC, 0xAA },
    };

    for (int i = 0; i < PICO8_PALETTE_COLORS; i++) {
        assert(PICO8_PALETTE_RGB[i].r == expected[i][0]);
        assert(PICO8_PALETTE_RGB[i].g == expected[i][1]);
        assert(PICO8_PALETTE_RGB[i].b == expected[i][2]);
    }
}

int main(void) {
    test_playback_starts_at_the_first_frame_and_walks_in_order();
    test_playback_loops_back_to_the_first_frame_at_the_end();
    test_a_single_frame_capture_stays_on_its_only_frame();
    test_each_refusal_names_the_path_and_its_own_reason();
    test_a_refusal_truncates_rather_than_overflowing();
    test_the_shared_palette_is_the_published_pico8_one();

    printf("test_pico8_screencast_player: OK\n");
    return 0;
}
