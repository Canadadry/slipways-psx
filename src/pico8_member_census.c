#include "pico8_member_census.h"

#ifdef PICO8_MEMBER_CENSUS
#define _POSIX_C_SOURCE 200112L
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pico8_list.h"
#include "pico8_struct.h"
#include "pico8_type_tag.h"
#include "pico8_value.h"

typedef struct {
    const char *name;
    int parent;
    int by_name;
} pico8_member_census_typedef_t;

typedef struct {
    int td;
    const char *name;
    size_t offset;
    int by_name;
} pico8_member_census_member_t;

#include "pico8_member_census_table.h"

#define TYPEDEF_COUNT ((int)(sizeof g_census_typedefs / sizeof g_census_typedefs[0]))
#define MEMBER_COUNT PICO8_MEMBER_CENSUS_MEMBER_COUNT
#define MAX_DESCRIPTORS 128
#define MAX_TARGETS 8
#define MAX_PENDING 1024
#define MAX_UNKNOWN 64
#define MAX_ERRORS 16
#define NAME_CAP 32

enum {
    K_FIXE = 1,
    K_CHAINE = 2,
    K_BOOL = 4,
    K_FONCTION = 8,
    K_LISTE = 16,
    K_TABLE = 32,
    K_AUTRE = 64,
    K_STRUCT = 128
};

enum {
    F_NIL_HELD = 1,
    F_NIL_READ = 2,
    F_NIL_WRITTEN = 4,
    F_READ = 8,
    F_WRITTEN = 16,
    F_DEFERRED = 32,
    F_TARGET_OVERFLOW = 64,
    F_MUTED = 128
};

typedef struct {
    const void *cls;
    char name[NAME_CAP];
    int found;
    int is_write;
    unsigned kinds;
    const void *target;
} pico8_member_census_pending_t;

static unsigned g_kinds[MEMBER_COUNT + 1];
static unsigned g_flags[MEMBER_COUNT + 1];
static const void *g_target_cls[MEMBER_COUNT + 1][MAX_TARGETS];
static long g_seen[TYPEDEF_COUNT];
static int g_in_hv[TYPEDEF_COUNT];
static int g_first[TYPEDEF_COUNT];
static int g_count[TYPEDEF_COUNT];
static const void *g_cls[MAX_DESCRIPTORS];
static int g_cls_td[MAX_DESCRIPTORS];
static int g_cls_count;
static pico8_member_census_pending_t g_pending[MAX_PENDING];
static int g_pending_count;
static const void *g_unknown[MAX_UNKNOWN];
static int g_unknown_count;
static char g_errors[MAX_ERRORS][96];
static int g_error_count;
static long g_muted;
static long g_deferred;
static long g_by_name_nil;
static long g_outside;
static long g_overflow;
static int g_started;

static void report_error(const char *what, const char *a, const char *b) {
    if (g_error_count < MAX_ERRORS) {
        snprintf(g_errors[g_error_count], sizeof g_errors[0], "%s %s %s", what, a, b);
    }
    g_error_count++;
}

static void add_unknown(const void *cls) {
    int i;

    for (i = 0; i < g_unknown_count; i++) {
        if (g_unknown[i] == cls) {
            return;
        }
    }
    if (g_unknown_count < MAX_UNKNOWN) {
        g_unknown[g_unknown_count++] = cls;
    } else {
        g_overflow++;
    }
}

static int td_of_cls(const void *cls) {
    int i;

    for (i = 0; i < g_cls_count; i++) {
        if (g_cls[i] == cls) {
            return g_cls_td[i];
        }
    }
    return -1;
}

static void kinds_to_text(unsigned kinds, const void *const *targets, char *out, size_t cap);

static void dump(void) {
    const char *dir = getenv("PICO8_MEMBER_CENSUS_OUT");
    char path[512];
    FILE *f;
    int i;
    int k;

    if (dir == NULL || dir[0] == '\0') {
        return;
    }
    snprintf(path, sizeof path, "%s/%ld.txt", dir, (long)getpid());
    f = fopen(path, "a");
    if (f == NULL) {
        return;
    }
    for (i = 0; i < g_pending_count; i++) {
        add_unknown(g_pending[i].cls);
    }
    for (i = 0; i < TYPEDEF_COUNT; i++) {
        if (g_seen[i] > 0 || g_in_hv[i]) {
            fprintf(f, "T\t%s\t%ld\t%d\n", g_census_typedefs[i].name, g_seen[i], g_in_hv[i]);
        }
    }
    for (i = 0; i < MEMBER_COUNT; i++) {
        char kinds[512];
        char flags[16];
        int n = 0;

        if (g_kinds[i] == 0 && g_flags[i] == 0) {
            continue;
        }
        kinds_to_text(g_kinds[i], g_target_cls[i], kinds, sizeof kinds);
        if (g_flags[i] & F_NIL_HELD) {
            flags[n++] = 'h';
        }
        if (g_flags[i] & F_NIL_READ) {
            flags[n++] = 'l';
        }
        if (g_flags[i] & F_NIL_WRITTEN) {
            flags[n++] = 'e';
        }
        if (g_flags[i] & F_READ) {
            flags[n++] = 'r';
        }
        if (g_flags[i] & F_WRITTEN) {
            flags[n++] = 'w';
        }
        if (g_flags[i] & F_DEFERRED) {
            flags[n++] = 'd';
        }
        if (g_flags[i] & F_MUTED) {
            flags[n++] = 'm';
        }
        if (g_flags[i] & F_TARGET_OVERFLOW) {
            flags[n++] = 'o';
        }
        flags[n] = '\0';
        fprintf(f, "M\t%s\t%s\t%s\t%s\n", g_census_typedefs[g_census_members[i].td].name, g_census_members[i].name,
                kinds[0] != '\0' ? kinds : "-", n > 0 ? flags : "-");
    }
    for (i = 0; i < MEMBER_COUNT; i++) {
        for (k = 0; k < MAX_TARGETS && g_target_cls[i][k] != NULL; k++) {
            if (td_of_cls(g_target_cls[i][k]) < 0) {
                add_unknown(g_target_cls[i][k]);
            }
        }
    }
    for (i = 0; i < g_unknown_count; i++) {
        if (td_of_cls(g_unknown[i]) < 0) {
            fprintf(f, "U\t%p\n", g_unknown[i]);
        }
    }
    for (i = 0; i < g_error_count && i < MAX_ERRORS; i++) {
        fprintf(f, "X\t%s\n", g_errors[i]);
    }
    fprintf(f, "S\tmuets\t%ld\n", g_muted);
    fprintf(f, "S\tdifferees\t%ld\n", g_deferred);
    fprintf(f, "S\tnil-par-nom\t%ld\n", g_by_name_nil);
    fprintf(f, "S\thors-membre\t%ld\n", g_outside);
    fprintf(f, "S\tdebordements\t%ld\n", g_overflow);
    fprintf(f, "S\terreurs\t%d\n", g_error_count);
    fclose(f);
}

static void start(void) {
    int i;
    int m;

    if (g_started) {
        return;
    }
    g_started = 1;
    for (i = 0; i < TYPEDEF_COUNT; i++) {
        g_first[i] = -1;
    }
    for (m = 0; m < MEMBER_COUNT; m++) {
        int td = g_census_members[m].td;
        if (g_first[td] < 0) {
            g_first[td] = m;
        }
        g_count[td]++;
    }
    atexit(dump);
}

static int td_by_name(const char *typedef_name) {
    int i;

    for (i = 0; i < TYPEDEF_COUNT; i++) {
        if (strcmp(g_census_typedefs[i].name, typedef_name) == 0) {
            return i;
        }
    }
    return -1;
}

/* The member `name` of the chain of `td`, most derived first; -1 when none. */
static int resolve(int td, const char *name) {
    for (; td >= 0; td = g_census_typedefs[td].parent) {
        int m;
        for (m = g_first[td]; m >= 0 && m < g_first[td] + g_count[td]; m++) {
            if (strcmp(g_census_members[m].name, name) == 0) {
                return m;
            }
        }
    }
    return -1;
}

static void add_target(int m, const void *cls) {
    int k;

    for (k = 0; k < MAX_TARGETS && g_target_cls[m][k] != NULL; k++) {
        if (g_target_cls[m][k] == cls) {
            return;
        }
    }
    if (k == MAX_TARGETS) {
        g_flags[m] |= F_TARGET_OVERFLOW;
        return;
    }
    g_target_cls[m][k] = cls;
}

static unsigned kind_of(const pico8_value_t *v, const void **target) {
    *target = NULL;
    switch (v->kind) {
    case PICO8_VALUE_FIXED:
        return K_FIXE;
    case PICO8_VALUE_STRING:
        return K_CHAINE;
    case PICO8_VALUE_BOOL:
        return K_BOOL;
    case PICO8_VALUE_FUNCTION:
        return K_FONCTION;
    case PICO8_VALUE_REF:
        if (v->as.ref == NULL) {
            return 0;
        }
        switch (pico8_type_tag_of(v->as.ref)) {
        case PICO8_TYPE_TAG_LIST:
            return K_LISTE;
        case PICO8_TYPE_TAG_OB_TABLE:
            return K_TABLE;
        case PICO8_TYPE_TAG_STRUCT:
            *target = ((const pico8_struct_header_t *)v->as.ref)->cls;
            return K_STRUCT;
        default:
            return K_AUTRE;
        }
    }
    return K_AUTRE;
}

static int is_nil(const pico8_value_t *v) {
    return v->kind == PICO8_VALUE_REF && v->as.ref == NULL;
}

static void note_value(int m, const pico8_value_t *v) {
    const void *target;
    unsigned k = kind_of(v, &target);

    if (is_nil(v)) {
        g_flags[m] |= F_NIL_HELD;
        return;
    }
    g_kinds[m] |= k;
    if (target != NULL) {
        add_target(m, target);
    }
}

static const pico8_value_t *member_at(const void *obj, int m) {
    return (const pico8_value_t *)((const char *)obj + g_census_members[m].offset);
}

static void snapshot(const void *obj, int td) {
    for (; td >= 0; td = g_census_typedefs[td].parent) {
        int m;
        for (m = g_first[td]; m >= 0 && m < g_first[td] + g_count[td]; m++) {
            if (!g_census_members[m].by_name) {
                note_value(m, member_at(obj, m));
            }
        }
    }
}

static void apply_read(int m, const void *e, int found, const pico8_value_t *out) {
    g_flags[m] |= F_READ;
    if (found) {
        note_value(m, g_census_members[m].by_name ? out : member_at(e, m));
        return;
    }
    if (g_census_members[m].by_name) {
        g_flags[m] |= F_NIL_READ;
        g_by_name_nil++;
    } else if (is_nil(member_at(e, m))) {
        g_flags[m] |= F_NIL_READ | F_NIL_HELD;
    } else {
        g_flags[m] |= F_MUTED;
        g_muted++;
    }
}

static void apply_write(int m, const pico8_value_t *value) {
    g_flags[m] |= F_WRITTEN;
    if (is_nil(value)) {
        g_flags[m] |= F_NIL_WRITTEN;
    }
    note_value(m, value);
}

static void add_pending(const void *cls, const char *name, int found, int is_write, const pico8_value_t *v) {
    pico8_member_census_pending_t p;
    int i;

    memset(&p, 0, sizeof p);
    p.cls = cls;
    strncpy(p.name, name, NAME_CAP - 1);
    p.found = found;
    p.is_write = is_write;
    if (v != NULL && (found || is_write)) {
        p.kinds = kind_of(v, &p.target);
        if (is_nil(v)) {
            p.kinds = 0;
        }
    }
    for (i = 0; i < g_pending_count; i++) {
        if (memcmp(&g_pending[i], &p, sizeof p) == 0) {
            return;
        }
    }
    if (g_pending_count == MAX_PENDING) {
        g_overflow++;
        return;
    }
    g_pending[g_pending_count++] = p;
}

static void resolve_pending(const void *cls, int td) {
    int i = 0;

    while (i < g_pending_count) {
        pico8_member_census_pending_t p = g_pending[i];
        int m;

        if (p.cls != cls) {
            i++;
            continue;
        }
        g_pending[i] = g_pending[--g_pending_count];
        m = resolve(td, p.name);
        if (m < 0) {
            if (p.is_write) {
                report_error("ecriture-hors-membre", g_census_typedefs[td].name, p.name);
            } else if (p.found) {
                g_outside++;
            }
            continue;
        }
        if (p.is_write) {
            g_flags[m] |= F_WRITTEN;
            if (p.kinds == 0 && p.target == NULL) {
                g_flags[m] |= F_NIL_WRITTEN | F_NIL_HELD;
            }
        } else {
            g_flags[m] |= F_READ;
            if (!p.found) {
                g_flags[m] |= F_NIL_READ | F_DEFERRED;
                g_deferred++;
            }
        }
        g_kinds[m] |= p.kinds;
        if (p.target != NULL) {
            add_target(m, p.target);
        }
    }
}

void pico8_member_census_see(const void *obj, const char *typedef_name) {
    const void *cls = ((const pico8_struct_header_t *)obj)->cls;
    int td;
    int known;

    start();
    td = td_by_name(typedef_name);
    if (td < 0) {
        report_error("typedef-inconnu", typedef_name, "");
        return;
    }
    known = td_of_cls(cls);
    if (known >= 0 && known != td) {
        report_error("descripteur-a-deux-typedefs", g_census_typedefs[known].name, typedef_name);
        return;
    }
    if (known < 0) {
        if (g_cls_count == MAX_DESCRIPTORS) {
            g_overflow++;
            return;
        }
        g_cls[g_cls_count] = cls;
        g_cls_td[g_cls_count] = td;
        g_cls_count++;
        resolve_pending(cls, td);
    }
    g_seen[td]++;
    snapshot(obj, td);
}

int pico8_member_census_read(const void *e, const char *name, int found, const void *out_value) {
    const void *cls = ((const pico8_struct_header_t *)e)->cls;
    int td;
    int m;

    start();
    td = td_of_cls(cls);
    if (td < 0) {
        add_pending(cls, name, found, 0, out_value);
        return found;
    }
    m = resolve(td, name);
    if (m < 0) {
        if (found) {
            g_outside++;
        }
        return found;
    }
    apply_read(m, e, found, out_value);
    return found;
}

void pico8_member_census_write(const void *e, const char *name, const void *value) {
    const void *cls = ((const pico8_struct_header_t *)e)->cls;
    int td;
    int m;

    start();
    td = td_of_cls(cls);
    if (td < 0) {
        add_pending(cls, name, 0, 1, value);
        return;
    }
    m = resolve(td, name);
    if (m < 0) {
        report_error("ecriture-hors-membre", g_census_typedefs[td].name, name);
        return;
    }
    apply_write(m, value);
}

void pico8_member_census_walk(const void *hv) {
    const pico8_list_t *list = hv;
    int i;

    start();
    for (i = 0; i < list->count; i++) {
        const pico8_value_t *v = &list->items[i];
        int td;

        if (v->kind != PICO8_VALUE_REF || pico8_type_tag_of(v->as.ref) != PICO8_TYPE_TAG_STRUCT) {
            continue;
        }
        td = td_of_cls(((const pico8_struct_header_t *)v->as.ref)->cls);
        if (td < 0) {
            add_unknown(((const pico8_struct_header_t *)v->as.ref)->cls);
            continue;
        }
        g_in_hv[td] = 1;
        snapshot(v->as.ref, td);
    }
}

static void append(char *out, size_t cap, const char *word) {
    size_t n = strlen(out);

    if (n > 0 && n + 1 < cap) {
        out[n++] = ',';
        out[n] = '\0';
    }
    strncat(out, word, cap - n - 1);
}

static void kinds_to_text(unsigned kinds, const void *const *targets, char *out, size_t cap) {
    int k;

    out[0] = '\0';
    if (kinds & K_FIXE) {
        append(out, cap, "fixe");
    }
    if (kinds & K_CHAINE) {
        append(out, cap, "chaine");
    }
    if (kinds & K_BOOL) {
        append(out, cap, "bool");
    }
    if (kinds & K_FONCTION) {
        append(out, cap, "fonction");
    }
    if (kinds & K_LISTE) {
        append(out, cap, "ref:liste");
    }
    if (kinds & K_TABLE) {
        append(out, cap, "ref:table");
    }
    if (kinds & K_AUTRE) {
        append(out, cap, "ref:autre");
    }
    for (k = 0; k < MAX_TARGETS && targets[k] != NULL; k++) {
        int td = td_of_cls(targets[k]);
        char word[64];

        snprintf(word, sizeof word, "ref:%s", td >= 0 ? g_census_typedefs[td].name : "?");
        append(out, cap, word);
    }
}
#endif
