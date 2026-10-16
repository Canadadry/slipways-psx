#include "platform/psx_gpu_peaks.h"

#include <assert.h>

void psx_gpu_peaks_reset(psx_gpu_peaks_t *peaks) {
    assert(peaks);
    *peaks = (psx_gpu_peaks_t){ 0, 0, 0, 0 };
}

void psx_gpu_peaks_record(psx_gpu_peaks_t *peaks, unsigned long bytes, int cluts, int patterns,
                          unsigned long build_bytes) {
    assert(peaks);
    assert(cluts >= 0 && patterns >= 0);
    if (bytes > peaks->bytes) peaks->bytes = bytes;
    if (cluts > peaks->cluts) peaks->cluts = cluts;
    if (patterns > peaks->patterns) peaks->patterns = patterns;
    if (build_bytes > peaks->build_bytes) peaks->build_bytes = build_bytes;
}
