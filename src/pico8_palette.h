/* Chunk 4 de docs/prd/triage/screencast-capture-and-viewer.md ("Le
 * visualiseur") : les seize couleurs de la palette par defaut de
 * PICO-8, en un seul endroit, pour les deux programmes raylib de ce
 * depot -- le pilote interactif (src/pico8_raylib_backend.c) et le
 * visualiseur de screencast (src/main_screencast_viewer.c).
 *
 * POURQUOI CE FICHIER EXISTE. Ces valeurs etaient une table `static
 * const Color` a l'interieur de src/pico8_raylib_backend.c, ou elles
 * etaient parfaitement a leur place tant qu'un seul programme
 * presentait la memoire ecran. Le visualiseur en est un second, et le
 * PRD lui interdit d'en avoir une a lui : "la conversion d'un index de
 * palette PICO-8 vers une couleur est DEJA faite par le backend
 * interactif ; le visualiseur affiche exactement la meme chose et ne
 * doit pas redefinir sa propre palette". Deux tables recopiees se
 * repondraient tant que personne n'en corrige une seule -- et le jour
 * ou cela arriverait, le visualiseur montrerait autre chose que le jeu,
 * ce qui est exactement ce qu'un instrument de validation visuelle ne
 * doit pas pouvoir faire. Le deplacement n'a change aucune valeur et
 * n'a change aucun comportement du pilote interactif.
 *
 * PAS DE RAYLIB ICI, ET C'EST CE QUI PERMET DE LES TESTER. La table est
 * en triplets d'octets et non en `Color` : chaque fichier raylib
 * fabrique son `Color` au point d'usage, en ajoutant l'alpha opaque.
 * Un `Color` aurait impose raylib.h a ce fichier, donc l'aurait rendu
 * inatteignable par `make unit` -- alors que recopier a la main
 * quarante-huit octets hexadecimaux est precisement l'operation ou une
 * faute de frappe se cache : elle ne casse aucune compilation et ne se
 * voit qu'a l'oeil, dans une fenetre qu'un agent ne peut pas ouvrir.
 * tests/test_pico8_screencast_player.c les affirme une par une.
 *
 * Ce sont les valeurs publiees et universellement documentees de la
 * palette par defaut de PICO-8 (voir par exemple la page "Palette" du
 * wiki PICO-8), de la plomberie de presentation et non une fonction
 * portee depuis pico8/lua.lua. L'alpha est toujours opaque : la
 * transparence propre a PICO-8 (palt()) est deja traitee dans la couche
 * de mimetisme avant qu'un pixel n'atteigne la memoire ecran que ces
 * deux programmes lisent. */

#ifndef PICO8_PALETTE_H
#define PICO8_PALETTE_H

/* Les seize index que la memoire ecran de PICO-8 peut contenir. */
#define PICO8_PALETTE_COLORS 16

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pico8_rgb_t;

/* La palette, dans l'ordre des index : PICO8_PALETTE_RGB[i] est la
 * couleur de l'index i, 0 <= i <= 15. */
extern const pico8_rgb_t PICO8_PALETTE_RGB[PICO8_PALETTE_COLORS];

#endif
