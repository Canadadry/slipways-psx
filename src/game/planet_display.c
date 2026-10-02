#include "game/planet_display.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "p8/audio.h"
#include "engine/collect.h"
#include "engine/dispatch_context.h"
#include "game/economy.h"
#include "engine/entity_geom.h"
#include "game/main_economy.h"
#include "game/game_data.h"
#include "game/hud.h"
#include "engine/entity.h"
#include "game/map_tables.h"
#include "game/message_box.h"
#include "game/planet.h"
#include "game/research.h"
#include "game/research_display.h"
#include "game/slipway.h"
#include "p8/str.h"
#include "game/tech_gate.h"
#include "game/ui_button.h"
#include "engine/frame_zone.h"
#include "game/ui_zone.h"

/* `kw` as `lq:mn` reads it, `{qe=kw.qe}` (`mu` is never read there), in
 * the frame zone. */
static const data_row_t *planet_production_tuple(const production_t *kw) {
    data_row_t *t = (data_row_t *)allocator_alloc(frame_zone(), sizeof *t);

    memset(t, 0, sizeof *t);
    t->has_resource = kw->has_resource;
    t->resource = kw->resource;
    return t;
}

void planet_refresh(planet_t *self) {
    const pointer_list_t *needs_list;
    const entity_list_t *by_list;
    const p8_fixed_t *qe = NULL;
    p8_fixed_t mu = p8_fixed_from_int(0);
    layout_row_t *layout_row_result;
    const allocator_t *slot;

    if (self == NULL) {
        return;
    }

    needs_list = self->needs;
    if (self->production != NULL) {
        qe = self->production->has_resource ? &self->production->resource : NULL;
        mu = self->production->amount;
    }
    by_list = self->outgoing_routes;

    slot = ui_zone_slot(SLOT_KIND_ROW);
    layout_row_result = research_display_planet_resource_row(slot, needs_list, qe, mu, p8_fixed_from_int(by_list != NULL ? by_list->count : 0),
                            p8_fixed_from_int(0));
    if (layout_row_result == NULL) {
        arena_release(slot);
        slot = NULL;
    }
    if (self->layout_row_slot != NULL) {
        arena_release(self->layout_row_slot);
    }
    self->layout_row = layout_row_result;
    self->layout_row_slot = slot;
}

void planet_release_row(entity_t *self) {
    planet_t *jo = (planet_t *)self;

    if (jo->layout_row_slot != NULL) {
        arena_release(jo->layout_row_slot);
    }
    jo->layout_row = NULL;
    jo->layout_row_slot = NULL;
}

void planet_release_method(entity_t *self) {
    planet_release_row(self);
    planet_grab_arena_released((const planet_t *)self);
}

typedef struct {
    pointer_list_t unmet_needs;
} planet_inspection_panel_ctx_t;

/* Filters on `gc == 0`, and also appends the element to `ji` in `ctx` once
 * per `gc`: the Lua predicate has that side effect. */
static int planet_inspection_panel_filter_step(void *e, void *ctx_ptr) {
    planet_inspection_panel_ctx_t *ctx = (planet_inspection_panel_ctx_t *)ctx_ptr;
    p8_fixed_t gc = p8_fixed_from_int(0);
    int count;
    int j;

    if (e != NULL) {
        const need_t *i = e;
        if (i->has_satisfied) {
            gc = i->satisfied;
        }
    }
    count = p8_fixed_flr_int(gc);
    for (j = 0; j < count; j++) {
        pointer_list_add(&ctx->unmet_needs, e);
    }
    return gc == p8_fixed_from_int(0);
}

/* The most items an icon row of `jo:gh`, or an `lq` row of `jo:fe`, holds. */
#define PLANET_ROW_CAP 8

/* The most rows `jo:gh` passes to `pz`: the icon, needs or wants more,
 * receives, makes or exports. */
#define PLANET_INSPECTION_PANEL_ROWS 4

/* The cart's makes or exports row: its head, the count, the tuple, then
 * the gap. */
#define PLANET_INSPECTION_PANEL_EXPORTS_CAP 4

/* Room for `n` positionals of a row of `jo:gh`, the gap's cell included. */
static layout_row_cell_t *planet_inspection_panel_row_items(int n) {
    return (layout_row_cell_t *)allocator_alloc(frame_zone(), (size_t)n * sizeof(layout_row_cell_t));
}

/* `s` followed by `$`, in the frame zone. */
static char *planet_dollar(const char *s) {
    return p8_concat2(frame_zone(), s, "$");
}

/* A planet's `pz` frame: `ep` and `ra` nil when NULL, `ca` and `gq` always. */
static open_box_frame_t planet_open_box_frame(const planet_t *self) {
    open_box_frame_t frame = {0};

    frame.description = self->description;
    frame.has_back_color = 1;
    frame.back_color = self->archetype->back_color;
    frame.row_step = &self->archetype->row_step;
    frame.pos = self->pos;
    return frame;
}

void planet_inspection_panel(const planet_t *self) {
    button_keys_t buttons[PLANET_INSPECTION_PANEL_ROWS];
    layout_row_keys_t icon;
    layout_row_cell_t icon_items[PLANET_ROW_CAP];
    layout_row_keys_t h_keys;
    layout_row_cell_t h_item;
    int row_count = 0;
    const data_row_t *icon_row = NULL;
    layout_row_keys_t row_keys[PLANET_INSPECTION_PANEL_ROWS];
    layout_row_cell_t *row_items[PLANET_INSPECTION_PANEL_ROWS];
    int row_cap[PLANET_INSPECTION_PANEL_ROWS];
    planet_inspection_panel_ctx_t filter_context;
    const pointer_list_t *needs;
    pointer_list_t unmet_needs;
    open_box_frame_t frame;
    int index;

    if (self == NULL) {
        return;
    }

    memset(buttons, 0, sizeof buttons);
    memset(row_keys, 0, sizeof row_keys);
    if ((self->header.class->lineage & LINEAGE(LINEAGE_STRUCTURE)) && self->archetype->name != NULL) {
        icon_row = self->archetype->name;
    } else if (self->has_development_tier) {
        int mi = p8_fixed_flr_int(self->development_tier);

        if (mi >= 1 && mi <= PLANET_MOOD_ROWS_COUNT) {
            icon_row = &planet_mood_rows()[mi - 1];
        }
    }
    if (icon_row != NULL) {
        icon = layout_row_keys_of_row(icon_row, icon_items, PLANET_ROW_CAP);
        buttons[0].layout_row_keys = &icon;
    }
    buttons[0].has_side_tooltip = 1;
    if (self->has_net_income) {
        int discard_color = 0;

        h_item = layout_row_cell_string(planet_dollar(collect_format_signed(frame_zone(), self->net_income, 0, NULL, &discard_color)));
        memset(&h_keys, 0, sizeof h_keys);
        h_keys.items = &h_item;
        h_keys.count = 1;
        h_keys.has_background = 1;
        h_keys.background_is_bool = self->net_income < p8_fixed_from_int(0);
        h_keys.background = self->net_income >= p8_fixed_from_int(0) ? p8_fixed_from_int(1) : 0;
        buttons[0].side_tooltip_row = &h_keys;
    }
    row_count = 1;

    pointer_list_init(&filter_context.unmet_needs, frame_zone());

    needs = self->needs;
    unmet_needs = collect_filter_pointer_list(frame_zone(), needs, planet_inspection_panel_filter_step, &filter_context);

    if (unmet_needs.count > 0) {
        row_cap[row_count] = 1 + unmet_needs.count + 1;
        row_items[row_count] = planet_inspection_panel_row_items(row_cap[row_count]);
        row_items[row_count][row_keys[row_count].count++] = layout_row_cell_string("needs:");
        for (index = 0; index < unmet_needs.count; index++) {
            row_items[row_count][row_keys[row_count].count++] = layout_row_cell_row(unmet_needs.items[index]);
        }
        row_count++;
    } else {
        p8_fixed_t development_tier_fixed = self->has_development_tier ? self->development_tier : p8_fixed_from_int(0);
        const production_t *kw = self->production;
        int r_w;
        int resource_is_r_w;

        resource_is_r_w = kw != NULL && kw->has_resource && resource_code_lookup("w", &r_w) && kw->resource == p8_fixed_from_int(r_w);

        if (development_tier_fixed < p8_fixed_from_int(4) && needs != NULL && needs->count > 0 && !resource_is_r_w) {
            int last_only = (self->header.class->lineage & LINEAGE(LINEAGE_STRUCTURE)) && self->archetype->is_structure;

            row_cap[row_count] = 1 + (last_only ? 1 : needs->count) + 1;
            row_items[row_count] = planet_inspection_panel_row_items(row_cap[row_count]);
            row_items[row_count][row_keys[row_count].count++] = layout_row_cell_string("wants more:");
            if (last_only) {
                row_items[row_count][row_keys[row_count].count++] = layout_row_cell_row(needs->items[needs->count - 1]);
            } else {
                for (index = 0; index < needs->count; index++) {
                    row_items[row_count][row_keys[row_count].count++] = layout_row_cell_row(needs->items[index]);
                }
            }
            row_count++;
        }
    }

    /* A `ji` the predicate appended nothing to leaves no row at all, not a
     * row holding false: Lua's `add(t,nil)` is a no-op. */
    if (filter_context.unmet_needs.count >= 1) {
        row_cap[row_count] = 1 + filter_context.unmet_needs.count + 1;
        row_items[row_count] = planet_inspection_panel_row_items(row_cap[row_count]);
        row_items[row_count][row_keys[row_count].count++] = layout_row_cell_string("receives:");
        for (index = 0; index < filter_context.unmet_needs.count; index++) {
            row_items[row_count][row_keys[row_count].count++] = layout_row_cell_row(filter_context.unmet_needs.items[index]);
        }
        row_count++;
    }

    {
        const production_t *kw = self->production;
        p8_fixed_t mu = kw != NULL ? kw->amount : p8_fixed_from_int(0);

        if (mu > p8_fixed_from_int(0)) {
            int makes = kw->has_resource && int_seq_has_fixed(planet_made_resources(), kw->resource);

            row_cap[row_count] = PLANET_INSPECTION_PANEL_EXPORTS_CAP;
            row_items[row_count] = planet_inspection_panel_row_items(row_cap[row_count]);

            if (makes) {
                char *amount_str = p8_tostr_fixed(frame_zone(), mu);
                row_items[row_count][row_keys[row_count].count++] = layout_row_cell_string("makes:");
                row_items[row_count][row_keys[row_count].count++] = layout_row_cell_string(amount_str);
            } else {
                const entity_list_t *by = self->outgoing_routes;
                int by_count = (by != NULL) ? by->count : 0;
                char *count_str = p8_tostr_fixed(frame_zone(), p8_fixed_from_int(by_count));
                char *amount_str = p8_tostr_fixed(frame_zone(), mu);
                char *combined = p8_concat2(frame_zone(), p8_concat2(frame_zone(), count_str, "/"), amount_str);
                row_items[row_count][row_keys[row_count].count++] = layout_row_cell_string("exports:");
                row_items[row_count][row_keys[row_count].count++] = layout_row_cell_string(combined);
            }
            row_items[row_count][row_keys[row_count].count++] = layout_row_cell_row(planet_production_tuple(kw));
            row_count++;
        }
    }

    /* From the second row on: the first row's `lq` is a single icon, not a
     * list. */
    for (index = 1; index < row_count; index++) {
        static const data_item_t gap_items[] = {{.is_false = 1}, {.number = 1}};
        static const data_row_t gap = {.count = 2, .items = gap_items};

        assert(row_keys[index].count < row_cap[index] && "planet_inspection_panel: every row after the first keeps its last cell for the gap");
        row_items[index][row_keys[index].count++] = layout_row_cell_row(&gap);
        row_keys[index].items = row_items[index];
        buttons[index].layout_row_keys = &row_keys[index];
    }

    frame = planet_open_box_frame(self);
    message_box_open_box(&frame, buttons, row_count);
}

down_t planet_accept_drop(const planet_t *self, const entity_t *s) {
    const char *state_val;
    down_t out = {0, NULL};

    if (self == NULL) {
        return out;
    }
    state_val = self->header.state;
    if (state_val == NULL || strcmp(state_val, "settled") != 0) {
        return out;
    }
    /* Only a held `jc` carries `qu`: a held `bg`, `mz` or planet reads it absent. */
    if (s != NULL && s->class == slipway_class() && ((const slipway_t *)s)->start == &self->header) {
        planet_inspection_panel(self);
        return out;
    }
    out.stop = 1;
    return out;
}

/* C has no closures: the three values the Lua `my` callback captures ride
 * on the row as `_jo_fe_*` fields, which `pz()`'s own `js(b)` copies
 * verbatim onto the button this bridge is later dispatched on. */
static void planet_projects_panel_on_click_method(button_t *self) {
    planet_t *pt;
    research_project_t *b;
    p8_fixed_t od = p8_fixed_from_int(0);

    if (self == NULL) {
        return;
    }
    pt = self->project_planet;
    b = self->project;
    if (self->has_project_cost) {
        od = self->project_cost;
    }

    assert(pt != NULL && "the projects panel button lost its captured planet");
    if (economy_spend(main_economy_typed(), od)) {
        planet_apply_project(pt, b);
    }
}

/* One button of `jo:fe`: its keys and the rows they point at, alive until
 * `pz` returns. */
typedef struct {
    layout_row_keys_t layout_row;
    layout_row_cell_t layout_row_items[PLANET_ROW_CAP];
    layout_row_keys_t cost_keys;
    layout_row_cell_t cost_item;
} planet_projects_panel_row_t;

static void planet_projects_panel_keys(planet_t *self, research_project_t *b, button_keys_t *keys, planet_projects_panel_row_t *row) {
    p8_fixed_t od = research_project_cost(b);

    memset(keys, 0, sizeof *keys);
    if (b->layout_row != NULL) {
        row->layout_row = layout_row_keys_of_row(b->layout_row, row->layout_row_items, PLANET_ROW_CAP);
        keys->layout_row_keys = &row->layout_row;
    } else {
        keys->layout_row_copy = research_project_layout_row(b);
    }

    keys->has_side_tooltip = 1;
    keys->side_tooltip_is_false = od <= p8_fixed_from_int(0);
    if (od > p8_fixed_from_int(0)) {
        row->cost_item = layout_row_cell_string(planet_dollar(p8_tostr_fixed(frame_zone(), od)));
        memset(&row->cost_keys, 0, sizeof row->cost_keys);
        row->cost_keys.items = &row->cost_item;
        row->cost_keys.count = 1;
        row->cost_keys.has_color = 1;
        row->cost_keys.color = p8_fixed_from_int(13);
        keys->side_tooltip_row = &row->cost_keys;
    }

    keys->project_planet = self;
    keys->project = b;
    keys->has_project_cost = 1;
    keys->project_cost = od;
    keys->has_on_click = 1;
    keys->on_click = planet_projects_panel_on_click_method;
    keys->has_tooltip = 1;
    keys->tooltip_row = research_project_tooltip(b);
}

void planet_projects_panel(planet_t *self) {
    available_projects_kept_t available_projects_list;
    button_keys_t *bs;
    planet_projects_panel_row_t *rows;
    open_box_frame_t frame;
    size_t cap;
    int n = 0;
    int i;

    if (self == NULL) {
        return;
    }

    available_projects_list = research_display_available_projects((const entity_t *)self);
    cap = available_projects_list.count > 0 ? (size_t)available_projects_list.count : 1;
    bs = (button_keys_t *)allocator_alloc(frame_zone(), cap * sizeof *bs);
    rows = (planet_projects_panel_row_t *)allocator_alloc(frame_zone(), cap * sizeof *rows);
    memset(bs, 0, cap * sizeof *bs);
    memset(rows, 0, cap * sizeof *rows);
    for (i = 0; i < available_projects_list.count; i++) {
        planet_projects_panel_keys(self, available_projects_list.items[i], &bs[n], &rows[n]);
        n++;
    }

    frame = planet_open_box_frame(self);
    message_box_open_box(&frame, bs, n);
}

void planet_draw(p8_graphics_t *graphics, p8_memory_t *memory, planet_t *self, vec_t screen_pos) {
    vec_t shaken_pos;
    const char *state_val;
    int screen_x;
    int screen_y;
    int spr_index = 43;

    if (self == NULL || !entity_geom_is_on_screen((const entity_t *)self)) {
        return;
    }

    /* Also decays `self.ma`. Everything below draws at `adjusted`, never
     * at the `p` that was passed in. */
    shaken_pos = entity_geom_shake(screen_pos, (entity_t *)self);

    state_val = self->header.state;
    if (state_val != NULL && strcmp(state_val, "vanishing") == 0) {
        p8_fixed_t t = self->header.state_time;
        if (p8_fixed_rnd(p8_fixed_from_int(30)) < p8_fixed_sub(t, p8_fixed_from_int(30))) {
            return;
        }
    }

    screen_x = p8_fixed_flr_int(shaken_pos.x);
    screen_y = p8_fixed_flr_int(shaken_pos.y);

    p8_circfill(graphics, memory, screen_x, screen_y, 7, 0);

    /* An absent `qw` reads as 0, i.e. the default palette block. */
    hud_select_palette_block(memory, (self->header.class->lineage & LINEAGE(LINEAGE_TRADING_HUB)) ? self->palette_block : p8_fixed_from_int(0));
    p8_palt(memory, 3, 0);
    p8_palt(memory, 0, 1);

    /* `A and B or C`: `nk.s` is never absent, so only a nil `nk` falls
     * through to `self.s`, then to the literal 43. */
    if (self->planet_type != NULL) {
        spr_index = self->planet_type->sprite;
    } else if (self->header.class->lineage & LINEAGE(LINEAGE_STRUCTURE)) {
        spr_index = p8_fixed_flr_int(self->sprite);
    }
    p8_spr(graphics, memory, spr_index, screen_x - 4, screen_y - 4, 2, 2);
    hud_select_palette_block_default(memory);

    if (planet_is_colonized(self)) {
        int mi = p8_fixed_flr_int(self->has_development_tier ? self->development_tier : p8_fixed_from_int(0));
        int color = 0;
        int_seq_at(planet_ring_colors()->items, planet_ring_colors()->count, mi, &color);
        p8_circ(graphics, memory, screen_x, screen_y, 6, color);
        p8_spr_default(graphics, memory, mi + 1, screen_x - 3, screen_y - 6);
    }

    {
        if (self->layout_row != NULL) {
            vec_t offset =
                vec_add(shaken_pos, vec_make(p8_fixed_from_int(0), p8_fixed_from_int(7)));
            assert(self->layout_row->header.class->draw_at != NULL && "a layout row has a draw_at method");
            self->layout_row->header.class->draw_at(&self->layout_row->header, offset, P8_FIXED_LIT(0x00008000, 0.5));
        }
    }

    {
        if (self->targeted_by != NULL) {
            p8_fixed_t dw = p8_fixed_from_int(0);
            p8_fixed_t radius_f;
            int radius;

            if (self->targeted_by->targeting != NULL) {
                dw = self->targeted_by->targeting->archetype->attach_distance;
            }
            radius_f = p8_fixed_mul(p8_fixed_sqrt(dw), p8_fixed_from_int(128));
            radius = p8_fixed_flr_int(radius_f);

            /* The pattern literal's fractional `.1` is PICO-8's
             * transparency flag: its set bits leave the screen untouched,
             * so this is `_transparent`, not plain `pico8_fillp`. */
            p8_fillp_transparent(graphics, 0xAAAAu);
            p8_circfill(graphics, memory, screen_x, screen_y, radius, 2);
            p8_fillp_reset(graphics);
            self->targeted_by = NULL;
        }
    }
}

void planet_refresh_method(entity_t *self) {
    planet_refresh((planet_t *)self);
}

down_t planet_accept_drop_method(entity_t *self, const entity_t *s) {
    return planet_accept_drop((const planet_t *)self, s);
}

void planet_draw_method(entity_t *self, const vec_t *ra) {
    vec_t p = ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    planet_draw(dispatch_context_gfx(), dispatch_context_mem(), (planet_t *)self, p);
}

down_t planet_left_press_method(entity_t *self, vec_t *qa, vec_t *ra) {
    (void)qa;
    (void)ra;
    return planet_left_press((planet_t *)self);
}

down_t planet_left_press(planet_t *self) {
    const char *state_val;
    down_t out = {0, NULL};

    if (self == NULL || self->header.state == NULL) {
        return out;
    }
    state_val = self->header.state;
    if (strcmp(state_val, "known") == 0) {
        planet_projects_panel(self);
        out.stop = 1;
        return out;
    }
    if (strcmp(state_val, "settled") == 0) {
        slipway_keys_t keys = {0};

        assert(slipway_ready() && "slipway_init() must run before planet_left_press builds a slipway");
        keys.start = (entity_t *)self;
        out.held = (entity_t *)slipway_new(&keys);
        out.stop = out.held != NULL;
    }
    return out;
}

void planet_apply_project(planet_t *self, research_project_t *research_project_instance) {
    planet_t *result_planet;
    p8_fixed_t fp = p8_fixed_from_int(2);

    research_project_assert(research_project_instance);
    if (self == NULL) {
        return;
    }
    result_planet = self;

    if (research_project_instance != NULL && research_project_instance->class_name != NULL) {
        const entity_kind_t *target_kind = NULL;
        planet_keys_t keys;

        self->is_dead = 1;

        target_kind = planet_kind_named(research_project_instance->class_name);

        keys.map_pos = self->map_pos;
        keys.pos = self->pos;
        keys.planet_id = self->planet_id;

        result_planet = (target_kind != NULL) ? (planet_t *)target_kind->new_entity(&keys) : NULL;
    } else if (research_project_instance != NULL && research_project_instance->next_type != NULL) {
        self->planet_type = planet_types_lookup(research_project_instance->next_type);
    } else {
        entity_set_state((entity_t *)self, "settled");
        planet_install_project(self, research_project_instance);
        self->fold_anchor = p8_fixed_from_int(1);
        fp = tech_gate_if_unlocked("autoassemblers", p8_fixed_from_int(3), p8_fixed_from_int(2));
    }

    economy_advance_clock(main_economy_typed(), fp);

    if (research_project_instance != NULL && research_project_instance->has_play_sfx) {
        p8_play_sfx(research_project_instance->play_sfx);
    } else {
        p8_play_sfx(p8_fixed_from_int(55));
    }

    planet_restart_shake(result_planet);
}
