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
#include "../src/pico8_entity_geom.h"
#include "../src/pico8_game_data.h"
#include "../src/pico8_gen.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_particle.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_slipway.h"
#include "../src/pico8_trade.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_zone_interface.h"
#include "../src/pico8_zone_particules.h"
#include "../src/pico8_zone_partie.h"

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
    int ts;
    size_t partie;
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
    r->ts = pico8_zone_particules_vivantes(PICO8_PARTICULES_TS);
    r->partie = pico8_zone_partie_prise();
}

/* Particules et listes tenues par les emetteurs inscrits dans `hv`, eclats
 * inscrits, trainees des slipways de `cg.jc` : ce que les comptes de la
 * zone particules doivent dire. */
typedef struct {
    int particules;
    int listes;
    int eclats;
    int trainees;
} inscrits_t;

static inscrits_t compter_inscrits(void) {
    inscrits_t n = {0, 0, 0, 0};
    const pico8_ents_t *hv = pico8_hv();
    for (int i = 0; hv != NULL && i < hv->count; i++) {
        const pico8_struct_header_t *e = hv->items[i];
        if (e->emitter != NULL && e->emitter->qy != NULL) {
            n.listes++;
            n.particules += e->emitter->qy->count;
        }
        if (e->cls == pico8_ga_struct_class()) n.eclats++;
    }
    const pico8_ents_t *jc = pico8_cg("jc");
    for (int i = 0; jc != NULL && i < jc->count; i++) {
        const pico8_jc_t *s = (const pico8_jc_t *)jc->items[i];
        if (s->ts != NULL) n.trainees += s->ts->count;
    }
    return n;
}

/* Lu apres un vidage : une particule, une liste ou un eclat perdu le casse
 * a la frame meme ou il l'est. */
static void verifier_invariant(const char *e2e, int frame) {
    releve_t r;
    relever(&r, frame);
    inscrits_t n = compter_inscrits();
    if (r.particule != n.particules || r.qy + r.qy_fv != n.listes || r.ga != n.eclats || r.ts != n.trainees) {
        fprintf(stderr, "  %s f%d : particule=%d (inscrites %d) qy+qy_fv=%d (listes %d) ga=%d (eclats %d) ts=%d (trainees %d)\n",
                e2e, frame, r.particule, n.particules, r.qy + r.qy_fv, n.listes, r.ga, n.eclats, r.ts, n.trainees);
        assert(0 && "une particule, une liste qy, un eclat ou une trainee vit hors des emetteurs inscrits ou de cg.jc");
    }
}

static void imprimer(const char *e2e, const releve_t *r) {
    printf("  %s f%d :", e2e, r->frame);
    for (int s = 0; s < PICO8_SORTE_COUNT; s++) {
        printf(" %s=%d", NOMS[s], r->vivantes[s]);
    }
    printf(" particule=%d qy=%d qy_fv=%d ga=%d partie=%lu\n", r->particule, r->qy, r->qy_fv, r->ga,
           (unsigned long)r->partie);
}

/* Charge le cart pour rejouer `e2e` (NULL : aucune entree ; un chemin s'il
 * contient un « / ») et rend ses entrees, statiques. */
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
        snprintf(path, sizeof path, strchr(e2e, '/') != NULL ? "%s" : "tests/e2e/%s.json", e2e);
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

#define ASAN_LOG "tmp/test_places_vivantes_asan.txt"

static char *lire_fichier(const char *path, size_t *len);

/* L'enfant doit mourir par ASan en relisant un bloc rendu. */
static void dans_un_enfant_qui_meurt_par_asan(void (*test)(void)) {
    fflush(stdout);
    remove(ASAN_LOG);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        assert(freopen(ASAN_LOG, "w", stderr) != NULL);
        test();
        _exit(0);
    }
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    size_t len;
    char *err = lire_fichier(ASAN_LOG, &len);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        fprintf(stderr, "%s", err);
        assert(0 && "le bloc rendu se relit : la generation n'a pas ete rendue");
    }
    if (strstr(err, "use-after-poison") == NULL) {
        fprintf(stderr, "%s", err);
        assert(0 && "attendu use-after-poison sur le bloc rendu");
    }
    printf("  pause_restart f13 : le bloc relu plante (use-after-poison)\n");
    free(err);
    remove(ASAN_LOG);
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

/* volatile : la lecture d'une sentinelle rendue ne doit pas etre retiree
 * par l'optimiseur, c'est elle que le cas observe. */
static volatile unsigned char g_lu;

static pico8_input_t *g_input;
static pico8_cheat_t g_cheat;
static int g_frame;

/* Sans `relever` : son vidage de la file rendrait la generation lui-meme,
 * a la place de l'update60 de la frame qui suit le clic. */
static void jouer_jusqu_a(int derniere) {
    for (; g_frame <= derniere && g_frame < g_input->frame_count; g_frame++) {
        pico8_dispatch_set_input(g_input);
        pico8_cheat_step(&g_cheat, g_input);
        pico8__update60();
        pico8__draw();
        if (g_frame < g_input->frame_count - 1) pico8_input_step(g_input);
    }
    assert(g_frame == derniere + 1 && "une frame de sentinelle est hors du rejeu");
}

static void demarrer(const char *e2e) {
    g_input = charger(e2e);
    pico8_cheat_init(&g_cheat);
    g_frame = 0;
}

static unsigned char *sentinelle(void) {
    unsigned char *s = pico8_zone_alloc(pico8_zone_partie(), 64);
    for (int i = 0; i < 64; i++) s[i] = (unsigned char)i;
    return s;
}

static void verifier_sentinelle(const unsigned char *s) {
    for (int i = 0; i < 64; i++) assert(s[i] == i && "la sentinelle de la zone partie a change");
}

/* f12 : clic « challenging », donc `eu` : la zone bascule, la generation
 * de la partie quittee (et du titre de f7) vit jusqu'a la fin de la frame. */
static void pause_restart_garde_la_partie_quittee_pendant_le_clic(void) {
    demarrer("pause_restart");
    jouer_jusqu_a(11);
    assert(!pico8_zone_partie_en_attente());
    unsigned char *s = sentinelle();
    size_t prise = pico8_zone_partie_prise();
    jouer_jusqu_a(12);
    assert(pico8_zone_partie_en_attente() && "le clic de f12 fait basculer la zone partie");
    verifier_sentinelle(s);
    assert(pico8_zone_partie_prise() >= prise && "les octets pris comptent la partie quittee jusqu'a son rendu");
    printf("  pause_restart f12 : partie=%lu, sentinelle lue\n", (unsigned long)pico8_zone_partie_prise());
}

/* f13 : l'update60 vide la file puis rend la generation quittee. */
static void pause_restart_rend_la_partie_quittee_a_la_frame_suivante(void) {
    demarrer("pause_restart");
    jouer_jusqu_a(11);
    unsigned char *s = sentinelle();
    jouer_jusqu_a(13);
    g_lu = s[63];
}

static const pico8_jo_t *planete_de_la_premiere_partie(void) {
    demarrer("pause_restart");
    jouer_jusqu_a(6);
    const pico8_ents_t *jo = pico8_cg("jo");
    assert(jo != NULL && jo->count > 0 && "la premiere partie a des planetes");
    return (const pico8_jo_t *)jo->items[0];
}

/* f12 : le clic bascule ; les planetes de la partie quittee vivent encore. */
static void pause_restart_garde_les_planetes_de_la_partie_quittee_pendant_le_clic(void) {
    const pico8_jo_t *p = planete_de_la_premiere_partie();
    jouer_jusqu_a(12);
    assert(pico8_zone_partie_en_attente());
    g_lu = (unsigned char)p->has_jh;
}

/* f13 : elles meurent avec leur generation. */
static void pause_restart_rend_les_planetes_de_la_partie_quittee(void) {
    const pico8_jo_t *p = planete_de_la_premiere_partie();
    jouer_jusqu_a(13);
    g_lu = (unsigned char)p->has_jh;
}

/* f7 : « restart » (`_init`) ne bascule pas. */
static void pause_restart_garde_la_partie_a_travers_restart(void) {
    demarrer("pause_restart");
    jouer_jusqu_a(6);
    unsigned char *s = sentinelle();
    jouer_jusqu_a(11);
    assert(!pico8_zone_partie_en_attente() && "« restart » ne fait pas basculer la zone partie");
    verifier_sentinelle(s);
}

/* Octets que le bw de chaque eu prend dans sa generation : jo.oc, sa
 * source et jo.ir. */
static size_t etats_de_bw(void) {
    return (2 * PICO8_ZONE_CELLS(sizeof(pico8_jf_state_t)) + PICO8_ZONE_CELLS(sizeof(pico8_gn_state_t))) *
           sizeof(pico8_zone_cell_t);
}

/* Octets qu'une liste a pris dans sa generation depuis un jl() : les
 * capacites 4, 8, ... jusqu'a la sienne, car chaque agrandissement copie et
 * laisse l'ancien bloc mort jusqu'au rendu. NULL (un `cg` jamais vu) : rien. */
static size_t octets_d_un_registre(const pico8_ents_t *liste) {
    size_t octets = 0;
    for (int k = 4; liste != NULL && k <= liste->capacity; k *= 2) {
        octets += PICO8_ZONE_CELLS((size_t)k * sizeof(pico8_struct_header_t *)) * sizeof(pico8_zone_cell_t);
    }
    return octets;
}

static size_t octets_des_registres(void) {
    static const char *const CV[] = {"kj", "ev", "hd", "hk", "hb"};
    static const char *const CG[] = {"qh", "hj", "nr", "g_", "mh", "jc", "mz", "jo", "np", "prober"};
    size_t r = octets_d_un_registre(pico8_hv());
    for (size_t i = 0; i < sizeof CV / sizeof CV[0]; i++) r += octets_d_un_registre(pico8_cv(CV[i]));
    for (size_t i = 0; i < sizeof CG / sizeof CG[0]; i++) r += octets_d_un_registre(pico8_cg(CG[i]));
    return r;
}

static size_t octets_d_un_bloc(size_t taille) {
    return PICO8_ZONE_CELLS(taille) * sizeof(pico8_zone_cell_t);
}

/* Octets d'une liste de planete faite dans la generation : son conteneur,
 * puis ses blocs comme un registre. */
static size_t octets_d_une_liste_de_planete(const pico8_ents_t *liste) {
    if (liste == NULL) return 0;
    return octets_d_un_bloc(sizeof *liste) + octets_d_un_registre(liste);
}

/* Empreinte `P` des planetes de `cg.jo` : structure, `p`, `ra`, identifiant
 * et listes `oe`, `by`, `cc`. Sans `jy` ni copies de besoins, que les
 * planetes de `bw` n'ont pas avant une action. */
static size_t octets_des_planetes(void) {
    const pico8_ents_t *jo = pico8_cg("jo");
    size_t octets = 0;
    for (int i = 0; jo != NULL && i < jo->count; i++) {
        const pico8_jo_t *p = (const pico8_jo_t *)jo->items[i];
        assert(p->jy == NULL && "une planete a des besoins, que P ne compte pas");
        octets += octets_d_un_bloc(sizeof *p) + octets_d_un_bloc(sizeof *p->p) + octets_d_un_bloc(sizeof *p->ra) +
                  octets_d_un_bloc(strlen(p->id) + 1) + octets_d_une_liste_de_planete(p->oe) +
                  octets_d_une_liste_de_planete(p->by) + octets_d_une_liste_de_planete(p->cc);
    }
    return octets;
}

/* Apres le rendu de f13, la generation ne tient que ce qu'a fait le eu de
 * f12 et ce qui a grandi depuis : les etats de bw, les registres, blocs
 * morts compris, et ses planetes. */
static void pause_restart_borne_les_blocs_morts_des_registres(void) {
    const size_t g = etats_de_bw();
    demarrer("pause_restart");
    jouer_jusqu_a(13);
    const size_t r = octets_des_registres();
    const size_t p = octets_des_planetes();
    printf("  pause_restart f13 : partie=%lu, etats de bw=%lu, registres=%lu, planetes=%lu\n",
           (unsigned long)pico8_zone_partie_prise(), (unsigned long)g, (unsigned long)r, (unsigned long)p);
    assert(pico8_zone_partie_prise() == g + r + p && "les registres prennent 2C - 4 pointeurs, et rien d'autre n'entre");
}

/* f12 : le bw de la nouvelle partie prend ses etats a cote de ceux de la
 * partie quittee, rendus a f13. */
static void pause_restart_compte_les_etats_de_bw(void) {
    const size_t g = etats_de_bw();
    demarrer("pause_restart");
    jouer_jusqu_a(11);
    const size_t f11 = pico8_zone_partie_prise();
    assert(f11 >= g && "une partie : une generation d'etats de bw");
    jouer_jusqu_a(12);
    const size_t f12 = pico8_zone_partie_prise();
    assert(f12 >= f11 + g && "les etats des deux parties vivent jusqu'au rendu");
    jouer_jusqu_a(13);
    assert(pico8_zone_partie_prise() == g + octets_des_registres() + octets_des_planetes() &&
           "le rendu de f13 laisse les etats, les registres et les planetes de la nouvelle partie");
    assert(pico8_zone_partie_max() >= f12);
    printf("  pause_restart f11/f12/f13 : partie=%lu/%lu/%lu, max=%lu\n", (unsigned long)f11, (unsigned long)f12,
           (unsigned long)pico8_zone_partie_prise(), (unsigned long)pico8_zone_partie_max());
}

/* Octets de la generation hors registres, que `hv` peut faire grandir en
 * inscrivant des entites. */
static size_t partie_sans_registres(void) {
    return pico8_zone_partie_prise() - octets_des_registres();
}

/* f3-51 : 25 sauts d'annee, donc autant de `bd`, qui ne doivent pas faire
 * grandir la generation : `cc` est videe en place. */
static void game_end_final_score_bd_ne_fait_pas_grandir_la_partie(void) {
    demarrer("game_end_final_score");
    jouer_jusqu_a(2);
    const size_t f2 = partie_sans_registres();
    jouer_jusqu_a(51);
    const size_t f51 = partie_sans_registres();
    printf("  game_end_final_score f2/f51 : partie - registres = %lu/%lu\n", (unsigned long)f2, (unsigned long)f51);
    assert(f2 == f51 && "un bd fait grandir la generation");
}

/* f140-151 : saut d'annee de f140 sur sept slipways poses, donc `jo:ib`,
 * `jo:cn` et `nr:cn`, qui reproposent des routes connues. Le `bd` de f139
 * garde, lui, la route du slipway pose a f136. Le slipway refuse du clic
 * de f142 prend une arene, que celui de f148 reprend. */
static void trade_rich_mine_bd_sur_des_routes_ne_fait_pas_grandir_la_partie(void) {
    demarrer("trade_rich_mine");
    jouer_jusqu_a(139);
    const size_t f139 = partie_sans_registres();
    jouer_jusqu_a(151);
    const size_t f151 = partie_sans_registres();
    printf("  trade_rich_mine f139/f151 : partie - registres = %lu/%lu\n", (unsigned long)f139,
           (unsigned long)f151);
    assert(f151 == f139 + PICO8_JO_PRISE_OCTETS &&
           "un bd sur des routes fait grandir la generation, ou le refus de f148 ne reprend pas l'arene de f142");
}

static const pico8_jo_t *planete_d_id(const char *id) {
    const pico8_ents_t *np = pico8_cg("np");
    for (int i = 0; np != NULL && i < np->count; i++) {
        const pico8_jo_t *p = (const pico8_jo_t *)np->items[i];
        if (p->id != NULL && strcmp(p->id, id) == 0) return p;
    }
    assert(0 && "planete introuvable dans cg.np");
    return NULL;
}

/* Une route de `by` gardee par le `bd` de f139 se relit a f151, apres douze
 * remises a zero de la frame ou ses candidates sont faites. */
static void trade_rich_mine_une_route_gardee_survit_a_la_frame(void) {
    pico8_struct_header_t *lp[16];
    pico8_struct_header_t *rl[16];
    demarrer("trade_rich_mine");
    jouer_jusqu_a(139);
    const pico8_ents_t *by = planete_d_id("p27")->by;
    assert(by != NULL && by->count > 0 && "f139 : p27 garde une route");
    const pico8_trade_t *t = (const pico8_trade_t *)by->items[by->count - 1];
    const int nlp = t->lp->count;
    const int nrl = t->rl->count;
    assert(nlp <= 16 && nrl <= 16);
    memcpy(lp, t->lp->items, (size_t)nlp * sizeof *lp);
    memcpy(rl, t->rl->items, (size_t)nrl * sizeof *rl);
    jouer_jusqu_a(151);
    assert(t->lp->count == nlp && memcmp(lp, t->lp->items, (size_t)nlp * sizeof *lp) == 0);
    assert(t->rl->count == nrl && memcmp(rl, t->rl->items, (size_t)nrl * sizeof *rl) == 0);
    printf("  trade_rich_mine : route de p27 (%d planetes, %d slipways) relue a f151\n", nlp, nrl);
}

/* Les trainees encore vivantes en fin de partie reviennent a la zone
 * particules par le `rendre` des slipways, au chargement suivant. */
static void trade_rich_mine_rend_ses_trainees_au_rechargement(void) {
    demarrer("trade_rich_mine");
    jouer_jusqu_a(g_input->frame_count - 1);
    const int fin = pico8_zone_particules_vivantes(PICO8_PARTICULES_TS);
    pico8_cart_load();
    const int apres = pico8_zone_particules_vivantes(PICO8_PARTICULES_TS);
    printf("  trade_rich_mine : ts %d en fin de rejeu, %d apres rechargement (max %d, liste max %d)\n", fin, apres,
           pico8_zone_particules_max_vivantes(PICO8_PARTICULES_TS),
           pico8_zone_particules_max_elements(PICO8_PARTICULES_TS));
    assert(fin > 0 && "trade_rich_mine finit avec des trainees vivantes");
    assert(apres == 0 && "le rendre du slipway rend ses trainees");
}

#define PRISES "tests/fixtures/prises_refusees.json"

/* Fixture PRISES : f0-20 d'empty_planet_fade ; puis des prises de 7 frames
 * (pression sur l'icone (122,124), glisse, relache, trois d'attente) :
 * vingt refusees sur p34 (f21-160), une posee (f161-167), une refusee
 * (f168-174) ; « restart » et nouvelle partie (clic f185) ; une prise
 * (f189-195). */
static void prises_refusees_ne_font_pas_grandir_la_partie(void) {
    demarrer(PRISES);
    jouer_jusqu_a(27);
    const size_t une = partie_sans_registres();
    jouer_jusqu_a(160);
    const size_t vingt = partie_sans_registres();
    printf("  prises refusees 1/20 : partie - registres = %lu/%lu\n", (unsigned long)une, (unsigned long)vingt);
    assert(une == vingt && "une prise refusee laisse sa structure dans la generation");
}

/* La structure posee de f161-164, seule planete « s… » de `cg.jo`. */
static const pico8_jo_t *la_prise_posee(void) {
    const pico8_ents_t *jo = pico8_cg("jo");
    const pico8_jo_t *posee = NULL;
    for (int i = 0; jo != NULL && i < jo->count; i++) {
        const pico8_jo_t *p = (const pico8_jo_t *)jo->items[i];
        if (p->id[0] != 's') continue;
        assert(posee == NULL && "une seule prise a ete posee");
        posee = p;
    }
    assert(posee != NULL && strcmp(posee->header.me, "ly") == 0 && "la prise de f161-164 n'a pas ete posee");
    return posee;
}

/* Le `bd` de la pose fait `by` et `cc` dans la generation, hors de l'arene. */
static void une_prise_posee_garde_son_arene(void) {
    demarrer(PRISES);
    jouer_jusqu_a(160);
    const size_t avant = partie_sans_registres();
    jouer_jusqu_a(174);
    const size_t apres = partie_sans_registres();
    const pico8_jo_t *posee = la_prise_posee();
    const size_t listes = octets_d_une_liste_de_planete(posee->by) + octets_d_une_liste_de_planete(posee->cc);
    printf("  prise posee : partie - registres = %lu -> %lu (arene %u, by/cc %lu)\n", (unsigned long)avant,
           (unsigned long)apres, PICO8_JO_PRISE_OCTETS, (unsigned long)listes);
    assert(apres == avant + PICO8_JO_PRISE_OCTETS + listes && "une prise posee garde son arene, et rien d'autre");
}

/* Une frame de rejeu ; sans `dessiner`, aucun `kj` ne remet `fo` a nil. */
static void jouer_une_frame(int dessiner) {
    pico8_dispatch_set_input(g_input);
    pico8_cheat_step(&g_cheat, g_input);
    pico8__update60();
    if (dessiner) pico8__draw();
    if (g_frame < g_input->frame_count - 1) pico8_input_step(g_input);
    g_frame++;
}

static pico8_jo_t *planete_dont_fo_est(const pico8_struct_header_t *tenue) {
    const pico8_ents_t *np = pico8_cg("np");
    for (int i = 0; np != NULL && i < np->count; i++) {
        pico8_jo_t *p = (pico8_jo_t *)np->items[i];
        if (p->fo == tenue) return p;
    }
    return NULL;
}

/* f23 : la structure tenue sur p34 y accroche son `fo` ; p34 passe hors
 * ecran avant le draw, son `kj` sort sans remettre `fo` a nil. Refusee a
 * f24, la structure est rendue au vidage de f26, et la pression de f28
 * reprend son arene : `fo` ne doit pas viser la nouvelle structure. */
static void le_fo_d_une_planete_hors_ecran_oublie_la_prise_refusee(void) {
    static pico8_vec_t loin;
    demarrer(PRISES);
    jouer_jusqu_a(22);
    pico8_dispatch_set_input(g_input);
    pico8_cheat_step(&g_cheat, g_input);
    pico8__update60();
    const pico8_struct_header_t *tenue = pico8_ok()->lheld;
    assert(tenue != NULL && "f23 : une structure est tenue");
    pico8_jo_t *p34 = planete_dont_fo_est(tenue);
    assert(p34 != NULL && "f23 : la structure tenue accroche le fo d'une planete");
    loin = pico8_vec_add(*((const pico8_qb_t *)pico8_mo())->ra, pico8_vec_v(pico8_fixed_from_int(500), 0));
    p34->ra = &loin;
    assert(!pico8_gd(&p34->header) && "p34 est hors ecran");
    pico8__draw();
    assert(p34->fo == tenue && "hors ecran, kj laisse fo");
    if (g_frame < g_input->frame_count - 1) pico8_input_step(g_input);
    g_frame++;
    while (g_frame <= 27) jouer_une_frame(1);
    jouer_une_frame(0);
    assert(pico8_ok()->lheld == tenue && "f28 : la prise suivante reprend l'arene, a la meme adresse");
    assert(p34->fo == NULL && "le fo d'une planete hors ecran vise une prise refusee recyclee");
}

static void une_prise_refusee_traverse_une_nouvelle_partie(void) {
    demarrer(PRISES);
    jouer_jusqu_a(195);
}

#define SLIPWAYS "tests/fixtures/slipways_refuses.json"

/* Fixture SLIPWAYS : f0-34 de slipway_build (« p27 » et « p48 »
 * colonisees) ; puis des cycles de 15 frames : pression sur « p27 » (88,37),
 * dix frames tenues, relache, trois d'attente. Vingt refus tenus en (40,100)
 * (f35-334), un glisse pose de p27 a p48 (f335-338, attente f339-341), un
 * refus tenu en (73,91), au-dela de p48 (f342-356) ; « restart » et nouvelle
 * partie (clic f367) ; une prise d'icone (f371-377). Un refus est relache a
 * la 11e frame du cycle et rendu au vidage de la 13e. */
static const pico8_jc_t *le_slipway_tenu(void) {
    const pico8_struct_header_t *tenu = pico8_ok()->lheld;
    assert(tenu != NULL && tenu->cls == pico8_jc_struct_class() && "un slipway est tenu");
    return (const pico8_jc_t *)tenu;
}

static void un_slipway_tenu_ne_fait_pas_grandir_la_partie(void) {
    demarrer(SLIPWAYS);
    jouer_jusqu_a(36);
    const pico8_jc_t *tenu = le_slipway_tenu();
    const size_t f36 = pico8_zone_partie_prise();
    jouer_jusqu_a(45);
    assert(le_slipway_tenu() == tenu && tenu->re == &tenu->point.header && "f45 : tenu au-dessus du vide");
    printf("  slipway tenu f36/f45 : partie = %lu/%lu\n", (unsigned long)f36, (unsigned long)pico8_zone_partie_prise());
    assert(pico8_zone_partie_prise() == f36 && "un slipway tenu alloue a chaque frame");
}

static void slipways_refuses_ne_font_pas_grandir_la_partie(void) {
    demarrer(SLIPWAYS);
    jouer_jusqu_a(49);
    const size_t un = partie_sans_registres();
    jouer_jusqu_a(334);
    const size_t vingt = partie_sans_registres();
    printf("  slipways refuses 1/20 : partie - registres = %lu/%lu\n", (unsigned long)un, (unsigned long)vingt);
    assert(un == vingt && "un slipway refuse laisse son arene dans la generation");
}

static size_t octets_des_oe_de_p27_et_p48(void) {
    return octets_d_un_registre(planete_d_id("p27")->oe) + octets_d_un_registre(planete_d_id("p48")->oe);
}

/* Le `kf` de la pose fait grandir `oe` aux deux bouts, dans la generation,
 * hors de l'arene. */
static void un_slipway_pose_garde_son_arene(void) {
    demarrer(SLIPWAYS);
    jouer_jusqu_a(334);
    const size_t avant = partie_sans_registres();
    const size_t oe_avant = octets_des_oe_de_p27_et_p48();
    jouer_jusqu_a(356);
    const size_t apres = partie_sans_registres();
    const size_t oe = octets_des_oe_de_p27_et_p48() - oe_avant;
    assert(pico8_cg("jc") != NULL && pico8_cg("jc")->count == 1 && "f338 : un slipway pose");
    printf("  slipway pose puis refuse : partie - registres = %lu -> %lu (arene %u, oe %lu)\n", (unsigned long)avant,
           (unsigned long)apres, PICO8_JO_PRISE_OCTETS, (unsigned long)oe);
    assert(apres == avant + PICO8_JO_PRISE_OCTETS + oe &&
           "un slipway pose garde son arene, le refuse suivant en reprend une");
}

/* f345 : le slipway tenu de p27 vers (73,91) accroche le `fo` de p48, sur
 * son chemin, qu'on pousse hors ecran : son `kj` ne remet plus `fo` a nil.
 * Refuse a f353, le slipway est rendu au vidage de f355. */
static void le_fo_d_une_planete_hors_ecran_oublie_le_slipway_refuse(void) {
    static pico8_vec_t loin;
    demarrer(SLIPWAYS);
    jouer_jusqu_a(344);
    pico8_dispatch_set_input(g_input);
    pico8_cheat_step(&g_cheat, g_input);
    pico8__update60();
    const pico8_jc_t *tenu = le_slipway_tenu();
    pico8_jo_t *p = planete_dont_fo_est(&tenu->header);
    assert(p != NULL && "f345 : le slipway tenu accroche le fo d'une planete");
    loin = pico8_vec_add(*((const pico8_qb_t *)pico8_mo())->ra, pico8_vec_v(pico8_fixed_from_int(500), 0));
    p->ra = &loin;
    assert(!pico8_gd(&p->header) && "la planete est hors ecran");
    pico8__draw();
    if (g_frame < g_input->frame_count - 1) pico8_input_step(g_input);
    g_frame++;
    jouer_jusqu_a(354);
    assert(p->fo == &tenu->header && "hors ecran, kj laisse fo");
    jouer_jusqu_a(355);
    printf("  slipway refuse : fo de %s coupe au vidage de f355\n", p->id);
    assert(p->fo == NULL && "le fo d'une planete hors ecran vise un slipway refuse rendu");
}

static void un_slipway_refuse_traverse_une_nouvelle_partie(void) {
    demarrer(SLIPWAYS);
    jouer_jusqu_a(356);
    assert(pico8_cg("jc") != NULL && pico8_cg("jc")->count == 1);
    jouer_jusqu_a(371);
    assert(pico8_cg("jc") == NULL || pico8_cg("jc")->count == 0);
    assert(pico8_ok()->lheld != NULL && "f371 : une prise dans la nouvelle partie");
    jouer_jusqu_a(377);
}

/* f11 : titre apres « restart ». Recharger rend les places des entites de
 * hv, et le jl() d'_init ne relit pas les registres remis a zero. */
static void pause_restart_recharge_apres_le_titre(void) {
    releve_t neuf, titre, recharge;
    demarrer("pause_restart");
    relever(&neuf, AVANT_F0);
    jouer_jusqu_a(11);
    relever(&titre, 11);
    imprimer("pause_restart", &titre);
    pico8_cart_shutdown();
    pico8_cart_load();
    pico8__init();
    pico8__update60();
    pico8__draw();
    relever(&recharge, 0);
    imprimer("pause_restart recharge", &recharge);
    assert(memes_comptes(&neuf, &recharge) && "recharger doit rendre les places des entites de hv");
}

/* Apres le rendu de f13, jo.ir et jo.oc doivent viser la nouvelle
 * generation : sinon ASan plante a la premiere lecture. */
static void pause_restart_devoile_sur_les_etats_de_la_nouvelle_partie(void) {
    demarrer("pause_restart");
    jouer_jusqu_a(13);
    const pico8_ents_t *jo = pico8_cg("jo");
    const pico8_df_entry_t *ll = pico8_df_lookup("ll");
    int devoilees = 0;
    int par_oc = 0;
    for (int i = 0; jo != NULL && i < jo->count && devoilees < 5; i++) {
        pico8_jo_t *p = (pico8_jo_t *)jo->items[i];
        if (p->header.me == NULL || strcmp(p->header.me, "unknown") != 0) continue;
        pico8_jo_hh(p);
        devoilees++;
        par_oc += p->nk != NULL && p->nk != ll;
    }
    assert(devoilees > 0 && "la nouvelle partie a des planetes unknown");
    assert(par_oc > 0 && "aucun devoilement n'a lu jo.oc");
    printf("  pause_restart f13 : %d planetes devoilees, %d par jo.oc\n", devoilees, par_oc);
}

/* f42-90 : la boite de l'annee retenue par son lh, sans clic. */
static void message_box_hold_ne_bouge_pas_sans_action(void) {
    const int frames[] = {45, 90};
    releve_t r[2];
    rejouer("message_box_hold", frames, 2, r);
    assert(memes_comptes(&r[0], &r[1]) && r[0].particule == r[1].particule && "une sorte prend une place par tick");
    assert(r[0].partie == r[1].partie && "la zone partie prend des octets par tick");
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
 * <vivantes> » par sorte d'interface puis de particules, les maxima de
 * la session par sorte de particules, puis les octets pris de la zone
 * partie et leur maximum, puis ceux de la sous-zone chargement, en fin de
 * session, sans toucher capture, dump ni
 * code de sortie. */
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
    static const char *const PARTICULES[PICO8_PARTICULES_COUNT] = {"particule", "qy", "qy_fv", "ga", "ts"};
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
    for (int s = PICO8_PARTICULES_QY; s < PICO8_PARTICULES_COUNT; s++) {
        if (s == PICO8_PARTICULES_GA) continue;
        snprintf(ligne, sizeof ligne, "max_elements %s %d\n", PARTICULES[s],
                 pico8_zone_particules_max_elements((pico8_particules_sorte_t)s));
        strcat(attendu, ligne);
    }
    snprintf(ligne, sizeof ligne, "partie %lu\nmax_octets partie %lu\n", (unsigned long)pico8_zone_partie_prise(),
             (unsigned long)pico8_zone_partie_max());
    strcat(attendu, ligne);
    snprintf(ligne, sizeof ligne, "chargement %lu\n", (unsigned long)pico8_zone_chargement_prise());
    strcat(attendu, ligne);
    snprintf(ligne, sizeof ligne, "max_octets chargement %lu\n", (unsigned long)pico8_zone_chargement_max());
    strcat(attendu, ligne);
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
    printf("  invariant particule = somme des #qy inscrits, ts = somme des #ts de cg.jc : %d e2e, chaque frame\n", e2e);
    assert(e2e == 26 && "tests/e2e/ compte 26 rejeux");
    dans_un_enfant(pause_restart_revient_au_titre);
    dans_un_enfant(pause_restart_garde_la_partie_quittee_pendant_le_clic);
    dans_un_enfant_qui_meurt_par_asan(pause_restart_rend_la_partie_quittee_a_la_frame_suivante);
    dans_un_enfant(pause_restart_garde_la_partie_a_travers_restart);
    dans_un_enfant(pause_restart_garde_les_planetes_de_la_partie_quittee_pendant_le_clic);
    dans_un_enfant_qui_meurt_par_asan(pause_restart_rend_les_planetes_de_la_partie_quittee);
    dans_un_enfant(pause_restart_compte_les_etats_de_bw);
    dans_un_enfant(pause_restart_borne_les_blocs_morts_des_registres);
    dans_un_enfant(pause_restart_recharge_apres_le_titre);
    dans_un_enfant(pause_restart_devoile_sur_les_etats_de_la_nouvelle_partie);
    dans_un_enfant(game_end_final_score_bd_ne_fait_pas_grandir_la_partie);
    dans_un_enfant(trade_rich_mine_bd_sur_des_routes_ne_fait_pas_grandir_la_partie);
    dans_un_enfant(trade_rich_mine_une_route_gardee_survit_a_la_frame);
    dans_un_enfant(trade_rich_mine_rend_ses_trainees_au_rechargement);
    dans_un_enfant(prises_refusees_ne_font_pas_grandir_la_partie);
    dans_un_enfant(une_prise_posee_garde_son_arene);
    dans_un_enfant(une_prise_refusee_traverse_une_nouvelle_partie);
    dans_un_enfant(le_fo_d_une_planete_hors_ecran_oublie_la_prise_refusee);
    dans_un_enfant(un_slipway_tenu_ne_fait_pas_grandir_la_partie);
    dans_un_enfant(slipways_refuses_ne_font_pas_grandir_la_partie);
    dans_un_enfant(un_slipway_pose_garde_son_arene);
    dans_un_enfant(un_slipway_refuse_traverse_une_nouvelle_partie);
    dans_un_enfant(le_fo_d_une_planete_hors_ecran_oublie_le_slipway_refuse);
    dans_un_enfant(message_box_hold_ne_bouge_pas_sans_action);
    dans_un_enfant(empty_planet_fade_ne_monte_pas);
    dans_un_enfant(tech_geoharvesting_burst_rend_l_eclat);
    dans_un_enfant(ecran_titre_sans_entree_atteint_un_regime);
    dans_un_enfant(le_rejeu_ecrit_le_remplissage_en_fin_de_session);
    dans_un_enfant(le_rejeu_ecrit_le_pic_de_la_session);
    printf("test_pico8_places_vivantes: all tests passed\n");
    return 0;
}
