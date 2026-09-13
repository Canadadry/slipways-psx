#include "planetgen.h"

#include <stddef.h>

#define PLANETGEN_PLACEMENT_MIN ((fixed_t)(0.125 * FX_ONE))
#define PLANETGEN_PLACEMENT_SPAN ((fixed_t)(2.75 * FX_ONE))
#define PLANETGEN_MAX_ATTEMPTS_PER_POINT 10000

static int planetgen_fits(const galaxy_t *g, fixed_t x, fixed_t y, int64_t min_dist_sq) {
    for (int i = 0; i < g->count; i++) {
        int64_t dx = (int64_t)fixed_sub(x, g->positions[i].x);
        int64_t dy = (int64_t)fixed_sub(y, g->positions[i].y);
        int64_t dist_sq = dx * dx + dy * dy;
        if (dist_sq <= min_dist_sq) {
            return 0;
        }
    }
    return 1;
}

int planetgen_generate(galaxy_t *out, int count, int64_t min_dist_sq, rng_t *rng) {
    if (out == NULL || rng == NULL) {
        return PLANETGEN_FAIL;
    }
    if (count < 0 || count > PLANETGEN_MAX_PLANETS) {
        out->count = 0;
        return PLANETGEN_FAIL;
    }

    out->count = 0;
    for (int i = 0; i < count; i++) {
        int placed = 0;
        for (int attempt = 0; attempt < PLANETGEN_MAX_ATTEMPTS_PER_POINT; attempt++) {
            fixed_t x = fixed_add(PLANETGEN_PLACEMENT_MIN, (fixed_t)rng_next_range(rng, 0u, (uint32_t)PLANETGEN_PLACEMENT_SPAN));
            fixed_t y = fixed_add(PLANETGEN_PLACEMENT_MIN, (fixed_t)rng_next_range(rng, 0u, (uint32_t)PLANETGEN_PLACEMENT_SPAN));

            if (planetgen_fits(out, x, y, min_dist_sq)) {
                out->positions[out->count].x = x;
                out->positions[out->count].y = y;
                out->count++;
                placed = 1;
                break;
            }
        }
        if (!placed) {
            out->count = 0;
            return PLANETGEN_FAIL;
        }
    }

    return PLANETGEN_OK;
}
