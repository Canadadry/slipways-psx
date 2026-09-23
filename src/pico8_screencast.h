/* Le format d'une capture, en entier. Aucun autre module ne le connait :
 * ni le pilote headless ni le visualiseur ne calculent d'offset.
 * Concatenation brute des frames dans l'ordre de capture, sans en-tete,
 * sans index, sans numero de version. Une frame occupe
 * PICO8_SCREENCAST_FRAME_BYTES = 8192 octets : les 16384 pixels de
 * l'ecran 128x128, deux pixels par octet, dans l'ordre de balayage de la
 * memoire ecran. L'octet d'indice k tient le pixel 2k dans son quartet de
 * poids FAIBLE et le pixel 2k+1 dans celui de poids fort. La frame N
 * commence donc a l'offset N x 8192, et le nombre de frames est la taille
 * du fichier divisee par 8192, jamais un compteur stocke. Ce contrat est
 * public -- un script exterieur doit pouvoir lire le fichier sans le code
 * du projet -- et tests/test_pico8_screencast.c asserte l'ordre des
 * quartets sur les octets bruts, ce qu'un aller-retour ne prouverait pas.
 *
 * Rien ici n'inclut raylib.h et rien ici ne doit l'inclure : un fichier
 * qui lie raylib sort de `make unit`, donc le format, la lecture et la
 * validation de taille ne seraient plus verifiables sans fenetre. */

#ifndef PICO8_SCREENCAST_H
#define PICO8_SCREENCAST_H

#include "pico8_gfx.h"
#include "pico8_mem.h"

#include <stdio.h>

/* Les 16384 pixels d'une frame, un par octet : la taille du tampon que
 * pico8_screencast_read_frame remplit. */
#define PICO8_SCREENCAST_FRAME_PIXELS (PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT)

/* Les 8192 octets qu'une frame occupe DANS LE FICHIER, deux pixels par
 * octet. */
#define PICO8_SCREENCAST_FRAME_BYTES (PICO8_SCREENCAST_FRAME_PIXELS / 2)

/* La capture en cours d'ecriture ; `file` ne s'ecrit pas directement. */
typedef struct {
    FILE *file;
} pico8_screencast_writer_t;

/* Commence une capture a `path`, en tronquant un fichier existant. Rend
 * 1 si le fichier est ouvert, 0 sinon ; apres un echec, `*writer` reste
 * sur a passer a pico8_screencast_writer_close. */
int pico8_screencast_writer_open(pico8_screencast_writer_t *writer, const char *path);

/* Ajoute une frame a la capture, lue depuis la memoire ecran de `mem`
 * (PICO8_ADDR_SCREEN) et empaquetee dans le format ci-dessus. Rend 1 si
 * les 8192 octets ont ete ecrits, 0 sinon. L'echec est rendu a
 * l'appelant et non absorbe : contrairement a pico8_session_recorder,
 * perdre une frame ici produit un fichier tronque dont on tirerait une
 * empreinte fausse. */
int pico8_screencast_write_frame(pico8_screencast_writer_t *writer, const pico8_mem_t *mem);

/* Termine la capture et rend 1 si la fermeture a reussi, 0 sinon --
 * fclose est le dernier endroit ou une ecriture tamponnee peut echouer.
 * Sur un writer dont l'ouverture avait echoue, ne fait rien et rend 1 ;
 * appeler deux fois est sans effet. */
int pico8_screencast_writer_close(pico8_screencast_writer_t *writer);

/* Le sort d'une ouverture en lecture. La distinction entre les causes
 * appartient a ce module, le message a l'appelant. */
typedef enum {
    /* Le fichier est ouvert et sa taille est un nombre entier et non nul
     * de frames. */
    PICO8_SCREENCAST_OK = 0,
    /* Absent, ou illisible. */
    PICO8_SCREENCAST_UNREADABLE = 1,
    /* Present et vide. Une valeur a part, et non un cas de taille
     * invalide : zero EST un multiple de 8192, donc sans ce refus
     * explicite un fichier vide passerait pour une capture de zero
     * frame. */
    PICO8_SCREENCAST_EMPTY = 2,
    /* La taille n'est pas un multiple de PICO8_SCREENCAST_FRAME_BYTES :
     * le fichier se termine au milieu d'une frame. */
    PICO8_SCREENCAST_TRUNCATED = 3
} pico8_screencast_status_t;

/* Une capture ouverte en lecture. Les deux champs ne valent quelque
 * chose que si l'ouverture a rendu PICO8_SCREENCAST_OK ; dans les trois
 * autres cas le fichier est deja referme et le compte nul, de sorte que
 * pico8_screencast_reader_close soit valide quel que soit le sort. */
typedef struct {
    FILE *file;
    int frame_count;
} pico8_screencast_reader_t;

/* Ouvre une capture en lecture et valide sa taille. Le fichier est
 * ouvert en "rb" et JAMAIS en ecriture : relire une capture ne doit pas
 * pouvoir en changer un octet. */
pico8_screencast_status_t pico8_screencast_reader_open(pico8_screencast_reader_t *reader, const char *path);

/* Combien de frames la capture contient. 0 si l'ouverture a echoue.
 * C'est la borne des index acceptes ci-dessous. */
int pico8_screencast_frame_count(const pico8_screencast_reader_t *reader);

/* Lit la frame `index` (0 a frame_count-1) dans `out_pixels`, qui doit
 * pouvoir tenir PICO8_SCREENCAST_FRAME_PIXELS octets. Chaque octet rendu
 * est UN pixel, de valeur 0 a 15 : le depaquetage des quartets est fait
 * ici. Rend 1 en cas de succes ; rend 0 et laisse `out_pixels` intact si
 * l'index est hors des frames du fichier ou si la lecture echoue. Rien
 * ne garde de position courante, donc les index peuvent etre demandes
 * dans n'importe quel ordre et autant de fois qu'on veut. */
int pico8_screencast_read_frame(pico8_screencast_reader_t *reader, int index, unsigned char *out_pixels);

/* Referme la capture et rend `*reader` a son etat d'avant l'ouverture.
 * Sur une ouverture qui avait echoue, et sur un reader deja ferme, ne
 * fait rien. Rien ne peut echouer : aucune ecriture n'est en attente. */
void pico8_screencast_reader_close(pico8_screencast_reader_t *reader);

#endif
