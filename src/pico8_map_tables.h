#ifndef PICO8_MAP_TABLES_H
#define PICO8_MAP_TABLES_H

enum {
    PICO8_R_COUNT = 13,
    PICO8_IZ_COUNT = 11,
    PICO8_HC_COUNT = 6,
    PICO8_FD_COUNT = 42,
    PICO8_DR_COUNT = 7,
    PICO8_NB_COUNT = 19
};

typedef struct {
    const char *key;
    int qe;
} pico8_r_entry_t;

typedef struct {
    const char *key;
    const char *name;
} pico8_iz_entry_t;

/* values[0..count-1] are the entry's positionals; mc is NULL when absent. */
typedef struct {
    const char *key;
    int count;
    int values[4];
    const char *mc;
} pico8_hc_entry_t;

/* An item is the string s when s is non-NULL, the row when row is non-NULL,
 * the number n otherwise. */
typedef struct pico8_jb_row pico8_jb_row_t;
typedef struct {
    const char *s;
    int n;
    const pico8_jb_row_t *row;
} pico8_jb_item_t;

/* c is only meaningful when has_c is set. */
struct pico8_jb_row {
    int count;
    const pico8_jb_item_t *items;
    int has_c;
    int c;
};

/* A non-NULL header makes the entry a bare string, the rr of the records that
 * follow, every other field zero. Otherwise positions 1..npos are present:
 * px, lr, fj, ja, i, o, il, dm; il_false marks position 7 written `f` (false),
 * il then being NULL. ii and rn are only meaningful under has_ii/has_rn. */
typedef struct {
    const char *header;
    int npos;
    const char *px;
    const char *lr;
    int fj;
    const char *ja;
    const char *i;
    const char *o;
    const char *il;
    const char *dm;
    int il_false;
    int has_ii;
    int ii;
    const char *go;
    const char *cw;
    int has_rn;
    int rn;
} pico8_fd_entry_t;

/* nk_key is position 3 as written ("t"); nk is r[nk_key]. */
typedef struct {
    const char *px;
    int lv;
    const char *nk_key;
    int nk;
    int od;
    const pico8_jb_row_t *oy;
} pico8_nb_entry_t;

extern const pico8_r_entry_t pico8_r_table[PICO8_R_COUNT];
extern const pico8_iz_entry_t pico8_iz_table[PICO8_IZ_COUNT];
extern const pico8_hc_entry_t pico8_hc_table[PICO8_HC_COUNT];
extern const pico8_fd_entry_t pico8_fd_table[PICO8_FD_COUNT];
extern const pico8_jb_row_t pico8_dr_table[PICO8_DR_COUNT];
extern const pico8_nb_entry_t pico8_nb_table[PICO8_NB_COUNT];

/* Returns 1 and writes *out_qe when key is present, 0 and leaves *out_qe
 * untouched otherwise. */
int pico8_r_lookup(const char *key, int *out_qe);
/* NULL when key is absent. */
const char *pico8_iz_lookup(const char *key);
/* NULL when key is absent; otherwise a pointer into pico8_hc_table. */
const pico8_hc_entry_t *pico8_hc_lookup(const char *key);

#endif
