#ifndef PLANETGEN_H
#define PLANETGEN_H

#include <stdint.h>

#include "fixed.h"
#include "rng.h"

#define PLANETGEN_MAX_PLANETS 90

#define PLANETGEN_OK 0
#define PLANETGEN_FAIL (-1)

typedef struct {
    fixed_t x;
    fixed_t y;
} planet_pos_t;

typedef struct {
    planet_pos_t positions[PLANETGEN_MAX_PLANETS];
    int count;
} galaxy_t;

int planetgen_generate(galaxy_t *out, int count, int64_t min_dist_sq, rng_t *rng);

#endif
