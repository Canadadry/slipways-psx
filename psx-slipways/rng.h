#ifndef RNG_H
#define RNG_H

#include <stdint.h>

typedef struct {
    uint32_t state;
} rng_t;

void rng_seed(rng_t *rng, uint32_t seed);

uint32_t rng_next_u32(rng_t *rng);

uint32_t rng_next_range(rng_t *rng, uint32_t min, uint32_t max_exclusive);

#endif
