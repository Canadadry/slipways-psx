#include "pico8_float_audit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PICO8_FLOAT_AUDIT_MAX_SITES 512

typedef struct {
    const char *file;
    const char *func;
    int line;
    unsigned long passes;
    unsigned long negatives;
    unsigned long mismatches;
    int has_first_bad;
    pico8_fixed_t first_bad;
} pico8_float_audit_site_t;

static pico8_float_audit_site_t sites[PICO8_FLOAT_AUDIT_MAX_SITES];
static int site_count;
static int overflowed;
static int registered;

/* Ajoute a $PICO8_FLOAT_AUDIT_OUT (stderr sinon), en mode ajout : les
 * deversements de plusieurs rejeux s'additionnent dans un fichier que
 * tools/float_audit.sh replie. atexit ne tourne pas apres abort() : un rejeu
 * mort sur un assert ne deverse rien, et ses sites sortent NON ATTEINT ;
 * make float-audit echoue alors sur son code de sortie. */
static void pico8_float_audit_dump(void) {
    const char *path = getenv("PICO8_FLOAT_AUDIT_OUT");
    FILE *out = NULL;
    int i;

    if (path != NULL && path[0] != '\0') {
        out = fopen(path, "a");
    }
    if (out == NULL) {
        out = stderr;
    }
    if (overflowed) {
        fprintf(out, "OVERFLOW\tPICO8_FLOAT_AUDIT_MAX_SITES trop petit\n");
    }
    for (i = 0; i < site_count; i++) {
        const pico8_float_audit_site_t *s = &sites[i];
        fprintf(out, "AUD\t%s\t%s\t%d\t%lu\t%lu\t%lu\t", s->file, s->func, s->line, s->passes, s->negatives, s->mismatches);
        if (s->has_first_bad) {
            fprintf(out, "0x%08lX\n", (unsigned long)(uint32_t)s->first_bad);
        } else {
            fprintf(out, "-\n");
        }
    }
    if (out != stderr) {
        fclose(out);
    }
}

int32_t pico8_float_audit_flr_int(const char *file, const char *func, int line, pico8_fixed_t x) {
    pico8_float_audit_site_t *site = NULL;
    /* Entre parentheses : l'appel de la vraie fonction, pas de la macro. */
    int32_t now = (pico8_fixed_flr_int)(x);
    int before = (int)pico8_fixed_to_double(x);
    int i;

    if (!registered) {
        registered = 1;
        atexit(pico8_float_audit_dump);
    }
    for (i = 0; i < site_count; i++) {
        if (sites[i].line == line && strcmp(sites[i].file, file) == 0) {
            site = &sites[i];
            break;
        }
    }
    if (site == NULL && site_count < PICO8_FLOAT_AUDIT_MAX_SITES) {
        site = &sites[site_count++];
        memset(site, 0, sizeof *site);
        site->file = file;
        site->func = func;
        site->line = line;
    }
    if (site == NULL) {
        overflowed = 1;
        return now;
    }
    site->passes++;
    if (x < 0) {
        site->negatives++;
    }
    if (before != now) {
        site->mismatches++;
    }
    if ((x < 0 || before != now) && !site->has_first_bad) {
        site->has_first_bad = 1;
        site->first_bad = x;
    }
    return now;
}
