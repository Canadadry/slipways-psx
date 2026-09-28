#ifndef PICO8_MINIMIZE_H
#define PICO8_MINIMIZE_H

#include "pico8_input.h"

/* Étage mécanique du minimiseur, pur : copie dans `out` (capacité
 * `frame_count`) les frames gardées de `frames`, dans l'ordre, sans en
 * modifier aucune, et rend leur nombre, au moins 1 si `frame_count` >= 1.
 *
 * Une frame est comparée à celle qui la précède dans l'entrée ; avant la
 * frame 0, une frame fictive souris en (0,0), rien d'appuyé.
 * - transition : mouse_buttons ou le d-pad diffère ; toujours gardée ;
 * - inactive : ni transition, souris immobile, aucun bouton ni d-pad
 *   appuyé ; toujours retirée ;
 * - déplacement : ni transition, souris déplacée, boutons et d-pad
 *   éventuellement tenus (un glisser en est un) ; sur les m déplacements
 *   d'un segment entre deux transitions, en garde m / 5 (aucun sous 5),
 *   espacés régulièrement, dont le dernier s'il en reste un ; la
 *   transition porte sa propre position, le clic tombe au même endroit ;
 * - tenue immobile (bouton ou d-pad tenu, souris immobile) : gardée.
 * Une frame gardée redevenue inactive par rapport à la frame gardée qui la
 * précède (trajet revenu sur ses pas) est retirée. Si rien n'est gardé, la
 * dernière frame de l'entrée l'est. */
int pico8_minimize_mechanical(const pico8_input_frame_t *frames, int frame_count, pico8_input_frame_t *out);

#endif
