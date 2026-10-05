#include "game/slipway_display.h"

#include <assert.h>
#include <stddef.h>

#include "engine/dispatch_context.h"
#include "engine/entity_geom.h"
#include "engine/entity_id.h"
#include "game/planet.h"
#include "game/slipway.h"
#include "engine/vec.h"
#include "game/particle_zone.h"

typedef char transit_dot_fits_its_slot_t[sizeof(transit_dot_t) <= TRANSIT_DOTS_SLOT ? 1 : -1];

static int to_int(p8_fixed_t x) {
    return p8_fixed_flr_int(x);
}

void slipway_spawn_transit_dot(slipway_t *self, const entity_t *f, const entity_t *t) {
    if (self == NULL) {
        return;
    }
    /* The random draw must stay before the served planets test, or the RNG desynchronises. */
    if (p8_fixed_rnd_default() >= P8_FIXED_LIT(0x0000147a, 0.08)) {
        return;
    }

    if (self->served_planets != NULL) {
        /* The mouse point slipway_drag hangs the end on is not a planet. */
        const char *t_id = t != NULL && planet_is_family(t) ? ((const planet_t *)t)->planet_id : NULL;
        if (slipway_served_planets_has(self->served_planets, t_id)) {
            if (self->transit_dots != NULL) {
                vec_t f_pos = slipway_end_pos(f);
                vec_t t_pos = slipway_end_pos(t);
                vec_t d = vec_sub(t_pos, f_pos);
                vec_t v = vec_normalize(d);
                transit_dot_t *particle = (transit_dot_t *)allocator_alloc(particle_zone(PARTICLE_SLOT_TRANSIT_DOT),
                                                                      sizeof *particle);

                particle->pos = vec_add(f_pos, vec_perpendicular(v));
                particle->velocity = v;
                particle->life = vec_length(d);

                pointer_list_add(self->transit_dots, particle);
                particle_zone_record_elements(PARTICLE_SLOT_TRANSIT_DOT, self->transit_dots->count);
            }
        }
    }
}

void slipway_draw_line(const p8_graphics_t *gfx, p8_vram_t *mem, const slipway_t *self, p8_fixed_t l,
                  p8_fixed_t h, p8_fixed_t s, int c) {
    entity_t *qu;
    entity_t *re;
    p8_fixed_t t;
    vec_t start_point;
    vec_t end_point;
    p8_fixed_t dx;

    if (self == NULL) {
        return;
    }

    assert(s > p8_fixed_from_int(0) && "slipway_draw_line only ascends: a zero step hangs, a negative one draws nothing");
    assert(gfx != NULL && mem != NULL && "p8_line dereferences the graphics state and memory: neither may be NULL here");

    qu = slipway_start(self);
    re = slipway_end(self);
    t = self->header.state_time;

    start_point = vec_add(slipway_end_pos(qu),
                           vec_from_polar(P8_FIXED_LIT(0x00018000, 1.5), p8_fixed_div(t, p8_fixed_from_int(126))));
    end_point = vec_add(slipway_end_pos(re),
                           vec_from_polar(P8_FIXED_LIT(0x00018000, 1.5), p8_fixed_div(t, p8_fixed_from_int(176))));

    for (dx = l; dx <= h; dx = p8_fixed_add(dx, s)) {
        p8_fixed_t dy;
        for (dy = l; dy <= h; dy = p8_fixed_add(dy, s)) {
            int x1 = to_int(p8_fixed_add(start_point.x, dx));
            int y1 = to_int(p8_fixed_add(start_point.y, dy));
            int x2 = to_int(p8_fixed_add(end_point.x, dx));
            int y2 = to_int(p8_fixed_add(end_point.y, dy));
            p8_line(gfx, mem, x1, y1, x2, y2, c);
        }
    }
}

void slipway_draw(const p8_graphics_t *graphics, p8_vram_t *memory, slipway_t *self) {
    entity_t *start_node;
    entity_t *end_node;

    if (self == NULL) {
        return;
    }

    start_node = slipway_start(self);
    end_node = slipway_end(self);

    if (entity_geom_is_on_screen(start_node) || entity_geom_is_on_screen(end_node)) {
        p8_fixed_t fade_in_step = self->fade_in_step;
        int fade_in_step_i = to_int(fade_in_step);
        p8_fixed_t fade_color = p8_fixed_from_int(0);
        int target_or_targeted_by;
        int infraspace_truthy;
        p8_fixed_t line_color;
        p8_fixed_t border_color;
        p8_fixed_t dot_color;

        assert(fade_in_step_i >= 1 && "trail colors are 1-based: a fade-in step of 0 would silently draw the slipway black");

        if (self->archetype->trail_colors != NULL) {
            int entry;
            if (int_seq_at(self->archetype->trail_colors->items, self->archetype->trail_colors->count, fade_in_step_i, &entry)) {
                fade_color = p8_fixed_from_int(entry);
            }
        }

        if (fade_in_step < p8_fixed_from_int(20)) {
            self->fade_in_step = p8_fixed_add(fade_in_step, p8_fixed_from_int(1));
        }

        target_or_targeted_by = targeting_target(self->targeting) != NULL || entity_get(self->targeted_by) != NULL;
        infraspace_truthy = entity_has(&self->header, ENTITY_HAS_INFRASPACE);

        line_color = target_or_targeted_by ? p8_fixed_from_int(2) : (infraspace_truthy ? p8_fixed_from_int(0) : fade_color);
        border_color = infraspace_truthy ? fade_color : p8_fixed_from_int(0);
        dot_color = infraspace_truthy ? p8_fixed_from_int(5) : p8_fixed_from_int(13);

        slipway_draw_line(graphics, memory, self, p8_fixed_from_int(-1), p8_fixed_from_int(2), p8_fixed_from_int(3),
                    to_int(border_color));
        slipway_draw_line(graphics, memory, self, p8_fixed_from_int(0), p8_fixed_from_int(1), p8_fixed_from_int(1),
                    to_int(line_color));

        {
            pointer_list_t *ts = self->transit_dots;
            if (ts != NULL) {
                int i;
                for (i = 0; i < ts->count; i++) {
                    transit_dot_t *t = ts->items[i];
                    vec_t new_pos;
                    p8_fixed_t new_l;

                    if (t == NULL) {
                        continue;
                    }

                    new_pos = vec_add(t->pos, t->velocity);
                    t->pos = new_pos;

                    new_l = p8_fixed_sub(t->life, p8_fixed_from_int(1));
                    t->life = new_l;

                    /* No `else`: an expired particle is still drawn once more at its final position. */
                    if (new_l <= p8_fixed_from_int(0)) {
                        pointer_list_del(ts, t);
                        allocator_release(particle_zone(PARTICLE_SLOT_TRANSIT_DOT), (unsigned char *)t);
                    }

                    {
                        int x = to_int(new_pos.x);
                        int y = to_int(new_pos.y);
                        p8_rectfill(graphics, memory, x, y, x + 1, y + 1, 0);
                        p8_pset(graphics, memory, x, y, to_int(dot_color));
                    }
                }
            }
        }

        slipway_spawn_transit_dot(self, start_node, end_node);
        slipway_spawn_transit_dot(self, end_node, start_node);
    }

    self->targeted_by = ENTITY_ID_NONE;
}

void slipway_draw_method(entity_t *self, const vec_t *screen_pos) {
    (void)screen_pos;
    slipway_draw(dispatch_context_gfx(), dispatch_context_mem(), (slipway_t *)self);
}
