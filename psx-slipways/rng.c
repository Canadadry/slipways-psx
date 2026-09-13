#include "rng.h"

void rng_seed(rng_t *rng, uint32_t seed) {
    rng->state = (seed != 0u) ? seed : 1u;
}

uint32_t rng_next_u32(rng_t *rng) {
    uint32_t x = rng->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng->state = x;
    return x;
}

uint32_t rng_next_range(rng_t *rng, uint32_t min, uint32_t max_exclusive) {
    uint32_t span = max_exclusive - min;
    if (span == 0u) {
        return min;
    }
    return min + (rng_next_u32(rng) % span);
}
