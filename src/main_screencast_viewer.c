/* Le visualiseur de screencast : une fenetre raylib qui joue le fichier
 * de pixels fabrique par `make screencast`, pour qu'un humain puisse
 * regarder une session.
 *
 * Il n'ouvre rien en ecriture, nulle part : un aller-retour par le
 * visualiseur qui changerait un octet ferait diverger l'empreinte de
 * `make e2e`. C'est pourquoi il n'appelle pas
 * pico8_raylib_backend_init()/present() -- init() ouvre un enregistrement
 * de session et present() lit la memoire d'un jeu qui tourne. Les deux
 * seules fonctions du backend qu'il emprunte sont celles qui ne recoivent
 * pas de backend, et leur signature est la garantie.
 *
 * Toutes les decisions sont ailleurs et toutes y sont testees : le format
 * et la lecture indexee dans pico8_screencast, le curseur de lecture et
 * la phrase de refus dans pico8_screencast_player, la vitesse et la ligne
 * d'etat dans pico8_replay_control, les seize couleurs dans
 * pico8_palette. Ce qui reste ici demande une fenetre, donc `make unit`
 * ne l'exerce pas. */

#include "pico8_screencast.h"
#include "pico8_screencast_player.h"
#include "pico8_palette.h"
#include "pico8_replay_control.h"
#include "pico8_raylib_backend.h"
#include "pico8_gfx.h"

#include "raylib.h"

#include <stdio.h>

/* Les pixels de la frame courante (un octet par pixel) et leur
 * conversion en couleurs. Statiques parce que 16 Ko + 64 Ko n'ont rien a
 * faire dans un cadre de pile. */
static unsigned char g_frame_pixels[PICO8_SCREENCAST_FRAME_PIXELS];
static Color g_frame_colors[PICO8_SCREENCAST_FRAME_PIXELS];

int main(int argc, char **argv) {
    /* Pas de chemin par defaut, volontairement : il ouvrirait une fenetre
     * sur une capture qui n'est pas celle que l'humain croit regarder, et
     * c'est sur cette confiance que repose la signature d'empreinte. */
    if (argc != 2) {
        fprintf(stderr,
                "screencast-viewer: usage : %s <fichier de pixels>\n"
                "screencast-viewer: le fabriquer : make screencast EVENTS=<events.json> (ecrit tmp/screencast.bin)\n",
                argv[0]);
        return 1;
    }
    const char *path = argv[1];

    /* Le refus arrive avant le moindre appel a raylib : une fenetre qui
     * s'ouvre sur du noir est indiscernable d'une session qui commence
     * par un ecran noir. */
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
    /* Au plus proche voisin : lisser rendrait flou exactement ce qu'un
     * humain vient verifier, le pixel. */
    SetTextureFilter(screen_texture, TEXTURE_FILTER_POINT);

    int exit_code = 0;
    while (!WindowShouldClose()) {
        pico8_raylib_backend_poll_replay_controls(&control);
        /* `replay_active` vaut toujours 1, y compris une fois la lecture
         * figee : il n'y a ici aucune partie vivante a qui rendre les 60
         * FPS du jeu, et passer 0 annulerait silencieusement l'echelle de
         * vitesse. */
        pico8_raylib_backend_set_frame_rate_cap(pico8_replay_control_target_fps(&control, 1));

        int index = pico8_screencast_player_current_frame(&player);
        if (!pico8_screencast_read_frame(&reader, index, g_frame_pixels)) {
            /* L'index vient du curseur et ne sort jamais du fichier :
             * arriver ici, c'est la lecture elle-meme qui a echoue. On
             * sort en erreur plutot que de continuer a afficher la frame
             * precedente, qui ferait passer une panne pour une image
             * figee. */
            fprintf(stderr, "screencast-viewer: lecture de la frame %d de '%s' impossible -- arret.\n", index, path);
            exit_code = 1;
            break;
        }

        for (int i = 0; i < PICO8_SCREENCAST_FRAME_PIXELS; i++) {
            pico8_rgb_t rgb = PICO8_PALETTE_RGB[g_frame_pixels[i] & 0x0f];
            g_frame_colors[i] = (Color){ rgb.r, rgb.g, rgb.b, 0xFF };
        }
        UpdateTexture(screen_texture, g_frame_colors);

        /* format_status compte les frames a partir de 1, `index` a partir
         * de 0 : d'ou le +1. */
        char status_line[PICO8_REPLAY_CONTROL_STATUS_MAX];
        pico8_replay_control_format_status(&control, index + 1, frame_count, status_line, sizeof status_line);

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureEx(screen_texture, (Vector2){ 0, 0 }, 0.0f, (float)PICO8_RAYLIB_BACKEND_SCALE, WHITE);
        /* Bandeau translucide : la ligne doit rester lisible quelle que
         * soit l'image qui defile dessous. */
        DrawRectangle(0, 0, PICO8_SCREEN_WIDTH * PICO8_RAYLIB_BACKEND_SCALE, 26, (Color){ 0, 0, 0, 190 });
        DrawText(status_line, 8, 5, 18, RAYWHITE);
        EndDrawing();

        /* Avance APRES l'affichage : la premiere frame du fichier est la
         * premiere montree. Aucune condition de fin ici -- le curseur se
         * fige tout seul sur la derniere frame. */
        pico8_screencast_player_advance(&player);
    }

    UnloadTexture(screen_texture);
    CloseWindow();
    pico8_screencast_reader_close(&reader);

    return exit_code;
}
