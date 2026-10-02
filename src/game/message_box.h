#ifndef GAME_MESSAGE_BOX_H
#define GAME_MESSAGE_BOX_H

#include "engine/entity_class.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/ui_button.h"
#include "engine/vec.h"

/* Throughout this module a NULL instance argument is a safe no-op, and an
 * absent or wrong-kind field reads as its zero/false default. */

typedef struct message_box message_box_t;

/* `mh.lh`: the Lua truth of what it returns, which `mh:ev` tests. */
typedef int (*message_box_keep_open_fn_t)(message_box_t *self);

/* The cart builds a box twice: `pj:hk` (up to 3 articles) and `pj:ph`
 * (one per row of `bz`). */
enum { MESSAGE_BOX_ITEMS_CAP = SCORE_ROWS_COUNT };

/* `items` is the cart's `self[1..n]`, the option row `mh:ev` lays out
 * through `bl`, in the order the construction table stored them. An
 * article `ld` is a copy in `place`, its `lq` and vector included; any
 * other stays shared with the caller, as the cart's box, a shallow copy of
 * its table, shares the rows of `pj.ea` and `bz`. The struct lives in
 * `place`, given back by the exit queue once `he` takes the box out. */
struct message_box {
    entity_t header;
    const allocator_t *slot;
    const message_box_archetype_t *archetype;
    p8_fixed_t duration_scale;
    message_box_keep_open_fn_t keep_open; /* NULL: `lh` is nil */
    int is_dead;
    const data_row_t *items[MESSAGE_BOX_ITEMS_CAP];
    int items_count;
};

/* The message box currently open, or NULL for none. The setter never frees
 * the box it replaces: `kc` borrows a struct that `lt` registered into
 * `hv`/`cv`/`cg`, and the exit queue resets it to NULL as it gives that
 * struct's place back. */
message_box_t *shown_message_box(void);
void message_box_set_shown(message_box_t *mh);

void message_box_init(void);

/* Clears `kc`, never frees any box. Idempotent, and safe before the first
 * `pico8_mh_init()`. */
void message_box_shutdown(void);

/* 1 from pico8_mh_init() to pico8_mh_shutdown(), 0 otherwise. */
int message_box_ready(void);

/* The cart's keys of a message box: `items` its positionals in order, each
 * an article of `bl` (an article `ld` copied, any other borrowed for the
 * box's life), `rh` under its flag, `lh` NULL for nil. */
typedef struct {
    const data_row_t *const *items;
    int count;
    int has_duration_scale; /* 0: `rh` keeps the archetype's */
    p8_fixed_t duration_scale;
    message_box_keep_open_fn_t keep_open;
} message_box_keys_t;

/* `keys` is read, not kept (NULL: none), but for the articles it borrows.
 * The struct registers itself through `lt`. */
message_box_t *message_box_new(const message_box_keys_t *keys);

void message_box_construct(message_box_t *self);

/* A NULL `gfx`/`mem` skips the drawing and the camera reset only; the
 * closing timer and the `kc` bookkeeping still run. */
void message_box_draw_overlay(p8_graphics_t *gfx, p8_memory_t *mem, message_box_t *self);

/* Calls pico8_pi_init(). */
void toast_init(void);

enum { TOAST_TEXT_CAP = 16 };

/* `at1`/`at2` are the cart's `self[1]` (the text) and `self[2]` (the forced
 * color). `at1` NULL is nil, else it points at `at1_buf`, a copy of the
 * key's text; a missing `[2]` leaves `at2` at 0, the color `kj` gave nil.
 * `ra` deliberately stays the key's vector, borrowed, until the first `ph`:
 * `jo:hk` passes its planet's `ra`, which `na` moves in place before that
 * `ph`, and the cart reads it moved. From then on, and when `pq` places the
 * toast itself, `ra` points at `ra_box`. The struct lives in `place`, given
 * back by the exit queue once `he` takes the toast out. */
typedef struct {
    entity_t header;
    const allocator_t *slot;
    vec_t pos_box;
    char text_buf[TOAST_TEXT_CAP + 1];
    const toast_archetype_t *archetype;
    p8_fixed_t fade;
    const vec_t *pos;
    int is_dead;
    const char *text;
    p8_fixed_t color;
} toast_t;

void toast_shutdown(void);

/* 1 from pico8_iv_init() to pico8_iv_shutdown(), 0 otherwise. */
int toast_ready(void);

/* The cart's keys `ra`, `[1]` and `[2]` of a toast. NULL or 0 is an absent
 * key, which leaves the member's default. */
typedef struct {
    const vec_t *pos;
    const char *text;
    p8_fixed_t color;
} toast_keys_t;

/* `keys` is read, not kept (NULL: none), but for `ra`, borrowed until the
 * first `ph`; `at1` is copied. The struct registers itself through `lt`. */
toast_t *toast_new(const toast_keys_t *keys);

void toast_construct(toast_t *self);
void toast_update(toast_t *self);

/* A NULL `gfx`/`mem` makes the whole call a no-op. */
void toast_draw(const p8_graphics_t *gfx, p8_memory_t *mem, const toast_t *self, vec_t p);

/* What `pz` reads of its `p`, filled by the caller from its own members: a
 * NULL pointer or a 0 has_* is nil; `ff` is the Lua truth of `p.ff`. `pz`
 * reads it during the call and keeps none of it. */
typedef struct {
    const char *description;
    int has_open_sfx;
    p8_fixed_t open_sfx;
    const vec_t *fixed_pos;
    int clear_ephemerals_after;
    int has_back_color;
    p8_fixed_t back_color;
    const vec_t *row_step;
    const vec_t *pos;
} open_box_frame_t;

/* The `n` rows of `bs` are the keys of `ok.le`'s buttons, each read on a
 * copy that gets `ra`, `qf` and `fa`; `bs` stays the caller's, unchanged.
 * The `le` it replaces goes to the exit queue. A NULL `frame` is a no-op. */
void message_box_open_box(const open_box_frame_t *frame, const button_keys_t *buttons, int button_count);

#endif
