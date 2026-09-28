#include "pico8_research.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_research_display.h"
#include "pico8_tech_gate.h"
#include "pico8_type_tag.h"

static int g_hp_up = 0;

/* `gb[k]`, 1-based rank k: the projects `build_gb` made, which it owns. */
static pico8_hp_t *g_gb[PICO8_FD_COUNT];
static int g_gb_count = 0;
/* `gb[px]`: the last project registered under each `px` (several share
 * one), built by `build_gb` or not. 0 until `build_gb`. `px` is the hp's own
 * pointer, borrowed, never copied: it must outlive the hp (every caller
 * passes a string of `pico8_fd_table`). */
static struct {
    const char *px;
    pico8_hp_t *hp;
} g_gb_px[PICO8_FD_COUNT];
static int g_gb_px_count = 0;
static int g_gb_up = 0;
/* Borrowed from `pico8_fd_table`, never copied or freed here. */
static const char *g_rr = NULL;

static int value_is_nil(pico8_value_t v) {
    return v.kind == PICO8_VALUE_REF && v.as.ref == NULL;
}

/* Lua truthiness, widened so this port's "absent" filler REF(NULL) is
 * falsy too. */
static int pico8_research_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static void pico8_research_free_gb_instances(void) {
    int i;

    for (i = 0; i < g_gb_count; i++) {
        free(g_gb[i]);
    }
    g_gb_count = 0;
    g_gb_px_count = 0;
}

static void pico8_research_gb_register(pico8_hp_t *hp) {
    int i;

    for (i = 0; i < g_gb_px_count; i++) {
        if (strcmp(g_gb_px[i].px, hp->px) == 0) {
            g_gb_px[i].hp = hp;
            return;
        }
    }
    assert(g_gb_px_count < PICO8_FD_COUNT && "gb[px]: every px comes from fd, which has fewer names than records");
    g_gb_px[g_gb_px_count].px = hp->px;
    g_gb_px[g_gb_px_count].hp = hp;
    g_gb_px_count++;
}

static pico8_value_t pico8_hp_lq_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(pico8_hp_lq((pico8_hp_t *)self));
}

/* A nil `lq` member falls back to the class's `lq` method, as the cart's
 * `mv(b,"lq")` finds it: the member written by the constructor shadows the
 * method for a project with `ii`. An `lq` entry in the descriptor's methods
 * would be looked up first and invert that. */
static int pico8_hp_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_hp_t *hp = self;

    if (strcmp(name, "mr") == 0) {
        if (hp->mr == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)hp->mr);
        return 1;
    }
    if (strcmp(name, "gp") == 0) {
        if (hp->gp == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)hp->gp);
        return 1;
    }
    if (strcmp(name, "fk") == 0) {
        if (hp->fk == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)hp->fk);
        return 1;
    }
    if (strcmp(name, "px") == 0) {
        if (hp->px == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hp->px);
        return 1;
    }
    if (strcmp(name, "lr") == 0) {
        if (hp->lr == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hp->lr);
        return 1;
    }
    if (strcmp(name, "fj") == 0) {
        if (!hp->has_fj) {
            return 0;
        }
        *out_value = pico8_value_fixed(hp->fj);
        return 1;
    }
    if (strcmp(name, "ja") == 0) {
        if (hp->ja == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hp->ja);
        return 1;
    }
    if (strcmp(name, "i") == 0) {
        if (hp->i == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(hp->i);
        return 1;
    }
    if (strcmp(name, "o") == 0) {
        if (!hp->has_o) {
            return 0;
        }
        *out_value = pico8_value_fixed(hp->o);
        return 1;
    }
    if (strcmp(name, "il") == 0) {
        pico8_value_t v = pico8_hp_il(hp);
        if (value_is_nil(v)) {
            return 0;
        }
        *out_value = v;
        return 1;
    }
    if (strcmp(name, "dm") == 0) {
        if (!hp->has_dm) {
            return 0;
        }
        *out_value = pico8_value_fixed(hp->dm);
        return 1;
    }
    if (strcmp(name, "rr") == 0) {
        if (hp->rr == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hp->rr);
        return 1;
    }
    if (strcmp(name, "ii") == 0) {
        if (!hp->has_ii) {
            return 0;
        }
        *out_value = pico8_value_fixed(hp->ii);
        return 1;
    }
    if (strcmp(name, "cw") == 0) {
        if (hp->cw == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hp->cw);
        return 1;
    }
    if (strcmp(name, "go") == 0) {
        if (hp->go == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hp->go);
        return 1;
    }
    if (strcmp(name, "rn") == 0) {
        if (!hp->has_rn) {
            return 0;
        }
        *out_value = pico8_value_fixed(hp->rn);
        return 1;
    }
    if (strcmp(name, "lq") == 0) {
        if (hp->lq == NULL) {
            *out_value = pico8_value_function(pico8_hp_lq_dispatch);
            return 1;
        }
        *out_value = pico8_value_ref((void *)hp->lq);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_hp_struct_class = {
    {NULL},
    pico8_hp_field,
    NULL,
};

/* `s` NULL with `is_fixed` 0 is false. `i` gets the raw string that
 * `hp:pq` turns into its list afterwards. */
static void hp_pose(pico8_hp_t *self, const char *name, const char *s, int is_fixed, pico8_fixed_t fixed,
                    const char **i_src) {
    if (strcmp(name, "px") == 0 || strcmp(name, "lr") == 0 || strcmp(name, "ja") == 0 || strcmp(name, "i") == 0) {
        assert(s != NULL && "hp.px/lr/ja/i: a position that is not a string does not fit the member's type "
                            "-- stop, do not widen the member");
        if (strcmp(name, "px") == 0) {
            self->px = s;
        } else if (strcmp(name, "lr") == 0) {
            self->lr = s;
        } else if (strcmp(name, "ja") == 0) {
            self->ja = s;
        } else {
            *i_src = s;
        }
    } else if (strcmp(name, "fj") == 0 || strcmp(name, "o") == 0 || strcmp(name, "dm") == 0) {
        assert(is_fixed && "hp.fj/o/dm: a position that is not a number does not fit the member's type "
                           "-- stop, do not widen the member");
        if (strcmp(name, "fj") == 0) {
            self->fj = fixed;
            self->has_fj = 1;
        } else if (strcmp(name, "o") == 0) {
            self->o = fixed;
            self->has_o = 1;
        } else {
            self->dm = fixed;
            self->has_dm = 1;
        }
    } else {
        assert(strcmp(name, "il") == 0 && !is_fixed &&
               "hp: `mr` names a member hp does not hold, or `il` gets a number -- stop, do not add one");
        self->il = s;
        self->has_il = 1;
    }
}

/* `ho(gb, mr, gp)` on one record of `fd`: position `p` names the member
 * `mr[p]`, and a member that `gp` names takes `r[value]`. */
static void hp_ho(pico8_hp_t *self, const pico8_fd_entry_t *ri, const char **i_src) {
    const char *const strings[8] = {ri->px, ri->lr, NULL, ri->ja, ri->i, ri->o, ri->il, ri->dm};
    int p;

    assert(ri->header == NULL && "hp: a header entry of fd is the rr of the records after it, not a project");
    assert(ri->npos <= self->mr->count && ri->npos <= 8);
    for (p = 0; p < ri->npos; p++) {
        const char *name = self->mr->items[p];
        const char *s = p == 6 && ri->il_false ? NULL : strings[p];
        int is_fixed = p == 2;
        pico8_fixed_t fixed = is_fixed ? pico8_fixed_from_int(ri->fj) : 0;

        if (s != NULL && pico8_ho_gp_has(self->gp, name)) {
            int qe = 0;
            int found = pico8_r_lookup(s, &qe);

            assert(found && "hp: a value that gp maps through r is missing from r; it would stay a string in a "
                            "fixed member -- stop, do not widen the member");
            (void)found;
            s = NULL;
            is_fixed = 1;
            fixed = pico8_fixed_from_int(qe);
        }
        hp_pose(self, name, s, is_fixed, fixed, i_src);
    }
}

pico8_value_t pico8_jy_qe(const pico8_jy_t *i) {
    return i->row.has_qe ? pico8_value_fixed(i->row.qe) : pico8_value_ref(NULL);
}

pico8_jy_t *pico8_jy_new(const pico8_fixed_t *qe) {
    pico8_jy_t *i = calloc(1, sizeof *i);

    assert(i != NULL);
    if (qe != NULL) {
        i->row.has_qe = 1;
        i->row.qe = *qe;
    }
    return i;
}

/* TROU-7 */
/* `i` must be rewritten after `ho`, which leaves the raw string in it; the
 * real `gb[px]` must hold the struct. */
static void *pico8_hp_struct_new(const pico8_hp_keys_t *keys) {
    pico8_hp_t *self = malloc(sizeof *self);
    pico8_list_t *new_i_heap;
    const char *i_src = "";
    size_t c;

    self->header.tag = PICO8_TYPE_TAG_STRUCT;
    self->header.cls = &g_hp_struct_class;
    self->mr = NULL;
    self->gp = NULL;
    self->fk = NULL;
    self->px = NULL;
    self->lr = NULL;
    self->fj = 0;
    self->has_fj = 0;
    self->ja = NULL;
    self->i = NULL;
    self->o = 0;
    self->has_o = 0;
    self->il = NULL;
    self->has_il = 0;
    self->dm = 0;
    self->has_dm = 0;
    self->rr = NULL;
    self->ii = 0;
    self->has_ii = 0;
    self->cw = NULL;
    self->go = NULL;
    self->rn = 0;
    self->has_rn = 0;
    self->lq = NULL;
    self->mr = pico8_hp_start.mr;
    self->gp = pico8_hp_start.gp;
    self->fk = pico8_hp_start.fk;

    if (keys != NULL && keys->ri != NULL) {
        const pico8_fd_entry_t *ri = keys->ri;

        hp_ho(self, ri, &i_src);
        if (ri->has_ii) {
            self->ii = pico8_fixed_from_int(ri->ii);
            self->has_ii = 1;
        }
        self->cw = ri->cw;
        self->go = ri->go;
        if (ri->has_rn) {
            self->rn = pico8_fixed_from_int(ri->rn);
            self->has_rn = 1;
        }
    }
    if (keys != NULL) {
        self->rr = keys->rr;
    }

    if (self->px != NULL) {
        assert(g_gb_up && "hp:pq registers itself in gb: build gb before instantiating an hp");
        pico8_research_gb_register(self);
    }

    if (self->has_ii) {
        assert(self->px != NULL && "hp.lq: {px, 32, {ii, 8}} has no hole; a project with ii and no px "
                                   "does not fit the row -- stop, do not widen the member");
        self->lq_inner_items[0] = (pico8_jb_item_t){.is_fixed = 1, .fixed = self->ii};
        self->lq_inner_items[1] = (pico8_jb_item_t){.n = 8};
        self->lq_inner = (pico8_jb_row_t){.count = 2, .items = self->lq_inner_items};
        self->lq_own_items[0] = (pico8_jb_item_t){.s = self->px};
        self->lq_own_items[1] = (pico8_jb_item_t){.n = 32};
        self->lq_own_items[2] = (pico8_jb_item_t){.row = &self->lq_inner};
        self->lq_own = (pico8_jb_row_t){.count = 3, .items = self->lq_own_items};
        self->lq = &self->lq_own;
    }

    /* A want for every character, never skipping one: the Lua table
     * constructor is non-nil even when `r[c]` is absent. */
    new_i_heap = malloc(sizeof *new_i_heap);
    pico8_list_init(new_i_heap);
    for (c = 0; i_src[c] != '\0'; c++) {
        char key[2] = {i_src[c], '\0'};
        int r_qe;
        pico8_fixed_t qe;

        if (pico8_r_lookup(key, &r_qe)) {
            qe = pico8_fixed_from_int(r_qe);
            pico8_list_add(new_i_heap, pico8_value_ref(pico8_jy_new(&qe)));
        } else {
            pico8_list_add(new_i_heap, pico8_value_ref(pico8_jy_new(NULL)));
        }
    }
    self->i = new_i_heap;

#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("hp");
#endif
    return self;
}

pico8_value_t pico8_hp_il(const pico8_hp_t *self) {
    if (self == NULL || !self->has_il) {
        return pico8_value_ref(NULL);
    }
    if (self->il == NULL) {
        return pico8_value_bool(0);
    }
    return pico8_value_string(self->il);
}

void pico8_hp_assert(const pico8_hp_t *self) {
    assert((self == NULL ||
            (pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT && self->header.cls == &g_hp_struct_class)) &&
           "hp: every gb entry, every planet's hn and every row's project is an hp; anything else would be read "
           "through the wrong layout");
    (void)self;
}

/* A root class: no parent, so it never self-registers into hv/cv/cg. */
void pico8_research_init(void) {
    pico8_research_shutdown();
    g_hp_up = 1;
}

void pico8_research_shutdown(void) {
    pico8_research_free_gb_instances();
    g_gb_up = 0;
    g_rr = NULL;
    g_hp_up = 0;
}

int pico8_hp_ready(void) {
    return g_hp_up;
}

const pico8_struct_class_t *pico8_hp_struct_class(void) {
    return &g_hp_struct_class;
}

const pico8_jb_row_t *pico8_hp_fk(void) {
    return pico8_hp_start.fk;
}

pico8_hp_t *pico8_hp_new(const pico8_hp_keys_t *keys) {
    return pico8_hp_struct_new(keys);
}

void pico8_research_build_gb(void) {
    int i;

    pico8_research_free_gb_instances();
    g_gb_up = 1;
    g_rr = NULL;

    for (i = 0; i < PICO8_FD_COUNT; i++) {
        const pico8_fd_entry_t *e = &pico8_fd_table[i];

        if (e->header != NULL) {
            g_rr = e->header;
        } else {
            pico8_hp_keys_t keys = {0};
            pico8_hp_t *inst;

            keys.ri = e;
            keys.rr = g_rr;
            inst = pico8_hp_new(&keys);
            g_gb[g_gb_count++] = inst;
        }
    }
}

int pico8_gb_count(void) {
    return g_gb_count;
}

pico8_hp_t *pico8_gb_at(int k) {
    if (k < 1 || k > g_gb_count) {
        return NULL;
    }
    return g_gb[k - 1];
}

pico8_hp_t *pico8_gb_find(const char *px) {
    int i;

    for (i = 0; px != NULL && i < g_gb_px_count; i++) {
        if (strcmp(g_gb_px[i].px, px) == 0) {
            return g_gb_px[i].hp;
        }
    }
    return NULL;
}

const pico8_hc_entry_t *pico8_hp_po(const pico8_hp_t *self) {
    pico8_value_t il;
    const pico8_hc_entry_t *qp = NULL;

    pico8_hp_assert(self);
    if (self == NULL) {
        return NULL;
    }
    if (self->ja != NULL) {
        qp = pico8_hc_lookup(self->ja);
    }

    il = pico8_hp_il(self);
    if (pico8_research_truthy(il) && pico8_oj(il) && qp != NULL && qp->mc != NULL) {
        const pico8_hc_entry_t *hc_mc = pico8_hc_lookup(qp->mc);

        if (hc_mc != NULL) {
            return hc_mc;
        }
    }
    return qp;
}

pico8_fixed_t pico8_hp_od(const pico8_hp_t *self) {
    pico8_fixed_t od = pico8_fixed_from_int(0);
    pico8_value_t il;
    int has_ii;

    pico8_hp_assert(self);
    il = pico8_hp_il(self);
    if (self != NULL && self->has_fj) {
        od = self->fj;
    }
    if (self != NULL && pico8_research_truthy(il) && pico8_oj(il)) {
        od = pico8_fixed_mul(od, PICO8_FIXED_LIT(0x00018000, 1.5));
    }
    has_ii = self != NULL && self->has_ii;
    if (!has_ii && pico8_oj(pico8_value_string("nanomaterials"))) {
        od = pico8_fixed_mul(od, PICO8_FIXED_LIT(0x0000d999, 0.85));
    }
    return pico8_fixed_flr(od);
}
