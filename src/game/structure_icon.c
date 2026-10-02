#include "game/structure_icon.h"

#include <assert.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "engine/entity_geom.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "engine/entity.h"
#include "game/layout.h"
#include "game/draw_functions.h"
#include "game/planet.h"
#include "game/probe.h"
#include "p8/str.h"
#include "game/tech_gate.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

static int g_structure_icon_up = 0;

/* `ie.gy`, `ie.id`: mutated on the class by the cart, so one cursor and one
 * counter shared by every icon, never a member. `gy` is rebound, never
 * written through: an icon's `ra`, and `eh`, may hold its REF. Only `_init`
 * resets `gy`, only pico8_research_icon_init() resets `id`. */
static const vec_t *g_cursor = NULL;
static p8_fixed_t g_sprite_counter = 0;
static int g_has_sprite_counter = 0;

static int structure_icon_to_int(p8_fixed_t x) {
    return p8_fixed_flr_int(x);
}

/* Returns the kind held in `self.ke`, or NULL when the member is nil. The
 * result is borrowed. */
static const entity_kind_t *structure_icon_own_kind(const structure_icon_t *self) {
    return self != NULL ? self->kind : NULL;
}

static const entity_class_t g_structure_icon_kind_view_class = {0};

structure_icon_kind_t structure_icon_kind_view(const structure_icon_t *self) {
    const entity_kind_t *kind = structure_icon_own_kind(self);
    structure_icon_kind_t view;

    entity_init_header(&view.header, &g_structure_icon_kind_view_class);
    view.required_tech = NULL;
    view.sprite = 0;
    view.has_sprite = 0;
    view.state = NULL;
    view.cost = 0;
    view.has_cost = 0;
    if (kind != NULL) {
        view.required_tech = kind->record->required_tech;
        view.sprite = kind->record->sprite;
        view.has_sprite = kind->record->has_sprite;
        view.state = kind->record->state;
        view.cost = kind->record->cost;
        view.has_cost = kind->record->has_cost;
    }
    return view;
}

const entity_class_t *structure_icon_kind_view_class(void) {
    return &g_structure_icon_kind_view_class;
}

void structure_icon_refresh(structure_icon_t *self) {
    const char *rr;

    if (self == NULL) {
        return;
    }

    if (self->has_is_revealed && self->is_revealed) {
        return;
    }

    /* `qh`/`os` never set `rr`, so `rr` stays absent for them and
     * pico8_oj() then answers true unconditionally. */
    rr = structure_icon_kind_view(self).required_tech;

    if (!tech_gate_is_unlocked(rr)) {
        return;
    }

    assert(g_structure_icon_up && "pico8_research_icon_init() must run before ie:hb writes ie.gy");

    /* `self.ra` points at the vector `ie.gy` points at now. The `-=` below rebinds
     * `ie.gy` to a fresh vector rather than writing through the old one, so
     * this snapshot survives it. */
    self->pos = g_cursor;
    self->is_revealed = 1;
    self->has_is_revealed = 1;

    {
        vec_t current =
            g_cursor != NULL ? *g_cursor : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
        vec_t delta = vec_make(p8_fixed_from_int(10), p8_fixed_from_int(0));
        vec_t *box = (vec_t *)allocator_alloc(game_zone(), sizeof *box);
        *box = vec_sub(current, delta);
        g_cursor = box;
    }
}

static void structure_icon_refresh_method(entity_t *self) {
    structure_icon_refresh((structure_icon_t *)self);
}

down_t structure_icon_left_press(structure_icon_t *self) {
    p8_fixed_t new_id;
    const entity_kind_t *entity_kind;
    char *id_num_str;
    char *id_str;
    planet_keys_t keys;
    down_t out = {0, NULL};

    assert(g_structure_icon_up && "pico8_research_icon_init() must run before ie:ldown writes ie.id");
    new_id = p8_fixed_add(g_has_sprite_counter ? g_sprite_counter : p8_fixed_from_int(0),
                             p8_fixed_from_int(1));
    g_sprite_counter = new_id;
    g_has_sprite_counter = 1;

    entity_kind = structure_icon_own_kind(self);
    if (entity_kind == NULL) {
        return out;
    }

    /* The grab opens before `id_str` is made: the planet keeps that exact
     * pointer as its `id`, so it must live and die with the grab's arena. */
    planet_grab_arena_open();
    id_num_str = p8_tostr_fixed(frame_zone(), new_id);
    id_str = p8_concat2(planet_zone_construction(), "s", id_num_str);
    keys.planet_id = id_str;
    keys.map_pos = NULL;
    keys.pos = NULL;
    out.held = entity_kind->new_entity(&keys);
    planet_grab_arena_close();
    out.stop = out.held != NULL;
    return out;
}

static down_t structure_icon_left_press_method(entity_t *self, vec_t *qa, vec_t *ra) {
    (void)qa;
    (void)ra;
    return structure_icon_left_press((structure_icon_t *)self);
}

void structure_icon_draw(p8_graphics_t *gfx, p8_memory_t *mem, structure_icon_t *self, vec_t p) {
    structure_icon_kind_t view;

    /* An icon not yet revealed by `ie:hb` draws nothing at all. */
    if (self == NULL || !(self->has_is_revealed && self->is_revealed)) {
        return;
    }
    if (gfx == NULL || mem == NULL) {
        return;
    }

    p8_palt(mem, 3, 0);

    view = structure_icon_kind_view(self);
    p8_spr_default(gfx, mem, view.has_sprite ? structure_icon_to_int(view.sprite) : 0, structure_icon_to_int(p.x),
                      structure_icon_to_int(p.y));

    /* `dl(self.ke)` is not inert: every kind `self.ke` can hold inherits
     * `qh`'s `me="kz"`, so `dl` passes its own state check and draws the
     * kind-level build cost. */
    if (self->is_hovered) {
        if (structure_icon_own_kind(self) != NULL) {
            targeting_t pb = {.archetype = &targeting_archetype_none};

            pb.has_cost = view.has_cost;
            pb.cost = view.cost;
            targeting_tooltip_show(gfx, mem, view.state, &pb);
        }
        if (self->tooltip != NULL) {
            tooltip_draw_at(gfx, mem, self->tooltip, p8_fixed_from_int(0), p8_fixed_from_int(0));
        }
    }

    p8_palt(mem, 3, 1);
}

/* `dc("kj")` calls `qc[nt](qc,qc.ra)`, so `ra` is always `self.ra`. */
static void structure_icon_draw_method(entity_t *self, const vec_t *ra) {
    vec_t p = ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    structure_icon_draw(dispatch_context_gfx(), dispatch_context_mem(), (structure_icon_t *)self, p);
}

/* `ie.hd=gk`: a field assignment in the cart, not a method definition. */
static int structure_icon_hit_test_method(entity_t *self, const vec_t *qa, const vec_t *ra) {
    return entity_geom_contains_cursor(self, qa, ra);
}

static const entity_class_t g_structure_icon_class = {
    .draw = structure_icon_draw_method,
    .hit_test = structure_icon_hit_test_method,
    .refresh = structure_icon_refresh_method,
    .left_press = structure_icon_left_press_method,
};

static structure_icon_t *structure_icon_create(const structure_icon_keys_t *keys) {
    structure_icon_t *self = (structure_icon_t *)allocator_alloc(game_zone(), sizeof *self);

    assert(g_structure_icon_up && "pico8_research_icon_init() must run before an ie is built");
    entity_init_instance(&self->header, &g_structure_icon_class, structure_icon_archetype.state, structure_icon_archetype.state_time);
    self->archetype = &structure_icon_archetype;
    self->header.layer = &self->archetype->layer;
    self->header.pos = &self->pos;
    self->header.hit_box = &self->archetype->hit_box;
    self->pos = &self->archetype->pos;
    self->kind = NULL;
    self->tooltip = NULL;
    self->is_revealed = 0;
    self->has_is_revealed = 0;
    self->header.is_hovered = &self->is_hovered;
    self->is_hovered = 0;
    if (keys != NULL) {
        self->kind = keys->kind;
        self->tooltip = keys->tooltip;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("structure_icon");
#endif
    entity_construct(&self->header);
    return self;
}

void structure_icon_init(void) {
    structure_icon_shutdown();

    draw_functions_init();

    g_structure_icon_up = 1;
    g_cursor = NULL;
    g_sprite_counter = p8_fixed_from_int(0);
    g_has_sprite_counter = 1;
}

void structure_icon_shutdown(void) {
    g_structure_icon_up = 0;
    g_cursor = NULL;
}

int structure_icon_ready(void) {
    return g_structure_icon_up;
}

const entity_class_t *structure_icon_class(void) {
    return &g_structure_icon_class;
}

structure_icon_t *structure_icon_new(const structure_icon_keys_t *keys) {
    return structure_icon_create(keys);
}

const vec_t *structure_icon_rest_cursor(void) {
    return g_structure_icon_up ? &structure_icon_archetype.rest_count : NULL;
}

const vec_t *structure_icon_cursor(void) {
    return g_cursor;
}

int structure_icon_sprite_counter(p8_fixed_t *out) {
    if (!g_has_sprite_counter) {
        return 0;
    }
    *out = g_sprite_counter;
    return 1;
}

void structure_icon_set_cursor(const vec_t *gy) {
    g_cursor = gy;
}
