#include "game/research.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "game/tech_gate.h"
#include "engine/game_zone.h"

static int g_hp_up = 0;

/* `gb[k]`, 1-based rank k: the projects `build_gb` made, in the load
 * sub-zone. */
static pico8_hp_t *g_gb[PROJECT_RECORDS_COUNT];
static int g_gb_count = 0;
/* `gb[px]`: the last project registered under each `px` (several share
 * one), built by `build_gb` or not. 0 until `build_gb`. `px` is the hp's own
 * pointer, borrowed, never copied: it must outlive the hp (every caller
 * passes a string of `pico8_fd_table`). */
static struct {
    const char *px;
    pico8_hp_t *hp;
} g_gb_px[PROJECT_RECORDS_COUNT];
static int g_gb_px_count = 0;
static int g_gb_up = 0;
/* Borrowed from `pico8_fd_table`, never copied or freed here. */
static const char *g_rr = NULL;

static void pico8_research_forget_gb(void) {
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
    assert(g_gb_px_count < PROJECT_RECORDS_COUNT && "gb[px]: every px comes from fd, which has fewer names than records");
    g_gb_px[g_gb_px_count].px = hp->px;
    g_gb_px[g_gb_px_count].hp = hp;
    g_gb_px_count++;
}

static const entity_class_t g_hp_struct_class = {0};

static const char *hp_string(const char *s) {
    assert(s != NULL && "hp.px/lr/ja/i: a position that is not a string does not fit the member's type "
                        "-- stop, do not widen the member");
    return s;
}

/* `r[s]`, for the two positions `gp` names: `o` and `dm`. */
static p8_fixed_t hp_r(const char *s) {
    int qe = 0;
    int found;

    assert(s != NULL && "hp.fj/o/dm: a position that is not a number does not fit the member's type "
                        "-- stop, do not widen the member");
    found = resource_code_lookup(s, &qe);
    assert(found && "hp: a value that gp maps through r is missing from r; it would stay a string in a "
                    "fixed member -- stop, do not widen the member");
    (void)found;
    return p8_fixed_from_int(qe);
}

/* `ho(gb, mr, gp)` on one record of `fd`: positions 1..npos are `px`, `lr`,
 * `fj`, `ja`, `i`, `o`, `il`, `dm`. `i` gets the raw string that `hp:pq`
 * turns into its list afterwards. */
static void hp_positions(pico8_hp_t *self, const project_records_entry_t *ri, const char **i_src) {
    int n = ri->npos;

    assert(ri->header == NULL && "hp: a header entry of fd is the rr of the records after it, not a project");
    assert(n <= 8 && "hp: fd has eight positions, px to dm");
    if (n >= 1) {
        self->px = hp_string(ri->name);
    }
    if (n >= 2) {
        self->lr = hp_string(ri->planet_codes);
    }
    if (n >= 3) {
        self->fj = p8_fixed_from_int(ri->base_cost);
        self->has_fj = 1;
    }
    if (n >= 4) {
        self->ja = hp_string(ri->production_key);
    }
    if (n >= 5) {
        *i_src = hp_string(ri->inputs);
    }
    if (n >= 6) {
        self->o = hp_r(ri->output);
        self->has_o = 1;
    }
    if (n >= 7) {
        self->il = ri->upgrade_tech_false ? NULL : ri->upgrade_tech;
        self->has_il = 1;
    }
    if (n >= 8) {
        self->dm = hp_r(ri->byproduct);
        self->has_dm = 1;
    }
}

pico8_qe_t pico8_jy_qe(const pico8_jy_t *i) {
    return i->row.has_resource ? (pico8_qe_t){.kind = PICO8_QE_FIXED, .n = i->row.resource} : (pico8_qe_t){.kind = PICO8_QE_NIL};
}

pico8_jy_t *pico8_jy_new(const allocator_t *zone, const p8_fixed_t *qe) {
    pico8_jy_t *i = (pico8_jy_t *)allocator_alloc(zone, sizeof *i);

    memset(i, 0, sizeof *i);
    if (qe != NULL) {
        i->row.has_resource = 1;
        i->row.resource = *qe;
    }
    return i;
}

/* `i` must be rewritten after `ho`, which leaves the raw string in it; the
 * real `gb[px]` must hold the struct. */
static pico8_hp_t *pico8_hp_struct_new(const pico8_hp_keys_t *keys) {
    const allocator_t *chargement = game_zone_load_zone();
    pico8_hp_t *self = (pico8_hp_t *)allocator_alloc(chargement, sizeof *self);
    pointer_list_t *new_i;
    const char *i_src = "";
    size_t c;

    entity_init_header(&self->header, &g_hp_struct_class);
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
    self->def = &research_project_archetype;

    if (keys != NULL && keys->ri != NULL) {
        const project_records_entry_t *ri = keys->ri;

        hp_positions(self, ri, &i_src);
        if (ri->has_icon_sprite) {
            self->ii = p8_fixed_from_int(ri->icon_sprite);
            self->has_ii = 1;
        }
        self->cw = ri->class_name;
        self->go = ri->next_type;
        if (ri->has_play_sfx) {
            self->rn = p8_fixed_from_int(ri->play_sfx);
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
        self->lq_inner_items[0] = (data_item_t){.is_fixed = 1, .fixed = self->ii};
        self->lq_inner_items[1] = (data_item_t){.number = 8};
        self->lq_inner = (data_row_t){.count = 2, .items = self->lq_inner_items};
        self->lq_own_items[0] = (data_item_t){.text = self->px};
        self->lq_own_items[1] = (data_item_t){.number = 32};
        self->lq_own_items[2] = (data_item_t){.row = &self->lq_inner};
        self->lq_own = (data_row_t){.count = 3, .items = self->lq_own_items};
        self->lq = &self->lq_own;
    }

    /* A want for every character, never skipping one: the Lua table
     * constructor is non-nil even when `r[c]` is absent. */
    new_i = (pointer_list_t *)allocator_alloc(chargement, sizeof *new_i);
    if (i_src[0] == '\0') {
        pointer_list_init(new_i, chargement);
    } else {
        pointer_list_init_reserve(new_i, chargement, (int)strlen(i_src));
    }
    for (c = 0; i_src[c] != '\0'; c++) {
        char key[2] = {i_src[c], '\0'};
        int r_qe;
        p8_fixed_t qe;

        if (resource_code_lookup(key, &r_qe)) {
            qe = p8_fixed_from_int(r_qe);
            pointer_list_add(new_i, pico8_jy_new(chargement, &qe));
        } else {
            pointer_list_add(new_i, pico8_jy_new(chargement, NULL));
        }
    }
    self->i = new_i;

#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("research_project");
#endif
    return self;
}

const char *pico8_hp_il(const pico8_hp_t *self) {
    if (self == NULL || !self->has_il) {
        return NULL;
    }
    return self->il;
}

void pico8_hp_assert(const pico8_hp_t *self) {
    assert((self == NULL ||
            self->header.class == &g_hp_struct_class) &&
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
    pico8_research_forget_gb();
    g_gb_up = 0;
    g_rr = NULL;
    g_hp_up = 0;
}

int pico8_hp_ready(void) {
    return g_hp_up;
}

const entity_class_t *pico8_hp_struct_class(void) {
    return &g_hp_struct_class;
}

const data_row_t *pico8_hp_fk(void) {
    return research_project_archetype.row_tail;
}

pico8_hp_t *pico8_hp_new(const pico8_hp_keys_t *keys) {
    return pico8_hp_struct_new(keys);
}

void pico8_research_build_gb(void) {
    int i;

    pico8_research_forget_gb();
    g_gb_up = 1;
    g_rr = NULL;

    for (i = 0; i < PROJECT_RECORDS_COUNT; i++) {
        const project_records_entry_t *e = &project_records_table[i];

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

const production_tables_entry_t *pico8_hp_po(const pico8_hp_t *self) {
    const char *il;
    const production_tables_entry_t *qp = NULL;

    pico8_hp_assert(self);
    if (self == NULL) {
        return NULL;
    }
    if (self->ja != NULL) {
        qp = production_tables_lookup(self->ja);
    }

    il = pico8_hp_il(self);
    if (il != NULL && pico8_oj(il) && qp != NULL && qp->upgraded_entry != NULL) {
        const production_tables_entry_t *hc_mc = production_tables_lookup(qp->upgraded_entry);

        if (hc_mc != NULL) {
            return hc_mc;
        }
    }
    return qp;
}

p8_fixed_t pico8_hp_od(const pico8_hp_t *self) {
    p8_fixed_t od = p8_fixed_from_int(0);
    const char *il;
    int has_ii;

    pico8_hp_assert(self);
    il = pico8_hp_il(self);
    if (self != NULL && self->has_fj) {
        od = self->fj;
    }
    if (self != NULL && il != NULL && pico8_oj(il)) {
        od = p8_fixed_mul(od, P8_FIXED_LIT(0x00018000, 1.5));
    }
    has_ii = self != NULL && self->has_ii;
    if (!has_ii && pico8_oj("nanomaterials")) {
        od = p8_fixed_mul(od, P8_FIXED_LIT(0x0000d999, 0.85));
    }
    return p8_fixed_flr(od);
}
