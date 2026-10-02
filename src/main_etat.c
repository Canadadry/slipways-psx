/* Le pilote de make etat : rejoue argv[1] depuis l'ecran titre comme
 * pico8_replay, cheat compris, puis ecrit l'etat de la derniere frame dans
 * argv[2] (defaut tmp/etat.txt). Un pico8_fatal en cours de rejeu y ecrit
 * quand meme l'etat, ligne d'arret en tete, et sort avec 3. Sans argument, ou sur un argument qui
 * commence par '-' : l'usage et le code 2. */

#include "pico8_etat.h"
#include "pico8_rejeu.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2 || argv[1][0] == '-') {
        fprintf(stderr, "usage: pico8_etat <events.json> [<etat.txt>]\n");
        return 2;
    }
    const char *events_path = argv[1];
    const char *etat_path = (argc > 2) ? argv[2] : "tmp/etat.txt";

    static pico8_rejeu_t rejeu;
    pico8_rejeu_ouvrir(&rejeu, events_path);
    int frame_count = pico8_rejeu_frames(&rejeu);
    static int frame;
    pico8_etat_ecrire_a_l_arret(etat_path, &frame);
    for (frame = 0; frame < frame_count; frame++) pico8_rejeu_jouer_frame(&rejeu, frame);
    pico8_etat_lacher();

    FILE *out = fopen(etat_path, "w");
    if (out == NULL) {
        fprintf(stderr, "pico8 etat: impossible d'ouvrir '%s'\n", etat_path);
        return 1;
    }
    pico8_etat_ecrire(out);
    if (fclose(out) != 0) {
        fprintf(stderr, "pico8 etat: echec d'ecriture de '%s'\n", etat_path);
        return 1;
    }
    pico8_rejeu_fermer(&rejeu);
    printf("pico8 etat: %d frame(s) de %s ; etat ecrit dans %s\n", frame_count, events_path, etat_path);
    return 0;
}
