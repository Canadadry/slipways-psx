#include "game/research_display.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "game/layout.h"
#include "game/map_tables.h"
#include "game/planet.h"
#include "game/research.h"
#include "p8/str.h"
#include "game/tech_gate.h"
#include "engine/frame_zone.h"

/* In the frame zone. */
static char *research_project_tooltip_join(const research_project_t *self) {
    const pointer_list_t *i_list;
    size_t cap = 16;
    size_t len = 0;
    char *out = (char *)allocator_alloc(frame_zone(), cap);
    int first = 1;
    pointer_list_iter_t iter;
    void *entry;

    out[0] = '\0';
    if (self->inputs == NULL) {
        return out;
    }
    i_list = self->inputs;
    iter = pointer_list_all(i_list);
    while (pointer_list_iter_next(&iter, &entry)) {
        resource_t qe = {.kind = RESOURCE_NIL};
        char *num_str;
        const char *resource_names_name;
        const char *piece = "";
        size_t piece_len;
        size_t add_len;

        if (entry != NULL) {
            qe = need_resource(entry);
        }
        num_str = qe.kind == RESOURCE_FIXED ? p8_tostr_fixed(frame_zone(), qe.number) : NULL;
        resource_names_name = num_str != NULL ? resource_names_lookup(num_str) : NULL;
        if (resource_names_name != NULL) {
            piece = resource_names_name;
        }

        piece_len = strlen(piece);
        add_len = (first ? 0 : 1) + piece_len;
        if (len + add_len + 1 > cap) {
            size_t new_cap = (len + add_len + 1) * 2;

            out = (char *)allocator_grow(frame_zone(), (unsigned char *)out, cap, new_cap);
            cap = new_cap;
        }
        if (!first) {
            out[len++] = ',';
        }
        memcpy(out + len, piece, piece_len + 1);
        len += piece_len;
        first = 0;
    }
    return out;
}

/* The `jb` of `hp:jb`, its one line and their items, in one block. */
typedef struct {
    data_row_t outer;
    data_item_t outer_item;
    data_row_t inner;
    data_item_t inner_items[3];
} research_project_tooltip_t;

const data_row_t *research_project_tooltip(const research_project_t *self) {
    research_project_tooltip_t *jb;

    research_project_assert(self);
    if (self == NULL) {
        return NULL;
    }
    if (self->has_icon_sprite) {
        return NULL;
    }

    jb = (research_project_tooltip_t *)allocator_alloc(frame_zone(), sizeof *jb);
    memset(jb, 0, sizeof *jb);
    jb->inner_items[0].text = research_project_tooltip_join(self);
    jb->inner_items[1].number = 13;
    jb->inner.count = 2;
    jb->inner.items = jb->inner_items;

    if (self->has_output) {
        char *o_str = p8_tostr_fixed(frame_zone(), self->output);
        const char *resource_names_o = resource_names_lookup(o_str);

        if (resource_names_o != NULL) {
            jb->inner_items[2].text = resource_names_o;
            jb->inner.count = 3;
        }
    }

    jb->outer_item.row = &jb->inner;
    jb->outer.count = 1;
    jb->outer.items = &jb->outer_item;
    return &jb->outer;
}

/* A tuple of `lq:mn` and its items, in one block of the frame zone. */
typedef struct {
    data_row_t row;
    data_item_t items[4];
} planet_resource_row_tuple_t;

static planet_resource_row_tuple_t *planet_resource_row_tuple(int count) {
    planet_resource_row_tuple_t *t = (planet_resource_row_tuple_t *)allocator_alloc(frame_zone(), sizeof *t);

    memset(t, 0, sizeof *t);
    t->row.count = count;
    t->row.items = t->items;
    return t;
}

static data_item_t planet_resource_row_fixed(p8_fixed_t x) {
    return (data_item_t){.is_fixed = 1, .fixed = x};
}

layout_row_t *research_display_planet_resource_row(const allocator_t *zone, const pointer_list_t *inputs, const p8_fixed_t *output,
                        p8_fixed_t amount, p8_fixed_t used, p8_fixed_t spacing) {
    layout_row_cell_t *cells;
    int l_count = 0;
    int l_cap;
    layout_row_keys_t keys = {0};
    int crowded;
    int is_count = (inputs != NULL) ? inputs->count : 0;
    p8_fixed_t outputs[2];
    int outputs_count;
    int wide;
    layout_row_t *result;

    if (output == NULL) {
        return NULL;
    }

    crowded = (is_count >= 3) ? 1 : 0;

    {
        int r_w;
        int o_is_r_w = resource_code_lookup("w", &r_w) && *output == p8_fixed_from_int(r_w);

        if (o_is_r_w) {
            outputs[0] = p8_fixed_from_int(25);
            outputs[1] = p8_fixed_from_int(23);
            outputs_count = 2;
        } else {
            outputs[0] = *output;
            outputs_count = 1;
        }
    }

    wide = p8_fixed_mul(amount, p8_fixed_from_int(outputs_count)) >= p8_fixed_from_int(3);

    {
        p8_fixed_t amount_drawn = amount > p8_fixed_from_int(5) ? p8_fixed_from_int(1) : amount;
        p8_fixed_t loop_bound = (amount_drawn > p8_fixed_from_int(1)) ? amount_drawn : p8_fixed_from_int(1);

        l_cap = is_count + (is_count > 0 ? 2 : 0) + (amount > p8_fixed_from_int(5) ? 2 : 0) +
                outputs_count * (p8_fixed_flr_int(loop_bound) + 1);
    }
    cells = (layout_row_cell_t *)allocator_alloc(frame_zone(), (size_t)l_cap * sizeof *cells);

    if (inputs != NULL) {
        pointer_list_iter_t it = pointer_list_all(inputs);
        void *entry;

        while (pointer_list_iter_next(&it, &entry)) {
            planet_resource_row_tuple_t *tuple = planet_resource_row_tuple(3);
            resource_t qe = {.kind = RESOURCE_NIL};
            int gc_zero = 0;

            if (entry != NULL) {
                const need_t *i = entry;

                qe = need_resource(i);
                gc_zero = i->has_satisfied && i->satisfied == 0;
            }
            tuple->items[0] = qe.kind == RESOURCE_FIXED ? planet_resource_row_fixed(qe.number)
                                                           : (data_item_t){.is_ref = 1};
            tuple->items[1] = planet_resource_row_fixed(p8_fixed_sub(p8_fixed_from_int(3), p8_fixed_from_int(crowded)));
            /* Lua's `i.gc~=0 and 0 or 7`: gc==0 -> 7, gc~=0 or absent -> 0.
             * 0 is truthy in Lua, so it is NOT "gc==0 -> 0". */
            tuple->items[2] = planet_resource_row_fixed(p8_fixed_from_int(gc_zero ? 7 : 0));
            cells[l_count++] = layout_row_cell_row(&tuple->row);
        }
    }

    if (is_count > 0) {
        planet_resource_row_tuple_t *t1 = planet_resource_row_tuple(2);
        planet_resource_row_tuple_t *t2 = planet_resource_row_tuple(2);
        int use_35 = (spacing == p8_fixed_from_int(1)) || wide;

        t1->items[0] = (data_item_t){.is_false = 1};
        t1->items[1] = planet_resource_row_fixed(p8_fixed_add(spacing, p8_fixed_from_int(crowded)));
        cells[l_count++] = layout_row_cell_row(&t1->row);

        if (use_35) {
            t2->items[0] = planet_resource_row_fixed(p8_fixed_from_int(35));
            t2->items[1] = planet_resource_row_fixed(p8_fixed_add(p8_fixed_from_int(2), spacing));
        } else {
            t2->items[0] = planet_resource_row_fixed(p8_fixed_from_int(32));
            t2->items[1] = planet_resource_row_fixed(p8_fixed_from_int(5));
        }
        cells[l_count++] = layout_row_cell_row(&t2->row);
    }

    if (amount > p8_fixed_from_int(5)) {
        char *amount_text = p8_tostr_fixed(frame_zone(), amount);
        static const data_item_t tail_row_items[] = {{.is_false = 1}, {.number = -4}};
        static const data_row_t tail_row = {.count = 2, .items = tail_row_items};

        cells[l_count++] = layout_row_cell_string(amount_text);
        cells[l_count++] = layout_row_cell_row(&tail_row);
        keys.has_color = 1;
        keys.color = p8_fixed_from_int(13);
        amount = p8_fixed_from_int(1);
    }

    {
        int output_index;

        for (output_index = 0; output_index < outputs_count; output_index++) {
            p8_fixed_t qq = outputs[output_index];
            p8_fixed_t loop_bound = (amount > p8_fixed_from_int(1)) ? amount : p8_fixed_from_int(1);
            int n = p8_fixed_flr_int(loop_bound);
            int i;

            for (i = 1; i <= n; i++) {
                planet_resource_row_tuple_t *t4 = planet_resource_row_tuple(4);
                p8_fixed_t i_fixed = p8_fixed_from_int(i);
                int within = (i_fixed <= amount) && (i_fixed > used);

                t4->items[0] = planet_resource_row_fixed(qq);
                t4->items[1] = planet_resource_row_fixed(p8_fixed_from_int(6));
                t4->items[2] = planet_resource_row_fixed(p8_fixed_from_int(within ? 0 : 7));
                t4->items[3] = planet_resource_row_fixed(p8_fixed_from_int(wide ? -4 : -3));
                cells[l_count++] = layout_row_cell_row(&t4->row);
            }

            cells[l_count++] = layout_row_cell_row(planet_spacer());
        }
    }

    assert(l_count == l_cap && "jo.ex: its row is sized at entry from #is, jq and ml; a positional more or less "
                               "is a miscount of that size");
    keys.items = cells;
    keys.count = l_count;
    result = layout_row_new(zone, &keys);
    return result;
}

layout_row_t *research_project_layout_row(research_project_t *self) {
    p8_fixed_t jq;
    const pointer_list_t *is = NULL;
    const production_tables_entry_t *po;
    layout_row_t *l;

    research_project_assert(self);
    if (self == NULL || !research_project_ready()) {
        return NULL;
    }

    if (self->inputs != NULL) {
        is = self->inputs;
    }

    po = research_project_production(self);
    jq = p8_fixed_from_int(1);
    if (po != NULL && po->count >= 1) {
        p8_fixed_t first_production = p8_fixed_from_int(po->values[0]);
        jq = (first_production > p8_fixed_from_int(1)) ? first_production : p8_fixed_from_int(1);
    }

    l = research_display_planet_resource_row(frame_zone(), is, self->has_output ? &self->output : NULL, jq, p8_fixed_from_int(0), p8_fixed_from_int(1));
    if (l == NULL) {
        return NULL;
    }

    layout_row_add(l, layout_row_cell_row(research_project_row_tail()));
    if (self->name != NULL) {
        layout_row_add(l, layout_row_cell_string(self->name));
    }

    return l;
}

/* `jo.nk.px` is one character wide by construction, so `k==jo.nk.px` is a
 * plain char comparison here -- no string identity involved. */
static int available_projects_predicate(const research_project_t *b, const entity_t *planet_instance) {
    const char *name_str = NULL;
    int mg = 0;

    research_project_assert(b);

    if (!tech_gate_is_unlocked(b->required_tech)) {
        return 0;
    }

    if (planet_instance != NULL) {
        const planet_t *jo;

        assert(planet_is_family(planet_instance) && "bj: jo:ex passes its own planet");
        jo = (const planet_t *)planet_instance;
        if (jo->planet_type != NULL) {
            name_str = jo->planet_type->name;
        }
    }

    if (name_str != NULL && name_str[0] != '\0' && name_str[1] == '\0' && b->planet_codes != NULL) {
        size_t n = strlen(b->planet_codes);
        size_t i;

        for (i = 0; i < n; i++) {
            if (b->planet_codes[i] == name_str[0]) {
                mg = 1;
                break;
            }
        }
    }
    return mg;
}

available_projects_kept_t research_display_available_projects(const entity_t *planet_instance) {
    available_projects_kept_t result;
    int k;

    assert(research_projects_count() <= PROJECT_RECORDS_COUNT && "gb holds one hp per entry of fd at most: #fd bounds bj");
    result.count = 0;
    for (k = 1; k <= research_projects_count(); k++) {
        research_project_t *b = research_projects_at(k);

        if (available_projects_predicate(b, planet_instance)) {
            result.items[result.count++] = b;
        }
    }
    return result;
}
