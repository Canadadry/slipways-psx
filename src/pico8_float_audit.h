#ifndef PICO8_FLOAT_AUDIT_H
#define PICO8_FLOAT_AUDIT_H

#include "pico8_fixed.h"

/* Build d'audit de make float-audit, jamais le build par defaut : chaque
 * appel de pico8_fixed_flr_int calcule aussi l'ancienne expression
 * (int)pico8_fixed_to_double(x) et la reference floor() de la libc, les passe
 * a pico8_float_audit_record, puis rend la valeur de la vraie fonction.
 *
 * pico8_fixed.c doit etre compile SANS -DPICO8_FLOAT_AUDIT : la macro
 * reecrirait la definition de la fonction elle-meme.
 *
 * "Argument negatif = echec" ne vaut que pour les sites C1, dont le signe est
 * prouve : tools/float_audit_c1.txt les nomme, tout autre appel est un site D
 * ou un negatif est legitime. */
/* Point d'entree generique d'une paire ancien/nouveau (D2, D3 y branchent la
 * leur) : l'appelant calcule les deux valeurs et une reference, et passe
 * l'argument brut 16.16, si ancien != nouveau et si nouveau != reference. Par
 * (fichier, ligne) : appels, negatifs, fractionnaires (16 bits bas non nuls),
 * ecarts, et frame du premier ancien != nouveau, lue sur l'entree du contexte
 * de dispatch : 0 pendant l'initialisation, -1 si aucune entree n'est posee. */
void pico8_float_audit_record(const char *file, const char *func, int line, pico8_fixed_t arg,
                              int old_differs, int new_differs_from_ref);

int32_t pico8_float_audit_flr_int(const char *file, const char *func, int line, pico8_fixed_t x);

#define pico8_fixed_flr_int(x) pico8_float_audit_flr_int(__FILE__, __func__, __LINE__, (x))

#endif
