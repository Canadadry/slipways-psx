#include "../src/pico8_frame_hash.h"
#include "../src/pico8_screencast.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vecteurs calcules hors du code teste (FNV-1a 64 en Python sur les 8192
 * octets empaquetes au format de la capture). */
#define HASH_OF_ALL_ZERO_FRAME UINT64_C(0xb9d103fd6854a325)
#define HASH_OF_PIXEL_I_EQUALS_I_MOD_16 UINT64_C(0x620576d025a56325)

static unsigned char frame[PICO8_SCREENCAST_FRAME_PIXELS];

static void fill_pattern(void) {
    for (int i = 0; i < PICO8_SCREENCAST_FRAME_PIXELS; i++) {
        frame[i] = (unsigned char)(i % 16);
    }
}

static void test_known_vector_of_all_zero_frame(void) {
    memset(frame, 0, sizeof(frame));
    assert(pico8_frame_hash(frame) == HASH_OF_ALL_ZERO_FRAME);
}

static void test_known_vector_of_fixed_pattern(void) {
    fill_pattern();
    assert(pico8_frame_hash(frame) == HASH_OF_PIXEL_I_EQUALS_I_MOD_16);
}

static void test_identical_frames_give_identical_hashes(void) {
    static unsigned char copy[PICO8_SCREENCAST_FRAME_PIXELS];
    fill_pattern();
    memcpy(copy, frame, sizeof(frame));
    assert(pico8_frame_hash(frame) == pico8_frame_hash(copy));
}

/* Pixel pair = quartet de poids faible de l'octet empaquete, impair =
 * poids fort ; bit 0 et bit 3 couvrent les deux bouts du quartet. */
static void assert_changing_one_pixel_changes_hash(int index, unsigned char flip) {
    fill_pattern();
    uint64_t before = pico8_frame_hash(frame);
    frame[index] = (unsigned char)(frame[index] ^ flip);
    assert(pico8_frame_hash(frame) != before);
}

static void test_one_pixel_anywhere_changes_hash(void) {
    const int last = PICO8_SCREENCAST_FRAME_PIXELS - 1;
    const int indices[] = {0, 1, last - 1, last, 8191, 8192};
    const unsigned char flips[] = {0x1, 0x8};
    for (size_t i = 0; i < sizeof(indices) / sizeof(indices[0]); i++) {
        for (size_t f = 0; f < sizeof(flips); f++) {
            assert_changing_one_pixel_changes_hash(indices[i], flips[f]);
        }
    }
}

int main(void) {
    test_known_vector_of_all_zero_frame();
    test_known_vector_of_fixed_pattern();
    test_identical_frames_give_identical_hashes();
    test_one_pixel_anywhere_changes_hash();
    printf("test_pico8_frame_hash: all tests passed\n");
    return 0;
}
