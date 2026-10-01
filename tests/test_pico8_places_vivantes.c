/* Places vivantes de la zone interface, sur rejeu d'un e2e de tests/e2e/
 * pilote comme main.c : apres une session, chaque sorte revient a son
 * niveau de depart, et aucune ne monte pendant une phase sans action.
 * Chaque compte est lu apres un vidage de la file de sortie. Un rejeu par
 * processus enfant : le cart n'est charge qu'une fois par processus. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_cheat.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_zone_interface.h"
#include "../src/pico8_zone_particules.h"

#define AVANT_F0 (-1)
#define MAX_RELEVES 4

static const char *const NOMS[PICO8_SORTE_COUNT] = {"js", "fq", "mh", "iv", "rangee", "le"};

typedef struct {
    int frame;
    int vivantes[PICO8_SORTE_COUNT];
} releve_t;

static void relever(releve_t *r, int frame) {
    pico8_cart_vider_file_sortie();
    r->frame = frame;
    for (int s = 0; s < PICO8_SORTE_COUNT; s++) {
        r->vivantes[s] = pico8_zone_interface_vivantes((pico8_sorte_t)s);
    }
}

static void imprimer(const char *e2e, const releve_t *r) {
    printf("  %s f%d :", e2e, r->frame);
    for (int s = 0; s < PICO8_SORTE_COUNT; s++) {
        printf(" %s=%d", NOMS[s], r->vivantes[s]);
    }
    printf("\n");
}

/* Releve les comptes apres chaque frame de `frames` (AVANT_F0 : apres
 * _init, avant la frame 0), et rend la derniere frame jouee. */
static void rejouer(const char *e2e, const int *frames, int n, releve_t *out) {
    static pico8_mem_t mem;
    static pico8_gfx_t gfx;
    static pico8_input_t input;
    char path[256];
    int k = 0;

    snprintf(path, sizeof path, "tests/e2e/%s.json", e2e);
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    assert(pico8_input_load(&input, path));
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);
    pico8_cart_load();
    pico8__init();

    if (k < n && frames[k] == AVANT_F0) {
        relever(&out[k], AVANT_F0);
        imprimer(e2e, &out[k]);
        k++;
    }
    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (int i = 0; i < input.frame_count && k < n; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();
        if (frames[k] == i) {
            relever(&out[k], i);
            imprimer(e2e, &out[k]);
            k++;
        }
        if (i < input.frame_count - 1) pico8_input_step(&input);
    }
    assert(k == n && "une frame de releve est hors du rejeu");
}

static int memes_comptes(const releve_t *a, const releve_t *b) {
    return memcmp(a->vivantes, b->vivantes, sizeof a->vivantes) == 0;
}

static int aucun_ne_monte(const releve_t *a, const releve_t *b) {
    for (int s = 0; s < PICO8_SORTE_COUNT; s++) {
        if (b->vivantes[s] > a->vivantes[s]) return 0;
    }
    return 1;
}

static void dans_un_enfant(void (*test)(void)) {
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        test();
        fflush(stdout);
        _exit(0);
    }
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0 && "un releve de places vivantes a echoue");
}

/* f7 : « restart » ; f11 : ecran titre, avant le clic de f12. */
static void pause_restart_revient_au_titre(void) {
    const int frames[] = {AVANT_F0, 11};
    releve_t r[2];
    rejouer("pause_restart", frames, 2, r);
    assert(memes_comptes(&r[0], &r[1]) && "jl() doit rendre toutes les places qu'il abandonne");
}

/* f42-90 : la boite de l'annee retenue par son lh, sans clic. */
static void message_box_hold_ne_bouge_pas_sans_action(void) {
    const int frames[] = {45, 90};
    releve_t r[2];
    rejouer("message_box_hold", frames, 2, r);
    assert(memes_comptes(&r[0], &r[1]) && "une sorte prend une place par tick");
}

/* f25-94 : « p40 » s'efface ; sa rangee rendue peut faire baisser `rangee`. */
static void empty_planet_fade_ne_monte_pas(void) {
    const int frames[] = {25, 94};
    releve_t r[2];
    rejouer("empty_planet_fade", frames, 2, r);
    assert(aucun_ne_monte(&r[0], &r[1]) && "une sorte monte pendant l'effacement de p40");
}

/* Tampon alloue, termine par un nul, que l'appelant libere. */
static char *lire_fichier(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    assert(f != NULL);
    assert(fseek(f, 0, SEEK_END) == 0);
    long size = ftell(f);
    assert(size >= 0);
    rewind(f);
    char *buf = malloc((size_t)size + 1);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)size, f) == (size_t)size);
    buf[size] = '\0';
    fclose(f);
    *len = (size_t)size;
    return buf;
}

static int memes_octets(const char *a_path, const char *b_path) {
    size_t a_len, b_len;
    char *a = lire_fichier(a_path, &a_len);
    char *b = lire_fichier(b_path, &b_len);
    int same = a_len == b_len && memcmp(a, b, a_len) == 0;
    free(a);
    free(b);
    return same;
}

#define DRIVER "./tmp/pico8_replay"
#define REMPLISSAGE "tmp/test_places_vivantes_remplissage.txt"
#define CAPTURE_A "tmp/test_places_vivantes_a.bin"
#define CAPTURE_B "tmp/test_places_vivantes_b.bin"
#define DUMP_A "tmp/test_places_vivantes_a.txt"
#define DUMP_B "tmp/test_places_vivantes_b.txt"

/* PICO8_REMPLISSAGE=<fichier> : le rejeu y ecrit une ligne « <sorte>
 * <vivantes> » par sorte d'interface puis de particules, et les maxima de
 * la session par sorte de particules, en fin de session, sans toucher
 * capture, dump ni code de sortie. */
static void le_rejeu_ecrit_le_remplissage_en_fin_de_session(void) {
    const int frames[] = {17};
    releve_t r[1];
    rejouer("pause_restart", frames, 1, r);

    remove(REMPLISSAGE);
    assert(system(DRIVER " tests/e2e/pause_restart.json " CAPTURE_A " " DUMP_A " > /dev/null") == 0);
    assert(system("PICO8_REMPLISSAGE=" REMPLISSAGE " " DRIVER " tests/e2e/pause_restart.json " CAPTURE_B " " DUMP_B
                  " > /dev/null") == 0);
    assert(memes_octets(CAPTURE_A, CAPTURE_B) && "le remplissage ne change pas la capture");
    assert(memes_octets(DUMP_A, DUMP_B) && "le remplissage ne change pas le dump");

    char attendu[1024] = "";
    char ligne[48];
    for (int s = 0; s < PICO8_SORTE_COUNT; s++) {
        snprintf(ligne, sizeof ligne, "%s %d\n", NOMS[s], r[0].vivantes[s]);
        strcat(attendu, ligne);
    }
    static const char *const PARTICULES[PICO8_PARTICULES_COUNT] = {"particule", "qy", "qy_fv", "ga"};
    for (int s = 0; s < PICO8_PARTICULES_COUNT; s++) {
        snprintf(ligne, sizeof ligne, "%s %d\n", PARTICULES[s],
                 pico8_zone_particules_vivantes((pico8_particules_sorte_t)s));
        strcat(attendu, ligne);
    }
    for (int s = 0; s < PICO8_PARTICULES_COUNT; s++) {
        snprintf(ligne, sizeof ligne, "max_vivantes %s %d\n", PARTICULES[s],
                 pico8_zone_particules_max_vivantes((pico8_particules_sorte_t)s));
        strcat(attendu, ligne);
    }
    for (int s = PICO8_PARTICULES_QY; s < PICO8_PARTICULES_COUNT; s++) {
        snprintf(ligne, sizeof ligne, "max_octets %s %lu\n", PARTICULES[s],
                 (unsigned long)pico8_zone_particules_max_octets((pico8_particules_sorte_t)s));
        strcat(attendu, ligne);
    }
    for (int s = PICO8_PARTICULES_QY; s <= PICO8_PARTICULES_QY_FV; s++) {
        snprintf(ligne, sizeof ligne, "max_elements %s %d\n", PARTICULES[s],
                 pico8_zone_particules_max_elements((pico8_particules_sorte_t)s));
        strcat(attendu, ligne);
    }
    size_t len;
    char *ecrit = lire_fichier(REMPLISSAGE, &len);
    printf("  remplissage ecrit par le rejeu :\n%s", ecrit);
    assert(strcmp(ecrit, attendu) == 0 && "le rejeu ecrit les comptes de la derniere frame, apres vidage");
    free(ecrit);
    remove(REMPLISSAGE);
    remove(CAPTURE_A);
    remove(CAPTURE_B);
    remove(DUMP_A);
    remove(DUMP_B);
}

int main(void) {
    dans_un_enfant(pause_restart_revient_au_titre);
    dans_un_enfant(message_box_hold_ne_bouge_pas_sans_action);
    dans_un_enfant(empty_planet_fade_ne_monte_pas);
    dans_un_enfant(le_rejeu_ecrit_le_remplissage_en_fin_de_session);
    printf("test_pico8_places_vivantes: all tests passed\n");
    return 0;
}
