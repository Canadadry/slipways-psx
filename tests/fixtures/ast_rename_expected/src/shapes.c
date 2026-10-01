#include <assert.h>
#include <stddef.h>

#include "shapes.h"

static const int button_sequence[] = {1, 2, 3};
#define SHAPE_SIZE sizeof(figure_t)
#define SEQUENCE_LENGTH (sizeof button_sequence / sizeof button_sequence[0])

static const figure_t unit_shape = { .position = 1, .t = 2 };

int figure_count;

#ifdef SHAPES_COVERAGE
int coverage_hits;
#endif

/* shape_t is mentioned here, and in "shape_t" below: neither is renamed. */
int shape_area(const figure_t *s) {
    assert(SHAPE_SIZE > 0);
    assert(s != NULL && s->position >= 0);
    const char *label = "héllo shape_t"; int area_base = s->position + figure_count;
    (void)label;
#ifdef SHAPES_COVERAGE
    coverage_hits++;
#endif
    return (int)sizeof(figure_t) + area_base * FIGURE_SCALE + (int)SEQUENCE_LENGTH + unit_shape.t;
}
