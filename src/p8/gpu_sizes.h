#ifndef P8_GPU_SIZES_H
#define P8_GPU_SIZES_H

/* Sized at the make state peak + 1/8: GPU_SHIPPED_FRAME_BUFFER_BYTES rounded up to 1 KiB, the
 * slots to a whole slot. */
#define GPU_SHIPPED_FRAME_BUFFER_BYTES 73728
/* Probe builds (make psx-probe) get room for a probe that calls cart_draw twice; the backend, the
 * meter's gpu_frame zone and the overrun check all read this one definition. */
#ifdef PSX_PROBE
#define GPU_FRAME_BUFFER_BYTES (2 * GPU_SHIPPED_FRAME_BUFFER_BYTES)
#else
#define GPU_FRAME_BUFFER_BYTES GPU_SHIPPED_FRAME_BUFFER_BYTES
#endif
#define GPU_CLUT_SLOTS 6
#define GPU_PATTERN_SLOTS 4

#endif
