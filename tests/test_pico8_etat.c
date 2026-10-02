/* make etat : l'etat texte de la derniere frame d'un rejeu, lu de
 * l'exterieur comme un agent le lirait. Les rejeux en processus, faits pour
 * comparer, tournent chacun dans un enfant : le cart n'est charge qu'une
 * fois par processus. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/pico8_entity.h"
#include "../src/pico8_etat.h"
#include "../src/pico8_fatal.h"
#include "../src/pico8_file_sortie.h"
#include "../src/pico8_rejeu.h"
#include "../src/pico8_zone_frame.h"
#include "../src/pico8_zone_interface.h"
#include "../src/pico8_zone_partie.h"

#define OUTIL "./tmp/pico8_etat"
#define ETAT_TRM "tmp/test_etat_trade_rich_mine.txt"
#define ETAT_TRICHE "tmp/test_etat_cheat_code.txt"
#define REJEU "./tmp/pico8_replay"
#define REMPLISSAGE "tmp/test_etat_remplissage.txt"
#define CAPTURE "tmp/test_etat_capture.bin"
#define DUMP "tmp/test_etat_dump.txt"

static char *lire_fichier(const char *path) {
    FILE *f = fopen(path, "rb");
    assert(f != NULL && "fichier d'etat absent");
    assert(fseek(f, 0, SEEK_END) == 0);
    long n = ftell(f);
    assert(n >= 0);
    rewind(f);
    char *buf = malloc((size_t)n + 1);
    assert(buf != NULL);
    assert(fread(buf, 1, (size_t)n, f) == (size_t)n);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

/* Lignes qui commencent par `prefixe`. */
static int compter_lignes(const char *texte, const char *prefixe) {
    int n = 0;
    size_t len = strlen(prefixe);
    for (const char *l = texte; *l != '\0';) {
        if (strncmp(l, prefixe, len) == 0) n++;
        const char *fin = strchr(l, '\n');
        if (fin == NULL) break;
        l = fin + 1;
    }
    return n;
}

/* Le premier entier apres `cle` en debut de ligne. */
static long valeur(const char *texte, const char *cle) {
    char motif[96];
    long v;
    snprintf(motif, sizeof motif, "\n%s ", cle);
    const char *l = strstr(texte, motif);
    if (l == NULL) {
        fprintf(stderr, "cle absente : %s\n", cle);
        assert(0 && "cle absente de l'etat");
    }
    assert(sscanf(l + strlen(motif), "%ld", &v) == 1);
    return v;
}

/* Les n entiers apres `cle`. */
static void valeurs(const char *texte, const char *cle, long *out, int n) {
    char motif[96];
    snprintf(motif, sizeof motif, "\n%s ", cle);
    const char *l = strstr(texte, motif);
    if (l == NULL) {
        fprintf(stderr, "cle absente : %s\n", cle);
        assert(0 && "cle absente de l'etat");
    }
    l += strlen(motif);
    for (int i = 0; i < n; i++) {
        char *fin;
        out[i] = strtol(l, &fin, 10);
        assert(fin != l && "valeur manquante");
        l = fin;
    }
}

/* Le nombre d'entites sous l'etiquette `tag` a la fin d'un rejeu en
 * processus de `events`, compte dans un enfant. */
static int compter_apres_rejeu(const char *events, const char *tag) {
    int tube[2];
    assert(pipe(tube) == 0);
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        static pico8_rejeu_t r;
        close(tube[0]);
        pico8_rejeu_ouvrir(&r, events);
        for (int i = 0; i < pico8_rejeu_frames(&r); i++) pico8_rejeu_jouer_frame(&r, i);
        const pico8_ents_t *l = pico8_cg(tag);
        int n = l != NULL ? l->count : 0;
        assert(write(tube[1], &n, sizeof n) == (ssize_t)sizeof n);
        _exit(0);
    }
    close(tube[1]);
    int n = -1;
    assert(read(tube[0], &n, sizeof n) == (ssize_t)sizeof n);
    close(tube[0]);
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    return n;
}

static void une_ligne_planete_par_planete_du_jeu(void) {
    remove(ETAT_TRM);
    assert(system(OUTIL " tests/e2e/trade_rich_mine.json " ETAT_TRM " > /dev/null") == 0);
    char *etat = lire_fichier(ETAT_TRM);
    int planetes = compter_apres_rejeu("tests/e2e/trade_rich_mine.json", "jo");
    printf("  trade_rich_mine : %d planetes\n", planetes);
    assert(planetes > 0);
    assert(compter_lignes(etat, "planete ") == planetes && "une ligne planete par planete de cg.jo");
    assert(valeur(etat, "compte planetes") == planetes);
    free(etat);
}

/* Six slipways depuis p27 (trois fournisseurs, trois labos), puis p1 vers le
 * labo s1 ; aucun ne passe par une infragate. */
static void le_reseau_construit_apparait(void) {
    char *etat = lire_fichier(ETAT_TRM);
    static const char *const attendus[] = {
        "\nslipway p27 p1 normal\n", "\nslipway p27 p71 normal\n", "\nslipway p27 p48 normal\n",
        "\nslipway p27 s1 normal\n", "\nslipway p27 s2 normal\n", "\nslipway p27 s3 normal\n",
        "\nslipway p1 s1 normal\n",
    };
    for (size_t i = 0; i < sizeof attendus / sizeof attendus[0]; i++) {
        if (strstr(etat, attendus[i]) == NULL) {
            fprintf(stderr, "absent :%s", attendus[i]);
            assert(0 && "slipway attendu absent de l'etat");
        }
    }
    assert(compter_lignes(etat, "slipway ") == 7);
    assert(valeur(etat, "compte slipways") == 7);
    assert(strstr(etat, "\nplanete p27 colonisee ") != NULL);
    assert(valeur(etat, "compte colonies") == 7 && "p1, p27, p48, p71 et les trois labos");
    assert(compter_lignes(etat, "structure ") == 0);
    assert(valeur(etat, "compte structures") == 0);
    free(etat);
}

/* La triche rejouee telle quelle : 99 de recherche, 200 $ a la derniere
 * frame (f35). */
static void les_ressources_refletent_la_triche(void) {
    remove(ETAT_TRICHE);
    assert(system(OUTIL " tests/e2e/cheat_code.json " ETAT_TRICHE " > /dev/null") == 0);
    char *etat = lire_fichier(ETAT_TRICHE);
    assert(valeur(etat, "ressources credits") == 200);
    assert(valeur(etat, "ressources science") == 99);
    free(etat);
    remove(ETAT_TRICHE);
}

static void les_techs_inventees_sont_nommees(void) {
    char *etat = lire_fichier(ETAT_TRM);
    assert(strstr(etat, "\ntechs space folding\n") != NULL);
    assert(strstr(etat, "\ntechs geneseeds\n") != NULL);
    assert(compter_lignes(etat, "techs ") == 2);
    assert(valeur(etat, "compte techs") == 2);
    free(etat);
}

/* Le pic de la zone partie est celui que pico8_replay ecrit dans
 * PICO8_REMPLISSAGE pour le meme rejeu. */
static void le_pic_partie_est_celui_du_remplissage(void) {
    remove(REMPLISSAGE);
    assert(system("PICO8_REMPLISSAGE=" REMPLISSAGE " " REJEU " tests/e2e/trade_rich_mine.json " CAPTURE " " DUMP
                  " > /dev/null") == 0);
    char *remplissage = lire_fichier(REMPLISSAGE);
    char *etat = lire_fichier(ETAT_TRM);
    long partie[3];
    valeurs(etat, "zone partie", partie, 3);
    assert(partie[1] == valeur(remplissage, "max_octets partie"));
    assert(partie[2] == 704L * 1024L && "la generation de partie, chargement exclu");
    free(remplissage);
    free(etat);
    remove(REMPLISSAGE);
    remove(CAPTURE);
    remove(DUMP);
}

/* « zone|cap <nom> <courant> <pic> <capacite> » : le pic tient la valeur
 * courante et reste sous la capacite. */
static void verifier_pic(const char *etat, const char *cle, long *pic) {
    long v[3];
    valeurs(etat, cle, v, 3);
    printf("  %s : courant %ld, pic %ld, capacite %ld\n", cle, v[0], v[1], v[2]);
    assert(v[1] >= v[0] && "un pic sous la valeur courante");
    assert(v[1] <= v[2] && "un pic au-dela de la capacite");
    if (pic != NULL) *pic = v[1];
}

static void chaque_zone_et_chaque_cap_a_son_pic(void) {
    char *etat = lire_fichier(ETAT_TRM);
    long pic;
    verifier_pic(etat, "zone frame", &pic);
    assert(pic > 0 && "la zone frame sert a chaque frame");
    static const char *const zones[] = {
        "zone interface js", "zone interface fq", "zone interface mh", "zone interface iv",
        "zone interface rangee", "zone interface le", "zone particules particule", "zone particules qy",
        "zone particules qy_fv", "zone particules ga", "zone particules ts", "zone partie", "zone chargement",
    };
    for (size_t i = 0; i < sizeof zones / sizeof zones[0]; i++) verifier_pic(etat, zones[i], NULL);
    static const char *const caps[] = {
        "cap js", "cap fq", "cap mh", "cap iv", "cap rangee", "cap le", "cap particule",
        "cap qy", "cap qy_fv", "cap ga", "cap ts", "cap file_sortie",
    };
    for (size_t i = 0; i < sizeof caps / sizeof caps[0]; i++) verifier_pic(etat, caps[i], NULL);
    verifier_pic(etat, "cap rangee", &pic);
    assert(pic > 0 && "les panneaux ouverts prennent des rangees");
    verifier_pic(etat, "cap file_sortie", &pic);
    assert(pic > 0 && "des entites quittent hv pendant le rejeu");
    free(etat);
}

typedef struct {
    size_t partie;
    size_t chargement;
    size_t frame;
    int file_sortie;
    int vivantes[PICO8_SORTE_COUNT];
} memoire_t;

static void mesurer(memoire_t *m) {
    memset(m, 0, sizeof *m);
    m->partie = pico8_zone_partie_prise();
    m->chargement = pico8_zone_chargement_prise();
    m->frame = pico8_zone_frame_prise();
    m->file_sortie = pico8_file_sortie_compte();
    for (int s = 0; s < PICO8_SORTE_COUNT; s++) m->vivantes[s] = pico8_zone_interface_vivantes((pico8_sorte_t)s);
}

/* Dans un enfant : l'etat ecrit apres le rejeu n'a rien pris ni rendu. */
static void ecrire_sans_toucher_a_la_memoire(void) {
    static pico8_rejeu_t r;
    pico8_rejeu_ouvrir(&r, "tests/e2e/trade_rich_mine.json");
    for (int i = 0; i < pico8_rejeu_frames(&r); i++) pico8_rejeu_jouer_frame(&r, i);
    memoire_t avant, apres;
    mesurer(&avant);
    FILE *out = fopen("/dev/null", "w");
    assert(out != NULL);
    pico8_etat_ecrire(out);
    fclose(out);
    mesurer(&apres);
    assert(memcmp(&avant, &apres, sizeof avant) == 0 && "ecrire l'etat a touche aux zones du jeu");
    _exit(0);
}

static void ecrire_l_etat_ne_touche_pas_aux_zones(void) {
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) ecrire_sans_toucher_a_la_memoire();
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

#define ETAT_FATAL "tmp/test_etat_fatal.txt"

/* Un pico8_fatal en cours de rejeu est lui-meme une mesure : l'etat est
 * ecrit quand meme, la frame de l'arret en tete. */
static void un_fatal_ecrit_l_etat_jusqu_a_l_arret(void) {
    remove(ETAT_FATAL);
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        static pico8_rejeu_t r;
        static int frame;
        assert(freopen("/dev/null", "w", stderr) != NULL);
        pico8_rejeu_ouvrir(&r, "tests/e2e/trade_rich_mine.json");
        pico8_etat_ecrire_a_l_arret(ETAT_FATAL, &frame);
        for (frame = 0; frame < 120; frame++) pico8_rejeu_jouer_frame(&r, frame);
        pico8_fatal("zone de test pleine");
    }
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 3 && "pico8_fatal sort avec 3");
    char *etat = lire_fichier(ETAT_FATAL);
    assert(strncmp(etat, "arret pico8_fatal frame 120\n", 28) == 0);
    assert(valeur(etat, "compte planetes") > 0);
    free(etat);
    remove(ETAT_FATAL);
}

/* Un rejeu qui va au bout n'ecrit pas de ligne d'arret. */
static void un_rejeu_complet_n_a_pas_d_arret(void) {
    char *etat = lire_fichier(ETAT_TRM);
    assert(strstr(etat, "arret") == NULL);
    free(etat);
}

int main(void) {
    une_ligne_planete_par_planete_du_jeu();
    le_reseau_construit_apparait();
    les_techs_inventees_sont_nommees();
    les_ressources_refletent_la_triche();
    le_pic_partie_est_celui_du_remplissage();
    chaque_zone_et_chaque_cap_a_son_pic();
    ecrire_l_etat_ne_touche_pas_aux_zones();
    un_fatal_ecrit_l_etat_jusqu_a_l_arret();
    un_rejeu_complet_n_a_pas_d_arret();
    printf("test_pico8_etat: all tests passed\n");
    return 0;
}
