/* Places vivantes des zones interface et particules, sur rejeu d'un e2e de tests/e2e/
 * pilote comme main.c : apres une session, chaque sorte revient a son
 * niveau de depart, et aucune ne monte pendant une phase sans action.
 * Chaque compte est lu apres un vidage de la file de sortie. Un rejeu par
 * processus enfant : le cart n'est charge qu'une fois par processus. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <dirent.h>
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
#include "../src/pico8_entity.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_particle.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_zone_interface.h"
#include "../src/pico8_zone_particules.h"

#define AVANT_F0 (-1)
#define MAX_RELEVES 4

static const char *const NOMS[PICO8_SORTE_COUNT] = {"js", "fq", "mh", "iv", "rangee", "le"};

typedef struct {
    int frame;
    int vivantes[PICO8_SORTE_COUNT];
    int particule;
    int qy;
    int qy_fv;
    int ga;
} releve_t;

static void relever(releve_t *r, int frame) {
    pico8_cart_vider_file_sortie();
    r->frame = frame;
    for (int s = 0; s < PICO8_SORTE_COUNT; s++) {
        r->vivantes[s] = pico8_zone_interface_vivantes((pico8_sorte_t)s);
    }
    r->particule = pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE);
    r->qy = pico8_zone_particules_vivantes(PICO8_PARTICULES_QY);
    r->qy_fv = pico8_zone_particules_vivantes(PICO8_PARTICULES_QY_FV);
    r->ga = pico8_zone_particules_vivantes(PICO8_PARTICULES_GA);
}

/* Particules et listes tenues par les emetteurs inscrits dans `hv`, et
 * eclats inscrits : ce que les comptes de la zone particules doivent dire. */
typedef struct {
    int particules;
    int listes;
    int eclats;
} inscrits_t;

static inscrits_t compter_inscrits(void) {
    inscrits_t n = {0, 0, 0};
    const pico8_ents_t *hv = pico8_hv();
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        const pico8_struct_header_t *e = hv->items[i];
        if (e->emitter != NULL && e->emitter->qy != NULL) {
            n.listes++;
            n.particules += e->emitter->qy->count;
        }
        if (e->cls == pico8_ga_struct_class()) n.eclats++;
    }
    return n;
}

/* Lu apres un vidage : une particule, une liste ou un eclat perdu le casse
 * a la frame meme ou il l'est. */
static void verifier_invariant(const char *e2e, int frame) {
    releve_t r;
    relever(&r, frame);
    inscrits_t n = compter_inscrits();
    if (r.particule != n.particules || r.qy + r.qy_fv != n.listes || r.ga != n.eclats) {
        fprintf(stderr, "  %s f%d : particule=%d (inscrites %d) qy+qy_fv=%d (listes %d) ga=%d (eclats %d)\n", e2e, frame,
               r.particule, n.particules, r.qy + r.qy_fv, n.listes, r.ga, n.eclats);
        assert(0 && "une particule, une liste qy ou un eclat vit hors des emetteurs inscrits");
    }
}

static void imprimer(const char *e2e, const releve_t *r) {
    printf("  %s f%d :", e2e, r->frame);
    for (int s = 0; s < PICO8_SORTE_COUNT; s++) {
        printf(" %s=%d", NOMS[s], r->vivantes[s]);
    }
    printf(" particule=%d qy=%d qy_fv=%d ga=%d\n", r->particule, r->qy, r->qy_fv, r->ga);
}

/* Charge le cart pour rejouer `e2e` (NULL : aucune entree) et rend ses
 * entrees, statiques. */
static pico8_input_t *charger(const char *e2e) {
    static pico8_mem_t mem;
    static pico8_gfx_t gfx;
    static pico8_input_t input;
    char path[256];

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    if (e2e != NULL) {
        snprintf(path, sizeof path, "tests/e2e/%s.json", e2e);
        assert(pico8_input_load(&input, path));
    }
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);
    pico8_cart_load();
    pico8__init();
    return &input;
}

/* Releve les comptes apres chaque frame de `frames` (AVANT_F0 : apres
 * _init, avant la frame 0), et rend la derniere frame jouee. */
static void rejouer(const char *e2e, const int *frames, int n, releve_t *out) {
    pico8_input_t *input = charger(e2e);
    int k = 0;

    if (k < n && frames[k] == AVANT_F0) {
        relever(&out[k], AVANT_F0);
        imprimer(e2e, &out[k]);
        k++;
    }
    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (int i = 0; i < input->frame_count && k < n; i++) {
        pico8_dispatch_set_input(input);
        pico8_cheat_step(&cheat, input);
        pico8__update60();
        pico8__draw();
        if (frames[k] == i) {
            relever(&out[k], i);
            imprimer(e2e, &out[k]);
            k++;
        }
        if (i < input->frame_count - 1) pico8_input_step(input);
    }
    assert(k == n && "une frame de releve est hors du rejeu");
}

/* Rejoue `e2e` en entier, l'invariant verifie a chaque frame. */
static void rejouer_sous_invariant(const char *e2e) {
    pico8_input_t *input = charger(e2e);
    verifier_invariant(e2e, AVANT_F0);
    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (int i = 0; i < input->frame_count; i++) {
        pico8_dispatch_set_input(input);
        pico8_cheat_step(&cheat, input);
        pico8__update60();
        pico8__draw();
        verifier_invariant(e2e, i);
        if (i < input->frame_count - 1) pico8_input_step(input);
    }
}

/* Tout sauf `particule`, qu'un emetteur actif fait varier sans rien
 * perdre : l'invariant le suit frame par frame. */
static int memes_comptes(const releve_t *a, const releve_t *b) {
    return memcmp(a->vivantes, b->vivantes, sizeof a->vivantes) == 0 && a->qy == b->qy && a->qy_fv == b->qy_fv &&
           a->ga == b->ga;
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

static char g_e2e[128];

static void invariant_sur_l_e2e_courant(void) {
    rejouer_sous_invariant(g_e2e);
}

/* Chaque frame de chaque e2e de tests/e2e/ ; rend le nombre d'e2e. */
static int invariant_sur_tous_les_e2e(void) {
    DIR *dir = opendir("tests/e2e");
    struct dirent *ent;
    int n = 0;
    assert(dir != NULL);
    while ((ent = readdir(dir)) != NULL) {
        size_t len = strlen(ent->d_name);
        if (len <= 5 || strcmp(ent->d_name + len - 5, ".json") != 0) continue;
        assert(len - 5 < sizeof g_e2e);
        memcpy(g_e2e, ent->d_name, len - 5);
        g_e2e[len - 5] = '\0';
        dans_un_enfant(invariant_sur_l_e2e_courant);
        n++;
    }
    closedir(dir);
    return n;
}

/* f7 : « restart » ; f11 : ecran titre, avant le clic de f12. */
static void pause_restart_revient_au_titre(void) {
    const int frames[] = {AVANT_F0, 11};
    releve_t r[2];
    rejouer("pause_restart", frames, 2, r);
    assert(memes_comptes(&r[0], &r[1]) && "jl() doit rendre toutes les places qu'il abandonne");
    assert(r[0].qy_fv == 1 && r[0].qy == 0 && r[0].ga == 0);
    /* Aucune particule de l'aide ne meurt avant 115 frames : 5 par draw,
     * aucun draw avant f0, ceux de f7 a f11 pour l'aide du « restart ». */
    assert(r[0].particule == 0 && r[1].particule == 5 * 5 && "jl() doit rendre les particules de l'aide");
}

/* f42-90 : la boite de l'annee retenue par son lh, sans clic. */
static void message_box_hold_ne_bouge_pas_sans_action(void) {
    const int frames[] = {45, 90};
    releve_t r[2];
    rejouer("message_box_hold", frames, 2, r);
    assert(memes_comptes(&r[0], &r[1]) && r[0].particule == r[1].particule && "une sorte prend une place par tick");
}

/* f25-94 : « p40 » s'efface ; sa rangee rendue peut faire baisser `rangee`. */
static void empty_planet_fade_ne_monte_pas(void) {
    const int frames[] = {25, 94};
    releve_t r[2];
    rejouer("empty_planet_fade", frames, 2, r);
    assert(aucun_ne_monte(&r[0], &r[1]) && "une sorte monte pendant l'effacement de p40");
}

/* f56 : « harvest », p27 devient un eclat. A f139, sa troisieme particule
 * vit encore : on prolonge le rejeu sans action (la souris reste en
 * (100,62), sans clic) jusqu'a son extinction. */
static void tech_geoharvesting_burst_rend_l_eclat(void) {
    const int frames[] = {55, 56, 139};
    releve_t r[3];
    releve_t fin;
    int frame;
    rejouer("tech_geoharvesting_burst", frames, 3, r);
    assert(r[0].ga == 0 && r[1].ga == 1 && "le « harvest » fait un eclat");
    assert(r[1].particule - r[0].particule >= 140 && "un eclat nait avec 140 particules");
    fin = r[2];
    for (frame = 140; fin.ga != 0 && frame < 160; frame++) {
        pico8__update60();
        pico8__draw();
        relever(&fin, frame);
    }
    imprimer("tech_geoharvesting_burst", &fin);
    assert(fin.ga == 0 && "un eclat eteint doit rendre sa place");
    assert(fin.qy == r[0].qy && "un eclat eteint doit rendre sa liste");
}

/* Sans entree, l'aide emet 5 particules par draw et chacune vit 115
 * frames : au-dela, le compte plafonne. */
static void ecran_titre_sans_entree_atteint_un_regime(void) {
    int plafond[4] = {0, 0, 0, 0};
    charger(NULL);
    for (int i = 0; i < 400; i++) {
        pico8__update60();
        pico8__draw();
        verifier_invariant("titre", i);
        assert(pico8_zone_particules_vivantes(PICO8_PARTICULES_QY_FV) == 1 && "l'aide garde une seule liste");
        int p = pico8_zone_particules_vivantes(PICO8_PARTICULES_PARTICULE);
        if (p > plafond[i / 100]) plafond[i / 100] = p;
    }
    printf("  titre sans entree, particule max par fenetre de 100 frames : %d %d %d %d\n", plafond[0], plafond[1],
           plafond[2], plafond[3]);
    assert(plafond[3] <= plafond[2] && "le compte de particules de l'aide monte encore apres sa duree de vie");
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

/* La valeur de la ligne « <cle> <n> » du remplissage. */
static long valeur(const char *remplissage, const char *cle) {
    char motif[64];
    long n;
    snprintf(motif, sizeof motif, "\n%s ", cle);
    const char *l = strstr(remplissage, motif);
    assert(l != NULL && "cle absente du remplissage");
    assert(sscanf(l + strlen(motif), "%ld", &n) == 1);
    return n;
}

/* L'eclat de f56 meurt a f141, apres la fin du rejeu (f139), mais ses 140
 * particules naissent d'un coup et la plupart sont mortes a f139 : un
 * maximum lu en fin de session seulement resterait sous 140. */
static void le_rejeu_ecrit_le_pic_de_la_session(void) {
    remove(REMPLISSAGE);
    assert(system("PICO8_REMPLISSAGE=" REMPLISSAGE " " DRIVER " tests/e2e/tech_geoharvesting_burst.json " CAPTURE_A
                  " " DUMP_A " > /dev/null") == 0);
    size_t len;
    char *ecrit = lire_fichier(REMPLISSAGE, &len);
    long particule = valeur(ecrit, "particule");
    long max_particule = valeur(ecrit, "max_vivantes particule");
    printf("  tech_geoharvesting_burst : particule %ld en fin de session, max %ld ; ga %ld, max %ld\n", particule,
           max_particule, valeur(ecrit, "ga"), valeur(ecrit, "max_vivantes ga"));
    assert(valeur(ecrit, "max_vivantes ga") >= 1 && "le « harvest » fait un eclat");
    assert(max_particule >= 140 && particule < 140 && "le maximum se releve a chaque prise, pas en fin de session");
    free(ecrit);
    remove(REMPLISSAGE);
    remove(CAPTURE_A);
    remove(DUMP_A);
}

int main(void) {
    int e2e = invariant_sur_tous_les_e2e();
    printf("  invariant particule = somme des #qy inscrits : %d e2e, chaque frame\n", e2e);
    assert(e2e == 26 && "tests/e2e/ compte 26 rejeux");
    dans_un_enfant(pause_restart_revient_au_titre);
    dans_un_enfant(message_box_hold_ne_bouge_pas_sans_action);
    dans_un_enfant(empty_planet_fade_ne_monte_pas);
    dans_un_enfant(tech_geoharvesting_burst_rend_l_eclat);
    dans_un_enfant(ecran_titre_sans_entree_atteint_un_regime);
    dans_un_enfant(le_rejeu_ecrit_le_remplissage_en_fin_de_session);
    dans_un_enfant(le_rejeu_ecrit_le_pic_de_la_session);
    printf("test_pico8_places_vivantes: all tests passed\n");
    return 0;
}
