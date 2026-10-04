#include "game/research.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "game/tech_gate.h"
#include "engine/game_zone.h"

static int g_research_project_up = 0;

static research_project_t *g_projects[PROJECT_RECORDS_COUNT];
static int g_projects_count = 0;
/* The name is borrowed, never copied: it must outlive the project. */
static struct {
    const char *name;
    research_project_t *project;
} g_projects_by_name[PROJECT_RECORDS_COUNT];
static int g_projects_by_name_count = 0;
static int g_projects_up = 0;
static const char *g_required_tech = NULL;

static void forget_projects(void) {
    g_projects_count = 0;
    g_projects_by_name_count = 0;
}

static void projects_register(research_project_t *hp) {
    int i;

    for (i = 0; i < g_projects_by_name_count; i++) {
        if (strcmp(g_projects_by_name[i].name, hp->name) == 0) {
            g_projects_by_name[i].project = hp;
            return;
        }
    }
    assert(g_projects_by_name_count < PROJECT_RECORDS_COUNT && "projects_register: every name comes from the project records, which have fewer names than records");
    g_projects_by_name[g_projects_by_name_count].name = hp->name;
    g_projects_by_name[g_projects_by_name_count].project = hp;
    g_projects_by_name_count++;
}

static const entity_class_t research_project_class_def = {0};

static const char *research_project_string(const char *s) {
    assert(s != NULL && "research_project_string: a position that is not a string does not fit the member's type "
                        "-- stop, do not widen the member");
    return s;
}

static p8_fixed_t resource_code_of(const char *s) {
    int qe = 0;
    int found;

    assert(s != NULL && "resource_code_of: a position that is not a number does not fit the member's type "
                        "-- stop, do not widen the member");
    found = resource_code_lookup(s, &qe);
    assert(found && "resource_code_of: a routed position missing from the resource codes would stay a string in a "
                    "fixed member -- stop, do not widen the member");
    (void)found;
    return p8_fixed_from_int(qe);
}

static void research_project_positions(research_project_t *self, const project_records_entry_t *ri, const char **input_codes) {
    int n = ri->npos;

    assert(ri->header == NULL && "research_project_positions: a header entry sets the required_tech of the records after it, it is not a project");
    assert(n <= 8 && "research_project_positions: a record has eight positions, name to byproduct");
    if (n >= 1) {
        self->name = research_project_string(ri->name);
    }
    if (n >= 2) {
        self->planet_codes = research_project_string(ri->planet_codes);
    }
    if (n >= 3) {
        self->base_cost = p8_fixed_from_int(ri->base_cost);
        self->has_base_cost = 1;
    }
    if (n >= 4) {
        self->production_key = research_project_string(ri->production_key);
    }
    if (n >= 5) {
        *input_codes = research_project_string(ri->inputs);
    }
    if (n >= 6) {
        self->output = resource_code_of(ri->output);
        self->has_output = 1;
    }
    if (n >= 7) {
        self->upgrade_tech = ri->upgrade_tech_false ? NULL : ri->upgrade_tech;
        self->has_upgrade_tech = 1;
    }
    if (n >= 8) {
        self->byproduct = resource_code_of(ri->byproduct);
        self->has_byproduct = 1;
    }
}

resource_t need_resource(const need_t *i) {
    return i->row.has_resource ? (resource_t){.kind = RESOURCE_FIXED, .number = i->row.resource} : (resource_t){.kind = RESOURCE_NIL};
}

need_t *need_new(const allocator_t *zone, const p8_fixed_t *qe) {
    need_t *i = (need_t *)allocator_alloc(zone, sizeof *i);

    memset(i, 0, sizeof *i);
    if (qe != NULL) {
        i->row.has_resource = 1;
        i->row.resource = *qe;
    }
    return i;
}

static research_project_t *project_create(const research_project_keys_t *keys) {
    const allocator_t *load_zone = game_zone_load_zone();
    research_project_t *self = (research_project_t *)allocator_alloc(load_zone, sizeof *self);
    pointer_list_t *new_i;
    const char *input_codes = "";
    size_t input_count;

    entity_init_header(&self->header, &research_project_class_def);
    self->name = NULL;
    self->planet_codes = NULL;
    self->base_cost = 0;
    self->has_base_cost = 0;
    self->production_key = NULL;
    self->inputs = NULL;
    self->output = 0;
    self->has_output = 0;
    self->upgrade_tech = NULL;
    self->has_upgrade_tech = 0;
    self->byproduct = 0;
    self->has_byproduct = 0;
    self->required_tech = NULL;
    self->icon_sprite = 0;
    self->has_icon_sprite = 0;
    self->class_name = NULL;
    self->next_type = NULL;
    self->layout_row = NULL;
    self->archetype = &research_project_archetype;

    if (keys != NULL && keys->record != NULL) {
        const project_records_entry_t *ri = keys->record;

        research_project_positions(self, ri, &input_codes);
        if (ri->has_icon_sprite) {
            self->icon_sprite = p8_fixed_from_int(ri->icon_sprite);
            self->has_icon_sprite = 1;
        }
        self->class_name = ri->class_name;
        self->next_type = ri->next_type;
    }
    if (keys != NULL) {
        self->required_tech = keys->required_tech;
    }

    if (self->name != NULL) {
        assert(g_projects_up && "project_create: a named project registers itself, so research_build_projects() runs first");
        projects_register(self);
    }

    if (self->has_icon_sprite) {
        assert(self->name != NULL && "project_create: the row {name, 32, {icon_sprite, 8}} has no hole; a project with an icon_sprite and no name "
                                   "does not fit the row -- stop, do not widen the member");
        self->layout_row_inner_items[0] = (data_item_t){.is_fixed = 1, .fixed = self->icon_sprite};
        self->layout_row_inner_items[1] = (data_item_t){.number = 8};
        self->layout_row_inner = (data_row_t){.count = 2, .items = self->layout_row_inner_items};
        self->layout_row_own_items[0] = (data_item_t){.text = self->name};
        self->layout_row_own_items[1] = (data_item_t){.number = 32};
        self->layout_row_own_items[2] = (data_item_t){.row = &self->layout_row_inner};
        self->layout_row_own = (data_row_t){.count = 3, .items = self->layout_row_own_items};
        self->layout_row = &self->layout_row_own;
    }

    /* A character that is no resource code still makes a need, with no resource. */
    new_i = (pointer_list_t *)allocator_alloc(load_zone, sizeof *new_i);
    if (input_codes[0] == '\0') {
        pointer_list_init(new_i, load_zone);
    } else {
        pointer_list_init_reserve(new_i, load_zone, (int)strlen(input_codes));
    }
    for (input_count = 0; input_codes[input_count] != '\0'; input_count++) {
        char key[2] = {input_codes[input_count], '\0'};
        int r_resource;
        p8_fixed_t qe;

        if (resource_code_lookup(key, &r_resource)) {
            qe = p8_fixed_from_int(r_resource);
            pointer_list_add(new_i, need_new(load_zone, &qe));
        } else {
            pointer_list_add(new_i, need_new(load_zone, NULL));
        }
    }
    self->inputs = new_i;

    return self;
}

const char *research_project_upgrade_tech(const research_project_t *self) {
    if (self == NULL || !self->has_upgrade_tech) {
        return NULL;
    }
    return self->upgrade_tech;
}

void research_project_assert(const research_project_t *self) {
    assert((self == NULL ||
            self->header.class == &research_project_class_def) &&
           "research_project_assert: every listed project, every planet's project and every row's project is a research project; anything else would be read "
           "through the wrong layout");
    (void)self;
}

void research_init(void) {
    research_shutdown();
    g_research_project_up = 1;
}

void research_shutdown(void) {
    forget_projects();
    g_projects_up = 0;
    g_required_tech = NULL;
    g_research_project_up = 0;
}

int research_project_ready(void) {
    return g_research_project_up;
}

const entity_class_t *research_project_class(void) {
    return &research_project_class_def;
}

const data_row_t *research_project_row_tail(void) {
    return research_project_archetype.row_tail;
}

research_project_t *research_project_new(const research_project_keys_t *keys) {
    return project_create(keys);
}

void research_build_projects(void) {
    int i;

    forget_projects();
    g_projects_up = 1;
    g_required_tech = NULL;

    for (i = 0; i < PROJECT_RECORDS_COUNT; i++) {
        const project_records_entry_t *e = &project_records_table[i];

        if (e->header != NULL) {
            g_required_tech = e->header;
        } else {
            research_project_keys_t keys = {0};
            research_project_t *inst;

            keys.record = e;
            keys.required_tech = g_required_tech;
            inst = research_project_new(&keys);
            g_projects[g_projects_count++] = inst;
        }
    }
}

int research_projects_count(void) {
    return g_projects_count;
}

research_project_t *research_projects_at(int k) {
    if (k < 1 || k > g_projects_count) {
        return NULL;
    }
    return g_projects[k - 1];
}

research_project_t *research_projects_find(const char *px) {
    int i;

    for (i = 0; px != NULL && i < g_projects_by_name_count; i++) {
        if (strcmp(g_projects_by_name[i].name, px) == 0) {
            return g_projects_by_name[i].project;
        }
    }
    return NULL;
}

const production_tables_entry_t *research_project_production(const research_project_t *self) {
    const char *il;
    const production_tables_entry_t *qp = NULL;

    research_project_assert(self);
    if (self == NULL) {
        return NULL;
    }
    if (self->production_key != NULL) {
        qp = production_tables_lookup(self->production_key);
    }

    il = research_project_upgrade_tech(self);
    if (il != NULL && tech_gate_is_unlocked(il) && qp != NULL && qp->upgraded_entry != NULL) {
        const production_tables_entry_t *production_tables_upgraded_entry = production_tables_lookup(qp->upgraded_entry);

        if (production_tables_upgraded_entry != NULL) {
            return production_tables_upgraded_entry;
        }
    }
    return qp;
}

p8_fixed_t research_project_cost(const research_project_t *self) {
    p8_fixed_t od = p8_fixed_from_int(0);
    const char *il;
    int has_icon_sprite;

    research_project_assert(self);
    il = research_project_upgrade_tech(self);
    if (self != NULL && self->has_base_cost) {
        od = self->base_cost;
    }
    if (self != NULL && il != NULL && tech_gate_is_unlocked(il)) {
        od = p8_fixed_mul(od, P8_FIXED_LIT(0x00018000, 1.5));
    }
    has_icon_sprite = self != NULL && self->has_icon_sprite;
    if (!has_icon_sprite && tech_gate_is_unlocked("nanomaterials")) {
        od = p8_fixed_mul(od, P8_FIXED_LIT(0x0000d999, 0.85));
    }
    return p8_fixed_flr(od);
}
