#include "pico8_float_audit.h"

#include "pico8_dispatch_context.h"

#include <math.h>
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
    unsigned long fractional;
    unsigned long not_ref;
    int has_first_bad;
    pico8_fixed_t first_bad;
    int has_first_mismatch_frame;
    int first_mismatch_frame;
} pico8_float_audit_site_t;

static pico8_float_audit_site_t sites[PICO8_FLOAT_AUDIT_MAX_SITES];
static int site_count;
static int overflowed;
static int registered;

/* Ajoute a $PICO8_FLOAT_AUDIT_OUT (stderr sinon), en mode ajout ; make
 * float-audit donne un fichier par fichier d'evenements. atexit ne tourne pas
 * apres abort() : un rejeu mort sur un assert ne deverse rien, son fichier
 * manque et tools/float_audit.sh echoue. */
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
            fprintf(out, "0x%08lX", (unsigned long)(uint32_t)s->first_bad);
        } else {
            fprintf(out, "-");
        }
        fprintf(out, "\t%lu\t%lu\t", s->fractional, s->not_ref);
        if (s->has_first_mismatch_frame) {
            fprintf(out, "%d\n", s->first_mismatch_frame);
        } else {
            fprintf(out, "-\n");
        }
    }
    if (out != stderr) {
        fclose(out);
    }
}

void pico8_float_audit_record(const char *file, const char *func, int line, pico8_fixed_t arg,
                              int old_differs, int new_differs_from_ref) {
    pico8_float_audit_site_t *site = NULL;
    const pico8_input_t *input = pico8_dispatch_input();
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
        return;
    }
    site->passes++;
    if (arg < 0) {
        site->negatives++;
    }
    if (((uint32_t)arg & 0xFFFFu) != 0) {
        site->fractional++;
    }
    if (old_differs) {
        site->mismatches++;
        if (!site->has_first_mismatch_frame) {
            site->has_first_mismatch_frame = 1;
            site->first_mismatch_frame = (input != NULL) ? input->current_frame : -1;
        }
    }
    if (new_differs_from_ref) {
        site->not_ref++;
    }
    if ((arg < 0 || old_differs) && !site->has_first_bad) {
        site->has_first_bad = 1;
        site->first_bad = arg;
    }
}

int32_t pico8_float_audit_flr_int(const char *file, const char *func, int line, pico8_fixed_t x) {
    /* Entre parentheses : l'appel de la vraie fonction, pas de la macro. */
    int32_t now = (pico8_fixed_flr_int)(x);
    int before = (int)pico8_fixed_to_double(x);
    int32_t reference = (int32_t)floor(pico8_fixed_to_double(x));

    pico8_float_audit_record(file, func, line, x, before != now, now != reference);
    return now;
}

void pico8_float_audit_dj(const char *file, const char *func, int line, pico8_fixed_t len_sq, pico8_fixed_t bq,
                          pico8_fixed_t x, pico8_fixed_t scale, pico8_fixed_t cost) {
    pico8_fixed_t old_ec = pico8_fixed_from_double(pow(pico8_fixed_to_double(len_sq) / pico8_fixed_to_double(bq), 0.75));
    pico8_fixed_t half = (pico8_fixed_t)0x00008000;
    pico8_fixed_t old_clamped = (old_ec > half) ? old_ec : half;
    pico8_fixed_t old_cost = pico8_fixed_flr(pico8_fixed_mul(scale, old_clamped));

    pico8_float_audit_record(file, func, line, x, old_cost != cost, 0);
}
