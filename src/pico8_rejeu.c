#include "pico8_rejeu.h"

#include <assert.h>

#include "pico8_assets.h"
#include "pico8_cart.h"
#include "pico8_dispatch_context.h"

void pico8_rejeu_ouvrir(pico8_rejeu_t *r, const char *events_path) {
    pico8_mem_init(&r->mem);
    pico8_gfx_init(&r->gfx);
    assert(pico8_load_gfx_json(&r->mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&r->mem, "pico8/map.json"));
    pico8_input_init(&r->input);
    assert(pico8_input_load(&r->input, events_path));
    pico8_dispatch_set_gfx(&r->gfx);
    pico8_dispatch_set_mem(&r->mem);
    pico8_dispatch_set_input(&r->input);
    pico8_cart_load();
    pico8__init();
    pico8_cheat_init(&r->cheat);
}

int pico8_rejeu_frames(const pico8_rejeu_t *r) {
    return r->input.frame_count;
}

void pico8_rejeu_jouer_frame(pico8_rejeu_t *r, int i) {
    pico8_dispatch_set_input(&r->input);
    pico8_cheat_step(&r->cheat, &r->input);
    pico8__update60();
    pico8__draw();
    if (i < r->input.frame_count - 1) pico8_input_step(&r->input);
}

void pico8_rejeu_fermer(pico8_rejeu_t *r) {
    pico8_input_free(&r->input);
}
