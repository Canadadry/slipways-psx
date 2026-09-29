#ifndef PICO8_REF_AUDIT_H
#define PICO8_REF_AUDIT_H

#include "pico8_type_tag.h"
#include <assert.h>

/* Per-site instrumentation of REF reads, compiled only under
 * -DPICO8_REF_AUDIT: pico8_ref_audit_note records, site by site, what each
 * REF that reached it pointed at and dumps the table at exit;
 * pico8_ref_audit_expect reports an object whose tag is not the expected
 * one. No macro or call site uses either any more, so a run records no
 * site. An ordinary build gets nothing from this header but its includes.
 *
 * Header-only, function-local statics and all: each translation unit
 * keeps its own site table and registers its own atexit dump, so the
 * sites of one .c file never appear in another's and the report tool
 * (tools/ref_audit_report.sh, run by `make ref-audit`) simply
 * concatenates. */

#ifdef PICO8_REF_AUDIT

#include <stdio.h>
#include <stdlib.h>

#define PICO8_REF_AUDIT_MAX_SITES 64

typedef struct {
    const char *file;
    int line;
    unsigned long n_list;
    unsigned long n_instance;
    unsigned long n_unknown;
    unsigned long n_null;
} pico8_ref_audit_site_t;

typedef struct {
    pico8_ref_audit_site_t sites[PICO8_REF_AUDIT_MAX_SITES];
    int count;
    int registered;
    int overflowed;
} pico8_ref_audit_state_t;

/* One table per translation unit (see this file's own top comment). */
static inline pico8_ref_audit_state_t *pico8_ref_audit_state(void) {
    static pico8_ref_audit_state_t state;
    return &state;
}

/* Appends this TU's sites to $PICO8_REF_AUDIT_OUT (stderr when unset).
 * Append mode on purpose: one run of one binary contributes its share,
 * and a whole campaign of binaries over a whole set of fixtures
 * accumulates into one file the report tool then folds together.
 *
 * KNOWN LIMIT, worth knowing before believing a NOT COVERED: this runs
 * from atexit(), and abort() does not run atexit handlers. A binary that
 * dies on a failed assert() (or on a sanitizer report) therefore
 * contributes NOTHING, not even the sites it reached before dying, and
 * every one of them reads back as NOT COVERED. tools/ref_audit_report.sh
 * prints each binary's exit status for exactly that reason -- a run whose
 * status is not 0 makes that binary's share of the coverage numbers
 * meaningless. */
static inline void pico8_ref_audit_dump(void) {
    pico8_ref_audit_state_t *state = pico8_ref_audit_state();
    const char *path = getenv("PICO8_REF_AUDIT_OUT");
    FILE *out = NULL;
    int i;

    if (path != NULL && path[0] != '\0') {
        out = fopen(path, "a");
    }
    if (out == NULL) {
        out = stderr;
    }
    if (state->overflowed) {
        fprintf(out, "OVERFLOW\t%s\tPICO8_REF_AUDIT_MAX_SITES too small\n",
                state->count > 0 ? state->sites[0].file : "?");
    }
    for (i = 0; i < state->count; i++) {
        const pico8_ref_audit_site_t *s = &state->sites[i];
        fprintf(out, "SITE\t%s\t%d\tlist=%lu\tinstance=%lu\tunknown=%lu\tnull=%lu\n", s->file, s->line, s->n_list,
                s->n_instance, s->n_unknown, s->n_null);
    }
    if (out != stderr) {
        fclose(out);
    }
}

/* Records one arrival at one site and hands the pointer straight back. */
static inline const void *pico8_ref_audit_note(const char *file, int line, const void *ref) {
    pico8_ref_audit_state_t *state = pico8_ref_audit_state();
    pico8_ref_audit_site_t *site = NULL;
    int i;

    if (!state->registered) {
        state->registered = 1;
        atexit(pico8_ref_audit_dump);
    }
    for (i = 0; i < state->count; i++) {
        if (state->sites[i].line == line && state->sites[i].file == file) {
            site = &state->sites[i];
            break;
        }
    }
    if (site == NULL) {
        if (state->count >= PICO8_REF_AUDIT_MAX_SITES) {
            state->overflowed = 1;
            return ref;
        }
        site = &state->sites[state->count++];
        site->file = file;
        site->line = line;
        site->n_list = 0;
        site->n_instance = 0;
        site->n_unknown = 0;
        site->n_null = 0;
    }
    if (ref == NULL) {
        site->n_null++;
        return ref;
    }
    switch (pico8_type_tag_of(ref)) {
    default:
        site->n_unknown++;
        break;
    }
    return ref;
}

/* Reports each distinct (function, wrong tag) pair once and then goes
 * quiet, so a mismatch in a per-frame caller does not bury the run in
 * megabytes of identical lines. */
static inline void pico8_ref_audit_expect(const char *what, const void *p, pico8_type_tag_t expected) {
    static const char *reported[16];
    static pico8_type_tag_t reported_tag[16];
    static int reported_count;
    pico8_type_tag_t actual;
    const char *path;
    FILE *out;
    int i;

    if (p == NULL) {
        return;
    }
    actual = pico8_type_tag_of(p);
    if (actual == expected) {
        return;
    }
    for (i = 0; i < reported_count; i++) {
        if (reported[i] == what && reported_tag[i] == actual) {
            return;
        }
    }
    if (reported_count < 16) {
        reported[reported_count] = what;
        reported_tag[reported_count] = actual;
        reported_count++;
    }
    path = getenv("PICO8_REF_AUDIT_OUT");
    out = (path != NULL && path[0] != '\0') ? fopen(path, "a") : NULL;
    if (out == NULL) {
        out = stderr;
    }
    fprintf(out, "MISMATCH\t%s\texpected=0x%08X\tactual=0x%08X\n", what, (unsigned)expected, (unsigned)actual);
    if (out != stderr) {
        fclose(out);
    }
}

#endif /* PICO8_REF_AUDIT */

#endif /* PICO8_REF_AUDIT_H */
