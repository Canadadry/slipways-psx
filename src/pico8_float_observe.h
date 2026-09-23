#ifndef PICO8_FLOAT_OBSERVE_H
#define PICO8_FLOAT_OBSERVE_H

#include "pico8_fixed.h"

/* Build d'observation de make float-observe, jamais le build par defaut :
 * chaque appel de pico8_fixed_to_double enregistre, par (fichier, fonction,
 * ligne), le minimum de la valeur brute 16.16 recue et le nombre de passages,
 * puis rend la meme valeur que la vraie fonction.
 *
 * pico8_fixed.c doit etre compile SANS -DPICO8_FLOAT_OBSERVE : la macro
 * reecrirait la definition de la fonction elle-meme. */
double pico8_float_observe_to_double(const char *file, const char *func, int line, pico8_fixed_t x);

#define pico8_fixed_to_double(x) pico8_float_observe_to_double(__FILE__, __func__, __LINE__, (x))

#endif
