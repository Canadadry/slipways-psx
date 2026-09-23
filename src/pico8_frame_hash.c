#include "pico8_frame_hash.h"

#include "pico8_screencast.h"

#include <assert.h>

#define FNV1A64_OFFSET_BASIS UINT64_C(0xcbf29ce484222325)
#define FNV1A64_PRIME UINT64_C(0x100000001b3)

uint64_t pico8_frame_hash(const unsigned char *pixels) {
    assert(pixels);

    uint64_t hash = FNV1A64_OFFSET_BASIS;
    for (int k = 0; k < PICO8_SCREENCAST_FRAME_BYTES; k++) {
        unsigned char low = pixels[2 * k];
        unsigned char high = pixels[2 * k + 1];
        assert(low <= 0x0f && high <= 0x0f && "un pixel est un index de couleur 0-15");
        hash ^= (uint64_t)(unsigned char)(low | (high << 4));
        hash *= FNV1A64_PRIME;
    }
    return hash;
}
