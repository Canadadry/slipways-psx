#include "shapes.h"

int main(void) {
    figure_t s = { 3, 4 };
    camera_t camera = { 0 };
    figure_t *alias = (figure_t *)&s;
    camera.p = FIGURE_AREA(alias);
    figure_count = camera.p;
    return shape_area(&s) == 0;
}
