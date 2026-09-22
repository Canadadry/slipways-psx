/* Chunk 1 de docs/prd/triage/screencast-capture-and-viewer.md ("Le
 * module de screencast et son format") : tout ce qui touche au format
 * du fichier de pixels d'une session -- ecrire une frame depuis la
 * memoire du jeu, ouvrir un fichier en lecture, en donner le nombre de
 * frames, et lire la frame N.
 *
 * Le PRD en fait un module profond au sens ou l'entend le depot : une
 * interface etroite devant un format qui pourrait changer, et surtout
 * "le seul endroit ou le format est connu : ni le pilote headless, ni
 * le visualiseur ne calculent d'offset eux-memes". C'est la raison
 * d'etre du fichier : le chunk 2 (capture dans le pilote headless) et
 * le chunk 4 (visualiseur raylib) parlent tous deux de frames, jamais
 * d'octets.
 *
 * AUCUNE DEPENDANCE RAYLIB, JAMAIS. Le consommateur naturel de la
 * moitie lecture est le visualiseur du chunk 4, qui est un binaire
 * raylib -- et un agent ne peut ni lancer une fenetre (CLAUDE.md) ni
 * meme lier libraylib.a partout, donc rien de ce qui vit dans un
 * fichier incluant raylib.h n'est exercable par `make unit`. Le PRD en
 * tire la consequence explicite : "Tout ce qui est decidable sans
 * fenetre vit hors du module raylib : le format, la lecture, le nombre
 * de frames, la validation de taille (...). Ce n'est pas une preference
 * de style : c'est la seule facon pour l'agent qui ecrit ce visualiseur
 * de verifier son travail avant de le livrer." Trois modules l'ont deja
 * fait pour la meme raison -- pico8_raylib_backend_coords, pico8_replay_
 * source, pico8_session_recorder -- et celui-ci est le quatrieme. Rien
 * ici n'inclut raylib.h, et rien ici ne l'inclura.
 *
 * LE FORMAT, EN ENTIER. La concatenation brute des frames dans l'ordre
 * de capture. Chaque frame occupe exactement PICO8_SCREENCAST_FRAME_
 * BYTES = 8 192 octets : les 16 384 pixels de l'ecran 128x128, deux
 * pixels par octet, un quartet chacun, dans l'ordre de balayage de la
 * memoire ecran (ligne par ligne, de gauche a droite). L'octet d'indice
 * k tient le pixel 2k dans son quartet de poids FAIBLE et le pixel 2k+1
 * dans celui de poids fort -- c'est la representation de la memoire
 * ecran de PICO-8 reelle, et tests/test_pico8_screencast.c l'assertion
 * sur les octets bruts parce qu'un aller-retour par ce module ne
 * pourrait pas la prouver. Pas d'en-tete, pas de nombre magique, pas
 * d'index, pas de separateur, pas de numero de version : la frame N
 * commence a l'offset N x 8192 et le nombre de frames est la taille du
 * fichier divisee par 8 192. Le PRD veut ce format "trivial et
 * documente, afin qu'un script jetable puisse le lire sans dependre du
 * code du projet" (user story 21) ; ce paragraphe est ce contrat, et
 * les constantes ci-dessous ne sont qu'une commodite pour le C.
 *
 * REJETE : TOUTE COMPRESSION. La decision a ete prise sur mesures dans
 * le PRD, qui annule une exigence initiale. Sur les 9 729 frames reelles
 * de l'events.json du depot, un XOR-delta + RLE maison atteint 41x, `xz`
 * 142x, un decoupage en tuiles 8x8 puis `xz` 192x. Aucun de ces gains
 * n'a de destinataire : le fichier n'est ni versionne, ni transmis, ni
 * conserve. Il coute en revanche un format, un decodeur, et surtout la
 * propriete dont depend tout l'instrument -- l'empreinte du chunk 3
 * serait alors celle de (pixels x codec x version de bibliotheque x
 * options x nombre de coeurs), et une empreinte fausse pour une raison
 * qui n'a rien a voir avec le jeu detruit la capacite de distinguer "le
 * code a change" de "ma machine a change". Le prix accepte a la place
 * est la taille : ~80 Mo pour l'events.json actuel, dans le repertoire
 * tmp/ deja ignore par git.
 *
 * REJETE : UN EN-TETE (nombre magique, version, dimensions, compteur de
 * frames). Il rendrait le fichier auto-descriptif, mais il ferait porter
 * l'empreinte du chunk 3 sur autre chose que les pixels, et il
 * introduirait un compteur de frames capable de mentir sur la taille
 * reelle du fichier -- alors que derivee de la taille, cette reponse ne
 * peut pas diverger de ce qui est ecrit. C'est aussi ce qui permet a
 * l'ecriture de ne jamais revenir en arriere : une capture interrompue
 * est un prefixe valide, et un prefixe tronque en milieu de frame est
 * detecte par la seule validation de taille ci-dessous.
 *
 * REJETE : que la lecture serve les frames a travers un curseur ("donne
 * moi la suivante"), comme le fait pico8_replay_source. Ce module la
 * n'a qu'un consommateur, une boucle de jeu qui avance d'une frame par
 * frame ; ici le visualiseur relit, recule, et le PRD exige de pouvoir
 * lire "y compris hors sequence". Un curseur aurait donc coexiste avec
 * un acces indexe, c'est a dire deux facons de designer la meme frame,
 * dont une seule aurait ete testee. L'index explicite est la seule.
 *
 * REJETE : que le module imprime un message et appelle exit() lui-meme
 * sur un fichier invalide ou une ecriture impossible. Meme raison que
 * pour pico8_replay_source : cela rendrait le module intestable
 * exactement la ou il importe, puisque le cas d'erreur tuerait le
 * processus de test, et cela enterrerait une decision de politique --
 * quoi imprimer, ou, avec quel code de sortie -- dans un module dont la
 * raison d'etre est d'etre une decision pure et verifiable. Les chunks
 * 2 et 4 possedent chacun leur message et leur code de sortie. */

#ifndef PICO8_SCREENCAST_H
#define PICO8_SCREENCAST_H

#include "pico8_gfx.h"
#include "pico8_mem.h"

#include <stdio.h>

/* Les 16 384 pixels d'une frame, un par octet : la taille du tampon que
 * pico8_screencast_read_frame remplit, et la representation que voit
 * tout le monde sauf ce module. */
#define PICO8_SCREENCAST_FRAME_PIXELS (PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT)

/* Les 8 192 octets qu'une frame occupe DANS LE FICHIER, deux pixels par
 * octet. La valeur est derivee des dimensions de l'ecran plutot
 * qu'ecrite en dur, mais le PRD fixe 8 192 comme une constante du
 * format : pico8_screencast.c en fait une assertion de compilation, de
 * sorte qu'un changement des dimensions de l'ecran casse la
 * construction au lieu de changer silencieusement le format. */
#define PICO8_SCREENCAST_FRAME_BYTES (PICO8_SCREENCAST_FRAME_PIXELS / 2)

/* La capture en cours d'ecriture. Un seul champ, le fichier ouvert :
 * ce module ne compte pas les frames ecrites parce que la seule reponse
 * qui ne peut pas mentir est la taille du fichier, que la moitie
 * lecture calcule deja. Ne pas ecrire ce champ directement. */
typedef struct {
    FILE *file;
} pico8_screencast_writer_t;

/* Commence une capture a `path`, en tronquant un fichier existant --
 * deux sessions concatenees dans un meme fichier seraient la capture
 * d'une partie que personne n'a jouee. Rend 1 si le fichier est ouvert,
 * 0 sinon (chemin invalide, repertoire absent, droits). L'appelant DOIT
 * regarder ce resultat : le chunk 2 en fait une erreur fatale, parce
 * qu'une capture qui n'a pas commence produirait une empreinte calculee
 * sur rien. Apres un echec, `*writer` reste sur qu'a fermer. */
int pico8_screencast_writer_open(pico8_screencast_writer_t *writer, const char *path);

/* Ajoute une frame a la capture, lue depuis la memoire ecran de `mem`
 * (PICO8_ADDR_SCREEN, le meme acces que pico8_dump_screen) et
 * immediatement empaquetee dans le format decrit en tete de ce fichier.
 * `mem` est const et rien ici n'ecrit dans le jeu : une capture qui
 * modifierait ce qu'elle observe invaliderait tout l'instrument (PRD,
 * user story 26).
 *
 * Rend 1 si les 8 192 octets ont ete ecrits, 0 sinon -- disque plein,
 * fichier ferme sous nos pieds. L'echec est rendu a l'appelant, et non
 * absorbe : c'est la politique INVERSE de pico8_session_recorder, qui
 * prefere perdre une frame plutot que tuer la partie d'un humain, et le
 * PRD dit pourquoi le contexte est inverse -- "personne ne joue, et un
 * fichier tronque produirait silencieusement une empreinte fausse,
 * c'est-a-dire un faux rouge indiscernable d'une regression". */
int pico8_screencast_write_frame(pico8_screencast_writer_t *writer, const pico8_mem_t *mem);

/* Termine la capture et rend 1 si la fermeture a reussi, 0 sinon. Le
 * resultat compte : fclose est le dernier endroit ou une ecriture
 * tamponnee peut echouer, et une capture dont la derniere frame n'a pas
 * atteint le disque est exactement le fichier tronque que la validation
 * de taille refusera plus tard. Sur un writer dont l'ouverture avait
 * echoue, ne fait rien et rend 1 ; appeler deux fois est sans effet. */
int pico8_screencast_writer_close(pico8_screencast_writer_t *writer);

/* Le sort d'une ouverture en lecture. Quatre valeurs et non un booleen,
 * parce que le PRD exige que le visualiseur "refuse clairement un
 * fichier tronque ou absent" (user story 27) : "une fenetre qui s'ouvre
 * sur du noir est indiscernable d'une session qui commence par un ecran
 * noir", et un appelant qui ne tiendrait qu'un booleen ne pourrait pas
 * nommer la raison. Le message est la seule interface humaine d'un
 * echec ; il appartient a l'appelant, la distinction appartient ici. */
typedef enum {
    /* Le fichier est ouvert et sa taille est un nombre entier et non nul
     * de frames. */
    PICO8_SCREENCAST_OK = 0,
    /* Absent, ou illisible. */
    PICO8_SCREENCAST_UNREADABLE = 1,
    /* Present et vide. Une valeur a part, et non un cas de taille
     * invalide : zero EST un multiple de 8 192, donc sans ce refus
     * explicite un fichier vide passerait pour une capture de zero
     * frame et le visualiseur ouvrirait une fenetre sur rien. */
    PICO8_SCREENCAST_EMPTY = 2,
    /* La taille n'est pas un multiple de PICO8_SCREENCAST_FRAME_BYTES :
     * le fichier se termine au milieu d'une frame, donc il a ete tronque
     * ou son ecriture a echoue en cours de route. */
    PICO8_SCREENCAST_TRUNCATED = 3
} pico8_screencast_status_t;

/* Une capture ouverte en lecture. `frame_count` est derive de la taille
 * du fichier a l'ouverture, une fois ; se lit par l'accesseur ci-dessous
 * et ne s'ecrit pas. Les deux champs ne valent quelque chose que si
 * l'ouverture a rendu PICO8_SCREENCAST_OK -- dans les trois autres cas
 * le fichier est deja referme et le compte est nul, de sorte que
 * pico8_screencast_reader_close soit valide quel que soit le sort. */
typedef struct {
    FILE *file;
    int frame_count;
} pico8_screencast_reader_t;

/* Ouvre une capture en lecture et valide sa taille, avant que le chunk
 * 4 n'ouvre une fenetre. Le fichier est ouvert en "rb" et JAMAIS en
 * ecriture : un aller-retour par la lecture ne doit pas pouvoir changer
 * un octet et donc invalider l'empreinte que le chunk 3 vient de poser
 * (PRD, user story 28). */
pico8_screencast_status_t pico8_screencast_reader_open(pico8_screencast_reader_t *reader, const char *path);

/* Combien de frames la capture contient, c'est a dire la taille du
 * fichier divisee par PICO8_SCREENCAST_FRAME_BYTES -- jamais un
 * compteur stocke dans le fichier, qu'il n'y a pas. 0 si l'ouverture a
 * echoue. C'est le "total" de la ligne d'etat du visualiseur, et la
 * borne des index acceptes ci-dessous. */
int pico8_screencast_frame_count(const pico8_screencast_reader_t *reader);

/* Lit la frame `index` (0 a frame_count-1) dans `out_pixels`, qui doit
 * pouvoir tenir PICO8_SCREENCAST_FRAME_PIXELS octets. Chaque octet rendu
 * est UN pixel, de valeur 0 a 15, dans l'ordre de balayage de l'ecran :
 * le depaquetage des quartets est fait ici, pour que personne d'autre
 * n'ait a connaitre le format. Rend 1 en cas de succes.
 *
 * Rend 0 et laisse `out_pixels` intact si l'index est hors des frames
 * du fichier, ou si la lecture echoue. Les index peuvent etre demandes
 * dans n'importe quel ordre et autant de fois qu'on veut : la frame N
 * est a l'offset N x 8192 et rien ici ne garde de position courante, ce
 * qui est ce dont le visualiseur a besoin pour reculer. */
int pico8_screencast_read_frame(pico8_screencast_reader_t *reader, int index, unsigned char *out_pixels);

/* Referme la capture et rend `*reader` a son etat d'avant l'ouverture.
 * Sur une ouverture qui avait echoue, et sur un reader deja ferme, ne
 * fait rien. Rien ne peut echouer ici : le fichier n'a ete ouvert qu'en
 * lecture, il n'y a aucune ecriture tamponnee a perdre. */
void pico8_screencast_reader_close(pico8_screencast_reader_t *reader);

#endif
