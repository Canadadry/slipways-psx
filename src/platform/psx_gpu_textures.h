#ifndef PLATFORM_PSX_GPU_TEXTURES_H
#define PLATFORM_PSX_GPU_TEXTURES_H

#include <stdint.h>

enum { PSX_GPU_TEXTURES_CLUT_ENTRIES = 16 };

/* spr's CLUT: entry i is psx_colour_clut_entry(draw_palette[i]), so 0x0000 is transparent. */
void psx_gpu_textures_clut(const unsigned char draw_palette[PSX_GPU_TEXTURES_CLUT_ENTRIES],
                           uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES]);

#endif
