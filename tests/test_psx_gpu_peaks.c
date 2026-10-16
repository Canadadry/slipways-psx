#include "platform/psx_gpu_peaks.h"

#include <assert.h>
#include <stdio.h>

static void test_each_field_keeps_its_own_maximum(void) {
    psx_gpu_peaks_t peaks;
    psx_gpu_peaks_reset(&peaks);
    psx_gpu_peaks_record(&peaks, 1000, 1, 3, 0);
    psx_gpu_peaks_record(&peaks, 4000, 2, 0, 8180);
    psx_gpu_peaks_record(&peaks, 2000, 5, 1, 96);
    assert(peaks.bytes == 4000);
    assert(peaks.cluts == 5);
    assert(peaks.patterns == 3);
    assert(peaks.build_bytes == 8180);
}

static void test_reset_zeroes_every_field(void) {
    psx_gpu_peaks_t peaks;
    psx_gpu_peaks_reset(&peaks);
    psx_gpu_peaks_record(&peaks, 18280, 3, 1, 8180);
    psx_gpu_peaks_reset(&peaks);
    assert(peaks.bytes == 0);
    assert(peaks.cluts == 0);
    assert(peaks.patterns == 0);
    assert(peaks.build_bytes == 0);
    psx_gpu_peaks_record(&peaks, 10, 0, 0, 0);
    assert(peaks.bytes == 10);
}

int main(void) {
    test_each_field_keeps_its_own_maximum();
    test_reset_zeroes_every_field();
    printf("test_psx_gpu_peaks: ok\n");
    return 0;
}
