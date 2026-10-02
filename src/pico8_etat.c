#include "pico8_etat.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_fm.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_planet_family_trade.h"
#include "pico8_file_sortie.h"
#include "pico8_slipway.h"
#include "pico8_zone_frame.h"
#include "pico8_zone_interface.h"
#include "pico8_zone_particules.h"
#include "pico8_zone_partie.h"
#include "pico8_zone_sizes.h"

static const char *const SORTES[PICO8_SORTE_COUNT] = {"js", "fq", "mh", "iv", "rangee", "le"};
static const char *const PARTICULES[PICO8_PARTICULES_COUNT] = {"particule", "qy", "qy_fv", "ga", "ts"};

static void ecrire_fixe(FILE *out, pico8_fixed_t x) {
    int64_t v = x;
    if (v < 0) {
        fputc('-', out);
        v = -v;
    }
    long entier = (long)(v >> 16);
    long centiemes = (long)(((v & 0xffff) * 100) >> 16);
    if (centiemes == 0) {
        fprintf(out, "%ld", entier);
    } else {
        fprintf(out, "%ld.%02ld", entier, centiemes);
    }
}

static pico8_vec_t ra_de(const pico8_struct_header_t *e) {
    const pico8_vec_t *ra = e != NULL && e->ra != NULL ? *e->ra : NULL;
    return ra != NULL ? *ra : pico8_vec_v(0, 0);
}

static void ecrire_position(FILE *out, const pico8_struct_header_t *e, pico8_vec_t camera) {
    pico8_vec_t ra = ra_de(e);
    fputs(" monde ", out);
    ecrire_fixe(out, ra.x);
    fputc(' ', out);
    ecrire_fixe(out, ra.y);
    fputs(" ecran ", out);
    ecrire_fixe(out, pico8_fixed_sub(ra.x, camera.x));
    fputc(' ', out);
    ecrire_fixe(out, pico8_fixed_sub(ra.y, camera.y));
    fprintf(out, " visible %d", pico8_gd(e));
}

/* Le nom de classe du cart, ou NULL hors de l'arbre `jo`. */
static const char *classe_de(const pico8_struct_header_t *e) {
    const pico8_struct_class_t *c = e->cls;
    if (c == pico8_jo_struct_class()) return "jo";
    if (c == pico8_qh_struct_class()) return "qh";
    if (c == pico8_nl_struct_class()) return "nl";
    if (c == pico8_os_struct_class()) return "os";
    if (c == pico8_hj_struct_class()) return "hj";
    if (c == pico8_pm_struct_class()) return "pm";
    if (c == pico8_mx_struct_class()) return "mx";
    if (c == pico8_ga_struct_class()) return "ga";
    if (c == pico8_nr_struct_class()) return "nr";
    if (c == pico8_fr_struct_class()) return "fr";
    return NULL;
}

static const char *etat_planete(const char *me) {
    if (me == NULL) return "-";
    if (strcmp(me, "unknown") == 0) return "inconnue";
    if (strcmp(me, "known") == 0) return "connue";
    if (strcmp(me, "ly") == 0) return "colonisee";
    if (strcmp(me, "ll") == 0) return "vide";
    return me;
}

static const char *id_de(const pico8_struct_header_t *e) {
    if (e == NULL || classe_de(e) == NULL) return "-";
    const char *id = ((const pico8_jo_t *)e)->id;
    return id != NULL ? id : "-";
}

static int nombre(const pico8_ents_t *l) {
    return l != NULL ? l->count : 0;
}

static void ecrire_horloge(FILE *out) {
    const pico8_hy_t *fm = pico8_fm_hy();
    if (fm == NULL) {
        fputs("horloge annee - mois -\n", out);
        return;
    }
    fputs("horloge annee ", out);
    ecrire_fixe(out, fm->yr);
    fputs(" mois ", out);
    ecrire_fixe(out, fm->lo);
    fputc('\n', out);
}

/* Avant `eu()`, seul le `fm` provisoire porte les ressources, que la triche
 * ecrit. */
static void ecrire_ressources(FILE *out) {
    const pico8_hy_t *fm = pico8_fm_hy();
    const pico8_fm_placeholder_t *provisoire = pico8_fm_placeholder();
    fputs("ressources credits ", out);
    if (fm != NULL) {
        ecrire_fixe(out, fm->nz);
    } else if (provisoire != NULL && provisoire->has_nz) {
        ecrire_fixe(out, provisoire->nz);
    } else {
        fputc('-', out);
    }
    fputs("\nressources science ", out);
    if (fm != NULL) {
        ecrire_fixe(out, fm->jk);
    } else if (provisoire != NULL && provisoire->has_jk) {
        ecrire_fixe(out, provisoire->jk);
    } else {
        fputc('-', out);
    }
    fputs("\nressources revenu ", out);
    if (fm != NULL) {
        ecrire_fixe(out, fm->ge);
    } else {
        fputc('-', out);
    }
    fputc('\n', out);
}

static int ecrire_techs(FILE *out) {
    const pico8_hy_t *fm = pico8_fm_hy();
    if (fm == NULL || fm->invented == NULL) return 0;
    for (int i = 0; i < fm->invented->count; i++) {
        fprintf(out, "techs %s\n", (const char *)fm->invented->items[i]);
    }
    return fm->invented->count;
}

static pico8_vec_t ecrire_camera(FILE *out) {
    pico8_vec_t camera = ra_de(pico8_mo());
    fputs("camera monde ", out);
    ecrire_fixe(out, camera.x);
    fputc(' ', out);
    ecrire_fixe(out, camera.y);
    fputc('\n', out);
    return camera;
}

static void ecrire_planetes(FILE *out, pico8_vec_t camera) {
    const pico8_ents_t *jo = pico8_cg("jo");
    for (int i = 0; i < nombre(jo); i++) {
        const pico8_jo_t *p = (const pico8_jo_t *)jo->items[i];
        const char *classe = classe_de(&p->header);
        fprintf(out, "planete %s %s %s %s", id_de(&p->header), etat_planete(p->header.me),
                classe != NULL ? classe : "-", p->nk != NULL && p->nk->px != NULL ? p->nk->px : "-");
        ecrire_position(out, &p->header, camera);
        fputc('\n', out);
    }
}

/* Un slipway encore tenu a la souris est en `kz` ; `d_` le fait passer en
 * `ph` quand il est pose. */
static int slipway_pose(const pico8_struct_header_t *e) {
    return e->me == NULL || strcmp(e->me, "kz") != 0;
}

static int ecrire_slipways(FILE *out) {
    const pico8_ents_t *jc = pico8_cg("jc");
    int n = 0;
    for (int i = 0; i < nombre(jc); i++) {
        const pico8_jc_t *s = (const pico8_jc_t *)jc->items[i];
        if (!slipway_pose(&s->header)) continue;
        fprintf(out, "slipway %s %s %s\n", id_de(s->qu), id_de(s->re),
                s->has_mt && s->mt != 0 ? "infragate" : "normal");
        n++;
    }
    return n;
}

static void ecrire_structures(FILE *out, pico8_vec_t camera) {
    const pico8_ents_t *nr = pico8_cg("nr");
    for (int i = 0; i < nombre(nr); i++) {
        const pico8_jo_t *g = (const pico8_jo_t *)nr->items[i];
        fprintf(out, "structure %s %s", id_de(&g->header),
                g->header.cls == pico8_fr_struct_class() ? "infragate" : "slipgate");
        ecrire_position(out, &g->header, camera);
        fprintf(out, " liens %d\n", nombre(g->oe));
    }
}

/* Les planetes en `ly`, labos compris : `jo:gg` ecarterait ceux sans `hf`. */
static int compter_colonies(void) {
    const pico8_ents_t *jo = pico8_cg("jo");
    int n = 0;
    for (int i = 0; i < nombre(jo); i++) {
        const char *me = jo->items[i]->me;
        n += me != NULL && strcmp(me, "ly") == 0;
    }
    return n;
}

/* Octets en places entieres pour les zones a places. */
static void ecrire_zones(FILE *out) {
    fprintf(out, "zone frame %lu %lu %lu\n", (unsigned long)pico8_zone_frame_prise(),
            (unsigned long)pico8_zone_frame_max(), (unsigned long)PICO8_ZONE_FRAME_BYTES);
    for (int i = 0; i < PICO8_SORTE_COUNT; i++) {
        pico8_sorte_t s = (pico8_sorte_t)i;
        unsigned long taille = (unsigned long)pico8_zone_interface_taille(s);
        fprintf(out, "zone interface %s %lu %lu %lu\n", SORTES[i], pico8_zone_interface_vivantes(s) * taille,
                pico8_zone_interface_max_vivantes(s) * taille, pico8_zone_interface_capacite(s) * taille);
    }
    for (int i = 0; i < PICO8_PARTICULES_COUNT; i++) {
        pico8_particules_sorte_t s = (pico8_particules_sorte_t)i;
        unsigned long taille = (unsigned long)pico8_zone_particules_taille(s);
        fprintf(out, "zone particules %s %lu %lu %lu\n", PARTICULES[i], pico8_zone_particules_vivantes(s) * taille,
                pico8_zone_particules_max_vivantes(s) * taille, pico8_zone_particules_capacite(s) * taille);
    }
    fprintf(out, "zone partie %lu %lu %lu\n", (unsigned long)pico8_zone_partie_prise(),
            (unsigned long)pico8_zone_partie_max(), (unsigned long)PICO8_ZONE_GENERATION_BYTES);
    fprintf(out, "zone chargement %lu %lu %lu\n", (unsigned long)pico8_zone_chargement_prise(),
            (unsigned long)pico8_zone_chargement_max(), (unsigned long)PICO8_ZONE_CHARGEMENT_BYTES);
}

/* En places, sauf `elements_*` (elements d'une liste, courant inconnu : -)
 * et `octets_*` (octets d'une place-arene, courant inconnu : -). */
static void ecrire_caps(FILE *out) {
    for (int i = 0; i < PICO8_SORTE_COUNT; i++) {
        pico8_sorte_t s = (pico8_sorte_t)i;
        fprintf(out, "cap %s %d %d %d\n", SORTES[i], pico8_zone_interface_vivantes(s),
                pico8_zone_interface_max_vivantes(s), pico8_zone_interface_capacite(s));
    }
    for (int i = 0; i < PICO8_PARTICULES_COUNT; i++) {
        pico8_particules_sorte_t s = (pico8_particules_sorte_t)i;
        fprintf(out, "cap %s %d %d %d\n", PARTICULES[i], pico8_zone_particules_vivantes(s),
                pico8_zone_particules_max_vivantes(s), pico8_zone_particules_capacite(s));
    }
    fprintf(out, "cap file_sortie %d %d %d\n", pico8_file_sortie_compte(), pico8_file_sortie_max(),
            PICO8_FILE_SORTIE_CAP);
    fprintf(out, "cap elements_qy - %d %d\n", pico8_zone_particules_max_elements(PICO8_PARTICULES_QY),
            PICO8_QY_CAPACITE);
    fprintf(out, "cap elements_qy_fv - %d %d\n", pico8_zone_particules_max_elements(PICO8_PARTICULES_QY_FV),
            PICO8_QY_FV_CAPACITE);
    fprintf(out, "cap elements_ts - %d -\n", pico8_zone_particules_max_elements(PICO8_PARTICULES_TS));
    for (int i = PICO8_PARTICULES_QY; i <= PICO8_PARTICULES_GA; i++) {
        pico8_particules_sorte_t s = (pico8_particules_sorte_t)i;
        fprintf(out, "cap octets_%s - %lu %lu\n", PARTICULES[i], (unsigned long)pico8_zone_particules_max_octets(s),
                (unsigned long)pico8_zone_particules_taille(s));
    }
}

void pico8_etat_ecrire(FILE *out) {
    ecrire_horloge(out);
    ecrire_ressources(out);
    int techs = ecrire_techs(out);
    pico8_vec_t camera = ecrire_camera(out);
    ecrire_planetes(out, camera);
    int slipways = ecrire_slipways(out);
    ecrire_structures(out, camera);
    fprintf(out, "compte planetes %d\n", nombre(pico8_cg("jo")));
    fprintf(out, "compte colonies %d\n", compter_colonies());
    fprintf(out, "compte slipways %d\n", slipways);
    fprintf(out, "compte structures %d\n", nombre(pico8_cg("nr")));
    fprintf(out, "compte techs %d\n", techs);
    ecrire_zones(out);
    ecrire_caps(out);
}

static const char *g_arret_path;
static const int *g_arret_frame;
static int g_arret_inscrit;

static void ecrire_a_l_arret(void) {
    if (g_arret_path == NULL) return;
    FILE *out = fopen(g_arret_path, "w");
    if (out == NULL) return;
    fprintf(out, "arret pico8_fatal frame %d\n", *g_arret_frame);
    pico8_etat_ecrire(out);
    fclose(out);
    fprintf(stderr, "pico8 etat: arret a la frame %d, etat ecrit dans %s\n", *g_arret_frame, g_arret_path);
}

void pico8_etat_ecrire_a_l_arret(const char *path, const int *frame) {
    assert(path != NULL && frame != NULL);
    g_arret_path = path;
    g_arret_frame = frame;
    if (!g_arret_inscrit) {
        int inscrit = atexit(ecrire_a_l_arret) == 0;
        assert(inscrit);
        (void)inscrit;
        g_arret_inscrit = 1;
    }
}

void pico8_etat_lacher(void) {
    g_arret_path = NULL;
}
