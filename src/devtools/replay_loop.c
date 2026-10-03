#include "devtools/replay_loop.h"

#include <assert.h>

#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "p8/assets.h"

void replay_loop_open(replay_loop_t *r, const char *events_path) {
    p8_mem_init(&r->memory);
    p8_gfx_init(&r->graphics);
    assert(p8_load_gfx_json(&r->memory, "pico8/gfx.json"));
    assert(p8_load_map_json(&r->memory, "pico8/map.json"));
    p8_input_init(&r->input);
    assert(p8_input_load(&r->input, events_path));
    dispatch_context_set_gfx(&r->graphics);
    dispatch_context_set_mem(&r->memory);
    dispatch_context_set_input(&r->input);
    cart_load();
    cart_init();
    cheat_init(&r->cheat);
}

int replay_loop_frames(const replay_loop_t *r) {
    return r->input.frame_count;
}

void replay_loop_play_frame(replay_loop_t *r, int i) {
    dispatch_context_set_input(&r->input);
    cheat_step(&r->cheat, &r->input);
    cart_update60();
    cart_draw();
    if (i < r->input.frame_count - 1) p8_input_step(&r->input);
}

void replay_loop_close(replay_loop_t *r) {
    p8_input_free(&r->input);
}
