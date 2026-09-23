#include "pico8_float_observe.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PICO8_FLOAT_OBSERVE_MAX_SITES 512

typedef struct {
    const char *file;
    const char *func;
    int line;
    pico8_fixed_t minimum;
    unsigned long passes;
} pico8_float_observe_site_t;

static pico8_float_observe_site_t sites[PICO8_FLOAT_OBSERVE_MAX_SITES];
static int site_count;
static int overflowed;
static int registered;

/* Ajoute a $PICO8_FLOAT_OBSERVE_OUT (stderr sinon), en mode ajout : les
 * deversements de plusieurs rejeux s'additionnent dans un fichier que
 * tools/float_observe.sh replie. atexit ne tourne pas apres abort() : un
 * rejeu mort sur un assert ne deverse rien, et ses casts sortent NON
 * COUVERT ; make float-observe echoue alors sur son code de sortie. */
static void pico8_float_observe_dump(void) {
    const char *path = getenv("PICO8_FLOAT_OBSERVE_OUT");
    FILE *out = NULL;
    int i;

    if (path != NULL && path[0] != '\0') {
        out = fopen(path, "a");
    }
    if (out == NULL) {
        out = stderr;
    }
    if (overflowed) {
        fprintf(out, "OVERFLOW\tPICO8_FLOAT_OBSERVE_MAX_SITES trop petit\n");
    }
    for (i = 0; i < site_count; i++) {
        const pico8_float_observe_site_t *s = &sites[i];
        fprintf(out, "OBS\t%s\t%s\t%d\t%ld\t%lu\n", s->file, s->func, s->line, (long)s->minimum, s->passes);
    }
    if (out != stderr) {
        fclose(out);
    }
}

double pico8_float_observe_to_double(const char *file, const char *func, int line, pico8_fixed_t x) {
    pico8_float_observe_site_t *site = NULL;
    int i;

    if (!registered) {
        registered = 1;
        atexit(pico8_float_observe_dump);
    }
    for (i = 0; i < site_count; i++) {
        if (sites[i].line == line && strcmp(sites[i].file, file) == 0) {
            site = &sites[i];
            break;
        }
    }
    if (site == NULL && site_count < PICO8_FLOAT_OBSERVE_MAX_SITES) {
        site = &sites[site_count++];
        site->file = file;
        site->func = func;
        site->line = line;
        site->minimum = x;
        site->passes = 0;
    }
    if (site == NULL) {
        overflowed = 1;
    } else {
        if (x < site->minimum) {
            site->minimum = x;
        }
        site->passes++;
    }
    /* Entre parentheses : l'appel de la vraie fonction, pas de la macro. */
    return (pico8_fixed_to_double)(x);
}
