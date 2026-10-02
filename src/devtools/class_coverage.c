#include "devtools/class_coverage.h"

#ifdef PICO8_CLASS_COVERAGE
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *name;
    long count;
} class_coverage_entry_t;

static class_coverage_entry_t *g_coverage = NULL;
static int g_coverage_count = 0;
static int g_coverage_capacity = 0;

void class_coverage_record(const char *name) {
    int i;

    assert(name != NULL && "coverage: a NULL name cannot be reported as missing from KNOWN[]");
    for (i = 0; i < g_coverage_count; i++) {
        if (strcmp(g_coverage[i].name, name) == 0) {
            g_coverage[i].count++;
            return;
        }
    }
    if (g_coverage_count == g_coverage_capacity) {
        int new_capacity = g_coverage_capacity == 0 ? 32 : g_coverage_capacity * 2;
        class_coverage_entry_t *grown = realloc(g_coverage, (size_t)new_capacity * sizeof *grown);
        if (grown == NULL) {
            abort();
        }
        g_coverage = grown;
        g_coverage_capacity = new_capacity;
    }
    g_coverage[g_coverage_count].name = name;
    g_coverage[g_coverage_count].count = 1;
    g_coverage_count++;
}

long class_coverage_count(const char *name) {
    int i;

    for (i = 0; i < g_coverage_count; i++) {
        if (strcmp(g_coverage[i].name, name) == 0) {
            return g_coverage[i].count;
        }
    }
    return 0;
}

static int is_known(const char *name, const char *const *known, int known_count) {
    int i;

    for (i = 0; i < known_count; i++) {
        if (strcmp(known[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

const char *class_coverage_first_unknown(const char *const *known, int known_count) {
    int i;

    for (i = 0; i < g_coverage_count; i++) {
        if (!is_known(g_coverage[i].name, known, known_count)) {
            return g_coverage[i].name;
        }
    }
    return NULL;
}
#endif
