#include "game/ui_button.h"

#include <string.h>

#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "engine/geom.h"
#include "engine/entity.h"
#include "game/layout.h"
#include "game/ui_zone.h"

static int g_button_up = 0;
static int g_modal_backdrop_up = 0;

static void release_button(entity_t *self) {
    button_t *js = (button_t *)self;

    if (js->layout_row_slot != NULL) {
        arena_release(js->layout_row_slot);
    }
    arena_release(js->slot);
}

static void release_modal_backdrop(entity_t *self) {
    arena_release(((modal_backdrop_t *)self)->slot);
}

static int ui_to_int(p8_fixed_t x) {
    return p8_fixed_flr_int(x);
}

static void button_update_method(entity_t *self, p8_fixed_t t) {
    (void)t;
    button_update((button_t *)self);
}

static void button_draw_overlay_method(entity_t *self, vec_t screen_pos) {
    (void)screen_pos;
    button_draw_overlay(dispatch_context_gfx(), dispatch_context_mem(), (button_t *)self);
}

static void button_draw_method(entity_t *self, vec_t screen_pos) {
    button_draw(dispatch_context_gfx(), dispatch_context_mem(), (button_t *)self, screen_pos);
}

static int button_hit_test_method(entity_t *self, mouse_pos_t mouse) {
    return entity_geom_contains_cursor(self, mouse);
}

static down_t button_left_press_method(entity_t *self, mouse_pos_t mouse) {
    (void)mouse;
    return button_left_press((button_t *)self);
}

static int button_has_refresh(const entity_t *self) {
    return ((const button_t *)self)->refresh != NULL;
}

static void button_refresh_method(entity_t *self) {
    button_t *js = (button_t *)self;

    if (js->refresh != NULL) {
        js->refresh(js);
    }
}

static const entity_class_t button_class_def = {
    .draw = button_draw_method,
    .draw_overlay = button_draw_overlay_method,
    .hit_test = button_hit_test_method,
    .refresh = button_refresh_method,
    .has_refresh = button_has_refresh,
    .left_press = button_left_press_method,
    .states = {[ENTITY_STATE_NORMAL] = button_update_method},
    .release = release_button,
};

static button_t *button_create(const button_keys_t *keys) {
    const allocator_t *slot = ui_zone_slot(SLOT_KIND_BUTTON);
    button_t *self = (button_t *)allocator_alloc(slot, sizeof *self);

    self->slot = slot;
    entity_init_instance(&self->header, &button_class_def, button_archetype.state, button_archetype.state_time);
    self->layout_row = NULL;
    self->layout_row_slot = NULL;
    self->width = 0;
    self->side_tooltip = NULL;
    self->has_side_tooltip = 0;
    self->tooltip = NULL;
    self->has = 0;
    self->slide_target = vec_make(0, 0);
    self->on_click = NULL;
    self->has_on_click = 0;
    self->header.has |= ENTITY_HAS_IS_HOVERED;
    self->header.is_hovered = 0;
    self->refresh = NULL;
    self->name = NULL;
    self->trade = 0;
    self->has_trade = 0;
    self->type_bias = 0;
    self->has_type_bias = 0;
    self->technology = NULL;
    self->project_planet = ENTITY_ID_NONE;
    self->project = RESEARCH_PROJECT_ID_NONE;
    self->project_cost = 0;
    self->has_project_cost = 0;
    self->archetype = &button_archetype;
    self->header.has |= ENTITY_HAS_LAYER;
    self->header.layer = self->archetype->layer;
    if (keys != NULL) {
        if (keys->has_layer) {
            self->header.layer = keys->layer;
        }
        if (keys->has_pos) {
            entity_set_pos(&self->header, keys->pos);
        }
        if (keys->has_slide_target) {
            self->slide_target = keys->slide_target;
            self->has |= BUTTON_HAS_SLIDE_TARGET;
        }
        self->on_click = keys->on_click;
        self->has_on_click = keys->has_on_click;
        self->refresh = keys->refresh;
        self->name = keys->name;
        self->trade = keys->trade;
        self->has_trade = keys->has_trade;
        self->type_bias = keys->type_bias;
        self->has_type_bias = keys->has_type_bias;
        self->technology = keys->technology;
        self->project_planet = keys->project_planet;
        self->project = keys->project;
        self->project_cost = keys->project_cost;
        self->has_project_cost = keys->has_project_cost;
    }
    button_construct(self, keys);
    entity_construct(&self->header);
    return self;
}

void button_init(void) {
    button_shutdown();
    g_button_up = 1;
}

void button_shutdown(void) {
    g_button_up = 0;
}

int button_ready(void) {
    return g_button_up;
}

const entity_class_t *button_class(void) {
    return &button_class_def;
}

void button_keys_set_tooltip_row(button_keys_t *keys, const data_row_t *row) {
    keys->has_tooltip_row = row != NULL;
    if (row != NULL) {
        keys->tooltip_row = *row;
    }
}

button_t *button_new(const button_keys_t *keys) {
    return button_create(keys);
}

void button_construct(button_t *self, const button_keys_t *keys) {
    static const button_keys_t none;

    if (self == NULL) {
        return;
    }
    if (keys == NULL) {
        keys = &none;
    }

    self->layout_row = layout_row_new(self->slot, &keys->layout_row_keys);
    self->width = p8_fixed_add(self->layout_row->width, p8_fixed_from_int(2));
    self->header.has |= ENTITY_HAS_HIT_BOX;
    self->header.hit_box = (box_t){p8_fixed_from_int(0), p8_fixed_from_int(-1), self->width, p8_fixed_from_int(11)};

    /* GAP-23 */
    if (keys->has_side_tooltip_row || (keys->has_side_tooltip && !keys->side_tooltip_is_false)) {
        self->side_tooltip = layout_row_new(self->slot, keys->has_side_tooltip_row ? &keys->side_tooltip_row : NULL);
        self->has_side_tooltip = 1;
    } else if (keys->has_side_tooltip) {
        self->side_tooltip = NULL;
        self->has_side_tooltip = 1;
    }
    if (keys->has_tooltip_row || keys->has_tooltip) {
        self->tooltip = tooltip_new(self->slot, keys->has_tooltip_row ? &keys->tooltip_row : NULL);
    }
}

void button_update(button_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->has & BUTTON_HAS_SLIDE_TARGET) {
        vec_t qf = self->slide_target;
        vec_t ra = entity_pos(&self->header);
        vec_t d = vec_sub(qf, ra);
        p8_fixed_t length_squared = vec_length_squared(d);
        vec_t delta = (length_squared <= p8_fixed_from_int(1)) ? d : vec_mul(d, P8_FIXED_LIT(0x00006666, 0.4));
        entity_set_pos(&self->header, vec_add(ra, delta));
    }
}

void button_draw_overlay(p8_graphics_t *gfx, p8_vram_t *mem, const button_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->tooltip != NULL && self->header.is_hovered) {
        tooltip_draw_at(gfx, mem, self->tooltip, p8_fixed_from_int(P8_SCREEN_HEIGHT - 1), p8_fixed_from_int(1));
    }
}

void button_draw(p8_graphics_t *graphics, p8_vram_t *memory, const button_t *self, vec_t draw_pos) {
    p8_fixed_t width;
    p8_fixed_t right;
    p8_fixed_t bottom;
    int has_on_click;
    int hovered;
    int left_pixel;
    int top_pixel;
    int right_pixel;
    int bottom_pixel;

    if (self == NULL) {
        return;
    }

    width = self->width;
    right = p8_fixed_sub(p8_fixed_add(draw_pos.x, width), p8_fixed_from_int(1));
    bottom = p8_fixed_add(draw_pos.y, p8_fixed_from_int(9));
    has_on_click = self->on_click != NULL;
    hovered = self->header.is_hovered;

    left_pixel = ui_to_int(draw_pos.x);
    top_pixel = ui_to_int(draw_pos.y);
    right_pixel = ui_to_int(right);
    bottom_pixel = ui_to_int(bottom);

    p8_rect(graphics, memory, left_pixel - 1, top_pixel, right_pixel + 1, bottom_pixel + 1, 1);
    p8_rectfill(graphics, memory, left_pixel, top_pixel, right_pixel, bottom_pixel, has_on_click ? (hovered ? 12 : 13) : 5);
    p8_rectfill(graphics, memory, left_pixel, top_pixel, right_pixel, top_pixel, has_on_click ? 6 : 5);

    if (self->layout_row != NULL) {
        vec_t off = vec_add(draw_pos, vec_make(p8_fixed_from_int(1), p8_fixed_from_int(2)));
        layout_row_draw_at_default(graphics, memory, self->layout_row, off);
    }

    if (self->side_tooltip != NULL) {
        const layout_row_t *c = self->side_tooltip;
        p8_fixed_t c_w = c->width;
        int bg = 2;
        p8_fixed_t x2 = p8_fixed_add(right, c_w);

        if (c->has_background && !c->background_is_bool) {
            bg = ui_to_int(c->background);
        }
        p8_rectfill(graphics, memory, ui_to_int(right) + 3, top_pixel + 1, ui_to_int(x2) + 4, bottom_pixel, bg);

        {
            vec_t off2 =
                vec_add(draw_pos, vec_make(p8_fixed_add(width, p8_fixed_from_int(3)), p8_fixed_from_int(2)));
            layout_row_draw_at_default(graphics, memory, c, off2);
        }
    }
}

down_t button_left_press(button_t *self) {
    int has_on_click = self != NULL && self->on_click != NULL;
    down_t out = {1, ENTITY_ID_NONE};

    if (has_on_click) {
        self->on_click(self);
    }

    return out;
}

static down_t modal_backdrop_left_press_method(entity_t *self, mouse_pos_t mouse) {
    (void)mouse;
    return modal_backdrop_left_press((modal_backdrop_t *)self);
}

static const entity_class_t modal_backdrop_class_def = {
    .hit_test = entity_registry_hit_test_always,
    .left_press = modal_backdrop_left_press_method,
    .release = release_modal_backdrop,
};

static modal_backdrop_t *modal_backdrop_create(const modal_backdrop_keys_t *keys) {
    const allocator_t *slot = ui_zone_slot(SLOT_KIND_MODAL_BACKDROP);
    modal_backdrop_t *self = (modal_backdrop_t *)allocator_alloc(slot, sizeof *self);

    self->slot = slot;
    entity_init_instance(&self->header, &modal_backdrop_class_def, modal_backdrop_archetype.state, modal_backdrop_archetype.state_time);
    self->archetype = &modal_backdrop_archetype;
    self->header.has |= ENTITY_HAS_LAYER;
    self->header.layer = self->archetype->layer;
    self->on_click = NULL;
    self->header.has |= ENTITY_HAS_IS_DEAD;
    self->header.is_dead = 0;
    self->header.has |= ENTITY_HAS_IS_HOVERED;
    self->header.is_hovered = 0;
    if (keys != NULL) {
        self->on_click = keys->on_click;
    }
    entity_construct(&self->header);
    return self;
}

void modal_backdrop_init(void) {
    modal_backdrop_shutdown();
    g_modal_backdrop_up = 1;
}

void modal_backdrop_shutdown(void) {
    g_modal_backdrop_up = 0;
}

int modal_backdrop_ready(void) {
    return g_modal_backdrop_up;
}

const entity_class_t *modal_backdrop_class(void) {
    return &modal_backdrop_class_def;
}

modal_backdrop_t *modal_backdrop_new(const modal_backdrop_keys_t *keys) {
    return modal_backdrop_create(keys);
}

down_t modal_backdrop_left_press(modal_backdrop_t *self) {
    down_t out = {1, ENTITY_ID_NONE};

    if (self == NULL) {
        return out;
    }
    self->header.is_dead = 0;
    if (self->on_click != NULL) {
        self->header.is_dead = self->on_click(self) != 0;
    }
    return out;
}
