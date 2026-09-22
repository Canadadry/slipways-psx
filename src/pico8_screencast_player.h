/* Chunk 4 de docs/prd/triage/screencast-capture-and-viewer.md ("Le
 * visualiseur") : les deux seules decisions que prend le visualiseur de
 * screencast et qui ne demandent pas de fenetre -- quelle frame du
 * fichier afficher maintenant, et ce qu'on dit a un humain d'un fichier
 * qu'on refuse de jouer.
 *
 * CINQUIEME APPLICATION DU MEME PRECEDENT, ET POUR LA MEME RAISON :
 * pico8_raylib_backend_coords, pico8_replay_source, pico8_session_
 * recorder, pico8_screencast, puis celui-ci. src/main_screencast_
 * viewer.c inclut raylib.h ; il ne peut etre ni lance (pas de serveur
 * graphique dans les environnements d'agent, et CLAUDE.md l'interdit),
 * ni meme lie partout (raylib n'y est pas toujours vendorise), donc
 * rien de ce qui vit dans ce fichier-la n'est exercable par `make
 * unit`. Le PRD en fait une exigence explicite plutot qu'une preference
 * de style : "tout ce qui est decidable sans fenetre vit hors du module
 * raylib (...) si une partie du visualiseur n'est pas testable sans
 * fenetre, c'est qu'elle est au mauvais endroit". Rien ici n'inclut
 * raylib.h, et rien ici ne l'inclura.
 *
 * CE MODULE EST PETIT PARCE QUE LE RESTE EXISTE DEJA, et c'est voulu :
 * le format et la lecture indexee sont au chunk 1 (pico8_screencast.
 * {c,h}, qui sait deja distinguer un fichier absent, vide ou tronque),
 * l'echelle de vitesse et la ligne d'etat sont dans pico8_replay_
 * control.{c,h}, ecrit et teste pour le rejeu du pilote interactif. Ce
 * qui restait sans domicile tenait en deux decisions, et les voici --
 * un module de dix lignes teste vaut mieux que dix lignes noyees dans
 * un fichier que personne ne peut executer.
 *
 * LA LECTURE REBOUCLE EN FIN DE FICHIER. Le PRD demande que le
 * visualiseur joue le fichier "en continu", et la raison est celle qui
 * a deja donne sa ligne de progression au rejeu interactif : une image
 * figee sur la derniere frame est indiscernable d'un programme plante,
 * et lever ce doute est precisement ce qu'on attend d'un instrument
 * qu'un humain regarde pour SIGNER une empreinte. Le rebouclage donne
 * aussi, gratuitement, le seul geste de navigation dont ce chunk a
 * besoin : revoir un passage sans rien relancer.
 *
 * REJETE : la pause, le retour en arriere, le saut a une frame
 * arbitraire, les chapitres depuis le fichier de marqueurs. Hors
 * perimetre, le PRD les nomme un par un. L'acces indexe du chunk 1 ne
 * leur ferme aucune porte -- la frame N est a l'offset N x 8192 et
 * pico8_screencast_read_frame accepte les index dans n'importe quel
 * ordre -- donc les ajouter le jour ou ils seront demandes ne demandera
 * que d'ecrire autre chose dans le seul entier que ce module tient.
 *
 * REJETE : mettre la phrase de refus dans pico8_screencast, a cote de
 * la validation qui produit le statut. Ce module-la est partage avec le
 * pilote headless du chunk 2, dont la politique d'erreur est autre
 * (erreur fatale a l'ecriture), et son en-tete dit deja pourquoi il
 * n'imprime rien lui-meme : "les chunks 2 et 4 possedent chacun leur
 * message et leur code de sortie". Ceci est celui du chunk 4.
 *
 * REJETE : une seconde echelle de vitesse. pico8_replay_control existe,
 * il est teste, et deux echelles divergentes dans le meme depot seraient
 * exactement la derive que ce depot combat partout ailleurs. Le
 * visualiseur l'appelle directement ; ce module ne connait pas la
 * vitesse, seulement l'ordre des frames. */

#ifndef PICO8_SCREENCAST_PLAYER_H
#define PICO8_SCREENCAST_PLAYER_H

#include "pico8_screencast.h"

#include <stddef.h>

/* De quoi tenir le plus long des trois messages de refus ci-dessous,
 * chemin compris. La fonction tronque plutot que de deborder ce qu'on
 * lui donne, donc c'est une commodite pour l'appelant et non un contrat
 * qu'il pourrait violer -- meme forme que PICO8_REPLAY_CONTROL_STATUS_
 * MAX. */
#define PICO8_SCREENCAST_PLAYER_REFUSAL_MAX 256

/* Ou en est la lecture. Un seul chiffre qui bouge, l'index de la frame
 * affichee ; `frame_count` est fixe a l'ouverture et ne change plus. */
typedef struct {
    int frame_count;
    int current;
} pico8_screencast_player_t;

/* Commence la lecture sur la premiere frame du fichier. `frame_count`
 * est celui que pico8_screencast_frame_count vient de rendre, donc au
 * moins 1 : un fichier vide a deja ete refuse avant, et un curseur sur
 * zero frame n'aurait aucune frame a designer. */
void pico8_screencast_player_init(pico8_screencast_player_t *player, int frame_count);

/* L'index de la frame a afficher maintenant, a passer tel quel a
 * pico8_screencast_read_frame : toujours entre 0 et frame_count-1,
 * quelle que soit la duree de la lecture. C'est aussi ce que la ligne
 * d'etat affiche, a un pres -- pico8_replay_control_format_status
 * compte les frames a partir de 1, la numerotation dans laquelle un
 * humain compte, et l'appelant ajoute donc 1. */
int pico8_screencast_player_current_frame(const pico8_screencast_player_t *player);

/* Passe a la frame suivante, ou revient a la premiere quand la
 * derniere vient d'etre affichee : la lecture ne se termine jamais
 * d'elle-meme, elle reboucle (voir en tete de ce fichier). Rien
 * d'autre ne fait avancer la lecture, et rien ici ne touche au
 * fichier. */
void pico8_screencast_player_advance(pico8_screencast_player_t *player);

/* Ecrit dans `out` (toujours termine par un NUL, tronque plutot que
 * deborde) la phrase que le visualiseur imprime sur stderr avant de
 * s'arreter, pour un `status` que pico8_screencast_reader_open a rendu
 * et qui n'est pas PICO8_SCREENCAST_OK. */
void pico8_screencast_player_format_refusal(pico8_screencast_status_t status, const char *path, char *out, size_t out_size);

#endif
