#include "game/message_box.h"
#include "engine/entity_id.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "p8/fixed.h"
#include "game/hud.h"
#include "engine/entity.h"
#include "game/layout.h"
#include "game/draw_functions.h"
#include "game/scroll.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "game/ui_zone.h"

static int g_message_box_up = 0;
static int g_toast_up = 0;
static entity_id_t g_shown_message_box;

message_box_t *shown_message_box(void) {
    return (message_box_t *)entity_get(g_shown_message_box);
}

void message_box_set_shown(message_box_t *mh) {
    g_shown_message_box = entity_id_link(mh != NULL ? &mh->header : NULL);
}

void message_box_construct(message_box_t *self) {
    message_box_t *previous;

    if (self == NULL) {
        return;
    }

    previous = shown_message_box();
    if (previous != NULL) {
        previous->is_dead = 1;
    }
    message_box_set_shown(self);
}

static void message_box_draw_overlay_method(entity_t *self, const vec_t *screen_pos) {
    (void)screen_pos;
    message_box_draw_overlay(dispatch_context_gfx(), dispatch_context_mem(), (message_box_t *)self);
}

void message_box_draw_overlay(p8_graphics_t *gfx, p8_vram_t *mem, message_box_t *self) {
    p8_fixed_t self_t, rh, t, nc, t3;

    if (self == NULL) {
        return;
    }

    self_t = self->header.state_time;
    rh = self->duration_scale;
    t = p8_fixed_sub(p8_fixed_div(self_t, rh), p8_fixed_from_int(10));

    if (t >= 0) {
        if (self->keep_open != NULL && self->keep_open(self)) {
            self->header.state_time = p8_fixed_mul(rh, p8_fixed_from_int(10));
            t = p8_fixed_from_int(0);
        }
    }

    t3 = p8_fixed_mul(p8_fixed_mul(t, t), t);
    nc = p8_fixed_div(p8_fixed_neg(t3), p8_fixed_from_int(9));

    if (gfx != NULL && mem != NULL) {
        draw_functions_draw_rows(gfx, mem, self->items, self->items_count, nc);
        p8_camera_reset(gfx);
    }

    if (t > P8_FIXED_LIT(0x000a8000, 10.5)) {
        self->is_dead = 1;
        message_box_set_shown(NULL);
        assert(shown_message_box() == NULL && "message_box_draw_overlay: the closing box marks itself dead and clears the shown message box");
    }
}

static void message_box_release(entity_t *self) {
    arena_release(((message_box_t *)self)->slot);
}

static const entity_class_t message_box_class_def = {
    .draw_overlay = message_box_draw_overlay_method,
    .tags = TAG(TAG_MESSAGE_BOX),
    .release = message_box_release,
};

typedef struct {
    data_row_t row;
    data_item_t items[3];
    vec_t pos;
} message_box_draw_row_t;

static const data_row_t *message_box_article(const allocator_t *slot, const data_row_t *e) {
    message_box_draw_row_t *copy;
    const vec_t *ra;

    if (e->draw_function != DRAW_FUNCTION_DRAW_ROW) {
        return e;
    }
    assert(e->count == 3 && e->items[0].is_ref && e->items[1].is_ref &&
           "message_box_article: a draw_row article is {layout row, position, alignment}: the box copies its layout row and position");
    copy = (message_box_draw_row_t *)allocator_alloc(slot, sizeof *copy);
    ra = e->items[1].ref;
    if (ra != NULL) {
        copy->pos = *ra;
    }
    copy->items[0] = (data_item_t){.is_ref = 1, .ref = layout_row_copy(slot, e->items[0].ref)};
    copy->items[1] = (data_item_t){.is_ref = 1, .ref = ra != NULL ? &copy->pos : NULL};
    copy->items[2] = e->items[2];
    copy->row = *e;
    copy->row.items = copy->items;
    return &copy->row;
}

static message_box_t *message_box_create(const message_box_keys_t *keys) {
    const allocator_t *slot = ui_zone_slot(SLOT_KIND_MESSAGE_BOX);
    message_box_t *self = (message_box_t *)allocator_alloc(slot, sizeof *self);
    int i;

    self->slot = slot;
    entity_init_instance(&self->header, &message_box_class_def, message_box_archetype.state, message_box_archetype.state_time);
    self->archetype = &message_box_archetype;
    self->header.layer = &self->archetype->layer;
    self->duration_scale = self->archetype->duration_scale;
    self->keep_open = NULL;
    self->header.is_dead = &self->is_dead;
    self->is_dead = 0;
    self->items_count = 0;
    if (keys != NULL) {
        assert(keys->count <= MESSAGE_BOX_ITEMS_CAP &&
               "message_box_create: the game builds two boxes, score_panel_new_year's (3 articles at most) and score_panel_update's (one per score row): a third needs more");
        for (i = 0; i < keys->count; i++) {
            self->items[self->items_count++] = message_box_article(slot, keys->items[i]);
        }
        if (keys->has_duration_scale) {
            self->duration_scale = keys->duration_scale;
        }
        self->keep_open = keys->keep_open;
    }
    entity_construct(&self->header);
    message_box_construct(self);
    return self;
}

void message_box_init(void) {
    message_box_shutdown();
    g_message_box_up = 1;
}

void message_box_shutdown(void) {
    g_message_box_up = 0;
    message_box_set_shown(NULL);
}

int message_box_ready(void) {
    return g_message_box_up;
}

message_box_t *message_box_new(const message_box_keys_t *keys) {
    return message_box_create(keys);
}

static vec_t toast_vec_or_zero(const vec_t *p) {
    if (p != NULL) {
        return *p;
    }
    return vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
}

void toast_construct(toast_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->pos == NULL) {
        const mouse_t *je = main_mouse_typed();
        const camera_t *mo = main_camera_typed();
        vec_t main_mouse_pos = toast_vec_or_zero(je != NULL ? je->pos : NULL);
        vec_t main_camera_pos = toast_vec_or_zero(mo != NULL ? mo->pos : NULL);
        vec_t offset = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(5));
        self->pos_box = vec_sub(vec_add(main_mouse_pos, main_camera_pos), offset);
        self->pos = &self->pos_box;
    }
}

void toast_update(toast_t *self) {
    p8_fixed_t new_l;

    if (self == NULL) {
        return;
    }

    self->pos_box = vec_add(toast_vec_or_zero(self->pos), toast_vec_or_zero(&self->archetype->velocity));
    self->pos = &self->pos_box;

    new_l = p8_fixed_add(self->fade, P8_FIXED_LIT(0x00001999, 0.1));
    self->fade = new_l;

    self->is_dead = new_l >= p8_fixed_from_int(5);
}

static void toast_update_method(entity_t *self, p8_fixed_t t) {
    (void)t;
    toast_update((toast_t *)self);
}

void toast_draw(const p8_graphics_t *gfx, p8_vram_t *mem, const toast_t *self, vec_t p) {
    const char *text;
    int color;
    p8_fixed_t a_center = P8_FIXED_LIT(0x00008000, 0.5);

    if (self == NULL || gfx == NULL || mem == NULL) {
        return;
    }

    hud_select_palette_block(mem, self->fade);

    text = self->text != NULL ? self->text : "";
    color = p8_fixed_flr_int(self->color);

    hud_print_outlined(gfx, mem, text, p.x, p.y, color, &a_center);

    hud_select_palette_block_default(mem);
}

static void toast_draw_method(entity_t *self, const vec_t *screen_pos) {
    vec_t p = screen_pos != NULL ? *screen_pos : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    toast_draw(dispatch_context_gfx(), dispatch_context_mem(), (const toast_t *)self, p);
}

static void toast_release(entity_t *self) {
    arena_release(((toast_t *)self)->slot);
}

static const entity_class_t toast_class_def = {
    .draw = toast_draw_method,
    .states = {[ENTITY_STATE_NORMAL] = toast_update_method},
    .release = toast_release,
};

static toast_t *toast_create(const toast_keys_t *keys) {
    const allocator_t *slot = ui_zone_slot(SLOT_KIND_TOAST);
    toast_t *self = (toast_t *)allocator_alloc(slot, sizeof *self);

    self->slot = slot;

    entity_init_instance(&self->header, &toast_class_def, toast_archetype.state, toast_archetype.state_time);
    self->archetype = &toast_archetype;
    self->header.layer = &self->archetype->layer;
    self->header.pos = &self->pos;
    self->fade = self->archetype->fade;
    self->pos = NULL;
    self->header.is_dead = &self->is_dead;
    self->is_dead = 0;
    self->text = NULL;
    self->color = p8_fixed_from_int(0);
    if (keys != NULL) {
        self->pos = keys->pos;
        if (keys->text != NULL) {
            assert(strlen(keys->text) <= TOAST_TEXT_CAP &&
                   "toast_create: a toast's text is a price or \"-no money-\": a longer one needs a bigger text_buf and toast slot");
            strcpy(self->text_buf, keys->text);
            self->text = self->text_buf;
        }
        self->color = keys->color;
    }
    toast_construct(self);
    entity_construct(&self->header);
    return self;
}

void toast_init(void) {
    toast_shutdown();
    g_toast_up = 1;
}

void toast_shutdown(void) {
    g_toast_up = 0;
}

int toast_ready(void) {
    return g_toast_up;
}

toast_t *toast_new(const toast_keys_t *keys) {
    return toast_create(keys);
}

typedef struct {
    const open_box_frame_t *frame;
    vec_t button_pos;
    vec_t pos;
} open_box_ctx_t;

static button_t *open_box_step(open_box_ctx_t *ctx, const button_keys_t *row) {
    button_keys_t b = *row;
    vec_t qf = ctx->button_pos;
    vec_t gq;

    b.pos = &ctx->pos;
    b.slide_target = &qf;

    /* GAP-9 */
    if (ctx->frame->has_back_color) {
        b.layer = ctx->frame->back_color;
        b.has_layer = 1;
    }

    gq = ctx->frame->row_step != NULL ? *ctx->frame->row_step : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    ctx->button_pos = vec_add(ctx->button_pos, gq);

    return button_new(&b);
}

static void open_box_add(entity_id_list_t *le, button_t *button) {
    if (le != NULL) {
        entity_id_list_add(le, entity_id_link(&button->header));
    }
}

void message_box_open_box(const open_box_frame_t *frame, const button_keys_t *buttons, int button_count) {
    layout_row_cell_t description_cell;
    vec_t box_pos;
    open_box_ctx_t step_context;
    entity_id_list_t *ephemerals;
    ui_t *dispatcher;
    button_keys_t description_row = {0};
    layout_row_keys_t description_layout_row = {0};
    int has_description;
    int index;

    if (frame == NULL) {
        return;
    }

    has_description = frame->description != NULL;
    if (has_description) {
        description_cell = layout_row_cell_string(frame->description);
        description_layout_row.items = &description_cell;
        description_layout_row.count = 1;
        description_layout_row.has_color = 1;
        description_layout_row.color = p8_fixed_from_int(13);
        description_row.layout_row_keys = &description_layout_row;
    }

    if (frame->fixed_pos != NULL) {
        box_pos = *frame->fixed_pos;
    } else if (frame->pos != NULL) {
        box_pos = *frame->pos;
    } else {
        box_pos = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    }

    step_context.frame = frame;
    step_context.button_pos = vec_add(box_pos, vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0)));
    step_context.pos = box_pos;

    dispatcher = main_ui();
    ui_clear_ephemerals(dispatcher);

    ephemerals = dispatcher != NULL ? ui_ephemerals_new() : NULL;
    if (has_description) {
        open_box_add(ephemerals, open_box_step(&step_context, &description_row));
    }
    for (index = 0; index < button_count; index++) {
        open_box_add(ephemerals, open_box_step(&step_context, &buttons[index]));
    }
    if (dispatcher != NULL) {
        ui_set_ephemerals(dispatcher, ephemerals);
    }

    if (frame->clear_ephemerals_after && dispatcher != NULL) {
        ui_set_ephemerals(dispatcher, ui_ephemerals_new());
    }
}
