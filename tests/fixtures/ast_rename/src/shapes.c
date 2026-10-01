#include <assert.h>
#include <stddef.h>

#include "shapes.h"

static const int sequence[] = {1, 2, 3};
#define SHAPE_SIZE sizeof(shape_t)
#define SEQUENCE_LENGTH (sizeof sequence / sizeof sequence[0])

static const shape_t unit_shape = { .p = 1, .t = 2 };

int shape_count;

#ifdef SHAPES_COVERAGE
int shape_coverage_hits;
#endif

/* shape_t is mentioned here, and in "shape_t" below: neither is renamed. */
int shape_area(const shape_t *s) {
    assert(SHAPE_SIZE > 0);
    assert(s != NULL && s->p >= 0);
    const char *label = "héllo shape_t"; int ab = s->p + shape_count;
    (void)label;
#ifdef SHAPES_COVERAGE
    shape_coverage_hits++;
#endif
    return (int)sizeof(shape_t) + ab * SHAPE_SCALE + (int)SEQUENCE_LENGTH + unit_shape.t;
}
