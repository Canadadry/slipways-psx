#include "platform/psx_sprite.h"

#include "p8/vram.h"
#include "platform/psx_tiles.h"

#include <assert.h>

enum { SHEET_COLUMNS = P8_SPRITE_SHEET_WIDTH / P8_SPRITE_SIZE };

/* Moves the near edge of [*start, *start + *size) up to 0 and the far one down to limit, *texel
 * following *start. Returns 0 when nothing is left. */
static int clip_axis(int *start, int *texel, int *size, int limit) {
    if (*start < 0) {
        *texel -= *start;
        *size += *start;
        *start = 0;
    }
    if (*start + *size > limit) {
        *size = limit - *start;
    }
    return *size > 0;
}

int psx_sprite_clip(psx_sprite_t *sprite) {
    return clip_axis(&sprite->x, &sprite->u, &sprite->w, P8_SCREEN_WIDTH)
        && clip_axis(&sprite->y, &sprite->v, &sprite->h, P8_SCREEN_HEIGHT);
}

int psx_sprite_place(const p8_graphics_t *gfx, int n, int x, int y, int w, int h, psx_sprite_t *sprite) {
    if (w <= 0 || h <= 0) {
        return 0;
    }
    psx_sprite_t placed = {
        x - gfx->camera_x, y - gfx->camera_y, (n % SHEET_COLUMNS) * P8_SPRITE_SIZE, (n / SHEET_COLUMNS) * P8_SPRITE_SIZE,
        w * P8_SPRITE_SIZE, h * P8_SPRITE_SIZE,
    };
    if (!psx_sprite_clip(&placed)) {
        return 0;
    }
    assert(n >= 0 && placed.u + placed.w <= P8_SPRITE_SHEET_WIDTH && placed.v + placed.h <= P8_SPRITE_SHEET_HEIGHT
           && "the SPRT would read past the sheet, where the software draws colour 0: no replay does");
    *sprite = placed;
    return 1;
}

psx_sprite_t psx_sprite_double(psx_sprite_t sprite) {
    psx_sprite_t doubled = {
        sprite.x * PSX_TILES_SCALE, sprite.y * PSX_TILES_SCALE, sprite.u * PSX_TILES_SCALE,
        sprite.v * PSX_TILES_SCALE, sprite.w * PSX_TILES_SCALE, sprite.h * PSX_TILES_SCALE,
    };
    return doubled;
}
