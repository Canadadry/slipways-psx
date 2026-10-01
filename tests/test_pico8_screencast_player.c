/* Les deux seules decisions que prend le visualiseur de screencast et
 * qui ne demandent pas de fenetre : quelle frame afficher maintenant,
 * et ce qu'on dit a un humain dont le fichier ne peut pas etre joue.
 *
 * CE QUE CE FICHIER NE PEUT PAS AFFIRMER : qu'une fenetre s'ouvre, que
 * l'image est a l'echelle 6, que PAGE UP accelere reellement la
 * lecture, que la ligne d'etat est lisible. Tout cela demande un
 * serveur graphique et un humain, et CLAUDE.md interdit d'ouvrir cette
 * fenetre. Ce qui n'est pas testable sans fenetre est au mauvais
 * endroit ; ce fichier teste ce qui a ete mis au bon endroit, et ne
 * simule ni touche ni pixel.
 *
 * On affirme partout la SUITE des index servis, un par un et dans
 * l'ordre, jamais leur nombre : un test qui compterait les avances
 * passerait sur un curseur qui resservirait toujours la frame 0, ou qui
 * les servirait a l'envers. */

#include "../src/pico8_screencast_player.h"

#include "p8/palette.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* La lecture commence sur la premiere frame du fichier et avance d'une
 * frame par appel, dans l'ordre. C'est la logique neuve de ce module :
 * le format et la lecture indexee sont dans pico8_screencast, la
 * vitesse dans pico8_replay_control. */
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
 * se fige sur la derniere frame au lieu de reboucler : une capture a
 * une fin, et la derniere image est celle qu'un humain vient examiner
 * avant de signer une empreinte -- une lecture qui repartirait au debut
 * la lui retirerait au moment ou il la regarde.
 *
 * L'avance est appelee PLUSIEURS fois au-dela de la fin, et l'index est
 * asserti a chaque tour : un curseur qui se figerait une fois puis
 * repartirait, ou qui deborderait a la deuxieme avance de trop,
 * passerait un test qui s'arreterait a la premiere. */
static void test_playback_freezes_on_the_last_frame(void) {
    pico8_screencast_player_t player;
    pico8_screencast_player_init(&player, 3);

    assert(pico8_screencast_player_current_frame(&player) == 0);
    pico8_screencast_player_advance(&player);
    assert(pico8_screencast_player_current_frame(&player) == 1);
    pico8_screencast_player_advance(&player);
    assert(pico8_screencast_player_current_frame(&player) == 2);

    /* La derniere frame est atteinte : a partir d'ici, plus rien ne
     * bouge, et surtout jamais vers 0. */
    for (int i = 0; i < 50; i++) {
        pico8_screencast_player_advance(&player);
        assert(pico8_screencast_player_current_frame(&player) == 2);
    }
}

/* Un fichier d'une seule frame est une capture valide (8 192 octets), et
 * c'est le cas ou une avance mal ecrite sort du fichier le plus vite :
 * l'index reste 0, quel que soit le nombre d'avances. Un index hors
 * bornes ne doit jamais etre demande a pico8_screencast_read_frame, qui
 * le refuserait. */
static void test_a_single_frame_capture_stays_on_its_only_frame(void) {
    pico8_screencast_player_t player;
    pico8_screencast_player_init(&player, 1);

    for (int i = 0; i < 100; i++) {
        assert(pico8_screencast_player_current_frame(&player) == 0);
        pico8_screencast_player_advance(&player);
    }
}

/* LE REFUS, QUI EST L'AUTRE MOITIE DU MODULE. pico8_screencast sait
 * deja DISTINGUER les trois facons dont un fichier de pixels peut etre
 * injouable (absent/illisible, vide, tronque) ; ce qu'il ne fait pas,
 * et ne doit pas faire, c'est decider quoi en dire a un humain. Une
 * fenetre qui s'ouvre sur du noir est indiscernable d'une session qui
 * commence par un ecran noir, donc le refus doit etre EXPLICITE et doit
 * arriver AVANT la fenetre.
 *
 * Les trois messages sont affirmes un par un et en entier. Un test qui
 * verifierait seulement qu'une chaine non vide sort passerait sur la
 * mutation la plus probable de ce module -- un seul message generique
 * pour les trois cas. Chacun nomme le chemin (celui qu'on a tape n'est
 * pas toujours celui qu'on croit), dit la cause, et dit qu'aucune
 * fenetre ne s'est ouverte. */
static void test_each_refusal_names_the_path_and_its_own_reason(void) {
    char out[PICO8_SCREENCAST_PLAYER_REFUSAL_MAX];

    pico8_screencast_player_format_refusal(PICO8_SCREENCAST_UNREADABLE, "tmp/absent.bin", out, sizeof out);
    assert(strcmp(out,
                  "screencast-viewer: 'tmp/absent.bin' est introuvable ou illisible -- aucune fenetre n'a ete ouverte. "
                  "Fabriquer les pixels : make screencast EVENTS=<events.json>") == 0);

    pico8_screencast_player_format_refusal(PICO8_SCREENCAST_EMPTY, "tmp/vide.bin", out, sizeof out);
    assert(strcmp(out,
                  "screencast-viewer: 'tmp/vide.bin' est vide -- aucune fenetre n'a ete ouverte. "
                  "Une capture interrompue avant sa premiere frame donne ce fichier ; la refaire : make screencast EVENTS=<events.json>") == 0);

    pico8_screencast_player_format_refusal(PICO8_SCREENCAST_TRUNCATED, "tmp/coupe.bin", out, sizeof out);
    assert(strcmp(out,
                  "screencast-viewer: 'tmp/coupe.bin' est tronque : sa taille n'est pas un multiple de 8192 octets, "
                  "donc il se termine au milieu d'une frame -- aucune fenetre n'a ete ouverte. "
                  "La refaire : make screencast EVENTS=<events.json>") == 0);
}

/* Le meme contrat que pico8_replay_control_format_status : une sortie
 * toujours terminee par un NUL, tronquee plutot que debordee. Un
 * message d'erreur n'a pas le droit d'etre lui-meme la faute -- et
 * celui-ci est ecrit juste avant un exit, la ou plus rien ne rattrape
 * un debordement. Les 16 octets de `out` sont plus courts que le plus
 * court des trois messages, donc la troncature est certaine. */
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
 * visualiseur ne redefinit pas sa palette : la conversion d'un index
 * PICO-8 vers une couleur est deja faite par le backend interactif, et
 * les deux binaires raylib lisent la meme table (src/p8/palette.c,
 * sans raylib, trois octets par couleur).
 *
 * Quarante-huit octets hexadecimaux ecrits a la main sont exactement ou
 * une faute de frappe se cache sans rien casser de visible : un bleu
 * legerement faux ne fait echouer aucune compilation et ne se remarque
 * qu'a l'oeil, sur une fenetre que personne ici ne peut ouvrir. D'ou
 * ces seize assertions, une par couleur. Les valeurs sont celles,
 * publiees et universellement documentees, de la palette par defaut de
 * PICO-8 -- pas une fonction portee depuis pico8/lua.lua. Elles vivent
 * dans ce binaire plutot que dans une cible a elles parce qu'une cible
 * de plus pour seize constantes serait plus de Makefile que de test. */
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
    test_playback_freezes_on_the_last_frame();
    test_a_single_frame_capture_stays_on_its_only_frame();
    test_each_refusal_names_the_path_and_its_own_reason();
    test_a_refusal_truncates_rather_than_overflowing();
    test_the_shared_palette_is_the_published_pico8_one();

    printf("test_pico8_screencast_player: OK\n");
    return 0;
}
