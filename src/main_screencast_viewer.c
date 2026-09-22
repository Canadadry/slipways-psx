/* Chunk 4 de docs/prd/triage/screencast-capture-and-viewer.md ("Le
 * visualiseur") : une fenetre raylib qui ouvre le fichier de pixels
 * fabrique par `make screencast` et le joue comme une video.
 *
 * A QUOI IL SERT, ET POURQUOI IL EST LE DERNIER CHUNK. Les chunks 1 a 3
 * forment deja un instrument complet : une capture, une empreinte
 * versionnee, et une suite qui rougit quand le rendu change. Il leur
 * manque exactement une chose, et c'est ce fichier : "personne ne peut
 * valider un fichier binaire de plusieurs centaines de megaoctets, mais
 * tout le monde peut regarder une session defiler". C'est ce qui rend
 * l'empreinte SIGNABLE -- mettre a jour events.json.sha256 doit vouloir
 * dire "j'ai regarde et j'accepte", et sans visualiseur cela ne veut
 * rien dire du tout.
 *
 * CE FICHIER NE SERA JAMAIS LANCE PAR UN AGENT, ET C'EST CE QUI DICTE
 * SA FORME. Il n'y a pas de serveur graphique dans les environnements
 * d'agent, raylib n'y est pas toujours vendorise, CLAUDE.md interdit
 * d'ouvrir une fenetre a ce stade, et un agent n'a de toute facon aucun
 * moyen de REGARDER une fenetre. Le PRD en tire une exigence de
 * decoupage plutot qu'une exigence de test : "tout ce qui est decidable
 * sans fenetre vit hors du module raylib (...) si une partie du
 * visualiseur n'est pas testable sans fenetre, c'est qu'elle est au
 * mauvais endroit". Ce qui reste ici est donc, litteralement : lire un
 * argument, ouvrir une fenetre, televerser une image, la dessiner, et
 * fermer. Toutes les decisions sont ailleurs, et toutes sont testees :
 *
 *   - le format et la lecture indexee : src/pico8_screencast.{c,h}
 *     (chunk 1), y compris le refus d'un fichier absent, vide ou
 *     tronque, que ce fichier ne refait pas ;
 *   - quelle frame afficher maintenant, et ce qu'on dit d'un fichier
 *     qu'on refuse : src/pico8_screencast_player.{c,h} ;
 *   - l'echelle de vitesse et la ligne d'etat : src/pico8_replay_
 *     control.{c,h}, deja ecrit et deja teste pour le rejeu du pilote
 *     interactif -- une seconde echelle de vitesse dans le meme depot
 *     serait exactement la divergence que ce depot combat ailleurs ;
 *   - les seize couleurs : src/pico8_palette.{c,h}.
 *
 * IL N'OUVRE JAMAIS LE FICHIER EN ECRITURE. pico8_screencast_reader_
 * open l'ouvre en "rb" et ce programme n'ecrit nulle part : ni dans le
 * fichier de pixels, ni ailleurs. C'est une exigence du PRD (user story
 * 28) et elle porte sur l'instrument entier -- un aller-retour par le
 * visualiseur qui changerait un seul octet invaliderait l'empreinte que
 * `make screencast-sign` vient de poser, c'est-a-dire ferait mentir la
 * seule chose que tout ce PRD produit.
 *
 * POURQUOI IL EMPRUNTE DEUX FONCTIONS AU PILOTE INTERACTIF plutot que
 * de relire trois touches lui-meme. pico8_raylib_backend_poll_replay_
 * controls() et pico8_raylib_backend_set_frame_rate_cap() sont les deux
 * seules fonctions de src/pico8_raylib_backend.c qui ne recoivent PAS
 * de backend -- leur signature est la garantie, comme le dit leur
 * declaration : elles ne peuvent atteindre ni l'enregistrement, ni le
 * fichier de marqueurs, ni la memoire du jeu, meme par accident. Les
 * appeler donne au visualiseur les MEMES trois touches que le rejeu
 * interactif par construction et non par coincidence ; les recopier ici
 * aurait cree une deuxieme table de touches, qu'une edition future
 * n'aurait corrigee que d'un cote. Le prix est que l'edition de liens
 * tire pico8_session_recorder.c et pico8_input.c, dont pas une ligne ne
 * s'execute ici : aucune fonction de ce programme n'ouvre ni n'ecrit
 * quoi que ce soit.
 *
 * REJETE : appeler pico8_raylib_backend_init()/present() pour presenter
 * l'image, ce qui aurait ete la reutilisation la plus litterale. Cela
 * n'etait pas possible et n'aurait pas ete souhaitable : init() ouvre un
 * ENREGISTREMENT de session (tmp/events_recorded_<horodatage>.json) et
 * cache le curseur de l'hote, donc un visualiseur qui l'appellerait
 * creerait un fichier a chaque ouverture -- dans un programme dont le
 * PRD exige qu'il n'ecrive rien ; et present() lit une pico8_mem_t,
 * c'est-a-dire la memoire d'un jeu qui tourne, alors qu'ici il n'y a pas
 * de jeu, seulement des pixels deja dessines. Ce qui est reutilise est
 * donc ce qui peut l'etre sans tirer un jeu entier derriere : l'echelle
 * (PICO8_RAYLIB_BACKEND_SCALE, le meme facteur 6 pour la meme image
 * 128x128 -- "il n'y a aucune raison de diverger"), la palette, le
 * filtrage au plus proche voisin, et la forme de la ligne d'etat. Le
 * televersement lui-meme est une quinzaine de lignes de raylib.
 *
 * REJETE : la navigation aleatoire (barre de defilement, saut a une
 * frame, chapitres depuis le fichier de marqueurs). Explicitement hors
 * perimetre du PRD ; le format brut et l'acces indexe du chunk 1 la
 * rendent triviale a ajouter le jour ou elle sera demandee. */

#include "pico8_screencast.h"
#include "pico8_screencast_player.h"
#include "pico8_palette.h"
#include "pico8_replay_control.h"
#include "pico8_raylib_backend.h"
#include "pico8_gfx.h"

#include "raylib.h"

#include <stdio.h>

/* Les pixels de la frame courante (un octet par pixel, tels que le
 * chunk 1 les depaquette) et leur conversion en couleurs. Statiques
 * plutot que sur la pile : 16 Ko + 64 Ko, ce qui n'a rien a faire dans
 * un cadre de pile, et un seul exemplaire suffit -- il n'y a qu'une
 * frame affichee a la fois. */
static unsigned char g_frame_pixels[PICO8_SCREENCAST_FRAME_PIXELS];
static Color g_frame_colors[PICO8_SCREENCAST_FRAME_PIXELS];

int main(int argc, char **argv) {
    /* Un seul argument positionnel, le fichier de pixels : ni option, ni
     * selecteur de fichier, ni chemin par defaut. Un chemin par defaut
     * serait le piege que tout ce chunk evite -- il ouvrirait une
     * fenetre sur une capture qui n'est pas celle que l'humain croit
     * regarder, et c'est precisement sur cette confiance-la qu'une
     * signature d'empreinte repose. */
    if (argc != 2) {
        fprintf(stderr,
                "screencast-viewer: usage : %s <fichier de pixels>\n"
                "screencast-viewer: le fabriquer : make screencast (ecrit tmp/screencast.bin)\n",
                argv[0]);
        return 1;
    }
    const char *path = argv[1];

    /* LE REFUS ARRIVE AVANT LA FENETRE, ET C'EST TOUT L'INTERET. Le PRD
     * dit pourquoi en une phrase : "une fenetre qui s'ouvre sur du noir
     * est indiscernable d'une session qui commence par un ecran noir".
     * Un fichier absent, illisible, vide, ou dont la taille n'est pas un
     * multiple de 8 192 octets est donc rejete ici, avec un message qui
     * nomme la cause et un code de sortie non nul -- avant le moindre
     * appel a raylib. La distinction entre les quatre cas vient du
     * chunk 1, qui la fait deja et qu'on ne refait pas ; la phrase vient
     * de pico8_screencast_player, ou elle est testee mot pour mot. */
    pico8_screencast_reader_t reader;
    pico8_screencast_status_t status = pico8_screencast_reader_open(&reader, path);
    if (status != PICO8_SCREENCAST_OK) {
        char refusal[PICO8_SCREENCAST_PLAYER_REFUSAL_MAX];
        pico8_screencast_player_format_refusal(status, path, refusal, sizeof refusal);
        fprintf(stderr, "%s\n", refusal);
        pico8_screencast_reader_close(&reader);
        return 1;
    }

    int frame_count = pico8_screencast_frame_count(&reader);

    pico8_screencast_player_t player;
    pico8_screencast_player_init(&player, frame_count);

    /* La meme echelle de vitesse que le rejeu du pilote interactif, au
     * meme defaut (4x) : une capture de 9 729 frames se regarde en
     * quarante secondes au lieu de deux minutes et demie, sans qu'on
     * cesse de pouvoir reconnaitre l'instant ou quelque chose cloche.
     * Voir src/pico8_replay_control.h pour l'echelle et son arbitrage. */
    pico8_replay_control_t control;
    pico8_replay_control_init(&control);

    fprintf(stderr,
            "screencast-viewer: '%s', %d frames, lecture en boucle.\n"
            "screencast-viewer: PAGE UP plus vite, PAGE DOWN moins vite, END sans plafond ; fermer la fenetre pour quitter.\n",
            path, frame_count);

    InitWindow(PICO8_SCREEN_WIDTH * PICO8_RAYLIB_BACKEND_SCALE, PICO8_SCREEN_HEIGHT * PICO8_RAYLIB_BACKEND_SCALE,
               "Slipways screencast viewer");

    Image blank = GenImageColor(PICO8_SCREEN_WIDTH, PICO8_SCREEN_HEIGHT, BLACK);
    Texture2D screen_texture = LoadTextureFromImage(blank);
    UnloadImage(blank);
    /* Au plus proche voisin, comme le pilote interactif : agrandir six
     * fois une image de 128x128 en lissant rendrait flou exactement ce
     * qu'un humain vient verifier, le pixel. */
    SetTextureFilter(screen_texture, TEXTURE_FILTER_POINT);

    int exit_code = 0;
    while (!WindowShouldClose()) {
        /* Les trois memes touches que le rejeu interactif, lues par la
         * meme fonction -- voir le commentaire en tete de fichier. */
        pico8_raylib_backend_poll_replay_controls(&control);
        /* `replay_active` vaut toujours 1 : ici la lecture ne s'epuise
         * jamais, elle reboucle, et il n'y a pas de "partie vivante" a
         * laquelle rendre les 60 FPS du jeu. Passer 0 ramenerait
         * silencieusement toute lecture a la vitesse du jeu et
         * annulerait l'echelle de vitesse. */
        pico8_raylib_backend_set_frame_rate_cap(pico8_replay_control_target_fps(&control, 1));

        int index = pico8_screencast_player_current_frame(&player);
        if (!pico8_screencast_read_frame(&reader, index, g_frame_pixels)) {
            /* L'index vient du curseur, qui ne sort jamais du fichier :
             * arriver ici veut dire que la lecture elle-meme a echoue
             * (fichier tronque pendant qu'on le regardait, erreur
             * disque). On le dit et on sort en erreur plutot que de
             * continuer a afficher la frame precedente, qui ferait
             * passer une panne pour une image figee. */
            fprintf(stderr, "screencast-viewer: lecture de la frame %d de '%s' impossible -- arret.\n", index, path);
            exit_code = 1;
            break;
        }

        for (int i = 0; i < PICO8_SCREENCAST_FRAME_PIXELS; i++) {
            /* La palette partagee, jamais une copie locale (PRD) ; le
             * masque reproduit celui du pilote interactif, et le chunk 1
             * ne rend de toute facon que des quartets. */
            pico8_rgb_t rgb = PICO8_PALETTE_RGB[g_frame_pixels[i] & 0x0f];
            g_frame_colors[i] = (Color){ rgb.r, rgb.g, rgb.b, 0xFF };
        }
        UpdateTexture(screen_texture, g_frame_colors);

        /* "frame courante / total, vitesse", par la fonction qui compose
         * deja cette ligne pour le rejeu interactif et dont le texte est
         * teste. Elle compte les frames a partir de 1 -- la numerotation
         * dans laquelle un humain compte -- d'ou le +1 sur l'index, qui
         * est celui du fichier. User story 14 : pouvoir citer un numero
         * de frame, et ne pas croire le programme fige. */
        char status_line[PICO8_REPLAY_CONTROL_STATUS_MAX];
        pico8_replay_control_format_status(&control, index + 1, frame_count, status_line, sizeof status_line);

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureEx(screen_texture, (Vector2){ 0, 0 }, 0.0f, (float)PICO8_RAYLIB_BACKEND_SCALE, WHITE);
        /* Bandeau translucide sous le texte, comme dans le pilote
         * interactif : la ligne doit rester lisible quelle que soit
         * l'image capturee qui defile dessous. */
        DrawRectangle(0, 0, PICO8_SCREEN_WIDTH * PICO8_RAYLIB_BACKEND_SCALE, 26, (Color){ 0, 0, 0, 190 });
        DrawText(status_line, 8, 5, 18, RAYWHITE);
        EndDrawing();

        /* Avance APRES l'affichage : la premiere frame du fichier est la
         * premiere montree. Le rebouclage en fin de fichier est dans le
         * curseur, ou il est teste. */
        pico8_screencast_player_advance(&player);
    }

    UnloadTexture(screen_texture);
    CloseWindow();
    pico8_screencast_reader_close(&reader);

    return exit_code;
}
