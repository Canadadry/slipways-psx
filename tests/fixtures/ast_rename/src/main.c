#include "shapes.h"

int main(void) {
    shape_t s = { 3, 4 };
    camera_t camera = { 0 };
    shape_t *alias = (shape_t *)&s;
    camera.p = SHAPE_AREA(alias);
    shape_count = camera.p;
    return shape_area(&s) == 0;
}
