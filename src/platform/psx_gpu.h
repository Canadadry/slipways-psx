#ifndef PLATFORM_PSX_GPU_H
#define PLATFORM_PSX_GPU_H

#include "p8/draw_backend.h"

/* After ResetGraph: sets up the NTSC 320x240 display and the VRAM layout of
 * platform/psx_vram_layout.h, and uploads the packed sprite sheet, the font texture and its CLUTs. */
void psx_gpu_init(void);

/* Static table, never freed. Draws into the frame begun by psx_gpu_frame_begin. */
const p8_draw_backend_t *psx_gpu_backend(void);

/* Every draw call before the next psx_gpu_frame_end goes into this frame's primitive buffer; frame
 * names it in a p8_fatal on overflow. */
void psx_gpu_frame_begin(int frame);

/* Clears this frame's display buffer and appends the x2 upscale, waits for the previous frame,
 * shows it and submits this one, with no VSync wait. p8_fatal when the frame sent more bytes than metered_bytes. */
void psx_gpu_frame_end(unsigned long metered_bytes);

/* After psx_gpu_frame_end: waits for the frame it submitted and shows it at once. The next
 * psx_gpu_frame_end shows that same frame again, then moves on as usual. */
void psx_gpu_frame_show_last(void);

#endif
