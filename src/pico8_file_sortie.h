#ifndef PICO8_FILE_SORTIE_H
#define PICO8_FILE_SORTIE_H

#include "pico8_struct.h"

#define PICO8_FILE_SORTIE_CAP 256

/* Une entite qui a quitte hv, et dont la classe a `rendre` : la frame en
 * cours la relit encore, on ne la rend qu'au vidage. pico8_fatal si la file
 * est pleine. */
void pico8_file_sortie_mettre(pico8_struct_header_t *e);

int pico8_file_sortie_compte(void);

/* Pour chaque entree, dans l'ordre : `oublier(e)`, qui remet a nil ce qui
 * la designe encore, puis `rendre`. La file est vide au retour. */
void pico8_file_sortie_vider(void (*oublier)(const pico8_struct_header_t *e));

#endif
