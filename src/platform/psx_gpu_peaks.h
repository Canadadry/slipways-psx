#ifndef PLATFORM_PSX_GPU_PEAKS_H
#define PLATFORM_PSX_GPU_PEAKS_H

/* Each field is its own peak over the frames recorded since the reset. */
typedef struct {
    unsigned long bytes;
    int cluts;
    int patterns;
    unsigned long build_bytes;
} psx_gpu_peaks_t;

void psx_gpu_peaks_reset(psx_gpu_peaks_t *peaks);

void psx_gpu_peaks_record(psx_gpu_peaks_t *peaks, unsigned long bytes, int cluts, int patterns,
                          unsigned long build_bytes);

#endif
