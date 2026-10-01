#ifndef SHAPES_H
#define SHAPES_H

#define SHAPE_SCALE 4
#define SHAPE_AREA(s) ((s)->p * (s)->t)

typedef struct {
    int p;
    int t;
} shape_t;

typedef struct {
    int p;
} camera_t;

extern int shape_count;

int shape_area(const shape_t *s);

#endif
