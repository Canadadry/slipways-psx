#include "pico8_state.h"

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

static const char *const SORTS[PICO8_SORTE_COUNT] = {"js", "fq", "mh", "iv", "rangee", "le"};
static const char *const PARTICLES[PICO8_PARTICULES_COUNT] = {"particule", "qy", "qy_fv", "ga", "ts"};

static void write_fixed(FILE *out, pico8_fixed_t x) {
    int64_t v = x;
    if (v < 0) {
        fputc('-', out);
        v = -v;
    }
    long whole = (long)(v >> 16);
    long hundredths = (long)(((v & 0xffff) * 100) >> 16);
    if (hundredths == 0) {
        fprintf(out, "%ld", whole);
    } else {
        fprintf(out, "%ld.%02ld", whole, hundredths);
    }
}

static pico8_vec_t ra_of(const pico8_struct_header_t *e) {
    const pico8_vec_t *ra = e != NULL && e->ra != NULL ? *e->ra : NULL;
    return ra != NULL ? *ra : pico8_vec_v(0, 0);
}

static void write_position(FILE *out, const pico8_struct_header_t *e, pico8_vec_t camera) {
    pico8_vec_t ra = ra_of(e);
    fputs(" world ", out);
    write_fixed(out, ra.x);
    fputc(' ', out);
    write_fixed(out, ra.y);
    fputs(" screen ", out);
    write_fixed(out, pico8_fixed_sub(ra.x, camera.x));
    fputc(' ', out);
    write_fixed(out, pico8_fixed_sub(ra.y, camera.y));
    fprintf(out, " visible %d", pico8_gd(e));
}

/* The cart class name, or NULL outside the `jo` tree. */
static const char *class_of(const pico8_struct_header_t *e) {
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

static const char *planet_state(const char *me) {
    if (me == NULL) return "-";
    if (strcmp(me, "unknown") == 0) return "unknown";
    if (strcmp(me, "known") == 0) return "known";
    if (strcmp(me, "ly") == 0) return "colonized";
    if (strcmp(me, "ll") == 0) return "empty";
    return me;
}

static const char *id_of(const pico8_struct_header_t *e) {
    if (e == NULL || class_of(e) == NULL) return "-";
    const char *id = ((const pico8_jo_t *)e)->id;
    return id != NULL ? id : "-";
}

static int count_of(const pico8_ents_t *l) {
    return l != NULL ? l->count : 0;
}

static void write_clock(FILE *out) {
    const pico8_hy_t *fm = pico8_fm_hy();
    if (fm == NULL) {
        fputs("clock year - month -\n", out);
        return;
    }
    fputs("clock year ", out);
    write_fixed(out, fm->yr);
    fputs(" month ", out);
    write_fixed(out, fm->lo);
    fputc('\n', out);
}

/* Before `eu()`, only the placeholder `fm` holds the resources, which the
 * cheat writes. */
static void write_resources(FILE *out) {
    const pico8_hy_t *fm = pico8_fm_hy();
    const pico8_fm_placeholder_t *placeholder = pico8_fm_placeholder();
    fputs("resources credits ", out);
    if (fm != NULL) {
        write_fixed(out, fm->nz);
    } else if (placeholder != NULL && placeholder->has_nz) {
        write_fixed(out, placeholder->nz);
    } else {
        fputc('-', out);
    }
    fputs("\nresources science ", out);
    if (fm != NULL) {
        write_fixed(out, fm->jk);
    } else if (placeholder != NULL && placeholder->has_jk) {
        write_fixed(out, placeholder->jk);
    } else {
        fputc('-', out);
    }
    fputs("\nresources income ", out);
    if (fm != NULL) {
        write_fixed(out, fm->ge);
    } else {
        fputc('-', out);
    }
    fputc('\n', out);
}

static int write_techs(FILE *out) {
    const pico8_hy_t *fm = pico8_fm_hy();
    if (fm == NULL || fm->invented == NULL) return 0;
    for (int i = 0; i < fm->invented->count; i++) {
        fprintf(out, "techs %s\n", (const char *)fm->invented->items[i]);
    }
    return fm->invented->count;
}

static pico8_vec_t write_camera(FILE *out) {
    pico8_vec_t camera = ra_of(pico8_mo());
    fputs("camera world ", out);
    write_fixed(out, camera.x);
    fputc(' ', out);
    write_fixed(out, camera.y);
    fputc('\n', out);
    return camera;
}

static void write_planets(FILE *out, pico8_vec_t camera) {
    const pico8_ents_t *jo = pico8_cg("jo");
    for (int i = 0; i < count_of(jo); i++) {
        const pico8_jo_t *p = (const pico8_jo_t *)jo->items[i];
        const char *cls = class_of(&p->header);
        fprintf(out, "planet %s %s %s %s", id_of(&p->header), planet_state(p->header.me),
                cls != NULL ? cls : "-", p->nk != NULL && p->nk->px != NULL ? p->nk->px : "-");
        write_position(out, &p->header, camera);
        fputc('\n', out);
    }
}

/* A slipway still held by the mouse is in `kz`; `d_` moves it to `ph`
 * once placed. */
static int slipway_placed(const pico8_struct_header_t *e) {
    return e->me == NULL || strcmp(e->me, "kz") != 0;
}

static int write_slipways(FILE *out) {
    const pico8_ents_t *jc = pico8_cg("jc");
    int n = 0;
    for (int i = 0; i < count_of(jc); i++) {
        const pico8_jc_t *s = (const pico8_jc_t *)jc->items[i];
        if (!slipway_placed(&s->header)) continue;
        fprintf(out, "slipway %s %s %s\n", id_of(s->qu), id_of(s->re),
                s->has_mt && s->mt != 0 ? "infragate" : "normal");
        n++;
    }
    return n;
}

static void write_structures(FILE *out, pico8_vec_t camera) {
    const pico8_ents_t *nr = pico8_cg("nr");
    for (int i = 0; i < count_of(nr); i++) {
        const pico8_jo_t *g = (const pico8_jo_t *)nr->items[i];
        fprintf(out, "structure %s %s", id_of(&g->header),
                g->header.cls == pico8_fr_struct_class() ? "infragate" : "slipgate");
        write_position(out, &g->header, camera);
        fprintf(out, " links %d\n", count_of(g->oe));
    }
}

/* Planets in `ly`, labs included: `jo:gg` would skip those without `hf`. */
static int count_colonies(void) {
    const pico8_ents_t *jo = pico8_cg("jo");
    int n = 0;
    for (int i = 0; i < count_of(jo); i++) {
        const char *me = jo->items[i]->me;
        n += me != NULL && strcmp(me, "ly") == 0;
    }
    return n;
}

/* Bytes in whole slots for slot zones. */
static void write_zones(FILE *out) {
    fprintf(out, "zone frame %lu %lu %lu\n", (unsigned long)pico8_zone_frame_used(),
            (unsigned long)pico8_zone_frame_max(), (unsigned long)PICO8_ZONE_FRAME_BYTES);
    for (int i = 0; i < PICO8_SORTE_COUNT; i++) {
        pico8_sorte_t s = (pico8_sorte_t)i;
        unsigned long slot_size = (unsigned long)pico8_zone_interface_slot_size(s);
        fprintf(out, "zone interface %s %lu %lu %lu\n", SORTS[i], pico8_zone_interface_vivantes(s) * slot_size,
                pico8_zone_interface_max_live(s) * slot_size, pico8_zone_interface_capacity(s) * slot_size);
    }
    for (int i = 0; i < PICO8_PARTICULES_COUNT; i++) {
        pico8_particules_sorte_t s = (pico8_particules_sorte_t)i;
        unsigned long slot_size = (unsigned long)pico8_zone_particules_slot_size(s);
        fprintf(out, "zone particules %s %lu %lu %lu\n", PARTICLES[i], pico8_zone_particules_vivantes(s) * slot_size,
                pico8_zone_particules_max_vivantes(s) * slot_size, pico8_zone_particules_capacity(s) * slot_size);
    }
    fprintf(out, "zone partie %lu %lu %lu\n", (unsigned long)pico8_zone_partie_prise(),
            (unsigned long)pico8_zone_partie_max(), (unsigned long)PICO8_ZONE_GENERATION_BYTES);
    fprintf(out, "zone chargement %lu %lu %lu\n", (unsigned long)pico8_zone_chargement_prise(),
            (unsigned long)pico8_zone_chargement_max(), (unsigned long)PICO8_ZONE_CHARGEMENT_BYTES);
}

/* In slots, except `elements_*` (elements of a list, current unknown: -)
 * and `bytes_*` (bytes of an arena slot, current unknown: -). */
static void write_caps(FILE *out) {
    for (int i = 0; i < PICO8_SORTE_COUNT; i++) {
        pico8_sorte_t s = (pico8_sorte_t)i;
        fprintf(out, "cap %s %d %d %d\n", SORTS[i], pico8_zone_interface_vivantes(s),
                pico8_zone_interface_max_live(s), pico8_zone_interface_capacity(s));
    }
    for (int i = 0; i < PICO8_PARTICULES_COUNT; i++) {
        pico8_particules_sorte_t s = (pico8_particules_sorte_t)i;
        fprintf(out, "cap %s %d %d %d\n", PARTICLES[i], pico8_zone_particules_vivantes(s),
                pico8_zone_particules_max_vivantes(s), pico8_zone_particules_capacity(s));
    }
    fprintf(out, "cap exit_queue %d %d %d\n", pico8_file_sortie_compte(), pico8_file_sortie_max(),
            PICO8_FILE_SORTIE_CAP);
    fprintf(out, "cap elements_qy - %d %d\n", pico8_zone_particules_max_elements(PICO8_PARTICULES_QY),
            PICO8_QY_CAPACITE);
    fprintf(out, "cap elements_qy_fv - %d %d\n", pico8_zone_particules_max_elements(PICO8_PARTICULES_QY_FV),
            PICO8_QY_FV_CAPACITE);
    fprintf(out, "cap elements_ts - %d -\n", pico8_zone_particules_max_elements(PICO8_PARTICULES_TS));
    for (int i = PICO8_PARTICULES_QY; i <= PICO8_PARTICULES_GA; i++) {
        pico8_particules_sorte_t s = (pico8_particules_sorte_t)i;
        fprintf(out, "cap bytes_%s - %lu %lu\n", PARTICLES[i], (unsigned long)pico8_zone_particules_max_octets(s),
                (unsigned long)pico8_zone_particules_slot_size(s));
    }
}

void pico8_state_write(FILE *out) {
    write_clock(out);
    write_resources(out);
    int techs = write_techs(out);
    pico8_vec_t camera = write_camera(out);
    write_planets(out, camera);
    int slipways = write_slipways(out);
    write_structures(out, camera);
    fprintf(out, "count planets %d\n", count_of(pico8_cg("jo")));
    fprintf(out, "count colonies %d\n", count_colonies());
    fprintf(out, "count slipways %d\n", slipways);
    fprintf(out, "count structures %d\n", count_of(pico8_cg("nr")));
    fprintf(out, "count techs %d\n", techs);
    write_zones(out);
    write_caps(out);
}

static const char *g_exit_path;
static const int *g_exit_frame;
static int g_exit_registered;

static void write_on_exit(void) {
    if (g_exit_path == NULL) return;
    FILE *out = fopen(g_exit_path, "w");
    if (out == NULL) return;
    fprintf(out, "stopped pico8_fatal frame %d\n", *g_exit_frame);
    pico8_state_write(out);
    fclose(out);
    fprintf(stderr, "pico8 state: stopped at frame %d, state written to %s\n", *g_exit_frame, g_exit_path);
}

void pico8_state_write_on_exit(const char *path, const int *frame) {
    assert(path != NULL && frame != NULL);
    g_exit_path = path;
    g_exit_frame = frame;
    if (!g_exit_registered) {
        int registered = atexit(write_on_exit) == 0;
        assert(registered);
        (void)registered;
        g_exit_registered = 1;
    }
}

void pico8_state_release(void) {
    g_exit_path = NULL;
}
