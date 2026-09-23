#ifndef PICO8_FLOAT_AUDIT_H
#define PICO8_FLOAT_AUDIT_H

#include "pico8_fixed.h"

/* Build d'audit de make float-audit, jamais le build par defaut : chaque
 * appel de pico8_fixed_flr_int calcule aussi l'ancienne expression
 * (int)pico8_fixed_to_double(x) et compte, par (fichier, fonction, ligne), les
 * passages, les arguments negatifs et les ecarts entre les deux, puis rend la
 * valeur de la vraie fonction.
 *
 * pico8_fixed.c doit etre compile SANS -DPICO8_FLOAT_AUDIT : la macro
 * reecrirait la definition de la fonction elle-meme.
 *
 * "Argument negatif = echec" ne vaut que pour les sites C1, dont le signe est
 * prouve. Des que D1 appellera pico8_fixed_flr_int a des sites (c), un negatif
 * y sera legitime : la porte devra alors distinguer les sites. */
int32_t pico8_float_audit_flr_int(const char *file, const char *func, int line, pico8_fixed_t x);

#define pico8_fixed_flr_int(x) pico8_float_audit_flr_int(__FILE__, __func__, __LINE__, (x))

#endif
