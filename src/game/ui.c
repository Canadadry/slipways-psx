#include "game/ui.h"

#include <assert.h>

#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/exit_queue.h"
#include "engine/entity.h"
#include "game/mouse_button.h"
#include "game/planet_family.h"
#include "game/scroll.h"
#include "game/ui_button.h"
#include "engine/vec.h"
#include "game/wormhole.h"
#include "engine/frame_zone.h"
#include "game/ui_zone.h"
#include "engine/game_zone.h"

static int g_ui_up = 0;
static entity_t *g_main_mouse = NULL;
static ui_t *g_main_ui = NULL;
static entity_list_t g_shared_ephemerals;

const box_t *ui_panel_box(void) {
    return g_ui_up ? &ui_archetype.panel_box : NULL;
}

entity_list_t *ui_ephemerals_shared(void) {
    return &g_shared_ephemerals;
}

entity_list_t *ui_ephemerals_new(void) {
    const allocator_t *slot = ui_zone_slot(SLOT_KIND_EPHEMERALS);
    entity_list_t *le = (entity_list_t *)allocator_alloc(slot, sizeof *le);

    entity_list_init(le, slot);
    return le;
}

void ui_set_ephemerals(ui_t *self, entity_list_t *le) {
    entity_list_t *old = self->ephemerals;

    self->ephemerals = le;
    if (old != NULL && old != &g_shared_ephemerals) {
        exit_queue_put_arena(old->zone);
    }
}

static const entity_class_t main_mouse_placeholder_class_def = {0};

typedef struct {
    entity_t header;
} main_mouse_placeholder_t;

/* The only one: never allocated, never freed. */
static main_mouse_placeholder_t g_main_mouse_placeholder = {{.class = &main_mouse_placeholder_class_def}};

static int ui_to_int(p8_fixed_t x) {
    return p8_fixed_flr_int(x);
}

static vec_t ui_vec_or_zero(const vec_t *v) {
    return v != NULL ? *v : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
}

/* The button of the main mouse on that side, NULL when it has none (the
 * member-less placeholder) or before its first update. */
static const mouse_button_t *ui_main_mouse_button(mouse_side_t side) {
    const mouse_button_t *b;

    assert(g_main_mouse != NULL && "ui_main_mouse_button: reading a button of an absent main mouse stops the run");
    if (main_mouse_typed() == NULL) {
        return NULL;
    }
    b = side == MOUSE_SIDE_LEFT ? mouse_left_button() : mouse_right_button();
    return b != NULL && b->has_held ? b : NULL;
}

static void ui_draw_overlay_method(entity_t *self, const vec_t *screen_pos) {
    (void)screen_pos;
    ui_draw_overlay(dispatch_context_gfx(), dispatch_context_mem(), (const ui_t *)self);
}

static void ui_ephemerals_release(ui_t *self) {
    if (self->ephemerals != NULL && self->ephemerals != &g_shared_ephemerals) {
        arena_release(self->ephemerals->zone);
    }
    self->ephemerals = NULL;
}

static void ui_release_method(entity_t *self) {
    ui_ephemerals_release((ui_t *)self);
}

static const entity_class_t ui_class_def = {
    .draw_overlay = ui_draw_overlay_method,
    .release = ui_release_method,
};

static ui_t *ui_create(void) {
    ui_t *self;
    int i;

    assert(g_ui_up && "ui_init() must run first");
    self = (ui_t *)allocator_alloc(game_zone(), sizeof *self);
    entity_init_instance(&self->header, &ui_class_def, ui_archetype.state, ui_archetype.state_time);
    self->archetype = &ui_archetype;
    self->header.layer = &self->archetype->layer;
    self->header.cursor_sprite = &self->archetype->cursor_sprite;
    self->ephemerals = &g_shared_ephemerals;
    self->mouse_world = NULL;
    self->hovered_by_layer = NULL;
    self->hovered_by_layer_own.layer_count = UI_HOVER_LAYERS;
    for (i = 0; i < UI_HOVER_LAYERS; i++) {
        entity_list_init(&self->hovered_by_layer_own.layers[i], frame_zone());
    }
    self->hovered = NULL;
    self->previous_hovered = NULL;
    self->left_held = NULL;
    self->right_held = NULL;
    entity_construct(&self->header);
    return self;
}

void ui_init(void) {
    ui_shutdown();
    entity_list_init(&g_shared_ephemerals, game_zone_load_zone());
    g_ui_up = 1;

    g_main_mouse = (entity_t *)&g_main_mouse_placeholder;
}

void ui_shutdown(void) {
    g_ui_up = 0;
    entity_list_free(&g_shared_ephemerals);
    g_main_mouse = NULL;
    g_main_ui = NULL;
}

const entity_class_t *main_mouse_placeholder_class(void) {
    return &main_mouse_placeholder_class_def;
}

int ui_ready(void) {
    return g_ui_up;
}

const entity_class_t *ui_class(void) {
    return &ui_class_def;
}

ui_t *ui_new(void) {
    return ui_create();
}

entity_t *main_mouse(void) {
    return g_main_mouse;
}

mouse_t *main_mouse_typed(void) {
    if (g_main_mouse == NULL || g_main_mouse == &g_main_mouse_placeholder.header) {
        return NULL;
    }
    assert(g_main_mouse->class == mouse_class() && "main_mouse_typed: only the placeholder or a mouse is ever stored");
    return (mouse_t *)g_main_mouse;
}

/* The outgoing instance lives on with its generation. `reset_world()`
 * rebuilds the main mouse and `main_ui()` mid-frame, from a click handled
 * inside ui_update, whose `self` is still the old one after it returns. */
void ui_set_main_mouse(entity_t *instance) {
    if (instance == g_main_mouse) {
        return;
    }
    g_main_mouse = instance;
}

ui_t *main_ui(void) {
    return g_main_ui;
}

/* Same non-freeing rule as ui_set_main_mouse(), and the same reason. */
void ui_set_main_ui(ui_t *ui) {
    if (ui == g_main_ui) {
        return;
    }
    g_main_ui = ui;
}

/* The ephemerals are read before the hover and button handling run: the
 * test further down is a pointer identity check against this frame's list. */
void ui_update(p8_memory_t *mem, ui_t *self) {
    entity_list_t *ephemerals_ptr_before;
    int ephemerals_count;
    vec_t qa;
    int hit;
    const mouse_button_t *left;
    const mouse_t *je;
    const camera_t *mo;

    (void)mem;
    if (self == NULL) {
        return;
    }

    ephemerals_ptr_before = self->ephemerals;
    ephemerals_count = self->ephemerals != NULL ? self->ephemerals->count : 0;

    je = main_mouse_typed();
    mo = main_camera_typed();
    qa = vec_add(ui_vec_or_zero(je != NULL ? je->pos : NULL),
                       ui_vec_or_zero(mo != NULL ? mo->pos : NULL));
    self->mouse_world_own = qa;
    self->mouse_world = &self->mouse_world_own;

    ui_update_hover(self);

    hit = (ephemerals_count == 0) || entity_list_has(ephemerals_ptr_before, self->hovered);
    if (hit) {
        ui_handle_button(self, MOUSE_SIDE_LEFT);
    }

    ui_handle_button(self, MOUSE_SIDE_RIGHT);

    left = ui_main_mouse_button(MOUSE_SIDE_LEFT);
    if (left != NULL && left->pressed && self->ephemerals == ephemerals_ptr_before) {
        ui_clear_ephemerals(self);
    }

    self->previous_hovered = self->hovered;
}

/* A right-held background shows sprite 9; a hovered entity whose own
 * `cursor_sprite` is absent falls through to self's own cursor sprite. */
void ui_draw_overlay(const p8_graphics_t *gfx, p8_memory_t *mem, const ui_t *self) {
    vec_t main_mouse_pos;
    const mouse_t *je;
    int spr_id = 0;

    if (self == NULL || gfx == NULL || mem == NULL) {
        return;
    }

    if (self->right_held != NULL) {
        spr_id = 9;
    } else if (self->hovered != NULL) {
        spr_id = ui_to_int(self->hovered->cursor_sprite != NULL ? *self->hovered->cursor_sprite : self->archetype->cursor_sprite);
    } else {
        spr_id = ui_to_int(self->archetype->cursor_sprite);
    }

    je = main_mouse_typed();
    main_mouse_pos = ui_vec_or_zero(je != NULL ? je->pos : NULL);
    p8_spr_default(gfx, mem, spr_id, ui_to_int(main_mouse_pos.x), ui_to_int(main_mouse_pos.y));
}

/* An absent previously hovered entity is skipped. The previous one is
 * cleared before the hovered one is set, so an entity hovered on two frames
 * in a row stays hovered. */
void ui_update_hover(ui_t *self) {
    const mouse_t *mouse;
    const vec_t *main_mouse_pos;
    const entity_list_t *hit_test_bucket;
    int index;

    if (self == NULL) {
        return;
    }

    for (index = 0; index < UI_HOVER_LAYERS; index++) {
        entity_list_init(&self->hovered_by_layer_own.layers[index], frame_zone());
    }
    self->hovered_by_layer = &self->hovered_by_layer_own;
    self->hovered = NULL;

    mouse = main_mouse_typed();
    main_mouse_pos = mouse != NULL ? mouse->pos : NULL;

    hit_test_bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
    for (index = 0; index < hit_test_bucket->count; index++) {
        entity_t *e = hit_test_bucket->items[index];

        if (e->class->hit_test != NULL && e->class->hit_test(e, self->mouse_world, main_mouse_pos)) {
            if (e->layer != NULL) {
                int fa = ui_to_int(*e->layer);

                assert(fa >= 0 && fa < UI_HOVER_LAYERS &&
                       "ui_update_hover: a hovered entity's layer outside 0..15, the layers ui_send_to_hovered walks -- stop, do not widen");
                entity_list_add(&self->hovered_by_layer->layers[fa], e);
            }
            self->hovered = e;
        }
    }

    if (self->previous_hovered != NULL) {
        assert(self->previous_hovered->is_hovered != NULL &&
               "ui_update_hover: the previously hovered entity has no is_hovered member -- stop, do not add one");
        *self->previous_hovered->is_hovered = 0;
    }

    if (self->hovered != NULL) {
        assert(self->hovered->is_hovered != NULL &&
               "ui_update_hover: the hovered entity has no is_hovered member -- stop, do not add one");
        *self->hovered->is_hovered = 1;
    }
}

static int ui_is_held_class(const entity_class_t *cls) {
    return cls == slipway_class() || cls == probe_class() || cls == wormhole_class() ||
           (cls->states[ENTITY_STATE_SETTLED] != NULL && cls != protostar_class());
}

void ui_handle_button(ui_t *self, mouse_side_t side) {
    int is_left;
    const mouse_button_t *button;
    entity_t *held_inst;
    mouse_t *mouse;
    vec_t *main_mouse_pos;

    if (self == NULL) {
        return;
    }

    is_left = side == MOUSE_SIDE_LEFT;
    button = ui_main_mouse_button(side);
    mouse = main_mouse_typed();
    main_mouse_pos = mouse != NULL ? mouse->pos : NULL;

    if (button != NULL && button->pressed) {
        entity_t *kv = NULL;
        down_t kn;

        if (ui_send_to_hovered(self, is_left ? SEND_TO_HOVERED_LEFT_PRESS : SEND_TO_HOVERED_RIGHT_PRESS, self->mouse_world, main_mouse_pos, NULL, &kv, &kn) &&
            kn.held != NULL) {
            if (is_left) {
                assert(ui_is_held_class(kn.held->class) &&
                       "ui_handle_button: a left press hands over a slipway; a probe or a wormhole from the background; a "
                       "structure, processor, slipgate, infraspace_gate, trading_hub, synthesizer or ascension from an icon");
                self->left_held = kn.held;
            } else {
                assert(kn.held->class == background_class() && "ui_handle_button: a right press hands over the background only");
                self->right_held = (background_t *)kn.held;
            }
        }
    }

    held_inst = is_left ? self->left_held : (self->right_held != NULL ? &self->right_held->header : NULL);

    if (held_inst != NULL && held_inst->class->drag != NULL) {
        held_inst->class->drag(held_inst, self->mouse_world, main_mouse_pos);
    }

    if (button != NULL && button->released && held_inst != NULL) {
        entity_t *target = NULL;
        down_t down;

        /* The drop target is the entity ui_send_to_hovered found, not a
         * field read on the held entity. */
        (void)ui_send_to_hovered(self, SEND_TO_HOVERED_ACCEPT_DROP, NULL, NULL, held_inst, &target, &down);

        if (held_inst->class->drop != NULL) {
            held_inst->class->drop(held_inst, target);
        }
        if (is_left) {
            self->left_held = NULL;
        } else {
            self->right_held = NULL;
        }
    }
}

/* Layers descend from 15 to 0 and the first answer whose `stop` is set
 * wins: that order is the dispatch priority between overlapping entities,
 * not an arbitrary walk. */
int ui_send_to_hovered(ui_t *self, send_to_hovered_slot_t message, vec_t *mouse_world, vec_t *mouse_screen, const entity_t *held,
                entity_t **out_target, down_t *out_down) {
    int layer;

    if (self == NULL) {
        return 0;
    }

    if (self->hovered_by_layer == NULL) {
        return 0;
    }

    for (layer = UI_HOVER_LAYERS - 1; layer >= 0; layer--) {
        const entity_list_t *bucket = &self->hovered_by_layer->layers[layer];
        int i;

        for (i = 0; i < bucket->count; i++) {
            entity_t *h = bucket->items[i];
            down_t kn = {0, NULL};

            switch (message) {
            case SEND_TO_HOVERED_LEFT_PRESS:
                if (h->class->left_press != NULL) {
                    kn = h->class->left_press(h, mouse_world, mouse_screen);
                }
                break;
            case SEND_TO_HOVERED_RIGHT_PRESS:
                if (h->class->right_press != NULL) {
                    kn = h->class->right_press(h, mouse_world, mouse_screen);
                }
                break;
            case SEND_TO_HOVERED_ACCEPT_DROP:
                if (h->class->accept_drop != NULL) {
                    kn = h->class->accept_drop(h, held);
                }
                break;
            }
            if (kn.stop) {
                *out_target = h;
                *out_down = kn;
                return 1;
            }
        }
    }

    return 0;
}

void ui_clear_ephemerals(ui_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->ephemerals != NULL) {
        entity_list_iter_t it = entity_list_all(self->ephemerals);
        entity_t *e;
        while (entity_list_iter_next(&it, &e)) {
            if (e != NULL) {
                entity_unregister(e);
            }
        }
    }

    ui_set_ephemerals(self, ui_ephemerals_new());
}
