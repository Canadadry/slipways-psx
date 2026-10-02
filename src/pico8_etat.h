#ifndef PICO8_ETAT_H
#define PICO8_ETAT_H

#include <stdio.h>

/* Ecrit dans `out` l'etat courant du moteur, une ligne
 * « <section> <cle> <valeurs...> » par fait. Lecture seule : n'alloue dans
 * aucune zone du jeu et n'appelle rien du cart qui ecrive. */
void pico8_etat_ecrire(FILE *out);

/* Jusqu'a pico8_etat_lacher(), toute sortie du processus, pico8_fatal
 * compris, ecrit l'etat dans `path` apres une ligne « arret pico8_fatal
 * frame <*frame> ». `path` et `frame` doivent vivre jusqu'a la sortie. */
void pico8_etat_ecrire_a_l_arret(const char *path, const int *frame);
void pico8_etat_lacher(void);

#endif
