#include "platform/psx_gpu_textures.h"

#include "platform/psx_colour.h"

void psx_gpu_textures_clut(const unsigned char draw_palette[PSX_GPU_TEXTURES_CLUT_ENTRIES],
                           uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES]) {
    for (int i = 0; i < PSX_GPU_TEXTURES_CLUT_ENTRIES; i++) {
        clut[i] = psx_colour_clut_entry(draw_palette[i]);
    }
}
