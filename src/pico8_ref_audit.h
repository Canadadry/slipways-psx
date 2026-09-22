#ifndef PICO8_REF_AUDIT_H
#define PICO8_REF_AUDIT_H

#include "pico8_list.h"
#include "pico8_type_tag.h"
#include <assert.h>

/* The REVERSE-direction half of this port's REF-cast problem, and the
 * reusable instrumentation that measures it.
 *
 * WHY THIS EXISTS. pico8_type_tag.h explains the forward direction: a
 * `pico8_list_t` read as a `pico8_ob_table_t` walks its backing array at
 * DOUBLE stride and overflows the allocation -- loud, ASan-visible, hit
 * four times. This header is about the mirror image, which is quiet: a
 * `pico8_ob_table_t` read as a `pico8_list_t`. The two structs have the
 * same layout down to `count`/`capacity` (tag, 4 bytes padding, a
 * pointer, two ints), so the header fields read back CORRECTLY -- it is
 * only the ELEMENTS that differ, 16-byte `pico8_value_t` against 32-byte
 * `pico8_ob_entry_t`. Reading N elements of 16 bytes out of an array of
 * N elements of 32 therefore stays comfortably inside the allocation and
 * never trips a sanitizer; what comes back is `items[0]` = entry 0's KEY
 * reinterpreted as a value (a PICO8_OB_KEY_INDEX kind of 0 reads as
 * PICO8_VALUE_FIXED, and the key's `name` pointer reads as that number's
 * bits), `items[1]` = entry 0's value, `items[2]` = entry 1's key, and so
 * on. The symptom is a wrong number on a panel, an empty line, or an
 * absurd glyph -- never a crash. Nobody notices.
 *
 * WHAT THIS HEADER IS. Two things, both deliberately zero-cost in an
 * ordinary build:
 *
 *   1. PICO8_REF_AS_LIST / PICO8_REF_AS_LIST_CONST: the ONE spelling
 *      every "REF value -> pico8_list_t *" conversion in src/ goes
 *      through. In a normal build each is exactly the plain cast the
 *      code already had, so nothing is added to the shipped program.
 *      Their real job is to make the set of such sites greppable and
 *      countable, which is what the pre-Phase-5 audit -- the one that
 *      marked `js:pq`'s own "lq" safe and was wrong -- had no way to be.
 *
 *   2. Under -DPICO8_REF_AUDIT, the same two macros record WHAT each
 *      site actually received, per site, tag by tag, and dump a report
 *      at exit. That turns "I read the code and believe this is a list"
 *      into a measurement, and -- just as importantly -- distinguishes a
 *      site that ran clean from a site no scenario ever reached. A site
 *      no method reaches is NOT COVERED, not safe; conflating the two is
 *      precisely how the previous audit produced its false negative.
 *
 * Deliberately header-only, function-local statics and all, for the same
 * reason pico8_type_tag.h is: every target in the Makefile would
 * otherwise have to grow a new .c in its source list, and one forgotten
 * list would produce a link error in an instrumented build nobody runs
 * by default. Each translation unit keeps its own little site table and
 * registers its own atexit dump; the sites of one .c file never appear
 * in another's, so the per-TU split costs nothing and the report tool
 * (tools/ref_audit_report.sh) simply concatenates.
 *
 * HOW TO RE-RUN IT. `make ref-audit`. That builds the headless replay
 * driver and the state-constructing tests into tmp/obj-refaudit (its own
 * object tree -- see the Makefile's own comment on why an instrumented
 * object may never be shared with an uninstrumented one), runs them over
 * every recorded events.json fixture in the repo, and prints one line
 * per cast site: the tags observed, or NOT COVERED. */

#ifdef PICO8_REF_AUDIT

#include <stdio.h>
#include <stdlib.h>

#define PICO8_REF_AUDIT_MAX_SITES 64

typedef struct {
    const char *file;
    int line;
    unsigned long n_list;
    unsigned long n_ob_table;
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
        fprintf(out, "SITE\t%s\t%d\tlist=%lu\tob_table=%lu\tinstance=%lu\tunknown=%lu\tnull=%lu\n", s->file, s->line,
                s->n_list, s->n_ob_table, s->n_instance, s->n_unknown, s->n_null);
    }
    if (out != stderr) {
        fclose(out);
    }
}

/* Records one arrival at one cast site and hands the pointer straight
 * back, so the macros below stay a pure pass-through. */
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
        site->n_ob_table = 0;
        site->n_instance = 0;
        site->n_unknown = 0;
        site->n_null = 0;
    }
    if (ref == NULL) {
        site->n_null++;
        return ref;
    }
    switch (pico8_type_tag_of(ref)) {
    case PICO8_TYPE_TAG_LIST:
        site->n_list++;
        break;
    case PICO8_TYPE_TAG_OB_TABLE:
        site->n_ob_table++;
        break;
    case PICO8_TYPE_TAG_INSTANCE:
        site->n_instance++;
        break;
    default:
        site->n_unknown++;
        break;
    }
    return ref;
}

#define PICO8_REF_AS_LIST(r) ((pico8_list_t *)pico8_ref_audit_note(__FILE__, __LINE__, (r)))
#define PICO8_REF_AS_LIST_CONST(r) ((const pico8_list_t *)pico8_ref_audit_note(__FILE__, __LINE__, (r)))

/* The SECOND dynamic channel, and the reason the audit does not simply
 * trust its own list of cast sites. The macros above can only see the
 * conversions this audit found and rewrote; this one sits at the ENTRY of
 * the list and ordered-table primitives themselves, so a mis-typed object
 * arriving through a path nobody enumerated -- a helper that takes an
 * already-converted pointer, a field this audit never looked at, a site
 * written after it -- still announces itself. Exactly the gap that let
 * the pre-Phase-5 audit conclude "already guarded" about `js:pq`'s "lq".
 *
 * Reports each distinct (function, wrong tag) pair once and then goes
 * quiet, so a mismatch in a per-frame primitive does not bury the run in
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

#define PICO8_REF_AUDIT_EXPECT_LIST(p) pico8_ref_audit_expect(__func__, (p), PICO8_TYPE_TAG_LIST)
#define PICO8_REF_AUDIT_EXPECT_OB_TABLE(p) pico8_ref_audit_expect(__func__, (p), PICO8_TYPE_TAG_OB_TABLE)

#else /* !PICO8_REF_AUDIT */

/* An ordinary build checks the tag and then casts. Reading a REF that is
 * not a list walks its backing array at the wrong stride, off the end of
 * the allocation -- the bug family that produced three segfaults. Each
 * site used to carry a comment arguing why its own cast was safe; the
 * check replaces all of them, and unlike a comment it cannot go stale. */
static inline pico8_list_t *pico8_ref_as_list(void *r) {
    assert((r == NULL || pico8_type_tag_of(r) == PICO8_TYPE_TAG_LIST) && "REF read as a list is not one");
    return (pico8_list_t *)r;
}

static inline const pico8_list_t *pico8_ref_as_list_const(const void *r) {
    assert((r == NULL || pico8_type_tag_of(r) == PICO8_TYPE_TAG_LIST) && "REF read as a list is not one");
    return (const pico8_list_t *)r;
}

#define PICO8_REF_AS_LIST(r) pico8_ref_as_list(r)
#define PICO8_REF_AS_LIST_CONST(r) pico8_ref_as_list_const(r)
#define PICO8_REF_AUDIT_EXPECT_LIST(p) ((void)0)
#define PICO8_REF_AUDIT_EXPECT_OB_TABLE(p) ((void)0)

#endif /* PICO8_REF_AUDIT */

#endif /* PICO8_REF_AUDIT_H */
