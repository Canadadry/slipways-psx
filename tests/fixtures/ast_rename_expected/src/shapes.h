#ifndef FIGURES_H
#define FIGURES_H

#define FIGURE_SCALE 4
#define FIGURE_AREA(s) ((s)->position * (s)->t)

typedef struct {
    int position;
    int t;
} figure_t;

typedef struct {
    int p;
} camera_t;

extern int figure_count;

int shape_area(const figure_t *s);

#endif
