/* Les seize couleurs de la palette par defaut de PICO-8, en un seul
 * endroit pour les deux programmes raylib du depot : le pilote interactif
 * (src/platform/raylib_backend.c) et le visualiseur de screencast
 * (src/cmd/screencast_viewer.c). Aucun des deux ne redefinit la sienne ;
 * deux tables recopiees finiraient par diverger, et le visualiseur
 * montrerait alors autre chose que le jeu.
 *
 * Ce sont les valeurs publiees de la palette par defaut de PICO-8, de la
 * plomberie de presentation et non une fonction portee depuis
 * pico8/lua.lua. La transparence de PICO-8 (palt()) est deja resolue par
 * pico8_gfx.c avant qu'un pixel n'atteigne la memoire ecran, donc l'alpha
 * est toujours opaque.
 *
 * Des triplets d'octets et non des `Color` : un `Color` imposerait
 * raylib.h a ce fichier et le sortirait de `make unit`, alors que
 * quarante-huit octets hexadecimaux sont exactement l'endroit ou une
 * faute de frappe ne se voit qu'a l'oeil, dans une fenetre.
 * tests/test_pico8_screencast_player.c les asserte une par une ; chaque
 * fichier raylib fabrique son `Color` au point d'usage. */

#ifndef PICO8_PALETTE_H
#define PICO8_PALETTE_H

/* Les seize index que la memoire ecran de PICO-8 peut contenir. */
#define PICO8_PALETTE_COLORS 16

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pico8_rgb_t;

/* PICO8_PALETTE_RGB[i] est la couleur de l'index i, 0 <= i <= 15. */
extern const pico8_rgb_t PICO8_PALETTE_RGB[PICO8_PALETTE_COLORS];

#endif
