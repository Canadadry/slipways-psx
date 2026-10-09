#ifndef P8_GPU_SIZES_H
#define P8_GPU_SIZES_H

/* Sized at the make state peak + 1/8: GPU_FRAME_BUFFER_BYTES rounded up to 1 KiB, the slots to a
 * whole slot. */
#define GPU_FRAME_BUFFER_BYTES 62464
#define GPU_CLUT_SLOTS 6
#define GPU_PATTERN_SLOTS 4

#endif
